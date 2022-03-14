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
#include "engine_weap.h"
#include "engine_player.h"
// DA
#include "da.h"
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsPlayerDataClass.h"
#include "EssentialsEventClass.h"
#include "EssentialsBadWordFilter.h"
#include "EssentialsUtils.h"

EssentialsBadWordFilterClass* EssentialsBadWordFilterClass::Instance = 0;

EssentialsBadWordFilterClass::EssentialsBadWordFilterClass() {
	BadWords = new DynamicVectorClass<StringClass>;
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED, INT_MAX);
	Register_Event(DAEvent::CHAT, INT_MAX);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Bad Word Filter feature.");
}

EssentialsBadWordFilterClass::~EssentialsBadWordFilterClass() {
	Instance = 0;

	BadWords->Clear();
	delete BadWords;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Bad Word Filter feature.");
}

void EssentialsBadWordFilterClass::Settings_Loaded_Event() {
	MaxWarns = DASettingsManager::Get_Int("Essentials", "BadWordFilterKickAfter", 0);
	ReplaceSpaces = DASettingsManager::Get_Bool("Essentials", "BadWordFilterRemoveSpaces", false);
	DASettingsManager::Get_String(WarnMessage, "Essentials", "BadWordFilterWarnMessage", "\"{WORD}\" is a disallowed word. If you keep saying disallowed words {REMAINING} more time(s), you will be kicked.");
	DASettingsManager::Get_String(KickMessage, "Essentials", "BadWordFilterKickReason", "You have been kicked from server for saying disallowed words for {COUNT} time(s).");

	INISection* Section = DASettingsManager::Get_Section("EssentialsBadWords");
	BadWords->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				BadWords->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d bad words.\n", BadWords->Count());
	}

	if (!BadWords->Count()) {
		Console_Output("[Essentials] No bad words were found! Disabling Bad Word Filter feature...\n");
		delete this;
		return;
	}
}

bool EssentialsBadWordFilterClass::Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID) {
	StringClass String = StringClass(Message);
	if (ReplaceSpaces) {
		String.Replace(" ", "", false);
	}

	for (int i = 0; i < BadWords->Count(); i++) {
		StringClass Word = (*BadWords)[i];
		if (stristr(String.Peek_Buffer(), Word.Peek_Buffer())) {
			EssentialsPlayerDataClass *Data = EssentialsEventClass::Instance->Get_Player_Data(Player);
			Data->Increment_BadWordCount();
			if (Data->Get_BadWordCount() >= MaxWarns) {
				DALogManager::Write_Log("_ESSENTIALS", "%ws has been kicked for saying bad words %d time(s). Blocked message is \"%ws\", detected word is \"%s\".", Player->Get_Name().Peek_Buffer(), Data->Get_BadWordCount(), Message, Word.Peek_Buffer());

				StringClass KickReply = KickMessage;
				KickReply.Replace("{COUNT}", StringFormat("%d", Data->Get_BadWordCount()).Peek_Buffer());
				KickReply.Replace("{WORD}", Word.Peek_Buffer());
				KickReply.Replace("{NAME}", StringClass(Player->Get_Name().Peek_Buffer()));

				Data->Set_BadWordCount(0);
				Evict_Client(Player->Get_Id(), WideStringClass(KickReply.Peek_Buffer()));
			} else {
				StringClass Count = StringFormat("%d", Data->Get_BadWordCount());
				StringClass Remaining = StringFormat("%d", MaxWarns - Data->Get_BadWordCount());

				DALogManager::Write_Log("_ESSENTIALS", "%ws has been warned for saying bad words %d/%d time(s). Blocked message is \"%ws\", detected word is \"%s\".", Player->Get_Name().Peek_Buffer(), Data->Get_BadWordCount(), MaxWarns, Message, Word.Peek_Buffer());

				StringClass WarnReply = WarnMessage;
				
				WarnReply.Replace("{COUNT}", Count.Peek_Buffer());
				WarnReply.Replace("{REMAINING}", Remaining.Peek_Buffer());
				WarnReply.Replace("{WORD}", Word.Peek_Buffer());
				WarnReply.Replace("{NAME}", StringClass(Player->Get_Name().Peek_Buffer()));
				DA::Page_Player(Player, WarnReply);
			}
			return false;
		}
	}

	return true;
}