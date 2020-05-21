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

		hSemEsc = CreateSemaphore(NULL, 1, 1, MUTEX_NOVO_TAXI_ESC);
		if (hSemEsc == NULL) {
			_tprintf(TEXT("Erro ao criar semaforo de escrita (%d).\n"), GetLastError());

			UnmapViewOfFile(t);
			CloseHandle(hMapFile);

			return taxi;
		}

		hSemLei = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_LEI);
		if (hSemLei == NULL) {
			_tprintf(TEXT("Erro ao criar semaforo de leitura (%d).\n"), GetLastError());

			UnmapViewOfFile(t);
			CloseHandle(hMapFile);
			CloseHandle(hSemEsc);

			return taxi;
		}

		hSemRes = CreateSemaphore(NULL, 0, 1, MUTEX_NOVO_TAXI_RES);
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

	/*
	__declspec(dllexport) void __cdecl comunicaAlteracao(Taxi taxi)
	{
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
	}
	*/
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

	/*
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

		hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_TAXI_SAI);

		if (hMutex == NULL) {
			_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());

			UnmapViewOfFile(sM);
			CloseHandle(hMapFile);
			return -1;
		}

		WaitForSingleObject(hMutex, INFINITE);
		CopyMemory(sM, &taxi, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente
		ReleaseMutex(hMutex);

		hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_SAI_TAXI);
		SetEvent(hEvent);

		UnmapViewOfFile(sM);

		CloseHandle(hMapFile);
		CloseHandle(hEvent);
		return 0;
	}
	*/
#ifdef __cplusplus
}
#endif