#include ".\..\Dll\dll.h"

typedef struct {
	int larguraMapa;
	int alturaMapa;
	int maxTaxis;
	int maxPass;
	int nTaxis;
	int* mapa; //0 edificio 1 estada
	Taxi* taxis;
	int sair;
}Centaxi;


DWORD WINAPI threadCriaTaxis(LPVOID lpParam);

void mostraComandos();

DWORD WINAPI carregaMapa(LPVOID lpParam);

void mostraCentaxi(Centaxi* m);

int trataComando(TCHAR comando[], Centaxi* m);

int sair(Centaxi* m);

void encerraTudo();

void listaTaxis(Centaxi* m);

void limpaEcra();

DWORD WINAPI threadComandos(LPVOID lpParam);

DWORD WINAPI threadComunicaTaxis(LPVOID lpParam);

DWORD WINAPI threadSaiTaxi(LPVOID lpParam);

void informaCentaxiAoTaxi(Centaxi* Centaxi);

DWORD WINAPI informaMapa(LPVOID lpParam);