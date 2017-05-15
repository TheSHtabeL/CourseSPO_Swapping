#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <conio.h>

DWORD DataSize = 4;
DWORD BufferSize = DataSize + 2 * sizeof(INT);

VOID ReadFileInMap(HANDLE, PINT, HANDLE, DWORD, DWORD);
VOID PrepareFileInMap(PINT, DWORD);

DWORD wmain(DWORD argc, WCHAR* argv[], WCHAR* envp[]){
	setlocale(LC_ALL, "Russian");
	DWORD ProcCount = 0;
	
	DWORD CountOfOperations = 0;
	PINT MapViewData;
	WCHAR ReadFileName[50];
	WCHAR WriteFileName[50];
	WCHAR MapFileName[50] = L"MapFile";
	WCHAR FileEntryName[50] = L"FileEntry";
	WCHAR CommandConsole[200] = L"Slave.exe";
	WCHAR TempForInt[12];
	HANDLE hReadFile;
	HANDLE hMapFile;
	HANDLE hWriteFile;
	HANDLE hFileEntry;
	STARTUPINFOW StartupInfo;
	PROCESS_INFORMATION ProcInfo;
	struct _stat64i32 FileInfo;

	//Инициализация переменных
	StartupInfo = { sizeof(StartupInfo) };
	StartupInfo.cb = sizeof(StartupInfo);
	wprintf(L"Введите количество процессов: ");
	if (!scanf("%d", &ProcCount)){
		wprintf(L"Произошла ошибка. Нажмите любую клавишу для выхода из программы...");
		_getch();
		return -1;
	}
	wprintf(L"Введите имя файла для чтения: ");
	if (!wscanf(L"%s", ReadFileName)){
		wprintf(L"Произошла ошибка. Нажмите любую клавишу для выхода из программы...");
		_getch();
		return -1;
	}
	wprintf(L"Введите имя файла для записи: ");
	if (!wscanf(L"%s", WriteFileName)){
		wprintf(L"Произошла ошибка. Нажмите любую клавишу для выхода из программы...");
		_getch();
		return -1;
	}

	//Открытие файла на чтение
	hReadFile = CreateFileW(ReadFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hReadFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hReadFile);
		return -1;
	}
	//Открытие файла на запись
	hWriteFile = CreateFileW(WriteFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		return -1;
	}
	CloseHandle(hWriteFile); //Дескриптор закрывается, так как не используется в пределах процесса

	//Создание события на доступ к критической секции
	hFileEntry = CreateEventW(NULL, TRUE, TRUE, FileEntryName);
	if (hFileEntry == INVALID_HANDLE_VALUE){
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		CloseHandle(hFileEntry);
		return -1;
	}

	//Открытие файла на своппинге системы
	hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, ProcCount*BufferSize, MapFileName);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"Ошибка при открытии файла. Нажмите любую клавишу для продолжения...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		CloseHandle(hFileEntry);
		return -1;
	}
	MapViewData = (PINT)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	//CloseHandle(hMapFile);

	//Подсчёт количества обращений на чтение
	_wstat(ReadFileName, &FileInfo);
	CountOfOperations = (FileInfo.st_size / DataSize);
	if (FileInfo.st_size % DataSize){
		CountOfOperations++;
	}
	_itow_s(CountOfOperations, TempForInt, 10);

	//Подготовка разделяемого ресурса к совместному использованию
	PrepareFileInMap(MapViewData, ProcCount);

	//Создание дочерних процессов
	wcscat_s(CommandConsole, L" ");
	wcscat_s(CommandConsole, WriteFileName);
	wcscat_s(CommandConsole, L" ");
	wcscat_s(CommandConsole, MapFileName);
	wcscat_s(CommandConsole, L" ");
	wcscat_s(CommandConsole, FileEntryName);
	wcscat_s(CommandConsole, L" ");
	wcscat_s(CommandConsole, ReadFileName);
	wcscat_s(CommandConsole, L" ");
	_itow_s(CountOfOperations, TempForInt, 10);
	wcscat_s(CommandConsole, TempForInt);
	wcscat_s(CommandConsole, L" ");
	_itow_s(DataSize, TempForInt, 10);
	wcscat_s(CommandConsole, TempForInt);
	wcscat_s(CommandConsole, L" ");
	_itow_s(BufferSize, TempForInt, 10);
	wcscat_s(CommandConsole, TempForInt);
	wcscat_s(CommandConsole, L" ");
	_itow_s(ProcCount, TempForInt, 10);
	wcscat_s(CommandConsole, TempForInt);
	wcscat_s(CommandConsole, L" ");

	for (DWORD i = 0; i < ProcCount; i++){
		CreateProcessW(NULL, CommandConsole, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcInfo);
	}

	ReadFileInMap(hReadFile, MapViewData, hFileEntry, CountOfOperations, ProcCount);

	wprintf(L"\n\n\nЗАВЕРШЕНО");
	_getch();
	UnmapViewOfFile(MapViewData);
	CloseHandle(hReadFile);
	CloseHandle(hFileEntry);
	CloseHandle(hMapFile);
	return 0;
}

VOID PrepareFileInMap(PINT MapViewData, DWORD ProcCount){
	for (DWORD i = 0; i < ProcCount; i++){
		MapViewData[i * BufferSize] = (-1);
	}

	return;
}

VOID ReadFileInMap(HANDLE hReadFile, PINT MapViewData, HANDLE hFileEntry, DWORD CountOfOperations, DWORD ProcCount){
	BYTE* BufferRead = (BYTE*)malloc(DataSize);
	BYTE* BufferWrite = (BYTE*)malloc(BufferSize);
	HANDLE hEvent;
	OVERLAPPED overlapped;
	DWORD PortionSize = 0;
	DWORD OperationNumber = 0;
	DWORD BeginOffset = BufferSize - DataSize;
	DWORD BlockOffset = 0;
	
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //Создание события с автоматическим сбросом
	overlapped.hEvent = hEvent;
	overlapped.Offset = 0;
	overlapped.OffsetHigh = 0;

	if (OperationNumber < CountOfOperations){
		ReadFile(hReadFile, BufferRead, DataSize, NULL, &overlapped);
		GetOverlappedResult(hReadFile, &overlapped, &PortionSize, TRUE);
		memcpy(BufferWrite, &BlockOffset, sizeof(DWORD));
		memcpy(BufferWrite + sizeof(DWORD), &PortionSize, sizeof(DWORD));
		memcpy(BufferWrite + BeginOffset, BufferRead, DataSize);
		BlockOffset += DataSize;
		overlapped.Offset += DataSize;
		OperationNumber++;
		while (OperationNumber <= CountOfOperations){
			if (OperationNumber < CountOfOperations){
				ReadFile(hReadFile, BufferRead, DataSize, NULL, &overlapped);
			}
			//Ревёрс
			while (true){
				BOOL Flag = false;
				WaitForSingleObject(hFileEntry, INFINITE);
				for (DWORD i = 0; i < ProcCount; i++){
					if (MapViewData[i * BufferSize] == (-1)){
						wprintf(L"Koko");
						memcpy(&MapViewData[i * BufferSize], BufferWrite, BufferSize);
						Flag = true;
					}
				}
				SetEvent(hFileEntry);
				if (Flag){
					break;
				}
				Sleep(0);
			}
			if (OperationNumber < CountOfOperations){
				GetOverlappedResult(hReadFile, &overlapped, &PortionSize, TRUE);
				memcpy(BufferWrite, &BlockOffset, sizeof(DWORD));
				memcpy(BufferWrite + sizeof(DWORD), &PortionSize, sizeof(DWORD));
				memcpy(BufferWrite + BeginOffset, BufferRead, DataSize);
				overlapped.Offset += DataSize;
				BlockOffset += DataSize;
			}
			OperationNumber++;
		}
	}

	CloseHandle(hEvent);
	free(BufferRead);
	free(BufferWrite);
	return;
}