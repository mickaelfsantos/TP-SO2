#include "CenTaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCriaTaxis, hThreadInformaMapa, hThreadMapa, hThreadComunicacao, hThreadSai, hEventThread, hThreadComandos, hLib, hThreadPassageiros, hThreadCriaPassageiros;
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

	m.aceitaTaxis = 1;
	m.sair = 0;
	m.nTaxis = 0;
	m.nPass = 0;
	m.nQuadriculas = NQ;
	m.nSegundos = SEGUNDOSDEFAULT;
	if (argc == 2) {
		m.maxTaxis = atoi(argv[1]);
		m.maxPass = LIMITE_PASS;
	}
	else if (argc == 3) {
		m.maxTaxis = atoi(argv[1]);
		m.maxPass = atoi(argv[2]);
	}
	else {
		m.maxTaxis = LIMITE_TAXIS;
		m.maxPass = LIMITE_PASS;
	}


	hThreadCriaTaxis = CreateThread(NULL, 0, threadCriaTaxis, &m, 0, NULL);
	hThreadCriaPassageiros = CreateThread(NULL, 0, threadCriaPassageiros, &m, 0, NULL);
	
	hThreadPassageiros = CreateThread(NULL, 0, threadPassageiros, &m, 0, NULL);
	hThreadInformaMapa = CreateThread(NULL, 0, informaMapa, &m, 0, NULL);

	
	hThreadComunicacao = CreateThread(NULL, 0, threadComunicaTaxis, &m, 0, NULL);
	hThreadComandos = CreateThread(NULL, 0, threadComandos, &m, 0, NULL);
	hThreadSai = CreateThread(NULL, 0, threadSaiTaxi, &m, 0, NULL);


	WaitForSingleObject(hThreadInformaMapa, INFINITE);
	WaitForSingleObject(hThreadCriaTaxis, INFINITE);
	WaitForSingleObject(hThreadCriaPassageiros, INFINITE);
	WaitForSingleObject(hThreadComunicacao, INFINITE);
	return 0;
}

void mostraComandos() {
	_tprintf(TEXT("\n\t---Comandos disponíveis---\n"));
	_tprintf(TEXT("\n\t1- expulsaTaxi T: expulsa o taxi T, se estiver sem passageiros"));
	_tprintf(TEXT("\n\t2- encerraTudo: encerra todos os processos"));
	_tprintf(TEXT("\n\t3- listaTaxis: lista taxis e estado atual assim como passageiros em transporte e em espera"));
	_tprintf(TEXT("\n\t4- atuaAceitacao: Pausa ou retoma a aceitacao de taxis"));
	_tprintf(TEXT("\n\t5- defineDuracao: Tempo que a CenTaxi aguarda por um taxi manifestar interesse em transporte um passageiro"));
	_tprintf(TEXT("\n\t6- mostraMapa: Mostra o mapa atualizado"));
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

int trataComando(int op, Centaxi* m) {
	switch (op)
	{
	case 1:
		break;
	case 2:
		return sair(m);
	case 3:
		listaTaxis(m);
		break;
	case 4:
		atuaAceitacao(m);
		break;
	case 5:
		break;
	case 6:
		mostraMapa(m);
		break;
	default:
		return 0;
	}
}

void atuaAceitacao(Centaxi* m) {
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, FALSE, CENTAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);

	if (m->aceitaTaxis == 0)
		m->aceitaTaxis = 1;
	else
		m->aceitaTaxis = 0;
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
}

int sair(Centaxi* m) {
	HANDLE hEvent, hMutex, hLib;

	
	hMutex = CreateMutex(NULL, FALSE, CENTAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());
		return;
	}
	WaitForSingleObject(hMutex, INFINITE);
	m->sair = 1;
	ReleaseMutex(hMutex);

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");
	
	hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_ENCERRA_TUDO);
	dll_registerV(EVENTO_ENCERRA_TUDO, 4);
	SetEvent(hEvent);
	return -1;
}

void listaTaxis(Centaxi* m) {

	HANDLE hMutex;

	limpaEcra();
	hMutex = CreateMutex(NULL, FALSE, CENTAXI);
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

DWORD WINAPI threadCriaPassageiros(LPVOID lpParam) {
	HANDLE hMapFile, hLib;
	Centaxi* m = (Centaxi*)lpParam;
	Passageiro* t;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, m->maxPass * sizeof(Passageiro), MEMPAR_PASS);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_PASS, 6);

	t = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Passageiro) * m->maxPass);

	if (t == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	dll_registerV(MEMPAR_PASS, 7);
	ZeroMemory(t, sizeof(Passageiro) * m->maxPass);

	m->passageiros = t;
}

DWORD WINAPI threadPassageiros(LPVOID lpParam) {
	HANDLE hMapFile, hLib, hPipe, hMutex, hMutexBufferCircular, hEvento[LIMITE_PASS], hThreads[LIMITE_PASS], hMapFileInteressados;
	Centaxi* m = (Centaxi*)lpParam;
	BufferCircular b;
	BufferCircular* bc = &b;
	Passageiro passageiro;
	DWORD dwRead, dwWrite, dwId;
	TCHAR buffer[BUFFSIZE];
	Interessados* st;
	int saltar=0;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");

	b.w = 0;
	b.r = 0;
	ZeroMemory(b.passageiros, sizeof(Passageiro) * LIMITE_PASS);

	hMapFileInteressados = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Interessados)*m->maxTaxis, MEMPAR_INT);

	if (hMapFileInteressados == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_INT, 6);

	st = (Interessados*)MapViewOfFile(hMapFileInteressados, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Interessados)*m->maxTaxis);

	if (st == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFileInteressados);
		return -1;
	}
	dll_registerV(MEMPAR_INT, 7);

	for (int i = 0; i < m->maxTaxis; i++) {
		_tcscpy_s(st[i].matricula, sizeof(st[i].matricula) / sizeof(TCHAR), TEXT(""));
		_tcscpy_s(st[i].nomePassageiro, sizeof(st[i].nomePassageiro) / sizeof(TCHAR), TEXT(""));
		st[i].nSegundos = m->nSegundos;
	}

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BufferCircular), MEMPAR_PASS);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}
	dll_registerV(MEMPAR_PASS, 6);

	bc = (BufferCircular*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(BufferCircular));

	if (bc == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}
	dll_registerV(MEMPAR_PASS, 7);

	hPipe = CreateNamedPipe(PIPENAME_NOVO, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
		sizeof(Passageiro), sizeof(Passageiro), 0, NULL);

	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Problema ao criar named pipe do servidor: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	dll_registerV(PIPENAME_NOVO, 8);


	hMutex = CreateMutex(NULL, FALSE, CENTAXI);

	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	dll_registerV(CENTAXI, 1);


	hMutexBufferCircular = CreateMutex(NULL, FALSE, BUFFERCIRCULAR);

	if (hMutexBufferCircular == NULL) {
		_tprintf(TEXT("Erro ao criar mutex: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	dll_registerV(BUFFERCIRCULAR, 1);

	for (int i = 0; i < LIMITE_PASS; i++) {
		_itot_s(i, buffer, sizeof(buffer) / sizeof(TCHAR), 2);
		hEvento[i] = CreateEvent(NULL, TRUE, FALSE, buffer);
		if (hEvento[i] == NULL) {
			_tprintf(TEXT("Erro ao criar semáforos: %d\n"), GetLastError());
			return EXIT_FAILURE;
		}
		dll_registerV(buffer, 3);
	}

	while (!m->sair) {

		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(TEXT("[ERRO] Ligação ao pipe do servior!\n"));
			exit(EXIT_FAILURE);
		}

		if (!ReadFile(hPipe, &passageiro, sizeof(Passageiro), &dwRead, NULL)) {
			_tprintf(TEXT("[ERRO] Leitura do named pipe: %d\n"), GetLastError());
			exit(EXIT_FAILURE);
		}
		if (!dwRead) {
			_tprintf(TEXT("[ERRO] Não foram lidos bytes \n"));
			exit(EXIT_FAILURE);
		}

		WaitForSingleObject(hMutexBufferCircular, INFINITE);
		if ((b.w == LIMITE_PASS && b.r == 0) || b.w + 1 == b.r) {
			_tcscpy_s(passageiro.resposta, sizeof(passageiro.resposta) / sizeof(TCHAR), TEXT("Buffer circular cheio. Tente mais tarde."));
			if (!WriteFile(hPipe, &passageiro, sizeof(Passageiro), &dwWrite, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
				exit(EXIT_FAILURE);
			}
			ReleaseMutex(hMutexBufferCircular);
			DisconnectNamedPipe(hPipe);
			continue;
		}
		WaitForSingleObject(hMutex, INFINITE);
		if (*(m->mapa + passageiro.x * m->larguraMapa + passageiro.y) == 0) {
			_tcscpy_s(passageiro.resposta, sizeof(passageiro.resposta) / sizeof(TCHAR), TEXT("Posição invalida. Insira-se na estrada"));
			if (!WriteFile(hPipe, &passageiro, sizeof(Passageiro), &dwWrite, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
				exit(EXIT_FAILURE);
			}
			ReleaseMutex(hMutex);
			ReleaseMutex(hMutexBufferCircular);
			DisconnectNamedPipe(hPipe);
			continue;
		}

		for (int i = 0; i < m->nPass; i++) {
			if (_tcscmp(m->passageiros[i].nome, passageiro.nome) == 0) {
				_tcscpy_s(passageiro.resposta, sizeof(passageiro.resposta) / sizeof(TCHAR), TEXT("Esse passageiro já se encontra registado"));
				if (!WriteFile(hPipe, &passageiro, sizeof(Passageiro), &dwWrite, NULL)) {
					_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
					exit(EXIT_FAILURE);
				}
				ReleaseMutex(hMutexBufferCircular);
				ReleaseMutex(hMutex);
				DisconnectNamedPipe(hPipe);
				saltar = 1;
			}
		}
		if (saltar == 1) {
			saltar = 0;
			continue;
		}

		m->passageiros[m->nPass].estado = passageiro.estado;
		m->passageiros[m->nPass].x = passageiro.x;
		m->passageiros[m->nPass].xPretendido = passageiro.xPretendido;
		m->passageiros[m->nPass].y = passageiro.y;
		m->passageiros[m->nPass].yPretendido = passageiro.yPretendido;
		_tcscpy_s((m->passageiros + m->nPass)->nome, sizeof(passageiro.nome) / sizeof(TCHAR), passageiro.nome);
		m->nPass++;
		if (b.w == LIMITE_PASS)
			b.w = 0;
		b.passageiros[b.w] = passageiro;
		
		StructThread s;

		s.pass = passageiro;
		s.nSegundos = m->nSegundos;
		s.inte = st;
		s.bc = bc;
		s.passageiros = m->passageiros;
		s.nPass = m->nPass;
		s.maxTaxis = m->maxTaxis;
		s.nSemaforo = b.w;

		ReleaseMutex(hMutex);
		hThreads[b.w] = CreateThread(NULL, 0, atribuiTaxiPassageiro, &s, 0, &dwId);

		if (hThreads[b.w] == NULL)
		{
			_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
			return -1;
		}
		SetEvent(hEvento[b.w]);

		b.w = (++b.w) % (LIMITE_PASS + 1);
		_tprintf(TEXT("\n\tNovo passageiro: %s\n\tComando:"), passageiro.nome);

		_tcscpy_s(passageiro.resposta, sizeof(passageiro.resposta) / sizeof(TCHAR), TEXT("Aguarde por um interessado"));
		if (!WriteFile(hPipe, &passageiro, sizeof(Passageiro), &dwWrite, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
			exit(EXIT_FAILURE);
		}
		ReleaseMutex(hMutexBufferCircular);
		DisconnectNamedPipe(hPipe);
	}

	WaitForMultipleObjects(LIMITE_PASS, hThreads, TRUE, INFINITE);
	UnmapViewOfFile(bc);
	CloseHandle(hMapFile);
	CloseHandle(hMutex);
	CloseHandle(hMutexBufferCircular);
}

DWORD WINAPI atribuiTaxiPassageiro(LPVOID lpParam) {
	StructThread* st = (StructThread*)lpParam;
	StructThread s;
	HANDLE hWaitableTimer, hLib, hPipe, hMutexBufferCircular, hMutex, hEvent, hPipeResposta;
	LARGE_INTEGER liDueTime;
	Taxi* taxis=NULL;
	DWORD dwWriten;
	int contador = 0, pos;
	TCHAR buffer[BUFFSIZE];

	s.pass = st->pass;
	s.nSemaforo = st->nSemaforo;

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");


	_itot_s(s.nSemaforo, buffer, sizeof(buffer) / sizeof(TCHAR), 2);
	hEvent = CreateEvent(NULL, TRUE, TRUE, buffer);
	if (hEvent == NULL) {
		_tprintf(TEXT("Erro ao abrir semáforo: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	
	hMutexBufferCircular = CreateMutex(NULL, FALSE, BUFFERCIRCULAR);

	if (hMutexBufferCircular == NULL) {
		_tprintf(TEXT("Erro ao criar mutex: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}

	hMutex = CreateMutex(NULL, FALSE, CENTAXI);

	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}
	
	hWaitableTimer = CreateWaitableTimer(NULL, TRUE, st->pass.nome);
	if (hWaitableTimer == NULL) {
		printf("Erro ao criar waitable timer (%d)\n", GetLastError());
		return 1;
	}
	dll_registerV(st->pass.nome, 5);

	liDueTime.QuadPart = (WAIT_VELOCIDADE_UM * st->nSegundos);

	if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0)){
		printf("Erro ao fazer setWaitableTimer (%d)\n", GetLastError());
		return 1;
	}
	WaitForSingleObject(hWaitableTimer, INFINITE);
	ResetEvent(hEvent);
	for (int i = 0; i < st->maxTaxis; i++) {
		if (_tcscmp(st->inte[i].nomePassageiro, s.pass.nome) == 0) {
			taxis = realloc(taxis, ((contador++) + 1) * sizeof(Taxi));
			_tcscpy_s(taxis[contador-1].matricula, sizeof(st->inte[i].matricula) / sizeof(TCHAR), st->inte[i].matricula);
			_tcscpy_s(taxis[contador - 1].pipe, sizeof(st->inte[i].pipe) / sizeof(TCHAR), st->inte[i].pipe);
			_tcscpy_s(st->inte[i].matricula, MATRICULA_BUFFER, TEXT(""));
			_tcscpy_s(st->inte[i].nomePassageiro, NOMEPASSAGEIRO, TEXT(""));
			_tcscpy_s(st->inte[i].pipe, PIPESIZE, TEXT(""));
		}
	}

	if (contador != 0) {

		srand((int)time(NULL));
		pos = rand() % (contador);

		_tcscpy_s(s.pass.taxi, sizeof(s.pass.taxi) / sizeof(TCHAR), taxis[pos].matricula);
		for (int i = 0; i < contador; i++) {

			if (!WaitNamedPipe(taxis[i].pipe, NMPWAIT_WAIT_FOREVER)) {
				_tprintf(TEXT("[ERRO] Ligar ao pipe %s!\n"), taxis[i].pipe);
				exit(EXIT_FAILURE);
			}
			hPipe = CreateFile(taxis[i].pipe, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hPipe == NULL) {
				_tprintf(TEXT("[ERRO] Ligar ao pipe: %s!\n"), taxis[i].pipe);
				exit(EXIT_FAILURE);
			}

			if (!WriteFile(hPipe, &s.pass, sizeof(Passageiro), &dwWriten, NULL)) {
				_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
				exit(EXIT_FAILURE);
			}
		}


		if (!WaitNamedPipe(PIPENAME_RESPOSTA, NMPWAIT_WAIT_FOREVER)) {
			_tprintf(TEXT("[ERRO] Ligar ao pipe %s!\n"), PIPENAME_RESPOSTA);
			exit(EXIT_FAILURE);
		}
		hPipeResposta = CreateFile(PIPENAME_RESPOSTA, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hPipeResposta == NULL) {
			_tprintf(TEXT("[ERRO] Ligar ao pipe: %s!\n"), PIPENAME_RESPOSTA);
			exit(EXIT_FAILURE);
		}
		_tcscpy_s(s.pass.taxi, sizeof(s.pass.taxi) / sizeof(TCHAR), taxis[pos].matricula);
		if (!WriteFile(hPipeResposta, &s.pass, sizeof(Passageiro), &dwWriten, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
			exit(EXIT_FAILURE);
		}

		WaitForSingleObject(hMutexBufferCircular, INFINITE);
		st->bc->r = (++st->bc->r) % (LIMITE_PASS);
		ReleaseMutex(hMutexBufferCircular);
		WaitForSingleObject(hMutex, INFINITE);
		for (int i = 0; i < st->nPass; i++) {
			if (_tcscmp(st->passageiros[i].nome, s.pass.nome) == 0) {
				while (i < st->nPass - 1) {
					st->passageiros[i].estado = st->passageiros[i + 1].estado;
					st->passageiros[i].x = st->passageiros[i + 1].x;
					st->passageiros[i].y = st->passageiros[i + 1].y;
					st->passageiros[i].xPretendido = st->passageiros[i + 1].xPretendido;
					st->passageiros[i].xPretendido = st->passageiros[i + 1].yPretendido;
					_tcscpy_s(st->passageiros[i].nome, sizeof(st->passageiros[i].nome) / sizeof(TCHAR), st->passageiros[i + 1].nome);
					_tcscpy_s(st->passageiros[i].resposta, sizeof(st->passageiros[i].resposta) / sizeof(TCHAR), st->passageiros[i + 1].resposta);
					i++;
				}
				st->nPass--;
			}
		}
		ReleaseMutex(hMutex);
	}
	else {

		if (!WaitNamedPipe(PIPENAME_RESPOSTA, NMPWAIT_WAIT_FOREVER)) {
			_tprintf(TEXT("[ERRO] Ligar ao pipe %s!\n"), PIPENAME_RESPOSTA);
			exit(EXIT_FAILURE);
		}
		hPipeResposta = CreateFile(PIPENAME_RESPOSTA, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hPipeResposta == NULL) {
			_tprintf(TEXT("[ERRO] Ligar ao pipe: %s!\n"), PIPENAME_RESPOSTA);
			exit(EXIT_FAILURE);
		}
		_tcscpy_s(s.pass.taxi, sizeof(s.pass.taxi) / sizeof(TCHAR), TEXT(""));
		if (!WriteFile(hPipeResposta, &s.pass, sizeof(Passageiro), &dwWriten, NULL)) {
			_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
			exit(EXIT_FAILURE);
		}
	}
	CloseHandle(hMutexBufferCircular);
	CloseHandle(hMutex);
}

DWORD WINAPI threadComandos(LPVOID lpParam) {
	int i, op;
	Centaxi* m = (Centaxi*)lpParam;

	limpaEcra();
	do {
		_tprintf(TEXT("\tCentral de taxis.\n\n"));
		mostraComandos();
		_tprintf(TEXT("\n\n\tComando: "));
		_tscanf_s(TEXT("%d"), &op);
		op = trataComando(op, m);
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

	hSemEsc = CreateSemaphore(NULL, 1, 1, SEM_NOVO_TAXI_ESC);
	if (hSemEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hMapFile);

		return -1;
	}
	dll_registerV(SEM_NOVO_TAXI_ESC, 3);

	hSemLei = CreateSemaphore(NULL, 0, 1, SEM_NOVO_TAXI_LEI);
	if (hSemLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hMapFile);
		
		return -1;
	}
	dll_registerV(SEM_NOVO_TAXI_LEI, 3);

	hSemRes = CreateSemaphore(NULL, 0, 1, SEM_NOVO_TAXI_RES);
	if (hSemRes == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de resposta (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hMapFile);

		return -1;
	}

	dll_registerV(SEM_NOVO_TAXI_RES, 3);

	hSemAtualizaEsc = CreateSemaphore(NULL, 1, 1, SEM_MAPA_ATUALIZAR_ESC); 
	if (hSemAtualizaEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hSemRes);
		CloseHandle(hMapFile);

		return -1;
	}
	dll_registerV(SEM_MAPA_ATUALIZAR_ESC, 3);

	hSemAtualizaLei = CreateSemaphore(NULL, 0, 1, SEM_MAPA_ATUALIZAR_LEI);
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
	dll_registerV(SEM_MAPA_ATUALIZAR_LEI, 3);

	hMutex= CreateMutex(NULL, FALSE, CENTAXI);
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
	dll_registerV(CENTAXI, 1);

	
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
			_tcscpy_s(aux.pipe, sizeof(aux.pipe) / sizeof(TCHAR), t->pipe);
			WaitForSingleObject(hMutex, INFINITE);
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
					aux.maxTaxis = m->maxTaxis;
				}
			}
			CopyMemory(t, &aux, sizeof(Taxi));
			ReleaseSemaphore(hSemRes, 1, NULL);
			if (aux.aceite) {
				WaitForSingleObject(hSemAtualizaEsc, INFINITE);
				m->taxis[m->nTaxis].id = aux.id;
				m->taxis[m->nTaxis].x = aux.x;
				m->taxis[m->nTaxis].y = aux.y;
				m->taxis[m->nTaxis].velocidade = aux.velocidade;
				_tcscpy_s(m->taxis[m->nTaxis].matricula, sizeof(m->taxis[m->nTaxis].matricula) / sizeof(TCHAR), aux.matricula);
				_tcscpy_s(m->taxis[m->nTaxis].pipe, sizeof(m->taxis[m->nTaxis].pipe) / sizeof(TCHAR), aux.pipe);
				m->nTaxis++;
				_tprintf(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), m->taxis[m->nTaxis - 1].matricula, m->taxis[m->nTaxis - 1].id);
				dll_logV(TEXT("\n\tNovo taxi! Matricula: %s. ID: %d"), m->taxis[m->nTaxis - 1].matricula, m->taxis[m->nTaxis - 1].id);
				_tprintf(TEXT("\n\tComando: "));
				ReleaseSemaphore(hSemAtualizaLei, 1, NULL);
			}
			ReleaseMutex(hMutex);
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
	CloseHandle(hMutex);
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

	hMutex = CreateMutex(NULL, FALSE, CENTAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		return -1;
	}
	hSemAtualizaEsc = CreateSemaphore(NULL, 1, 1, SEM_MAPA_ATUALIZAR_ESC);
	if (hSemAtualizaEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		CloseHandle(hMutex);

		return -1;
	}

	hSemAtualizaLei = CreateSemaphore(NULL, 0, 1, SEM_MAPA_ATUALIZAR_LEI);
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
				WaitForSingleObject(hSemAtualizaEsc, INFINITE);
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
				ReleaseSemaphore(hSemAtualizaLei, 1, NULL);
				break;
			}
		}
		ReleaseMutex(hMutex);
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hMutex);
}