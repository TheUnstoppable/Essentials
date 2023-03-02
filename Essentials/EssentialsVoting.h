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
	EssentialsPollClass() : StarterID(-1), Type(VOTE_NONE), StartTime(0) {
		
	}

	int Get_Starter_ID() const { return StarterID; }
	EssentialsVotingType Get_Type() const { return Type; }
	clock_t Get_Start_Time()  const { return StartTime; }
	const StringClass& Get_Data() const { return VoteData; }
	const DynamicVectorClass<cPlayer*>& Get_Yes_Voters() const { return YesVoters; }
	int Get_Yes_Count() const { return YesVoters.Count(); }
	bool Add_Yes_Voter(cPlayer* Player) { if (YesVoters.ID(Player) == -1) return YesVoters.Add(Player); else return false; }
	bool Remove_Yes_Voter(cPlayer* Player) { return YesVoters.DeleteObj(Player); }
	const DynamicVectorClass<cPlayer*>& Get_No_Voters() const { return NoVoters; }
	int Get_No_Count() const { return NoVoters.Count(); }
	bool Add_No_Voter(cPlayer* Player) { if (NoVoters.ID(Player) == -1) return NoVoters.Add(Player); else return false; }
	bool Remove_No_Voter(cPlayer* Player) { return NoVoters.DeleteObj(Player); }
	bool Remove_Voter(cPlayer* Player) { return Remove_Yes_Voter(Player) || Remove_No_Voter(Player); }

private:
	int StarterID;
	EssentialsVotingType Type;
	clock_t StartTime;
	StringClass VoteData;
	DynamicVectorClass<cPlayer*> YesVoters;
	DynamicVectorClass<cPlayer*> NoVoters;
};

class EssentialsVotingManagerClass : public DAEventClass {
public:
	static EssentialsVotingManagerClass* Instance;

	EssentialsVotingManagerClass();
	~EssentialsVotingManagerClass();

	void Settings_Loaded_Event() override;
	void Game_Over_Event() override;
	bool Chat_Command_Event(cPlayer* Player, TextMessageEnum Type, const StringClass& Command, const DATokenClass& Text, int ReceiverID) override;
	void Timer_Expired(int Number, unsigned Data) override;

	inline StringClass Get_Action_Description() const {
		switch (PollContext->Get_Type()) {
			case VOTE_TIME: return "extend the time";
			case VOTE_GAMEOVER: return "end the current game";
			case VOTE_RESTARTMAP: return "restart the current map";
			case VOTE_NEXTMAP: return StringFormat("change the next map to %s", PollContext->Get_Data());
			case VOTE_PLAYERKICK: {
				const char* pName = Get_Player_Name_By_ID(atoi(PollContext->Get_Data()));
				StringClass returnStr = StringFormat("kick player %s", pName);
				delete[] pName;
				return returnStr;
			}
			case VOTE_NOREPAIR: return "disable repairing buildings";

			case VOTE_NONE:
			default: return "do nothing";
		}
	}

	bool Is_Poll_In_Progress() const { return !!PollContext; }
	EssentialsPollClass* Get_Poll_Context() const { return PollContext; }
	EssentialsPollClass* Create_Poll(cPlayer* Starter, EssentialsVotingType Type, const char* Data = 0);
	bool End_Current_Poll();
	void Apply_Vote_Effect();

private:
	EssentialsPollClass* PollContext;
	float PollDuration;
	float PollAnnounceInterval;
	int PollStartMinLevel;
	int PollCancelMinLevel;
	int PollToggleMinLevel;

	StringClass PollStartSound;
	StringClass PollAnnounceSound;
	StringClass PollPassSound;
	StringClass PollFailSound;

	bool AllowVoting;
	bool AllowTimeVote;
	bool AllowGameOverVote;
	bool AllowRestartVote;
	bool AllowNextMapVote;
	bool AllowPlayerKickVote;
	bool AllowNoRepairVote;

	float TimePollAllowAfter;
	float TimePollExtraTime;
};