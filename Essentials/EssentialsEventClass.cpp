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
#include "engine_obj.h"
// DA
#include "da.h"
#include "da_log.h"
#include "da_settings.h"
#include "da_translation.h"
// Custom
#include "EssentialsEventClass.h"
#include "EssentialsUtils.h"
#include "EssentialsBadWordFilter.h"
#include "EssentialsGruntEffect.h"
#include "EssentialsHolograms.h"
#include "EssentialsJukebox.h"
#include "EssentialsMapHolograms.h"
#include "EssentialsCustomPowerUpSpawners.h"
#include "EssentialsAutoAnnounce.h"
#include "EssentialsAuthentication.h"
#include "EssentialsCustomTag.h"
#include "EssentialsVoting.h"
#include "EssentialsSnipingGameModeClass.h"
#include "EssentialsCMSManager.h"

EssentialsEventClass* EssentialsEventClass::Instance = 0;

EssentialsEventClass::EssentialsEventClass() {
	Instance = this;

	SpectateSpeed = 30.f;
	
	BlockedSpawnPresets = new DynamicVectorClass<StringClass>;
	AllowVehicleSpawn = true;
	AllowSoldierSpawn = true;
	AllowSimpleSpawn = true;
	AllowPowerupSpawn = true;
	
	BlockedCharacterPresets = new DynamicVectorClass<StringClass>;

	BlockedWeaponPresets = new DynamicVectorClass<StringClass>;

	FastSpawnList = new HashTemplateClass<StringClass, StringClass>;

	BlockedScripts = new DynamicVectorClass<StringClass>;

	BlockedModels = new DynamicVectorClass<StringClass>;

	MinSpeed = .0f;
	MaxSpeed = 100.f;

	ReviveBuildingAliases = new HashTemplateClass<StringClass, StringClass>;

	GodModePreset = "Walk-Thru";
	GodModeSpeed = 25.f;
	GodModeWeapons = new DynamicVectorClass<StringClass>;
	MakeGodModeSpy = true;
	MakeGodModeStealth = true;

	FloodingBlocksChatMessages = true;

	ShowConnectionRequestMessages = true;
	ShowConnectionLostMessages = true;

	StringClass ConnectionRequestMessageFormat = "{NAME} is loading to join server.";
	StringClass ConnectionLostMessageFormat = "{NAME} lost communication to server.";

	RestoreMap = false;
	AllowBuildingRepair = true;

	Init();
	Register_Event(DAEvent::LEVELLOADED, INT_MAX);
	Register_Event(DAEvent::SETTINGSLOADED, INT_MAX);
	Register_Event(DAEvent::CHAT, INT_MAX);
	Register_Event(DAEvent::CONNECTIONREQUEST, 0);
	Register_Event(DAEvent::RENLOG, INT_MAX);
	Register_Object_Event(DAObjectEvent::DAMAGERECEIVEDREQUEST, DAObjectEvent::BUILDING);
}

EssentialsEventClass::~EssentialsEventClass() {
	Instance = 0;
}

void EssentialsEventClass::Level_Loaded_Event() {
	AllowBuildingRepair = true;
	if (RestoreMap) {
		Set_Map(RestoreMapName, Get_Current_Map_Index());
		RestoreMap = false;
	}
}

void EssentialsEventClass::Settings_Loaded_Event() {
	bool hologramsSuccess = true;
	if (DefinitionClass* Def = Find_Named_Definition(DASettingsManager::Get_String(EssentialsHologramsManager::HologramPreset, "Essentials", "HologramPreset", "Walk-Thru"))) {
		if (Def->Get_Class_ID() != CID_Soldier) {
			Console_Output("[Essentials] Hologram preset has to be a soldier preset, defaulting to \"Walk-Thru\"...\n");
			EssentialsHologramsManager::HologramPreset = "Walk-Thru";

			if (DefinitionClass* Def2 = Find_Named_Definition(EssentialsHologramsManager::HologramPreset)) {
				if (Def2->Get_Class_ID() != CID_Soldier) {
					Console_Output("[Essentials] Default hologram preset is not compatible with Hologram feature.\n");
					hologramsSuccess = false;
				}
			}
			else {
				Console_Output("[Essentials] Default hologram preset could not be found, perhaps it is renamed or deleted?\n");
				hologramsSuccess = false;
			}
		}
	}

	if (!IS_FILE_AVAILABLE(StringFormat("%s.w3d", DASettingsManager::Get_String(EssentialsHologramsManager::HologramModel, "Essentials", "HologramModel", "v_invs_turret")))) {
		Console_Output("[Essentials] Hologram model could not be found, defaulting to \"v_invs_turret\"...\n");
		EssentialsHologramsManager::HologramModel = "v_invs_turret";

		if (!IS_FILE_AVAILABLE(StringFormat("%s.w3d", EssentialsHologramsManager::HologramModel))) {
			Console_Output("[Essentials] Default hologram model could not be found, perhaps it is renamed or deleted?\n");
			hologramsSuccess = false;
		}
	}

	if (!hologramsSuccess && EssentialsHologramsManager::Is_Initialized()) {
		Console_Output("[Essentials] Failed to reload settings for Holograms and feature will be disabled now.\n");
		Console_Output("[Essentials] If you have custom presets or server content, please read configuration for details about how to fix this issue.\n");
		Console_Output("[Essentials] Plugins depending on the Holograms feature may become unstable.\n");
		EssentialsHologramsManager::Shutdown();
	}
	else if (hologramsSuccess && !EssentialsHologramsManager::Is_Initialized()) {
		EssentialsHologramsManager::Init();
		Console_Output("[Essentials] Holograms feature is now enabled.\n");
	}

	EssentialsHologramsManager::Reinitialize_All_Holograms();


	bool EnableAuthentication = DASettingsManager::Get_Bool("Essentials", "EnableAuthentication", false);
	if (EnableAuthentication && !EssentialsAuthenticationManager::Is_Initialized()) {
		EssentialsAuthenticationManager::Init();
		Console_Output("[Essentials] Authentication system is now enabled.\n");
	}
	else if (!EnableAuthentication && EssentialsAuthenticationManager::Is_Initialized()) {
		EssentialsAuthenticationManager::Shutdown();
		Console_Output("[Essentials] Authentication system is now disabled.\n");
	}


	bool EnableCMS = DASettingsManager::Get_Bool("Essentials", "EnableCMS", false);
	if (EnableCMS && !EssentialsCMSManager::Is_Initialized()) {
		EssentialsCMSManager::Init();
		Console_Output("[Essentials] CMS is now enabled.\n");
	}
	else if (!EnableCMS && EssentialsCMSManager::Is_Initialized()) {
		EssentialsCMSManager::Shutdown();
		Console_Output("[Essentials] CMS is now disabled.\n");
	}


	bool BadWordFilter = DASettingsManager::Get_Bool("Essentials", "EnableBadWordFilter", false);
	if (BadWordFilter) {
		if (!EssentialsBadWordFilterClass::Instance) {
			new EssentialsBadWordFilterClass;
			EssentialsBadWordFilterClass::Instance->Settings_Loaded_Event();
		}
	} else {
		if (EssentialsBadWordFilterClass::Instance) {
			delete EssentialsBadWordFilterClass::Instance;
		}
	}


	bool GruntEffect = DASettingsManager::Get_Bool("Essentials", "EnableGruntEffect", false);
	if (GruntEffect) {
		if (!EssentialsGruntEffectClass::Instance) {
			new EssentialsGruntEffectClass;
			EssentialsGruntEffectClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsGruntEffectClass::Instance) {
			delete EssentialsGruntEffectClass::Instance;
		}
	}


	bool Jukebox = DASettingsManager::Get_Bool("Essentials", "EnableJukebox", false);
	if (Jukebox) {
		if (!EssentialsJukeboxClass::Instance) {
			new EssentialsJukeboxClass;
			EssentialsJukeboxClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsJukeboxClass::Instance) {
			delete EssentialsJukeboxClass::Instance;
		}
	}


	bool MapHolograms = DASettingsManager::Get_Bool("Essentials", "EnableMapHolograms", false);
	if (MapHolograms) {
		if (!EssentialsMapHologramsClass::Instance) {
			new EssentialsMapHologramsClass;
			EssentialsMapHologramsClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsMapHologramsClass::Instance) {
			delete EssentialsMapHologramsClass::Instance;
		}
	}


	bool CustomPowerUpSpawners = DASettingsManager::Get_Bool("Essentials", "EnableCustomPowerUpSpawners", false);
	if (CustomPowerUpSpawners) {
		if (!EssentialsCustomPowerUpSpawnersClass::Instance) {
			new EssentialsCustomPowerUpSpawnersClass;
			EssentialsCustomPowerUpSpawnersClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsCustomPowerUpSpawnersClass::Instance) {
			delete EssentialsCustomPowerUpSpawnersClass::Instance;
		}
	}


	bool AutoAnnounce = DASettingsManager::Get_Bool("Essentials", "EnableAutoAnnounce", false);
	if (AutoAnnounce) {
		if (!EssentialsAutoAnnounceClass::Instance) {
			new EssentialsAutoAnnounceClass;
			EssentialsAutoAnnounceClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsAutoAnnounceClass::Instance) {
			delete EssentialsAutoAnnounceClass::Instance;
		}
	}


	bool CustomTag = DASettingsManager::Get_Bool("Essentials", "EnableCustomTag", false);
	if (CustomTag) {
		if (!EssentialsCustomTagClass::Instance) {
			new EssentialsCustomTagClass;
			EssentialsCustomTagClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsCustomTagClass::Instance) {
			delete EssentialsCustomTagClass::Instance;
		}
	}


	bool Voting = DASettingsManager::Get_Bool("Essentials", "EnableVoting", false);
	if (Voting) {
		if (!EssentialsVotingManagerClass::Instance) {
			new EssentialsVotingManagerClass;
			EssentialsVotingManagerClass::Instance->Settings_Loaded_Event();
		}
	}
	else {
		if (EssentialsVotingManagerClass::Instance) {
			delete EssentialsVotingManagerClass::Instance;
		}
	}

	FloodingBlocksChatMessages = DASettingsManager::Get_Bool("Essentials", "FloodingBlocksChatMessages", false);

	ShowConnectionRequestMessages = DASettingsManager::Get_Bool("Essentials", "ShowConnectionRequestMessages", false);
	ShowConnectionLostMessages = DASettingsManager::Get_Bool("Essentials", "ShowConnectionLostMessages", false);
	DASettingsManager::Get_String(ConnectionRequestMessageFormat, "Essentials", "ConnectionRequestMessageFormat", "{NAME} is loading to join server.");
	DASettingsManager::Get_String(ConnectionLostMessageFormat, "Essentials", "ConnectionLostMessageFormat", "{NAME} lost communication to server.");

	INISection* Section = DASettingsManager::Get_Section("EssentialsBlockedSpawnPresets");
	BlockedSpawnPresets->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				BlockedSpawnPresets->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d blocked spawn presets.\n", BlockedSpawnPresets->Count());
	}
	AllowVehicleSpawn = DASettingsManager::Get_Bool("Essentials", "AllowVehicleSpawn", true);
	AllowPowerupSpawn = DASettingsManager::Get_Bool("Essentials", "AllowPowerupSpawn", true);
	AllowSimpleSpawn = DASettingsManager::Get_Bool("Essentials", "AllowSimpleSpawn", true);
	AllowSoldierSpawn = DASettingsManager::Get_Bool("Essentials", "AllowSoldierSpawn", true);

	Section = DASettingsManager::Get_Section("EssentialsBlockedCharacterPresets");
	BlockedCharacterPresets->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				BlockedCharacterPresets->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d blocked character presets.\n", BlockedCharacterPresets->Count());
	}

	Section = DASettingsManager::Get_Section("EssentialsBlockedWeaponPresets");
	BlockedWeaponPresets->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				BlockedWeaponPresets->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d blocked weapon presets.\n", BlockedWeaponPresets->Count());
	}

	Section = DASettingsManager::Get_Section("EssentialsFastSpawnList");
	FastSpawnList->Remove_All();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			DATokenParserClass Parser(Entry->Entry, '|');
			while (char* tok = Parser.Get_String()) {
				FastSpawnList->Insert(tok, Entry->Value);
			}
		}
		Console_Output("[Essentials] Loaded %d fast spawn presets.\n", FastSpawnList->Get_Size());
	}

	Section = DASettingsManager::Get_Section("EssentialsBlockedScripts");
	BlockedScripts->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				BlockedScripts->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d blocked scripts.\n", BlockedScripts->Count());
	}

	Section = DASettingsManager::Get_Section("EssentialsBlockedModels");
	BlockedModels->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				BlockedModels->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d blocked models.\n", BlockedModels->Count());
	}

	MinSpeed = DASettingsManager::Get_Float("Essentials", "MinSpeed", .0f);
	MaxSpeed = DASettingsManager::Get_Float("Essentials", "MaxSpeed", 100.f);

	Section = DASettingsManager::Get_Section("EssentialsReviveBuildingAliases");
	ReviveBuildingAliases->Remove_All();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			DATokenParserClass Parser(Entry->Entry, '|');
			while (char* tok = Parser.Get_String()) {
				ReviveBuildingAliases->Insert(tok, Entry->Value);
			}
		}
		Console_Output("[Essentials] Loaded %d building aliases.\n", ReviveBuildingAliases->Get_Size());
	}

	DASettingsManager::Get_String(GodModePreset, "Essentials", "GodModePreset", "Walk-Thru");
	GodModeSpeed = DASettingsManager::Get_Float("Essentials", "GodModeSpeed", 25.f);
	Section = DASettingsManager::Get_Section("EssentialsGodModeWeapons");
	GodModeWeapons->Clear();
	if (Section) {
		for (INIEntry* Entry = Section->EntryList.First(); Entry && Entry->Is_Valid(); Entry = Entry->Next()) {
			if (!strcmp(Entry->Value, "1")) {
				GodModeWeapons->Add(Entry->Entry);
			}
		}
		Console_Output("[Essentials] Loaded %d god mode weapons.\n", GodModeWeapons->Count());
	}
	MakeGodModeSpy = DASettingsManager::Get_Bool("Essentials", "MakeGodModeSpy", true);
	MakeGodModeStealth = DASettingsManager::Get_Bool("Essentials", "MakeGodModeStealth", true);
	
	CHATCMD_STNG_MT(Fly);
	CHATCMD_STNG_MT(Spectate);
	CHATCMD_STNG_ST(SpawnObject);
	CHATCMD_STNG_MT(ChangeCharacter);
	CHATCMD_STNG_MT(GiveWeapon);
	CHATCMD_STNG_ST(PrintFDS);
	CHATCMD_STNG_ST(FastSpawn);
	CHATCMD_STNG_MT(GiveMoney);
	CHATCMD_STNG_ST(GoTo);
	CHATCMD_STNG_ST(Put);
	CHATCMD_STNG_MT(PutAll);
	CHATCMD_STNG_MT(PutAllLine);
	CHATCMD_STNG_MT(PutAllCircle);
	CHATCMD_STNG_MT(SetHealth);
	CHATCMD_STNG_MT(SetArmor);
	CHATCMD_STNG_MT(SetMaxHealth);
	CHATCMD_STNG_MT(SetMaxArmor);
	CHATCMD_STNG_MT(SetClipAmmo);
	CHATCMD_STNG_MT(SetInventoryAmmo);
	CHATCMD_STNG_MT(SetObjectTeam);
	CHATCMD_STNG_MT(AttachScript);
	CHATCMD_STNG_MT(DetachScript);
	CHATCMD_STNG_MT(ChangeModel);
	CHATCMD_STNG_MT(GetPosition);
	CHATCMD_STNG_MT(SetSpeed);
	CHATCMD_STNG_ST(ReviveBuilding);
	CHATCMD_STNG_MT(GodMode);
	CHATCMD_STNG_ST(Screenshot);
	CHATCMD_STNG_MT(GetModelName);
	CHATCMD_STNG_MT(GetPresetName);
	CHATCMD_STNG_ST(Sudo);
	CHATCMD_STNG_ST(SudoTeam);
	CHATCMD_STNG_ST(ToggleGameplay);
	CHATCMD_STNG_ST(ReloadLevel);
	CHATCMD_STNG_ST(ReloadClient);

	Unregister_Chat_Command("!fly");
	Unregister_Chat_Command("!spectate");
	Unregister_Chat_Command("!spawnobj");
	Unregister_Chat_Command("!changechar");
	Unregister_Chat_Command("!giveweap");
	Unregister_Chat_Command("!printfds");
	Unregister_Chat_Command("!fastspawn");
	Unregister_Chat_Command("!givemoney");
	Unregister_Chat_Command("!goto");
	Unregister_Chat_Command("!put");
	Unregister_Chat_Command("!putall");
	Unregister_Chat_Command("!putallline");
	Unregister_Chat_Command("!putallcircle");
	Unregister_Chat_Command("!sethealth");
	Unregister_Chat_Command("!setarmor");
	Unregister_Chat_Command("!setmaxhealth");
	Unregister_Chat_Command("!setmaxarmor");
	Unregister_Chat_Command("!setclipammo");
	Unregister_Chat_Command("!setinvammo");
	Unregister_Chat_Command("!setobjteam");
	Unregister_Chat_Command("!attach");
	Unregister_Chat_Command("!detach");
	Unregister_Chat_Command("!changemodel");
	Unregister_Chat_Command("!getposition");
	Unregister_Chat_Command("!setspeed");
	Unregister_Chat_Command("!revivebld");
	Unregister_Chat_Command("!godmode");
	Unregister_Chat_Command("!screenshot");
	Unregister_Chat_Command("!getmodelname");
	Unregister_Chat_Command("!getpresetname");
	Unregister_Chat_Command("!sudo");
	Unregister_Chat_Command("!sudoteam");
	Unregister_Chat_Command("!togglegameplay");
	Unregister_Chat_Command("!reloadlevel");
	Unregister_Chat_Command("!reloadclient");
	
	CHATCMD_REG(EssentialsEventClass, Fly, "!fly", 0);
	CHATCMD_REG(EssentialsEventClass, Spectate, "!spectate|!specmode|!spec", 0);
	CHATCMD_REG(EssentialsEventClass, SpawnObject, "!spawnobj|!spawn|!spobj|!sp", 1);
	CHATCMD_REG(EssentialsEventClass, ChangeCharacter, "!changechar|!cchar|!cc", 1);
	CHATCMD_REG(EssentialsEventClass, GiveWeapon, "!giveweap|!gweap|!gw", 1);
	CHATCMD_REG(EssentialsEventClass, PrintFDS, "!printfds|!fds", 1);
	CHATCMD_REG(EssentialsEventClass, FastSpawn, "!fastspawn|!fastsp|!fsp", 1);
	CHATCMD_REG(EssentialsEventClass, GiveMoney, "!givemoney|!money|!gm", 1);
	CHATCMD_REG(EssentialsEventClass, GoTo, "!goto", 1);
	CHATCMD_REG(EssentialsEventClass, Put, "!put", 1);
	CHATCMD_REG(EssentialsEventClass, PutAll, "!putall", 0);
	CHATCMD_REG(EssentialsEventClass, PutAllLine, "!putallline", 1);
	CHATCMD_REG(EssentialsEventClass, PutAllCircle, "!putallcircle", 1);
	CHATCMD_REG(EssentialsEventClass, SetHealth, "!sethealth|!shealth|!sh", 1);
	CHATCMD_REG(EssentialsEventClass, SetArmor, "!setarmor|!sarmor|!sa", 1);
	CHATCMD_REG(EssentialsEventClass, SetMaxHealth, "!setmaxhealth|!smhealth|!smh", 1);
	CHATCMD_REG(EssentialsEventClass, SetMaxArmor, "!setmaxarmor|!smarmor|!sma", 1);
	CHATCMD_REG(EssentialsEventClass, SetClipAmmo, "!setclipammo|!scammo|!sca", 1);
	CHATCMD_REG(EssentialsEventClass, SetInventoryAmmo, "!setinvammo|!siammo|!sia", 1);
	CHATCMD_REG(EssentialsEventClass, SetObjectTeam, "!setobjteam|!soteam|!st", 1);
	CHATCMD_REG(EssentialsEventClass, AttachScript, "!attach", 2);
	CHATCMD_REG(EssentialsEventClass, DetachScript, "!detach", 2);
	CHATCMD_REG(EssentialsEventClass, ChangeModel, "!changemodel|!setmodel|!chmodel|!smodel|!sm", 1);
	CHATCMD_REG(EssentialsEventClass, GetPosition, "!getposition|!getpos|!gp", 0);
	CHATCMD_REG(EssentialsEventClass, SetSpeed, "!setspeed|!sspeed|!ssp", 1);
	CHATCMD_REG(EssentialsEventClass, ReviveBuilding, "!revivebld|!revive|!revbld|!rb", 1);
	CHATCMD_REG(EssentialsEventClass, GodMode, "!godmode|!god", 0);
	CHATCMD_REG(EssentialsEventClass, Screenshot, "!screenshot|!screenie|!sshot|!ss", 1);
	CHATCMD_REG(EssentialsEventClass, GetModelName, "!getmodelname|!mdlname|!gmn", 0);
	CHATCMD_REG(EssentialsEventClass, GetPresetName, "!getpresetname|!prsname|!gpn", 0);
	CHATCMD_REG(EssentialsEventClass, Sudo, "!sudo", 1);
	CHATCMD_REG(EssentialsEventClass, SudoTeam, "!sudoteam|!sudot", 1);
	CHATCMD_REG(EssentialsEventClass, ToggleGameplay, "!togglegameplay|!gameplay|!togglegp|!tgp|!sgp", 0);
	CHATCMD_REG(EssentialsEventClass, ReloadLevel, "!reloadlevel|!reloadmap|!restartlevel||!rellvl", 0);
	CHATCMD_REG(EssentialsEventClass, ReloadClient, "!reloadclient|!relclient|!relcli|!rc", 1);
}

bool EssentialsEventClass::Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID) {
	if (FloodingBlocksChatMessages) {
		if (!Player->Is_Flooding() && !Player->Get_DA_Player()->Is_Muted()) {
			// Player->Increment_Flood_Counter();
			return true;
		} else {
			return false;
		}
	} else {
		return true;
	}
}

ConnectionAcceptanceFilter::STATUS EssentialsEventClass::Connection_Request_Event(ConnectionRequest& Request, WideStringClass& RefusalMessage) {
	StringClass Line, Hash;
	if (!Request.password.Get_Length() || !Get_MD5_Hash(Request.password.Peek_Buffer(), Hash))
	{
		Hash = "00000000000000000000000000000000";
	}
	Line.Format("%ws requested to join server. [id=%d,ip=%s,ser=%s,ver=%f,rev=%d,key=%d,pwd=%s]", Request.clientName.Peek_Buffer(), Request.clientId, Long_To_IP(Request.clientAddress.sin_addr.s_addr).Peek_Buffer(), Request.clientSerialHash.Peek_Buffer(), Request.clientVersion, Request.clientRevisionNumber, Request.clientExeKey, Hash);
	DALogManager::Write_Log("_CONNECTION", Line.Peek_Buffer());

	if (ShowConnectionRequestMessages) {
		StringClass Message(ConnectionRequestMessageFormat);
		Message.Replace("{ID}", StringFormat("%d", Request.clientId));
		Message.Replace("{NAME}", StringClass(Request.clientName.Peek_Buffer()));
		Message.Replace("{VERSION}", StringFormat("%.2f", Request.clientVersion));
		Message.Replace("{REVISION}", StringFormat("%d", Request.clientRevisionNumber));

		DA::Host_Message(Message.Peek_Buffer());
	}

	return ConnectionAcceptanceFilter::STATUS_ACCEPTING;
}

void EssentialsEventClass::Ren_Log_Event(const char* Output) {
	if (ShowConnectionLostMessages) {
		if (stristr(Output, "Connection broken to client.")) {
			char* pIdPtr = const_cast<char*>(Output) + 29;
			int playerId = atoi(pIdPtr);

			if (pIdPtr > 0) {
				WideStringClass Name;
				for (SLNode<cPlayer>* z = Get_Player_List()->Head(); z; z = z->Next()) {
					cPlayer* Player = z->Data();
					if (Player->Is_Active() && Player->Get_Id() == playerId) {
						Name = Player->Get_Name();
						break;
					}
				}

				if (Name.Is_Empty())
					return;

				StringClass Message(ConnectionLostMessageFormat);
				Message.Replace("{ID}", StringFormat("%d", playerId));
				Message.Replace("{NAME}", StringClass(Name));

				DA::Host_Message(Message.Peek_Buffer());
			}
		}
	}
}

bool EssentialsEventClass::Damage_Request_Event(DamageableGameObj* Victim, ArmedGameObj* Damager, float& Damage, unsigned& Warhead, float Scale, DADamageType::Type Type) {
	if (Type == DADamageType::REPAIR) {
		return AllowBuildingRepair;
	}
	return true;
}

extern "C" {
	__declspec(dllexport) void Plugin_Init() {
		new EssentialsEventClass;
		EssentialsAuthenticationManager::Pre_Init();
		DASettingsManager::Add_Settings("Essentials.ini");
	}

	__declspec(dllexport) void Plugin_Shutdown() {
		delete EssentialsEventClass::Instance;
	}
}
