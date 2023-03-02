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
#include "da_cratemanager.h"

class EssentialsCrateManager : public DACrateManager {
	struct CustomPowerupClass {
		StringClass Preset;
		StringClass Model;
		int Odds;
		bool DoCrateBehavior;

		int CalculatedOdds;
	};
public:
	void Init() override;
	~EssentialsCrateManager();
	void Settings_Loaded_Event() override;
	void Game_Over_Event() override;
	void Timer_Expired(int Number, unsigned int Data) override;
	void PowerUp_Grant_Event(cPlayer* Player, const PowerUpGameObjDef* PowerUp, PowerUpGameObj* PowerUpObj) override;
	void Object_Destroyed_Event(GameObject* obj) override;
	
	bool SCDCommand(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
protected:
	void Delete_Customs();
	void Delete_Objects();
private:
	bool InjectCrates;
	int TotalOdds;
	DynamicVectorClass<CustomPowerupClass*> CustomPowerups;
	DynamicVectorClass<PhysicalGameObj*> CustomObjs;
};