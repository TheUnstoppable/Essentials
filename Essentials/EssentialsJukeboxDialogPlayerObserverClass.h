#pragma once

#include "da_player.h"

class EssentialsJukeboxDialogPlayerObserverClass : public DAPlayerObserverClass {
private:
	int DialogID;
	int CurrentlyPlayingDescriptionLabelID;
	int CurrentlyPlayingLabelID;
	int NextUpDescriptionLabelID;
	int NextUpLabelID;
	int MusicDurationLabelID;
	int TimeElapsedLabelID;
	int ProgressBarID;

	int StartStopButtonID;
	int NextButtonID;
	int ShuffleButtonID;
	int CloseButtonID;

	int LoopOffCheckboxID;
	int LoopAllCheckboxID;
	int LoopOneCheckboxID;

	bool JukeboxEnabled;

public:
	void Init() override;
	virtual ~EssentialsJukeboxDialogPlayerObserverClass();
	const char* Get_Observer_Name() override { return "EssentialsJukeboxDialogPlayerObserverClass"; }

	void Dialog_Message(DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control) override;
	void Timer_Expired(int Number, unsigned Data) override;

	void Create_Dialog();
	void Update_Dialog();
	void Destroy_Dialog();
};

