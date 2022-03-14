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

class EssentialsGruntEffectClass : public DAEventClass {
public:
	static EssentialsGruntEffectClass* Instance;

	EssentialsGruntEffectClass();
	~EssentialsGruntEffectClass();
	void Damage_Event(DamageableGameObj* Victim, ArmedGameObj* Damager, float Damage, unsigned Warhead, float Scale, DADamageType::Type Type) override;
	void Timer_Expired(int Number, unsigned Data) override;
};

