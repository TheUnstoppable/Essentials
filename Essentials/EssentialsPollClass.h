/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "engine_string.h"
#include "engine_vector.h"

typedef enum {
	VOTE_NONE = -1,
	VOTE_TIME,
	VOTE_GAMEOVER,
	VOTE_RESTARTMAP,
	VOTE_NEXTMAP,
	VOTE_PLAYERKICK,
	VOTE_NOREPAIR,
} EssentialsVotingType;

class EssentialsPollClass {
	friend class EssentialsVotingManagerClass;

public:
	EssentialsPollClass() : StarterID(-1), Forced(false), Type(VOTE_NONE), StartTime(0) {

	}

	int Get_Starter_ID() const { return StarterID; }
	bool Is_Forced() const { return Forced; }
	EssentialsVotingType Get_Type() const { return Type; }
	clock_t Get_Start_Time()  const { return StartTime; }
	const StringClass& Get_Data() const { return VoteData; }
	const DynamicVectorClass<cPlayer*>& Get_Yes_Voters() const { return YesVoters; }
	int Get_Yes_Count() const { return YesVoters.Count(); }
	bool Add_Yes_Voter(cPlayer* Player) { if (YesVoters.ID(Player) == -1) return YesVoters.Add(Player); else return false; }
	bool Remove_Yes_Voter(cPlayer* Player) { return YesVoters.DeleteObj(Player); }
	bool Has_Voted_Yes(cPlayer* Player) { return YesVoters.ID(Player) != -1; }
	const DynamicVectorClass<cPlayer*>& Get_No_Voters() const { return NoVoters; }
	int Get_No_Count() const { return NoVoters.Count(); }
	bool Add_No_Voter(cPlayer* Player) { if (NoVoters.ID(Player) == -1) return NoVoters.Add(Player); else return false; }
	bool Remove_No_Voter(cPlayer* Player) { return NoVoters.DeleteObj(Player); }
	bool Has_Voted_No(cPlayer* Player) { return NoVoters.ID(Player) != -1; }
	bool Remove_Voter(cPlayer* Player) { return Remove_Yes_Voter(Player) || Remove_No_Voter(Player); }

private:
	int StarterID;
	bool Forced;
	EssentialsVotingType Type;
	clock_t StartTime;
	StringClass VoteData;
	DynamicVectorClass<cPlayer*> YesVoters;
	DynamicVectorClass<cPlayer*> NoVoters;
};