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
// Custom
#include "EssentialsCMSView.h"
#include "EssentialsCMSManager.h"
#include "EssentialsCMSUtils.h"
#include "cScTextObj.h"

EssentialsCMSView::EssentialsCMSView(EssentialsCMSDefinition* Definition) : Definition(Definition) {
	EssentialsCMSHandler::Get_Instance()->Add_View(this);
}

EssentialsCMSView::~EssentialsCMSView() {
	EssentialsCMSHandler::Get_Instance()->Remove_View(this);
}

bool EssentialsCMSDialogView::Perform(int playerId) {
	if (!Find_Player(playerId)) return false;
	if (DialogID) return false;

	if (!(Definition && Definition->As_DialogDefinition())) return false;
	EssentialsCMSDialogDefinition* Def = Definition->As_DialogDefinition();

	ScriptedDialogClass* Dialog = 0;
	switch (Def->Get_Dialog_Type()) {
		case DIALOGTYPE_MENU: {
			Dialog = Create_Menu_Dialog(playerId);
			break;
		}
		case DIALOGTYPE_POPUP: {
			ScriptedPopupDialogClass* Popup = Create_Popup_Dialog(playerId);
			Popup->Set_Orientation(Def->Get_Dialog_Orientation());
			Popup->Set_Dialog_Location(const_cast<Vector2&>(Def->Get_Dialog_Position()));
			Popup->Set_Dialog_Size(const_cast<Vector2&>(Def->Get_Dialog_Size()));
			Popup->Set_Dialog_Title(WideStringClass(Def->Get_Dialog_Title()));
			Dialog = Popup;
			break;
		}
	}

	if (!Dialog) return false;

	for (SLNode<EssentialsCMSContentDefinition>* n = Def->Get_Content().Head(); n; n = n->Next()) {
		EssentialsCMSContentDefinition* BaseContentDef = n->Data();
		if (EssentialsCMSDialogContentDefinition* ContentDef = BaseContentDef->As_DialogContentDefinition()) {
			ScriptedControlClass* Control = 0;
			switch (ContentDef->Get_Content_Type()) {
				case EssentialsCMSDialogContentDefinition::DialogContentType::DialogText: {
					ScriptedLabelControlClass* Label = Create_Label_Control(
						Dialog,
						(int)ContentDef->Get_Position().X,
						(int)ContentDef->Get_Position().Y,
						(int)ContentDef->Get_Size().X,
						(int)ContentDef->Get_Size().Y,
						WideStringClass(Process_Placeholders(ContentDef->Get_Data(), playerId)),
						TEXTSTYLE_BODY,
						ContentDef->Get_Color());
					Label->Set_Orientation(ContentDef->Get_Text_Orientation());
					Control = Label;
					break;
				}

				case EssentialsCMSDialogContentDefinition::DialogContentType::DialogTitle: {
					ScriptedLabelControlClass* Label = Create_Label_Control(
						Dialog,
						(int)ContentDef->Get_Position().X,
						(int)ContentDef->Get_Position().Y,
						(int)ContentDef->Get_Size().X,
						(int)ContentDef->Get_Size().Y,
						WideStringClass(Process_Placeholders(ContentDef->Get_Data(), playerId)),
						TEXTSTYLE_TITLE,
						ContentDef->Get_Color());
					Label->Set_Orientation(ContentDef->Get_Text_Orientation());
					Control = Label;
					break;
				}

				case EssentialsCMSDialogContentDefinition::DialogContentType::DialogImage: {
					ScriptedImageControlClass* Image = Create_Image_Control(
						Dialog,
						(int)ContentDef->Get_Position().X,
						(int)ContentDef->Get_Position().Y,
						(int)ContentDef->Get_Size().X,
						(int)ContentDef->Get_Size().Y,
						Process_Placeholders(ContentDef->Get_Data(), playerId));
					Control = Image;
					break;
				}

				case EssentialsCMSDialogContentDefinition::DialogContentType::CloseBorderedButton: {
					ScriptedButtonControlClass* Button = Create_Bordered_Button_Control(
						Dialog,
						(int)ContentDef->Get_Position().X,
						(int)ContentDef->Get_Position().Y,
						(int)ContentDef->Get_Size().X,
						(int)ContentDef->Get_Size().Y,
						WideStringClass(Process_Placeholders(ContentDef->Get_Data(), playerId)));
					Control = Button;
					break;
				}

				case EssentialsCMSDialogContentDefinition::DialogContentType::CloseTitleButton: {
					ScriptedButtonControlClass* Button = Create_Title_Button_Control(
						Dialog,
						(int)ContentDef->Get_Position().X,
						(int)ContentDef->Get_Position().Y,
						(int)ContentDef->Get_Size().X,
						(int)ContentDef->Get_Size().Y,
						WideStringClass(Process_Placeholders(ContentDef->Get_Data(), playerId)));
					Control = Button;
					break;
				}

				case EssentialsCMSDialogContentDefinition::DialogContentType::CloseImageButton: {
					ScriptedButtonControlClass* Button = Create_Image_Button_Control(
						Dialog,
						(int)ContentDef->Get_Position().X,
						(int)ContentDef->Get_Position().Y,
						(int)ContentDef->Get_Size().X,
						(int)ContentDef->Get_Size().Y,
						ContentDef->Get_Data(),
						Process_Placeholders(ContentDef->Get_Data(), playerId));
					Control = Button;
					break;
				}
			}

			if (!Control) {
				Delete_Dialog(Dialog);
				return false;
			}
		}
	}

	DialogID = Dialog->Get_Dialog_ID();
	Show_Dialog(Dialog);

	return true;
}

bool EssentialsCMSChatView::Perform(int playerId) {
	if (!Find_Player(playerId)) return false;

	if (!(Definition && Definition->As_ChatDefinition())) return false;
	EssentialsCMSChatDefinition* Def = Definition->As_ChatDefinition();

	for (SLNode<EssentialsCMSContentDefinition>* n = Def->Get_Content().Head(); n; n = n->Next()) {
		EssentialsCMSContentDefinition* BaseContentDef = n->Data();
		if (EssentialsCMSChatContentDefinition* ContentDef = BaseContentDef->As_ChatContentDefinition()) {
			switch (ContentDef->Get_Content_Type()) {
				case EssentialsCMSChatContentDefinition::ChatContentType::HostMessage: {
					cScTextObj* Text = Send_Client_Text(WideStringClass(Process_Placeholders(ContentDef->Get_Data(), playerId)), TEXT_MESSAGE_PUBLIC, false, -1, -1, false, false);
					Text->Set_Object_Dirty_Bits(playerId, NetworkObjectClass::BIT_CREATION);
					DA::Create_2D_Sound_Player(playerId, "public_message.wav");
					break;
				}

				case EssentialsCMSChatContentDefinition::ChatContentType::ColorMessage: {
					const Vector3& Color = ContentDef->Get_Color();
					DA::Private_Color_Message(playerId, (int)(Color.X * 255), (int)(Color.Y * 255), (int)(Color.Z * 255), Process_Placeholders(ContentDef->Get_Data(), playerId));
					break;
				}

				case EssentialsCMSChatContentDefinition::ChatContentType::TeamColorMessage: {
					DA::Private_Color_Message_With_Team_Color(playerId, Get_Team(playerId), Process_Placeholders(ContentDef->Get_Data(), playerId));
					break;
				}

				case EssentialsCMSChatContentDefinition::ChatContentType::HUDMessage: {
					const Vector3& Color = ContentDef->Get_Color();
					DA::Private_HUD_Message(playerId, (int)(Color.X * 255), (int)(Color.Y * 255), (int)(Color.Z * 255), Process_Placeholders(ContentDef->Get_Data(), playerId));
					break;
				}

				case EssentialsCMSChatContentDefinition::ChatContentType::AdminMessage: {
					DA::Private_Admin_Message(playerId, Process_Placeholders(ContentDef->Get_Data(), playerId));
					break;
				}

				case EssentialsCMSChatContentDefinition::ChatContentType::Sound: {
					DA::Create_2D_Sound_Player(playerId, Process_Placeholders(ContentDef->Get_Data(), playerId));
					break;
				}
			}
		}
	}

	return true;
}
