#pragma once

#include "EssentialsPollClass.h"
#include "da_player.h"

class EssentialsVoteStatusPlayerObserverClass : public DAPlayerObserverClass {
private:
	int SurfaceID;
	int TitleElementID;
	int DescriptionElementID;
	int YesRectID;
	int NoRectID;
	int TimeRectID;
	int YesTextID;
	int NoTextID;
	int TimeTextID;

	EssentialsPollClass* PollContext;
	bool PollEnded;

public:
	void Init() override;
	virtual ~EssentialsVoteStatusPlayerObserverClass();
	const char* Get_Observer_Name() { return "EssentialsVoteStatusPlayerObserverClass"; }

	inline WideStringClass Get_Action_Description() const {
		if (!PollContext) return L"N/A";
		switch (PollContext->Get_Type()) {
			case VOTE_TIME: return L"add more time";
			case VOTE_GAMEOVER: return L"end current game";
			case VOTE_RESTARTMAP: return L"restart current map";
			case VOTE_NEXTMAP: return WideStringFormat(L"change next to %hs", PollContext->Get_Data());
			case VOTE_PLAYERKICK: {
				const char* pName = Get_Player_Name_By_ID(atoi(PollContext->Get_Data()));
				WideStringClass returnStr = WideStringFormat(L"kick player %hs", pName);
				delete[] pName;
				return returnStr;
			}
			case VOTE_NOREPAIR: return L"disable building repairs";

			case VOTE_NONE:
			default: return L"do nothing";
		}
	}

	void On_Poll_Start(EssentialsPollClass* poll);
	void On_Poll_End(bool passed);

	void Create_Surface();
	void Destroy_Surface();

	void Update_Data();
};

