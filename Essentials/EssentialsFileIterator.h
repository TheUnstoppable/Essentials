#pragma once

class EssentialsFileIterator {
public:
	EssentialsFileIterator(const wchar_t* path, const wchar_t* extension = L"*.*");
	~EssentialsFileIterator();
	void operator++();
	operator bool();

	const wchar_t* Get_Full_Path();
	const wchar_t* Get_File();

private:
	HANDLE CurrentFile;
	WIN32_FIND_DATAW CurrentFileData;
	WideStringClass Path;
	bool Exists;
};
