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
// DA
#include "da.h"
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsAutoAnnounce.h"

EssentialsAutoAnnounceClass* EssentialsAutoAnnounceClass::Instance = 0;

EssentialsAutoAnnounceClass::EssentialsAutoAnnounceClass() {
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Auto Announce feature.");
}

EssentialsAutoAnnounceClass::~EssentialsAutoAnnounceClass() {
	Instance = 0;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Auto Announce feature.");
}

void EssentialsAutoAnnounceClass::Settings_Loaded_Event() {
	AnnounceTexts.Delete_All();
	AnnounceDelay = DASettingsManager::Get_Float("Essentials", "AnnounceDelay", 120.f);
	HostAnnounce = DASettingsManager::Get_Bool("Essentials", "HostAnnounce", true);
	
	StringClass announceColor;
	DASettingsManager::Get_String(announceColor, "Essentials", "AnnounceColor", "0,0,0");
	DATokenParserClass token(announceColor, ',');
	int r, g, b;
	if (token.Get_Int(r) && token.Get_Int(g) && token.Get_Int(b)) {
		AnnounceColor.X = (float)r;
		AnnounceColor.Y = (float)g;
		AnnounceColor.Z = (float)b;
	}

	INISection* Section = DASettingsManager::Get_Section("EssentialsAutoAnnounces");
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				AnnounceTexts.Add(Entry->Entry);
			}
		}
	}

	if (!AnnounceTexts.Count()) {
		Console_Output("[Essentials] No announcements were found! Disabling Auto Announce feature...\n");
		delete this;
		return;
	}

	AnnounceIndex = 0;
	Reset_Timer();
}

void EssentialsAutoAnnounceClass::Timer_Expired(int Number, unsigned int Data) {
	if (Number == 101) {
		if (Get_Player_Count() == 0) return;

		StringClass message = AnnounceTexts[AnnounceIndex++];
		message.Replace("{SERVERNAME}", The_Game() ? StringClass(The_Game()->Get_Game_Title()) : "");
		message.Replace("{DAVER}", DA::Get_Version());
		message.Replace("{TTVER}", StringFormat("%.2f", GetTTVersion()));
		message.Replace("{TTREV}", StringFormat("%u", GetTTRevision()));

		if (HostAnnounce) {
			DA::Host_Message(message);
		}
		else {
			DA::Color_Message((int)AnnounceColor.X, (int)AnnounceColor.Y, (int)AnnounceColor.Z, message);
		}

		if (AnnounceIndex >= AnnounceTexts.Count()) {
			AnnounceIndex = 0;
		}
	}
}

void EssentialsAutoAnnounceClass::Reset_Timer() {
	Stop_Timer(101);
	Start_Timer(101, AnnounceDelay, true);
}
