#include ".\..\Dll\dll.h"

typedef struct {
	int larguraMapa;
	int alturaMapa;
	int maxTaxis;
	int maxPass;
	int nTaxis;
	int nPass;
	int aceitaTaxis;
	int* mapa; //0 edificio 1 estada
	Taxi* taxis;
	Passageiro* passageiros;
	int sair;
	Interessados* interessados;
	int nSegundos;
	int nQuadriculas;
}Centaxi;

typedef struct {
	Passageiro pass;
	BufferCircular* bc;
	Centaxi* centaxi;
	int nSemaforo;
}StructThread;

void mostraComandos();
void atuaAceitacao(Centaxi* m);
void mostraMapa(Centaxi* m);
int trataComando(int op, Centaxi* m);
int sair(Centaxi* m);
void listaTaxis(Centaxi* m);
void limpaEcra();
DWORD WINAPI informaMapa(LPVOID lpParam);
DWORD WINAPI carregaMapa(LPVOID lpParam);
DWORD WINAPI threadCriaTaxis(LPVOID lpParam);
DWORD WINAPI threadPassageiros(LPVOID lpParam);
DWORD WINAPI threadComandos(LPVOID lpParam);
DWORD WINAPI threadComunicaTaxis(LPVOID lpParam);
DWORD WINAPI threadSaiTaxi(LPVOID lpParam);
DWORD WINAPI atribuiTaxiPassageiro(LPVOID lpParam);
DWORD WINAPI threadCriaPassageiros(LPVOID lpParam);
DWORD WINAPI threadSaiPassageiro(LPVOID lpParam);