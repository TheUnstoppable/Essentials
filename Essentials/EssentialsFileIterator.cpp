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
// DA
#include "da.h"
#include "da_settings.h"
// Custom
#include "EssentialsFileIterator.h"

EssentialsFileIterator::EssentialsFileIterator(const wchar_t* path, const wchar_t* extension) : Path(path) {
	wchar_t searchPath[2048];
	wsprintfW(searchPath, L"%s\\%s", path, extension);

	CurrentFile = FindFirstFileW(searchPath, &CurrentFileData);
	Exists = CurrentFile != INVALID_HANDLE_VALUE;
}

EssentialsFileIterator::~EssentialsFileIterator() {
	FindClose(CurrentFile);
}

void EssentialsFileIterator::operator++() {
	Exists = !!FindNextFileW(CurrentFile, &CurrentFileData);
}

EssentialsFileIterator::operator bool() {
	return Exists;
}

const wchar_t* EssentialsFileIterator::Get_Full_Path() {
	return WideStringFormat(L"%s\\%s", Path, Get_File());
}

const wchar_t* EssentialsFileIterator::Get_File() {
	return CurrentFileData.cFileName;
}
