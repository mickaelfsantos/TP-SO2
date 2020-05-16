#include "Header.h"

DWORD WINAPI threadCom(LPVOID lpParam);
void mostraComandos();
int trataComando(TCHAR comando[]);
void encerraTudo();
int sair(Shared* sh); 
void listaTaxis();
void limpaEcra();
DWORD WINAPI threadComandos(LPVOID lpParam);
void carregaMapa(Mapa* m);
void mostraMapa(Mapa* m);