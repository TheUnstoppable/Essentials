#include "General.h"
#include "EssentialsJukeboxDialogPlayerObserverClass.h"

#include "EssentialsEventClass.h"
#include "EssentialsUtils.h"

void EssentialsJukeboxDialogPlayerObserverClass::Init() {
	Create_Dialog();
	Timer_Expired(1000, 0);
	Start_Timer(1000, 0.25f, true);
}

EssentialsJukeboxDialogPlayerObserverClass::~EssentialsJukeboxDialogPlayerObserverClass() {
	Destroy_Dialog();
}

void EssentialsJukeboxDialogPlayerObserverClass::Dialog_Message(DialogMessageType Type, ScriptedDialogClass* Dialog, ScriptedControlClass* Control) {
	if (Type == MESSAGE_TYPE_DIALOG_ESCAPE) {
		Set_Delete_Pending();
	}
	else if (Type == MESSAGE_TYPE_CONTROL_MOUSE_CLICK && Control) {
		if (Control->Get_Control_ID() == StartStopButtonID) {
			JukeboxEnabled ? EssentialsJukeboxClass::Instance->Jukebox_Stop(Get_Owner()) : EssentialsJukeboxClass::Instance->Jukebox_Start(Get_Owner());
		}
		else if (Control->Get_Control_ID() == NextButtonID) {
			EssentialsJukeboxClass::Instance->Jukebox_Next(Get_Owner());
		}
		else if (Control->Get_Control_ID() == ShuffleButtonID) {
			EssentialsJukeboxClass::Instance->Jukebox_Shuffle(Get_Owner());
		}
		else if (Control->Get_Control_ID() == CloseButtonID) {
			Set_Delete_Pending();
		}
		else if (Control->Get_Control_ID() == LoopOffCheckboxID) {
			if (((ScriptedCheckBoxControlClass*)Control)->Is_Checked()) {
				((ScriptedCheckBoxControlClass*)Dialog->Find_Control(LoopAllCheckboxID))->Set_Checked(false);
				((ScriptedCheckBoxControlClass*)Dialog->Find_Control(LoopOneCheckboxID))->Set_Checked(false);
				if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Get_Owner())) {
					Data->Set_LoopMode(0);
				}
			}
			else {
				((ScriptedCheckBoxControlClass*)Control)->Set_Checked(true);
			}
		}
		else if (Control->Get_Control_ID() == LoopAllCheckboxID) {
			if (((ScriptedCheckBoxControlClass*)Control)->Is_Checked()) {
				((ScriptedCheckBoxControlClass*)Dialog->Find_Control(LoopOffCheckboxID))->Set_Checked(false);
				((ScriptedCheckBoxControlClass*)Dialog->Find_Control(LoopOneCheckboxID))->Set_Checked(false);
				if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Get_Owner())) {
					Data->Set_LoopMode(1);
				}
			}
			else {
				((ScriptedCheckBoxControlClass*)Control)->Set_Checked(true);
			}
		}
		else if (Control->Get_Control_ID() == LoopOneCheckboxID) {
			if (((ScriptedCheckBoxControlClass*)Control)->Is_Checked()) {
				((ScriptedCheckBoxControlClass*)Dialog->Find_Control(LoopOffCheckboxID))->Set_Checked(false);
				((ScriptedCheckBoxControlClass*)Dialog->Find_Control(LoopAllCheckboxID))->Set_Checked(false);
				if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Get_Owner())) {
					Data->Set_LoopMode(2);
				}
			}
			else {
				((ScriptedCheckBoxControlClass*)Control)->Set_Checked(true);
			}
		}
	}
}

void EssentialsJukeboxDialogPlayerObserverClass::Timer_Expired(int Number, unsigned Data) {
	if (Number == 1000) {
		if (ScriptedDialogClass* Dialog = Find_Dialog(DialogID)) {
			if (EssentialsPlayerDataClass* Data = EssentialsEventClass::Instance->Get_Player_Data(Get_Owner())) {
				JukeboxEnabled = !Data->Get_IsStopped();
				if (ScriptedButtonControlClass* StartStopButton = (ScriptedButtonControlClass*)Dialog->Find_Control(StartStopButtonID)) {
					StartStopButton->Set_Button_Text(JukeboxEnabled ? L"Stop" : L"Start");
				}
				if (ScriptedButtonControlClass* ShuffleButton = (ScriptedButtonControlClass*)Dialog->Find_Control(ShuffleButtonID)) {
					if (ShuffleButton->Is_Enabled() != (JukeboxEnabled && EssentialsJukeboxClass::Instance->EnableShuffle))
						ShuffleButton->Set_Enabled(JukeboxEnabled && EssentialsJukeboxClass::Instance->EnableShuffle);
				}
				int CheckboxID = 0;
				switch(Data->Get_LoopMode()) {
					case 0: CheckboxID = LoopOffCheckboxID; break;
					case 1: CheckboxID = LoopAllCheckboxID; break;
					case 2: CheckboxID = LoopOneCheckboxID; break;
					default: break;
				}
				if (CheckboxID) {
					if (ScriptedCheckBoxControlClass* LoopCheckbox = (ScriptedCheckBoxControlClass*)Dialog->Find_Control(CheckboxID)) {
						if (!LoopCheckbox->Is_Checked())
							LoopCheckbox->Set_Checked(true);
					}
				}
				if (EssentialsJukeboxMusic* Music = Data->Get_CurrentMusic()) {
					if (ScriptedLabelControlClass* CurrentlyPlayingLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(CurrentlyPlayingLabelID)) {
						CurrentlyPlayingLabel->Set_Label_Text(WideStringClass(Music->Name).Peek_Buffer());
					}
					if (ScriptedLabelControlClass* DurationLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(MusicDurationLabelID)) {
						DurationLabel->Set_Label_Text(WideStringClass(Format_Seconds((int)Music->Duration)).Peek_Buffer());
					}

					clock_t startTime = Data->Get_MusicStartTime();
					clock_t endTime = startTime + (clock_t)(Music->Duration * 1000);

					if (ScriptedLabelControlClass* RemainingLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(TimeElapsedLabelID)) {
						RemainingLabel->Set_Label_Text(WideStringClass(Format_Seconds((clock() - startTime) / 1000)).Peek_Buffer());
					}
					if (ScriptedProgressBarControlClass* ProgressBar = (ScriptedProgressBarControlClass*)Dialog->Find_Control(ProgressBarID)) {
						ProgressBar->Set_Minimum(0);
						ProgressBar->Set_Maximum(endTime - startTime);
						ProgressBar->Set_Progress(clock() - startTime);
					}
				} else {
					if (ScriptedLabelControlClass* CurrentlyPlayingLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(CurrentlyPlayingLabelID)) {
						CurrentlyPlayingLabel->Set_Label_Text(L"");
					}
					if (ScriptedLabelControlClass* DurationLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(MusicDurationLabelID)) {
						DurationLabel->Set_Label_Text(L"");
					}
					if (ScriptedLabelControlClass* RemainingLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(TimeElapsedLabelID)) {
						RemainingLabel->Set_Label_Text(L"");
					}
					if (ScriptedProgressBarControlClass* ProgressBar = (ScriptedProgressBarControlClass*)Dialog->Find_Control(ProgressBarID)) {
						ProgressBar->Set_Minimum(0);
						ProgressBar->Set_Maximum(1);
						ProgressBar->Set_Progress(0);
					}
				}
				if (!Data->Get_IsStopped()) {
					if (ScriptedLabelControlClass* NextUpLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(NextUpLabelID)) {
						NextUpLabel->Set_Label_Text(Data->Peek_Next_Music() ? WideStringClass(Data->Peek_Next_Music()->Name).Peek_Buffer() : L"");
					}
				} else {
					if (ScriptedLabelControlClass* NextUpLabel = (ScriptedLabelControlClass*)Dialog->Find_Control(NextUpLabelID)) {
						NextUpLabel->Set_Label_Text(L"");
					}
				}
			}
		}
	}
}

void EssentialsJukeboxDialogPlayerObserverClass::Create_Dialog() {
	ScriptedPopupDialogClass* Popup = Create_Centered_Popup(Get_ID(), 250, 123, L"Jukebox");
	DialogID = Popup->Get_Dialog_ID();

	CurrentlyPlayingDescriptionLabelID = Create_Label_Control(Popup, 5, 5, 115, 8, L"Now Playing: ")->Get_Control_ID();
	CurrentlyPlayingLabelID = Create_Label_Control(Popup, 8, 13, 112, 16, L"")->Get_Control_ID();

	NextUpDescriptionLabelID = Create_Label_Control(Popup, 130, 5, 115, 8, L"Next Up: ")->Get_Control_ID();
	NextUpLabelID = Create_Label_Control(Popup, 133, 13, 112, 16, L"")->Get_Control_ID();

	TimeElapsedLabelID = Create_Label_Control(Popup, 5, 35, 120, 8, L"00:00")->Get_Control_ID();
	auto MusicDurationLabel = Create_Label_Control(Popup, 125, 35, 120, 8, L"00:00");
	MusicDurationLabel->Set_Orientation(TEXTORIENTATION_RIGHT);
	MusicDurationLabelID = MusicDurationLabel->Get_Control_ID();
	ProgressBarID = Create_ProgressBar_Control(Popup, 5, 42, 240, 12, 0, 100)->Get_Control_ID();

	StartStopButtonID = Create_Bordered_Button_Control(Popup, 5, 61, 90, 12, L"Start / Stop")->Get_Control_ID();
	NextButtonID = Create_Bordered_Button_Control(Popup, 5, 76, 90, 12, L"Next")->Get_Control_ID();
	ShuffleButtonID = Create_Bordered_Button_Control(Popup, 5, 91, 90, 12, L"Shuffle")->Get_Control_ID();
	CloseButtonID = Create_Bordered_Button_Control(Popup, 5, 106, 90, 12, L"Close")->Get_Control_ID();

	Create_Label_Control(Popup, 102, 61, 143, 8, L"Selected Loop Mode: ");
	LoopOffCheckboxID = Create_CheckBox_Control(Popup, 102, 71, 143, 10, L"Don't loop.")->Get_Control_ID();
	LoopAllCheckboxID = Create_CheckBox_Control(Popup, 102, 83, 143, 10, L"Loop the playlist.")->Get_Control_ID();
	LoopOneCheckboxID = Create_CheckBox_Control(Popup, 102, 95, 143, 10, L"Loop the currently playing music.")->Get_Control_ID();

	Show_Dialog(Popup);
}

void EssentialsJukeboxDialogPlayerObserverClass::Destroy_Dialog() {
	if (ScriptedDialogClass* Dialog = Find_Dialog(DialogID)) {
		Delete_Dialog(Dialog);
	}
}
