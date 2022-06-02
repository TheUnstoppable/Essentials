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

class EssentialsMapHologramsClass : public DAEventClass {
	friend class EssentialsHologramDataClass;

public:
	static EssentialsMapHologramsClass* Instance;

	EssentialsMapHologramsClass();
	~EssentialsMapHologramsClass();

	void Settings_Loaded_Event() override;

protected:
	void Clear_Holograms();

private:
	DynamicVectorClass<StringClass> Holograms;
};

