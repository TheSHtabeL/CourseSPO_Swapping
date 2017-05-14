#include <Windows.h>
#include <locale.h>
#include <stdio.h>
#include <sys/stat.h>
#include <conio.h>

VOID ReadFile(HANDLE, HANDLE);

DWORD wmain(DWORD argc, WCHAR* argv[], WCHAR* envp[]){
	setlocale(LC_ALL, "Russian");
	DWORD ProcCount = 0;
	DWORD BufferSize = 4;
	DWORD CountOfOperations = 0;
	TCHAR ReadFileName[50];
	TCHAR WriteFileName[50];
	TCHAR MapFileName[50] = "MapFile";
	TCHAR FileEntryName[50] = "FileEntry";
	TCHAR CommandConsole[128] = "Slave.exe";
	TCHAR charCountOfOperations[12];
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
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, BufferSize*ProcCount, MapFileName);
	if (hMapFile == INVALID_HANDLE_VALUE){
		wprintf(L"������ ��� �������� �����. ������� ����� ������� ��� �����������...");
		_getch();
		CloseHandle(hMapFile);
		CloseHandle(hWriteFile);
		CloseHandle(hReadFile);
		CloseHandle(hFileEntry);
		return -1;
	}

	//������� ���������� ��������� �� ������
	stat(ReadFileName, &FileInfo);
	CountOfOperations = (FileInfo.st_size / BufferSize);
	if (FileInfo.st_size % BufferSize){
		CountOfOperations++;
	}
	_itoa_s(CountOfOperations, charCountOfOperations, 10);

	//�������� �������� ���������
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, WriteFileName);
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, MapFileName);
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, FileEntryName);
	strcat_s(CommandConsole, " ");
	strcat_s(CommandConsole, charCountOfOperations);
	strcat_s(CommandConsole, " ");

	for (DWORD i = 0; i < ProcCount; i++){
		TCHAR uniqueCommandConsole[100];
		TCHAR Number[12];

		strcpy_s(uniqueCommandConsole, CommandConsole);
		_itoa_s(i, Number, 10);
		strcat_s(uniqueCommandConsole, Number);
		CreateProcess(NULL, (LPSTR)uniqueCommandConsole, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcInfo);
	}

	CloseHandle(hReadFile);
	CloseHandle(hMapFile);
	CloseHandle(hFileEntry);
	return 0;
}