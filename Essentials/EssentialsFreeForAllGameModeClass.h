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
#include <SpawnerClass.h>

class EssentialsFreeForAllGameModeClass : public DAGameModeClass, public DAEventClass
{
public:
	virtual void Init();
	virtual void Game_Over_Event();
	virtual void Player_Join_Event(cPlayer* Player);
	virtual PurchaseStatus Character_Purchase_Request_Event(BaseControllerClass* Base, cPlayer* Player, float& Cost, const SoldierGameObjDef* Item) { return PurchaseStatus_OutOfStock; }
	virtual PurchaseStatus Vehicle_Purchase_Request_Event(BaseControllerClass* Base, cPlayer* Player, float& Cost, const VehicleGameObjDef* Item) { return PurchaseStatus_OutOfStock; }
	virtual void Object_Created_Event(GameObject* obj);

private:
	void Load_Map_Spawn_Points();
	void Load_Config_Spawn_Points(const DASettingsClass* Settings);
	void Load_Weapon_Spawners(const DASettingsClass* Settings);

	Vector3 Select_Spawner();

	DynamicVectorClass<Vector3> SoldierSpawnPoints;
	StringClass PlayerModel;
	int PlayerTeam;
	int GameOverTeam;
	bool UseMapSpawners;
	bool PointsFix;
};

