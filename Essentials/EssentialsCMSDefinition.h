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

class EssentialsCMSView;

class EssentialsCMSDialogContentDefinition;
class EssentialsCMSChatContentDefinition;
class ESSENTIALS_API EssentialsCMSContentDefinition {
public:
	typedef enum {
		CMS_DIALOG,
		CMS_CHAT
	} CMSContentDefinitionType;

	virtual ~EssentialsCMSContentDefinition() {}
	virtual CMSContentDefinitionType Get_Type() = 0;
	virtual EssentialsCMSDialogContentDefinition* As_DialogContentDefinition() { return NULL; }
	virtual EssentialsCMSChatContentDefinition* As_ChatContentDefinition() { return NULL; }
};
class ESSENTIALS_API EssentialsCMSDialogContentDefinition : public EssentialsCMSContentDefinition {
public:
	typedef enum {
		DialogText,
		DialogTitle,
		DialogImage,
		CloseBorderedButton,
		CloseTitleButton,
		CloseImageButton,
		Sound
	} DialogContentType;

	virtual CMSContentDefinitionType Get_Type() { return CMS_DIALOG; }
	virtual EssentialsCMSDialogContentDefinition* As_DialogContentDefinition() { return this; }

	DialogContentType Get_Content_Type() const { return ContentType; }
	TextOrientation Get_Text_Orientation() const { return TextOrientation; }
	const Vector3& Get_Color() const { return Color; }
	const StringClass& Get_Data() const { return Data; }
	const Vector2& Get_Position() const { return Position; }
	const Vector2& Get_Size() const { return Size; }

	void Set_Content_Type(DialogContentType type) { ContentType = type; }
	void Set_Text_Orientation(TextOrientation orientation) { TextOrientation = orientation; }
	void Set_Color(const Vector3& color) { Color = color; }
	void Set_Data(const char* data) { Data = data; }
	void Set_Position(const Vector2& position) { Position = position; }
	void Set_Size(const Vector2& size) { Size = size; }

protected:
	DialogContentType ContentType;
	Vector2 Position;
	Vector2 Size;
	TextOrientation TextOrientation;
	Vector3 Color;
	StringClass Data;
};
class ESSENTIALS_API EssentialsCMSChatContentDefinition : public EssentialsCMSContentDefinition {
public:
	typedef enum {
		HostMessage,
		ColorMessage,
		TeamColorMessage,
		HUDMessage,
		AdminMessage,
		Sound
	} ChatContentType;

	virtual CMSContentDefinitionType Get_Type() { return CMS_CHAT; }
	virtual EssentialsCMSChatContentDefinition* As_ChatContentDefinition() { return this; }

	ChatContentType Get_Content_Type() const { return ContentType; }
	const Vector3& Get_Color() const { return Color; }
	const StringClass& Get_Data() const { return Data; }

	void Set_Content_Type(ChatContentType type) { ContentType = type; }
	void Set_Color(const Vector3& color) { Color = color; }
	void Set_Data(const char* data) { Data = data; }

protected:
	ChatContentType ContentType;
	Vector3 Color;
	StringClass Data;
};

struct EssentialsCMSTrigger {
	typedef enum {
		CHATPHRASEMATCH,
		CHATPHRASEEXACT,
		KEYHOOK,
		PLAYERJOIN
	} TriggerType;

	TriggerType Type;
	StringClass Data;
};

class EssentialsCMSDialogDefinition;
class EssentialsCMSChatDefinition;
class ESSENTIALS_API EssentialsCMSDefinition {
public:
	typedef enum {
		CMS_NONE = 0,
		CMS_DIALOG = 1 << 0,
		CMS_CHAT = 1 << 1
	} CMSDefinitionType;

	virtual ~EssentialsCMSDefinition();
	virtual CMSDefinitionType Get_Type() = 0;
	virtual EssentialsCMSDialogDefinition* As_DialogDefinition() { return NULL; }
	virtual EssentialsCMSChatDefinition* As_ChatDefinition() { return NULL; }

	virtual EssentialsCMSView* Create_Instance() = 0;

	const StringClass& Get_Name() const { return Name; }
	const SList<EssentialsCMSContentDefinition>& Get_Content() const { return Content; }
	const SList<EssentialsCMSTrigger>& Get_Triggers() const { return Triggers; }

	void Set_Name(const char* name) { Name = name; }
	void Add_Content(EssentialsCMSContentDefinition* content) { Content.Add_Tail(content); }
	void Add_Trigger(EssentialsCMSTrigger* trigger) { Triggers.Add_Tail(trigger); }

protected:
	StringClass Name;
	SList<EssentialsCMSContentDefinition> Content;
	SList<EssentialsCMSTrigger> Triggers;
};
class ESSENTIALS_API EssentialsCMSDialogDefinition : public EssentialsCMSDefinition {
public:
	virtual CMSDefinitionType Get_Type() { return CMS_DIALOG; }
	virtual EssentialsCMSDialogDefinition* As_DialogDefinition() { return this; }

	virtual EssentialsCMSView* Create_Instance();
	DialogType Get_Dialog_Type() const { return DialogType; }
	DialogOrientation Get_Dialog_Orientation() const { return DialogOrientation; }
	const Vector2& Get_Dialog_Position() const { return Position; }
	const Vector2& Get_Dialog_Size() const { return Size; }
	const StringClass& Get_Dialog_Title() const { return Title; }

	void Set_Dialog_Type(DialogType type) { DialogType = type; }
	void Set_Dialog_Orientation(DialogOrientation orientation) { DialogOrientation = orientation; }
	void Set_Dialog_Position(const Vector2& position) { Position = position; }
	void Set_Dialog_Size(const Vector2& size) { Size = size; }
	void Set_Dialog_Title(const char* title) { Title = title; }

protected:
	DialogType DialogType;
	Vector2 Position;
	Vector2 Size;
	DialogOrientation DialogOrientation;
	StringClass Title;
};
class ESSENTIALS_API EssentialsCMSChatDefinition : public EssentialsCMSDefinition {
public:
	virtual CMSDefinitionType Get_Type() { return CMS_CHAT; }
	virtual EssentialsCMSChatDefinition* As_ChatDefinition() { return this; }

	virtual EssentialsCMSView* Create_Instance();
};