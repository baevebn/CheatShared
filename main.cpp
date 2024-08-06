#include "main.h"

int main()
{
	char* EXE = new char[MAX_PATH];
	char* DLL = new char[MAX_PATH];

	EXE = IniRead(szDirHack("CheatShared.ini").c_str(), "Launcher", "EXE");
	DLL = IniRead(szDirHack("CheatShared.ini").c_str(), "Launcher", "DLL");

	DWORD pID = FindProcessByName(EXE);

	if (pID != NULL)
	{
		string str(EXE);

		if (str.find(".exe") != string::npos)
		{
			SetConsoleTitle("CheatShared");

			printf("BETA: v1.5\n");
			printf("Launcher: %s - has been started!\n", EXE);
		

			HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
			uintptr_t moduleBaseAddr;

			if (strstr(EXE, DLL))
			{
				moduleBaseAddr = GetModuleBaseAddress(EXE, pID);
			}
			else
			{
				moduleBaseAddr = GetModuleBaseAddress(DLL, pID);
			}

			uintptr_t address = readAddressFromFile(szDirHack("CheatShared.ini").c_str(), "Address");
			vector<uintptr_t> offsets = readOffsetsFromFile(szDirHack("CheatShared.ini").c_str(), "Offsets");

			uintptr_t addr;

			for (uintptr_t offset : offsets)
			{
				addr = GetPointerAddress(pID, moduleBaseAddr, address, offsets);
			}

			while (true)
			{
				string number;
				int num = hasSection(szDirHack("CheatShared.ini").c_str());

				if (num == 1)
				{
					number = readValueFromFile(szDirHack("CheatShared.ini").c_str(), "int");

					int value = stoi(number);

					WriteProcessMemory(pHandle, (LPVOID)addr, &value, sizeof(value), NULL);
				}

				if (num == 2)
				{
					number = readValueFromFile(szDirHack("CheatShared.ini").c_str(), "float");

					float value = stof(number);

					WriteProcessMemory(pHandle, (LPVOID)addr, &value, sizeof(value), NULL);
				}

				if (num == 3)
				{
					number = readValueFromFile(szDirHack("CheatShared.ini").c_str(), "double");

					double value = stod(number);

					WriteProcessMemory(pHandle, (LPVOID)addr, &value, sizeof(value), NULL);
				}
			}

			CloseHandle(pHandle);
		}
	}

	return 0;
}