#include "Header.h"

DWORD WINAPI threadCom(LPVOID lpParam);


int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread, hEventThread;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	_tprintf(TEXT("Central de taxis.\n\n"));
	hThread = CreateThread(NULL, 0, threadCom, NULL, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	return 0;
}

DWORD WINAPI threadCom(LPVOID lpParam) {

	HANDLE hMapFile, hEvent, hMutex;
	Taxi taxi;
	taxi.nProx = 0;
	taxi.matricula[0] = '\0';
	Taxi* sM = &taxi;
	TCHAR buff[256];

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

	hMutex = CreateMutex(NULL, FALSE, MUTEX_NOVO_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("Erro ao criar mutex (%d).\n"), GetLastError());

		UnmapViewOfFile(sM);
		CloseHandle(hMapFile);
		
		return -1;
	}

	CopyMemory(sM, &sharedMsg, sizeof(Taxi));

	while (1) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, EVENTO_NOVO_TAXI);
		WaitForSingleObject(hEvent, INFINITE);

		WaitForSingleObject(hMutex, INFINITE);
		CopyMemory(&sharedMsg, sM, sizeof(Taxi)); //mete em sdata o valor que está na memória partilhada em sd
		_tcscpy_s(buff, sizeof(buff), sharedMsg.matricula);
		ReleaseMutex(hMutex);
		_tprintf(TEXT("Novo taxi: %s\n"), buff);
		
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
	CloseHandle(hMutex);
}
