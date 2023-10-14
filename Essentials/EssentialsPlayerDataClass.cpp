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
#include "engine_common.h"
#include "engine_obj.h"
// DA
#include "da.h"
#include "da_settings.h"
// Custom
#include "EssentialsPlayerDataClass.h"
#include "EssentialsEventClass.h"
#include "EssentialsUtils.h"

void EssentialsPlayerDataClass::Init() {
	IsFlying = false;
	IsSpectating = false;
	BadWordCount = 0;
	CurrentMusic = 0;
	QueueList.Delete_All();
	IsStopped = false;
	LoopMode = 0;
}

void EssentialsPlayerDataClass::Clear_Level() {
	IsFlying = false;
	IsSpectating = false;
	CurrentMusic = 0;
}

void EssentialsPlayerDataClass::Clear_Session() {
	IsFlying = false;
	IsSpectating = false;
	BadWordCount = 0;
	CurrentMusic = 0;
	QueueList.Delete_All();
	IsStopped = false;
	LoopMode = 0;
}

bool EssentialsPlayerDataClass::Get_IsFlying() {
	IsFlying = Get_Owner()->Get_GameObj()->Get_Fly_Mode();
	return IsFlying;
}

bool EssentialsPlayerDataClass::Set_IsFlying(bool value) {
	if (!Get_Owner()->Get_GameObj()->Get_Vehicle()) {
		if (Get_IsFlying()) {
			if (!value) {
				DA::Page_Player(Get_Owner(), "You are no longer flying.");
				Get_Owner()->Get_GameObj()->Toggle_Fly_Mode();
			}
		}
		else {
			if (value) {
				DA::Page_Player(Get_Owner(), "You are now flying.");
				Get_Owner()->Get_GameObj()->Toggle_Fly_Mode();
			}
		}
		IsFlying = value;
		return true;
	}

	return false;
}

bool EssentialsPlayerDataClass::Get_IsSpectating() {
	IsSpectating = Get_Object_Type(Get_Owner()->Get_GameObj()) == -4;
	return IsSpectating;
}

bool EssentialsPlayerDataClass::Set_IsSpectating(bool value) {
	if (!Get_Owner()->Get_GameObj()->Get_Vehicle()) {
		IsSpectating = value;
		if (!IsSpectating) {
			DA::Page_Player(Get_Owner(), "You are no longer spectating.");
			Commands->Destroy_Object(Get_Owner()->Get_GameObj());
		}
		else {
			IsSpectating = Make_Spectator(Get_Owner());
			if (IsSpectating) {
				DA::Page_Player(Get_Owner(), "You are now spectating.");
			}
		}
		return true;
	}

	return false;
}

int EssentialsPlayerDataClass::Get_BadWordCount() const {
	return BadWordCount;
}

int EssentialsPlayerDataClass::Set_BadWordCount(int value) {
	BadWordCount = value;
	return BadWordCount;
}

int EssentialsPlayerDataClass::Increment_BadWordCount() {
	return ++BadWordCount;
}

int EssentialsPlayerDataClass::Decrement_BadWordCount() {
	return --BadWordCount;
}

int EssentialsPlayerDataClass::Get_MusicIndex() {
	return QueueList.ID(CurrentMusic);
}

EssentialsJukeboxMusic* EssentialsPlayerDataClass::Get_CurrentMusic() const {
	return CurrentMusic;
}

EssentialsJukeboxMusic* EssentialsPlayerDataClass::Set_CurrentMusic(EssentialsJukeboxMusic* music) {
	if (!IsStopped) {
		CurrentMusic = music;
		if (CurrentMusic) {
			DA::Private_Color_Message(Get_Owner(), JUKEBOXCOLOR, "[Jukebox] Now playing: %s (%s) - Use \"!jb help\" to control jukebox.", CurrentMusic->Name, Format_Seconds((int)CurrentMusic->Duration));
			Set_Background_Music_Player(Get_Owner()->Get_GameObj(), CurrentMusic->File);
		}
		else {
			Stop_Background_Music_Player(Get_Owner()->Get_GameObj());
		}
	}
	return CurrentMusic;
}

void EssentialsPlayerDataClass::Add_Music(EssentialsJukeboxMusic* music) {
	QueueList.Add(music);
}

void EssentialsPlayerDataClass::Remove_Music(EssentialsJukeboxMusic* music) {
	while (QueueList.DeleteObj(music)) { }
}

EssentialsJukeboxMusic* EssentialsPlayerDataClass::Select_Next_Music() {
	if (QueueList.Count() == 0)
		return 0;

	EssentialsJukeboxMusic* Music = QueueList[Get_MusicIndex() + 1 > QueueList.Count() - 1 ? 0 : Get_MusicIndex() + 1];
	if (LoopMode == 0)
		QueueList.DeleteObj(Music);

	return Music;
}

DynamicVectorClass<EssentialsJukeboxMusic*>& EssentialsPlayerDataClass::Get_Queue() {
	return QueueList;
}

void EssentialsPlayerDataClass::Clear_Queue() {
	QueueList.Delete_All();
}

void EssentialsPlayerDataClass::Shuffle_List() {
	for(int i = 0; i < QueueList.Count(); ++i) {
		int Random = Get_Random_Int(0, QueueList.Count());
		EssentialsJukeboxMusic* Music = QueueList[Random];
		QueueList[Random] = QueueList[i];
		QueueList[i] = Music;
	}
}

int EssentialsPlayerDataClass::Get_LoopMode() const {
	return LoopMode;
}

int EssentialsPlayerDataClass::Set_LoopMode(int mode) {
	LoopMode = mode;
	return LoopMode;
}

bool EssentialsPlayerDataClass::Get_IsStopped() const {
	return IsStopped;
}

bool EssentialsPlayerDataClass::Set_IsStopped(bool stopped) {
	IsStopped = stopped;

	if (IsStopped && CurrentMusic) {
		DA::Private_Color_Message(Get_Owner(), JUKEBOXCOLOR, "[Jukebox] Jukebox is stopped for you in this session.");
		Stop_Background_Music_Player(Get_Owner()->Get_GameObj());
		CurrentMusic = 0;
	}

	return IsStopped;
}
