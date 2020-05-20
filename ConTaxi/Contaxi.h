#include ".\..\Dll\dll.h"

void carregaMapa(Contaxi* c);
DWORD WINAPI threadEncerra(LPVOID lpParam);
DWORD WINAPI threadInformacao(LPVOID lpParam);

typedef int(__cdecl* dll2_comunicaSaida)(Taxi taxi);
typedef Contaxi(__cdecl* dll2_carregaMapa)(Contaxi c);