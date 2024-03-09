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
#include "EssentialsCMSDefinition.h"
#include "EssentialsCMSView.h"

#define PERFORM_VIEW(Player, Def, View, AutoDelete) \
	EssentialsCMSView* View = (Def)->Create_Instance(); \
	if (!View->Perform((Player)->Get_Id())) { \
		Console_Output("[Essentials] Failed to perform CMS view \"%s\".\n", View->Get_Definition() ? View->Get_Definition()->Get_Name().Peek_Buffer() : "**Missing Definition**"); \
		delete View; \
	} else { \
		if (!!(AutoDelete) || View->Auto_Delete_After_Perform()) delete View; \
	}

class ESSENTIALS_API EssentialsCMSHandler : public DAEventClass {
	//friend class EssentialsCMSView;

public:
	EssentialsCMSHandler();
	~EssentialsCMSHandler();

	static EssentialsCMSHandler* Get_Instance() { return Instance; }

	virtual void Settings_Loaded_Event();
	virtual bool Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID);
	virtual bool Key_Hook_Event(cPlayer* Player, const StringClass& Key);
	virtual void Player_Join_Event(cPlayer* Player);
	virtual void Dialog_Event(cPlayer* Player, DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control);

private:
	static EssentialsCMSHandler* Instance;
};

class ESSENTIALS_API EssentialsCMSManager {
	friend class EssentialsCMSView;

public:
	static void Init();
	static void Shutdown();
	static bool Is_Initialized();

	static EssentialsCMSDefinition* Find_CMS(const char* Name);
	static EssentialsCMSDefinition* Find_CMS_With_Trigger(EssentialsCMSTrigger::TriggerType type, const StringClass& data);
	static void Reload();
	static void Cleanup();

	static void Add_View(EssentialsCMSView* v) { Views.Add_Head(v); }
	static const SList<EssentialsCMSView>& Get_Views() { return Views; }
	static void Remove_View(EssentialsCMSView* v) { Views.Remove(v); }

private:
	static SList<EssentialsCMSDefinition> Definitions;
	static SList<EssentialsCMSView> Views;
};