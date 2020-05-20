#include "Contaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCom, hThreadEnc, hThreadInf;
	Taxi taxi;
	Contaxi c;
	int i;

	HINSTANCE hLib, hCom;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadEnc = CreateThread(NULL, 0, threadEncerra, NULL, 0, NULL);
	hThreadInf = CreateThread(NULL, 0, threadInformacao, &taxi, 0, NULL);

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	hCom = LoadLibrary(TEXT("Dll.dll"));

	if (hLib == NULL || hCom == NULL) {
		_tprintf(TEXT("\nErro a fazer LoadLibrary (%d) - Load da dll"), GetLastError());
		return -1;
	}

	dll_log dll_logV = (dll_log)GetProcAddress(hLib, "dll_log");
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");
	dll2_comunica dll2_comunicaV = (dll2_comunica)GetProcAddress(hCom, "comunica");
	dll2_comunicaSaida dll2_comunicaSV = (dll2_comunicaSaida)GetProcAddress(hCom, "comunicaSaida");
	dll2_carregaMapa dll2_carregaM = (dll2_carregaMapa)GetProcAddress(hCom, "carregaMapa");


	if (dll2_comunicaV == NULL) {
		_tprintf(TEXT("\nErro no ponteiro para a DLL."));
		return 0;
	}
	
	WaitForSingleObject(hThreadInf, INFINITE);
	taxi = dll2_comunicaV(taxi);
	if (taxi.aceite != 1) {
		_tprintf(TEXT("\nN�o foi aceite por parte da central"));
		FreeLibrary(hLib);
		FreeLibrary(hCom);
		return 0;
	}

	c.alturaMapa = taxi.alturaMapa;
	c.larguraMapa = taxi.larguraMapa;
	c.mapa = NULL;
	c = dll2_carregaM(c);

	for (int i = 0, k = 0; i < c.alturaMapa; i++) {
		for (int j = 0; j < c.larguraMapa; j++, k++) {
			if (c.mapa[k] == 1) {
				_tprintf(TEXT("_"));
			}
			else if (c.mapa[k] == 2) {
				_tprintf(TEXT("C"));
			}
			else {
				_tprintf(TEXT("X"));
			}
		}
		_tprintf(TEXT("\n"));
	}
	_getch();
	return 0;
}

DWORD WINAPI threadInformacao(LPVOID lpParam) {
	Taxi* taxi = (Taxi*)lpParam;
	int i;

	taxi->id = GetCurrentProcessId();
	_tprintf(TEXT("Ol�. O seu ID �: %d"), taxi->id);
	_tprintf(TEXT("\nIntroduza a sua matricula: "));
	_fgetts(taxi->matricula, sizeof(taxi->matricula) / sizeof(TCHAR), stdin);
	_tprintf(TEXT("\nIntroduza a posi��o onde come�a (x, y): "));
	_tscanf_s(TEXT("%d, %d"), &taxi->x, &taxi->y);
	for (i = 0; taxi->matricula[i] != '\n'; i++);
	taxi->matricula[i] = '\0';
	taxi->id = GetCurrentProcessId();
	taxi->aceite = -1;
}

DWORD WINAPI threadEncerra(LPVOID lpParam) {
	HANDLE hEvent;

	hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_ENCERRA_TUDO);
	WaitForSingleObject(hEvent, INFINITE);
	ResetEvent(hEvent);
	exit(-1);
}