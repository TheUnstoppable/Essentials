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

class EssentialsAutoAnnounceClass : public DAEventClass {
public:
	static EssentialsAutoAnnounceClass* Instance;

	EssentialsAutoAnnounceClass();
	~EssentialsAutoAnnounceClass();

	void Settings_Loaded_Event() override;
	void Timer_Expired(int Number, unsigned int Data) override;

protected:
	void Reset_Timer();

private:
	float AnnounceDelay;
	bool HostAnnounce;
	Vector3 AnnounceColor;
	int AnnounceIndex;
	DynamicVectorClass<StringClass> AnnounceTexts;
};

