/*
    Essentials - Essential features for servers
    Copyright (C) 2022 Unstoppable

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

extern bool Make_Spectator(cPlayer* Player);
extern GameObject* Spawn_Object(cPlayer* Player, unsigned long DefinitionID);
extern bool Teleport_Player(cPlayer* Player, cPlayer* Target);
extern bool Teleport_Player(cPlayer* Player, PhysicalGameObj* Object);
extern StringClass Format_Seconds(int Seconds, bool AppendUnit = true);
extern StringClass Format_Seconds_Friendly(int Seconds);
extern bool Get_MD5_Hash(StringClass text, StringClass& out);
extern FileClass* Create_Or_Get_Essentials_Data_File(StringClass name, int mode = 0);
extern bool Ensure_Essentials_Data_Folder_Exists(StringClass name);
extern bool String_Contains(StringClass first, StringClass second);