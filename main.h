#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <WtsApi32.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <codecvt>
#include <locale>
#include <iostream>
#include <vector>
#include <tchar.h>
#include <vector>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <Psapi.h>
#include <locale>
#include <filesystem>
#include <ctime>
#include <chrono>
#include <stdexcept>
#include <codecvt>
#include <sstream>
#include <stdarg.h>
#include <cstdarg>
#include <thread>
#include <ctype.h>
#include <cmath>
#include <type_traits>

#pragma warning(disable:4996)

using namespace std;
using namespace chrono;

namespace fs = filesystem;
using namespace filesystem;

inline char* IniRead(const char* filename, const char* section, const char* key)
{
	char* out = new char[MAX_PATH];
	GetPrivateProfileString((LPCSTR)section, (LPCSTR)key, NULL, out, MAX_PATH, (LPCSTR)filename);
	return out;
}

inline vector<uintptr_t> readOffsetsFromFile(const char* filename, const string& section) 
{
	vector<uintptr_t> offsets;
	ifstream file(filename);

	file.imbue(std::locale(file.getloc(), new std::codecvt_utf8_utf16<wchar_t>));

	if (file.is_open())
	{
		string line;
		bool inNameSection = false;

		while (getline(file, line)) {
			if (line.find("[" + section + "]") != std::string::npos)
			{
				inNameSection = true;
			}
			else if (inNameSection && line != "") 
			{
				line.erase(0, line.find_first_not_of(" "));
				line.erase(line.find_last_not_of(" ") + 1);

				uintptr_t offset = std::stoul(line, nullptr, 16);
				offsets.push_back(offset);
			}
		}

		file.close();
	}

	return offsets;
}

inline uintptr_t readAddressFromFile(const char* filename, const string& section)
{
	uintptr_t address = 0;
	ifstream file(filename);

	file.imbue(locale(file.getloc(), new codecvt_utf8_utf16<wchar_t>));

	if (file.is_open())
	{
		string line;
		bool inNameSection = false;

		while (getline(file, line)) 
		{
			if (line.find("[" + section + "]") != string::npos)
			{
				inNameSection = true;
			}
			else if (inNameSection && line != "") 
			{
			
				line.erase(0, line.find_first_not_of(" "));
				line.erase(line.find_last_not_of(" ") + 1);

				address = stoul(line, nullptr, 16); 
				break; 
			}
		}

		file.close();
	}

	return address;
} 

inline string readValueFromFile(const char* filename, const string& section)
{
	string value;
	ifstream file(filename);

	file.imbue(locale(file.getloc(), new codecvt_utf8_utf16<wchar_t>));

	if (file.is_open()) 
	{
		string line;
		bool inNameSection = false;

		while (getline(file, line)) 
		{
			if (line.find('[' + section + ']') != string::npos)
			{
				inNameSection = true;
			}
			else if (inNameSection && line != "") 
			{
			
				line.erase(0, line.find_first_not_of(" "));
				line.erase(line.find_last_not_of(" ") + 1);

				value = line;
				break; 
			}
		}

		file.close();
	}

	return value;
}

inline int hasSection(const string& filename)
{
	ifstream file(filename);

	if (!file.is_open())
	{
		return 0;
	}

	string line;

	while (getline(file, line))
	{
		if (line.empty() || line[0] == ';') 
		{
			continue;
		}

		if (line.find("[int]") != string::npos)
		{
			return 1;
		}

		if (line.find("[float]") != string::npos)
		{
			return 2;
		}

		if (line.find("[double]") != string::npos)
		{
			return 3;
		}

		/*if (line.find("[string]") != string::npos)
		{
			return 4;
		}*/
	}

	file.close();

	return 0;
}

inline string GetPathToHard()
{
	char buffer[MAX_PATH];
	GetSystemDirectory(buffer, sizeof(buffer));

	string windowsDir(buffer);
	size_t pos = windowsDir.find('\\', 3);

	string diskPath = windowsDir.substr(0, pos - 7);

	char cmd[256];
	sprintf(cmd, "%s", diskPath.c_str());

	return cmd;
}

inline string GetPathToUSB(const char* str)
{
	char szLogicalDrivers[64];
	char USB[MAX_PATH];

	DWORD dwResult = GetLogicalDriveStringsA(sizeof(szLogicalDrivers), szLogicalDrivers);
	string szPath = str;

	if (dwResult > 0 && dwResult <= sizeof(szLogicalDrivers))
	{
		char* szSingleDriver = szLogicalDrivers;

		while (*szSingleDriver)
		{
			UINT driveType = GetDriveTypeA(szSingleDriver);

			if (driveType == DRIVE_REMOVABLE || 
				driveType == DRIVE_FIXED)
			{
				sprintf(USB, "%s", szSingleDriver);
				
			}

			szSingleDriver += strlen(szSingleDriver) + 1;
		}
	}

	return (USB + szPath);
}

inline string szDirHack(const char* szName)
{
	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(sizeof(buffer), buffer);

	char str[MAX_PATH];
	sprintf(str, "%s\\", buffer);

	string pDir = str;
	return (pDir + szName);
}

inline string pszExtHack(const char* Path, const char* ExtensionName)
{
	string pszStr;

	for (const auto& entry : directory_iterator(Path))
	{
		if (is_regular_file(entry))
		{
			string fileName = entry.path().filename().string();

			if (fileName.find(ExtensionName) != string::npos)
			{
				pszStr = fileName.substr(0, fileName.find_last_of(".")).c_str();
			}
		}
	}

	return pszStr;
}


inline DWORD FindProcessByName(const char* procname)
{
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;

	int pid = 0;
	BOOL hResult;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		return 0;
	}

	pe.dwSize = sizeof(PROCESSENTRY32);
	hResult = Process32First(hSnapshot, &pe);

	while (hResult)
	{
		if (strcmp(procname, pe.szExeFile) == 0)
		{
			pid = pe.th32ProcessID;
			break;
		}

		hResult = Process32Next(hSnapshot, &pe);
	}

	CloseHandle(hSnapshot);

	return pid;
}

inline DWORD GetModuleBaseAddress(const char* lpszModuleName, DWORD pID)
{
	DWORD dwModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);

	MODULEENTRY32 ModuleEntry32 = { 0 };
	ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnapshot, &ModuleEntry32))
	{
		do {
			if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0)
			{
				dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnapshot, &ModuleEntry32));
	}

	CloseHandle(hSnapshot);
	return dwModuleBaseAddress;
}

uintptr_t GetPointerAddress(DWORD pID, uintptr_t gameBaseAddr, uintptr_t address, vector<unsigned int> offsets)
{
	HANDLE phandle = NULL;
	phandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

	uintptr_t offset_null = NULL;

	ReadProcessMemory(phandle, (LPVOID*)(gameBaseAddr + address), &offset_null, sizeof(offset_null), 0);
	uintptr_t pointeraddress = offset_null;

	for (unsigned int i = 0; i < offsets.size() - 1; i++)
	{
		ReadProcessMemory(phandle, (LPVOID*)(pointeraddress + offsets.at(i)), &pointeraddress, sizeof(pointeraddress), 0);
	}

	return pointeraddress += offsets.at(offsets.size() - 1);
}
