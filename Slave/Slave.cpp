#include <Windows.h>
#include <locale.h>
#include <conio.h>
#include <stdio.h>
#include <sys/stat.h>

DWORD wmain(DWORD argc, WCHAR* argv[], WCHAR* envp[]){
	setlocale(LC_ALL, "Russian");
	HANDLE hWriteFile;
	HANDLE hMapFile;
	HANDLE hFileEntry;
	HANDLE hEvent;
	DWORD CountOfOperations = 0;
	DWORD ProcNumber = 0;
	DWORD DataSize;
	DWORD BufferSize;
	DWORD ProcCount;
	DWORD Offset;
	DWORD SizeToWrite;
	PINT MapViewData;
	BYTE* BufferRead;
	BYTE* BufferWrite;
	struct _stat64i32 FileInfo;
	WCHAR ReadFileName[50];
	WCHAR WriteFileName[50];
	OVERLAPPED overlapped;

	if (argc != 9){
		wprintf(L"Произошла ошибка при передаче параметров: невероятное количество параметров на входе.");
		_getch();
		return -1;
	}

	//Приём входных параметров
	wcscpy(WriteFileName, argv[1]);
	wcscpy(ReadFileName, argv[4]);
	DataSize = _wtoi(argv[6]);
	BufferSize = _wtoi(argv[7]);
	CountOfOperations = _wtoi(argv[5]);
	ProcCount = _wtoi(argv[8]);
	BufferRead = (BYTE*)malloc(DataSize * sizeof(BYTE));
	BufferWrite = (BYTE*)malloc(DataSize * sizeof(BYTE));

	hWriteFile = CreateFileW(argv[1], GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла на запись. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hWriteFile);
		return -1;
	}
	
	hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, argv[2]);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"Ошибка при открытии файла на своппинге. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		return -1;
	}
	MapViewData = (PINT)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, ProcCount*BufferSize);
	CloseHandle(hMapFile);

	hFileEntry = OpenEventW(NULL, FALSE, argv[3]);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"Ошибка при открытии события. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		CloseHandle(hFileEntry);
		return -1;
	}

	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;

	wprintf(L"Koko");
	while (true){
		DWORD SizeTest = 0;
		BOOL Flag = false;

		WaitForSingleObject(hFileEntry, INFINITE);

		for (DWORD i = 0; i < ProcCount; i++){	
			if (MapViewData[i * BufferSize] != (-1)){
				wprintf(L"%Koko", ProcCount);
				SizeToWrite = MapViewData[i*BufferSize];
				Offset = MapViewData[i*BufferSize + sizeof(DWORD)];
				memcpy(BufferRead, &MapViewData[i*BufferSize], BufferSize);
				MapViewData[i*BufferSize] = (-1);
				WriteFile(hWriteFile, &BufferRead[BufferSize - DataSize], DataSize, NULL, &overlapped);
				WaitForSingleObject(hEvent, INFINITE);
				overlapped.Offset += DataSize;
			}
		}
		SetEvent(hFileEntry);

		_wstat(argv[1], &FileInfo);
		SizeTest = FileInfo.st_size;
		_wstat(argv[4], &FileInfo);
		if (SizeTest == FileInfo.st_size){
			wprintf(L"%d %d", FileInfo.st_size, SizeTest);
			break;
		}
	}

	wprintf(L"\n\n\nКопирование завершено");
	_getch();
	CloseHandle(hWriteFile);
	CloseHandle(hFileEntry);
	return 0;
}