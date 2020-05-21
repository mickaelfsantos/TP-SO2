#include ".\..\Dll\dll.h"

void carregaMapa(Contaxi* c);
DWORD WINAPI threadEncerra(LPVOID lpParam);
DWORD WINAPI threadInformacao(LPVOID lpParam);
DWORD WINAPI threadMovimentacao(LPVOID lpParam);
void movimentaCarro(Contaxi * c);


void moveEsquerda(Contaxi* c, float y);
void moveDireita(Contaxi* c, float y);
void moveBaixo(Contaxi* c, float x);
void moveCima(Contaxi* c, float x);
typedef int(__cdecl* dll2_comunicaSaida)(Taxi taxi);
typedef Contaxi(__cdecl* dll2_carregaMapa)(Contaxi c);