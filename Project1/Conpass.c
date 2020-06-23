#include "Conpass.h"

int _tmain(int argc, TCHAR* argv[]) {

	int opcao;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	HANDLE hThreadComandos, hThreadRecebeRespostas;

	hThreadComandos = CreateThread(NULL, 0, threadComandos, NULL, 0, NULL);
	hThreadRecebeRespostas = CreateThread(NULL, 0, threadRecebeRespostas, NULL, 0, NULL);

	WaitForSingleObject(hThreadComandos, INFINITE);
}

void menu() {
	_tprintf(TEXT("\n\t1 - Transportar passageiro"));
	_tprintf(TEXT("\n\t2 - Sair"));
}

DWORD WINAPI threadComandos(LPVOID lpParam) {
	int op;

	do {
		menu();
		_tprintf(TEXT("\n\n\tOpção: "));
		fflush(stdin);
		_tscanf_s(TEXT("%d"), &op);
		op = trataComando(op);
	} while (op != -1);

}

DWORD WINAPI threadRecebeRespostas(LPVOID lpParam) {
	HANDLE hPipe;
	Passageiro p;
	DWORD dwWriten, dwRead;

	hPipe = CreateNamedPipe(PIPENAME_RESPOSTA, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1,
		sizeof(Passageiro), sizeof(Passageiro), 0, NULL);

	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Problema ao criar named pipe do servidor: %d\n"), GetLastError());
		return EXIT_FAILURE;
	}

	while (1) {

		if (!ConnectNamedPipe(hPipe, NULL)) {
			_tprintf(TEXT("[ERRO] Ligação ao pipe do servior!\n"));
			exit(EXIT_FAILURE);
		}

		if (!ReadFile(hPipe, &p, sizeof(Passageiro), &dwRead, NULL)) {
			_tprintf(TEXT("[ERRO] Leitura do named pipe: %d\n"), GetLastError());
			exit(EXIT_FAILURE);
		}
		if (!dwRead) {
			_tprintf(TEXT("[ERRO] Não foram lidos bytes \n"));
			exit(EXIT_FAILURE);
		}
		if (_tcscmp(p.taxi, TEXT("")) == 0) {
			p.estado = 1;
			_tprintf(TEXT("\n\tNão foi atribuido nenhum taxi ao passageiro %s\n\tOpção:"), p.nome);
		}
		else {
			_tprintf(TEXT("\n\tFoi atribuido o taxi %s ao passageiro %s\n\tOpção:"), p.taxi, p.nome);
		}
		DisconnectNamedPipe(hPipe);
	}
}

DWORD WINAPI criaPassageiro(LPVOID lpParam) {

	HANDLE hPipe, hPipeResposta;
	Passageiro *passageiro = (Passageiro*)lpParam;
	Passageiro p;
	DWORD dwWriten, dwRead;
	TCHAR lixo[2];
	int i;

	p.estado = passageiro->estado;
	p.x = passageiro->x;
	p.y = passageiro->y;
	p.xPretendido = passageiro->xPretendido;
	p.yPretendido = passageiro->yPretendido;
	_tcscpy_s(p.nome, sizeof(p.nome) / sizeof(TCHAR), passageiro->nome);


	if (!WaitNamedPipe(PIPENAME_NOVO, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe %s!\n"), PIPENAME_NOVO);
		exit(EXIT_FAILURE);
	}
	hPipe = CreateFile(PIPENAME_NOVO, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe: %s!\n"), PIPENAME_NOVO);
		exit(EXIT_FAILURE);
	}
	if (!WriteFile(hPipe, &p, sizeof(Passageiro), &dwWriten, NULL)) {
		_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
		exit(EXIT_FAILURE);
	}

	if (!ReadFile(hPipe, &p, sizeof(Passageiro), &dwRead, NULL)) {
		_tprintf(TEXT("[ERRO] Leitura do named pipe: %d\n"), GetLastError());
		exit(EXIT_FAILURE);
	}
	if (!dwRead) {
		_tprintf(TEXT("[ERRO] Não foram lidos bytes \n"));
		exit(EXIT_FAILURE);
	}
	_tprintf(TEXT("\n\t%s\n\tOpção: "), p.resposta);
}


int trataComando(int op) {
	HANDLE hThreadEnvia;
	Passageiro p;

	switch (op){
	case 1: 
		p = obtemDados();
		hThreadEnvia = CreateThread(NULL, 0, criaPassageiro, &p, 0, NULL);
		break;
	case 2:
		return -1;
	default:
		break;
	}
}

Passageiro obtemDados() {
	TCHAR lixo[2];
	Passageiro p;
	int i;

	_tprintf(TEXT("\n\tIntroduza o seu nome: "));
	_fgetts(lixo, 2, stdin);
	_fgetts(p.nome, sizeof(p.nome) / sizeof(TCHAR), stdin);
	for (i = 0; p.nome[i] != '\n'; i++);
	p.nome[i] = '\0';
	_tprintf(TEXT("\n\tPosições onde se encontra: "));
	_tscanf_s(TEXT("%d, %d"), &p.x, &p.y);

	_tprintf(TEXT("\n\tPosições para onde pretende ir: "));
	_tscanf_s(TEXT("%d, %d"), &p.xPretendido, &p.yPretendido);
	p.estado = 0;
	return p;
}
