#include "CenTaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCriaTaxis, hThreadMapa, hThreadComunicacao, hThreadSai, hEventThread, hThreadComandos;
	Centaxi m;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadMapa = CreateThread(NULL, 0, carregaMapa, &m, 0, NULL);

	hThreadCriaTaxis = CreateThread(NULL, 0, threadCriaTaxis, &m, 0, NULL);
	
	m.sair = 0;
	m.nTaxis = 0;
	if (argc == 2) {
		m.maxTaxis = atoi(argv[1]);
	}
	else if (argc == 3) {
		m.maxTaxis = atoi(argv[1]);
		m.maxPass = atoi(argv[2]);
	}
	else {
		m.maxTaxis = LIMITE_TAXIS;
		m.maxPass = LIMITE_PASS;
	}
	hThreadComunicacao = CreateThread(NULL, 0, threadCom, &m, 0, NULL);
	hThreadComandos = CreateThread(NULL, 0, threadComandos, &m, 0, NULL);
	//hThreadSai = CreateThread(NULL, 0, threadSaiTaxi, &m, 0, NULL);


	WaitForSingleObject(hThreadCriaTaxis, INFINITE);
	WaitForSingleObject(hThreadComunicacao, INFINITE);
	_getch();
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
}

void mostraMapa(Centaxi* m) {
	_tprintf(TEXT("\n"));
	for (int i = 0, k=0; i< m->alturaMapa; i++) {
		for (int j = 0; j< m->larguraMapa; j++, k++) {
			if (m->mapa[k] == 1) {
				_tprintf(TEXT("_"));
			}
			else if(m->mapa[k] == 2){
				_tprintf(TEXT("C"));
			}
			else {
				_tprintf(TEXT("X"));
			}
		}
		_tprintf(TEXT("\n"));
	}
}

int trataComando(TCHAR comando[], Centaxi* m) {
	if (!_tcscmp(comando, TEXT("encerraTudo"))) {
		return sair(m);
	}
	else if (!_tcscmp(comando, TEXT("listaTaxis"))) {
		listaTaxis(m);
	}
	else if (!_tcscmp(comando, TEXT("mostraMapa"))) {
		mostraMapa(m);
	}
	return 0;
}

int sair(Centaxi* m) {
	HANDLE hMutex, hSem, hEvent;

	m->sair = 1;
	
	hSem = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_LEI);
	if (hSem == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());
		return -1;
	}
	ReleaseSemaphore(hSem, 1, NULL);

	hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_TAXI_SAI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());
		return -1;
	}

	hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_SAI_TAXI);
	SetEvent(hEvent);
	SetEvent(hMutex);
	CloseHandle(hSem);
	CloseHandle(hMutex);
	return -1;
}

void listaTaxis(Centaxi* m) {

	HANDLE hMutex;

	limpaEcra();
	hMutex = CreateMutex(NULL, FALSE, ATUALIZA_ARRAY_TAXIS);
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
	CloseHandle(hMutex);
}

void limpaEcra() {
	for(int i=0; i<30; i++)
		_tprintf(TEXT("\n"));
}

DWORD WINAPI carregaMapa(LPVOID lpParam) {

	FILE* file = NULL;
	int c, altura, i, largura, j;
	HANDLE hMapFile;
	Centaxi* m = (Centaxi*)lpParam;
	int* mapa;

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
	m->larguraMapa = i;

	fclose(file);

	err = fopen_s(&file, ".\\..\\f.txt", "r");

	if (file == NULL) {
		_tprintf(TEXT("%d"), err);
		return NULL;
	}

	c = fgetc(file);
	for (i = 0, j = 0; feof(file) == 0; i++)
	{
		if ((char)c == TEXT('\n')) {
			j++;
		}
		c = fgetc(file);
	}
	fclose(file);

	m->alturaMapa = j + 1;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int) * m->alturaMapa * m->larguraMapa, MEMPAR_MAPA);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}

	mapa = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * m->alturaMapa * m->larguraMapa);

	if (mapa == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	err = fopen_s(&file, ".\\..\\f.txt", "r");

	if (file == NULL) {
		_tprintf(TEXT("%d"), err);
		return NULL;
	}

	c = fgetc(file);
	for (i = 0; feof(file) == 0; i++)
	{
		if ((char)c == TEXT('_')) {
			mapa[i] = 1;
		}
		else if ((char)c == TEXT('X')) {
			mapa[i] = 0;
		}
		else {
			i--;
		}
		c = fgetc(file);
	}

	fclose(file);
	m->mapa = mapa;
}

DWORD WINAPI threadCriaTaxis(LPVOID lpParam) {
	HANDLE hMapFile;
	Centaxi* m = (Centaxi*)lpParam;
	Taxi* t;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m->maxTaxis * sizeof(Taxi), MEMPAR_TAXIS);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}

	t = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi) * m->maxTaxis);

	if (t == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}
	ZeroMemory(t, sizeof(Taxi) * m->maxTaxis);

	m->taxis = t;
}

DWORD WINAPI threadComandos(LPVOID lpParam) {
	TCHAR comando[256];
	int i, op;
	Centaxi* m = (Centaxi*)lpParam;

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

	HANDLE hMapFile, hSemLei, hSemEsc, hSemRes, hSemAtualizaEsc, hSemAtualizaLei, hMutex;
	Taxi aux;
	Taxi* t;
	Centaxi* m = (Centaxi*)lpParam;
	int pos;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Taxi), MEMPAR_TAXI_NOVO);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}

	t = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

	if (t == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	hSemEsc = CreateSemaphore(NULL, 1, 1, MUTEX_NOVO_TAXI_ESC);
	if (hSemEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hMapFile);

		return -1;
	}

	hSemLei = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_LEI);
	if (hSemLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hMapFile);
		
		return -1;
	}

	hSemRes = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_RES);
	if (hSemRes == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de resposta (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hMapFile);

		return -1;
	}

	hSemAtualizaEsc = CreateSemaphore(NULL, 1, 1, MUTEX_PODE_ATUALIZAR_ARRAY_ESC); 
	if (hSemAtualizaEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hSemRes);
		CloseHandle(hMapFile);

		return -1;
	}

	hSemAtualizaLei = CreateSemaphore(NULL, 0, 1, MUTEX_PODE_ATUALIZAR_ARRAY_LEI);
	if (hSemAtualizaLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hSemRes);
		CloseHandle(hSemAtualizaEsc);
		CloseHandle(hMapFile);

		return -1;
	}

	hMutex = CreateMutex(NULL, FALSE, ATUALIZA_ARRAY_TAXIS);
	if (hMutex == NULL) {
		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hSemRes);
		CloseHandle(hMapFile);
		CloseHandle(hSemAtualizaEsc);
		CloseHandle(hSemAtualizaLei);
		return -1;
	}
	while (!m->sair) {
		WaitForSingleObject(hSemLei, INFINITE);
		if (m->sair)
			break;
		aux.id = t->id;
		aux.x = t->x;
		aux.y = t->y;
		_tcscpy_s(aux.matricula, sizeof(aux.matricula) / sizeof(TCHAR), t->matricula);
		
		if (m->nTaxis != m->maxTaxis) {
			for (int j = 0; j < m->nTaxis; j++) {
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
			aux.larguraMapa = m->larguraMapa;
			aux.alturaMapa = m->alturaMapa;
		}
		CopyMemory(t, &aux, sizeof(Taxi));
		ReleaseSemaphore(hSemRes, 1, NULL);
		if (aux.aceite) {
			//WaitForSingleObject(hSemAtualizaEsc, INFINITE);
			WaitForSingleObject(hMutex, INFINITE);
			m->taxis[m->nTaxis].id = aux.id;
			m->taxis[m->nTaxis].x = aux.x;
			m->taxis[m->nTaxis].y = aux.y;
			_tcscpy_s(m->taxis[m->nTaxis].matricula, sizeof(m->taxis[m->nTaxis].matricula) / sizeof(TCHAR), aux.matricula);
			m->nTaxis++;
			_tprintf(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), m->taxis[m->nTaxis - 1].matricula, m->taxis[m->nTaxis - 1].id);
			_tprintf(TEXT("\n\tComando: "));
			ReleaseMutex(hMutex);
			//ReleaseSemaphore(hSemAtualizaLei, 1, NULL);
		}
		ReleaseSemaphore(hSemEsc, 1, NULL);
	}

	UnmapViewOfFile(t);

	CloseHandle(hMapFile);
	CloseHandle(hSemEsc);
	CloseHandle(hSemLei);
	CloseHandle(hSemRes);
	CloseHandle(hSemAtualizaEsc);
	CloseHandle(hSemAtualizaLei);
}
/*
DWORD WINAPI threadSaiTaxi(LPVOID lpParam) {

	HANDLE hMapFile, hMutex = NULL, hEvent = NULL, hMutexS;
	Taxi* sM;
	Mapa* m = (Mapa*)lpParam;
	Taxi t;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Taxi), MEMPAR_SAI_TAXI);

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

	hMutexS = CreateMutex(NULL, FALSE, MUTEX_TAXI_SAI);
	if (hMutexS == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);

		return -1;
	}
	hMutex = CreateMutex(NULL, FALSE, MUTEX_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		CloseHandle(hMutexS);
		return -1;
	}
	while (!m->sair) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_SAI_TAXI);
		WaitForSingleObject(hEvent, INFINITE);

		WaitForSingleObject(hMutexS, INFINITE);

		if (m->sair)
			break;

		for (int i = 0; i < m->nTaxis; i++) {
			if (!_tcscmp(m->taxis[i].matricula, sM->matricula)) {
				_tprintf(TEXT("\n\tTaxi com a matricula %s e ID %d acabou de sair!"), m->taxis[i].matricula, m->taxis[i].id);
				_tprintf(TEXT("\n\tComando: "));
				WaitForSingleObject(hMutex, INFINITE);
				for (int j = i; j < m->nTaxis-1; j++) {
					m->taxis[j].aceite = m->taxis[j + 1].aceite;
					m->taxis[j].id = m->taxis[j + 1].id;
					m->taxis[j].x = m->taxis[j + 1].x;
					m->taxis[j].y = m->taxis[j + 1].y;
					_tcscpy_s(m->taxis[j].matricula, sizeof(m->taxis[j].matricula), m->taxis[j+1].matricula);
				}
				m->nTaxis--;
				ReleaseMutex(hMutex);
				break;
			}
		}

		ReleaseMutex(hMutexS);
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hMutexS);
}

void informaMapaAoTaxi(Centaxi* mapa) {
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
}*/