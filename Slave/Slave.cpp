#include <Windows.h>
#include <locale.h>
#include <conio.h>
#include <stdio.h>

DWORD wmain(DWORD argc, WCHAR* argv[], WCHAR* envp[]){
	setlocale(LC_ALL, "Russian");

	wprintf(L"Kokoko");
	_getch();
	
	return 0;
}