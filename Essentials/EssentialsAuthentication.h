/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "EssentialsEventClass.h"

#define CREDENTIALS_DBNAME "credentials.dat"
#define CREDENTIALS_DBHEADER 0x42444345
#define CREDENTIALS_USERHEADER 0x32

class EssentialsAuthenticationManager;

struct EssentialsAuthUser {
	WideStringClass Nick;
	StringClass PasswordHash;
	int AccessLevel;
};

class EssentialsAuthClient {
	friend class EssentialsAuthenticationManager;
	friend class EssentialsAuthenticationHandler;

public:
	EssentialsAuthClient(ConnectionRequest* Request);
	~EssentialsAuthClient();

	void Dialog_Message(DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control);
	void Password_Validation_Callback(bool isSuccess, const WideStringClass& reason) const;

protected:
	void Create_Dialog();
	void Destroy_Dialog();
	void Create_Controls();

private:
	ConnectionRequest* Request;
	int AuthState; // 0 = not authenticated, 1 = authenticated, 2 = nick change pending, 3 = password pending, 4 = leave pending

	// AuthState == 2
	WideStringClass NewNick;

	// AuthState == 3
	StringClass PasswordAttempt;

	int DialogState; // 0 = options, 1 = password, 2 = nick
	ScriptedPopupDialogClass* Dialog;

	// State = 0
	ScriptedButtonControlClass* PasswordButton;
	ScriptedButtonControlClass* NickButton;
	ScriptedButtonControlClass* LeaveButton;

	// State = 1
	ScriptedButtonControlClass* PasswordShowHideButton;

	// State = 1,2
	ScriptedLabelControlClass* StatusLabel;
	ScriptedTextAreaControlClass* TextArea;
	ScriptedButtonControlClass* ProceedButton;
	ScriptedButtonControlClass* BackButton;

};

class EssentialsAuthenticationHandler : public DAEventClass {
	friend class EssentialsAuthenticationManager;
	friend class EssentialsAuthClient;

public:
	EssentialsAuthenticationHandler();
	~EssentialsAuthenticationHandler();
	void Settings_Loaded_Event() override;
	void Game_Over_Event() override;
	void Level_Loaded_Event() override;
	ConnectionAcceptanceFilter::STATUS Connection_Request_Event(ConnectionRequest& Request, WideStringClass& RefusalMessage) override;
	void Player_Join_Event(cPlayer* Player) override;

	static void Dialog_Event(int ClientID, int DialogID, int ControlID, DialogMessageType Message);

	class EssentialsAuthenticationFilter : public ConnectionAcceptanceFilter {
	public:
		void handleInitiation(const ConnectionRequest& connectionRequest) override {}
		void handleTermination(const ConnectionRequest& connectionRequest) override;
		void handleCancellation(const ConnectionRequest& connectionRequest) override {}
		STATUS getStatus(const ConnectionRequest& connectionRequest, WideStringClass& refusalMessage) override;
	};

protected:
	bool Is_Authenticating(ConnectionRequest& Request);
	EssentialsAuthClient* Get_Auth_Context(int Client);
	EssentialsAuthClient* Get_Auth_Context(ConnectionRequest& Request);
	void Do_Authentication(ConnectionRequest& Request);
	bool Requires_Authentication(ConnectionRequest& Request);
	void Cleanup_Context(EssentialsAuthClient* AuthContext);

	static int Do_Nick_Change(EssentialsAuthClient* AuthContext, WideStringClass NewNick);
	static int Do_Password_Validation(EssentialsAuthClient* AuthContext);
	static int Do_Leave(EssentialsAuthClient* AuthContext);

private:
	static EssentialsAuthenticationHandler* Instance;
	int DialogHookID;
	StringClass AuthMessage;
	bool AllowRegisteredOnly;
	DynamicVectorClass<EssentialsAuthClient*> AuthContexts;
	DynamicVectorClass<int> AuthenticatedIDs;
};

class ESSENTIALS_API EssentialsAuthenticationManager {
public:
	static void Init();
	static void Shutdown();
	static bool Is_Initialized();
	static void Reload_Database();
	static void Save_Database();
	static void Cleanup_Database();
	static bool Force_Authenticate_User(int Client);

	static EssentialsAuthUser* Add_Auth_User(const WideStringClass& PlayerName, const StringClass& Password, int AccessLevel = 0);;
	static EssentialsAuthUser* Get_Auth_User(const WideStringClass& PlayerName);
	static bool Delete_Auth_User(const WideStringClass& PlayerName);

private:
	static SList<EssentialsAuthUser> ProtectedUsers;
};



class EssentialsRegisterCommand : public ConsoleFunctionClass {
	const char* Get_Name(void) override { return "essentialsregister"; }
	const char* Get_Alias(void) override { return "essregister"; }
	const char* Get_Help(void) override { return "Registers or updates password given name with given password and access level."; }
	void Activate(const char* pArgs) override;
};

class EssentialsForceAuthCommand : public ConsoleFunctionClass {
	const char* Get_Name(void) override { return "essentialsforceauth"; }
	const char* Get_Alias(void) override { return "essfauth"; }
	const char* Get_Help(void) override { return "Forcefully authenticates given player ID without password."; }
	void Activate(const char* pArgs) override;
};

class EssentialsDeleteRegisterCommand : public ConsoleFunctionClass {
	const char* Get_Name(void) override { return "essentialsdeleteregister"; }
	const char* Get_Alias(void) override { return "essdelregister"; }
	const char* Get_Help(void) override { return "Registers or updates password given name with given password and access level."; }
	void Activate(const char* pArgs) override;
};