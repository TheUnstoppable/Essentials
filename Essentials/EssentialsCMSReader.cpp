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
// Custom
#include "EssentialsCMSReader.h"
#include "EssentialsUtils.h"

EssentialsCMSReader::EssentialsCMSReader() : LineIndex(0), LineCharIndex(0) { }

EssentialsCMSReader::EssentialsCMSReader(const char* file) : LineIndex(0), LineCharIndex(0) {
	File = new TextFileClass();
	File->Set_Name(file);
}

EssentialsCMSReader::~EssentialsCMSReader() {
	File->Close();
	delete File;
}

bool EssentialsCMSReader::Exists() {
	return File->Is_Available(0);
}

bool EssentialsCMSReader::Next_Line() {
	if (!File->Is_Open()) {
		File->Open(1);
	}

	LineIndex++;
	LineCharIndex = 0;
	File->Read_Line(CurrentLine);
	return CurrentLine.Get_Length() > 0;
}
 
bool EssentialsCMSReader::Next_Token(StringClass& buf) {
	if (CurrentLine[LineCharIndex] == 0) {
		return false;
	}

	buf.Erase(0, buf.Get_Length());

	while(CurrentLine[LineCharIndex] != ' ' && CurrentLine[LineCharIndex] != 0) {
		buf += CurrentLine[LineCharIndex++];
	}

	if (CurrentLine[LineCharIndex] == ' ') {
		LineCharIndex++;
	}
	
	return true;
}

bool EssentialsCMSReader::Next_Token_As_Int(int& buf) {
	StringClass sbuf;
	if (!Next_Token(sbuf)) {
		return false;
	}
	buf = atoi(sbuf);
	return true;
}

bool EssentialsCMSReader::Next_Token_As_Float(float& buf) {
	StringClass sbuf;
	if (!Next_Token(sbuf)) {
		return false;
	}
	buf = (float)atof(sbuf);
	return true;
}

bool EssentialsCMSReader::Next_Token_As_Vector2(Vector2& buf) {
	float x, y;
	if (!Next_Token_As_Float(x) || !Next_Token_As_Float(y)) {
		return false;
	}
	buf.X = x;
	buf.Y = y;
	return true;
}

bool EssentialsCMSReader::Next_Token_As_Vector3(Vector3& buf) {
	float x, y, z;
	if (!Next_Token_As_Float(x) || !Next_Token_As_Float(y) || !Next_Token_As_Float(z)) {
		return false;
	}
	buf.X = x;
	buf.Y = y;
	buf.Z = z;
	return true;
}

bool EssentialsCMSReader::Next_Token_As_Color(Vector3& buf) {
	StringClass sbuf;
	if (!Next_Token(sbuf)) {
		return false;
	}
	if (sbuf.Get_Length() != 7) {
		return false;
	}
	if (sbuf == "DEFAULT") {
		buf = Vector3(1.f, 0.9f, 0.f);
		return true;
	} else if (sbuf[0] == '#') {
		long colorCode = strtol(&sbuf[1], NULL, 16);
		buf.X = ((colorCode & 0xFF0000) >> 16) * 0.003922f;
		buf.Y = ((colorCode & 0x00FF00) >> 8) * 0.003922f;
		buf.Z = (colorCode & 0x0000FF) * 0.003922f;
		return true;
	} else {
		return false;
	}
}

bool EssentialsCMSReader::Remaining_Text(StringClass& buf) {
	if (CurrentLine[LineCharIndex] == 0) {
		return false;
	}

	buf.Erase(0, buf.Get_Length());

	while (CurrentLine[LineCharIndex] != 0) {
		buf += CurrentLine[LineCharIndex++];
	}

	return true;
}

int EssentialsCMSReader::Line_Length() {
	return CurrentLine.Get_Length();
}

int EssentialsCMSReader::Line_Index() {
	return LineIndex;
}

bool EssentialsCMSReader::Is_Line_Empty() {
	for(int i = 0; i < Line_Length(); ++i) {
		if (CurrentLine[i] != ' ' && CurrentLine[i] != '\r' && CurrentLine[i] != '\n' && CurrentLine[i] != '\t') {
			return false;
		}
	}
	return true;
}
