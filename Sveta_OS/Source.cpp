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
	TCHAR ReadFileName[50];
	TCHAR WriteFileName[50];
	TCHAR MapFileName[50] = "MapFile";
	TCHAR FileEntryName[50] = "FileEntry";
	TCHAR CommandConsole[128] = "Slave.exe";
	TCHAR TempForInt[12];
	HANDLE hReadFile;
	HANDLE hMapFile;
	HANDLE hWriteFile;
	HANDLE hFileEntry;
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcInfo;
	struct stat FileInfo;

	//������������� ����������
	StartupInfo = { sizeof(StartupInfo) };
	StartupInfo.cb = sizeof(StartupInfo);
	wprintf(L"������� ���������� ���������: ");
	if (!scanf("%d", &ProcCount)){
		wprintf(L"��������� ������. ������� ����� ������� ��� ������ �� ���������...");
		_getch();
		return -1;
	}
	wprintf(L"������� ��� ����� ��� ������: ");
	if (!scanf("%s", ReadFileName)){
		wprintf(L"��������� ������. ������� ����� ������� ��� ������ �� ���������...");
		_getch();
		return -1;
	}
	wprintf(L"������� ��� ����� ��� ������: ");
	if (!scanf("%s", WriteFileName)){
		wprintf(L"��������� ������. ������� ����� ������� ��� ������ �� ���������...");
		_getch();
		return -1;
	}

	//�������� ����� �� ������
	hReadFile = CreateFile(ReadFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (hReadFile == INVALID_HANDLE_VALUE) {
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hReadFile);
		return -1;
	}
	//�������� ����� �� ������
	hWriteFile = CreateFile(WriteFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
	if (hWriteFile == INVALID_HANDLE_VALUE) {
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		return -1;
	}
	CloseHandle(hWriteFile); //���������� �����������, ��� ��� �� ������������ � �������� ��������

	//�������� ������� �� ������ � ����������� ������
	hFileEntry = CreateEvent(NULL, TRUE, TRUE, FileEntryName);
	if (hFileEntry == INVALID_HANDLE_VALUE){
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		CloseHandle(hFileEntry);
		return -1;
	}

	//�������� ����� �� ��������� �������
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, ProcCount*BufferSize, MapFileName);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		CloseHandle(hFileEntry);
		return -1;
	}
	MapViewData = (PINT)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	CloseHandle(hMapFile);

	//������� ���������� ��������� �� ������
	stat(ReadFileName, &FileInfo);
	CountOfOperations = (FileInfo.st_size / DataSize);
	if (FileInfo.st_size % DataSize){
		CountOfOperations++;
	}
	_itoa_s(CountOfOperations, TempForInt, 10);

	//���������� ������������ ������� � ����������� �������������
	PrepareFileInMap(MapViewData, ProcCount);

	//�������� �������� ���������
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, WriteFileName);
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, MapFileName);
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, FileEntryName);
	strcat_s(CommandConsole, " ");
	_itoa_s(CountOfOperations, TempForInt, 10);
	strcat_s(CommandConsole, TempForInt);
	strcat_s(CommandConsole, " ");
	_itoa_s(DataSize, TempForInt, 10);
	strcat_s(CommandConsole, TempForInt);
	strcat_s(CommandConsole, " ");
	_itoa_s(BufferSize, TempForInt, 10);
	strcat_s(CommandConsole, TempForInt);
	strcat_s(CommandConsole, " ");

	for (DWORD i = 0; i < ProcCount; i++){
		TCHAR uniqueCommandConsole[100];
		TCHAR Number[12];

		strcpy_s(uniqueCommandConsole, CommandConsole);
		_itoa_s(i, Number, 10);
		strcat_s(uniqueCommandConsole, Number);
		CreateProcess(NULL, (LPSTR)uniqueCommandConsole, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcInfo);
	}

	ReadFileInMap(hReadFile, MapViewData, hFileEntry, CountOfOperations, ProcCount);

	UnmapViewOfFile(MapViewData);
	CloseHandle(hReadFile);
	CloseHandle(hFileEntry);
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
	
	hEvent = CreateEvent(NULL, FALSE, TRUE, NULL); //�������� ������� � �������������� �������
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
		OperationNumber++;
		while (OperationNumber <= CountOfOperations){
			if (OperationNumber < CountOfOperations){
				ReadFile(hReadFile, BufferRead, DataSize, NULL, &overlapped);
			}
			//�����
			while (true){
				BOOL Flag = false;
				WaitForSingleObject(hFileEntry, INFINITE);
				for (DWORD i = 0; i < ProcCount; i++){
					if (MapViewData[i * BufferSize] == (-1)){
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
				memcpy(BufferWrite, BufferRead, DataSize);
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