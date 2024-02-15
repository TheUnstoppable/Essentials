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
#include "EssentialsPollClass.h"

class EssentialsForcedVotingPlayerObserverClass;
class EssentialsVoteStatusPlayerObserverClass;

class EssentialsVotingManagerClass : public DAEventClass {
	friend class EssentialsForcedVotingPlayerObserverClass;
	friend class EssentialsVoteStatusPlayerObserverClass;
public:
	static EssentialsVotingManagerClass* Instance;

	EssentialsVotingManagerClass();
	~EssentialsVotingManagerClass();

	void Player_Join_Event(cPlayer* Player) override;
	void Player_Leave_Event(cPlayer* Player) override;
	void Level_Loaded_Event() override;
	void Settings_Loaded_Event() override;
	void Game_Over_Event() override;
	void Timer_Expired(int Number, unsigned Data) override;

	bool Vote_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
	bool Yes_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
	bool No_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);

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
	EssentialsPollClass* Create_Poll(cPlayer* Starter, EssentialsVotingType Type, bool Force = false, const char* Data = 0);
	bool End_Current_Poll();
	void Apply_Vote_Effect();

	void Force_State_Changed();
	void Add_Force_Context(cPlayer* Player);
	void Remove_Force_Context(cPlayer* Player);
	void Clear_Force_Contexts();
	void Add_Vote_Status(cPlayer* Player);
	void Remove_Vote_Status(cPlayer* Player);
	void Clear_Vote_Status();

private:
	EssentialsPollClass* PollContext;
	float PollDuration;
	float PollAnnounceInterval;
	float ForcedPollDuration;
	float ForcedPollAnnounceInterval;
	int PollStartMinLevel;
	int PollForceMinLevel;
	int PollCancelMinLevel;
	int PollToggleMinLevel;
	int PollEnableMinLevel;
	float PollCooldown;
	int RequiredPercentageToPass;

	StringClass PollStartSound;
	StringClass PollAnnounceSound;
	StringClass PollPassSound;
	StringClass PollFailSound;

	bool AllowVoting;
	int TimeVoteMinLevel;
	int GameOverVoteMinLevel;
	int RestartVoteMinLevel;
	int NextMapVoteMinLevel;
	int PlayerKickVoteMinLevel;
	int NoRepairVoteMinLevel;

	float TimePollAllowAfter;
	float TimePollExtraTime;

	float AutoSkipPollAfter;
	float AutoTimePollBefore;
	bool AutoTimePollStarted;

	SList<EssentialsForcedVotingPlayerObserverClass> ForceContexts;
	SList<EssentialsVoteStatusPlayerObserverClass> VoteStatus;
};
