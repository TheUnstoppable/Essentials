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
#include "engine_common.h"
#include "engine_game.h"
#include "engine_player.h"
#include "engine_obj.h"
#include "engine_obj2.h"
// DA
#include "da.h"
#include "da_settings.h"
// Custom
#include "EssentialsUtils.h"
#include "EssentialsEventClass.h"

bool Make_Spectator(cPlayer* Player) {
	SoldierGameObj *Soldier = Player->Get_GameObj();
	if (Get_Object_Type(Soldier) != -4) {
		if (!Soldier->Get_Vehicle()) {
			if (!Get_Fly_Mode(Soldier)) {
				Toggle_Fly_Mode(Soldier);
			}
			Commands->Set_Model(Soldier, "NULL");
			Commands->Clear_Weapons(Soldier);
			Commands->Disable_All_Collisions(Soldier);
			Commands->Set_Is_Visible(Soldier, false);
			Commands->Set_Is_Rendered(Soldier, false);
			
			Attach_Script_Once(Soldier, "KAK_Prevent_Kill", "");
			Disarm_All_C4_Beacons(Get_Player_ID(Soldier));
			Set_Object_Type(Soldier, -4);
			Lock_Soldier_Collision_Group(Soldier, UNCOLLIDEABLE_GROUP);

			Soldier->Set_Max_Speed(EssentialsEventClass::Instance->SpectateSpeed);
			
			for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
				cPlayer *p = z->Data();
				if (p->Is_Active()) {
					if (p->Get_Id() == Get_Player_ID(Soldier)) {
						Update_Network_Object_Player(Soldier, p->Get_Id());
					}
					else {
						bool isPending = Soldier->Is_Delete_Pending();
						Soldier->Set_Is_Delete_Pending(true);
						Update_Network_Object_Player(Soldier, p->Get_Id());
						Soldier->Set_Is_Delete_Pending(isPending);
					}
				}
			}
			Soldier->Clear_Object_Dirty_Bits();
			return true;
		}
	}
	return false;
}

GameObject* Spawn_Object(cPlayer *Player, unsigned long DefinitionID) {
	DefinitionClass* Definition = Find_Definition(DefinitionID);
	if (Definition) {
		if (Definition->Get_Class_ID() == CID_Soldier || Definition->Get_Class_ID() == CID_PowerUp || Definition->Get_Class_ID() == CID_Vehicle || Definition->Get_Class_ID() == CID_Simple) {
			SoldierGameObj* Soldier = Player->Get_GameObj();
			Vector3 Position = Commands->Get_Position(Soldier);
			if (Definition->Get_Class_ID() == CID_PowerUp) {
				float Facing = Commands->Get_Facing(Soldier);
				Position.X += 2.5f * cos(DEG2RAD(Facing));
				Position.Y += 2.5f * sin(DEG2RAD(Facing));
				Position.Z += 1.5f;
			}

			PhysicalGameObj* Object = Create_Object(Definition, Position);
			if (!Object->As_PowerUpGameObj()) {
				Fix_Stuck_Object(Soldier, 5.f);
				if (Object->Peek_Physical_Object()->As_MoveablePhysClass()) {
					Fix_Stuck_Object(Object, 5.f);
				}
			}

			return Object;
		}
	}

	return 0;
}

bool Teleport_Player(cPlayer* Player, cPlayer* Target) {
	PhysicalGameObj* SourceObject = 0;
	PhysicalGameObj* TargetObject = 0;
	
	if (Player->Is_Active()) {
		if (Player->Get_GameObj()->Get_Vehicle()) {
			SourceObject = Player->Get_GameObj()->Get_Vehicle();
		} else {
			SourceObject = Player->Get_GameObj();
		}
	}

	if (Target->Is_Active()) {
		if (Target->Get_GameObj()->Get_Vehicle()) {
			TargetObject = Target->Get_GameObj()->Get_Vehicle();
		}
		else {
			TargetObject = Target->Get_GameObj();
		}
	}

	if (SourceObject && TargetObject) {
		Commands->Set_Position(SourceObject, Commands->Get_Position(TargetObject));
		if (!SourceObject->Peek_Physical_Object()->Is_Immovable()) {
			Fix_Stuck_Object(SourceObject, 5.f);
		}
		if (!TargetObject->Peek_Physical_Object()->Is_Immovable()) {
			Fix_Stuck_Object(TargetObject, 5.f);
		}
		return true;
	}

	return false;
}

bool Teleport_Player(cPlayer* Player, PhysicalGameObj* Object) {
	PhysicalGameObj* SourceObject = 0;

	if (Player->Is_Active()) {
		if (Player->Get_GameObj()->Get_Vehicle()) {
			SourceObject = Player->Get_GameObj()->Get_Vehicle();
		}
		else {
			SourceObject = Player->Get_GameObj();
		}
	}

	if (SourceObject && Object) {
		Commands->Set_Position(SourceObject, Commands->Get_Position(Object));
		if (!SourceObject->Peek_Physical_Object()->Is_Immovable()) {
			Fix_Stuck_Object(SourceObject, 5.f);
		}
		if (!Object->Peek_Physical_Object()->Is_Immovable()) {
			Fix_Stuck_Object(Object, 5.f);
		}
		return true;
	}

	return false;
}

StringClass Format_Seconds(int Seconds, bool AppendUnit) {
	int hours = Seconds / 3600;
	int minutes = (Seconds / 60) - (hours * 60);
	int seconds = Seconds - (((hours * 60) + minutes) * 60);

	if (AppendUnit) {
		if (hours > 0) {
			return StringFormat("%02d:%02d:%02d hours", hours, minutes, seconds);
		}
		else {
			return StringFormat("%02d:%02d minutes", minutes, seconds);
		}
	}
	else {
		return StringFormat("%02d:%02d:%02d", hours, minutes, seconds);
	}
}
