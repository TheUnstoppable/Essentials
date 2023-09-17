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
#include "da.h"
// Custom
#include "EssentialsUtils.h"
#include "EssentialsCMSUtils.h"

StringClass Process_Placeholders(const StringClass& Data, int PlayerID) {
	StringClass New = Data;
	New.Replace("{PLAYER}", Get_String_Player_Name_By_ID(PlayerID));
	New.Replace("{TITLE}", StringClass(The_Game()->Get_Game_Title().Peek_Buffer()));
	New.Replace("{MAP}", The_Game()->Get_Map_Name());
	return New;
}