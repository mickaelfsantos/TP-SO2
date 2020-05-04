#include "./../CenTaxi/Header.h"

DWORD WINAPI threadCom(LPVOID lpParam);

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThread;

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

	HANDLE hMapFile, hEvent, hMutex;
	Taxi* sM;
	Taxi sharedMsg;
	TCHAR buff[12];

	_tprintf(TEXT("Bem-vindo. Introduza a sua matricula: "));
	_fgetts(buff, 12, stdin);

	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MEMPAR_NOVO_TAXI);

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return 1;
	}

	sM = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Taxi));

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

	CopyMemory(&sharedMsg, sM, sizeof(Taxi)); //mete em sharedMsg o valor que está na memória partilhada em sd
	sharedMsg.nProx++;
	_tcscpy_s(sharedMsg.matricula, sizeof(sharedMsg.matricula), buff);
	CopyMemory(sM, &sharedMsg, sizeof(Taxi));	//atualiza o valor, metendo-o em sd novamente
	
	ReleaseMutex(hMutex); 
	
	hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_NOVO_TAXI);
	SetEvent(hEvent);
	
	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
}