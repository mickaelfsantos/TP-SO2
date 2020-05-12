#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <wchar.h>

#define EVENTO_NOVO_TAXI TEXT("novoTaxiEvento")
#define MEMPAR_NOVO_TAXI TEXT("novoTaxiMemPar")
#define MUTEX_NOVO_TAXI TEXT("novoTaxiMutex")
#define EVENTO_ENCERRA_TUDO TEXT("encerraTudo")
#define MATRICULA_BUFFER 12
#define LIMITE_TAXIS 5

typedef struct {
	int id;
	int x;
	int y;
	TCHAR matricula[MATRICULA_BUFFER];
}Taxi;

typedef struct {
	int sair;
	int nTaxis;
	Taxi taxis[LIMITE_TAXIS];
}Shared;
