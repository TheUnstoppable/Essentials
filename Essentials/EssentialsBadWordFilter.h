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

class EssentialsBadWordFilterClass : public DAEventClass {
public:
	static EssentialsBadWordFilterClass* Instance;

	EssentialsBadWordFilterClass();
	~EssentialsBadWordFilterClass();
	void Settings_Loaded_Event() override;
	bool Chat_Event(cPlayer* Player, TextMessageEnum Type, const wchar_t* Message, int ReceiverID) override;
private:
	int MaxWarns;
	bool ReplaceSpaces;
	StringClass WarnMessage;
	StringClass KickMessage;
	DynamicVectorClass<StringClass> BadWords;
};