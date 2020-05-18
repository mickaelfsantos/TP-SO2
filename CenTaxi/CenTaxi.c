#include "CenTaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread, hEventThread, hThreadComandos;
	Shared sh;
	Mapa m;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	carregaMapa(&m);
	mostraMapa(&m);

	sh.sair = 0;
	sh.nTaxis = 0;
	hThreadComandos = CreateThread(NULL, 0, threadComandos, &sh, 0, NULL);
	hThread = CreateThread(NULL, 0, threadCom, &sh, 0, NULL);


	WaitForSingleObject(hThreadComandos, INFINITE);
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

void carregaMapa(Mapa* m) {

	FILE* file=NULL;
	int c, altura, i, largura, j;
	
	errno_t err = fopen_s(&file, ".\\..\\f.txt", "r");

	if (file == NULL) {
		_tprintf(TEXT("%d"), err);
		return NULL;
	}

	c = fgetc(file);
	for (i = 0; feof(file) == 0 && (char)c != TEXT('\n'); i++)
	{
		c = fgetc(file);
	}
	m->largura = i;

	fclose(file);

	err = fopen_s(&file, ".\\..\\f.txt", "r");

	if (file == NULL) {
		_tprintf(TEXT("%d"), err);
		return NULL;
	}

	c = fgetc(file);
	for (i = 0, j=0; feof(file) == 0; i++)
	{
		if ((char)c == TEXT('\n')) {
			j++;
		}
		c = fgetc(file);
	}
	fclose(file);

	m->altura = j+1;


	m->estrada = (int**)malloc(m->largura* sizeof(int*));
	for (i = 0; i < m->largura; i++) {
		m->estrada[i] = (int *)malloc(sizeof(int) * m->altura);
	}
	

	err = fopen_s(&file, ".\\..\\f.txt", "r");

	if (file == NULL) {
		_tprintf(TEXT("%d"), err);
		return NULL;
	}

	c = fgetc(file);
	for (int i = 0, j = 0; feof(file) == 0; j++)
	{
		if ((char)c == TEXT('_')) {
			m->estrada[i][j] = 1;
		}
		else if ((char)c == TEXT('X')) {
			m->estrada[i][j] = 0;
		}
		else {
			j = -1;
			i++;
		}
		c = fgetc(file);
	}

	fclose(file);

}


void mostraMapa(Mapa* m) {
	_tprintf(TEXT("\n"));
	for (int i = 0; i< m->altura; i++) {
		for (int j = 0; j< m->largura; j++) {
			if (m->estrada[i][j]) {
				_tprintf(TEXT("_"));
			}
			else {
				_tprintf(TEXT("X"));
			}
		}
		_tprintf(TEXT("\n"));
	}
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

	hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_TAXI);
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
	hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);
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

DWORD WINAPI threadComandos(LPVOID lpParam) {
	TCHAR comando[256];
	int i, op;
	Shared* sh = (Shared*)lpParam;

	limpaEcra();
	do {
		_tprintf(TEXT("\tCentral de taxis.\n\n"));
		mostraComandos();
		_tprintf(TEXT("\n\n\tComando: "));
		_fgetts(comando, 256, stdin);
		for (i = 0; comando[i] != '\n'; i++);
		comando[i] = '\0';
		op = trataComando(comando, sh);
	} while (op != -1);
	return 0;
}

DWORD WINAPI threadCom(LPVOID lpParam) {

	HANDLE hMapFile, hEvent = NULL, hSemLei, hSemEsc, hSemRes, hMutex;
	Taxi taxi;
	Taxi aux;
	Taxi* sM = &taxi;
	TCHAR buff[256];
	Shared* sh = (Shared*)lpParam;
	int i, pos;

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

	hSemLei = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_LEI);
	if (hSemLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		
		return -1;
	}

	hSemEsc = CreateSemaphore(NULL, 1, 1, MUTEX_NOVO_TAXI_ESC);
	if (hSemEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);

		return -1;
	}

	hSemRes = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_RES);
	if (hSemRes == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de resposta (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);

		return -1;
	}
	hMutex = CreateMutex(NULL, TRUE, MUTEX_TAXI);
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

		WaitForSingleObject(hSemLei, INFINITE);
		if (sh->sair)
			break;
		aux.id = sM->id;
		_tcscpy_s(aux.matricula, sizeof(aux.matricula) / sizeof(TCHAR), sM->matricula);
		pos = sh->nTaxis;

		if (sh->nTaxis != LIMITE_TAXIS) {
			for (int j = 0; j < pos; j++) {
				if (!_tcscmp(aux.matricula, sh->taxis[j].matricula)) {
					aux.aceite = 0;
					break;
				}
			}
		}
		else {
			aux.aceite = 0;
		}
		if (aux.aceite != 0) {
			aux.aceite = 1;
		}
		CopyMemory(sM, &aux, sizeof(Taxi));
		ReleaseSemaphore(hSemRes, 1, NULL);
		Sleep(10000);
		ReleaseSemaphore(hSemEsc, 1, NULL);
		if (aux.aceite) {
			WaitForSingleObject(hMutex, INFINITE);
			sh->taxis[sh->nTaxis].id = aux.id;
			_tcscpy_s(sh->taxis[sh->nTaxis].matricula, sizeof(sh->taxis[sh->nTaxis].matricula) / sizeof(TCHAR), aux.matricula);
			sh->nTaxis++;
			_tprintf(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), sh->taxis[sh->nTaxis - 1].matricula, sh->taxis[sh->nTaxis - 1].id);
			_tprintf(TEXT("\n\tComando: "));
			ReleaseMutex(hMutex);
		}
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hSemEsc);
	CloseHandle(hSemLei);
	CloseHandle(hSemRes);
	CloseHandle(hMutex);
}
