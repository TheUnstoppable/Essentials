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
#include "EssentialsJukebox.h"
#include "EssentialsUtils.h"

EssentialsJukeboxClass* EssentialsJukeboxClass::Instance = 0;

EssentialsJukeboxClass::EssentialsJukeboxClass() {
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED, INT_MAX);
	Register_Event(DAEvent::PLAYERJOIN, INT_MAX);
	Register_Event(DAEvent::PLAYERLEAVE, INT_MAX);
	Register_Event(DAEvent::PLAYERLOADED, INT_MAX);
	Register_Event(DAEvent::GAMEOVER, INT_MAX);

	Register_Chat_Command((DAECC)&EssentialsJukeboxClass::Jukebox_Command, "!jukebox|!jbox|!jb|!player|!music|!musicplayer");

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Jukebox feature.");
}

EssentialsJukeboxClass::~EssentialsJukeboxClass() {
	Instance = 0;

	Clear_Musics();

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Jukebox feature.");
}

void EssentialsJukeboxClass::Settings_Loaded_Event() {
	Reset_Playlists();
	Clear_Musics();

	EnableShuffle = DASettingsManager::Get_Bool("Essentials", "EnableShuffle", true);
	ShuffleInitially = DASettingsManager::Get_Bool("Essentials", "ShuffleInitially", true);
	DASettingsManager::Get_String(GameOverMusic, "Essentials", "GameOverMusic", "NULL");

	INISection* Section = DASettingsManager::Get_Section("EssentialsJukeboxMusics");
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			DATokenParserClass Token(Entry->Value, '|');
			float duration;
			if (!Token.Get_Float(duration)) {
				Console_Output("[Essentials] Could not read duration for music \"%s\", skipping...\n", Entry->Entry);
				continue;
			}

			EssentialsJukeboxMusic* Music = new EssentialsJukeboxMusic;
			Music->File = Entry->Entry;
			Music->Name = Token.Get_Remaining_String();
			Music->Duration = duration;
			Musics.Add_Tail(Music);
		}
	}

	if (!Musics.Get_Count()) {
		Console_Output("[Essentials] No musics were found! Disabling Jukebox feature...\n");
		delete this;
		return;
	}

	for(SLNode<cPlayer> *z = Get_Player_List()->Head(); z; z = z->Next()) {
		cPlayer* Player = z->Data();
		if (Player && Player->Get_Is_In_Game() && Player->Is_Active()) {
			Add_All_Musics(Player);
			Start_Timer(7290001, 1.f, false, Player->Get_Id());
		}
	}
}

void EssentialsJukeboxClass::Player_Join_Event(cPlayer* Player) {
	Start_Timer(7290002, 1.f, false, Player->Get_Id());
}

void EssentialsJukeboxClass::Player_Leave_Event(cPlayer* Player) {
	Start_Timer(7290002, 1.f, false, Player->Get_Id());
}

void EssentialsJukeboxClass::Player_Loaded_Event(cPlayer* Player) {
	if (!Is_Music_Playing(Player)) {
		Add_All_Musics(Player);
		Select_And_Play(Player);
	}
}

void EssentialsJukeboxClass::Game_Over_Event() {
	Reset_Playlists();
	if (GameOverMusic != "NULL") {
		Commands->Set_Background_Music(GameOverMusic);
	}
}

void EssentialsJukeboxClass::Timer_Expired(int Number, unsigned Data) {
	if (Number == 7290001) {
		if (cPlayer* Player = Find_Player(Data)) {
			Select_And_Play(Player);
		}
	}
	else if (Number == 7290002) {
		if (cPlayer* Player = Find_Player(Data)) {
			if (!Is_Music_Playing(Player)) {
				Add_All_Musics(Player);
				Select_And_Play(Player);
			}
		}
	}
}

bool EssentialsJukeboxClass::Jukebox_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType) {
	if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Player)) {
		if (Text.Size() == 0) {
			if (EssentialsJukeboxMusic* Music = Data->Get_CurrentMusic()) {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Now playing: %s (%s) - Use \"!jb help\" to control jukebox.", Music->Name, Format_Seconds((int)Music->Duration));
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] There is nothing playing at the moment.");
			}
		}
		else if (Text[1] == "help") {
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Help for Jukebox: ");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox: Shows the currently playing music.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox help: Shows commands to control Jukebox.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox stop: Stops jukebox for the session.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox start: Starts the jukebox if it was stopped.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox list: Lists all available musics.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox next: Skips to next song.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox queue: Lists the musics in your queue.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox clear: Removes all musics in your queue.");
			if (EnableShuffle) {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox shuffle: Shuffles the queue.");
			}
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox loop <off|all|single>: Toggles the loop mode for your queue.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox set <name>: Plays specified music.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox add <name>: Adds specified music in your queue.");
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] !jukebox remove <name>: Removes specified music from your queue.");
		}
		else if (Text[1] == "stop") {
			if (!Data->Get_IsStopped()) {
				Data->Set_IsStopped(true);
				Stop_Timer(7290001, Player->Get_Id());
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Jukebox is already stopped for you.");
			}
		}
		else if (Text[1] == "start") {
			if (Data->Get_IsStopped()) {
				Data->Set_IsStopped(false);
				Start_Timer(7290001, 1.f, false, Player->Get_Id());
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Jukebox is enabled for you.");
			}
		}
		else if (Text[1] == "list") {
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] All musics (%d in total): ", Musics.Get_Count());
			for (SLNode<EssentialsJukeboxMusic>* Node = Musics.Head(); Node; Node = Node->Next()) {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] %s (%s)", Node->Data()->Name, Format_Seconds((int)Node->Data()->Duration));
			}
		}
		else if (Text[1] == "next") {
			if (EssentialsJukeboxMusic* Music = Data->Get_CurrentMusic()) {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] \"%s\" has been skipped.", Music->Name);
				Select_And_Play(Player);
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] There is nothing playing at the moment.");
			}
		}
		else if (Text[1] == "queue") {
			DynamicVectorClass<EssentialsJukeboxMusic*>& queueMusics = Data->Get_Queue();
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Musics in your queue (%d in total): ", queueMusics.Count());
			for (int i = 0; i < queueMusics.Count(); ++i) {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] #%d: %s (%s)", i + 1, queueMusics[i]->Name, Format_Seconds((int)queueMusics[i]->Duration, false));
			}
		}
		else if (Text[1] == "clear") {
			Data->Clear_Queue();
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Your queue has been reset.");
		}
		else if (Text[1] == "shuffle") {
			if (EnableShuffle) {
				Data->Shuffle_List();
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Your queue has been shuffled.");
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Shuffling is not enabled.");
			}
		}
		else if (Text[1] == "loop") {
			if (Text.Size() >= 2) {
				if (Text[2] == "off") {
					Data->Set_LoopMode(0);
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Looping is now disabled.");
				}
				else if (Text[2] == "all") {
					Data->Set_LoopMode(1);
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Your queue is now looping.");
				}
				else if (Text[2] == "single") {
					Data->Set_LoopMode(2);
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Currently playing music is now looping.");
				}
				else {
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Invalid command usage, please use \"!jb help\" for the command help.");
				}
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Invalid command usage, please use \"!jb help\" for the command help.");
			}
		}
		else if (Text[1] == "set") {
			if (Text.Size() >= 2) {
				if (!Data->Get_IsStopped()) {
					EssentialsJukeboxMusic* music;
					int found = Find_Music(Text[2], music);
					if (found < 1) {
						DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Could not find any music with specified search term.");
					}
					else if (found > 1) {
						DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Too many musics found with specified search term, please be more specific.");
					}
					else {
						Data->Set_CurrentMusic(music);
						Stop_Timer(7290001, Player->Get_Id());
						Start_Timer(7290001, music->Duration, false, Player->Get_Id());
					}
				}
				else {
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Can't play when jukebox is disabled, please enable jukebox first using \"!jb start\".");
				}
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Invalid command usage, please use \"!jb help\" for the command help.");
			}
		}
		else if (Text[1] == "add") {
			if (Text.Size() >= 2) {
				EssentialsJukeboxMusic* music;
				int found = Find_Music(Text[2], music);
				if (found < 1) {
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Could not find any music with specified search term.");
				}
				else if (found > 1) {
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Too many musics found with specified search term, please be more specific.");
				}
				else {
					Data->Add_Music(music);
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] \"%s\" is successfully added to your queue.", music->Name);

					if (!Data->Get_IsStopped() && !Data->Get_CurrentMusic()) {
						Data->Set_CurrentMusic(music);
						Stop_Timer(7290001, Player->Get_Id());
						Start_Timer(7290001, music->Duration, false, Player->Get_Id());
					}
				}
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Invalid command usage, please use \"!jb help\" for the command help.");
			}
		}
		else if (Text[1] == "remove") {
			if (Text.Size() >= 2) {
				EssentialsJukeboxMusic* music;
				int found = Find_Music(Text[2], music);
				if (found < 1) {
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Could not find any music with specified search term.");
				}
				else if (found > 1) {
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Too many musics found with specified search term, please be more specific.");
				}
				else {
					Data->Remove_Music(music);
					DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] \"%s\" is successfully removed from your queue.", music->Name);
				}
			}
			else {
				DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Invalid command usage, please use \"!jb help\" for the command help.");
			}
		}
		else {
			DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Unknown command, please use \"!jb help\" for the command help.");
		}
	}
	else {
		DA::Private_Color_Message(Player, JUKEBOXCOLOR, "[Jukebox] Could not find Jukebox information for you.");
	}

	return false;
}

void EssentialsJukeboxClass::Clear_Musics() {
	for (SLNode<EssentialsJukeboxMusic>* Node = Musics.Head(); Node; Node = Node->Next()) {
		delete Node->Data();
	}

	Musics.Remove_All();
}

void EssentialsJukeboxClass::Reset_Playlists() {
	for(SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
		cPlayer* Player = z->Data();
		if (Player && Player->Get_Is_In_Game() && Player->Is_Active()) {
			if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Player)) {
				Data->Clear_Queue();
				Data->Set_CurrentMusic(0);
				Data->Set_LoopMode(0);
				Stop_Timer(7290001, Player->Get_Id());
			}
		}
	}
}

void EssentialsJukeboxClass::Add_All_Musics(cPlayer* Player) {
	if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Player)) {
		bool isStopped = Data->Get_IsStopped();
		Data->Set_IsStopped(false);
		for (SLNode<EssentialsJukeboxMusic>* Node = Musics.Head(); Node; Node = Node->Next()) {
			Data->Add_Music(Node->Data());
		}
		if (ShuffleInitially) {
			Data->Shuffle_List();
		}
		Data->Set_IsStopped(isStopped);
	}
}

void EssentialsJukeboxClass::Select_And_Play(cPlayer* Player) {
	if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Player)) {
		if (Data->Get_LoopMode() == 2) {
			if (EssentialsJukeboxMusic* Music = Data->Get_CurrentMusic()) {
				Data->Set_CurrentMusic(Music);
				Stop_Timer(7290001, Player->Get_Id());
				Start_Timer(7290001, Music->Duration, false, Player->Get_Id());
			}
		}
		else {
			if (EssentialsJukeboxMusic* Music = Data->Select_Next_Music()) {
				Data->Set_CurrentMusic(Music);
				Stop_Timer(7290001, Player->Get_Id());
				Start_Timer(7290001, Music->Duration, false, Player->Get_Id());
			}
			else {
				if (Data->Get_LoopMode() == 0) {
					Add_All_Musics(Player);
					Select_And_Play(Player);
				}
				else {
					Data->Set_CurrentMusic(0);
				}
			}
		}
	}
}

bool EssentialsJukeboxClass::Is_Music_Playing(cPlayer* Player) {
	if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Player)) {
		return !Data->Get_IsStopped() && Data->Get_CurrentMusic();
	}

	return false;
}

int EssentialsJukeboxClass::Find_Music(StringClass Name, EssentialsJukeboxMusic*& Music) {
	int found = 0;
	for (SLNode<EssentialsJukeboxMusic>* Node = Musics.Head(); Node; Node = Node->Next()) {
		if (stristr(Node->Data()->Name, Name)) {
			Music = Node->Data();
			found++;
		}
	}

	return found;
}
