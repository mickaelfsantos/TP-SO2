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

	HANDLE hMapFile, hEvent;
	SharedMSG* sM;
	SharedMSG sharedMsg;
	TCHAR buff[12];

	_tprintf(TEXT("Ola. Introduza a sua matricula: "));
	_fgetts(buff, 12, stdin);

	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, TEXT("novoTaxi"));

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Erro ao fazer CreateFileMapping (%d).\n"), GetLastError());
		return 1;
	}

	sM = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedMSG));

	if (sM == NULL)
	{
		_tprintf(TEXT("Erro ao fazer MapViewOfFile (%d).\n"), GetLastError());

		CloseHandle(hMapFile);
		return 1;
	}

	CopyMemory(&sharedMsg, sM, sizeof(SharedMSG)); //mete em sharedMsg o valor que está na memória partilhada em sd
	sharedMsg.nProx++;
	_tcscpy_s(sharedMsg.matricula, sizeof(sharedMsg.matricula), buff);
	CopyMemory(sM, &sharedMsg, sizeof(SharedMSG));	//atualiza o valor, metendo-o em sd novamente
	
	hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("novoTaxiEvento"));
	SetEvent(hEvent);
	
	UnmapViewOfFile(sM);

	CloseHandle(hMapFile);
	CloseHandle(hEvent);
}