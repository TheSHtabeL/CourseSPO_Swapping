#include <Windows.h>
#include <locale.h>
#include <conio.h>
#include <stdio.h>

DWORD wmain(DWORD argc, WCHAR* argv[], WCHAR* envp[]){
	setlocale(LC_ALL, "Russian");
	HANDLE hWriteFile;
	HANDLE hMapFile;
	HANDLE hFileEntry;
	DWORD CountOfOperations = 0;
	DWORD ProcNumber = 0;

	if (argc != 6){
		wprintf(L"Произошла ошибка при передаче параметров: невероятное количество параметров на входе.");
		_getch();
		return -1;
	}

	//Приём входных параметров
	hWriteFile = CreateFile((LPCSTR)argv[1], GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hWriteFile);
		return -1;
	}

	hMapFile = OpenFileMapping(NULL, FALSE, (LPCSTR)argv[3]);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		return -1;
	}

	hFileEntry = OpenEvent(NULL, FALSE, (LPCSTR)argv[3]);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		return -1;
	}
	
	CountOfOperations = _wtoi(argv[4]);
	ProcNumber = _wtoi(argv[5]);

	CloseHandle(hMapFile);
	CloseHandle(hWriteFile);
	CloseHandle(hFileEntry);
	return 0;
}