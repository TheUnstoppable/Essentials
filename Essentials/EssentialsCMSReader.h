/*
	Essentials - Essential features for servers
	Copyright (C) 2022 Unstoppable

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
*/

#pragma once

#include "RawFileClass.h"

class EssentialsCMSReader {
public:
	EssentialsCMSReader(const char* file);
	~EssentialsCMSReader();

	bool Exists();
	bool Next_Line();

	bool Next_Token(StringClass& buf);
	bool Next_Token_As_Int(int& buf);
	bool Next_Token_As_Float(float& buf);
	bool Next_Token_As_Vector2(Vector2& buf);
	bool Next_Token_As_Vector3(Vector3& buf);
	bool Next_Token_As_Color(Vector3& buf);
	bool Remaining_Text(StringClass& buf);

	int Line_Length();
	int Line_Index();
	bool Is_Line_Empty();

private:
	EssentialsCMSReader();

protected:
	TextFileClass* File;
	StringClass CurrentLine;
	int LineIndex;
	int LineCharIndex;
};
