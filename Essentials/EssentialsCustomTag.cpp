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
#include "engine_player.h"
// DA
#include "da.h"
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsEventClass.h"
#include "EssentialsCustomTag.h"
#include "EssentialsUtils.h"

EssentialsCustomTagClass* EssentialsCustomTagClass::Instance = 0;

EssentialsCustomTagClass::EssentialsCustomTagClass() {
	Instance = this;
	SettingsDirty = false;

	Register_Event(DAEvent::GAMEOVER);
	Register_Event(DAEvent::SETTINGSLOADED);
	Register_Event(DAEvent::PLAYERJOIN);

	Register_Chat_Command((DAECC)&EssentialsCustomTagClass::CustomTag_Command, "!tag|!settag|!mytag");
	Register_Chat_Command((DAECC)&EssentialsCustomTagClass::RemoveCustomTag_Command, "!deltag|!removetag|!tagdel");

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Custom Tag feature.");
}

EssentialsCustomTagClass::~EssentialsCustomTagClass() {
	Instance = 0;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Custom Tag feature.");
}

void EssentialsCustomTagClass::Game_Over_Event() {
	Settings_Loaded_Event();
}

void EssentialsCustomTagClass::Settings_Loaded_Event() {
	if (SettingsDirty) {
		Save_Tags();
		Console_Output("[Essentials] Changes in the custom tag database has been saved successfully.\n");
	}
	else {
		Load_Tags();
		Console_Output("[Essentials] Custom tag database has been loaded successfully.\n");
	}
}

void EssentialsCustomTagClass::Player_Join_Event(cPlayer* Player) {
	if (EssentialsCustomTag* Tag = Find_Tag(Player->Get_Name())) {
		Apply_Tag(Player->Get_Name());
	}
}

bool EssentialsCustomTagClass::CustomTag_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType) {
	if (Text.Size() > 0) {
		const_cast<StringClass&>(Text.Get_Delimiter()) = " ";
		StringClass tag(Text(0));
		Add_Tag(Player->Get_Name(), WideStringClass(tag));
		DA::Page_Player(Player, "Your tag has been changed.");
	}
	else {
		DA::Page_Player(Player, "Usage: !tag <Your tag>");
	}
	return true;
}

bool EssentialsCustomTagClass::RemoveCustomTag_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType) {
	if (EssentialsCustomTag* Tag = Find_Tag(Player->Get_Name())) {
		Remove_Tag(Player->Get_Name());
		DA::Page_Player(Player, "Your tag has been removed.");
	}
	else {
		DA::Page_Player(Player, "Could not remove your tag.");
	}
	return true;
}


void EssentialsCustomTagClass::Add_Tag(const WideStringClass& Name, const WideStringClass& Tag) {
	if (EssentialsCustomTag* obj = Find_Tag(Name)) {
		obj->CustomTag = Tag;
	}
	else {
		obj = new EssentialsCustomTag;
		obj->PlayerName = Name;
		obj->CustomTag = Tag;
		CustomTags.Add_Tail(obj);
	}
	Apply_Tag(Name);
	SettingsDirty = true;
}

EssentialsCustomTag* EssentialsCustomTagClass::Find_Tag(const WideStringClass& Name) {
	for (SLNode<EssentialsCustomTag>* n = CustomTags.Head(); n; n = n->Next()) {
		if (n->Data()->PlayerName == Name) {
			return n->Data();
		}
	}
	return 0;
}

bool EssentialsCustomTagClass::Remove_Tag(const WideStringClass& Name) {
	if (EssentialsCustomTag* Tag = Find_Tag(Name)) {
		CustomTags.Remove(Tag);
		Apply_Tag(Name);
		delete Tag;

		SettingsDirty = true;
		return true;
	}
	return false;
}

void EssentialsCustomTagClass::Apply_Tag(const WideStringClass& Name) {
	if (cPlayer* Player = Find_Player(Name)) {
		Player->Get_DA_Player()->Remove_Tags_With_Flag(DAPlayerFlags::LEGACYTAG);
		if (EssentialsCustomTag* Tag = Find_Tag(Player->Get_Name())) {
			Player->Get_DA_Player()->Add_Tag(StringClass(Tag->CustomTag), 0, DAPlayerFlags::LEGACYTAG | DAPlayerFlags::PERSISTMAP | DAPlayerFlags::PERSISTLEAVE);
		}
	}
}

void EssentialsCustomTagClass::Clear_Tags() {
	while (EssentialsCustomTag* Tag = CustomTags.Remove_Head()) {
		delete Tag;
	}
}

void EssentialsCustomTagClass::Load_Tags() {
	Clear_Tags();

	FileClass* file = Create_Or_Get_Essentials_Data_File(CUSTOMTAG_DBNAME);
	if (!file) {
		Console_Output("[Essentials] Failed to open custom tag database for reading.\n");
		return;
	}

	ChunkLoadClass cload(file);
	while (cload.Open_Chunk()) {
		if (cload.Cur_Chunk_ID() == CUSTOMTAG_DATAHEADER) {
			char version;
			cload.SimpleRead(version);

			if (version != CUSTOMTAG_DBVERSION) {
				Console_Output("[Essentials] The version of custom tag database is %s than the current database version.\n", version > CUSTOMTAG_DBVERSION ? "newer" : "older");
			}
			else {
				while (cload.Open_Micro_Chunk()) {
					if (cload.Cur_Micro_Chunk_ID() == CUSTOMTAG_DATAENTRY) {
						EssentialsCustomTag* Tag = new EssentialsCustomTag;

						int len;
						wchar_t* buf;

						cload.SimpleRead(len);
						buf = new wchar_t[len + 1];
						cload.Read(buf, len * 2);
						buf[len] = 0;
						Tag->PlayerName = buf;
						delete[] buf;

						cload.SimpleRead(len);
						buf = new wchar_t[len + 1];
						cload.Read(buf, len * 2);
						buf[len] = 0;
						Tag->CustomTag = buf;
						delete[] buf;

						CustomTags.Add_Tail(Tag);
					}
					cload.Close_Micro_Chunk();
				}
			}
		}
		cload.Close_Chunk();
	}
	delete file;

	SettingsDirty = false;
}

void EssentialsCustomTagClass::Save_Tags() {
	FileClass* file = Create_Or_Get_Essentials_Data_File(CUSTOMTAG_DBNAME, 2);
	if (!file) {
		Console_Output("[Essentials] Failed to open custom tag database for reading.\n");
		return;
	}

	ChunkSaveClass csave(file);
	csave.Begin_Chunk(CUSTOMTAG_DATAHEADER);
	csave.SimpleWrite(CUSTOMTAG_DBVERSION);
	for(SLNode<EssentialsCustomTag>* n = CustomTags.Head(); n; n = n->Next()) {
		csave.Begin_Micro_Chunk(CUSTOMTAG_DATAENTRY);

		csave.SimpleWrite(n->Data()->PlayerName.Get_Length());
		csave.Write(n->Data()->PlayerName.Peek_Buffer(), n->Data()->PlayerName.Get_Length() * 2);

		csave.SimpleWrite(n->Data()->CustomTag.Get_Length());
		csave.Write(n->Data()->CustomTag.Peek_Buffer(), n->Data()->CustomTag.Get_Length() * 2);

		csave.End_Micro_Chunk();
	}
	csave.End_Chunk();
	delete file;

	SettingsDirty = false;
}
