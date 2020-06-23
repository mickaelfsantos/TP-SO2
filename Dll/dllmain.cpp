// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "dll.h"

#define EOF (-1)

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif
	
	__declspec(dllexport) Taxi __cdecl comunica(Taxi taxi)
	{
		HANDLE hMapFile, hSemLei, hSemEsc, hSemRes;
		Taxi* t;

		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MEMPAR_TAXI_NOVO);

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return taxi;
		}

		t = (Taxi*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

		if (t == NULL)
		{
			_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

			CloseHandle(hMapFile);
			return taxi;
		}

		hSemEsc = CreateSemaphore(NULL, 1, 1, SEM_NOVO_TAXI_ESC);
		if (hSemEsc == NULL) {
			_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

			UnmapViewOfFile(t);
			CloseHandle(hMapFile);

			return taxi;
		}

		hSemLei = CreateSemaphore(NULL, 0, 1, SEM_NOVO_TAXI_LEI);
		if (hSemLei == NULL) {
			_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());

			UnmapViewOfFile(t);
			CloseHandle(hMapFile);
			CloseHandle(hSemEsc);

			return taxi;
		}

		hSemRes = CreateSemaphore(NULL, 0, 1, SEM_NOVO_TAXI_RES);
		if (hSemRes == NULL) {
			_tprintf(TEXT("Erro ao criar semaforo de resposta (%d).\n"), GetLastError());

			UnmapViewOfFile(t);
			CloseHandle(hMapFile);
			CloseHandle(hSemEsc);
			CloseHandle(hSemLei);

			return taxi;
		}

		WaitForSingleObject(hSemEsc, INFINITE);
		CopyMemory(t, &taxi, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente
		ReleaseSemaphore(hSemLei, 1, NULL);

		WaitForSingleObject(hSemRes, INFINITE);
		CopyMemory(&taxi, t, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente
		UnmapViewOfFile(t);
		CloseHandle(hMapFile);
		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hSemRes);
		return taxi;
	}

	__declspec(dllexport) Contaxi __cdecl carregaMapa(Contaxi c) {

		HANDLE hMapFile;
		int* mapa;

		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int) * c.alturaMapa * c.larguraMapa, MEMPAR_MAPA);

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return c;
		}

		mapa = (int *)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * c.alturaMapa * c.larguraMapa);

		if (mapa == NULL)
		{
			_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

			CloseHandle(hMapFile);
			return c;
		}

		c.mapa = mapa;
		return c;
	}

	__declspec(dllexport) int __cdecl comunicaSaida(Taxi taxi) {
		HANDLE hMapFile, hEvent, hMutex;
		Taxi* sM;

		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MEMPAR_SAI_TAXI);

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return 1;
		}

		sM =(Taxi*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

		if (sM == NULL)
		{
			_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

			CloseHandle(hMapFile);
			return 1;
		}

		CopyMemory(sM, &taxi, sizeof(Taxi));

		hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_SAI_TAXI);
		SetEvent(hEvent);

		UnmapViewOfFile(sM);

		CloseHandle(hMapFile);
		CloseHandle(hEvent);
		return 0;
	}

	__declspec(dllexport) int __cdecl threadRespostasCentaxi(Contaxi* c) {
		HANDLE hPipe, hEvent, hMutex;
		Passageiro passageiro;
		DWORD dwRead;

		hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
		if (hMutex == NULL) {
			_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
			return -1;
		}

		hPipe = CreateNamedPipe(c->taxi->pipe, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
			sizeof(Passageiro), sizeof(Passageiro), 0, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Problema ao criar named pipe do servidor: %d\n"), GetLastError());
			return EXIT_FAILURE;
		}

		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(TEXT("\n\tErro ao fazer ConnectNamedPipe: %d\n"), GetLastError());
			exit(-1);
		}

		if (!ReadFile(hPipe, &passageiro, sizeof(Passageiro), &dwRead, NULL)) {
			_tprintf(TEXT("[ERRO] Leitura do named pipe: %d\n"), GetLastError());
			exit(EXIT_FAILURE);
		}

		if (!dwRead) {
			_tprintf(TEXT("[ERRO] Não foram lidos bytes \n"));
			exit(EXIT_FAILURE);
		}
		WaitForSingleObject(hMutex, INFINITE);
		if (_tcscmp(passageiro.taxi, c->taxi->matricula) == 0) {
			_tprintf(TEXT("\n\tFoi-me atribuido o passageiro: %s\n\tComando:"), passageiro.nome);
			c->taxi->temPassageiro = 1;
			c->taxi->passageiro = passageiro;
			hEvent = CreateEvent(NULL, TRUE, FALSE, CHEGOU_PASSAGEIRO);
			SetEvent(hEvent);
		}
		else {
		}
		ReleaseMutex(hMutex);
		DisconnectNamedPipe(hPipe);
		CloseHandle(hPipe);

	}

	__declspec(dllexport) int __cdecl threadPassageiros(Contaxi *c) {
		HANDLE hMapFileInteressados, hEvento[LIMITE_PASS], hMapFile, hThreadRespostas, hMutex;
		Interessados* st;
		BufferCircular* bc;
		TCHAR buffer[BUFFSIZE];
		DWORD sPosicao, dwRead;
		int pos, segundos = 0;
		Passageiro passageiro;
		LARGE_INTEGER liDueTime;


		hMapFileInteressados = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Interessados) * c->maxTaxis, MEMPAR_INT);

		if (hMapFileInteressados == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return -1;
		}

		st = (Interessados*)MapViewOfFile(hMapFileInteressados, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Interessados) * c->maxTaxis);

		if (st == NULL)
		{
			_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

			CloseHandle(hMapFileInteressados);
			return -1;
		}

		for (int i = 0; i < LIMITE_PASS; i++) {
			_itot_s(i, buffer, sizeof(buffer) / sizeof(TCHAR), 2);
			hEvento[i] = CreateEvent(NULL, TRUE, FALSE, buffer);
			if (hEvento[i] == NULL) {
				_tprintf(TEXT("Erro ao criar semáforos: %d\n"), GetLastError());
				return EXIT_FAILURE;
			}
		}

		hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
		if (hMutex == NULL) {
			_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
			return -1;
		}

		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(BufferCircular), MEMPAR_PASS);

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return -1;
		}

		bc = (BufferCircular*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(BufferCircular));

		if (bc == NULL)
		{
			_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

			CloseHandle(hMapFile);
			return -1;
		}

		while (c->sair != 1) {
			sPosicao = WaitForMultipleObjects(c->maxTaxis, hEvento, FALSE, INFINITE);
			WaitForSingleObject(hMutex, INFINITE);
			if (c->taxi->temPassageiro == 1) {
				ReleaseMutex(hMutex);
				continue;
			}

			segundos = st->nSegundos;

			for (int i = 0; i < LIMITE_PASS; i++) {
				if (sPosicao == WAIT_OBJECT_0 + i) {
					pos = i;
					for (int j = 0; j < c->maxTaxis; j++) {
						if (_tcscmp(st[j].matricula, TEXT("")) == 0) {
							if (((bc->passageiros[i].x + c->nq > c->taxi->x) || ((bc->passageiros[i].x - c->nq < c->taxi->x))) &&
								((bc->passageiros[i].y + c->nq > c->taxi->y) || ((bc->passageiros[i].y - c->nq < c->taxi->y)))) {
								_tcscpy_s(st[j].matricula, sizeof(st[j].matricula) / sizeof(TCHAR), c->taxi->matricula);
								_tcscpy_s(st[j].pipe, sizeof(st[j].pipe) / sizeof(TCHAR), c->taxi->pipe);
								_tcscpy_s(st[j].nomePassageiro, sizeof(st[j].nomePassageiro) / sizeof(TCHAR), bc->passageiros[pos].nome);
								threadRespostasCentaxi(c);
								break;
							}
							else {
								break;
							}
							
						}
					}
					break;
				}
			}
			ReleaseMutex(hMutex);
		}
	}

#ifdef __cplusplus
}
#endif