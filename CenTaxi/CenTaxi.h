#include "Header.h"


void mostraComandos();

void carregaMapa(Mapa* m);

void mostraMapa(Mapa* m);

int trataComando(TCHAR comando[], Mapa* m);

int sair(Mapa* m);

void encerraTudo();

void listaTaxis(Mapa* m);

void limpaEcra();

DWORD WINAPI threadComandos(LPVOID lpParam);

DWORD WINAPI threadCom(LPVOID lpParam);

DWORD WINAPI threadSaiTaxi(LPVOID lpParam);

void informaMapaAoTaxi(Mapa* mapa);