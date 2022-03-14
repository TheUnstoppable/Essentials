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
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsMapHolograms.h"
#include "EssentialsHolograms.h"

EssentialsMapHologramsClass* EssentialsMapHologramsClass::Instance = 0;

EssentialsMapHologramsClass::EssentialsMapHologramsClass() {
	Holograms = new DynamicVectorClass<StringClass>;
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Map Holograms feature.");
}

EssentialsMapHologramsClass::~EssentialsMapHologramsClass() {
	Instance = 0;

	Clear_Holograms();
	delete Holograms;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Map Holograms feature.");
}

void EssentialsMapHologramsClass::Settings_Loaded_Event() {
	Clear_Holograms();

	INISection* Section = DASettingsManager::Get_Section("EssentialsHolograms");
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			StringClass Name = StringFormat("EssHolo_%s", Entry->Entry);
			Vector3 Position;
			int Color;

			DATokenParserClass Token(Entry->Value, '|');
			if (!(Token.Get_Float(Position.X) && Token.Get_Float(Position.Y) && Token.Get_Float(Position.Z) && Token.Get_Int(Color))) {
				Console_Output("[Essentials] Could not parse hologram %s, skipping...\n", Name);
				continue;
			}

			Holograms->Add(Name);
			EssentialsHologramsManager::Create_Hologram(Name, Token.Get_Remaining_String(), (EssentialsHologramColor::HologramColor)Color, Position);
		}
	}
}

void EssentialsMapHologramsClass::Clear_Holograms() {
	for (int i = 0; i < Holograms->Count(); ++i) {
		EssentialsHologramsManager::Delete_Hologram((*Holograms)[i]);
	}
	Holograms->Delete_All();
}
