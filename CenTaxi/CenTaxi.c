#include "CenTaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCriaTaxis, hThreadInformaMapa, hThreadMapa, hThreadComunicacao, hThreadSai, hEventThread, hThreadComandos, hLib;
	Centaxi m;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadMapa = CreateThread(NULL, 0, carregaMapa, &m, 0, NULL);
	WaitForSingleObject(hThreadMapa, INFINITE);

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));

	dll_log dll_logV = (dll_log)GetProcAddress(hLib, "dll_log");

	if (m.sair == 1) {
		_tprintf(TEXT("\n\tDimensões do mapa insuficientes"));
		dll_logV(TEXT("\n\tDimensões do mapa insuficientes"));
		return 0;
	}
	hThreadCriaTaxis = CreateThread(NULL, 0, threadCriaTaxis, &m, 0, NULL);
	hThreadInformaMapa = CreateThread(NULL, 0, informaMapa, &m, 0, NULL);

	m.aceitaTaxis = 1;
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
	hThreadComunicacao = CreateThread(NULL, 0, threadComunicaTaxis, &m, 0, NULL);
	hThreadComandos = CreateThread(NULL, 0, threadComandos, &m, 0, NULL);
	hThreadSai = CreateThread(NULL, 0, threadSaiTaxi, &m, 0, NULL);


	WaitForSingleObject(hThreadInformaMapa, INFINITE);
	WaitForSingleObject(hThreadCriaTaxis, INFINITE);
	WaitForSingleObject(hThreadComunicacao, INFINITE);
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
	else if (!_tcscmp(comando, TEXT("atuaAceitacao"))) {
		atuaAceitacao(m);
	}
	return 0;
}

void atuaAceitacao(Centaxi* m) {
	if (m->aceitaTaxis == 0)
		m->aceitaTaxis = 1;
	else
		m->aceitaTaxis = 0;
}

int sair(Centaxi* m) {
	HANDLE hSem, hEvent, hMutex, hLib;

	m->sair = 1;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");

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
	
	hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_ENCERRA_TUDO);
	dll_registerV(EVENTO_ENCERRA_TUDO, 4);
	SetEvent(hEvent);
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
		_tprintf(TEXT("\n\tTaxi %d: %s (matricula) %d (id) a andar a uma velocidade de %.1f"), i+1, m->taxis[i].matricula, m->taxis[i].id, m->taxis[i].velocidade);
	}
	ReleaseMutex(hMutex);
	_tprintf(TEXT("\n"));
	CloseHandle(hMutex);
}

void limpaEcra() {
	for(int i=0; i<30; i++)
		_tprintf(TEXT("\n"));
}

DWORD WINAPI informaMapa(LPVOID lpParam) {

	HANDLE hMapFile, hSem, hLib;
	Mapinfo* m;
	Mapinfo mapa;
	Centaxi* c = (Centaxi*)lpParam;


	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Mapinfo), MEMPAR_INFORMA_MAPA);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_INFORMA_MAPA, 6);

	m = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Mapinfo));

	if (m == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	dll_registerV(MEMPAR_INFORMA_MAPA, 7);

	hSem = CreateSemaphore(NULL, 0, 1, SEM_PODE_FECHAR_INFORMA_MAPA);
	if (hSem == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo (%d).\n"), GetLastError());

		UnmapViewOfFile(m);
		CloseHandle(hMapFile);

		return -1;
	}

	dll_registerV(SEM_PODE_FECHAR_INFORMA_MAPA, 3);

	mapa.alturaMapa = c->alturaMapa;
	mapa.larguraMapa = c->larguraMapa;
	mapa.maxPass = c->maxPass;
	mapa.maxTaxis = c->maxTaxis;

	CopyMemory(m, &mapa, sizeof(Mapinfo));

	WaitForSingleObject(hSem, INFINITE);

	UnmapViewOfFile(m);
	CloseHandle(hMapFile);
	CloseHandle(hSem);
	return;
}

DWORD WINAPI carregaMapa(LPVOID lpParam) {

	FILE* file = NULL;
	int c, altura, i, largura, j;
	HANDLE hMapFile, hLib;
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

	/*if (m->alturaMapa < 50 || m->larguraMapa < 50) {
		m->sair = 1;
		return;
	}*/

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int) * m->alturaMapa * m->larguraMapa, MEMPAR_MAPA);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	
	dll_registerV(MEMPAR_MAPA, 6);

	mapa = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * m->alturaMapa * m->larguraMapa);

	if (mapa == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}
	dll_registerV(MEMPAR_MAPA, 7);

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
	HANDLE hMapFile, hLib;
	Centaxi* m = (Centaxi*)lpParam;
	Taxi* t;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m->maxTaxis * sizeof(Taxi), MEMPAR_TAXIS);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_TAXIS, 6);

	t = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi) * m->maxTaxis);

	if (t == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	dll_registerV(MEMPAR_TAXIS, 7);
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

DWORD WINAPI threadComunicaTaxis(LPVOID lpParam) {

	HANDLE hMapFile, hSemLei, hSemEsc, hSemRes, hSemAtualizaEsc, hSemAtualizaLei, hMutex, hLib;
	Taxi aux;
	Taxi* t;
	Centaxi* m = (Centaxi*)lpParam;
	int pos;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");
	dll_log dll_logV = (dll_log)GetProcAddress(hLib, "dll_log");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Taxi), MEMPAR_TAXI_NOVO);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_TAXI_NOVO, 6);
	t = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

	if (t == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}
	dll_registerV(MEMPAR_TAXI_NOVO, 7);

	hSemEsc = CreateSemaphore(NULL, 1, 1, MUTEX_NOVO_TAXI_ESC);
	if (hSemEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hMapFile);

		return -1;
	}
	dll_registerV(MUTEX_NOVO_TAXI_ESC, 3);

	hSemLei = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_LEI);
	if (hSemLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hMapFile);
		
		return -1;
	}
	dll_registerV(MUTEX_NOVO_TAXI_LEI, 3);

	hSemRes = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_RES);
	if (hSemRes == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de resposta (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hMapFile);

		return -1;
	}

	dll_registerV(MUTEX_NOVO_TAXI_RES, 3);

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
	dll_registerV(MUTEX_PODE_ATUALIZAR_ARRAY_ESC, 3);

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
	dll_registerV(MUTEX_PODE_ATUALIZAR_ARRAY_LEI, 3);

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
	dll_registerV(ATUALIZA_ARRAY_TAXIS, 1);
	while (!m->sair) {
		WaitForSingleObject(hSemLei, INFINITE);
		if (m->sair)
			break;
		aux.atualizaMovimentacao = t->atualizaMovimentacao;
		if (aux.atualizaMovimentacao == 0) {
			aux.id = t->id;
			aux.x = t->x;
			aux.y = t->y;
			aux.velocidade = t->velocidade;
			aux.aceite = 1;
			_tcscpy_s(aux.matricula, sizeof(aux.matricula) / sizeof(TCHAR), t->matricula);

			if (m->nTaxis != m->maxTaxis) {
				for (int j = 0; j < m->nTaxis; j++) {
					if (!_tcscmp(aux.matricula, m->taxis[j].matricula)) {
						aux.aceite = 0;
						break;
					}
				}
				if (m->aceitaTaxis == 0) {
					aux.aceite = 0;
				}
			}
			else {
				aux.aceite = 0;
			}
			if (aux.aceite != 0) {
				for (int i = 0, k = 0; i < m->alturaMapa; i++) {
					for (int j = 0; j < m->larguraMapa; j++, k++) {
						if (i == aux.x && j == aux.y && m->mapa[k] == 0) {
							aux.aceite = 0;
							break;
						}
						else {
							aux.aceite = 1;
						}
					}
					if (aux.aceite == 0) {
						break;
					}
				}
				if (aux.aceite == 1) {
					aux.larguraMapa = m->larguraMapa;
					aux.alturaMapa = m->alturaMapa;
					aux.atualizaMovimentacao = 1;
				}
			}
			CopyMemory(t, &aux, sizeof(Taxi));
			ReleaseSemaphore(hSemRes, 1, NULL);
			if (aux.aceite) {
				WaitForSingleObject(hSemAtualizaEsc, INFINITE);
				WaitForSingleObject(hMutex, INFINITE);
				m->taxis[m->nTaxis].id = aux.id;
				m->taxis[m->nTaxis].x = aux.x;
				m->taxis[m->nTaxis].y = aux.y;
				m->taxis[m->nTaxis].velocidade = aux.velocidade;
				_tcscpy_s(m->taxis[m->nTaxis].matricula, sizeof(m->taxis[m->nTaxis].matricula) / sizeof(TCHAR), aux.matricula);
				m->nTaxis++;
				_tprintf(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), m->taxis[m->nTaxis - 1].matricula, m->taxis[m->nTaxis - 1].id);
				dll_logV(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), m->taxis[m->nTaxis - 1].matricula, m->taxis[m->nTaxis - 1].id);
				_tprintf(TEXT("\n\tComando: "));
				ReleaseMutex(hMutex);
				ReleaseSemaphore(hSemAtualizaLei, 1, NULL);
			}
			ReleaseSemaphore(hSemEsc, 1, NULL);
		}
		else {
			_tcscpy_s(aux.matricula, sizeof(aux.matricula) / sizeof(TCHAR), t->matricula);
			WaitForSingleObject(hSemAtualizaEsc, INFINITE);
			WaitForSingleObject(hMutex, INFINITE);
			for (pos = 0; pos < m->nTaxis; pos++) {
				if (!_tcscmp(aux.matricula, m->taxis[pos].matricula)) {
					aux = *t;
					break;
				}
			}
			m->taxis[pos].x = aux.x;
			m->taxis[pos].y = aux.y;
			m->taxis[pos].yA = aux.yA;
			m->taxis[pos].xA = aux.xA;
			m->taxis[pos].velocidade = aux.velocidade;
			ReleaseMutex(hMutex);
			ReleaseSemaphore(hSemAtualizaLei, 1, NULL);
			ReleaseSemaphore(hSemRes, 1, NULL);
			ReleaseSemaphore(hSemEsc, 1, NULL);
		}
	}

	UnmapViewOfFile(t);

	CloseHandle(hMapFile);
	CloseHandle(hSemEsc);
	CloseHandle(hSemLei);
	CloseHandle(hSemRes);
	CloseHandle(hSemAtualizaEsc);
	CloseHandle(hSemAtualizaLei);
}

DWORD WINAPI threadSaiTaxi(LPVOID lpParam) {

	HANDLE hMapFile, hMutex = NULL, hEvent = NULL, hSemAtualizaEsc, hSemAtualizaLei, hMutexS, hLib;
	Taxi* sM;
	Centaxi* m = (Centaxi*)lpParam;
	Taxi t;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");
	dll_log dll_logV = (dll_log)GetProcAddress(hLib, "dll_log");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Taxi), MEMPAR_SAI_TAXI);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_SAI_TAXI, 6);
	sM = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

	if (sM == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}
	dll_registerV(MEMPAR_SAI_TAXI, 7);
	hMutexS = CreateMutex(NULL, FALSE, ATUALIZA_ARRAY_TAXIS);
	if (hMutexS == NULL) {
		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		return -1;
	}

	hMutex = CreateMutex(NULL, FALSE, MUTEX_TAXI_SAI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		return -1;
	}
	hSemAtualizaEsc = CreateSemaphore(NULL, 1, 1, MUTEX_PODE_ATUALIZAR_ARRAY_ESC);
	if (hSemAtualizaEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		CloseHandle(hMutex);

		return -1;
	}

	hSemAtualizaLei = CreateSemaphore(NULL, 0, 1, MUTEX_PODE_ATUALIZAR_ARRAY_LEI);
	if (hSemAtualizaLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		CloseHandle(hMutex);
		CloseHandle(hSemAtualizaEsc);

		return -1;
	}
	while (!m->sair) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_SAI_TAXI);
		dll_registerV(EVENTO_SAI_TAXI, 4);
		WaitForSingleObject(hEvent, INFINITE);
		if (m->sair)
			break;

		WaitForSingleObject(hMutex, INFINITE);
		for (int i = 0; i < m->nTaxis; i++) {
			if (!_tcscmp(m->taxis[i].matricula, sM->matricula)) {
				_tprintf(TEXT("\n\tTaxi com a matricula %s e ID %d acabou de sair!"), m->taxis[i].matricula, m->taxis[i].id);
				dll_logV(TEXT("\n\tTaxi com a matricula %s e ID %d acabou de sair!"), m->taxis[i].matricula, m->taxis[i].id);
				_tprintf(TEXT("\n\tComando: "));
				ReleaseMutex(hMutex);
				WaitForSingleObject(hSemAtualizaEsc, INFINITE);
				WaitForSingleObject(hMutexS, INFINITE);
				for (int j = i; j < m->nTaxis-1; j++) {
					m->taxis[j].aceite = m->taxis[j + 1].aceite;
					m->taxis[j].id = m->taxis[j + 1].id;
					m->taxis[j].x = m->taxis[j + 1].x;
					m->taxis[j].y = m->taxis[j + 1].y;
					_tcscpy_s(m->taxis[j].matricula, sizeof(m->taxis[j].matricula), m->taxis[j+1].matricula);
				}
				m->taxis[m->nTaxis-1].aceite = 0; 
				m->taxis[m->nTaxis-1].id = -1;
				m->nTaxis--;
				ReleaseMutex(hMutexS);
				ReleaseSemaphore(hSemAtualizaLei, 1, NULL);
				break;
			}
		}
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hMutex);
}