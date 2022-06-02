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
#include "da_log.h"
#include "da_settings.h"
// Custom
#include "EssentialsCustomPowerUpSpawners.h"

EssentialsCustomPowerUpSpawnersClass* EssentialsCustomPowerUpSpawnersClass::Instance = 0;

RENEGADE_FUNCTION
void ScriptableGameObj::Start_Observers()
AT2(0x006B6C40, 0x006B64E0);

EssentialsCustomSpawnerClass::~EssentialsCustomSpawnerClass() {
	if (CurrentPowerUp) {
		CurrentPowerUp->Set_Delete_Pending();
	}
}

int EssentialsCustomSpawnerClass::Select_Preset() {
	int random = Commands->Get_Random_Int(0, PresetList.Count());
	return PresetList[random];
}

GameObject* EssentialsCustomSpawnerClass::Spawn() {
	if (!Can_Spawn()) {
		return 0;
	}
	
	PowerUpGameObjDef* PowerUpDef = (PowerUpGameObjDef*)Find_Definition(Select_Preset());
	PowerUpGameObj* PowerUp = (PowerUpGameObj*)PowerUpDef->Create();
	PowerUp->Set_Position(Position);
	PowerUp->Start_Observers();
	CurrentPowerUp = PowerUp;
	return PowerUp;
}



EssentialsCustomPowerUpSpawnersClass::EssentialsCustomPowerUpSpawnersClass() {
	Instance = this;

	Register_Event(DAEvent::SETTINGSLOADED, INT_MAX);
	Register_Event(DAEvent::POWERUPGRANT, INT_MAX);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Custom Power-Up Spawners feature.");
}

EssentialsCustomPowerUpSpawnersClass::~EssentialsCustomPowerUpSpawnersClass() {
	Instance = 0;

	Clear_Spawners();

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Custom Power-Up Spawners feature.");
}

void EssentialsCustomPowerUpSpawnersClass::Settings_Loaded_Event() {
	Clear_Spawners();

	for (int i = 1;; ++i) {
		Vector3 Location;
		DASettingsManager::Get_Vector3(Location, "EssentialsCustomPowerUpSpawners", StringFormat("PowerUpSpawner%d", i), Location);
		if (Location.X || Location.Y || Location.Z) {
			EssentialsCustomSpawnerClass* Spawner = new EssentialsCustomSpawnerClass;
			Spawner->ID = i;
			Spawner->Position = Location;
			Spawner->SpawnDelay = DASettingsManager::Get_Float("EssentialsCustomPowerUpSpawners", StringFormat("PowerUpSpawner%d_SpawnDelay", i), 60.f);
			Spawner->CurrentPowerUp = 0;

			StringClass Presets;
			DASettingsManager::Get_String(Presets, "EssentialsCustomPowerUpSpawners", StringFormat("PowerUpSpawner%d_PowerUps", i), "");
			DATokenParserClass Token(Presets, '|');
			while (char* Preset = Token.Get_String()) {
				if (DefinitionClass* Def = Find_Named_Definition(Preset)) {
					if (Def->Get_Class_ID() == CID_PowerUp) {
						Spawner->PresetList.Add(Def->Get_ID());
					}
				}
			}

			Spawners.Add_Tail(Spawner);
			Spawner->Spawn();
		} else {
			break;
		}
	}
}

void EssentialsCustomPowerUpSpawnersClass::Timer_Expired(int Number, unsigned int Data) {
	if (Number == 100) {
		Get_Spawner(Data)->Spawn();
	}
}

void EssentialsCustomPowerUpSpawnersClass::PowerUp_Grant_Event(cPlayer* Player, const PowerUpGameObjDef* PowerUp, PowerUpGameObj* PowerUpObj) {
	if (EssentialsCustomSpawnerClass* Spawner = Find_Spawner_By_PowerUp(PowerUpObj)) {
		Start_Timer(100, Spawner->Get_Spawn_Delay(), false, Spawner->Get_ID());
	}
}

EssentialsCustomSpawnerClass* EssentialsCustomPowerUpSpawnersClass::Get_Spawner(int ID) {
	for (SLNode<EssentialsCustomSpawnerClass>* Node = Spawners.Head(); Node; Node = Node->Next()) {
		if (Node->Data()->Get_ID() == ID) {
			return Node->Data();
		}
	}
	return 0;
}

EssentialsCustomSpawnerClass* EssentialsCustomPowerUpSpawnersClass::Find_Spawner_By_PowerUp(PowerUpGameObj* PowerUp) {
	for (SLNode<EssentialsCustomSpawnerClass>* Node = Spawners.Head(); Node; Node = Node->Next()) {
		if (Node->Data()->Get_Current_Object() == PowerUp) {
			return Node->Data();
		}
	}
	return 0;
}

void EssentialsCustomPowerUpSpawnersClass::Clear_Spawners() {
	for (SLNode<EssentialsCustomSpawnerClass>* Node = Spawners.Head(); Node; Node = Node->Next()) {
		Stop_Timer(100, Node->Data()->Get_ID());
		delete Node->Data();
	}
	Spawners.Remove_All();
}
