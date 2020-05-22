#include "Contaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCom, hThreadEnc, hThreadInf, hThreadMovimentacao, hWaitableTimer;
	Taxi taxi;
	LARGE_INTEGER liDueTime;
	Contaxi c;
	int i;

	HINSTANCE hLib, hCom;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadEnc = CreateThread(NULL, 0, threadEncerra, NULL, 0, NULL);
	hThreadInf = CreateThread(NULL, 0, threadInformacao, &taxi, 0, NULL);

	hLib = LoadLibrary(TEXT(".\\..\\Debug\\SO2_TP_DLL_64.dll"));
	hCom = LoadLibrary(TEXT("Dll.dll"));

	if (hLib == NULL || hCom == NULL) {
		_tprintf(TEXT("\nErro a fazer LoadLibrary (%d) - Load da dll"), GetLastError());
		return -1;
	}

	dll_log dll_logV = (dll_log)GetProcAddress(hLib, "dll_log");
	dll_register dll_registerV = (dll_register)GetProcAddress(hLib, "dll_register");
	dll2_comunica dll2_comunicaV = (dll2_comunica)GetProcAddress(hCom, "comunica");
	dll2_comunicaSaida dll2_comunicaSV = (dll2_comunicaSaida)GetProcAddress(hCom, "comunicaSaida");
	dll2_carregaMapa dll2_carregaM = (dll2_carregaMapa)GetProcAddress(hCom, "carregaMapa");


	if (dll2_comunicaV == NULL) {
		_tprintf(TEXT("\nErro no ponteiro para a DLL."));
		return 0;
	}
	
	WaitForSingleObject(hThreadInf, INFINITE);
	taxi = dll2_comunicaV(taxi);
	if (taxi.aceite != 1) {
		_tprintf(TEXT("\nNão foi aceite por parte da central"));
		_getch();
		FreeLibrary(hLib);
		FreeLibrary(hCom);
		return 0;
	}

	c.alturaMapa = taxi.alturaMapa;
	c.larguraMapa = taxi.larguraMapa;
	c.mapa = NULL;
	c = dll2_carregaM(c);

	taxi.velocidade = 2;
	taxi.atualizaMovimentacao = 1;
	taxi.xA = taxi.x;
	taxi.yA = taxi.y;

	c.m = (int*)malloc(sizeof(int*) * c.alturaMapa);
	for (int i = 0; i < c.larguraMapa; i++) {
		c.m[i] = malloc(sizeof(int) * c.larguraMapa);
	}

	for (int i = 0, k = 0; i < c.alturaMapa; i++) {
		for (int j = 0; j < c.larguraMapa; j++, k++) {
			c.m[i][j] = c.mapa[k];
		}
	}
	c.taxi = &taxi;

	hWaitableTimer =  CreateWaitableTimer(NULL, TRUE, NULL);
	if (hWaitableTimer ==NULL){
		printf("Erro ao criar waitable timer (%d)\n", GetLastError());
		return 1;
	}

	while (c.sair) {

		liDueTime.QuadPart = (WAIT_VELOCIDADE_UM / taxi.velocidade);
		
		if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			printf("Erro ao fazer setWaitableTimer (%d)\n", GetLastError());
			return 1;
		}
		WaitForSingleObject(hWaitableTimer, INFINITE);
		movimentaCarro(&c);
		taxi.x = c.taxi->x;
		taxi.y = c.taxi->y;
		taxi.velocidade = c.taxi->velocidade;
		taxi.xA = c.taxi->xA;
		taxi.yA = c.taxi->yA;
		dll2_comunicaV(taxi);
		
		
	}

	for (int i = 0; i < c.alturaMapa; i++)
	{
		free(c.m[i]);
	}
	free(c.m);
	
	_getch();
	return 0;
}


void movimentaCarro(Contaxi * c) {

	int direcao, pos = -1, sair, nPossiveis = 0, posAux;
	int aux[4] = { 0,0,0,0 };
	int* arr = NULL;
	int x, y, xA, yA;

	x = c->taxi->x;
	y = c->taxi->y;
	xA = c->taxi->xA;
	yA = c->taxi->yA;

	if (x >= 1 && c->m[x - 1][y] == 1) { //pode subir
		aux[0] = 1;
		nPossiveis++;
	}
	if (y <= c->larguraMapa - 1 && c->m[x][y + 1] == 1) { //pode ir para a direita
		aux[1] = 1;
		nPossiveis++;
	}
	if (y >= 1 && c->m[x][y - 1] == 1) { //pode ir para a esquerda
		aux[2] = 1;
		nPossiveis++;
	}
	if (x <= c->alturaMapa - 1 && c->m[x + 1][y] == 1) { //pode descer
		aux[3] = 1;
		nPossiveis++;
	}

	
	if (xA < x && yA == y) {//vem de cima
		if (nPossiveis > 1) {
			aux[0] = 0;
			nPossiveis--;
		}
	}
	else if (yA < y && xA == x) {//vem da esquerda
		if (nPossiveis > 1) {
			aux[2] = 0;
			nPossiveis--;
		}
	}
	else if (yA > y && xA == x) { //vem da direita
		if (nPossiveis > 1) {
			aux[1] = 0;
			nPossiveis--;
		}
	}
	
	else if (xA > x && yA == y) {//vem de baixo 
		if (nPossiveis > 1) {
			aux[3] = 0;
			nPossiveis--;
		}
	}

	if (nPossiveis > 0) {
		arr = malloc(sizeof(int) * nPossiveis);
		for (int i = 0, j = 0; i < 4; i++) {
			if (aux[i] == 1) {
				arr[j] = i;
				j++;
			}
		}
		srand((int)time(NULL));
		posAux = rand() % nPossiveis;
		pos = arr[posAux];
		free(arr);
	}
	else {
		for (int i = 0; i < 4; i++) {
			if (aux[i] == 2) {
				pos = i;
			}
		}
	}

	switch (pos) {
	case 0:
		moveCima(c, x);
		break;
	case 1:
		moveDireita(c, y);
		break;
	case 2:
		moveEsquerda(c, y);
		break;
	case 3:
		moveBaixo(c, x);
		break;
	}
}


void moveDireita(Contaxi * c, float y) {
	c->taxi->yA = c->taxi->y;
	c->taxi->y = y + 1;
	c->taxi->xA = c->taxi->x;
}

void moveEsquerda(Contaxi* c, float y) {
	c->taxi->yA = c->taxi->y;
	c->taxi->y = y - 1;
	c->taxi->xA = c->taxi->x;
}
void moveCima(Contaxi* c, float x) {
	c->taxi->xA = c->taxi->x;
	c->taxi->x = x - 1;
	c->taxi->yA = c->taxi->y;
}
void moveBaixo(Contaxi* c, float x) {
	c->taxi->xA = c->taxi->x;
	c->taxi->x = x + 1;
	c->taxi->yA = c->taxi->y;
}

DWORD WINAPI threadInformacao(LPVOID lpParam) {
	Taxi* taxi = (Taxi*)lpParam;
	int i;

	taxi->id = GetCurrentProcessId();
	_tprintf(TEXT("Olá. O seu ID é: %d"), taxi->id);
	_tprintf(TEXT("\nIntroduza a sua matricula: "));
	_fgetts(taxi->matricula, sizeof(taxi->matricula) / sizeof(TCHAR), stdin);
	_tprintf(TEXT("\nIntroduza a posição onde começa (x, y): "));
	_tscanf_s(TEXT("%d, %d"), &taxi->x, &taxi->y);
	for (i = 0; taxi->matricula[i] != '\n'; i++);
	taxi->matricula[i] = '\0';
	taxi->id = GetCurrentProcessId();
	taxi->aceite = -1;
	taxi->atualizaMovimentacao = 0;
}

DWORD WINAPI threadEncerra(LPVOID lpParam) {
	HANDLE hEvent;

	hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_ENCERRA_TUDO);
	WaitForSingleObject(hEvent, INFINITE);
	ResetEvent(hEvent);
	exit(-1);
}