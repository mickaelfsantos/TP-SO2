#include "Contaxi.h"

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hThreadCom, hThreadEnc, hThreadComandos, hThreadInf, hThreadMovimentacao, hWaitableTimer, hThreadPassageiros, hThreadRespostas, hThreadEsperaPassageiros;
	Taxi taxi;
	LARGE_INTEGER liDueTime;
	Contaxi c;
	int i;
	TCHAR pipe[PIPESIZE];

	HINSTANCE hLib, hCom;

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	hThreadEnc = CreateThread(NULL, 0, threadEncerra, &c, 0, NULL);
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
	dll2_carregaMapa dll2_carregaM = (dll2_carregaMapa)GetProcAddress(hCom, "carregaMapa");


	if (dll2_comunicaV == NULL || dll2_carregaM == NULL || dll_logV == NULL || dll_registerV==NULL) {
		_tprintf(TEXT("\nErro no ponteiro para a DLL."));
		return 0;
	}
	
	WaitForSingleObject(hThreadInf, INFINITE);

	taxi = dll2_comunicaV(taxi);
	if (taxi.aceite != 1) {
		_tprintf(TEXT("\nNão foi aceite por parte da central")); 
		dll_logV(TEXT("\nNão foi aceite por parte da central"));
		_getch();
		FreeLibrary(hLib);
		FreeLibrary(hCom);
		return 0;
	}

	c.alturaMapa = taxi.alturaMapa;
	c.larguraMapa = taxi.larguraMapa;
	c.maxTaxis = taxi.maxTaxis;
	c.mapa = NULL;
	c = dll2_carregaM(c);
	c.nq = NQ;

	taxi.atualizaMovimentacao = 1;
	taxi.temPassageiro = 0;
	taxi.xA = taxi.x;
	taxi.yA = taxi.y;
	taxi.aleatorio = 1;
	taxi.distancia = -1;

	
	c.taxi = &taxi;

	hWaitableTimer =  CreateWaitableTimer(NULL, TRUE, taxi.matricula);
	if (hWaitableTimer ==NULL){
		printf("Erro ao criar waitable timer (%d)\n", GetLastError());
		return 1;
	}
	dll_registerV(WAITABLETIMER, 5);


	hThreadComandos = CreateThread(NULL, 0, threadComandosTaxi, &c, 0, NULL);
	hThreadPassageiros = CreateThread(NULL, 0, threadPassageiros, &c, 0, NULL);
	hThreadEsperaPassageiros = CreateThread(NULL, 0, threadEsperaPassageiro, &c, 0, NULL);

	while (c.sair!=1) {

		liDueTime.QuadPart = (WAIT_VELOCIDADE_UM / taxi.velocidade);
		
		if (!SetWaitableTimer(hWaitableTimer, &liDueTime, 0, NULL, NULL, 0))
		{
			printf("Erro ao fazer setWaitableTimer (%d)\n", GetLastError());
			return 1;
		}
		WaitForSingleObject(hWaitableTimer, INFINITE);
		if (c.taxi->aleatorio == 1) {
			movimentaCarro(&c);
			taxi.x = c.taxi->x;
			taxi.y = c.taxi->y;
			taxi.velocidade = c.taxi->velocidade;
			taxi.xA = c.taxi->xA;
			taxi.yA = c.taxi->yA;
			dll_logV(TEXT("%d %d %f %s"), taxi.x, taxi.y, taxi.velocidade, taxi.matricula);
			dll2_comunicaV(taxi);
		}
		else {
			movimenta(&c);
			taxi.x = c.taxi->x;
			taxi.y = c.taxi->y;
			taxi.velocidade = c.taxi->velocidade;
			taxi.xA = c.taxi->xA;
			taxi.yA = c.taxi->yA;
			dll_logV(TEXT("%d %d %f %s"), taxi.x, taxi.y, taxi.velocidade, taxi.matricula);
			dll2_comunicaV(taxi);
		}
		
	}


	CloseHandle(hWaitableTimer);

	return 0;
}



int posValida(int *mat, int *visited, int x, int y, int altura, int largura)
{
	if (x <= altura && y <= largura && x >= 0 && y >= 0)
		if (*(mat + x * largura + y) == 1 && *(visited + x * largura + y) == 0)
			return 1;

	return 0;
}

int posValida2(int* mat, int x, int y, int altura, int largura)
{
	if (x <= altura && y <= largura && x >= 0 && y >= 0)
		if (*(mat + x * largura + y) == 1)
			return 1;

	return 0;
}


void encontraCaminho(int *mat, int *visited, int x, int y,
	int xPretendido, int yPretendido, int* min_dist, int dist, int*caminho, int altura, int largura)
{

	if (dist >= *min_dist)
		return;

	if (x == xPretendido && y == yPretendido)
	{
		if (dist < *min_dist) {
			*min_dist = dist;
			for (int k = 0; k < altura; k++) {
				for (int l = 0; l < largura; l++) {
					*(caminho + k * largura + l) = *(visited + k * largura + l);
				}
			}
			*(caminho + x * largura + y) = 1;
		}
		return;
	}

	*(visited + x * largura + y) = 1;

	// baixo
	if (posValida(mat, visited, x + 1, y, altura, largura)==1)
		encontraCaminho(mat, visited, x + 1, y, xPretendido, yPretendido, min_dist, dist + 1, caminho, altura, largura);

	// direita
	if (posValida(mat, visited, x, y + 1, altura, largura)==1)
		encontraCaminho(mat, visited, x, y + 1, xPretendido, yPretendido, min_dist, dist + 1, caminho, altura, largura);

	// cima
	if (posValida(mat, visited, x - 1, y, altura, largura)==1)
		encontraCaminho(mat, visited, x - 1, y, xPretendido, yPretendido, min_dist, dist + 1, caminho, altura, largura);

	// esquerda
	if (posValida(mat, visited, x, y - 1, altura, largura)==1)
		encontraCaminho(mat, visited, x, y - 1, xPretendido, yPretendido, min_dist, dist + 1, caminho, altura, largura);

	*(visited + x * largura + y) = 0;
}


void movimenta(Contaxi* c) {
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
		return -1;
	}

	WaitForSingleObject(hMutex, INFINITE);
	
	// baixo
	if (posValida2(c->taxi->caminho, c->taxi->x + 1, c->taxi->y, c->alturaMapa, c->larguraMapa) == 1) {
		moveBaixo(c, c->taxi->x);
	}
	else {
		// direita
		if (posValida2(c->taxi->caminho, c->taxi->x, c->taxi->y + 1, c->alturaMapa, c->larguraMapa) == 1) {
			moveDireita(c, c->taxi->y);
		}
		else {
			// cima
			if (posValida2(c->taxi->caminho, c->taxi->x - 1, c->taxi->y + 1, c->alturaMapa, c->larguraMapa) == 1) {
				moveCima(c, c->taxi->x);
			}
			else {
				// esquerda
				if (posValida2(c->taxi->caminho, c->taxi->x, c->taxi->y - 1, c->alturaMapa, c->larguraMapa) == 1) {
					moveEsquerda(c, c->taxi->y);
				}
			}
		}
	}

	*(c->taxi->caminho + c->taxi->xA * c->larguraMapa + c->taxi->yA) = 0;
	c->taxi->distancia--;
	if (c->taxi->distancia == 0) {
		c->taxi->aleatorio = 1;
	}
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

}
void movimentaCarro(Contaxi * c) {

	int direcao, pos = -1, sair, nPossiveis = 0, posAux;
	int aux[4] = { 0,0,0,0 };
	int* arr = NULL;
	int x, y, xA, yA;
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
		return -1;
	}

	WaitForSingleObject(hMutex, INFINITE);
	x = c->taxi->x;
	y = c->taxi->y;
	xA = c->taxi->xA;
	yA = c->taxi->yA;

	
	if (x >= 1 && *(c->mapa + (x - 1) * c->larguraMapa + y) == 1) { //pode subir
		aux[0] = 1;
		nPossiveis++;
	}
	if (y <= c->larguraMapa - 1 && *(c->mapa + x * c->larguraMapa + (y+1)) == 1) { //pode ir para a direita
		aux[1] = 1;
		nPossiveis++;
	}
	if (y >= 1 && *(c->mapa + x * c->larguraMapa + (y - 1)) == 1) { //pode ir para a esquerda
		aux[2] = 1;
		nPossiveis++;
	}
	if (x <= c->alturaMapa - 1 && *(c->mapa + (x+1) * c->larguraMapa + y) == 1) { //pode descer
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
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
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


DWORD WINAPI threadPassageiros(LPVOID lpParam) {
	Contaxi* c = (Contaxi*)lpParam;
	HANDLE hCom;
	hCom = LoadLibrary(TEXT("Dll.dll"));
	dll2_threadPassageiros dll_threadPassageiros = (dll2_threadPassageiros)GetProcAddress(hCom, "threadPassageiros");
	dll_threadPassageiros(c);
}

DWORD WINAPI threadEsperaPassageiro(LPVOID lpParam) {
	Contaxi* c = (Contaxi*)lpParam;
	HANDLE hCom, hEvent, hMutex;
	
	hEvent = CreateEvent(NULL, TRUE, FALSE, CHEGOU_PASSAGEIRO);

	hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
		return -1;
	}
	
	while (c->sair != 1) {
		WaitForSingleObject(hEvent, INFINITE);
		ResetEvent(hEvent);
		if (_tcscmp(c->taxi->passageiro.nome, TEXT("")) != 0) {
			int* visited = malloc(sizeof(int) * c->alturaMapa * c->larguraMapa);
			int* caminho = malloc(sizeof(int) * c->alturaMapa * c->larguraMapa);
			for (int i = 0; i < c->alturaMapa; i++) {
				for (int j = 0; j < c->larguraMapa; j++) {
					*(visited + i * c->larguraMapa + j) = 0;
				}
			}
			for (int i = 0; i < c->alturaMapa; i++) {
				for (int j = 0; j < c->larguraMapa; j++) {
					*(caminho + i * c->larguraMapa + j) = 0;
				}
			}
			
			int min_dist = INT_MAX;

			WaitForSingleObject(hMutex, INFINITE);
			encontraCaminho(c->mapa, visited, c->taxi->x, c->taxi->y,
				c->taxi->passageiro.x, c->taxi->passageiro.y, &min_dist, 0, caminho, c->alturaMapa, c->larguraMapa);
			c->taxi->caminho = malloc(sizeof(int) * c->alturaMapa * c->larguraMapa);
			memcpy(c->taxi->caminho, caminho, sizeof(int) * c->alturaMapa * c->larguraMapa);
			c->taxi->aleatorio = 0;
			c->taxi->distancia = min_dist;
			ReleaseMutex(hMutex);
			free(visited);
			free(caminho);
		}
	}

}

DWORD WINAPI threadInformacao(LPVOID lpParam) {
	Taxi* taxi = (Taxi*)lpParam;
	TCHAR lixo[2];
	int i;
	TCHAR pipe[PIPESIZE];

	taxi->id = GetCurrentProcessId();
	_tprintf(TEXT("Olá. O seu ID é: %d"), taxi->id);
	_tprintf(TEXT("\nIntroduza a sua matricula: "));
	_fgetts(taxi->matricula, sizeof(taxi->matricula) / sizeof(TCHAR), stdin);
	_tprintf(TEXT("\nIntroduza a posição onde começa (x, y): "));
	_tscanf_s(TEXT("%d, %d"), &taxi->x, &taxi->y);
	_fgetts(lixo, sizeof(lixo) / sizeof(TCHAR), stdin);
	for (i = 0; taxi->matricula[i] != '\n'; i++);
	taxi->matricula[i] = '\0';
	taxi->id = GetCurrentProcessId();
	taxi->aceite = -1;
	taxi->atualizaMovimentacao = 0; 
	taxi->velocidade = 1;
	_tcscpy_s(pipe, PIPESIZE, TEXT("\\\\.\\pipe\\pipe"));
	_tcscat_s(pipe, PIPESIZE, taxi->matricula);
	_tcscpy_s(taxi->pipe, PIPESIZE, pipe);
}

DWORD WINAPI threadEncerra(LPVOID lpParam) {
	HANDLE hEvent;
	Contaxi* c = (Contaxi*)lpParam;

	hEvent = CreateEvent(NULL, TRUE, FALSE, EVENTO_ENCERRA_TUDO);
	WaitForSingleObject(hEvent, INFINITE);
	ResetEvent(hEvent);
	c->sair = 1;
}

DWORD WINAPI threadComandosTaxi(LPVOID lpParam) {
	Contaxi* c = (Contaxi*)lpParam;
	int op, i;
	TCHAR comando[256];
	
	do {
		mostraComandos();
		_tprintf(TEXT("\n\n\tComando: ")); 
		fflush(stdin);
		_fgetts(comando, sizeof(comando) / sizeof(TCHAR), stdin);
		for (i = 0; comando[i] != '\n'; i++);
		comando[i] = '\0';
		op = trataComando(comando, c);
	} while (op != -1);

}

void acelerar(Contaxi* c) {
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
		return;
	}

	WaitForSingleObject(hMutex, INFINITE);
	if (c->taxi->velocidade <= 1.5) {
		c->taxi->velocidade += 0.5;
	}
	ReleaseMutex(hMutex, INFINITE);
}

void sair(Taxi t) {
	HANDLE hCom;
	hCom = LoadLibrary(TEXT("Dll.dll"));
	dll2_comunicaSaida dll2_comunicaSV = (dll2_comunicaSaida)GetProcAddress(hCom, "comunicaSaida");
	dll2_comunicaSV(t);
	exit(-1);
}

void desacelerar(Contaxi* c) {
	HANDLE hMutex;
	hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
		return;
	}

	WaitForSingleObject(hMutex, INFINITE);
	if (c->taxi->velocidade >= 1) {
		c->taxi->velocidade -= 0.5;
	}
	ReleaseMutex(hMutex, INFINITE);
}

void alterarNq(Contaxi* c) {
	HANDLE hMutex;
	int nq;
	hMutex = CreateMutex(NULL, FALSE, MUTEX_ALTERA_TAXI);
	if (hMutex == NULL) {
		_tprintf(TEXT("\nErro ao criar Mutex (%d)"), GetLastError());
		return;
	}
	_tprintf(TEXT("\n\tIntroduza o número de quadriculas que pretende: "));
	_tscanf_s(TEXT("%d"), &nq);
	WaitForSingleObject(hMutex, INFINITE);
	c->nq = nq;
	ReleaseMutex(hMutex, INFINITE);
}

int trataComando(int opcao, Contaxi* c) {
	switch (opcao)
	{
	case 1:
		acelerar(c);
		return 0;
	case 2:
		desacelerar(c);
		return 0;
	case 3:
		alterarNq(c);
		return 0;
	case 4:
		sair(*c->taxi);
		return -1;
	default:
		return 1;;
	}
}

void mostraComandos() {
	_tprintf(TEXT("\n\t---Comandos disponíveis---"));
	_tprintf(TEXT("\n\t1- acelerar: acelera o taxi"));
	_tprintf(TEXT("\n\t2- desacelerar: desacelerar o taxi"));
	_tprintf(TEXT("\n\t3- alterarNQ: alterar número de quadriculas no demonstrar interesse"));
	_tprintf(TEXT("\n\t4- sair\n"));
}

void limpaEcra() {
	for (int i = 0; i < 30; i++)
		_tprintf(TEXT("\n"));
}