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

void EssentialsForcedVotingPlayerObserverClass::Init() {
	PollContext = 0;
	PollEnded = false;
	Create_Dialog();
}

EssentialsForcedVotingPlayerObserverClass::~EssentialsForcedVotingPlayerObserverClass() {
	if (Find_Player(Get_ID())) {
		Destroy_Dialog();
	}
}

void EssentialsForcedVotingPlayerObserverClass::Dialog_Message(DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control) {
	if (!PollContext) return;
	if (PollEnded) return;

	if (Type == MESSAGE_TYPE_CONTROL_MOUSE_CLICK) {
		if (Control->Get_Control_ID() == YesButtonID) {
			PollContext->Remove_Voter(Get_Owner());
			PollContext->Add_Yes_Voter(Get_Owner());
			EssentialsVotingManagerClass::Instance->Force_State_Changed();
		}
		else if (Control->Get_Control_ID() == NoButtonID) {
			PollContext->Remove_Voter(Get_Owner());
			PollContext->Add_No_Voter(Get_Owner());
			EssentialsVotingManagerClass::Instance->Force_State_Changed();
		}
	}
}

void EssentialsForcedVotingPlayerObserverClass::On_Poll_Start(EssentialsPollClass* poll) {
	PollContext = poll;
	PollEnded = false;
	Update_Data();
}

void EssentialsForcedVotingPlayerObserverClass::On_Poll_End(bool passed) {
	if (ScriptedDialogClass* Dialog = Find_Dialog(DialogID)) {
		PollEnded = true;
		if (ScriptedLabelControlClass* TitleLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(TitleLabelID)) {
			TitleLabel->Set_Location(5, 30);
			TitleLabel->Set_Style(TEXTSTYLE_TITLE);
			TitleLabel->Set_Label_Text(passed ? L"Poll PASSED" : L"Poll FAILED");
		}
		if (ScriptedLabelControlClass* DescriptionLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(DescriptionLabelID)) {
			Dialog->Remove_Control(DescriptionLabel);
		}
		if (ScriptedLabelControlClass* YourVoteLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(YourVoteLabelID)) {
			Dialog->Remove_Control(YourVoteLabel);
		}
		if (ScriptedButtonControlClass* YesButton = (ScriptedButtonControlClass*)Dialog->Find_Control(YesButtonID)) {
			Dialog->Remove_Control(YesButton);
		}
		if (ScriptedButtonControlClass* NoButton = (ScriptedButtonControlClass*)Dialog->Find_Control(NoButtonID)) {
			Dialog->Remove_Control(NoButton);
		}
		if (ScriptedProgressBarControlClass* ProgressBar = (ScriptedProgressBarControlClass*)Dialog->Find_Control(PollDurationProgressBarID)) {
			Dialog->Remove_Control(ProgressBar);
		}
		if (ScriptedLabelControlClass* DurationLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(PollDurationLabelID)) {
			Dialog->Remove_Control(DurationLabel);
		}
		if (ScriptedLabelControlClass* DurationLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(PollDurationDescriptionLabelID)) {
			Dialog->Remove_Control(DurationLabel);
		}
	}
}

void EssentialsForcedVotingPlayerObserverClass::Create_Dialog() {
	ScriptedPopupDialogClass* Popup = Create_Centered_Popup(Get_ID(), 220, 122);
	DialogID = Popup->Get_Dialog_ID();

	TitleLabelID = Create_Label_Control(Popup, 5, 5, 210, 16, L"The poll is still initializing, please wait until the next network update. Consider yourself lucky for seeing this message.")->Get_Control_ID();
	DescriptionLabelID = Create_Label_Control(Popup, 5, 22, 38, 8, L"Your vote is: ")->Get_Control_ID();
	YourVoteLabelID = Create_Label_Control(Popup, 44, 22, 40, 8, L"Not Voted", TEXTSTYLE_BODY, Vector3(0.5, 0.5, 0.5))->Get_Control_ID();
	YesButtonID = Create_Title_Button_Control(Popup, 5, 35, 105, 25, L"Vote Yes")->Get_Control_ID();
	NoButtonID = Create_Title_Button_Control(Popup, 110, 35, 105, 25, L"Vote No")->Get_Control_ID();
	PollDurationDescriptionLabelID = Create_Label_Control(Popup, 5, 65, 50, 8, L"The poll ends in: ")->Get_Control_ID();
	PollDurationLabelID = Create_Label_Control(Popup, 56, 65, 140, 8, L"This poll is infinite, there's no way out.")->Get_Control_ID();
	PollDurationProgressBarID = Create_ProgressBar_Control(Popup, 5, 75, 210, 12)->Get_Control_ID();
	YesVotesLabelID = Create_Label_Control(Popup, 5, 95, 60, 8, L"Yes: 0 votes (50%)")->Get_Control_ID();
	auto TotalVotesLabel = Create_Label_Control(Popup, 70, 95, 80, 8, L"Total Votes: (0/0)", TEXTSTYLE_BODY, Vector3(1, 0, 0));
	TotalVotesLabel->Set_Orientation(TEXTORIENTATION_CENTER);
	TotalVotesLabelID = TotalVotesLabel->Get_Control_ID();
	auto NoVotesLabel = Create_Label_Control(Popup, 155, 95, 60, 8, L"No: 0 votes (50%)");
	NoVotesLabel->Set_Orientation(TEXTORIENTATION_RIGHT);
	NoVotesLabelID = NoVotesLabel->Get_Control_ID();
	VotesProgressBarID = Create_ProgressBar_Control(Popup, 5, 105, 210, 12)->Get_Control_ID();

	Show_Dialog(Popup);
}

void EssentialsForcedVotingPlayerObserverClass::Destroy_Dialog() {
	if (ScriptedDialogClass* Dialog = Find_Dialog(DialogID))
		Delete_Dialog(Dialog);
}

void EssentialsForcedVotingPlayerObserverClass::Update_Data() {
	if (!PollContext) return;
	if (PollEnded) return;

	if (ScriptedDialogClass* Dialog = Find_Dialog(DialogID)) {
		if (ScriptedLabelControlClass* TitleLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(TitleLabelID)) {
			TitleLabel->Set_Label_Text(WideStringFormat(L"A poll is in progress to %hs. Please vote below.", EssentialsVotingManagerClass::Instance->Get_Action_Description().Peek_Buffer()));
		}
		if (ScriptedLabelControlClass* VoteLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(YourVoteLabelID)) {
			if (PollContext->Has_Voted_Yes(Get_Owner())) {
				VoteLabel->Set_Label_Text(L"YES");
				VoteLabel->Set_Text_Color(0, 255, 0);
			} else if (PollContext->Has_Voted_No(Get_Owner())) {
				VoteLabel->Set_Label_Text(L"NO");
				VoteLabel->Set_Text_Color(255, 0, 0);
			} else {
				VoteLabel->Set_Label_Text(L"Not Voted");
				VoteLabel->Set_Text_Color(127, 127, 127);
			}
		}

		int duration = (int)EssentialsVotingManagerClass::Instance->ForcedPollDuration;
		int elapsed = (clock() - PollContext->Get_Start_Time()) / 1000;
		int remaining = duration - elapsed;

		if (ScriptedProgressBarControlClass* ProgressBar = (ScriptedProgressBarControlClass*)Dialog->Find_Control(PollDurationProgressBarID)) {
			ProgressBar->Set_Minimum(0);
			ProgressBar->Set_Maximum(duration);
			ProgressBar->Set_Progress(remaining);
		}

		if (ScriptedLabelControlClass* DurationLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(PollDurationLabelID)) {
			DurationLabel->Set_Label_Text(WideStringFormat(L"%d seconds", remaining));
		}

		int yes = PollContext->Get_Yes_Count(), no = PollContext->Get_No_Count(), total = yes + no;

		if (ScriptedProgressBarControlClass* ProgressBar = (ScriptedProgressBarControlClass*)Dialog->Find_Control(VotesProgressBarID)) {
			if (total > 0) {
				ProgressBar->Set_Minimum(0);
				ProgressBar->Set_Maximum(total);
				ProgressBar->Set_Progress(yes);
			} else {
				ProgressBar->Set_Minimum(0);
				ProgressBar->Set_Maximum(2);
				ProgressBar->Set_Progress(1);
			}
		}

		if (ScriptedLabelControlClass* YesVotesLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(YesVotesLabelID)) {
			YesVotesLabel->Set_Label_Text(WideStringFormat(L"Yes: %d votes (%d%%)", yes, total == 0 ? 0 : (int)((float)yes / total * 100)));
		}

		if (ScriptedLabelControlClass* NoVotesLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(NoVotesLabelID)) {
			NoVotesLabel->Set_Label_Text(WideStringFormat(L"No: %d votes (%d%%)", no, total == 0 ? 0 : (int)((float)no / total * 100)));
		}

		if (ScriptedLabelControlClass* TotalVotesLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(TotalVotesLabelID)) {
			int count = Get_Player_Count();
			TotalVotesLabel->Set_Label_Text(WideStringFormat(L"Total Votes: (%d/%d)", total, count));
			TotalVotesLabel->Set_Orientation(TEXTORIENTATION_CENTER);
			if (total < (count / 3.f)) {
				TotalVotesLabel->Set_Text_Color(255, 0, 0);
			} else {
				TotalVotesLabel->Set_Text_Color(255, 230, 0);
			}
		}
	}
}
