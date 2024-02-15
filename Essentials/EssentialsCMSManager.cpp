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
#include "da_settings.h"
// Custom
#include "EssentialsCMSManager.h"
#include "EssentialsCMSReader.h"
#include "EssentialsFileIterator.h"
#include "EssentialsUtils.h"

EssentialsCMSHandler* EssentialsCMSHandler::Instance = 0;
SList<EssentialsCMSDefinition> EssentialsCMSManager::Definitions;

EssentialsCMSHandler::EssentialsCMSHandler() {
	// If the assert below hits, then this singleton
	// class was attempted to be created twice.
	TT_RELEASE_ASSERT(!Instance);
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED);
	Register_Event(DAEvent::DIALOG);
	Register_Event(DAEvent::CHAT);
	Register_Event(DAEvent::KEYHOOK);
	Register_Event(DAEvent::PLAYERJOIN);
}

EssentialsCMSHandler::~EssentialsCMSHandler() {
	Instance = 0;
}

void EssentialsCMSHandler::Settings_Loaded_Event() {
	EssentialsCMSManager::Reload();
}

bool EssentialsCMSHandler::Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID) {
	if (EssentialsCMSManager::Is_Initialized()) {
		auto Def = EssentialsCMSManager::Find_CMS_With_Trigger(EssentialsCMSTrigger::CHATPHRASEEXACT, Message);
		if (Def) {
			EssentialsCMSView* View = Def->Create_Instance();
			if (!View->Perform(Player->Get_Id())) {
				Console_Output("[Essentials] Failed to perform CMS view \"%s\".\n", View->Get_Definition() ? View->Get_Definition()->Get_Name() : "**Missing Definition**");
			}
			return true;
		}
		Def = EssentialsCMSManager::Find_CMS_With_Trigger(EssentialsCMSTrigger::CHATPHRASEMATCH, Message);
		if (Def) {
			EssentialsCMSView* View = Def->Create_Instance();
			if (!View->Perform(Player->Get_Id())) {
				Console_Output("[Essentials] Failed to perform CMS view \"%s\".\n", View->Get_Definition() ? View->Get_Definition()->Get_Name() : "**Missing Definition**");
			}
			return true;
		}
	}
	return true;
}

bool EssentialsCMSHandler::Key_Hook_Event(cPlayer* Player, const StringClass& Key) {
	if (EssentialsCMSManager::Is_Initialized()) {
		auto Def = EssentialsCMSManager::Find_CMS_With_Trigger(EssentialsCMSTrigger::KEYHOOK, Key);
		if (Def) {
			EssentialsCMSView* View = Def->Create_Instance();
			if (!View->Perform(Player->Get_Id())) {
				Console_Output("[Essentials] Failed to perform CMS view \"%s\".\n", View->Get_Definition() ? View->Get_Definition()->Get_Name() : "**Missing Definition**");
			}
			return true;
		}
	}
	return true;
}

void EssentialsCMSHandler::Player_Join_Event(cPlayer* Player) {
	if (EssentialsCMSManager::Is_Initialized()) {
		auto Def = EssentialsCMSManager::Find_CMS_With_Trigger(EssentialsCMSTrigger::PLAYERJOIN, "");
		if (Def) {
			EssentialsCMSView* View = Def->Create_Instance();
			if (!View->Perform(Player->Get_Id())) {
				Console_Output("[Essentials] Failed to perform CMS view \"%s\".\n", View->Get_Definition() ? View->Get_Definition()->Get_Name() : "**Missing Definition**");
			}
		}
	}
}

void EssentialsCMSHandler::Dialog_Event(cPlayer* Player, DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control) {
	if (!Dialog) return;

	EssentialsCMSDialogView* View = 0;
	for(int i = 0; i < Views.Count(); ++i) {
		if (Views[i]->As_DialogContent() && ((EssentialsCMSDialogView*)Views[i])->Get_Dialog_ID() == Dialog->Get_Dialog_ID()) {
			View = Views[i]->As_DialogContent();
			break;
		}
	}

	if (!View) return;

	if (Type == MESSAGE_TYPE_DIALOG_ESCAPE && ((EssentialsCMSDialogDefinition*)View->Get_Definition())->Close_On_Escape()) {
		Delete_Dialog(Dialog);
		return;
	}
	if (Type == MESSAGE_TYPE_CONTROL_MOUSE_CLICK && Control->Get_Control_Type() == CONTROLTYPE_BUTTON) {
		Delete_Dialog(Dialog);
		return;
	}
}

void EssentialsCMSManager::Init() {
	new EssentialsCMSHandler;
}

void EssentialsCMSManager::Shutdown() {
	if (EssentialsCMSHandler::Get_Instance()) {
		delete EssentialsCMSHandler::Get_Instance();
	}
	Cleanup();
}

bool EssentialsCMSManager::Is_Initialized() {
	return !!EssentialsCMSHandler::Get_Instance();
}

EssentialsCMSDefinition* EssentialsCMSManager::Find_CMS(const char* Name) {
	for (SLNode<EssentialsCMSDefinition>* n = Definitions.Head(); n; n = n->Next()) {
		auto Def = n->Data();
		if (Def->Get_Name() == Name) {
			return Def;
		}
	}
	return 0;
}

EssentialsCMSDefinition* EssentialsCMSManager::Find_CMS_With_Trigger(EssentialsCMSTrigger::TriggerType type, const StringClass& data) {
	for (SLNode<EssentialsCMSDefinition>* n = Definitions.Head(); n; n = n->Next()) {
		auto Def = n->Data();
		for (SLNode<EssentialsCMSTrigger>* z = Def->Get_Triggers().Head(); z; z = z->Next()) {
			auto Trigger = z->Data();
			if (Trigger->Type == type) {
				if (Trigger->Type == EssentialsCMSTrigger::CHATPHRASEMATCH) {
					if (stristr(data, Trigger->Data)) {
						return Def;
					}
				} else {
					if (data == Trigger->Data) {
						return Def;
					}
				}
			}
		}
	}
	return 0;
}

void EssentialsCMSManager::Reload() {
	if (!Ensure_Essentials_Data_Folder_Exists("CMS")) {
		Console_Output("[Essentials] Failed to create CMS data folder.\n");
		return;
	}

	Cleanup();

	int loaded = 0;
	for(EssentialsFileIterator iter(L"EssentialsData\\CMS", L"*.cms"); iter; ++iter) {
		auto path = StringClass(iter.Get_Full_Path());
		auto filename = StringClass(iter.Get_File());
		EssentialsCMSReader reader(path);

		if (!reader.Next_Line()) {
			Console_Output("[Essentials] CMSParser:%s:%d:Error: Unexpected end of file.\n", filename, reader.Line_Index());
			continue;
		}

		StringClass token;
		if (!reader.Next_Token(token)) {
			Console_Output("[Essentials] CMSParser:%s:%d:Error: Could not read header - content type token missing.\n", filename, reader.Line_Index());
			continue;
		}

		EssentialsCMSDefinition *Def = 0;
		
		if (token == "CMSMenuDialog") {
			StringClass name, closeonesc;
			if (!reader.Next_Token(name)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSMenuDialog::name missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (!reader.Next_Token(closeonesc)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::closeonesc missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (closeonesc.Compare_No_Case("yes") && closeonesc.Compare_No_Case("no")) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::closeonesc invalid value. (\"Yes\" or \"No\" expected)\n", filename, reader.Line_Index());
				continue;
			}

			auto DialogDef = new EssentialsCMSDialogDefinition;
			DialogDef->Set_Dialog_Type(DIALOGTYPE_MENU);
			DialogDef->Set_Name(name);

			Def = DialogDef;
		} else if (token == "CMSPopupDialog") {
			Vector2 position, size;
			StringClass name, title, orientation, closeonesc;
			DialogOrientation cOrientation;
			if (!reader.Next_Token(name)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::name missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (!reader.Next_Token_As_Vector2(position)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::position missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (!reader.Next_Token_As_Vector2(size)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::size missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (!reader.Next_Token(orientation)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::orientation missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (orientation == "None") {
				cOrientation = DIALOGORIENTATION_NONE;
			} else if (orientation == "TopLeft") {
				cOrientation = DIALOGORIENTATION_TOPLEFT;
			} else if (orientation == "TopCenter") {
				cOrientation = DIALOGORIENTATION_TOPCENTER;
			} else if (orientation == "TopRight") {
				cOrientation = DIALOGORIENTATION_TOPRIGHT;
			} else if (orientation == "MiddleLeft") {
				cOrientation = DIALOGORIENTATION_MIDDLELEFT;
			} else if (orientation == "MiddleCenter") {
				cOrientation = DIALOGORIENTATION_MIDDLECENTER;
			} else if (orientation == "MiddleRight") {
				cOrientation = DIALOGORIENTATION_MIDDLERIGHT;
			} else if (orientation == "BottomLeft") {
				cOrientation = DIALOGORIENTATION_BOTTOMLEFT;
			} else if (orientation == "BottomCenter") {
				cOrientation = DIALOGORIENTATION_BOTTOMCENTER;
			} else if (orientation == "BottomRight") {
				cOrientation = DIALOGORIENTATION_BOTTOMRIGHT;
			} else {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::orientation invalid value.\n", filename, reader.Line_Index());
				continue;
			}
			if (!reader.Next_Token(closeonesc)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::closeonesc missing.\n", filename, reader.Line_Index());
				continue;
			}
			if (closeonesc.Compare_No_Case("yes") && closeonesc.Compare_No_Case("no")) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::closeonesc invalid value. (\"Yes\" or \"No\" expected)\n", filename, reader.Line_Index());
				continue;
			}
			if (!reader.Remaining_Text(title)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSPopupDialog::title missing.\n", filename, reader.Line_Index());
				continue;
			}

			auto DialogDef = new EssentialsCMSDialogDefinition;
			DialogDef->Set_Dialog_Type(DIALOGTYPE_POPUP);
			DialogDef->Set_Name(name);
			DialogDef->Set_Dialog_Position(position);
			DialogDef->Set_Dialog_Size(size);
			DialogDef->Set_Dialog_Orientation(cOrientation);
			DialogDef->Set_Close_On_Escape(!closeonesc.Compare_No_Case("Yes"));
			DialogDef->Set_Dialog_Title(title);

			Def = DialogDef;
		} else if (token == "CMSChat") {
			StringClass name;
			if (!reader.Next_Token(name)) {
				Console_Output("[Essentials] CMSParser:%s:%d:Error: CMSChat::name missing.\n", filename, reader.Line_Index());
				continue;
			}

			auto ChatDef = new EssentialsCMSChatDefinition;
			ChatDef->Set_Name(name);

			Def = ChatDef;
		} else {
			Console_Output("[Essentials] CMSParser:%s:%d:Error: Unknown CMS type header '%s'.\n", filename, reader.Line_Index(), token);
			continue;
		}

		if (Def) {
			while(reader.Next_Line()) {
				if (reader.Is_Line_Empty()) continue;

				if (!reader.Next_Token(token)) {
					Console_Output("[Essentials] CMSParser:%s:%d:Error: Content type header missing.\n", filename, reader.Line_Index());
				}

				EssentialsCMSContentDefinition* ContentDef = 0;

				if (token == "DialogText") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) != EssentialsCMSDefinition::CMS_DIALOG) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector2 position, size;
					Vector3 color;
					StringClass orientation, data;
					TextOrientation cOrientation;
					if (!reader.Next_Token_As_Vector2(position)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText::position missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Vector2(size)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText::size missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Color(color)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText::color missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token(orientation)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText::orientation missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (orientation == "Left") {
						cOrientation = TEXTORIENTATION_LEFT;
					} else if (orientation == "Center") {
						cOrientation = TEXTORIENTATION_CENTER;
					} else if (orientation == "Right") {
						cOrientation = TEXTORIENTATION_RIGHT;
					} else {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText::orientation invalid value.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogText::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto DialogTextContentDef = new EssentialsCMSDialogContentDefinition;
					DialogTextContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::DialogText);
					DialogTextContentDef->Set_Position(position);
					DialogTextContentDef->Set_Size(size);
					DialogTextContentDef->Set_Color(color);
					DialogTextContentDef->Set_Text_Orientation(cOrientation);
					DialogTextContentDef->Set_Data(data);

					ContentDef = DialogTextContentDef;
				} else if (token == "DialogTitle") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) != EssentialsCMSDefinition::CMS_DIALOG) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector2 position, size;
					Vector3 color;
					StringClass orientation, data;
					TextOrientation cOrientation;
					if (!reader.Next_Token_As_Vector2(position)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle::position missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Vector2(size)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle::size missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Color(color)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle::color missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token(orientation)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle::orientation missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (orientation == "Left") {
						cOrientation = TEXTORIENTATION_LEFT;
					} else if (orientation == "Center") {
						cOrientation = TEXTORIENTATION_CENTER;
					} else if (orientation == "Right") {
						cOrientation = TEXTORIENTATION_RIGHT;
					} else {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle::orientation invalid value.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogTitle::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto DialogTitleContentDef = new EssentialsCMSDialogContentDefinition;
					DialogTitleContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::DialogTitle);
					DialogTitleContentDef->Set_Position(position);
					DialogTitleContentDef->Set_Size(size);
					DialogTitleContentDef->Set_Color(color);
					DialogTitleContentDef->Set_Text_Orientation(cOrientation);
					DialogTitleContentDef->Set_Data(data);

					ContentDef = DialogTitleContentDef;
				} else if (token == "DialogImage") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) != EssentialsCMSDefinition::CMS_DIALOG) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogImage is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector2 position, size;
					StringClass data;
					if (!reader.Next_Token_As_Vector2(position)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogImage::position missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Vector2(size)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogImage::size missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: DialogImage::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto DialogImageContentDef = new EssentialsCMSDialogContentDefinition;
					DialogImageContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::DialogImage);
					DialogImageContentDef->Set_Position(position);
					DialogImageContentDef->Set_Size(size);
					DialogImageContentDef->Set_Data(data);

					ContentDef = DialogImageContentDef;
				} else if (token == "CloseBorderedButton") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) != EssentialsCMSDefinition::CMS_DIALOG) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseBorderedButton is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector2 position, size;
					StringClass data;
					if (!reader.Next_Token_As_Vector2(position)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseBorderedButton::position missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Vector2(size)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseBorderedButton::size missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseBorderedButton::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto CloseButtonContentDef = new EssentialsCMSDialogContentDefinition;
					CloseButtonContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::CloseBorderedButton);
					CloseButtonContentDef->Set_Position(position);
					CloseButtonContentDef->Set_Size(size);
					CloseButtonContentDef->Set_Data(data);

					ContentDef = CloseButtonContentDef;
				} else if (token == "CloseTitleButton") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) != EssentialsCMSDefinition::CMS_DIALOG) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseTitleButton is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector2 position, size;
					StringClass data;
					if (!reader.Next_Token_As_Vector2(position)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseTitleButton::position missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Vector2(size)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseTitleButton::size missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseTitleButton::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto CloseButtonContentDef = new EssentialsCMSDialogContentDefinition;
					CloseButtonContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::CloseTitleButton);
					CloseButtonContentDef->Set_Position(position);
					CloseButtonContentDef->Set_Size(size);
					CloseButtonContentDef->Set_Data(data);

					ContentDef = CloseButtonContentDef;
				} else if (token == "CloseImageButton") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) != EssentialsCMSDefinition::CMS_DIALOG) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseImageButton is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector2 position, size;
					StringClass data;
					if (!reader.Next_Token_As_Vector2(position)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseImageButton::position missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Next_Token_As_Vector2(size)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseImageButton::size missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: CloseImageButton::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto CloseButtonContentDef = new EssentialsCMSDialogContentDefinition;
					CloseButtonContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::CloseImageButton);
					CloseButtonContentDef->Set_Position(position);
					CloseButtonContentDef->Set_Size(size);
					CloseButtonContentDef->Set_Data(data);

					ContentDef = CloseButtonContentDef;
				} else if (token == "HostMessage") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_CHAT) != EssentialsCMSDefinition::CMS_CHAT) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: HostMessage is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					StringClass data;
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: HostMessage::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto ChatHostMessageContentDef = new EssentialsCMSChatContentDefinition;
					ChatHostMessageContentDef->Set_Content_Type(EssentialsCMSChatContentDefinition::HostMessage);
					ChatHostMessageContentDef->Set_Data(data);

					ContentDef = ChatHostMessageContentDef;
				} else if (token == "ColorMessage") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_CHAT) != EssentialsCMSDefinition::CMS_CHAT) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: ColorMessage is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector3 color;
					StringClass data;
					if (!reader.Next_Token_As_Color(color)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: ColorMessage::color missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: ColorMessage::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto ChatColorMessageContentDef = new EssentialsCMSChatContentDefinition;
					ChatColorMessageContentDef->Set_Content_Type(EssentialsCMSChatContentDefinition::ColorMessage);
					ChatColorMessageContentDef->Set_Color(color);
					ChatColorMessageContentDef->Set_Data(data);

					ContentDef = ChatColorMessageContentDef;
				} else if (token == "TeamColorMessage") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_CHAT) != EssentialsCMSDefinition::CMS_CHAT) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: TeamColorMessage is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					StringClass data;
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: TeamColorMessage::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto ChatTeamColorMessageContentDef = new EssentialsCMSChatContentDefinition;
					ChatTeamColorMessageContentDef->Set_Content_Type(EssentialsCMSChatContentDefinition::TeamColorMessage);
					ChatTeamColorMessageContentDef->Set_Data(data);

					ContentDef = ChatTeamColorMessageContentDef;
				} else if (token == "HUDMessage") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_CHAT) != EssentialsCMSDefinition::CMS_CHAT) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: HUDMessage is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					Vector3 color;
					StringClass data;
					if (!reader.Next_Token_As_Color(color)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: HUDMessage::color missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: HUDMessage::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto ChatHUDMessageContentDef = new EssentialsCMSChatContentDefinition;
					ChatHUDMessageContentDef->Set_Content_Type(EssentialsCMSChatContentDefinition::HUDMessage);
					ChatHUDMessageContentDef->Set_Color(color);
					ChatHUDMessageContentDef->Set_Data(data);

					ContentDef = ChatHUDMessageContentDef;
				} else if (token == "AdminMessage") {
					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_CHAT) != EssentialsCMSDefinition::CMS_CHAT) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: AdminMessage is not allowed in %s.\n", filename, reader.Line_Index(), token);
						continue;
					}

					StringClass data;
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: AdminMessage::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					auto ChatAdminMessageContentDef = new EssentialsCMSChatContentDefinition;
					ChatAdminMessageContentDef->Set_Content_Type(EssentialsCMSChatContentDefinition::AdminMessage);
					ChatAdminMessageContentDef->Set_Data(data);

					ContentDef = ChatAdminMessageContentDef;
				} else if (token == "Sound") {
					StringClass data;
					if (!reader.Remaining_Text(data)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: Sound::data missing.\n", filename, reader.Line_Index());
						continue;
					}

					if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_CHAT) == EssentialsCMSDefinition::CMS_CHAT) {
						auto ChatSoundContentDef = new EssentialsCMSChatContentDefinition;
						ChatSoundContentDef->Set_Content_Type(EssentialsCMSChatContentDefinition::Sound);
						ChatSoundContentDef->Set_Data(data);

						ContentDef = ChatSoundContentDef;
					}
					else if ((Def->Get_Type() & EssentialsCMSDefinition::CMS_DIALOG) == EssentialsCMSDefinition::CMS_DIALOG) {
						auto DialogSoundContentDef = new EssentialsCMSDialogContentDefinition;
						DialogSoundContentDef->Set_Content_Type(EssentialsCMSDialogContentDefinition::Sound);
						DialogSoundContentDef->Set_Data(data);

						ContentDef = DialogSoundContentDef;
					}
				} else if (token == "Trigger") {
					StringClass triggerType, data;
					EssentialsCMSTrigger::TriggerType cTriggerType;
					if (!reader.Next_Token(triggerType)) {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: Trigger::triggerType missing.\n", filename, reader.Line_Index());
						continue;
					}
					if (triggerType == "ChatPhraseMatch") {
						if (!reader.Remaining_Text(data)) {
							Console_Output("[Essentials] CMSParser:%s:%d:Error: Trigger::data missing.\n", filename, reader.Line_Index());
							continue;
						}
						cTriggerType = EssentialsCMSTrigger::CHATPHRASEMATCH;
					} else if (triggerType == "ChatPhraseExact") {
						if (!reader.Remaining_Text(data)) {
							Console_Output("[Essentials] CMSParser:%s:%d:Error: Trigger::data missing.\n", filename, reader.Line_Index());
							continue;
						}
						cTriggerType = EssentialsCMSTrigger::CHATPHRASEEXACT;
					} else if (triggerType == "KeyHook") {
						if (!reader.Remaining_Text(data)) {
							Console_Output("[Essentials] CMSParser:%s:%d:Error: Trigger::data missing.\n", filename, reader.Line_Index());
							continue;
						}
						cTriggerType = EssentialsCMSTrigger::KEYHOOK;
					} else if (triggerType == "PlayerJoin") {
						cTriggerType = EssentialsCMSTrigger::PLAYERJOIN;
					} else {
						Console_Output("[Essentials] CMSParser:%s:%d:Error: Trigger::triggerType invalid value.\n", filename, reader.Line_Index());
						continue;
					}

					auto trigger = new EssentialsCMSTrigger;
					trigger->Type = cTriggerType;
					trigger->Data = data;
					Def->Add_Trigger(trigger);
				}

				if (ContentDef) {
					Def->Add_Content(ContentDef);
				}
			}
			loaded++;
			Definitions.Add_Tail(Def);
		}
	}

	Console_Output("[Essentials] Loaded %d CMS definitions.\n", loaded);
}

void EssentialsCMSManager::Cleanup() {
	while (auto data = Definitions.Remove_Head()) {
		delete data;
	}
}
