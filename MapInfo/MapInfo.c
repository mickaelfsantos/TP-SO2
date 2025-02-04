#include "MapInfo.h"

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	Mapinfo m;
	HANDLE hThreadTaxis, hThreadEncerra;

	m = obtemDados();
	m = carregaMapa(m);
	
	hThreadEncerra = CreateThread(NULL, 0, threadEncerra, &m, 0, NULL);
	hThreadTaxis = CreateThread(NULL, 0, atualizaTaxis, &m, 0, NULL);

	WaitForSingleObject(hThreadTaxis, INFINITE);
	return 0;
}

DWORD WINAPI atualizaTaxis(LPVOID lpParam) {
	Mapinfo* m = (Mapinfo*)lpParam;
	HANDLE hSemLei, hSemEsc, hMapFile;
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

	hSemEsc = CreateSemaphore(NULL, 1, 1, SEM_MAPA_ATUALIZAR_ESC);
	if (hSemEsc == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hMapFile);

		return -1;
	}

	hSemLei = CreateSemaphore(NULL, 0, 1, SEM_MAPA_ATUALIZAR_LEI);
	if (hSemLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());

		UnmapViewOfFile(t);
		CloseHandle(hSemEsc);
		CloseHandle(hMapFile);

		return -1;
	}
	int c = 0;

	while (m->sair!= 1) {
		WaitForSingleObject(hSemLei, INFINITE);
		if (m->sair == 1)
			break;
		m->taxis= t;
		ReleaseSemaphore(hSemEsc, 1, NULL);
		system("cls");
		for (int i = 0; i < m->maxTaxis; i++) {
			if (m->taxis[i].id > 0) {
				*(m->mapa + m->taxis[i].x * m->larguraMapa + m->taxis[i].y) = 2;
				//m->mapaB[m->taxis[i].x][m->taxis[i].y] = 2;
			}
		}
		for (int i = 0; i < m->alturaMapa; i++) {
			for (int j = 0; j < m->larguraMapa; j++) {
				if(*(m->mapa + i * m->larguraMapa + j) == 1){
				//if (m->mapaB[i][j] == 1) {
					_tprintf(TEXT("_"));
				}
				else if(*(m->mapa + i*m->larguraMapa + j) == 2){
				//else if (m->mapaB[i][j] == 2) {
					_tprintf(TEXT("C"));
				}
				else {
					_tprintf(TEXT("X"));
				}
			}
			_tprintf(TEXT("\n"));
		}
		for (int i = 0; i < m->maxTaxis; i++) {
			if (m->taxis[i].id > 0) {
				*(m->mapa + m->taxis[i].x * m->larguraMapa + m->taxis[i].y) = 1;
				//m->mapaB[m->taxis[i].x][m->taxis[i].y] = 1;
			}
		}
	}

	CloseHandle(hSemEsc);
	CloseHandle(hMapFile);
	CloseHandle(hSemLei);
}

Mapinfo obtemDados() {
	
	HANDLE hMapFile, hSem;
	Mapinfo* mapa;
	Mapinfo m;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Mapinfo), MEMPAR_INFORMA_MAPA);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return;
	}

	mapa = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Mapinfo));

	if (mapa == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return;
	}

	hSem = CreateSemaphore(NULL, 0, 1, SEM_PODE_FECHAR_INFORMA_MAPA);
	if (hSem == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo (%d).\n"), GetLastError());

		UnmapViewOfFile(mapa);
		CloseHandle(hMapFile);

		return;
	}

	CopyMemory(&m, mapa, sizeof(Mapinfo));
	ReleaseSemaphore(hSem, 1, NULL);

	UnmapViewOfFile(mapa);
	CloseHandle(hMapFile);
	CloseHandle(hSem);
	return m;

}

Mapinfo carregaMapa(Mapinfo m) {
	
	HANDLE hMapFile;
	int* mapa;

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int) * m.alturaMapa * m.larguraMapa, MEMPAR_MAPA);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return m;
	}

	mapa = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * m.alturaMapa * m.larguraMapa);

	if (mapa == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return m;
	}

	m.mapa = mapa;
	return m;
}

DWORD WINAPI threadEncerra(LPVOID lpParam) {
	HANDLE hEvent, hSemLei;
	Mapinfo* m = (Mapinfo*)lpParam;

	hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_ENCERRA_TUDO);
	WaitForSingleObject(hEvent, INFINITE);
	ResetEvent(hEvent);
	m->sair = 1;

	hSemLei = CreateSemaphore(NULL, 0, 1, SEM_MAPA_ATUALIZAR_LEI);
	if (hSemLei == NULL) {
		_tprintf(TEXT("Erro ao criar semaforo de atualização (%d).\n"), GetLastError());
		return;
	}
	ReleaseSemaphore(hSemLei, 1, NULL);
}