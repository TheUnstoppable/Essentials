/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once
#include "da_event.h"

struct EssentialsJukeboxMusic {
	StringClass Name;
	StringClass File;
	float Duration;
};

class EssentialsJukeboxClass : public DAEventClass {
	friend class EssentialsJukeboxDialogPlayerObserverClass;

public:
	static EssentialsJukeboxClass* Instance;

	EssentialsJukeboxClass();
	~EssentialsJukeboxClass();

	void Settings_Loaded_Event() override;
	void Player_Join_Event(cPlayer* Player) override;
	void Player_Leave_Event(cPlayer* Player) override;
	void Player_Loaded_Event(cPlayer* Player) override;
	void Game_Over_Event() override;
	void Timer_Expired(int Number, unsigned Data) override;

	bool Jukebox_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
	bool JukeboxDialog_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);

	void Jukebox_Start(cPlayer* Player);
	void Jukebox_Stop(cPlayer* Player);
	void Jukebox_Next(cPlayer* Player);
	void Jukebox_Shuffle(cPlayer* Player);


protected:
	void Clear_Musics();
	void Reset_Playlists();
	void Add_All_Musics(cPlayer* Player);
	void Select_And_Play(cPlayer* Player, bool NextRequested = false);
	bool Is_Music_Playing(cPlayer* Player);
	int Find_Music(StringClass Name, EssentialsJukeboxMusic*& Music);

private:
	bool EnableShuffle;
	bool ShuffleInitially;
	StringClass GameOverMusic;
	SList<EssentialsJukeboxMusic> Musics;
};

