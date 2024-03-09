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
#include "EssentialsVoteStatusPlayerObserverClass.h"
#include "EssentialsPlayerDataClass.h"
#include "EssentialsUtils.h"

void EssentialsVoteStatusPlayerObserverClass::Init() {
	PollContext = 0;
	PollEnded = false;
	Create_Surface();
}

EssentialsVoteStatusPlayerObserverClass::~EssentialsVoteStatusPlayerObserverClass() {
	if (Find_Player(Get_ID())) {
		Destroy_Surface();
	}
}

void EssentialsVoteStatusPlayerObserverClass::On_Poll_Start(EssentialsPollClass* poll) {
	PollContext = poll;
	PollEnded = false;
	Update_Data();
}

void EssentialsVoteStatusPlayerObserverClass::On_Poll_End(bool passed) {
	if (HUDSurfaceClass* Surface = Find_HUD_Surface(SurfaceID)) {
		PollEnded = true;
		if (HUDTextElementClass* Title = (HUDTextElementClass*)Surface->Find_Element(TitleElementID)) {
			Title->Set_Text(passed ? L"Poll PASSED" : L"Poll FAILED");
		}
	}
}

const RectClass BOX_BG(0.79f, 0.36f, 0.997f, 0.64f);
const RectClass BOX_TITLE(0.02f, 0.02f, 0.98f, 0.18f);
const RectClass BOX_DESC(0.02f, 0.18f, 0.98f, 0.28f);
const RectClass BOX_YESTEXT(0.02f, 0.30f, 0.98f, 0.38f);
const RectClass BOX_YES(0.02f, 0.41f, 0.98f, 0.48f);
const RectClass BOX_NOTEXT(0.02f, 0.53f, 0.98f, 0.61f);
const RectClass BOX_NO(0.02f, 0.64f, 0.98f, 0.71f);
const RectClass BOX_TIMETEXT(0.02f, 0.78f, 0.98f, 0.86f);
const RectClass BOX_TIME(0.02f, 0.89f, 0.98f, 0.97f);

void EssentialsVoteStatusPlayerObserverClass::Create_Surface() {
	HUDSurfaceClass* Surface = Create_HUD_Surface(Get_ID());
	SurfaceID = Surface->Get_Surface_ID();

	RectClass bgRect(BOX_BG);
	bgRect.Scale(Vector2(Surface->Get_Boundary_Area().Width(), Surface->Get_Boundary_Area().Height()));
	bgRect.Snap_To_Units(Vector2(1,1));

	RectClass titleRect(BOX_TITLE);
	titleRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	titleRect.Snap_To_Units(Vector2(1,1));
	titleRect.Left += bgRect.Left;
	titleRect.Top += bgRect.Top;
	titleRect.Right += bgRect.Left;
	titleRect.Bottom += bgRect.Top;

	RectClass descRect(BOX_DESC);
	descRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	descRect.Snap_To_Units(Vector2(1,1));
	descRect.Left += bgRect.Left;
	descRect.Top += bgRect.Top;
	descRect.Right += bgRect.Left;
	descRect.Bottom += bgRect.Top;

	RectClass yesTextRect(BOX_YESTEXT);
	yesTextRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	yesTextRect.Snap_To_Units(Vector2(1, 1));
	yesTextRect.Left += bgRect.Left;
	yesTextRect.Top += bgRect.Top;
	yesTextRect.Right += bgRect.Left;
	yesTextRect.Bottom += bgRect.Top;

	RectClass yesRect(BOX_YES);
	yesRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	yesRect.Snap_To_Units(Vector2(1,1));
	yesRect.Left += bgRect.Left;
	yesRect.Top += bgRect.Top;
	yesRect.Right += bgRect.Left;
	yesRect.Bottom += bgRect.Top;

	RectClass noTextRect(BOX_NOTEXT);
	noTextRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	noTextRect.Snap_To_Units(Vector2(1, 1));
	noTextRect.Left += bgRect.Left;
	noTextRect.Top += bgRect.Top;
	noTextRect.Right += bgRect.Left;
	noTextRect.Bottom += bgRect.Top;

	RectClass noRect(BOX_NO);
	noRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	noRect.Snap_To_Units(Vector2(1,1));
	noRect.Left += bgRect.Left;
	noRect.Top += bgRect.Top;
	noRect.Right += bgRect.Left;
	noRect.Bottom += bgRect.Top;

	RectClass timeTextRect(BOX_TIMETEXT);
	timeTextRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	timeTextRect.Snap_To_Units(Vector2(1, 1));
	timeTextRect.Left += bgRect.Left;
	timeTextRect.Top += bgRect.Top;
	timeTextRect.Right += bgRect.Left;
	timeTextRect.Bottom += bgRect.Top;

	RectClass timeRect(BOX_TIME);
	timeRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
	timeRect.Snap_To_Units(Vector2(1,1));
	timeRect.Left += bgRect.Left;
	timeRect.Top += bgRect.Top;
	timeRect.Right += bgRect.Left;
	timeRect.Bottom += bgRect.Top;

	HUDRectangleElementClass* Background = (HUDRectangleElementClass*)Surface->Create_Element(HUD_ELEMENT_RECTANGLE);
	Background->Set_Rect(bgRect);
	Background->Set_Color(127, 0, 0, 0);

	//HUDRectangleElementClass* TitleBackground = (HUDRectangleElementClass*)Surface->Create_Element(HUD_ELEMENT_RECTANGLE);
	//TitleBackground->Set_Rect(titleRect);
	//TitleBackground->Set_Color(255, 0, 0, 0);

	titleRect.Left += 2; titleRect.Right -= 2;
	titleRect.Top += 2; titleRect.Bottom -= 2;

	HUDTextElementClass* Title = (HUDTextElementClass*)Surface->Create_Element(HUD_ELEMENT_TEXT);
	Title->Set_Clipping_Area(titleRect);
	Title->Set_Color(255, 255, 255, 255);
	Title->Set_Font(HUD_FONT_BIG_BODY);
	Title->Set_Text(L"Poll to");
	TitleElementID = Title->Get_Element_ID();

	//HUDRectangleElementClass* DescBackground = (HUDRectangleElementClass*)Surface->Create_Element(HUD_ELEMENT_RECTANGLE);
	//DescBackground->Set_Rect(descRect);
	//DescBackground->Set_Color(255, 0, 0, 0);

	descRect.Left += 2; descRect.Right -= 2;
	descRect.Top += 2; descRect.Bottom -= 2;

	HUDTextElementClass* Desc = (HUDTextElementClass*)Surface->Create_Element(HUD_ELEMENT_TEXT);
	Desc->Set_Clipping_Area(descRect);
	Desc->Set_Color(255, 255, 255, 255);
	Desc->Set_Font(HUD_FONT_NORMAL_BODY);
	Desc->Set_Text(L"Loading...");
	DescriptionElementID = Desc->Get_Element_ID();

	HUDRectangleElementClass* YesBackground = (HUDRectangleElementClass*)Surface->Create_Element(HUD_ELEMENT_RECTANGLE);
	YesBackground->Set_Rect(yesRect);
	YesBackground->Set_Color(255, 0, 127, 0);
	YesRectID = YesBackground->Get_Element_ID();

	HUDTextElementClass* YesText = (HUDTextElementClass*)Surface->Create_Element(HUD_ELEMENT_TEXT);
	YesText->Set_Clipping_Area(yesTextRect);
	YesText->Set_Color(255, 255, 255, 255);
	YesText->Set_Font(HUD_FONT_SMALL_HEADER);
	YesText->Set_Text(L"Yes: 0 Votes (0%)");
	YesTextID = YesText->Get_Element_ID();

	HUDRectangleElementClass* NoBackground = (HUDRectangleElementClass*)Surface->Create_Element(HUD_ELEMENT_RECTANGLE);
	NoBackground->Set_Rect(noRect);
	NoBackground->Set_Color(255, 200, 0, 0);
	NoRectID = NoBackground->Get_Element_ID();

	HUDTextElementClass* NoText = (HUDTextElementClass*)Surface->Create_Element(HUD_ELEMENT_TEXT);
	NoText->Set_Clipping_Area(noTextRect);
	NoText->Set_Color(255, 255, 255, 255);
	NoText->Set_Font(HUD_FONT_SMALL_HEADER);
	NoText->Set_Text(L"No: 0 Votes (0%)");
	NoTextID = NoText->Get_Element_ID();

	HUDRectangleElementClass* TimeBackground = (HUDRectangleElementClass*)Surface->Create_Element(HUD_ELEMENT_RECTANGLE);
	TimeBackground->Set_Rect(timeRect);
	TimeBackground->Set_Color(255, 127, 127, 255);
	TimeRectID = TimeBackground->Get_Element_ID();

	HUDTextElementClass* TimeText = (HUDTextElementClass*)Surface->Create_Element(HUD_ELEMENT_TEXT);
	TimeText->Set_Clipping_Area(timeTextRect);
	TimeText->Set_Color(255, 255, 255, 255);
	TimeText->Set_Font(HUD_FONT_SMALL_HEADER);
	TimeText->Set_Text(L"Loading...");
	TimeTextID = TimeText->Get_Element_ID();

	bgRect.Left -= 2; bgRect.Right += 2; bgRect.Top -= 2; bgRect.Bottom += 2;
	HUDOutlineElementClass* BGOutline = (HUDOutlineElementClass*)Surface->Create_Element(HUD_ELEMENT_OUTLINE);
	BGOutline->Set_Color(255, 255, 255, 255);
	BGOutline->Set_Thickness(3);
	BGOutline->Set_Rect(bgRect);

	yesRect.Left -= 2; yesRect.Right += 2; yesRect.Top -= 2; yesRect.Bottom += 2;
	HUDOutlineElementClass* YesOutline = (HUDOutlineElementClass*)Surface->Create_Element(HUD_ELEMENT_OUTLINE);
	YesOutline->Set_Color(255, 255, 255, 255);
	YesOutline->Set_Thickness(2);
	YesOutline->Set_Rect(yesRect);

	noRect.Left -= 2; noRect.Right += 2; noRect.Top -= 2; noRect.Bottom += 2;
	HUDOutlineElementClass* NoOutline = (HUDOutlineElementClass*)Surface->Create_Element(HUD_ELEMENT_OUTLINE);
	NoOutline->Set_Color(255, 255, 255, 255);
	NoOutline->Set_Thickness(2);
	NoOutline->Set_Rect(noRect);

	timeRect.Left -= 2; timeRect.Right += 2; timeRect.Top -= 2; timeRect.Bottom += 2;
	HUDOutlineElementClass* TimeOutline = (HUDOutlineElementClass*)Surface->Create_Element(HUD_ELEMENT_OUTLINE);
	TimeOutline->Set_Color(255, 255, 255, 255);
	TimeOutline->Set_Thickness(2);
	TimeOutline->Set_Rect(timeRect);

	Show_HUD_Surface(Surface);
}

void EssentialsVoteStatusPlayerObserverClass::Destroy_Surface() {
	if (HUDSurfaceClass* Surface = Find_HUD_Surface(SurfaceID))
		Delete_HUD_Surface(Surface);
}

void EssentialsVoteStatusPlayerObserverClass::Update_Data() {
	if (!PollContext) return;
	if (PollEnded) return;

	if (HUDSurfaceClass* Surface = Find_HUD_Surface(SurfaceID)) {
		RectClass bgRect(BOX_BG);
		bgRect.Scale(Vector2(Surface->Get_Boundary_Area().Width(), Surface->Get_Boundary_Area().Height()));
		bgRect.Snap_To_Units(Vector2(1, 1));

		HUDTextElementClass* Desc = (HUDTextElementClass*)Surface->Find_Element(DescriptionElementID);
		if (Desc) {
			Desc->Set_Text(Get_Action_Description());
		}

		int yes = PollContext->Get_Yes_Count(), no = PollContext->Get_No_Count();
		float yes_r = (float)yes / (yes + no), no_r = (float)no / (yes + no);

		HUDRectangleElementClass* Yes = (HUDRectangleElementClass*)Surface->Find_Element(YesRectID);
		if (Yes) {
			RectClass yesRect(BOX_YES);
			yesRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
			yesRect.Snap_To_Units(Vector2(1, 1));
			yesRect.Left += bgRect.Left;
			yesRect.Top += bgRect.Top;
			yesRect.Right += bgRect.Left;
			yesRect.Bottom += bgRect.Top;
			yesRect.Right = yesRect.Left + (yesRect.Width() * yes_r);
			Yes->Set_Rect(yesRect);
		}

		HUDTextElementClass* YesText = (HUDTextElementClass*)Surface->Find_Element(YesTextID);
		if (YesText) {
			YesText->Set_Text(WideStringFormat(L"Yes: %d vote%s (%d%%)", yes, yes == 1 ? L"" : L"s", yes + no == 0 ? 0 : (int)((float)yes / (yes + no) * 100)));
		}

		HUDRectangleElementClass* No = (HUDRectangleElementClass*)Surface->Find_Element(NoRectID);
		if (No) {
			RectClass noRect(BOX_NO);
			noRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
			noRect.Snap_To_Units(Vector2(1, 1));
			noRect.Left += bgRect.Left;
			noRect.Top += bgRect.Top;
			noRect.Right += bgRect.Left;
			noRect.Bottom += bgRect.Top;
			noRect.Right = noRect.Left + (noRect.Width() * no_r);
			No->Set_Rect(noRect);
		}

		HUDTextElementClass* NoText = (HUDTextElementClass*)Surface->Find_Element(NoTextID);
		if (NoText) {
			NoText->Set_Text(WideStringFormat(L"No: %d vote%s (%d%%)", no, no == 1 ? L"" : L"s", yes + no == 0 ? 0 : (int)((float)no / (yes + no) * 100)));
		}

		HUDRectangleElementClass* Time = (HUDRectangleElementClass*)Surface->Find_Element(TimeRectID);
		if (Time) {
			int duration = PollContext->Is_Forced() ? (int)EssentialsVotingManagerClass::Instance->ForcedPollDuration : (int)EssentialsVotingManagerClass::Instance->PollDuration;
			int elapsed = (clock() - PollContext->Get_Start_Time()) / 1000;
			int remaining = duration - elapsed;

			RectClass timeRect(BOX_TIME);
			timeRect.Scale(Vector2(bgRect.Width(), bgRect.Height()));
			timeRect.Snap_To_Units(Vector2(1, 1));
			timeRect.Left += bgRect.Left;
			timeRect.Top += bgRect.Top;
			timeRect.Right += bgRect.Left;
			timeRect.Bottom += bgRect.Top;
			timeRect.Right = timeRect.Left + (timeRect.Width() * ((float)remaining / duration));
			Time->Set_Rect(timeRect);

			bool percentage = ((float)(yes + no) >= ((EssentialsVotingManagerClass::Instance->RequiredPercentageToPass / 100.f) * The_Game()->Get_Current_Players()));
			if (percentage) {
				Time->Set_Color(255, 127, 127, 255);
			} else {
				Time->Set_Color(255, 255, 127, 0);
			}
		}

		HUDTextElementClass* TimeText = (HUDTextElementClass*)Surface->Find_Element(TimeTextID);
		if (TimeText) {
			int duration = PollContext->Is_Forced() ? (int)EssentialsVotingManagerClass::Instance->ForcedPollDuration : (int)EssentialsVotingManagerClass::Instance->PollDuration;
			int elapsed = (clock() - PollContext->Get_Start_Time()) / 1000;
			int remaining = duration - elapsed;
			TimeText->Set_Text(WideStringFormat(L"Poll ends in: %d second%s", remaining, remaining == 1 ? L"" : L"s"));
		}
	}
}
