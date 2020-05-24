#include ".\..\Dll\dll.h"
#define MUTEX_ALTERA_TAXI TEXT("mutexAlteraTaxi")
#define WAITABLETIMER TEXT("WAITABLETIMER")


void carregaMapa(Contaxi* c);
DWORD WINAPI threadEncerra(LPVOID lpParam);
DWORD WINAPI threadInformacao(LPVOID lpParam);
DWORD WINAPI threadMovimentacao(LPVOID lpParam);
DWORD WINAPI threadComandosTaxi(LPVOID lpParam);
void movimentaCarro(Contaxi * c);
void mostraComandos();
void sair(Contaxi* c);

void acelerar(Contaxi* c);
void desacelerar(Contaxi* c);
void limpaEcra();
int trataComando(TCHAR comando[], Contaxi* c);
void moveEsquerda(Contaxi* c, float y);
void moveDireita(Contaxi* c, float y);
void moveBaixo(Contaxi* c, float x);
void moveCima(Contaxi* c, float x);
typedef int(__cdecl* dll2_comunicaSaida)(Taxi taxi);
typedef Contaxi(__cdecl* dll2_carregaMapa)(Contaxi c);