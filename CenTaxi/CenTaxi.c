#include "CenTaxi.h"


int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread, hEventThread;
	Shared sh;
	TCHAR comando[256];
	int op, i;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	sh.sair = 0;
	sh.nTaxis = 0;
	hThread = CreateThread(NULL, 0, threadCom, &sh, 0, NULL);

	limpaEcra();
	do{
		_tprintf(TEXT("\tCentral de taxis.\n\n"));
		mostraComandos();
		_tprintf(TEXT("\n\n\tComando: "));
		_fgetts(comando, 256, stdin);
		for (i = 0; comando[i] != '\n'; i++);
		comando[i] = '\0';
		op = trataComando(comando, &sh);
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
	_tprintf(TEXT("\n\tdefineDuracao: Tempo que a CenTaxi aguarda por um taxi manifestar interesse em transporte um passageiro"));
	_tprintf(TEXT("\n\tsair: encerrar processo"));
}

int trataComando(TCHAR comando[], Shared* sh) {
	if (!_tcscmp(comando, TEXT("sair"))) {
		return sair(sh);
	}
	else if (!_tcscmp(comando, TEXT("encerraTudo"))) {
		encerraTudo();
	}
	else if (!_tcscmp(comando, TEXT("listaTaxis"))) {
		listaTaxis(sh);
	}
	return 0;
}

int sair(Shared* sh) {
	HANDLE hEvent, hMutex;

	sh->sair = 1;
	hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_NOVO_TAXI);
	SetEvent(hEvent);

	hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_NOVO_TAXI);
	ReleaseMutex(hMutex);

	encerraTudo();
	return -1;
}

void encerraTudo() {
	HANDLE hEvent;
	hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, EVENTO_ENCERRA_TUDO);
	SetEvent(hEvent);
}

void listaTaxis(Shared * sh) {

	HANDLE hMutex;

	limpaEcra();
	hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_NOVO_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);
	_getch();
	if (sh->nTaxis == 0) {
		_tprintf(TEXT("\n\tSem taxis registados\n"));
		ReleaseMutex(hMutex);
		return;
	}
	for (int i = 0; i < sh->nTaxis; i++) {
		_tprintf(TEXT("\n\tTaxi %d: %s (matricula) %d (id)"), i+1, sh->taxis[i].matricula, sh->taxis[i].id);
	}
	ReleaseMutex(hMutex);
	_tprintf(TEXT("\n"));
}

void limpaEcra() {
	for(int i=0; i<30; i++)
		_tprintf(TEXT("\n"));
}

DWORD WINAPI threadCom(LPVOID lpParam) {

	HANDLE hMapFile, hEvent=NULL, hMutex;
	Taxi taxi;
	Taxi* sM = &taxi;
	TCHAR buff[256];
	Shared* sh = (Shared*)lpParam;
	int i;

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

	while (!sh->sair) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_NOVO_TAXI);
		WaitForSingleObject(hEvent, INFINITE);

		WaitForSingleObject(hMutex, INFINITE);
		if (sh->sair)
			break;
		sh->taxis[sh->nTaxis].id = sM->id;
		_tcscpy_s(sh->taxis[sh->nTaxis].matricula, sizeof(sh->taxis[sh->nTaxis].matricula) / sizeof(TCHAR), sM->matricula);
		for (i = 0; sh->taxis[sh->nTaxis].matricula[i] != '\n'; i++);
		sh->taxis[sh->nTaxis].matricula[i] = '\0';
		sh->nTaxis++;
		ReleaseMutex(hMutex);
		if (sh->nTaxis == LIMITE_TAXIS) {
			_tprintf(TEXT("\n\tA central ficou cheia!"));
		}
		_tprintf(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), sh->taxis[sh->nTaxis-1].matricula, sh->taxis[sh->nTaxis-1].id);
		_tprintf(TEXT("\n\tComando: "));
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hMutex);
}
