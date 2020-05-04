#include "Header.h"

DWORD WINAPI threadCom(LPVOID lpParam);
void mostraComandos();
int trataComando(TCHAR comando[]);

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread, hEventThread;
	Shared *sh;
	TCHAR comando[256];
	int op, i;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	_tprintf(TEXT("\tCentral de taxis.\n\n"));
	hThread = CreateThread(NULL, 0, threadCom, NULL, 0, NULL);

	do{
		mostraComandos();
		_tprintf(TEXT("\n\n\tComando: "));
		_fgetts(comando, 256, stdin);
		for (i = 0; i != '\n'; i++);
		comando[i+1] = '\0';
		op = trataComando(comando);
	} while (op != -1);


	WaitForSingleObject(hThread, INFINITE);
	return 0;
}

void mostraComandos() {
	_tprintf(TEXT("\n\t---Comandos disponíveis---\n"));
	_tprintf(TEXT("\n\texpulsaTaxi T: expulsa o taxi T, se estiver sem passageiros"));
	_tprintf(TEXT("\n\tencerraTudo: encerra todos os processos"));
	_tprintf(TEXT("\n\tlistaTaxis: lista taxis e estado atual assim como passageiros em transporte e em espera"));
	_tprintf(TEXT("\n\tatuaAceitacao: Pausa ou retoma a aceitacao de taxis"));
	_tprintf(TEXT("\n\tdefineDuracao: Tempo que a CenTaxi aguarda por manifestacoes de interesse em transporte de um passageiro por parte de taxis"));
	_tprintf(TEXT("\n\tsair: encerrar processo"));
}

int trataComando(TCHAR comando[]) {
	if (!_tcscmp(comando, TEXT("sair"))) {
		return -1;
	}
	else if (!_tcscmp(comando, TEXT("encerraTudo"))) {
		HANDLE hEvent; 
		hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, EVENTO_ENCERRA_TUDO);
		SetEvent(hEvent);
	}
}

DWORD WINAPI threadCom(LPVOID lpParam) {

	HANDLE hMapFile, hEvent, hMutex;
	Taxi taxi;
	taxi.nProx = 0;
	Taxi* sM = &taxi;
	TCHAR buff[256];
	Shared* sh = (Shared*)lpParam;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Taxi), MEMPAR_NOVO_TAXI);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}

	sM = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

	if (sM == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	hMutex = CreateMutex(NULL, FALSE, MUTEX_NOVO_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		
		return -1;
	}

	ZeroMemory(taxi.matricula, sizeof(taxi.matricula));
	CopyMemory(sM, &taxi, sizeof(Taxi));

	while (1) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_NOVO_TAXI);
		WaitForSingleObject(hEvent, INFINITE);

		WaitForSingleObject(hMutex, INFINITE);
		CopyMemory(&taxi, sM, sizeof(Taxi)); //mete em sdata o valor que está na memória partilhada em sd
		_tcscpy_s(buff, sizeof(buff), taxi.matricula);
		ReleaseMutex(hMutex);
		_tprintf(TEXT("\n\tNovo taxi: %s"), buff);
		_tprintf(TEXT("\n\tComando: "));
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hMutex);
}
