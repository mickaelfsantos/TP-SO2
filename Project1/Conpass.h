#include ".\..\Dll\dll.h"


void menu();
DWORD WINAPI threadComandos(LPVOID lpParam);
DWORD WINAPI criaPassageiro(LPVOID lpParam);
int trataComando(int op);
Passageiro obtemDados();
DWORD WINAPI threadRecebeRespostas(LPVOID lpParam);