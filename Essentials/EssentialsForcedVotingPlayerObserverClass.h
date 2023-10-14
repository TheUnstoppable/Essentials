#pragma once

#include "EssentialsPollClass.h"
#include "da_player.h"

class EssentialsForcedVotingPlayerObserverClass : public DAPlayerObserverClass {
private:
	int DialogID;
	int TitleLabelID;
	int DescriptionLabelID;
	int YourVoteLabelID;
	int YesButtonID;
	int NoButtonID;
	int PollDurationDescriptionLabelID;
	int PollDurationLabelID;
	int PollDurationProgressBarID;
	int VotesProgressBarID;
	int YesVotesLabelID;
	int NoVotesLabelID;
	int TotalVotesLabelID;

	EssentialsPollClass* PollContext;
	bool PollEnded;

public:
	void Init() override;
	virtual ~EssentialsForcedVotingPlayerObserverClass();
	const char* Get_Observer_Name() { return "EssentialsForcedVotingPlayerObserverClass"; }

	void Dialog_Message(DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control) override;

	void On_Poll_Start(EssentialsPollClass* poll);
	void On_Poll_End(bool passed);

	void Create_Dialog();
	void Destroy_Dialog();

	void Update_Data();
};

