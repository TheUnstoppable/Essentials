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
#include "da_settings.h"
// Custom
#include "EssentialsHolograms.h"

StringClass EssentialsHologramsManager::HologramPreset = "Walk-Thru";
StringClass EssentialsHologramsManager::HologramModel = "v_invs_turret";
DynamicVectorClass<EssentialsHologramDataClass*>* EssentialsHologramsManager::Holograms = 0;

EssentialsHologramDataClass::EssentialsHologramDataClass(StringClass Name, Vector3 Pos) : Name(Name), Position(Pos), Text(WideStringClass()), Color(EssentialsHologramColor::COLOR_NEUTRAL) {

}

EssentialsHologramDataClass::~EssentialsHologramDataClass() {
	if (Object) {
		Object->Set_Delete_Pending();
	}
}

void EssentialsHologramDataClass::Reinitialize_Object() {
	if (Object) {
		Object->Set_Delete_Pending();
	}

	if (SoldierGameObjDef* Def = (SoldierGameObjDef*)Find_Named_Definition(EssentialsHologramsManager::HologramPreset)) {
		Object = Commands->Create_Object(EssentialsHologramsManager::HologramPreset, Position);
		Set_Object_Type(Object, Color);
		Commands->Set_Player_Type(Object, Color);
		((SoldierGameObj*)Object.Get_Ptr())->Re_Init(*Def);
		((SoldierGameObj*)Object.Get_Ptr())->Post_Re_Init();

		Commands->Set_Model(Object, EssentialsHologramsManager::HologramModel);
		Lock_Soldier_Collision_Group(Object, UNCOLLIDEABLE_GROUP);
		if (!((SoldierGameObj*)Object.Get_Ptr())->Get_Fly_Mode())
			((SoldierGameObj*)Object.Get_Ptr())->Toggle_Fly_Mode();
		((SoldierGameObj*)Object.Get_Ptr())->Set_Bot_Tag(StringClass(Text));
		((SoldierGameObj*)Object.Get_Ptr())->Set_Is_Visible(false);
		Object->Add_Observer(new EssentialsHologramObjectGameObjObserverClass);
	}
}

bool EssentialsHologramObjectGameObjObserverClass::Damage_Received_Request(ArmedGameObj* Damager, float& Damage, unsigned& Warhead, float Scale, DADamageType::Type Type) {
	return false;
}

const StringClass& EssentialsHologramDataClass::Get_Hologram_Name() {
	return Name;
}

EssentialsHologramColor::HologramColor EssentialsHologramDataClass::Get_Hologram_Color() {
	return Color;
}

const WideStringClass& EssentialsHologramDataClass::Get_Hologram_Text() {
	return Text;
}

const Vector3& EssentialsHologramDataClass::Get_Position() {
	return Position;
}

void EssentialsHologramDataClass::Set_Hologram_Color(EssentialsHologramColor::HologramColor NewColor) {
	Color = newColor;
	Reinitialize_Object();
}

void EssentialsHologramDataClass::Set_Hologram_Text(const wchar_t* NewText) {
	Text = newText;
	Reinitialize_Object();
}

void EssentialsHologramDataClass::Set_Position(const Vector3& NewPosition) {
	Position = newPosition;
	Reinitialize_Object();
}


void EssentialsHologramsManager::Init() {
	Holograms = new DynamicVectorClass<EssentialsHologramDataClass*>;
}

void EssentialsHologramsManager::Shutdown() {
	Clear_Holograms();
	delete Holograms;
	Holograms = 0;
}

bool EssentialsHologramsManager::Is_Initialized() {
	return !!Holograms;
}

EssentialsHologramDataClass* EssentialsHologramsManager::Create_Hologram(const char* Name, const wchar_t* Text, const EssentialsHologramColor::HologramColor Color, const Vector3& Position) {
	if (!Holograms) {
		return 0;
	}

	if(EssentialsHologramDataClass* Hologram = Find_Hologram(Name)) {
		Delete_Hologram(Hologram);
	}

	EssentialsHologramDataClass* Hologram = new EssentialsHologramDataClass(Name, Position);
	Hologram->Text = Text;
	Hologram->Color = Color;
	Hologram->Reinitialize_Object();
	Holograms->Add(Hologram);
	return Hologram;
}

EssentialsHologramDataClass* EssentialsHologramsManager::Find_Hologram(const char* Name) {
	if (!Holograms) {
		return 0;
	}

	for(int i = 0; i < Holograms->Count(); ++i) {
		EssentialsHologramDataClass* Hologram = (*Holograms)[i];
		if (Hologram->Get_Hologram_Name() == Name) {
			return Hologram;
		}
	}

	return 0;
}

bool EssentialsHologramsManager::Delete_Hologram(EssentialsHologramDataClass* Hologram) {
	if (!Holograms) {
		return false;
	}

	if (Hologram) {
		Holograms->DeleteObj(Hologram);
		delete Hologram;
		return true;
	}

	return false;
}

bool EssentialsHologramsManager::Delete_Hologram(const char* Name) {
	if (!Holograms) {
		return false;
	}

	if (EssentialsHologramDataClass* Hologram = Find_Hologram(Name)) {
		Holograms->DeleteObj(Hologram);
		delete Hologram;
		return true;
	}

	return false;
}

void EssentialsHologramsManager::Clear_Holograms() {
	if (!Holograms) {
		return;
	}

	for (int i = 0; i < Holograms->Count(); ++i) {
		delete (*Holograms)[i];
	}

	Holograms->Delete_All();
}

void EssentialsHologramsManager::Reinitialize_All_Holograms() {
	if (!Holograms) {
		return;
	}

	for (int i = 0; i < Holograms->Count(); ++i) {
		(*Holograms)[i]->Reinitialize_Object();
	}
}
