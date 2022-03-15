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
#include "da_gameobj.h"
#include "EssentialsEventClass.h"

namespace EssentialsHologramColor {
	typedef enum {
		COLOR_RENEGADE = -1, // White
		COLOR_NOD = 0, // Red
		COLOR_GDI = 1, // Yellow
		COLOR_NEUTRAL = 2 // Gray
	} HologramColor;
}

class EssentialsHologramObjectGameObjObserverClass : public DAGameObjObserverClass {
public:
	const char* Get_Name() override { return "EssentialsHologramObjectGameObjObserverClass"; }
	bool Damage_Received_Request(ArmedGameObj* Damager, float& Damage, unsigned& Warhead, float Scale, DADamageType::Type Type) override;
};

class ESSENTIALS_API EssentialsHologramDataClass {
	friend class EssentialsHologramsManager;

private:
	EssentialsHologramDataClass(StringClass Name, Vector3 Position);
	~EssentialsHologramDataClass();

public:
	const StringClass& Get_Hologram_Name();
	EssentialsHologramColor::HologramColor Get_Hologram_Color();
	const WideStringClass& Get_Hologram_Text();
	const Vector3& Get_Position();

	void Set_Hologram_Color(EssentialsHologramColor::HologramColor NewColor);
	void Set_Hologram_Text(const wchar_t* NewText);
	void Set_Position(const Vector3& NewPosition);
	void Reinitialize_Object();

private:
	StringClass Name;
	ReferencerClass Object;
	WideStringClass Text;
	EssentialsHologramColor::HologramColor Color;
	Vector3 Position;
};

class ESSENTIALS_API EssentialsHologramsManager {
	friend class EssentialsHologramDataClass;
	friend class EssentialsEventClass;

public:
	static void Init();
	static void Shutdown();
	static bool Is_Initialized();

	static EssentialsHologramDataClass* Create_Hologram(const char* Name, const wchar_t* Text, const EssentialsHologramColor::HologramColor Color, const Vector3& Position);
	static EssentialsHologramDataClass* Find_Hologram(const char* Name);
	static bool Delete_Hologram(EssentialsHologramDataClass* Hologram);
	static bool Delete_Hologram(const char* Name);

protected:
	static void Clear_Holograms();
	static void Reinitialize_All_Holograms();

private:
	static StringClass HologramPreset;
	static StringClass HologramModel;
	static DynamicVectorClass<EssentialsHologramDataClass*>* Holograms;
};
