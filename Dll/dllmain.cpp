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
		HANDLE hMapFile, hEvent, hMutex;
		Taxi* sM;
		Taxi sharedMsg;

		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MEMPAR_NOVO_TAXI);

		if (hMapFile == NULL)
		{
			_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
			return 1;
		}

		sM = (Taxi*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

		if (sM == NULL)
		{
			_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

			CloseHandle(hMapFile);
			return 1;
		}

		hMutex = OpenMutex(SYNCHRONIZE, FALSE, MUTEX_NOVO_TAXI);

		if (hMutex == NULL) {
			_tprintf(TEXT("Erro ao abrir mutex (%d).\n"), GetLastError());

			UnmapViewOfFile(sM);
			CloseHandle(hMapFile);
			return -1;
		}

		WaitForSingleObject(hMutex, INFINITE);

		//CopyMemory(&sharedMsg, sM, sizeof(Taxi)); //mete em sharedMsg o valor que está na memória partilhada em sd
		sharedMsg.id = taxi.id;
		_tcscpy_s(sharedMsg.matricula, sizeof(sharedMsg.matricula) / sizeof(TCHAR), taxi.matricula);
		CopyMemory(sM, &sharedMsg, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente

		ReleaseMutex(hMutex);

		hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_NOVO_TAXI);
		SetEvent(hEvent);

		UnmapViewOfFile(sM);

		CloseHandle(hMapFile);
		CloseHandle(hEvent);
		return 0;
	}

#ifdef __cplusplus
}
#endif