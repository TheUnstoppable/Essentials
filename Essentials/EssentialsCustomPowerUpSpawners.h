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

class EssentialsCustomSpawnerClass {
	friend class EssentialsCustomPowerUpSpawnersClass;

public:
	virtual ~EssentialsCustomSpawnerClass();

	int Get_ID() { return ID; }
	Vector3 Get_Position() { return Position; }
	float Get_Spawn_Delay() { return SpawnDelay; }
	DynamicVectorClass<int>& Get_Preset_List() { return PresetList; }
	GameObject* Get_Current_Object() { return CurrentPowerUp; }

	bool Can_Spawn() { return !CurrentPowerUp && !!PresetList.Count(); }
	int Select_Preset();
	GameObject* Spawn();

private:
	int ID;
	Vector3 Position;
	float SpawnDelay;
	DynamicVectorClass<int> PresetList;
	ReferencerClass CurrentPowerUp;
};

class EssentialsCustomPowerUpSpawnersClass : public DAEventClass {
public:
	static EssentialsCustomPowerUpSpawnersClass* Instance;

	EssentialsCustomPowerUpSpawnersClass();
	~EssentialsCustomPowerUpSpawnersClass();

	virtual void Settings_Loaded_Event();
	virtual void Timer_Expired(int Number, unsigned int Data);
	virtual void PowerUp_Grant_Event(cPlayer* Player, const PowerUpGameObjDef* PowerUp, PowerUpGameObj* PowerUpObj);

protected:
	EssentialsCustomSpawnerClass* Get_Spawner(int ID);
	EssentialsCustomSpawnerClass* Find_Spawner_By_PowerUp(PowerUpGameObj* PowerUp);
	void Clear_Spawners();

private:
	SList<EssentialsCustomSpawnerClass> Spawners;
};

