#include "CenTaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread, hEventThread, hThreadComandos;
	Mapa m;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	carregaMapa(&m);
	mostraMapa(&m);

	m.sair = 0;
	m.nTaxis = 0;
	hThread = CreateThread(NULL, 0, threadCom, &m, 0, NULL);
	hThreadComandos = CreateThread(NULL, 0, threadComandos, &m, 0, NULL);


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
	_tprintf(TEXT("\n\tmostraMapa: Mostra o mapa atualizado"));
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
			if (m->estrada[i][j] == 1) {
				_tprintf(TEXT("_"));
			}
			else if(m->estrada[i][j] == 2){
				_tprintf(TEXT("C"));
			}
			else {
				_tprintf(TEXT("X"));
			}
		}
		_tprintf(TEXT("\n"));
	}
}

int trataComando(TCHAR comando[], Mapa* m) {
	if (!_tcscmp(comando, TEXT("sair"))) {
		return sair(m);
	}
	else if (!_tcscmp(comando, TEXT("encerraTudo"))) {
		encerraTudo();
	}
	else if (!_tcscmp(comando, TEXT("listaTaxis"))) {
		listaTaxis(m);
	}
	else if (!_tcscmp(comando, TEXT("mostraMapa"))) {
		mostraMapa(m);
	}
	return 0;
}

int sair(Mapa* m) {
	HANDLE hEvent, hSem;

	m->sair = 1;
	
	hSem = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_LEI);
	if (hSem == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());
		return -1;
	}
	ReleaseSemaphore(hSem, 1, NULL);

	encerraTudo();
	return -1;
}

void encerraTudo() {
	HANDLE hEvent;
	hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, EVENTO_ENCERRA_TUDO);
	SetEvent(hEvent);
}

void listaTaxis(Mapa * m) {

	HANDLE hMutex;

	limpaEcra();
	hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);
	if (m->nTaxis == 0) {
		_tprintf(TEXT("\n\tSem taxis registados\n"));
		ReleaseMutex(hMutex);
		return;
	}
	for (int i = 0; i < m->nTaxis; i++) {
		_tprintf(TEXT("\n\tTaxi %d: %s (matricula) %d (id)"), i+1, m->taxis[i].matricula, m->taxis[i].id);
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
	Mapa* m = (Mapa*)lpParam;

	limpaEcra();
	do {
		_tprintf(TEXT("\tCentral de taxis.\n\n"));
		mostraComandos();
		_tprintf(TEXT("\n\n\tComando: "));
		_fgetts(comando, 256, stdin);
		for (i = 0; comando[i] != '\n'; i++);
		comando[i] = '\0';
		op = trataComando(comando, m);
	} while (op != -1);
	return 0;
}

DWORD WINAPI threadCom(LPVOID lpParam) {

	HANDLE hMapFile, hSemLei, hSemEsc, hSemRes, hMutex;
	Taxi taxi;
	Taxi aux;
	Taxi* sM = &taxi;
	TCHAR buff[256];
	Mapa* m = (Mapa*)lpParam;
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

	hSemEsc = CreateSemaphore(NULL, 1, 1, MUTEX_NOVO_TAXI_ESC);
	if (hSemEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
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

	hSemRes = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_RES);
	if (hSemRes == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de resposta (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);

		return -1;
	}
	hMutex = CreateMutex(NULL, FALSE, MUTEX_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);

		return -1;
	}

	ZeroMemory(taxi.matricula, sizeof(taxi.matricula));
	CopyMemory(sM, &taxi, sizeof(Taxi));

	while (!m->sair) {
		WaitForSingleObject(hSemLei, INFINITE);
		if (m->sair)
			break;
		aux.id = sM->id;
		aux.x = sM->x;
		aux.y = sM->y;
		_tcscpy_s(aux.matricula, sizeof(aux.matricula) / sizeof(TCHAR), sM->matricula);
		pos = m->nTaxis;

		if (m->nTaxis != LIMITE_TAXIS) {
			for (int j = 0; j < pos; j++) {
				if (!_tcscmp(aux.matricula, m->taxis[j].matricula)) {
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
		if (aux.aceite) {
			informaMapaAoTaxi(m);
			WaitForSingleObject(hMutex, INFINITE);
			m->taxis[m->nTaxis].id = aux.id;
			m->taxis[m->nTaxis].x = aux.x;
			m->taxis[m->nTaxis].y = aux.y;
			m->estrada[aux.x][aux.y] = 2;
			_tcscpy_s(m->taxis[m->nTaxis].matricula, sizeof(m->taxis[m->nTaxis].matricula) / sizeof(TCHAR), aux.matricula);
			m->nTaxis++;
			_tprintf(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), m->taxis[m->nTaxis - 1].matricula, m->taxis[m->nTaxis - 1].id);
			_tprintf(TEXT("\n\tComando: "));
			ReleaseMutex(hMutex);
		}
		ReleaseSemaphore(hSemEsc, 1, NULL);
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hSemEsc);
	CloseHandle(hSemLei);
	CloseHandle(hSemRes);
	CloseHandle(hMutex);
}

void informaMapaAoTaxi(Mapa * mapa) {
	HANDLE hMapFile, hEvent;
	Mapa* m;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Mapa), INFORMA_MAPA);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}

	m = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Mapa));

	if (m == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	hEvent = CreateEvent(NULL, TRUE, FALSE, MUTEX_MAPA);
	if (hEvent == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(m);
		CloseHandle(hMapFile);

		return -1;
	}
	CopyMemory(m, mapa, sizeof(Mapa));
	WaitForSingleObject(hEvent, INFINITE);
	ResetEvent(hEvent);

	UnmapViewOfFile(m);
	CloseHandle(hEvent);
	CloseHandle(hMapFile);
}