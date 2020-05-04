#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define EVENTO_NOVO_TAXI TEXT("novoTaxiEvento")
#define MEMPAR_NOVO_TAXI TEXT("novoTaxiMemPar")
#define MUTEX_NOVO_TAXI TEXT("novoTaxiMutex")

typedef struct {
	int nProx;
	TCHAR matricula[12];
}Taxi;
