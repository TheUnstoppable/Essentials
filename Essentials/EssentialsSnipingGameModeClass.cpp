#include "General.h"
#include "da.h"
#include "da_translation.h"
#include "da_settings.h"
#include "da_levelcleaner.h"
#include "engine_da.h"
#include "EssentialsSnipingGameModeClass.h"
#include "EssentialsHolograms.h"
#include "GameObjManager.h"

void EssentialsSnipingGameModeClass::Init() {
	Register_Event(DAEvent::SETTINGSLOADED);
	Register_Object_Event(DAObjectEvent::CREATED, DAObjectEvent::PLAYER);
	Register_Object_Event(DAObjectEvent::KILLDEALT, DAObjectEvent::PLAYER);
	Register_Object_Event(DAObjectEvent::POKE, DAObjectEvent::ALL);

	memset(SniperClasses, 0, sizeof(SniperClasses));
	memset(AdvancedTerminals, 0, sizeof(AdvancedTerminals));
	memset(BaseZones, 0, sizeof(BaseZones));
	memset(Spawners, 0, sizeof(Spawners));

	Cleaned = false;
}

EssentialsSnipingGameModeClass::~EssentialsSnipingGameModeClass() {
	Reset();
}

void EssentialsSnipingGameModeClass::Settings_Loaded_Event() {
	const DASettingsClass* Settings = DASettingsManager::Get_Settings(DASettingsManager::Get_Settings_Count() - 1);

	if (!Cleaned) {
		Cleaned = true;
		DALevelCleaner::Clean_Level();
		DALevelCleaner::Load_Blockers(Settings->Get_INI());
	}

	SpawnMoney = Settings->Get_Float("SpawnMoney", .0f);
	MoneyPerKill = Settings->Get_Float("MoneyPerKill", .0f);
	MoneyIncrementPerPlayer = Settings->Get_Float("MoneyIncrementPerPlayer", .0f);
	DisableScreenEffects = Settings->Get_Bool("DisableScreenEffects", false);
	Prices[0] = Settings->Get_Float("BasicSniperPrice", 500.f);
	Prices[1] = Settings->Get_Float("AdvancedSniperPrice", 1000.f);

	for (int i = 0; i < 2; ++i) {
		const char* TeamName = Get_Team_Name(i);

		/*
			Load presets.
		*/
		for (int j = 0; j < 2; ++j) {
			SoldierGameObjDef* SoldierDef = (SoldierGameObjDef*)Find_Definition(j ? (i ? 81930246 : 81930260) : (i ? 81930245 : 81930259));
			StringClass PresetBuffer;
			if (Settings->Get_String(PresetBuffer, StringFormat("%sSniperClass%s", j ? "Advanced" : "Basic", TeamName), 0)) {
				if (DefinitionClass* Def = Find_Named_Definition(PresetBuffer)) {
					if (Def->Get_Class_ID() == CID_Soldier) {
						SoldierDef = (SoldierGameObjDef*)Def;
					}
				}
			}
			SniperClasses[i][j] = SoldierDef;
		}
		
		/*
			Load purchase terminals.
		*/
		for (int j = 0; j < 2; ++j) {
			Vector3 PTPosition;
			float PTFacing;
			Settings->Get_Vector3(PTPosition, StringFormat("%sPurchaseTerminal%s", j ? "Advanced" : "Basic", TeamName), Vector3(0, 0, 0));
			PTFacing = Settings->Get_Float(StringFormat("%sPurchaseTerminal%s_Facing", j ? "Advanced" : "Basic", TeamName), .0f);
			PhysicalGameObj* Terminal = Create_Object(81960072, PTPosition);
			Commands->Set_Facing(Terminal, PTFacing);
			Commands->Enable_HUD_Pokable_Indicator(Terminal, true);
			Set_Object_Type(Terminal, i);
			AdvancedTerminals[i][j] = Terminal;
			PTPosition.Z -= 0.25f;
			EssentialsHologramsManager::Create_Hologram(StringFormat("Sniping%sPTHolo%s", j ? "Advanced" : "Basic", TeamName), WideStringFormat(L"Poke this terminal to\nbuy %s sniper.", j ? L"advanced" : L"basic"), (EssentialsHologramColor::HologramColor)i, PTPosition);
		}

		/*
			Load base zones.
		*/
		for (int j = 0; j < 2; ++j) {
			Vector3 BottomCorner, TopCorner;
			Settings->Get_Vector3(BottomCorner, StringFormat("Base%sZone%s_Bottom", j ? "Warning" : "Kill", TeamName), Vector3(0,0,0));
			Settings->Get_Vector3(TopCorner, StringFormat("Base%sZone%s_Top", j ? "Warning" : "Kill", TeamName), Vector3(0, 0, 0));

			AABoxClass box;
			box.Init(LineSegClass(BottomCorner, TopCorner));
			BaseZones[i][j] = box;
		}

		/*
			Load spawners.
		*/
		Settings->Get_Vector3(Spawners[i], StringFormat("Spawner%s", TeamName), Vector3(0,0,0));

		delete[] TeamName;
	}
}

void EssentialsSnipingGameModeClass::Game_Over_Event() {
	Reset();
}

void EssentialsSnipingGameModeClass::Object_Created_Event(GameObject* obj) {
	int Team = Get_Object_Type(obj);
	if (Team != 0 && Team != 1) {
		return;
	}

	uint32 ObjDefID = obj->Get_Definition().Get_ID();
	if (ObjDefID != SniperClasses[Team][0]->Get_ID() && ObjDefID != SniperClasses[Team][1]->Get_ID())
	{
		SoldierGameObj* Soldier = obj->As_SoldierGameObj();
		Soldier->Re_Init(*SniperClasses[Team][0]);
		Soldier->Post_Re_Init();
		Soldier->Set_Position(Spawners[Team]);
		Fix_Stuck_Object(Soldier, 7.0f);

		Soldier->Get_Player()->Set_Money(SpawnMoney);
	}

	if (!Is_Timer(1001)) {
		Start_Timer(1001, .1f, false, 0);
	}
}

void EssentialsSnipingGameModeClass::Kill_Event(DamageableGameObj* Victim, ArmedGameObj* Killer, float Damage, unsigned int Warhead, float Scale, DADamageType::Type Type) {
	if (Commands->Is_A_Star(Victim)) {
		((SoldierGameObj*)Killer)->Get_Player()->Increment_Money(MoneyPerKill + (MoneyIncrementPerPlayer * The_Game()->Get_Current_Players()));
	}
}

void EssentialsSnipingGameModeClass::Poke_Event(cPlayer* Player, PhysicalGameObj* obj) {
	int Team = Get_Object_Type(Player->Get_GameObj());
	if (Team != 0 && Team != 1) {
		return;
	}

	int DesiredClass = -1;
	if (AdvancedTerminals[Team][0] == obj) { // Wants to buy Basic character.
		DesiredClass = 0;
	}
	else if (AdvancedTerminals[Team][1] == obj) { // Wants to buy Advanced character.
		DesiredClass = 1;
	}
	else {
		return;
	}

	SoldierGameObj* Soldier = Player->Get_GameObj();
	if (Soldier->Get_Definition().Get_ID() != SniperClasses[Team][DesiredClass]->Get_ID()) {
		if (Player->Get_Money() >= Prices[DesiredClass]) {
			Soldier->Re_Init(*SniperClasses[Team][DesiredClass]);
			Soldier->Post_Re_Init();

			Player->Increment_Money(-Prices[DesiredClass]);
			DA::Private_Color_Message(Player, COLORGRAY, "Purchase request granted.");
			DA::Create_2D_Sound_Player(Player, "m00gbmg_secx0002i1gbmg_snd.wav");
		}
		else {
			DA::Private_Color_Message(Player, COLORGRAY, "You have insufficient funds for this purchase.");
			if (Team == 0) {
				DA::Create_2D_Sound_Player(Player, "m00evan_dsgn0024i1evan_snd.wav");
			}
			else {
				DA::Create_2D_Sound_Player(Player, "m00evag_dsgn0028i1evag_snd.wav");
			}
		}
	}
	else {
		DA::Private_Color_Message(Player, COLORGRAY, "You already own %s.", a_or_an_Prepend(DATranslationManager::Translate(Soldier)));
	}
}

void EssentialsSnipingGameModeClass::Timer_Expired(int Number, unsigned Data) {
	if (Number == 1001) {
		for(SLNode<SoldierGameObj>* n = GameObjManager::StarGameObjList.Head(); n; n = n->Next()) {
			SoldierGameObj* obj = n->Data();
			int team = Get_Object_Type(obj);

			if (Is_Inside_AABox(BaseZones[PTTEAM(team)][0], Commands->Get_Position(obj))) {
				DA::Page_Player(obj, "You have been killed for entering the enemy base.");
				obj->Set_Delete_Pending();
			}
			else if (Is_Inside_AABox(BaseZones[PTTEAM(team)][1], Commands->Get_Position(obj))) {
				if (!Is_Timer(9991, obj->Get_Player()->Get_Id())) {
					DA::Page_Player(obj, "Don't go into the enemy base, otherwise you will be killed.");
					Start_Timer(9991, 10.f, false, obj->Get_Player()->Get_Id());
				}
				if (!DisableScreenEffects) {
					Set_Screen_Fade_Color_Player(obj, 1.f, 0.f, 0.f, 0.f);
					Set_Screen_Fade_Opacity_Player(obj, .5f, 1.f);
				}
			}
			else {
				if (!DisableScreenEffects) {
					Set_Screen_Fade_Opacity_Player(obj, .0f, .5f);
				}
			}
		}
		if (The_Game()->Get_Current_Players() > 0) {
			Start_Timer(1001, .1f, false, Data);
		}
	}
}

void EssentialsSnipingGameModeClass::Reset() {
	if (AdvancedTerminals[0][0]) {
		AdvancedTerminals[0][0]->Set_Delete_Pending();
	}
	if (AdvancedTerminals[0][1]) {
		AdvancedTerminals[0][1]->Set_Delete_Pending();
	}
	if (AdvancedTerminals[1][0]) {
		AdvancedTerminals[1][0]->Set_Delete_Pending();
	}
	if (AdvancedTerminals[1][1]) {
		AdvancedTerminals[1][1]->Set_Delete_Pending();
	}

	memset(AdvancedTerminals, 0, sizeof(AdvancedTerminals));

	EssentialsHologramsManager::Delete_Hologram("SnipingBasicPTHoloGDI");
	EssentialsHologramsManager::Delete_Hologram("SnipingBasicPTHoloNod");
	EssentialsHologramsManager::Delete_Hologram("SnipingAdvancedPTHoloGDI");
	EssentialsHologramsManager::Delete_Hologram("SnipingAdvancedPTHoloNod");
}

Register_Game_Mode(EssentialsSnipingGameModeClass, "Snipers", "Snipers", 0);
