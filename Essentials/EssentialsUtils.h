/*
    Essentials - Essential features for servers
    Copyright (C) 2022 Unstoppable

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

class EssentialsUtils {
public:
	static bool Make_Spectator(cPlayer* Player);
	static GameObject* Spawn_Object(cPlayer* Player, unsigned long DefinitionID);
	static bool Teleport_Player(cPlayer* Player, cPlayer* Target);
	static bool Teleport_Player(cPlayer* Player, PhysicalGameObj *Object);
	static StringClass Format_Seconds(int Seconds, bool AppendUnit = true);
};
