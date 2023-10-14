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
#include "da_game.h"
#include "da_cratemanager.h"
#include "da_settings.h"
// Custom
#include "EssentialsEventClass.h"
#include "EssentialsCrateInjector.h"

#define VECTOR_FOREACH(Variable, Vector) for (int Variable = 0; Variable < Vector.Count(); ++Variable)
#define VECTOR_FOREACH_REVERSE(Variable, Vector) for (int Variable = Vector.Count()-1; Variable >= 0; --Variable)

void EssentialsCrateManager::Init() {
	InjectCrates = false;
	TotalOdds = 0;
	Register_Event(DAEvent::LEVELLOADED);
	Register_Event(DAEvent::SETTINGSLOADED);
	Register_Event(DAEvent::GAMEOVER);
	Register_Event(DAEvent::POWERUPGRANT);
	Register_Object_Event(DAObjectEvent::DESTROYED, DAObjectEvent::POWERUP);
	Register_Chat_Command((DAECC)&EssentialsCrateManager::SCDCommand, "!showcratedetails|!scd", 0, DAAccessLevel::ADMINISTRATOR);

	DACrateManager::Init();
}

EssentialsCrateManager::~EssentialsCrateManager() {
	if (!DAGameManager::Is_Shutdown_Pending()) {
		Delete_Objects();
	}

	DACrateManager::~DACrateManager();
}

void EssentialsCrateManager::Settings_Loaded_Event() {
	Delete_Customs();

	DACrateManager::Settings_Loaded_Event();

	InjectCrates = DASettingsManager::Get_Bool("Essentials", "EnableCrateInjection", false);

	if (InjectCrates) {
		INISection* Section = DASettingsManager::Get_Section("EssentialsCrateInjections");
		if (Section) {
			for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
				StringClass Preset, Model;
				int OddsNum = 0, DoCrateBehavior = 1;

				DATokenParserClass Token(Entry->Value, '|');
				Preset = Entry->Entry;

				if (Find_Named_Definition(Preset)) {
					if (Token.Get_Int(OddsNum) && Token.Get_Int(DoCrateBehavior)) {
						if (OddsNum > 0) {
							Model = Token.Get_Remaining_String();
						} else {
							Console_Output("[Essentials] 0 chance preset detected for crate injection \"%s\", skipping...\n", Preset.Peek_Buffer());
							continue;
						}
					} else {
						Console_Output("[Essentials] Invalid settings detected for crate injection \"%s\", skipping...\n", Preset.Peek_Buffer());
						continue;
					}
				} else if (Preset == "*") {
					if (Token.Get_Int(OddsNum)) {
						if (OddsNum <= 0) {
							Console_Output("[Essentials] 0 chance preset detected for crate injection \"%s\", skipping...\n", Preset.Peek_Buffer());
							continue;
						}
					} else {
						Console_Output("[Essentials] Invalid settings detected for crate injection \"%s\", skipping...\n", Preset.Peek_Buffer());
						continue;
					}
				} else {
					Console_Output("[Essentials] Invalid preset for crate injection \"%s\", skipping...\n", Preset.Peek_Buffer());
					continue;
				}

				CustomPowerupClass* Powerup = new CustomPowerupClass;
				Powerup->Preset = Preset;
				Powerup->Model = Model;
				Powerup->Odds = OddsNum;
				Powerup->DoCrateBehavior = DoCrateBehavior != 0;

				bool skip = false;
				VECTOR_FOREACH(i, CustomPowerups) {
					if (CustomPowerups[i]->Odds < Powerup->Odds) {
						CustomPowerups.Insert(i, Powerup);
						skip = true;
						break;
					}
				}
				if (!skip) {
					CustomPowerups.Add(Powerup);
				}
			}
		}

		// Calculate odds.
		TotalOdds = 0;
		VECTOR_FOREACH_REVERSE(i, CustomPowerups) {
			TotalOdds += CustomPowerups[i]->Odds;
			CustomPowerups[i]->CalculatedOdds = TotalOdds;
		}

		if (CustomPowerups.Count() == 0) {
			Console_Output("[Essentials] Could not find any custom powerups for crate injection. Disabling crate injection...\n");
			InjectCrates = false;
		} else {
			Console_Output("[Essentials] Loaded %d crate injections.\n", CustomPowerups.Count());
		}
	}
}

void EssentialsCrateManager::Game_Over_Event() {
	DACrateManager::Game_Over_Event();
}

void EssentialsCrateManager::Timer_Expired(int Number, unsigned int Data) {
	if (InjectCrates) {
		if (The_Game()->Get_Current_Players() && The_Game()->Is_Gameplay_Permitted()) {
			CustomPowerupClass* Custom = 0;
			int Number = Get_Random_Int(0, TotalOdds);
			VECTOR_FOREACH_REVERSE(i, CustomPowerups) {
				if (Number < CustomPowerups[i]->CalculatedOdds) {
					Custom = CustomPowerups[i];
					break;
				}
			}
			if (Custom) {
				if (Custom->Preset != "*") {
					if (Vector3* Position = Select_Spawner()) {
						if (PhysicalGameObj* CustomObj = Create_Object(Custom->Preset, *Position)) {
							CrateObjs.Add(CustomObj);
							if (!Custom->DoCrateBehavior) {
								CustomObjs.Add(CustomObj);
							}
							if (!Custom->Model.Is_Empty()) {
								Commands->Set_Model(CustomObj, Custom->Model);
							}
						} else {
							Console_Output("[Essentials] Failed to create custom crate \"%s\".\n", Custom->Preset.Peek_Buffer());
							Start_Timer(1, Get_Random_Float(SpawnTimeMin, SpawnTimeMax));
						}
					} else {
						Start_Timer(1, Get_Random_Float(SpawnTimeMin, SpawnTimeMax));
					}
				} else {
					DACrateManager::Timer_Expired(Number, Data);
				}
			} else {
				DACrateManager::Timer_Expired(Number, Data);
			}
		}
	} else {
		DACrateManager::Timer_Expired(Number, Data);
	}
}

void EssentialsCrateManager::PowerUp_Grant_Event(cPlayer* Player, const PowerUpGameObjDef* PowerUp, PowerUpGameObj* PowerUpObj) {
	if (InjectCrates) {
		if (PowerUpObj && CrateObjs.ID(PowerUpObj) != -1 && CustomObjs.ID(PowerUpObj) != -1) {
			Player->Get_GameObj()->Get_Position(&LastCratePosition);
			PowerUpObj->Set_Delete_Pending();
			Start_Timer(1, Get_Random_Float(SpawnTimeMin, SpawnTimeMax));
		} else {
			DACrateManager::PowerUp_Grant_Event(Player, PowerUp, PowerUpObj);
		}
	}
	else {
		DACrateManager::PowerUp_Grant_Event(Player, PowerUp, PowerUpObj);
	}
}

void EssentialsCrateManager::Object_Destroyed_Event(GameObject* obj) {
	if (InjectCrates) {
		CustomObjs.DeleteObj((PhysicalGameObj*)obj);
	}

	DACrateManager::Object_Destroyed_Event(obj);
}

bool EssentialsCrateManager::SCDCommand(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType) {
	DA::Page_Player(Player, "There are %d crate objects.", CrateObjs.Count());
	for (int i = 0; i < CrateObjs.Count(); ++i) {
		GameObject* Crate = CrateObjs[i];
		if (Crate) {
			Vector3 Position = Commands->Get_Position(Crate);
			DA::Page_Player(Player, "Crate #%d: %s with model %s at X: %.2f, Y: %.2f, Z: %.2f.", i, Commands->Get_Preset_Name(Crate), Get_Model(Crate), Position.X, Position.Y, Position.Z);
		} else {
			DA::Page_Player(Player, "Crate #%d: This crate is in illegal state!", i);
			CrateObjs.DeleteObj((PhysicalGameObj*)Crate);
		}
	}

	DA::Page_Player(Player, "There are %d custom power-ups.", CustomObjs.Count());
	for (int i = 0; i < CustomObjs.Count(); ++i) {
		GameObject* Crate = CustomObjs[i];
		if (Crate) {
			Vector3 Position = Commands->Get_Position(Crate);
			DA::Page_Player(Player, "POW #%d: %s with model %s at X: %.2f, Y: %.2f, Z: %.2f.", i, Commands->Get_Preset_Name(Crate), Get_Model(Crate), Position.X, Position.Y, Position.Z);
		}
		else {
			DA::Page_Player(Player, "POW #%d: Information not available.", i);
			CustomObjs.DeleteObj((PhysicalGameObj*)Crate);
		}
	}

	DA::Page_Player(Player, "There are %d spawners, %d are available for use.", Spawners.Count(), ActiveSpawners.Count());
	for (int i = 0; i < Spawners.Count(); ++i) {
		Vector3 *Spawner = &Spawners[i];
		DA::Page_Player(Player, "Spawner #%d at X: %.2f, Y: %.2f, Z: %.2f.", i, Spawner->X, Spawner->Y, Spawner->Z);
		if (ActiveSpawners.ID(Spawner) == -1) {
			DA::Page_Player(Player, "Spawner #%d is occupied.", i);
		}
	}

	return false;
}

void EssentialsCrateManager::Delete_Customs() {
	VECTOR_FOREACH(i, CustomPowerups) {
		delete CustomPowerups[i];
	}

	CustomPowerups.Delete_All();
}

void EssentialsCrateManager::Delete_Objects() {
	VECTOR_FOREACH(i, CustomObjs) {
		CrateObjs.DeleteObj(CustomObjs[i]);
		Commands->Destroy_Object(CustomObjs[i]);
	}

	CustomObjs.Delete_All();
}

Register_Game_Feature(EssentialsCrateManager, "Crates", "EnableCrates", 0);
