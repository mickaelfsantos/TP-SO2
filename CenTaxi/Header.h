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

typedef void(_cdecl* dll_log)(TCHAR* text);
typedef void(_cdecl* dll_register)(TCHAR* text);
typedef int(__cdecl* dll2_comunica)(Taxi taxi);

typedef struct {
	int estrada; //0 - edificio 1 - estrada
	//Passageiro passageiro;
	Taxi taxi;
}Cell;

typedef struct {
	Cell cell[50][50];
}Mapa;

typedef struct {
	int aceita; //0 - nao aceita 1 - aceita
	int sair;
	int nTaxis;
	Taxi taxis[LIMITE_TAXIS];
}Shared;


