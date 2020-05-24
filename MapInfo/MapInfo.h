#include ".\..\Dll\dll.h"

Mapinfo obtemDados();
DWORD WINAPI atualizaTaxis(LPVOID lpParam);
Mapinfo carregaMapa(Mapinfo m);
DWORD WINAPI threadEncerra(LPVOID lpParam);