/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#include "General.h"
// Engine
#include "engine.h"
#include "engine_da.h"
// DA
#include "da.h"
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsAuthentication.h"
#include "EssentialsUtils.h"

EssentialsAuthenticationHandler* EssentialsAuthenticationHandler::Instance = 0;
SList<EssentialsAuthUser> EssentialsAuthenticationManager::ProtectedUsers;

EssentialsAuthenticationHandler::EssentialsAuthenticationFilter filter;
EssentialsAuthenticationHandler::EssentialsAuthenticationHandler() {
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED, INT_MAX);
	Register_Event(DAEvent::GAMEOVER, INT_MAX);
	Register_Event(DAEvent::LEVELLOADED, INT_MAX);
	Register_Event(DAEvent::CONNECTIONREQUEST, INT_MAX);
	Register_Event(DAEvent::PLAYERJOIN, 1);

	addConnectionAcceptanceFilter(&filter);
	AuthMessage = "{PLAYER} is {LEVEL}.";
	AllowRegisteredOnly = false;
	DialogHookID = AddDialogHook(EssentialsAuthenticationHandler::Dialog_Event);
}

EssentialsAuthenticationHandler::~EssentialsAuthenticationHandler() {
	removeConnectionAcceptanceFilter(&filter);
	RemoveDialogHook(DialogHookID);
	Instance = 0;
}

void EssentialsAuthenticationHandler::Settings_Loaded_Event() {
	DASettingsManager::Get_String(AuthMessage, "Essentials", "AuthMessage", "{PLAYER} is {LEVEL}.");
	AllowRegisteredOnly = DASettingsManager::Get_Bool("Essentials", "AllowRegisteredOnly", false);
	EssentialsAuthenticationManager::Reload_Database();
}

void EssentialsAuthenticationHandler::Game_Over_Event() {
	// Game over causes scriptable dialog system to reset all dialogs.
	// To prevent client from being stuck in pre-join screen, reset dialogs on game over.
	for (int i = 0; i < AuthContexts.Count(); ++i) {
		if (!AuthContexts[i]->Dialog) continue;
		AuthContexts[i]->Destroy_Dialog();
	}
}

void EssentialsAuthenticationHandler::Level_Loaded_Event() {
	// Re-create dialogs for clients pending authentication.
	for (int i = 0; i < AuthContexts.Count(); ++i) {
		if (AuthContexts[i]->Dialog) continue;
		AuthContexts[i]->Create_Dialog();
	}
}

ConnectionAcceptanceFilter::STATUS EssentialsAuthenticationHandler::Connection_Request_Event(ConnectionRequest& Request, WideStringClass& RefusalMessage) {
	// This is to prevent allowing clients into the game after a name change
	// during authentication process. For a frame, the auth context gets
	// deleted to start over the authentication process.
	if (Requires_Authentication(Request)) {
		if (AuthenticatedIDs.ID(Request.clientId) == -1) {
			return ConnectionAcceptanceFilter::STATUS_INDETERMINATE;
		}
	}
	return ConnectionAcceptanceFilter::STATUS_ACCEPTING;
}

void EssentialsAuthenticationHandler::Player_Join_Event(cPlayer* Player) {
	if (AuthenticatedIDs.ID(Player->Get_Id()) != -1) {
		if (EssentialsAuthUser* AuthUser = EssentialsAuthenticationManager::Get_Auth_User(Player->Get_Name())) {
			Player->Get_DA_Player()->Set_Access_Level((DAAccessLevel::Level)AuthUser->AccessLevel);
			if (AuthUser->AccessLevel > 0) {
				StringClass authMessage(AuthMessage);
				authMessage.Replace("{PLAYER}", StringClass(Player->Get_Name()));

				switch (AuthUser->AccessLevel) {
					case 1: authMessage.Replace("{LEVEL}", "a VIP"); break;
					case 2: authMessage.Replace("{LEVEL}", "a temporary moderator"); break;
					case 3: authMessage.Replace("{LEVEL}", "a half moderator"); break;
					case 4: authMessage.Replace("{LEVEL}", "a full moderator"); break;
					case 5: authMessage.Replace("{LEVEL}", "a super moderator"); break;
					case 6: authMessage.Replace("{LEVEL}", "an administrator"); break;
					default: authMessage.Replace("{LEVEL}", "a staff"); break;
				}
				DA::Host_Message(authMessage);
			}
		}
		AuthenticatedIDs.DeleteObj(Player->Get_Id());
	}
}

void EssentialsAuthenticationHandler::EssentialsAuthenticationFilter::handleTermination(const ConnectionRequest& Request) {
	if (EssentialsAuthClient* AuthContext = Instance->Get_Auth_Context(const_cast<ConnectionRequest&>(Request))) {
		Instance->Cleanup_Context(AuthContext);
	}
}

ConnectionAcceptanceFilter::STATUS EssentialsAuthenticationHandler::EssentialsAuthenticationFilter::getStatus(const ConnectionRequest& constRequest, WideStringClass& refusalMessage) {
	ConnectionRequest& Request = const_cast<ConnectionRequest&>(constRequest);

	if (EssentialsAuthClient* AuthContext = Instance->Get_Auth_Context(Request)) {
		switch (AuthContext->AuthState) {
			case 1: {
				Console_Output("Authentication for %ws was successful.\n", Request.clientName);
				DALogManager::Write_Log("_ESSAUTH", "1 %d %ws 0", Request.clientId, Request.clientName);
				Instance->Cleanup_Context(AuthContext);
				Instance->AuthenticatedIDs.Add(Request.clientId);
				return STATUS_ACCEPTING;
			}
			case 2: {
				DALogManager::Write_Log("_ESSAUTH", "2 %d %ws %ws", Request.clientId, Request.clientName, AuthContext->NewNick);
				Request.clientName = AuthContext->NewNick;
				Instance->Cleanup_Context(AuthContext); // Start over.
				break;
			}
			case 3: {
				AuthContext->AuthState = 0;
				switch (Instance->Do_Password_Validation(AuthContext)) {
					case 0: {
						AuthContext->Password_Validation_Callback(false, L"Invalid password.");
						DALogManager::Write_Log("_ESSAUTH", "3.0 %d %ws 0", Request.clientId, Request.clientName);
						break;
					}
					case 1: {
						AuthContext->Password_Validation_Callback(true, L"Identification successful.");
						DALogManager::Write_Log("_ESSAUTH", "3.1 %d %ws 0", Request.clientId, Request.clientName);
						AuthContext->AuthState = 1;
						break;
					}
					case 4: {
						AuthContext->Password_Validation_Callback(false, L"Client does not need authentication.");
						DALogManager::Write_Log("_ESSAUTH", "3.2 %d %ws 0", Request.clientId, Request.clientName);
						break;
					}
				}
				break;
			}
			case 4: {
				DALogManager::Write_Log("_ESSAUTH", "4 %d %ws 0", Request.clientId, Request.clientName);
				refusalMessage = L"Unauthorized use of a protected name.";
				return STATUS_REFUSING;
			}
		}
	}
	else if (Instance->Requires_Authentication(Request)) {
		Instance->Do_Authentication(Request);
	}
	else {
		if (Instance->AllowRegisteredOnly) {
			refusalMessage = L"Only registered names can join this server.";
			return STATUS_REFUSING;
		}
		
		// Prevent false authentication.
		Instance->AuthenticatedIDs.DeleteObj(Request.clientId);
		return STATUS_ACCEPTING;
	}

	return STATUS_INDETERMINATE;
}

void EssentialsAuthenticationHandler::Dialog_Event(int ClientID, int DialogID, int ControlID, DialogMessageType Message) {
	if (ScriptedDialogClass* Dialog = Find_Dialog(DialogID)) {
		if (ScriptedControlClass* Control = Dialog->Find_Control(ControlID)) {
			if (EssentialsAuthClient* AuthContext = Instance->Get_Auth_Context(ClientID)) {
				AuthContext->Dialog_Message(Message, Dialog, Control);
			}
		}
	}
}

bool EssentialsAuthenticationHandler::Is_Authenticating(ConnectionRequest& Request) {
	return !!Get_Auth_Context(Request);
}

EssentialsAuthClient* EssentialsAuthenticationHandler::Get_Auth_Context(int Client) {
	for (int i = 0; i < AuthContexts.Count(); ++i) {
		if (AuthContexts[i]->Request->clientId == Client) {
			return AuthContexts[i];
		}
	}
	return nullptr;
}

EssentialsAuthClient* EssentialsAuthenticationHandler::Get_Auth_Context(ConnectionRequest& Request) {
	return Get_Auth_Context(Request.clientId);
}

void EssentialsAuthenticationHandler::Do_Authentication(ConnectionRequest& Request) {
	if (!Is_Authenticating(Request)) {
		EssentialsAuthClient* AuthContext = new EssentialsAuthClient(&Request);
		AuthContext->AuthState = 0;
		AuthContexts.Add(AuthContext);
	}
}

bool EssentialsAuthenticationHandler::Requires_Authentication(ConnectionRequest& Request) {
	if (EssentialsAuthenticationManager::Get_Auth_User(Request.clientName)) {
		return true;
	}
	return false;
}

void EssentialsAuthenticationHandler::Cleanup_Context(EssentialsAuthClient* AuthContext) {
	AuthContexts.DeleteObj(AuthContext);
	delete AuthContext;
}

int EssentialsAuthenticationHandler::Do_Nick_Change(EssentialsAuthClient* AuthContext, WideStringClass NewNick) {
	AuthContext->AuthState = 2;
	AuthContext->NewNick = NewNick;
	return 1;
}

int EssentialsAuthenticationHandler::Do_Password_Validation(EssentialsAuthClient* AuthContext) {
	if (EssentialsAuthUser* AuthUser = EssentialsAuthenticationManager::Get_Auth_User(AuthContext->Request->clientName)) {
		StringClass Hash;
		if (Get_MD5_Hash(AuthContext->PasswordAttempt, Hash)) {
			if (AuthUser->PasswordHash == Hash) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			return 0;
		}
	}
	return 2;
}

int EssentialsAuthenticationHandler::Do_Leave(EssentialsAuthClient* AuthContext) {
	AuthContext->AuthState = 4;
	return 1;
}

void EssentialsAuthenticationManager::Init() {
	if (!Is_Initialized()) {
		new EssentialsAuthenticationHandler;
		EssentialsAuthenticationHandler::Instance->Settings_Loaded_Event();
	}
}

void EssentialsAuthenticationManager::Shutdown() {
	if (Is_Initialized()) {
		delete EssentialsAuthenticationHandler::Instance;
	}
}

bool EssentialsAuthenticationManager::Is_Initialized() {
	return !!EssentialsAuthenticationHandler::Instance;
}

void EssentialsAuthenticationManager::Reload_Database() {
	if (!Is_Initialized()) {
		return;
	}

	Cleanup_Database();

	RawFileClass file(CREDENTIALS_DATABASE_FILENAME);
	file.Open(0);
	ChunkLoadClass loader(&file);
	
	while (loader.Open_Chunk()) {
		switch (loader.Cur_Chunk_ID()) {
			case CREDENTIALS_DATABASE_DBID: {
				while (loader.Open_Micro_Chunk()) {
					switch (loader.Cur_Micro_Chunk_ID()) {
						case CREDENTIALS_DATABASE_USERID: {
							EssentialsAuthUser* AuthUser = new EssentialsAuthUser;

							int nickLen = 0;
							loader.SimpleRead(nickLen);
							loader.Read(AuthUser->Nick.Get_Buffer(nickLen + 1), nickLen * 2);
							loader.Read(AuthUser->PasswordHash);
							loader.SimpleRead(AuthUser->AccessLevel);
							AuthUser->Nick[nickLen] = L'\0';

							ProtectedUsers.Add_Tail(AuthUser);
							break;
						}
					}
					loader.Close_Micro_Chunk();
				}
				break;
			}
		}
		loader.Close_Chunk();
	}
}

void EssentialsAuthenticationManager::Save_Database() {
	if (!Is_Initialized()) {
		return;
	}

	RawFileClass file(CREDENTIALS_DATABASE_FILENAME);
	if (!file.Open(2)) {
		Console_Output("[Essentials] Failed to write credentials database for Authentication System.\n");
		return;
	}

	ChunkSaveClass saver(&file);

	saver.Begin_Chunk(CREDENTIALS_DATABASE_DBID);
	for (SLNode<EssentialsAuthUser>* n = ProtectedUsers.Head(); n; n = n->Next()) {
		saver.Begin_Micro_Chunk(CREDENTIALS_DATABASE_USERID);
		int len = n->Data()->Nick.Get_Length();
		saver.SimpleWrite(len);
		saver.Write(n->Data()->Nick.Peek_Buffer(), len * 2);
		saver.Write(n->Data()->PasswordHash);
		saver.SimpleWrite(n->Data()->AccessLevel);
		saver.End_Micro_Chunk();
	}
	saver.End_Chunk();
	file.Close();
}

void EssentialsAuthenticationManager::Cleanup_Database() {
	while (SLNode<EssentialsAuthUser>* n = ProtectedUsers.Head()) {
		delete n->Data();
		ProtectedUsers.Remove_Head();
	}
}

EssentialsAuthUser* EssentialsAuthenticationManager::Add_Auth_User(const WideStringClass& PlayerName, const StringClass& Password, int AccessLevel) {
	if (!Is_Initialized()) {
		return NULL;
	}
	
	StringClass hash;
	if (!Get_MD5_Hash(Password, hash)) {
		return NULL;
	}

	EssentialsAuthUser* AuthUser = Get_Auth_User(PlayerName);

	if (AuthUser) {
		AuthUser->PasswordHash = hash;
		AuthUser->AccessLevel = AccessLevel;
	}
	else {
		AuthUser = new EssentialsAuthUser;
		AuthUser->Nick = PlayerName;
		AuthUser->PasswordHash = hash;
		AuthUser->AccessLevel = AccessLevel;
		ProtectedUsers.Add_Tail(AuthUser);
	}

	Save_Database();
	return AuthUser;
}

EssentialsAuthUser* EssentialsAuthenticationManager::Get_Auth_User(const WideStringClass& PlayerName) {
	if (!Is_Initialized()) {
		return NULL;
	}
	
	for (SLNode<EssentialsAuthUser>* n = ProtectedUsers.Head(); n; n = n->Next()) {
		if (n->Data()->Nick == PlayerName) {
			return n->Data();
		}
	}
	return nullptr;
}

bool EssentialsAuthenticationManager::Delete_Auth_User(const WideStringClass& PlayerName) {
	if (!Is_Initialized()) {
		return false;
	}
	
	if (EssentialsAuthUser* AuthUser = Get_Auth_User(PlayerName)) {
		ProtectedUsers.Remove(AuthUser);
		delete AuthUser;

		Save_Database();
		return true;
	}

	return false;
}

EssentialsAuthClient::EssentialsAuthClient(ConnectionRequest* Request) : Request(Request) {
	DialogState = 1; // Initialize with password prompt.

	Create_Dialog();
}

EssentialsAuthClient::~EssentialsAuthClient() {
	Destroy_Dialog();
}

void EssentialsAuthClient::Dialog_Message(DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control) {
	if (Dialog != this->Dialog) return;
	
	if (Type == MESSAGE_TYPE_CONTROL_MOUSE_CLICK) {
		if (Control == ProceedButton && (DialogState == 1 || DialogState == 2)) {
			switch (DialogState) {
				case 1: {
					PasswordAttempt = TextArea->Get_Text();
					AuthState = 3;

					StatusLabel->Set_Label_Text(L"Authenticating...");
					StatusLabel->Set_Text_Color(255, 255, 0);
					break;
				}
				case 2: {
					switch (EssentialsAuthenticationHandler::Instance->Do_Nick_Change(this, TextArea->Get_Text())) {
						case 1: {
							StatusLabel->Set_Label_Text(L"Please wait...");
							StatusLabel->Set_Text_Color(255, 255, 0);
							break;
						}
					}
					break;
				}
			}
		}
		else if (Control == BackButton && (DialogState == 1 || DialogState == 2)) {
			DialogState = 0;
			Destroy_Dialog();
			Create_Dialog();
		}
		else if (Control == PasswordShowHideButton && DialogState == 1) {
			TextArea->Set_Password_Field(!TextArea->Is_Password_Field());
			PasswordShowHideButton->Set_Button_Text(TextArea->Is_Password_Field() ? L"Show Password" : L"Hide Password");
		}
		else if (Control == PasswordButton && DialogState == 0) {
			DialogState = 1;
			Destroy_Dialog();
			Create_Dialog();
		}
		else if (Control == NickButton && DialogState == 0) {
			DialogState = 2;
			Destroy_Dialog();
			Create_Dialog();
		}
		else if (Control == LeaveButton && DialogState == 0) {
			AuthState = 4;
		}
	}
}

void EssentialsAuthClient::Create_Dialog() {
	int w, h;
	switch (DialogState) {
		case 0:
			w = 140;
			h = 153;
			break;
		default:
			w = 250;
			h = 90;
			break;
	}

	Dialog = Create_Centered_Popup(Request->clientId, w, h, L"Authentication Required");
	Create_Controls();
	Show_Dialog(Dialog);
}

void EssentialsAuthClient::Destroy_Dialog() {
	Delete_Dialog(Dialog);

	Dialog = 0;
	PasswordButton = 0;
	NickButton = 0;
	LeaveButton = 0;
	PasswordShowHideButton = 0;
	StatusLabel = 0;
	TextArea = 0;
	ProceedButton = 0;
	BackButton = 0;
}

void EssentialsAuthClient::Create_Controls() {
	if (DialogState == 0) {
		Create_Label_Control(Dialog, 10, 10, 120, 50, WideStringFormat(L"\"%ws\" is a protected name. You should authenticate, change your name or leave the server.", Request->clientName));

		PasswordButton = Create_Bordered_Button_Control(Dialog, 10, 70, 120, 21, L"Use Password");
		NickButton = Create_Bordered_Button_Control(Dialog, 10, 96, 120, 21, L"Change Name");
		LeaveButton = Create_Bordered_Button_Control(Dialog, 10, 122, 120, 21, L"Leave Server");
	}
	else if (DialogState == 1) {
		Create_Label_Control(Dialog, 10, 10, 230, 20, WideStringFormat(L"\"%ws\" is a protected name. Please enter the password below or go back for other options.", Request->clientName));
		
		TextArea = Create_TextArea_Control(Dialog, 10, 35, 230, 15);
		TextArea->Set_Password_Field(true);
		StatusLabel = Create_Label_Control(Dialog, 10, 52, 230, 8, L"", false, Vector3(1.f, 0.f, 0.f));

		BackButton = Create_Bordered_Button_Control(Dialog, 10, 62, 70, 18, L"Back");
		PasswordShowHideButton = Create_Bordered_Button_Control(Dialog, 90, 62, 70, 18, L"Show Password");
		ProceedButton = Create_Bordered_Button_Control(Dialog, 170, 62, 70, 18, L"Authenticate");
	}
	else if (DialogState == 2) {
		Create_Label_Control(Dialog, 10, 10, 230, 30, WideStringFormat(L"\"%ws\" is a protected name. Please enter a new name below or go back for other options.", Request->clientName));

		TextArea = Create_TextArea_Control(Dialog, 10, 35, 230, 15);
		StatusLabel = Create_Label_Control(Dialog, 10, 52, 230, 8, L"", false, Vector3(1.f, 0.f, 0.f));

		BackButton = Create_Bordered_Button_Control(Dialog, 10, 62, 70, 18, L"Back");
		ProceedButton = Create_Bordered_Button_Control(Dialog, 170, 62, 70, 18, L"Validate");
	}
}

void EssentialsAuthClient::Password_Validation_Callback(bool isSuccess, const WideStringClass& reason) const {
	if (DialogState == 1) {
		if (!isSuccess) {
			StatusLabel->Set_Label_Text(WideStringFormat(L"Identification failed: %s", reason));
			StatusLabel->Set_Text_Color(255, 0, 0);
		}
		else {
			StatusLabel->Set_Label_Text(reason);
			StatusLabel->Set_Text_Color(0, 255, 0);
		}
	}
}


void EssentialsRegisterCommand::Activate(const char* pArgs) {
	if (!EssentialsAuthenticationManager::Is_Initialized()) {
		return;
	}
	
	DATokenParserClass Token(pArgs, ' ');
	char *Name = Token.Get_String(), *Password = Token.Get_String();
	int AccessLevel;

	if (Name && Password && Token.Get_Int(AccessLevel)) {
		bool Exists = !!EssentialsAuthenticationManager::Get_Auth_User(Name);
		if (EssentialsAuthenticationManager::Add_Auth_User(Name, Password, AccessLevel)) {
			Console_Output("Player %s was successfully %s.\n", Name, Exists ? "updated" : "registered");
		}
		else {
			Console_Output("Player %s could not be registered.\n", Name);
		}
	}
	else {
		Console_Output("Invalid usage. Parameters: \"<Name> <Password> <Access Level>\"\n");
	}
}

Register_Console_Function(EssentialsRegisterCommand);