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
#include "da_player.h"
#include "EssentialsPlayerDataClass.h"

#define CHATCMD_DECL_MT(Name) bool Name##_ChatCommand(cPlayer *Player,const DATokenClass &Text,TextMessageEnum ChatType); \
	                          DAAccessLevel::Level Name##MinLevel; \
	                          DAAccessLevel::Level Name##OthersLevel

#define CHATCMD_DECL_ST(Name) bool Name##_ChatCommand(cPlayer *Player,const DATokenClass &Text,TextMessageEnum ChatType); \
	                          DAAccessLevel::Level Name##MinLevel

#define CHATCMD_STNG_MT(Name) Name##MinLevel = (DAAccessLevel::Level)DASettingsManager::Get_Int("Essentials", #Name ## "MinLevel", -1); \
							  Name##OthersLevel = (DAAccessLevel::Level)DASettingsManager::Get_Int("Essentials", #Name ## "OthersLevel", -1)

#define CHATCMD_STNG_ST(Name) Name##MinLevel = (DAAccessLevel::Level)DASettingsManager::Get_Int("Essentials", #Name ## "MinLevel", 0);

#define CHATCMD_DEF(Class, Name) bool Class##::Name##_ChatCommand(cPlayer *Player,const DATokenClass &Text,TextMessageEnum ChatType)

#define CHATCMD_REG(Class, Name, Aliases, Parameters) if (Name##MinLevel >= 0) Register_Chat_Command((DAECC)&Class##::Name##_ChatCommand, Aliases, Parameters, Name##MinLevel, DAChatType::ALL)

#define DECIDE_TARGET(Length, Index) Text.Size() >= Length ? Match_Player(Player, Text[Index], false, true) : Player

#define CAN_MANAGE_OTHERS(Name, Player) Player->Get_DA_Player()->Get_Access_Level() >= Name##OthersLevel

#define JUKEBOXCOLOR 133,207,29

#define ESSENTIALS_API __declspec(dllexport)

#define IS_FILE_AVAILABLE(FileName) Get_Data_File(FileName) && Get_Data_File(FileName)->Is_Available()

class EssentialsEventClass : public DAEventClass, public DAPlayerDataManagerClass<EssentialsPlayerDataClass> {
public:
	static EssentialsEventClass* Instance;
	
	EssentialsEventClass();
	~EssentialsEventClass();

	void Settings_Loaded_Event() override;
	bool Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID) override;
	ConnectionAcceptanceFilter::STATUS Connection_Request_Event(ConnectionRequest& Request, WideStringClass& RefusalMessage) override;
	void Ren_Log_Event(const char* Output) override;

	CHATCMD_DECL_MT(Fly);                // !fly
	CHATCMD_DECL_MT(Spectate);           // !spectate
	CHATCMD_DECL_ST(SpawnObject);        // !spawn
	CHATCMD_DECL_MT(ChangeCharacter);    // !changechar
	CHATCMD_DECL_MT(GiveWeapon);         // !giveweap
	CHATCMD_DECL_ST(PrintFDS);           // !fds
	CHATCMD_DECL_ST(FastSpawn);          // !fspawn
	CHATCMD_DECL_MT(GiveMoney);          // !money
	CHATCMD_DECL_ST(GoTo);               // !goto
	CHATCMD_DECL_ST(Put);                // !put
	CHATCMD_DECL_MT(PutAll);             // !putall
	CHATCMD_DECL_MT(PutAllLine);         // !putallline
	CHATCMD_DECL_MT(PutAllCircle);       // !putallcircle
	CHATCMD_DECL_MT(SetHealth);          // !sethealth
	CHATCMD_DECL_MT(SetArmor);           // !setarmor
	CHATCMD_DECL_MT(SetMaxHealth);       // !setmaxhealth
	CHATCMD_DECL_MT(SetMaxArmor);        // !setmaxarmor
	CHATCMD_DECL_MT(SetClipAmmo);        // !setclipammo
	CHATCMD_DECL_MT(SetInventoryAmmo);   // !setinvammo
	CHATCMD_DECL_MT(SetObjectTeam);      // !setobjteam
	CHATCMD_DECL_MT(AttachScript);       // !attach
	CHATCMD_DECL_MT(DetachScript);       // !detach
	CHATCMD_DECL_MT(ChangeModel);        // !setmodel
	CHATCMD_DECL_MT(GetPosition);        // !getpos
	CHATCMD_DECL_MT(SetSpeed);           // !sspeed
	CHATCMD_DECL_ST(ReviveBuilding);     // !revivebld
	CHATCMD_DECL_MT(GodMode);            // !godmode
	CHATCMD_DECL_ST(Screenshot);         // !screen
	CHATCMD_DECL_MT(GetModelName);       // !modelname
	CHATCMD_DECL_MT(GetPresetName);      // !presetname

	bool FloodingBlocksChatMessages;
	bool ShowConnectionRequestMessages;
	bool ShowConnectionLostMessages;

	StringClass ConnectionRequestMessageFormat;
	StringClass ConnectionLostMessageFormat;

	float SpectateSpeed;

	DynamicVectorClass<StringClass>* BlockedSpawnPresets;
	bool AllowVehicleSpawn;
	bool AllowSoldierSpawn;
	bool AllowSimpleSpawn;
	bool AllowPowerupSpawn;

	DynamicVectorClass<StringClass>* BlockedCharacterPresets;

	DynamicVectorClass<StringClass>* BlockedWeaponPresets;

	HashTemplateClass<StringClass, StringClass>* FastSpawnList;

	DynamicVectorClass<StringClass>* BlockedScripts;

	DynamicVectorClass<StringClass>* BlockedModels;

	float MinSpeed;
	float MaxSpeed;

	HashTemplateClass<StringClass, StringClass>* ReviveBuildingAliases;

	StringClass GodModePreset;
	float GodModeSpeed;
	DynamicVectorClass<StringClass>* GodModeWeapons;
	bool MakeGodModeSpy;
	bool MakeGodModeStealth;
};
