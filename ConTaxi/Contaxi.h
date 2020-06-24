#include ".\..\Dll\dll.h"
#define WAITABLETIMER TEXT("WAITABLETIMER")


void carregaMapa(Contaxi* c);
DWORD WINAPI threadEncerra(LPVOID lpParam);
DWORD WINAPI threadInformacao(LPVOID lpParam);
DWORD WINAPI threadMovimentacao(LPVOID lpParam);
DWORD WINAPI threadComandosTaxi(LPVOID lpParam);
DWORD WINAPI threadPassageiros(LPVOID lpParam);
DWORD WINAPI threadRespostasCentaxi(LPVOID lpParam);
DWORD WINAPI threadEsperaPassageiro(LPVOID lpParam);
void movimentaCarro(Contaxi * c);
void mostraComandos();
void sair(Contaxi* c);
void alterarNq(Contaxi* c);

void acelerar(Contaxi* c);
void movimenta(Contaxi* c);
int posValida2(int* mat, int x, int y, int altura, int largura);
void desacelerar(Contaxi* c);
void limpaEcra();
int trataComando(TCHAR comando[], Contaxi* c);
void moveEsquerda(Contaxi* c, float y);
void moveDireita(Contaxi* c, float y);
DWORD WINAPI threadSair(LPVOID lpParam);
void moveBaixo(Contaxi* c, float x);
void moveCima(Contaxi* c, float x);

void encontraCaminho(int* mat, int* visited, int x, int y,
	int xPretendido, int yPretendido, int* min_dist, int dist, int* caminho, int altura, int largura);

int posValida(int* mat, int* visited, int x, int y, int altura, int largura);
typedef int(__cdecl* dll2_comunicaSaida)(Taxi taxi);
typedef Contaxi(__cdecl* dll2_carregaMapa)(Contaxi c);