// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include ".\..\CenTaxi\Header.h"
#include <windows.h>

#define EOF (-1)

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

	__declspec(dllexport) int __cdecl comunica(Taxi taxi)
	{
		HANDLE hMapFile, hEvent, hSemLei, hSemEsc, hSemRes;
		Taxi* sM;
		Taxi sharedMsg;

		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MEMPAR_NOVO_TAXI);

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return -1;
		}

		sM = (Taxi*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

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

		WaitForSingleObject(hSemEsc, INFINITE);

		//CopyMemory(&sharedMsg, sM, sizeof(Taxi)); //mete em sharedMsg o valor que está na memória partilhada em sd
		sharedMsg.id = taxi.id;
		_tcscpy_s(sharedMsg.matricula, sizeof(sharedMsg.matricula) / sizeof(TCHAR), taxi.matricula);
		CopyMemory(sM, &sharedMsg, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente
		ReleaseSemaphore(hSemLei, 1, NULL);

		hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_NOVO_TAXI);
		SetEvent(hEvent);

		WaitForSingleObject(hSemRes, INFINITE);
		CopyMemory(&sharedMsg, sM, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente
		ReleaseSemaphore(hSemRes, 1, NULL);

		CloseHandle(hSemEsc);
		CloseHandle(hSemLei);
		CloseHandle(hSemRes);

		UnmapViewOfFile(sM);

		CloseHandle(hMapFile);
		CloseHandle(hEvent);
		if (sharedMsg.aceite == 1) {
			return 1;
		}
		return 0;
	}

#ifdef __cplusplus
}
#endif