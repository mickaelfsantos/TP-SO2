#include "./../CenTaxi/Header.h"


DWORD WINAPI threadCom(LPVOID lpParam);
DWORD WINAPI threadEncerra(LPVOID lpParam);

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCom, hThreadEnc;
	TCHAR buff[12];
	Taxi taxi;
	int i;
	Shared sh;
	sh.sair = 0;

	HINSTANCE hLib, hCom;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadEnc = CreateThread(NULL, 0, threadEncerra, &sh, 0, NULL);

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	hCom = LoadLibrary(TEXT("Dll.dll"));

	if (hLib == NULL || hCom == NULL) {
		_tprintf(TEXT("\nErro a fazer LoadLibrary (%d) - Load da dll"), GetLastError());
		return -1;
	}

	dll_log dll_logV = (dll_log) GetProcAddress(hLib, "dll_log");
	dll_register dll_registerV = (dll_register) GetProcAddress(hLib, "dll_register");
	dll2_comunica dll2_comunicaV = (dll2_comunica) GetProcAddress(hCom, "comunica");

	taxi.id = GetCurrentProcessId();
	_tprintf(TEXT("Olá. O seu ID é: %d"), taxi.id);
	_tprintf(TEXT("\nIntroduza a sua matricula: "));
	_fgetts(taxi.matricula, sizeof(taxi.matricula)/sizeof(TCHAR), stdin);
	_tprintf(TEXT("\nIntroduza a posição onde começa (x, y): "));
	_tscanf_s(TEXT("%d, %d"), &taxi.x, &taxi.y); 
	for (i = 0; taxi.matricula[i] != '\n'; i++);
	taxi.matricula[i] = '\0';
	taxi.id = GetCurrentProcessId();

	if (dll2_comunicaV != NULL) {
		(int)dll2_comunicaV(taxi);
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
