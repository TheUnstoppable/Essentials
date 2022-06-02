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
#include "engine_common.h"
#include "engine_da.h"
#include "engine_game.h"
#include "engine_player.h"
#include "engine_obj.h"
#include "engine_obj2.h"
// DA
#include "da.h"
#include "da_levelcleaner.h"
#include "da_log.h"
#include "da_settings.h"
#include "da_translation.h"
// Custom
#include "EssentialsFreeForAllGameModeClass.h"

REF_DEF2(DynamicVectorClass<SpawnerClass*>, SpawnerList, 0, 0x008564A8);
REF_DEF2(bool, cGameData::IsManualRestart, 0, 0x0082E30C);

void EssentialsFreeForAllGameModeClass::Init() {
	Register_Event(DAEvent::GAMEOVER, INT_MAX);
	Register_Event(DAEvent::PLAYERJOIN);
	Register_Event(DAEvent::CHARACTERPURCHASEREQUEST, INT_MAX);
	Register_Event(DAEvent::VEHICLEPURCHASEREQUEST, INT_MAX);
	Register_Object_Event(DAObjectEvent::CREATED, DAObjectEvent::SOLDIER);

	const DASettingsClass* Settings = DASettingsManager::Get_Settings(DASettingsManager::Get_Settings_Count() - 1);

	if (tolower(The_Game()->MapName[0]) == 'm') {
		DALevelCleaner::Clean_Level();
		DALevelCleaner::Load_Blockers(Settings->Get_INI());
	}

	Settings->Get_String(PlayerModel, "PlayerModel", "c_ag_gdi_mp");
	PlayerTeam = Settings->Get_Int("PlayerTeam", -1);
	GameOverTeam = Settings->Get_Int("GameOverTeam", 1);
	UseMapSpawners = Settings->Get_Bool("UseMapSpawners", true);
	PointsFix = Settings->Get_Bool("PointsFix", true);

	for (SLNode<cPlayer>* Node = Get_Player_List()->Head(); Node; Node = Node->Next()) {
		cPlayer* Player = Node->Data();
		if (Player && Player->Is_Active()) {
			Player->Set_Player_Type(PlayerTeam);
			Update_Network_Object(Player);
		}
	}

	SoldierSpawnPoints.Clear();

	if (UseMapSpawners) {
		Load_Map_Spawn_Points();
	}

	Load_Config_Spawn_Points(Settings);

	if (SoldierSpawnPoints.Count() == 0) {
		Console_Output("[Essentials] ERROR: No spawn points found, skipping the map...\n");
		cGameData::Set_Manual_Restart(true);
	}
}

void EssentialsFreeForAllGameModeClass::Game_Over_Event() {
	for (SLNode<cPlayer>* Node = Get_Player_List()->Head(); Node; Node = Node->Next()) {
		cPlayer* Player = Node->Data();
		if (Player && Player->Is_Active()) {
			Player->Set_Player_Type(GameOverTeam);
			Update_Network_Object(Player);
		}
	}
}

void EssentialsFreeForAllGameModeClass::Player_Join_Event(cPlayer* Player) {
	Player->Set_Player_Type(!The_Game()->Is_Game_Over() ? PlayerTeam : GameOverTeam);
	Update_Network_Object(Player);
}

void EssentialsFreeForAllGameModeClass::Object_Created_Event(GameObject* obj) {
	if (Commands->Get_Player_Type(obj) != PlayerTeam) {
		if (!The_Game()->Is_Game_Over()) {
			Commands->Set_Player_Type(obj, PlayerTeam);
		} else {
			Commands->Set_Player_Type(obj, GameOverTeam);
		}
	}

	if (PointsFix) {
		Set_Death_Points(obj, -Get_Death_Points(obj));
		Set_Damage_Points(obj, -Get_Damage_Points(obj));
	}

	Vector3 Location = Select_Spawner();
	Commands->Set_Model(obj, PlayerModel);
	Commands->Set_Position(obj, Location);
	Fix_Stuck_Object(obj->As_SoldierGameObj(), 0.3f);
	Update_Network_Object(obj);
}

void EssentialsFreeForAllGameModeClass::Load_Map_Spawn_Points() {
	for (int i = 0; i < SpawnerList.Count(); ++i) {
		SpawnerClass* Spawner = SpawnerList[i];
		const SpawnerDefClass* Definition = Spawner->definition;
		if (Definition && Definition->IsSoldierStartup) {
			SoldierSpawnPoints.Add(Spawner->transform.Get_Translation());
			for (int a = 0; a < Spawner->Get_Spawn_Point_List().Count(); ++a) {
				SoldierSpawnPoints.Add(Spawner->Get_Spawn_Point_List()[a].Get_Translation());
			}
		}
	}
}

void EssentialsFreeForAllGameModeClass::Load_Config_Spawn_Points(const DASettingsClass* Settings) {
	for (int i = 1;; ++i) {
		Vector3 Location;
		Settings->Get_Vector3(Location, StringFormat("FFASpawnPoint%d", i), Location);
		if (Location.X || Location.Y || Location.Z) {
			SoldierSpawnPoints.Add(Location);
		} else {
			break;
		}
	}
}

Vector3 EssentialsFreeForAllGameModeClass::Select_Spawner() {
	if (SoldierSpawnPoints.Count() > 0) {
		int Random = Get_Random_Int(0, SoldierSpawnPoints.Count());
		auto Point = SoldierSpawnPoints[Random];
		return Point;
	} else {
		return Vector3();
	}
}

Register_Game_Mode(EssentialsFreeForAllGameModeClass, "FFA", "Free For All", 0);