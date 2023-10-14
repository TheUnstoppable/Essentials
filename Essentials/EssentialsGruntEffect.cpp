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
#include "engine_da.h"
// DA
#include "da.h"
#include "da_log.h"
// Custom
#include "EssentialsEventClass.h"
#include "EssentialsGruntEffect.h"

EssentialsGruntEffectClass* EssentialsGruntEffectClass::Instance = 0;

EssentialsGruntEffectClass::EssentialsGruntEffectClass() {
	Instance = this;

	Register_Object_Event(DAObjectEvent::DAMAGERECEIVED, DAObjectEvent::PLAYER);

	DALogManager::Write_Log("_ESSENTIALS", "Loaded Grunt Effect feature.");
}

EssentialsGruntEffectClass::~EssentialsGruntEffectClass() {
	Instance = 0;

	DALogManager::Write_Log("_ESSENTIALS", "Unloaded Grunt Effect feature.");
}

void EssentialsGruntEffectClass::Damage_Event(DamageableGameObj* Victim, ArmedGameObj* Damager, float Damage, unsigned Warhead, float Scale, DADamageType::Type Type) {
	if (Victim->As_SoldierGameObj() && Damage > 0) {
		if (cPlayer* Player = ((SoldierGameObj*)Victim)->Get_Player()) {
			if (!Is_Timer(459001, Player->Get_Id())) {
				DA::Create_2D_Sound_Player(Player, "grunt_grunt.wav");
				Start_Timer(459001, 2.5f, false, Player->Get_Id());
			}

			if (!Is_Timer(459002, Player->Get_Id())) {
				Vector3 Pos = Commands->Get_Position(Victim);
				Pos.Z += 1.75f;
				Commands->Shake_Camera(Pos, 3.9f, .05f, .2f);
				Start_Timer(459002, .5f, false, Player->Get_Id());
			}
		}
	}
}

void EssentialsGruntEffectClass::Timer_Expired(int Number, unsigned Data) {
	
}
