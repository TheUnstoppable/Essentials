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
