#include <Windows.h>
#include <locale.h>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]){
	setlocale(LC_ALL, "Russian");

	return 0;
}