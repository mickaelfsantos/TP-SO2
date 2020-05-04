#include "Header.h"

DWORD WINAPI threadCom(LPVOID lpParam);


int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread, hEventThread;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThread = CreateThread(NULL, 0, threadCom, NULL, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	return 0;
}

DWORD WINAPI threadCom(LPVOID lpParam) {


	HANDLE hMapFile, hEvent;
	SharedMSG sharedMsg;
	sharedMsg.nProx = 0;
	sharedMsg.matricula[0] = '\0';
	SharedMSG* sM = &sharedMsg;
	TCHAR buff[256];

	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SharedMSG), TEXT("novoTaxi"));

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return -1;
	}

	sM = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMSG));

	if (sM == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return -1;
	}

	CopyMemory(sM, &sharedMsg, sizeof(SharedMSG));

	while (1) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, TEXT("novoTaxiEvento"));
		WaitForSingleObject(hEvent, INFINITE);
		CopyMemory(&sharedMsg, sM, sizeof(SharedMSG)); //mete em sdata o valor que está na memória partilhada em sd
		_tcscpy_s(buff, sizeof(buff), sharedMsg.matricula);
		_tprintf(TEXT("Novo taxi: %s\n"), buff);
	}

	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
}
