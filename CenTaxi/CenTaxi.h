#include "Header.h"

DWORD WINAPI threadCom(LPVOID lpParam);
void mostraComandos();
int trataComando(TCHAR comando[]);
void encerraTudo();
int sair(Mapa* m); 
void listaTaxis();
void informaMapaAoTaxi(Mapa* mapa);
void limpaEcra();
DWORD WINAPI threadComandos(LPVOID lpParam);
void carregaMapa(Mapa* m);
void mostraMapa(Mapa* m);