#include "./../CenTaxi/Header.h"

typedef void(_cdecl* dll_log)(TCHAR* text);
typedef void(_cdecl* dll_register)(TCHAR* text);
typedef int(_cdecl* dll2_comunica)(TCHAR* buff);

DWORD WINAPI threadCom(LPVOID lpParam);
DWORD WINAPI threadEncerra(LPVOID lpParam);

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCom, hThreadEnc;
	TCHAR buff[12];
	Shared sh;
	sh.sair = 0;

	HINSTANCE hLib, hCom;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadEnc = CreateThread(NULL, 0, threadEncerra, &sh, 0, NULL);

	hLib = LoadLibrary(TEXT("E:\\Universidade\\2019-2020\\2 semestre\\SO2\\TP-SO2\\Debug\\SO2_TP_DLL_64.dll"));
	hCom = LoadLibrary(TEXT("Dll.dll"));

	if (hLib == NULL || hCom == NULL) {
		_tprintf(TEXT("\nErro a fazer LoadLibrary (%d) - Load da dll"), GetLastError());
		return -1;
	}

	dll_log dll_log= GetProcAddress(hLib, "dll_log");
	dll_register dll_register = GetProcAddress(hLib, "dll_register");
	dll2_comunica dll2_comunica = GetProcAddress(hCom, "comunica");

	_tprintf(TEXT("Olá. Introduza a sua matricula: "));
	_fgetts(buff, 12, stdin);


	if (dll_log != NULL && dll_register != NULL && dll2_comunica != NULL) {
		(void)dll_log(TEXT("OLA"));
		(void)dll_register(TEXT("OLA"), 1);
		(int)dll2_comunica(buff);
	}
	FreeLibrary(hLib);
	FreeLibrary(hCom);

	WaitForSingleObject(hThreadEnc, INFINITE);
	return 0;
}


DWORD WINAPI threadEncerra(LPVOID lpParam) {
	HANDLE hEvent;
	Shared* sh = (Shared*)lpParam;

	hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_ENCERRA_TUDO);
	WaitForSingleObject(hEvent, INFINITE);
	sh->sair = 1;
	ResetEvent(hEvent);
}
