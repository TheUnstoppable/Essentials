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

#define CUSTOMTAG_DBVERSION (char)1
#define CUSTOMTAG_DATAHEADER 0x47415445U
#define CUSTOMTAG_DATAENTRY (char)1

struct EssentialsCustomTag {
    WideStringClass PlayerName;
    WideStringClass CustomTag;
};

class EssentialsCustomTagClass : public DAEventClass {
public:
    static EssentialsCustomTagClass* Instance;

    EssentialsCustomTagClass();
    ~EssentialsCustomTagClass() override;

    void Game_Over_Event() override;
    void Settings_Loaded_Event() override;
    void Player_Join_Event(cPlayer* Player) override;

    bool CustomTag_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);
    bool RemoveCustomTag_Command(cPlayer* Player, const DATokenClass& Text, TextMessageEnum ChatType);

protected:
    void Add_Tag(const WideStringClass& Name, const WideStringClass& Tag);
    EssentialsCustomTag* Find_Tag(const WideStringClass& Name);
	bool Remove_Tag(const WideStringClass& Name);
    void Apply_Tag(const WideStringClass& Name);
    void Clear_Tags();
    void Load_Tags();
    void Save_Tags();

private:
    bool SettingsDirty;
    SList<EssentialsCustomTag> CustomTags;
};
