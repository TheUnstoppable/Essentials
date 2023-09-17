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
#include "engine_da.h"
// Custom
#include "EssentialsCMSDefinition.h"
#include "EssentialsCMSView.h"

EssentialsCMSDefinition::~EssentialsCMSDefinition() {
	while(auto data = Content.Remove_Tail()) {
		delete data;
	}

	while (auto data = Triggers.Remove_Tail()) {
		delete data;
	}
}

EssentialsCMSView* EssentialsCMSDialogDefinition::Create_Instance() {
	return new EssentialsCMSDialogView(this);
}

EssentialsCMSView* EssentialsCMSChatDefinition::Create_Instance() {
	return new EssentialsCMSChatView(this);
}