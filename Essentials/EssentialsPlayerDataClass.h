/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "da_player.h"
#include "EssentialsJukebox.h"

namespace EssentialsAuthenticationState {
	typedef enum {
		AUTHSTATE_NOTREGISTERED,
		AUTHSTATE_NOTAUTHED,
		AUTHSTATE_AUTHED
	} AuthenticationState;
}

class EssentialsPlayerDataClass : public DAPlayerDataClass {
public:
	void Init() override;
	void Clear_Level() override;
	void Clear_Session() override;

	// Basic Commands
	bool Get_IsFlying();
	bool Set_IsFlying(bool value);
	bool Get_IsSpectating();
	bool Set_IsSpectating(bool value);

	// Bad Word Filter
	int Get_BadWordCount() const;
	int Set_BadWordCount(int value);
	int Increment_BadWordCount();
	int Decrement_BadWordCount();

	// Jukebox
	int Get_MusicIndex();
	EssentialsJukeboxMusic* Get_CurrentMusic() const;
	EssentialsJukeboxMusic* Set_CurrentMusic(EssentialsJukeboxMusic* music);
	void Add_Music(EssentialsJukeboxMusic* music);
	void Remove_Music(EssentialsJukeboxMusic* music);
	EssentialsJukeboxMusic* Select_Next_Music();
	DynamicVectorClass<EssentialsJukeboxMusic*>& Get_Queue();
	void Clear_Queue();
	void Shuffle_List();
	int Get_LoopMode() const;
	int Set_LoopMode(int mode);
	bool Get_IsStopped() const;
	bool Set_IsStopped(bool stopped);

private:
	// Basic Commands
	bool IsFlying;
	bool IsSpectating;

	// Bad Word Filter
	int BadWordCount;

	// Jukebox
	EssentialsJukeboxMusic* CurrentMusic;
	DynamicVectorClass<EssentialsJukeboxMusic*> QueueList;
	bool IsStopped;
	int LoopMode; // 0 = off, 1 = all, 2 = single
};
