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

class ESSENTIALS_API EssentialsCMSHandler : public DAEventClass {
	friend class EssentialsCMSView;

public:
	EssentialsCMSHandler();
	~EssentialsCMSHandler();

	static EssentialsCMSHandler* Get_Instance() { return Instance; }

	virtual void Settings_Loaded_Event();
	virtual bool Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID);
	virtual bool Key_Hook_Event(cPlayer* Player, const StringClass& Key);
	virtual void Player_Join_Event(cPlayer* Player);
	virtual void Dialog_Event(cPlayer* Player, DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control);

protected:
	void Add_View(EssentialsCMSView* v) { Views.Add(v); }
	void Remove_View(EssentialsCMSView* v) { Views.DeleteObj(v); }

private:
	static EssentialsCMSHandler* Instance;
	DynamicVectorClass<EssentialsCMSView*> Views;
};

class ESSENTIALS_API EssentialsCMSManager {
public:
	static void Init();
	static void Shutdown();
	static bool Is_Initialized();

	static EssentialsCMSDefinition* Find_CMS(const char* Name);
	static EssentialsCMSDefinition* Find_CMS_With_Trigger(EssentialsCMSTrigger::TriggerType type, const StringClass& data);
	static void Reload();
	static void Cleanup();

protected:
	static SList<EssentialsCMSDefinition> Definitions;
};