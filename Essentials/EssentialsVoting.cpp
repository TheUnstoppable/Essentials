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
#include "da.h"
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsVoting.h"
#include "HashTemplateIterator.h"
#include "engine_da.h"
#include "EssentialsEventClass.h"
#include "EssentialsPlayerDataClass.h"
#include "EssentialsUtils.h"

#define EqualsIgnoreCase(String, Second) (String.Compare_No_Case(Second) == 0)

EssentialsVotingManagerClass* EssentialsVotingManagerClass::Instance = 0;

EssentialsVotingManagerClass::EssentialsVotingManagerClass() {
	Instance = this;
	PollContext = 0;
	AllowVoting = true;

	Register_Event(DAEvent::SETTINGSLOADED);
	Register_Event(DAEvent::CHATCOMMAND);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Voting feature.");
}

EssentialsVotingManagerClass::~EssentialsVotingManagerClass() {
	Instance = 0;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Voting feature.");
}

void EssentialsVotingManagerClass::Settings_Loaded_Event() {
	PollDuration = DASettingsManager::Get_Float("Essentials", "PollDuration", 120.f);
	PollAnnounceInterval = DASettingsManager::Get_Float("Essentials", "PollAnnounceInterval", 15.f);
	PollStartMinLevel = DASettingsManager::Get_Int("Essentials", "PollStartMinLevel", 0);
	PollCancelMinLevel = DASettingsManager::Get_Int("Essentials", "PollCancelMinLevel", 3);
	PollToggleMinLevel = DASettingsManager::Get_Int("Essentials", "PollToggleMinLevel", 3);
	AllowTimeVote = DASettingsManager::Get_Bool("Essentials", "AllowTimeVote", true);
	AllowGameOverVote = DASettingsManager::Get_Bool("Essentials", "AllowGameOverVote", true);
	AllowRestartVote = DASettingsManager::Get_Bool("Essentials", "AllowRestartVote", true);
	AllowNextMapVote = DASettingsManager::Get_Bool("Essentials", "AllowNextMapVote", true);
	AllowPlayerKickVote = DASettingsManager::Get_Bool("Essentials", "AllowPlayerKickVote", true);
	AllowNoRepairVote = DASettingsManager::Get_Bool("Essentials", "AllowNoRepairVote", true);
	TimePollAllowAfter = DASettingsManager::Get_Float("Essentials", "TimePollAllowAfter", 900.f);
	TimePollExtraTime = DASettingsManager::Get_Float("Essentials", "TimePollExtraTime", 1800.f);
	DASettingsManager::Get_String(PollStartSound, "Essentials", "PollStartSound", "null");
	DASettingsManager::Get_String(PollAnnounceSound, "Essentials", "PollAnnounceSound", "null");
	DASettingsManager::Get_String(PollPassSound, "Essentials", "PollPassSound", "null");
	DASettingsManager::Get_String(PollFailSound, "Essentials", "PollFailSound", "null");
}

void EssentialsVotingManagerClass::Game_Over_Event() {
	End_Current_Poll();
}

bool EssentialsVotingManagerClass::Chat_Command_Event(cPlayer* Player, TextMessageEnum Type, const StringClass& Command, const DATokenClass& Text, int ReceiverID) {
	if (EqualsIgnoreCase(Command, "!vote") || EqualsIgnoreCase(Command, "!poll")) {
		if (EqualsIgnoreCase(Text[1], "toggle")) {
			if (PollToggleMinLevel >= -1) {
				if (Player->Get_DA_Player()->Get_Access_Level() >= PollToggleMinLevel) {
					AllowVoting = !AllowVoting;
					DA::Page_Player(Player, "Voting is now %s.", AllowVoting ? "enabled" : "disabled");
				}
				else {
					DA::Page_Player(Player, "You don't have permission to toggle voting.");
				}
			}
			else {
				DA::Page_Player(Player, "Toggling voting is disabled.");
			}
			return false;
		}
		if (AllowVoting) {
			if (EqualsIgnoreCase(Text[1], "yes")) {
				if (!Is_Poll_In_Progress()) {
					DA::Page_Player(Player, "There isn't anything to vote at the moment.");
					return false;
				}

				Get_Poll_Context()->Remove_Voter(Player);
				if (Get_Poll_Context()->Add_Yes_Voter(Player)) {
					DA::Page_Player(Player, "You have voted Yes.");
					return true;
				}
				else {
					DA::Page_Player(Player, "Could not register your vote.");
					return false;
				}
			}
			if (EqualsIgnoreCase(Text[1], "no")) {
				if (!Is_Poll_In_Progress()) {
					DA::Page_Player(Player, "There isn't anything to vote at the moment.");
					return false;
				}

				Get_Poll_Context()->Remove_Voter(Player);
				if (Get_Poll_Context()->Add_No_Voter(Player)) {
					DA::Page_Player(Player, "You have voted No.");
					return true;
				}
				else {
					DA::Page_Player(Player, "Could not register your vote.");
					return false;
				}
			}
			if (EqualsIgnoreCase(Text[1], "remove")) {
				if (!Is_Poll_In_Progress()) {
					DA::Page_Player(Player, "There isn't anything to vote at the moment.");
					return false;
				}

				if (Get_Poll_Context()->Remove_Voter(Player)) {
					DA::Page_Player(Player, "Your vote has been removed.");
					return true;
				}
				else {
					DA::Page_Player(Player, "Could not find your vote to the current poll.");
					return false;
				}
			}
			if (EqualsIgnoreCase(Text[1], "start")) {
				if (PollStartMinLevel >= -1) {
					if (Player->Get_DA_Player()->Get_Access_Level() >= PollStartMinLevel) {
						if (EqualsIgnoreCase(Text[2], "time")) {
							if (!AllowTimeVote) {
								DA::Page_Player(Player, "Creating polls with this type is now allowed");
								return false;
							}

							if (!The_Game()->Is_Time_Limit()) {
								DA::Page_Player(Player, "Can't extend the time on a map with no time limit.");
								return false;
							}

							if (The_Game()->Get_Time_Remaining_Seconds() > TimePollAllowAfter) {
								DA::Page_Player(Player, "Voting for more time is allowed after time left reaches %s.", Format_Seconds_Friendly((int)TimePollAllowAfter));
								return false;
							}

							if (Create_Poll(Player, VOTE_TIME)) {
								DA::Page_Player(Player, "Successfully created a poll with type \"time\".");
								return true;
							}
						}
						else if (EqualsIgnoreCase(Text[2], "skip") || EqualsIgnoreCase(Text[2], "nextmap") || EqualsIgnoreCase(Text[2], "next") || EqualsIgnoreCase(Text[2], "cyclemap")) {
							if (!AllowGameOverVote) {
								DA::Page_Player(Player, "Creating polls with this type is now allowed");
								return false;
							}

							if (Create_Poll(Player, VOTE_GAMEOVER)) {
								DA::Page_Player(Player, "Successfully created a poll with type \"skip\".");
								return true;
							}
						}
						else if (EqualsIgnoreCase(Text[2], "restart") || EqualsIgnoreCase(Text[2], "resetmap") || EqualsIgnoreCase(Text[2], "restartmap")) {
							if (!AllowRestartVote) {
								DA::Page_Player(Player, "Creating polls with this type is now allowed");
								return false;
							}

							if (Create_Poll(Player, VOTE_RESTARTMAP)) {
								DA::Page_Player(Player, "Successfully created a poll with type \"restart\".");
								return true;
							}
						}
						else if (EqualsIgnoreCase(Text[2], "changenext") || EqualsIgnoreCase(Text[2], "snm") || EqualsIgnoreCase(Text[2], "setmap") || EqualsIgnoreCase(Text[2], "map")) {
							if (!AllowNextMapVote) {
								DA::Page_Player(Player, "Creating polls with this type is now allowed");
								return false;
							}

							HashTemplateClass<StringClass, GameDefinition> gameDefs;
							Get_Game_Definitions(gameDefs);

							int matchCount = 0;
							StringClass mapName;

							for (HashTemplateIterator<StringClass, GameDefinition> it(gameDefs); it; ++it) {
								if (!String_Contains(it.getKey(), "C&C_")) {
									if (String_Contains(it.getKey(), Text[3])) {
										matchCount++;
										mapName = it.getValue().name;
									}
								}
							}

							if (!matchCount) {
								DA::Page_Player(Player, "Couldn't find any matches for the search term \"%s\".", Text[3]);
								return false;
							}
							if (matchCount > 1) {
								DA::Page_Player(Player, "Too many matches found for the search term \"%s\", please be more specific.", Text[3]);
								return false;
							}

							if (Create_Poll(Player, VOTE_NEXTMAP, mapName)) {
								DA::Page_Player(Player, "Successfully created a poll with type \"setmap\".");
								return true;
							}
						}
						else if (EqualsIgnoreCase(Text[2], "kick")) {
							if (!AllowPlayerKickVote) {
								DA::Page_Player(Player, "Creating polls with this type is now allowed");
								return false;
							}

							cPlayer* Match = Match_Player(Player, Text[3], false, true);
							if (!Match) {
								return false;
							}
							if (Match == Player) {
								DA::Page_Player(Player, "Creating a poll to kick yourself might not be a great idea.");
								return false;
							}

							if (Create_Poll(Player, VOTE_PLAYERKICK, StringFormat("%d", Match->Get_Id()))) {
								DA::Page_Player(Player, "Successfully created a poll with type \"kick\".");
								return true;
							}
						}
						else if (EqualsIgnoreCase(Text[2], "norepair") || EqualsIgnoreCase(Text[2], "suddendeath") || EqualsIgnoreCase(Text[2], "norep")) {
							if (!AllowNoRepairVote) {
								DA::Page_Player(Player, "Creating polls with this type is now allowed");
								return false;
							}

							if (Create_Poll(Player, VOTE_NOREPAIR)) {
								DA::Page_Player(Player, "Successfully created a poll with type \"norep\".");
								return true;
							}
						}
						else {
							if (Text.Size() > 1) {
								DA::Page_Player(Player, "%s is not a known poll type. Valid types are: \"time, skip, restart, setmap, kick, norep\"", Text[2]);
							}
							else {
								DA::Page_Player(Player, "Usage: !vote start <time/skip/restart/setmap/kick/norep>");
							}
							return false;
						}

						DA::Page_Player(Player, "Unable to create a poll with type \"%s\".", Text[2]);
						return false;
					}
					else {
						DA::Page_Player(Player, "You don't have permission to start a new poll.");
						return false;
					}
				}
				else {
					DA::Page_Player(Player, "Creating new polls are disabled.");
					return false;
				}
			}
			if (EqualsIgnoreCase(Text[1], "cancel")) {
				if (PollCancelMinLevel >= -1) {
					if (Player->Get_DA_Player()->Get_Access_Level() >= PollCancelMinLevel) {
						if (End_Current_Poll()) {
							DA::Page_Player(Player, "Successfully cancelled ongoing poll.");
							return true;
						}
						else {
							DA::Page_Player(Player, "Unable to create a poll with type \"%s\".", Text[2]);
							return false;
						}
					}
					else {
						DA::Page_Player(Player, "You don't have permission to cancel an ongoing poll.");
						return false;
					}
				}
				else {
					DA::Page_Player(Player, "Cancelling polls are disabled.");
				}
			}

			if (Text.Size() > 0) {
				DA::Page_Player(Player, "%s is an invalid option. Valid options are: yes, no, remove, start, cancel.", Text[1]);
			}
			else {
				DA::Page_Player(Player, "Usage: !vote <yes/no/remove/start/cancel/toggle>");
			}
			return false;
		}
		else {
			DA::Page_Player(Player, "Voting has been temporarily disabled by the staff.", Text[1]);
			return false;
		}
	}
	if (EqualsIgnoreCase(Command, "!yes")) {
		if (AllowVoting) {
			if (!Is_Poll_In_Progress()) {
				DA::Page_Player(Player, "There isn't anything to vote at the moment.");
				return false;
			}

			Get_Poll_Context()->Remove_Voter(Player);
			if (Get_Poll_Context()->Add_Yes_Voter(Player)) {
				DA::Page_Player(Player, "You have voted Yes.");
				return true;
			}
			else {
				DA::Page_Player(Player, "Could not register your vote.");
				return false;
			}
		}
		else {
			DA::Page_Player(Player, "Voting has been temporarily disabled by the staff.", Text[1]);
			return false;
		}
	}
	if (EqualsIgnoreCase(Command, "!no")) {
		if (AllowVoting) {
			if (!Is_Poll_In_Progress()) {
				DA::Page_Player(Player, "There isn't anything to vote at the moment.");
				return false;
			}

			Get_Poll_Context()->Remove_Voter(Player);
			if (Get_Poll_Context()->Add_No_Voter(Player)) {
				DA::Page_Player(Player, "You have voted No.");
				return true;
			}
			else {
				DA::Page_Player(Player, "Could not register your vote.");
				return false;
			}
		}
		else {
			DA::Page_Player(Player, "Voting has been temporarily disabled by the staff.", Text[1]);
			return false;
		}
	}

	return true;
}

void EssentialsVotingManagerClass::Timer_Expired(int Number, unsigned Data) {
	if (Number == 1000) {
		bool newPoll = Data == 1;
		clock_t voteEnd = PollContext->Get_Start_Time() + (clock_t)(PollDuration * 1000);
		DA::Host_Message("A poll %s to %s. You have %.0f seconds to vote \"!yes\" or \"!no\".", newPoll ? "has just started" : "is in progress", Get_Action_Description(), (voteEnd - clock()) / 1000.f);
		DA::Create_2D_Sound(newPoll ? PollStartSound : PollAnnounceSound);

		if (clock() + (PollAnnounceInterval * 1000) < voteEnd) {
			Start_Timer(1000, PollAnnounceInterval);
		}
	}
	else if (Number == 1001) {
		int yes = Get_Poll_Context()->Get_Yes_Count(), no = Get_Poll_Context()->Get_No_Count();

		DA::Host_Message("Poll to %s is over. [Yes Votes: %d | No Votes: %d]", Get_Action_Description(), yes, no);

		if ((float)(yes + no) < The_Game()->Get_Current_Players() / 3.f) {
			DA::Host_Message("Vote has FAILED, at least one third of the players must contribute to the poll.");
			DA::Create_2D_Sound(PollFailSound);
		}
		else {
			DA::Create_2D_Sound(yes > no ? PollPassSound : PollFailSound);

			if (yes > no) {
				DA::Host_Message("Vote has PASSED, there are %d more yes votes than no votes.", abs(yes - no));
				Apply_Vote_Effect();
			}
			else if (yes < no) {
				DA::Host_Message("Vote has FAILED, there are %d more no votes than yes votes.", abs(yes - no));
			}
			else {
				DA::Host_Message("Vote has FAILED, there are equal yes votes and no votes.");
			}
		}

		End_Current_Poll();
	}
	else if (Number == 2000) {
		cGameData::Set_Manual_Restart(true);
	}
	else if (Number == 2001) {
		Reload_Level();
	}
}

EssentialsPollClass* EssentialsVotingManagerClass::Create_Poll(cPlayer* Starter, EssentialsVotingType Type, const char* Data) {
	if (Is_Poll_In_Progress())
		return 0;

	PollContext = new EssentialsPollClass;
	PollContext->StarterID = Starter ? Starter->Get_Id() : 0;
	PollContext->Type = Type;
	PollContext->StartTime = clock();
	PollContext->VoteData = Data;

	Timer_Expired(1000, 1);
	Start_Timer(1001, PollDuration);

	return PollContext;
}

bool EssentialsVotingManagerClass::End_Current_Poll() {
	if (!Is_Poll_In_Progress())
		return false;

	Stop_Timer(1000);
	Stop_Timer(1001);

	delete PollContext;
	PollContext = 0;
	return true;
}

void EssentialsVotingManagerClass::Apply_Vote_Effect() {
	switch(PollContext->Type) {
		case VOTE_TIME: {
			Change_Time_Remaining(The_Game()->Get_Time_Remaining_Seconds() + TimePollExtraTime);
			DA::Host_Message("Time left extended by %s.", Format_Seconds_Friendly((int)TimePollExtraTime));
		}
		break;

		case VOTE_GAMEOVER: {
			Start_Timer(2000, 4.0f);
			DA::Host_Message("Ending the game in 4 seconds...");
		}
		break;

		case VOTE_RESTARTMAP: {
			Start_Timer(2001, 4.0f);
			DA::Host_Message("Restarting the current map in 4 seconds...");
		}
		break;

		case VOTE_NEXTMAP: {
			EssentialsEventClass::Instance->Set_Next_Map(PollContext->Get_Data());
			DA::Host_Message("Setting next map to %s...", PollContext->Get_Data());
		}
		break;

		case VOTE_PLAYERKICK: {
			Evict_Client(atoi(PollContext->Get_Data()), L"Poll to kick you passed.");
		}
		break;

		case VOTE_NOREPAIR: {
			EssentialsEventClass::Instance->AllowBuildingRepair = false;
			DA::Host_Message("Building repairing is now disabled.");
		}
		break;
	}
}
