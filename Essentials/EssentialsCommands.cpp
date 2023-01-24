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
#include "engine_weap.h"
#include "engine_player.h"
// DA
#include "da.h"
#include "da_translation.h"
// Custom
#include "ArmedGameObj.h"
#include "EssentialsEventClass.h"
#include "EssentialsUtils.h"
#include "GameObjManager.h"

#define VehicleElseSoldier(Name, Target) GameObject* Name = 0; if (Target->Get_GameObj()->Get_Vehicle()) { TargetObject = Target->Get_GameObj()->Get_Vehicle(); } else { Name = Target->Get_GameObj(); } Name

CHATCMD_DEF(EssentialsEventClass, Fly) {
	cPlayer *Target = CAN_MANAGE_OTHERS(Fly, Player) ? DECIDE_TARGET(1, 1) : Player;
	EssentialsPlayerDataClass* Data = Get_Player_Data(Target);
	if (Data->Set_IsFlying(!Data->Get_IsFlying())) {
		if (Data->Get_IsFlying()) {
			if (Target != Player) {
				DA::Page_Player(Player, "%ws is now flying.", Target->Get_Name().Peek_Buffer());
			}
		} else {
			if (Target != Player) {
				DA::Page_Player(Player, "%ws is no longer flying.", Target->Get_Name().Peek_Buffer());
			}
		}
	} else {
		DA::Page_Player(Player, "Couldn't toggle %ws's fly mode.", Target->Get_Name().Peek_Buffer());
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, Spectate) {
	cPlayer *Target = CAN_MANAGE_OTHERS(Spectate, Player) ? DECIDE_TARGET(1, 1) : Player;
	EssentialsPlayerDataClass *Data = Get_Player_Data(Target);
	if (Data->Get_IsSpectating()) {
		if (Data->Set_IsSpectating(false)) {
			if (Target != Player) {
				DA::Page_Player(Player, "%ws is no longer spectating.", Target->Get_Name().Peek_Buffer());
			}
		} else {
			DA::Page_Player(Player, "Couldn't toggle %ws's spectate mode.", Target->Get_Name().Peek_Buffer());
		}
	} else {
		if (Data->Set_IsSpectating(true)) {
			if (Target != Player) {
				DA::Page_Player(Player, "%ws is now spectating.", Target->Get_Name().Peek_Buffer());
			}
		} else {
			DA::Page_Player(Player, "Couldn't toggle %ws's spectate mode.", Target->Get_Name().Peek_Buffer());
		}
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SpawnObject) {
	StringClass Preset = Text[0];
	if (BlockedSpawnPresets->ID(Preset) == -1) {
		DefinitionClass* Def = Find_Named_Definition(Preset.Peek_Buffer());
		if (Def) {
			if ((Def->Get_Class_ID() == CID_Vehicle && AllowVehicleSpawn) || (Def->Get_Class_ID() == CID_Soldier && AllowSoldierSpawn) ||
				(Def->Get_Class_ID() == CID_Simple && AllowSimpleSpawn) || (Def->Get_Class_ID() == CID_PowerUp && AllowPowerupSpawn)) {
				GameObject* obj = Spawn_Object(Player, Def->Get_ID());
				if (obj) {
					DA::Page_Player(Player, "Successfully spawned \"%s\".", DATranslationManager::Translate(obj));
				} else {
					DA::Page_Player(Player, "Invalid preset type: %s", Def->Get_Name());
				}
			} else {
				DA::Page_Player(Player, "Disallowed preset: %s", Def->Get_Name());
			}
		} else {
			DA::Page_Player(Player, "Invalid preset: %s", Preset.Peek_Buffer());
		}
	} else {
		DA::Page_Player(Player, "Spawning \"%s\" is disabled.", Preset.Peek_Buffer());
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, ChangeCharacter) {
	cPlayer* Target = CAN_MANAGE_OTHERS(ChangeCharacter, Player) ? DECIDE_TARGET(2, 1) : Player;
	StringClass Preset = Text.Size() >= 2 ? Text[2] : Text[0];
	if (BlockedCharacterPresets->ID(Preset) == -1) {
		DefinitionClass* Def = Find_Named_Definition(Preset.Peek_Buffer());
		if (Def) {
			if (Def->Get_Class_ID() == CID_Soldier) {
				Target->Get_GameObj()->Re_Init(*(SoldierGameObjDef*)Def);
				Target->Get_GameObj()->Post_Re_Init();
				if (Target != Player) {
					DA::Page_Player(Player, "%ws's character has been changed to \"%s\".", Target->Get_Name().Peek_Buffer(), DATranslationManager::Translate(Def));
				} else {
					DA::Page_Player(Player, "Your character has been changed to \"%s\".", DATranslationManager::Translate(Def));
				}
			} else {
				DA::Page_Player(Player, "Invalid preset type: %s", Preset.Peek_Buffer());
			}
		} else {
			DA::Page_Player(Player, "Invalid preset: %s", Def->Get_Name());
		}
	} else {
		DA::Page_Player(Player, "Changing character to \"%s\" is disabled.", Preset.Peek_Buffer());
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GiveWeapon) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GiveWeapon, Player) ? DECIDE_TARGET(2, 1) : Player;
	StringClass Preset = Text.Size() >= 2 ? Text[2] : Text[0];
	if (BlockedWeaponPresets->ID(Preset) == -1) {
		DefinitionClass* Def = Find_Named_Definition(Preset.Peek_Buffer());
		if (Def) {
			if (Def->Get_Class_ID() == CID_Weapon) {
				WeaponDefinitionClass* WeaponDef = (WeaponDefinitionClass*)Def;
				VehicleElseSoldier(TargetObject, Target);

				if (TargetObject) {
					WeaponBagClass* Bag = TargetObject->As_PhysicalGameObj()->As_ArmedGameObj()->Get_Weapon_Bag();
					WeaponClass* Weapon = Bag->Add_Weapon(WeaponDef, 999);
					if (TargetObject->As_VehicleGameObj()) {
						Bag->Select_Weapon(Weapon);
					}

					if (Target != Player) {
						if (TargetObject->As_VehicleGameObj()) {
							DA::Page_Player(Player, "%ws's vehicle has been given \"%s\".", Target->Get_Name().Peek_Buffer(), DATranslationManager::Translate(Def));
						} else {
							DA::Page_Player(Player, "%ws has been given \"%s\".", Target->Get_Name().Peek_Buffer(), DATranslationManager::Translate(Def));
						}
					} else {
						if (TargetObject->As_VehicleGameObj()) {
							DA::Page_Player(Player, "Your vehicle has been given \"%s\".", DATranslationManager::Translate(Def));
						} else {
							DA::Page_Player(Player, "You have been given \"%s\".", DATranslationManager::Translate(Def));
						}
					}
				} else {
					DA::Page_Player(Player, "Couldn't find target object.");
				}
			} else {
				DA::Page_Player(Player, "Invalid preset type: %s", Def->Get_Name());
			}
		} else {
			DA::Page_Player(Player, "Invalid preset: %s", Preset.Peek_Buffer());
		}
	} else {
		DA::Page_Player(Player, "Giving weapon \"%s\" is disabled.", Preset.Peek_Buffer());
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, PrintFDS) {
	Console_Input(Text[0]);
	DA::Page_Player(Player, "Line has been sent to FDS console.");
	
	return false;
}

CHATCMD_DEF(EssentialsEventClass, FastSpawn) {
	if (FastSpawnList->Exists(Text[0])) {
		StringClass* Preset = FastSpawnList->Get(Text[0]);
		DefinitionClass* Def = Find_Named_Definition(Preset->Peek_Buffer());
		if (Def) {
			GameObject* obj = Spawn_Object(Player, Def->Get_ID());
			if (obj) {
				DA::Page_Player(Player, "Successfully spawned \"%s\".", DATranslationManager::Translate(Def));
			} else {
				DA::Page_Player(Player, "Invalid preset type: %s", Def->Get_Name());
			}
		} else {
			DA::Page_Player(Player, "Invalid preset: %s", Preset->Peek_Buffer());
		}
	} else {
		DA::Page_Player(Player, "Invalid alias: \"%s\".", Text[0]);
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GiveMoney) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GiveMoney, Player) ? DECIDE_TARGET(2, 1): Player;
	float amount = .0f;
	
	if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, amount)) {
		if (amount > .0f) {
			Target->Increment_Money(amount);
			if (Target != Player) {
				DA::Page_Player(Player, "%ws has been given %.0f credits.", Target->Get_Name().Peek_Buffer(), amount);
			} else {
				DA::Page_Player(Player, "You've been given %.0f credits.", amount);
			}
		} else {
			DA::Page_Player(Player, "Amount must be greater than zero.");
		}
	} else {
		DA::Page_Player(Player, "Amount must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GoTo) {
	cPlayer* Target = Match_Player(Player, Text[0], false, true);

	if (Target) {
		if (Teleport_Player(Player, Target)) {
			DA::Page_Player(Player, "You've successfully teleported to %ws's location.", Target->Get_Name().Peek_Buffer());
		} else {
			DA::Page_Player(Player, "Could not teleport you to %ws's location.", Target->Get_Name().Peek_Buffer());
		}
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, Put) {
	cPlayer* Source = Match_Player(Player, Text[1], false, true);
	cPlayer* Target = Match_Player(Player, Text[2], false, true);

	if (Source && Target) {
		if (Teleport_Player(Source, Target)) {
			DA::Page_Player(Player, "You've successfully teleported %ws to %ws's location.", Source->Get_Name().Peek_Buffer(), Target->Get_Name().Peek_Buffer());
		} else {
			DA::Page_Player(Player, "Could not teleport %ws to %ws's location.", Source->Get_Name().Peek_Buffer(), Target->Get_Name().Peek_Buffer());
		}
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, PutAll) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GetPosition, Player) ? DECIDE_TARGET(1, 1) : Player;
	VehicleElseSoldier(TargetObject, Target);

	if (TargetObject) {
		Vector3 Position = Commands->Get_Position(TargetObject);

		for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
			cPlayer* Soldier = z->Data();
			if (Soldier && Soldier->Is_Active() && Soldier->Get_GameObj()) {
				if (Soldier->Get_GameObj()->Get_Vehicle()) {
					Soldier_Transition_Vehicle(Soldier->Get_GameObj());
				}
				Commands->Set_Position(Soldier->Get_GameObj(), Position);
			}
		}
		Fix_Stuck_Objects(Position, 3.f, 25.f, false);

		if (Target != Player) {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Teleported everyone to %ws's vehicle's location.", Target->Get_Name().Peek_Buffer());
			} else {
				DA::Page_Player(Player, "Teleported everyone to %ws's location.", Target->Get_Name().Peek_Buffer());
			}
		} else {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Teleported everyone to your vehicle's location.");
			} else {
				DA::Page_Player(Player, "Teleported everyone to your location.");
			}
		}
	} else {
		DA::Page_Player(Player, "Couldn't find target object.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, PutAllLine) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GetPosition, Player) ? DECIDE_TARGET(2, 1) : Player;
	VehicleElseSoldier(TargetObject, Target);

	if (TargetObject) {
		float inc = 1.f;
		if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, inc)) {
			Vector3 Position = Commands->Get_Position(TargetObject);
			float facingSin = sin(DEG2RAD(Commands->Get_Facing(TargetObject)));
			float facingCos = cos(DEG2RAD(Commands->Get_Facing(TargetObject)));
			float currOffset = inc;

			for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
				cPlayer* Soldier = z->Data();
				if (Soldier && Soldier->Is_Active() && Soldier->Get_GameObj()) {
					if (Soldier->Get_GameObj()->Get_Vehicle()) {
						Soldier_Transition_Vehicle(Soldier->Get_GameObj());
					}
					Vector3 newLoc = Position;
					newLoc.X += currOffset * facingCos;
					newLoc.Y += currOffset * facingSin;
					Commands->Set_Position(Soldier->Get_GameObj(), newLoc);
					currOffset += inc;
				}
			}
	
			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Teleported everyone in line to in front of %ws's vehicle.", Target->Get_Name().Peek_Buffer());
				} else {
					DA::Page_Player(Player, "Teleported everyone in line to in front of %ws.", Target->Get_Name().Peek_Buffer());
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Teleported everyone in line to in front of your vehicle.");
				} else {
					DA::Page_Player(Player, "Teleported everyone in line to in front of you.");
				}
			}
		} else {
			DA::Page_Player(Player, "Value must be a number.");
		}
	} else {
		DA::Page_Player(Player, "Couldn't find target object.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, PutAllCircle) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GetPosition, Player) ? DECIDE_TARGET(2, 1) : Player;
	VehicleElseSoldier(TargetObject, Target);

	if (TargetObject) {
		float radius = 1.f;

		if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, radius)) {
			Vector3 Position = Commands->Get_Position(TargetObject);
			float inc = 360.f;
			float currRadius = 0.f;

			int count = 0;
			for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
				cPlayer* Soldier = z->Data();
				if (Soldier && Soldier->Is_Active() && Soldier->Get_GameObj()) {
					++count;
				}
			}

			inc /= count;

			for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
				cPlayer* Soldier = z->Data();
				if (Soldier && Soldier->Is_Active() && Soldier->Get_GameObj()) {
					if (Soldier->Get_GameObj()->Get_Vehicle()) {
						Soldier_Transition_Vehicle(Soldier->Get_GameObj());
					}
					Vector3 newLoc = Position;
					newLoc.X += radius * cos(DEG2RAD(currRadius));
					newLoc.Y += radius * sin(DEG2RAD(currRadius));
					Commands->Set_Position(Soldier->Get_GameObj(), newLoc);
					currRadius += inc;
				}
			}

			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Teleported everyone in circle around %ws's vehicle.", Target->Get_Name().Peek_Buffer());
				}
				else {
					DA::Page_Player(Player, "Teleported everyone in circle around %ws.", Target->Get_Name().Peek_Buffer());
				}
			}
			else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Teleported everyone in circle around your vehicle.");
				}
				else {
					DA::Page_Player(Player, "Teleported everyone in circle around you.");
				}
			}
		}
		else {
			DA::Page_Player(Player, "Value must be a number.");
		}
	}
	else {
		DA::Page_Player(Player, "Couldn't find target object.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetHealth) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetHealth, Player) ? DECIDE_TARGET(2, 1) : Player;
	float amount = .0f;

	if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, amount)) {
		VehicleElseSoldier(TargetObject, Target);

		if (TargetObject) {
			Commands->Set_Health(TargetObject, amount);
			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's health has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "%ws's health has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's health has been set to %.0f.", amount);
				} else {
					DA::Page_Player(Player, "Your health has been set to %.0f.", amount);
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Value must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetArmor) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetArmor, Player) ? DECIDE_TARGET(2, 1) : Player;
	float amount = .0f;

	if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, amount)) {
		VehicleElseSoldier(TargetObject, Target);

		if (TargetObject) {
			Commands->Set_Shield_Strength(TargetObject, amount);
			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's armor has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "%ws's armor has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's armor has been set to %.0f.", amount);
				} else {
					DA::Page_Player(Player, "Your armor has been set to %.0f.", amount);
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Value must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetMaxHealth) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetMaxHealth, Player) ? DECIDE_TARGET(2, 1) : Player;
	float amount = .0f;

	if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, amount)) {
		VehicleElseSoldier(TargetObject, Target);

		if (TargetObject) {
			Set_Max_Health_Without_Healing(TargetObject, amount);
			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's maximum health has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "%ws's maximum health has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's maximum health has been set to %.0f.", amount);
				} else {
					DA::Page_Player(Player, "Your maximum health has been set to %.0f.", amount);
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Value must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetMaxArmor) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetMaxArmor, Player) ? DECIDE_TARGET(2, 1) : Player;
	float amount = .0f;

	if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, amount)) {
		VehicleElseSoldier(TargetObject, Target);

		if (TargetObject) {
			Set_Max_Shield_Strength_Without_Healing(TargetObject, amount);
			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's maximum armor has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "%ws's maximum armor has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's maximum armor has been set to %.0f.", amount);
				} else {
					DA::Page_Player(Player, "Your maximum armor has been set to %.0f.", amount);
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Value must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetClipAmmo) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetClipAmmo, Player) ? DECIDE_TARGET(2, 1) : Player;
	int amount = 0;

	if (Text.As_Int(Text.Size() >= 2 ? 2 : 1, amount)) {
		VehicleElseSoldier(TargetObject, Target);
		
		if (TargetObject) {
			Set_Bullets(TargetObject, Get_Current_Weapon(TargetObject), amount);

			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's current weapon's clip ammo has been set to %d.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "%ws's current weapon's clip ammo has been set to %d.", Target->Get_Name().Peek_Buffer(), amount);
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's current weapon's clip ammo has been set to %d.", amount);
				} else {
					DA::Page_Player(Player, "Your current weapon's clip ammo has been set to %d.", amount);
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Value must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetInventoryAmmo) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetInventoryAmmo, Player) ? DECIDE_TARGET(2, 1) : Player;
	int amount = 0;

	if (Text.As_Int(Text.Size() >= 2 ? 2 : 1, amount)) {
		VehicleElseSoldier(TargetObject, Target);

		if (TargetObject) {
			Set_Clip_Bullets(TargetObject, Get_Current_Weapon(TargetObject), amount);

			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's current weapon's inventory ammo has been set to %d.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "%ws's current weapon's inventory ammo has been set to %d.", Target->Get_Name().Peek_Buffer(), amount);
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's current weapon's inventory ammo has been set to %d.", amount);
				} else {
					DA::Page_Player(Player, "Your current weapon's inventory ammo has been set to %d.", amount);
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Value must be a number.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetObjectTeam) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetObjectTeam, Player) ? DECIDE_TARGET(2, 1) : Player;
	int team = Target->Get_Player_Type();

	if (Text.As_Int(Text.Size() >= 2 ? 2 : 1, team)) {
		Set_Object_Type(Target->Get_GameObj(), team);

		if (Target != Player) {
			DA::Page_Player(Player, "%ws's object's team has been set to %ws.", Target->Get_Name().Peek_Buffer(), Get_Wide_Team_Name(team));
		} else {
			DA::Page_Player(Player, "Your object's team has been set to %ws.", Get_Wide_Team_Name(team));
		}
	} else {
		DA::Page_Player(Player, "Team value must be a number. (0 = Nod, 1 = GDI, -1 = Renegade, 2 = Neutral)");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, AttachScript) {
	StringClass Script = Text[2];
	if (BlockedScripts->ID(Script) == -1) {
		if (cPlayer* Target = CAN_MANAGE_OTHERS(AttachScript, Player) ? Match_Player(Player, Text[1], false, true) : Player) {
			VehicleElseSoldier(TargetObject, Target);

			if (TargetObject) {
				if (Text.Size() > 2) {
					Attach_Script_Once(TargetObject, Script.Peek_Buffer(), Text[3]);
				} else {
					Attach_Script_Once(TargetObject, Script.Peek_Buffer(), "");
				}

				if (Target != Player) {
					if (TargetObject->As_VehicleGameObj()) {
						DA::Page_Player(Player, "\"%s\" has been attached to %ws's vehicle with %s.", Script.Peek_Buffer(), Target->Get_Name().Peek_Buffer(), Text.Size() > 2 ? "parameters" : "no parameters");
					} else {
						DA::Page_Player(Player, "\"%s\" has been attached to %ws's with %s.", Script.Peek_Buffer(), Target->Get_Name().Peek_Buffer(), Text.Size() > 2 ? "parameters" : "no parameters");
					}
				} else {
					if (TargetObject->As_VehicleGameObj()) {
						DA::Page_Player(Player, "\"%s\" has been attached to your vehicle with %s.", Script.Peek_Buffer(), Text.Size() > 2 ? "parameters" : "no parameters");
					} else {
						DA::Page_Player(Player, "\"%s\" has been attached to you with %s.", Script.Peek_Buffer(), Text.Size() > 2 ? "parameters" : "no parameters");
					}
				}
			} else {
				DA::Page_Player(Player, "Couldn't find target object.");
			}
		}
	} else {
		DA::Page_Player(Player, "Attaching specified script is not allowed.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, DetachScript) {
	StringClass Script = Text[2];
	if (BlockedScripts->ID(Script) == -1) {
		if (cPlayer* Target = CAN_MANAGE_OTHERS(DetachScript, Player) ? Match_Player(Player, Text[1], false, true) : Player) {
			VehicleElseSoldier(TargetObject, Target);

			if (TargetObject) {
				if (ScriptImpClass* ScriptObj = Find_Script_On_Object(TargetObject, Script.Peek_Buffer())) {
					ScriptObj->Destroy_Script();
					if (Target != Player) {
						if (TargetObject->As_VehicleGameObj()) {
							DA::Page_Player(Player, "\"%s\" has been detached from %ws's vehicle.", Script.Peek_Buffer(), Target->Get_Name().Peek_Buffer());
						} else {
							DA::Page_Player(Player, "\"%s\" has been detached from %ws.", Script.Peek_Buffer(), Target->Get_Name().Peek_Buffer());
						}
					} else {
						if (TargetObject->As_VehicleGameObj()) {
							DA::Page_Player(Player, "\"%s\" has been detached from your vehicle.", Script.Peek_Buffer());
						} else {
							DA::Page_Player(Player, "\"%s\" has been detached from you.", Script.Peek_Buffer());
						}
					}
				} else {
					if (Target != Player) {
						if (TargetObject->As_VehicleGameObj()) {
							DA::Page_Player(Player, "Could not find script \"%s\" on %ws's vehicle.", Script.Peek_Buffer(), Target->Get_Name().Peek_Buffer());
						} else {
							DA::Page_Player(Player, "Could not find script \"%s\" on %ws.", Script.Peek_Buffer(), Target->Get_Name().Peek_Buffer());
						}
					} else {
						if (TargetObject->As_VehicleGameObj()) {
							DA::Page_Player(Player, "Could not find script \"%s\" on your vehicle.", Script.Peek_Buffer());
						} else {
							DA::Page_Player(Player, "Could not find script \"%s\" on you.", Script.Peek_Buffer());
						}
					}
				}
			} else {
				DA::Page_Player(Player, "Couldn't find target object.");
			}
		}
	} else {
		DA::Page_Player(Player, "Detaching specified script is not allowed.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, ChangeModel) {
	StringClass Model = Text.Size() > 1 ? Text[2] : Text[0];
	if (BlockedModels->ID(Model) == -1) {
		cPlayer* Target = CAN_MANAGE_OTHERS(ChangeModel, Player) ? DECIDE_TARGET(2, 1) : Player;
		VehicleElseSoldier(TargetObject, Target);

		if (TargetObject) {
			Commands->Set_Model(TargetObject, Model.Peek_Buffer());
			if (Target != Player) {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "%ws's vehicle's model has been set to %s.", Target->Get_Name().Peek_Buffer(), Model.Peek_Buffer());
				} else {
					DA::Page_Player(Player, "%ws's model has been set to %s.", Target->Get_Name().Peek_Buffer(), Model.Peek_Buffer());
				}
			} else {
				if (TargetObject->As_VehicleGameObj()) {
					DA::Page_Player(Player, "Your vehicle's model has been set to %s.", Model.Peek_Buffer());
				} else {
					DA::Page_Player(Player, "Your model has been set to %s.", Model.Peek_Buffer());
				}
			}
		} else {
			DA::Page_Player(Player, "Couldn't find target object.");
		}
	} else {
		DA::Page_Player(Player, "Changing to specified model is not allowed.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GetPosition) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GetPosition, Player) ? DECIDE_TARGET(1, 1) : Player;
	VehicleElseSoldier(TargetObject, Target);

	if (TargetObject) {
		Vector3 Position = Commands->Get_Position(TargetObject);
		if (Target != Player) {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Position of %ws's vehicle:", Target->Get_Name().Peek_Buffer());
			} else {
				DA::Page_Player(Player, "Position of %ws:", Target->Get_Name().Peek_Buffer());
			}
		} else {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Position of your vehicle:");
			} else {
				DA::Page_Player(Player, "Position of you:");
			}
		}
		DA::Page_Player(Player, "(X: %f) | (Y: %f) | (Z: %f) | (Facing: %f)", Position.X, Position.Y, Position.Z, Commands->Get_Facing(TargetObject));
	} else {
		DA::Page_Player(Player, "Couldn't find target object.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, SetSpeed) {
	cPlayer* Target = CAN_MANAGE_OTHERS(SetSpeed, Player) ? DECIDE_TARGET(2, 1) : Player;
	float amount = .0f;

	if (Text.As_Float(Text.Size() >= 2 ? 2 : 1, amount)) {
		if (amount >= MinSpeed) {
			if (amount <= MaxSpeed) {
				SoldierGameObj* Soldier = Target->Get_GameObj();
				Soldier->Set_Max_Speed(amount);
				if (Target != Player) {
					DA::Page_Player(Player, "%ws's speed has been set to %.0f.", Target->Get_Name().Peek_Buffer(), amount);
				} else {
					DA::Page_Player(Player, "Your speed has been set to %.0f.", amount);
				}
			} else {
				DA::Page_Player(Player, "Speed value must be less or equal to %.0f.", MaxSpeed);
			}
		} else {
			DA::Page_Player(Player, "Speed value must be greater or equal to %.0f.", MinSpeed);
		}
	} else {
		DA::Page_Player(Player, "Speed value must be a number.");
	}
	
	return false;
}

CHATCMD_DEF(EssentialsEventClass, ReviveBuilding) {
	int Team = -1;
	if (!strcmp(Text[1], "1") || !_stricmp(Text[1], Get_Team_Name(1))) {
		Team = 1;
	} else if (!strcmp(Text[1], "0") || !_stricmp(Text[1], Get_Team_Name(0))) {
		Team = 0;
	}

	if (Team > -1) {
		int Revived = 0;
		StringClass Preset = Text[2];
		if (StringClass *Alias = ReviveBuildingAliases->Get(Preset)) {
			Preset = Alias->Peek_Buffer();
		}
		for(SLNode<BuildingGameObj>* N = GameObjManager::BuildingGameObjList.Head(); N; N = N->Next()) {
			BuildingGameObj* Building = N->Data();
			if (Building) {
				if (Building->Is_Destroyed()) {
					if (Get_Object_Type(Building) == Team) {
						if (stristr(Commands->Get_Preset_Name(Building), Preset.Peek_Buffer())) {
							Building->On_Revived();
							DA::Page_Player(Player, "Revived %s.", DATranslationManager::Translate(Building));
							++Revived;
						}
					}
				}
			}
		}
		if (Revived > 0) {
			DA::Page_Player(Player, "Revived %d building%s in %s.", Revived, Revived != 1 ? "s" : "", Get_Team_Name(Team));
		} else {
			DA::Page_Player(Player, "Couldn't find any building to revive.");
		}
	} else {
		DA::Page_Player(Player, "Unable to find team \"%s\". Try (1 = %s) or (0 = %s)", Text[1], Get_Team_Name(1), Get_Team_Name(0));
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GodMode) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GodMode, Player) ? DECIDE_TARGET(1, 1) : Player;
	SoldierGameObj* Soldier = Target->Get_GameObj();
	
	Soldier->Re_Init((SoldierGameObjDef&)*Find_Named_Definition(GodModePreset.Peek_Buffer()));
	Soldier->Post_Re_Init();
	Attach_Script_Once(Soldier, "KAK_Prevent_Kill", "");
	Soldier->Set_Max_Speed(GodModeSpeed);
	WeaponBagClass* Bag = Soldier->Get_Weapon_Bag();
	for (int i = 0; i < GodModeWeapons->Count(); ++i) {
		StringClass Preset = (*GodModeWeapons)[i];
		if (DefinitionClass *Def = Find_Named_Definition(Preset)) {
			if (Def->Get_Class_ID() == CID_Weapon) {
				Bag->Add_Weapon(Def->Get_Name(), 999);
			}
		}
	}
	if (MakeGodModeSpy) {
		Soldier->Set_Is_Visible(false);
	}
	if (MakeGodModeStealth) {
		Commands->Enable_Stealth(Soldier, true);
	}

	if (Target != Player) {
		DA::Page_Player(Player, "%ws has been put into god mode.", Target->Get_Name());
	} else {
		DA::Page_Player(Player, "You've been put into god mode.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, Screenshot) {
	if (cPlayer* Target = Match_Player(Player, Text[0], false, true)) {
		Take_Screenshot(Target->Get_Id());
		DA::Page_Player(Player, "Command to take a screenshot of %ws's game has been sent.", Target->Get_Name());
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GetModelName) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GetModelName, Player) ? DECIDE_TARGET(1, 1) : Player;
	VehicleElseSoldier(TargetObject, Target);

	if (TargetObject) {
		Vector3 Position = Commands->Get_Position(TargetObject);
		if (Target != Player) {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Name of %ws's vehicle's model is \"%s\".", Target->Get_Name().Peek_Buffer(), Get_Model(TargetObject));
			}
			else {
				DA::Page_Player(Player, "Name of %ws's soldier's model is \"%s\".", Target->Get_Name().Peek_Buffer(), Get_Model(TargetObject));
			}
		}
		else {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Name of your vehicle's model is \"%s\".", Get_Model(TargetObject));
			}
			else {
				DA::Page_Player(Player, "Name of your soldier's model is \"%s\".", Get_Model(TargetObject));
			}
		}
	}
	else {
		DA::Page_Player(Player, "Couldn't find target object.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, GetPresetName) {
	cPlayer* Target = CAN_MANAGE_OTHERS(GetModelName, Player) ? DECIDE_TARGET(1, 1) : Player;
	VehicleElseSoldier(TargetObject, Target);

	if (TargetObject) {
		Vector3 Position = Commands->Get_Position(TargetObject);
		if (Target != Player) {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Name and ID of %ws's vehicle's preset is \"%s\" (%d).", Target->Get_Name().Peek_Buffer(), TargetObject->Get_Definition().Get_Name(), TargetObject->Get_Definition().Get_ID());
			}
			else {
				DA::Page_Player(Player, "Name and ID of %ws's soldier's preset is \"%s\" (%d).", Target->Get_Name().Peek_Buffer(), TargetObject->Get_Definition().Get_Name(), TargetObject->Get_Definition().Get_ID());
			}
		}
		else {
			if (TargetObject->As_VehicleGameObj()) {
				DA::Page_Player(Player, "Name and ID of your vehicle's preset is \"%s\" (%d).", TargetObject->Get_Definition().Get_Name(), TargetObject->Get_Definition().Get_ID());
			}
			else {
				DA::Page_Player(Player, "Name and ID of your soldier's preset is \"%s\" (%d).", TargetObject->Get_Definition().Get_Name(), TargetObject->Get_Definition().Get_ID());
			}
		}
	}
	else {
		DA::Page_Player(Player, "Couldn't find target object.");
	}

	return false;
}

CHATCMD_DEF(EssentialsEventClass, Sudo) {
	cPlayer* Target = Match_Player(Player, Text[1], false, true);
	
	if (Target) {
		StringClass Message = Text(2);
		DA::Page_Player(Player, "Forcing %ws to type: %s", Target->Get_Name(), Message);

		WideStringClass WideMessage(Message);
		if (DAEventManager::Chat_Event(Target, TEXT_MESSAGE_PUBLIC, WideMessage, -1)) {
			Send_Client_Text(WideMessage, TEXT_MESSAGE_PUBLIC, false, Target->Get_Id(), -1, true, true);
		}
	}
	return false;
}

CHATCMD_DEF(EssentialsEventClass, SudoTeam) {
	cPlayer* Target = Match_Player(Player, Text[1], false, true);

	if (Target) {
		StringClass Message = Text(2);
		DA::Page_Player(Player, "Forcing %ws to type: %s", Target->Get_Name(), Message);
		
		WideStringClass WideMessage(Message);
		if (DAEventManager::Chat_Event(Target, TEXT_MESSAGE_TEAM, WideMessage, -1)) {
			Send_Client_Text(WideMessage, TEXT_MESSAGE_TEAM, false, Target->Get_Id(), -1, true, true);
		}
	}
	return false;
}