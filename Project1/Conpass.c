#include "Conpass.h"

int _tmain(int argc, TCHAR* argv[]) {

	int opcao;
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	HANDLE hPipe;
	Passageiro p;
	DWORD dwWriten, dwRead;


	if (!WaitNamedPipe(PIPENAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe %s!\n"), PIPENAME);
		exit(EXIT_FAILURE);
	}
	hPipe = CreateFile(PIPENAME, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe: %s!\n"), PIPENAME);
		exit(EXIT_FAILURE);
	}
	p.x = 0;
	p.y = 1;
	_tcscpy_s(p.nome, sizeof(p.nome) / sizeof(TCHAR), TEXT("Chamo-me atrasado"));
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
	_tprintf(TEXT("Nao nao chamo, chamo %s"), p.nome);
	_getch();
	return 0;
}

void menu() {
	_tprintf(TEXT("1 - Criar passageiro"));
	_tprintf(TEXT("2 - Mover passageiro"));
	_tprintf(TEXT("3 - Sair"));
}

void trataOpcao(int opcao) {
	switch (opcao)
	{
	case 1:
		_tprintf(TEXT("Introduza o seu nome: "));
		_tprintf(TEXT("Posições para onde pretende ser levado: "));
		break;
	default:
		break;
	}
}