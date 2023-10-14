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
#include "EssentialsForcedVotingPlayerObserverClass.h"
#include "EssentialsPlayerDataClass.h"
#include "EssentialsUtils.h"

#define EqualsIgnoreCase(String, Second) (String.Compare_No_Case(Second) == 0)

EssentialsVotingManagerClass* EssentialsVotingManagerClass::Instance = 0;

EssentialsVotingManagerClass::EssentialsVotingManagerClass() {
	Instance = this;
	PollContext = 0;
	AllowVoting = true;

	Register_Event(DAEvent::PLAYERJOIN);
	Register_Event(DAEvent::PLAYERLEAVE);
	Register_Event(DAEvent::SETTINGSLOADED);
	Register_Event(DAEvent::GAMEOVER);
	Register_Event(DAEvent::CHATCOMMAND);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Voting feature.");
}

EssentialsVotingManagerClass::~EssentialsVotingManagerClass() {
	Instance = 0;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Voting feature.");
}

void EssentialsVotingManagerClass::Player_Join_Event(cPlayer* Player) {
	if (PollContext && PollContext->Is_Forced()) {
		Add_Force_Context(Player);
		Force_State_Changed();
	}
}

void EssentialsVotingManagerClass::Player_Leave_Event(cPlayer* Player) {
	if (PollContext) {
		PollContext->Remove_Voter(Player);
		if (PollContext->Is_Forced()) {
			Remove_Force_Context(Player);
		}
		Force_State_Changed();
	}
}

void EssentialsVotingManagerClass::Settings_Loaded_Event() {
	PollDuration = DASettingsManager::Get_Float("Essentials", "PollDuration", 120.f);
	PollAnnounceInterval = DASettingsManager::Get_Float("Essentials", "PollAnnounceInterval", 15.f);
	ForcedPollDuration = DASettingsManager::Get_Float("Essentials", "ForcedPollDuration", 20.f);
	ForcedPollAnnounceInterval = DASettingsManager::Get_Float("Essentials", "ForcedPollAnnounceInterval", 5.f);
	PollStartMinLevel = DASettingsManager::Get_Int("Essentials", "PollStartMinLevel", 0);
	PollForceMinLevel = DASettingsManager::Get_Int("Essentials", "PollForceMinLevel", 0);
	PollCancelMinLevel = DASettingsManager::Get_Int("Essentials", "PollCancelMinLevel", 3);
	PollToggleMinLevel = DASettingsManager::Get_Int("Essentials", "PollToggleMinLevel", 3);
	PollEnableMinLevel = DASettingsManager::Get_Int("Essentials", "PollEnableMinLevel", 3);
	AllowTimeVote = DASettingsManager::Get_Bool("Essentials", "AllowTimeVote", true);
	AllowGameOverVote = DASettingsManager::Get_Bool("Essentials", "AllowGameOverVote", true);
	AllowRestartVote = DASettingsManager::Get_Bool("Essentials", "AllowRestartVote", true);
	AllowNextMapVote = DASettingsManager::Get_Bool("Essentials", "AllowNextMapVote", true);
	AllowPlayerKickVote = DASettingsManager::Get_Bool("Essentials", "AllowPlayerKickVote", true);
	AllowNoRepairVote = DASettingsManager::Get_Bool("Essentials", "AllowNoRepairVote", true);
	TimePollAllowAfter = DASettingsManager::Get_Float("Essentials", "TimePollAllowAfter", 900.f);
	TimePollExtraTime = DASettingsManager::Get_Float("Essentials", "TimePollExtraTime", 1800.f);
	PollCooldown = DASettingsManager::Get_Float("Essentials", "PollCooldown", 300.f);
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
			if (EqualsIgnoreCase(Text[1], "start") || EqualsIgnoreCase(Text[1], "forcestart")) {
				bool force = EqualsIgnoreCase(Text[1], "forcestart");
				if (force ? PollForceMinLevel : PollStartMinLevel >= -1) {
					if (Player->Get_DA_Player()->Get_Access_Level() >= (force ? PollForceMinLevel : PollStartMinLevel)) {
						if (Is_Timer(3000)) {
							DA::Page_Player(Player, "Poll cooldown is active, please try again later.");
							return false;
						}

						bool ret = true;
						if (EqualsIgnoreCase(Text[2], "time")) {
							if (!AllowTimeVote) {
								DA::Page_Player(Player, "Creating polls of this type is not allowed.");
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

							ret = !!Create_Poll(Player, VOTE_TIME, force);
							if (ret) {
								DA::Page_Player(Player, "Successfully created a poll with type \"time\".");
							}
						}
						else if (EqualsIgnoreCase(Text[2], "skip") || EqualsIgnoreCase(Text[2], "nextmap") || EqualsIgnoreCase(Text[2], "next") || EqualsIgnoreCase(Text[2], "cyclemap")) {
							if (!AllowGameOverVote) {
								DA::Page_Player(Player, "Creating polls of this type is not allowed.");
								return false;
							}

							ret = !!Create_Poll(Player, VOTE_GAMEOVER, force);
							if (ret) {
								DA::Page_Player(Player, "Successfully created a poll with type \"skip\".");
							}
						}
						else if (EqualsIgnoreCase(Text[2], "restart") || EqualsIgnoreCase(Text[2], "resetmap") || EqualsIgnoreCase(Text[2], "restartmap")) {
							if (!AllowRestartVote) {
								DA::Page_Player(Player, "Creating polls of this type is not allowed.");
								return false;
							}

							ret = !!Create_Poll(Player, VOTE_RESTARTMAP, force);
							if (ret) {
								DA::Page_Player(Player, "Successfully created a poll with type \"restart\".");
							}
						}
						else if (EqualsIgnoreCase(Text[2], "changenext") || EqualsIgnoreCase(Text[2], "snm") || EqualsIgnoreCase(Text[2], "setmap") || EqualsIgnoreCase(Text[2], "map")) {
							if (!AllowNextMapVote) {
								DA::Page_Player(Player, "Creating polls of this type is not allowed.");
								return false;
							}

							HashTemplateClass<StringClass, GameDefinition> gameDefs;
							Get_Game_Definitions(gameDefs);

							int matchCount = 0;
							StringClass mapName;

							for (HashTemplateIterator<StringClass, GameDefinition> it(gameDefs); it; ++it) {
								if (it.getKey() == Text[3].AsUpper()) {
									matchCount = 1;
									mapName = it.getValue().name;
									break;
								}
								if (String_Contains(it.getKey(), Text[3])) {
									matchCount++;
									mapName = it.getValue().name;
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

							ret = !!Create_Poll(Player, VOTE_NEXTMAP, force, mapName);
							if (ret) {
								DA::Page_Player(Player, "Successfully created a poll with type \"setmap\".");
							}
						}
						else if (EqualsIgnoreCase(Text[2], "kick")) {
							if (!AllowPlayerKickVote) {
								DA::Page_Player(Player, "Creating polls of this type is not allowed.");
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

							ret = !!Create_Poll(Player, VOTE_PLAYERKICK, force, StringFormat("%d", Match->Get_Id()));
							if (ret) {
								DA::Page_Player(Player, "Successfully created a poll with type \"kick\".");
							}
						}
						else if (EqualsIgnoreCase(Text[2], "norepair") || EqualsIgnoreCase(Text[2], "suddendeath") || EqualsIgnoreCase(Text[2], "norep")) {
							if (!AllowNoRepairVote) {
								DA::Page_Player(Player, "Creating polls of this type is not allowed.");
								return false;
							}

							ret = !!Create_Poll(Player, VOTE_NOREPAIR, force);
							if (ret) {
								DA::Page_Player(Player, "Successfully created a poll with type \"norep\".");
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

						if (!ret) {
							DA::Page_Player(Player, "Unable to create a poll of type \"%s\".", Text[2]);
						} else {
							Start_Timer(3000, PollCooldown);
						}
						return ret;
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
							DA::Page_Player(Player, "There isn't an ongoing poll at the moment.");
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
			if (EqualsIgnoreCase(Text[1], "enable")) {
				if (PollEnableMinLevel >= -1) {
					if (Player->Get_DA_Player()->Get_Access_Level() >= PollEnableMinLevel) {
						Stop_Timer(3000);
						DA::Page_Player(Player, "Successfully expired the poll cooldown.");
						return true;
					}
					else {
						DA::Page_Player(Player, "You don't have permission to enable polls.");
						return false;
					}
				}
				else {
					DA::Page_Player(Player, "Enabling polls are disabled.");
				}
			}

			if (Text.Size() > 0) {
				DA::Page_Player(Player, "%s is an invalid option. Valid options are: yes, no, remove, start, cancel, enable.", Text[1]);
			}
			else {
				DA::Page_Player(Player, "Usage: !vote <yes/no/remove/start/cancel/toggle/enable>");
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
		float duration = (PollContext->Is_Forced() ? ForcedPollDuration : PollDuration);
		float interval = (PollContext->Is_Forced() ? ForcedPollAnnounceInterval : PollAnnounceInterval);

		clock_t voteEnd = PollContext->Get_Start_Time() + (clock_t)(duration * 1000);
		DA::Host_Message("A poll %s to %s. You have %.0f seconds to vote \"!yes\" or \"!no\".", newPoll ? "has just started" : "is in progress", Get_Action_Description(), (voteEnd - clock()) / 1000.f);
		DA::Create_2D_Sound(newPoll ? PollStartSound : PollAnnounceSound);

		if (clock() + (interval * 1000) < voteEnd) {
			Start_Timer(1000, interval);
		}
	}
	else if (Number == 1001) {
		int yes = Get_Poll_Context()->Get_Yes_Count(), no = Get_Poll_Context()->Get_No_Count();
		bool one_third = ((float)(yes + no) >= The_Game()->Get_Current_Players() / 3.f);
		bool pass = one_third && yes > no;

		DA::Host_Message("Poll to %s is over. [Yes Votes: %d | No Votes: %d]", Get_Action_Description(), yes, no);
		DA::Create_2D_Sound(pass ? PollPassSound : PollFailSound);

		if (pass) {
			DA::Host_Message("Vote has PASSED, there are %d more yes votes than no votes.", abs(yes - no));
		} else {
			if (!one_third) {
				DA::Host_Message("Vote has FAILED, at least one third of the players must contribute to the poll.");
			} else if (yes < no) {
				DA::Host_Message("Vote has FAILED, there are %d more no votes than yes votes.", abs(yes - no));
			} else {
				DA::Host_Message("Vote has FAILED, there are equal yes votes and no votes.");
			}
		}

		for (SLNode<EssentialsForcedVotingPlayerObserverClass>* Node = ForceContexts.Head(); Node; Node = Node->Next()) {
			Node->Data()->On_Poll_End(pass);
		}

		if (pass) {
			if (PollContext->Is_Forced()) {
				Start_Timer(4000, 5.0f);
			} else {
				Timer_Expired(4000, 0);
			}
		} else {
			if (PollContext->Is_Forced()) {
				Start_Timer(4001, 5.0f);
			}
		}
	}
	else if (Number == 1002) {
		Force_State_Changed();
	}
	else if (Number == 2000) {
		cGameData::Set_Manual_Restart(true);
	}
	else if (Number == 2001) {
		Reload_Level();
	}
	else if (Number == 4000) {
		Apply_Vote_Effect();
		End_Current_Poll();
	}
	else if (Number == 4001) {
		End_Current_Poll();
	}
}

EssentialsPollClass* EssentialsVotingManagerClass::Create_Poll(cPlayer* Starter, EssentialsVotingType Type, bool Force, const char* Data) {
	if (Is_Poll_In_Progress())
		return 0;

	PollContext = new EssentialsPollClass;
	PollContext->StarterID = Starter ? Starter->Get_Id() : 0;
	PollContext->Forced = Force;
	PollContext->Type = Type;
	PollContext->StartTime = clock();
	PollContext->VoteData = Data;

	if (Force) {
		for (SLNode<cPlayer>* n = Get_Player_List()->Head(); n; n = n->Next()) {
			if (n->Data()->Get_Is_Active() && n->Data()->Get_Is_In_Game())
				Add_Force_Context(n->Data());
		}

		Set_Gameplay_Allowed(false);

		Start_Timer(1002, 0.25f, true, 0);
	}

	Timer_Expired(1000, 1);
	Start_Timer(1001, Force ? ForcedPollDuration : PollDuration);

	return PollContext;
}

bool EssentialsVotingManagerClass::End_Current_Poll() {
	if (!Is_Poll_In_Progress())
		return false;

	if (PollContext->Is_Forced()) {
		Set_Gameplay_Allowed(true);
		Clear_Force_Contexts();
	}

	Stop_Timer(1000);
	Stop_Timer(1001);
	Stop_Timer(1002);

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

void EssentialsVotingManagerClass::Force_State_Changed() {
	if (!Is_Poll_In_Progress()) {
		return;
	}

	for (SLNode<EssentialsForcedVotingPlayerObserverClass>* Node = ForceContexts.Head(); Node; Node = Node->Next()) {
		Node->Data()->Update_Data();
	}
}

void EssentialsVotingManagerClass::Add_Force_Context(cPlayer* Player) {
	if (!Is_Poll_In_Progress()) {
		return;
	}

	for(SLNode<EssentialsForcedVotingPlayerObserverClass>* Node = ForceContexts.Head(); Node; Node = Node->Next()) {
		if (Node->Data()->Get_Owner() == Player) {
			return;
		}
	}

	EssentialsForcedVotingPlayerObserverClass* Context = new EssentialsForcedVotingPlayerObserverClass;
	Player->Get_DA_Player()->Add_Observer(Context);
	Context->On_Poll_Start(PollContext);

	ForceContexts.Add_Tail(Context);
}

void EssentialsVotingManagerClass::Remove_Force_Context(cPlayer* Player) {
	for (SLNode<EssentialsForcedVotingPlayerObserverClass>* Node = ForceContexts.Head(); Node; Node = Node->Next()) {
		if (Node->Data()->Get_Owner() == Player) {
			Node->Data()->Set_Delete_Pending();
			ForceContexts.Remove(Node->Data());
			return;
		}
	}
}

void EssentialsVotingManagerClass::Clear_Force_Contexts() {
	while (EssentialsForcedVotingPlayerObserverClass* Node = ForceContexts.Remove_Head()) {
		Node->Set_Delete_Pending();
	}
}
