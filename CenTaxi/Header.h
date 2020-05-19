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
#define INFORMA_MAPA TEXT("Mapa")
#define MUTEX_MAPA TEXT("MapaMutex")
#define MUTEX_NOVO_TAXI_LEI TEXT("novoTaxiMutexLei")
#define MUTEX_NOVO_TAXI_RES TEXT("novoTaxiMutexRes")
#define MUTEX_NOVO_TAXI_ESC TEXT("novoTaxiMutexEsc")
#define MUTEX_TAXI TEXT("novoTaxiMutex")
#define EVENTO_ENCERRA_TUDO TEXT("encerraTudo")
#define MATRICULA_BUFFER 12
#define LIMITE_TAXIS 5

typedef struct {
	int id;
	int x;
	int y;
	int aceite; //0-não foi aceite (carro ja existe)  // 1-aceite
	TCHAR matricula[MATRICULA_BUFFER];
}Taxi;

typedef struct {
	int** estrada; //0 - edificio 1 - estrada
	//Passageiro passageiro;
	Taxi taxis[LIMITE_TAXIS];
	int altura;
	int largura;
	int nTaxis;
	int aceita; //0 - não aceita   1 - aceita 
	int sair;
}Mapa;

typedef void(_cdecl* dll_log)(TCHAR* text);
typedef void(_cdecl* dll_register)(TCHAR* text);
typedef int(__cdecl* dll2_comunica)(Taxi taxi, Mapa* mapa);

/*typedef struct {
	int aceita; //0 - nao aceita 1 - aceita
	int sair;
	int nTaxis;
	Taxi taxis[LIMITE_TAXIS];
}Shared;*/