#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>

//vars
#define LIMITE_TAXIS 5
#define LIMITE_PASS 5


#define WAIT_VELOCIDADE_DOIS -5000000LL
#define WAIT_VELOCIDADE_UM -10000000LL
#define WAIT_VELOCIDADE_UM_E_MEIO -6666666LL
#define WAIT_VELOCIDADE_MEIO -20000000LL



#define MEMPAR_MAPA TEXT("MemoriaMapa")
#define MEMPAR_INFORMA_MAPA TEXT("InformaMapa")
#define MEMPAR_TAXIS TEXT("MemoriaTaxis")
#define MEMPAR_TAXI_NOVO TEXT("MemoriaTaxiNovo")
#define MEMPAR_PASS TEXT("MemoriaPassageiros")

#define MUTEX_NOVO_TAXI_LEI TEXT("novoTaxiMutexLei")
#define MUTEX_NOVO_TAXI_RES TEXT("novoTaxiMutexRes")
#define MUTEX_NOVO_TAXI_ESC TEXT("novoTaxiMutexEsc")
#define MUTEX_PODE_ATUALIZAR_ARRAY_ESC TEXT("podeEscrever")
#define MUTEX_PODE_ATUALIZAR_ARRAY_LEI TEXT("podeLer")
#define SEM_PODE_FECHAR_INFORMA_MAPA TEXT("podeFecharInformaMapa")
#define ATUALIZA_ARRAY_TAXIS TEXT("podeAtualizarArrayTaxis")


#define EVENTO_NOVO_TAXI TEXT("novoTaxiEvento")
#define MEMPAR_NOVO_TAXI TEXT("novoTaxiMemPar")
#define MEMPAR_SAI_TAXI TEXT("saiTaxiMemPar")
#define INFORMA_Centaxi TEXT("Centaxi")
#define EVENTO_SAI_TAXI TEXT("saiTaxiEvento")
#define MUTEX_MAPA TEXT("CentaxiMutex")
#define MUTEX_TAXI_SAI TEXT("mutexTaxiSai")
#define MUTEX_TAXI TEXT("novoTaxiMutex")
#define EVENTO_ENCERRA_TUDO TEXT("encerraTudo")
#define MATRICULA_BUFFER 12


//structs
typedef struct {
	int id;
	int x;
	int y;
	int xA;
	int yA;
	int larguraMapa;
	int alturaMapa;
	int aceite; //0-não foi aceite (carro ja existe)  // 1-aceite
	int atualizaMovimentacao; //0-carro novo // 1-atualiza carro
	float velocidade;
	TCHAR matricula[MATRICULA_BUFFER];
}Taxi;

typedef struct {
	int alturaMapa;
	int larguraMapa;
	int* mapa;
	Taxi* taxi;
	int** m;
	int sair;
} Contaxi;

typedef struct {
	int alturaMapa;
	int larguraMapa;
	int maxTaxis;
	int maxPass;
	int sair;
	int* mapa;
	int** mapaB;
	Taxi* taxis;
}Mapinfo;


typedef void(_cdecl* dll_log)(TCHAR* text);
typedef void(_cdecl* dll_register)(TCHAR* text);
typedef Taxi(__cdecl* dll2_comunica)(Taxi taxi);