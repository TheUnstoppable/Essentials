/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "da_gamemode.h"
#include "da_event.h"

class EssentialsSnipingGameModeClass : public DAGameModeClass, public DAEventClass
{
public:
	void Init() override;
	~EssentialsSnipingGameModeClass();

	void Settings_Loaded_Event() override;
	void Game_Over_Event() override;
	void Object_Created_Event(GameObject* obj) override;
	void Kill_Event(DamageableGameObj* Victim, ArmedGameObj* Killer, float Damage, unsigned int Warhead, float Scale, DADamageType::Type Type) override;
	void Poke_Event(cPlayer* Player, PhysicalGameObj* obj) override;
	void Timer_Expired(int Number, unsigned Data) override;

	void Reset();

private:
	bool Cleaned;

	float SpawnMoney;
	float MoneyPerKill;
	float MoneyIncrementPerPlayer;
	bool DisableScreenEffects;
	float Prices[2];
	SoldierGameObjDef* SniperClasses[2][2]; // Basic, Advanced
	PhysicalGameObj* AdvancedTerminals[2][2]; // Basic, Advanced
	AABoxClass BaseZones[2][2]; // Warning, Kill
	Vector3 Spawners[2];
};