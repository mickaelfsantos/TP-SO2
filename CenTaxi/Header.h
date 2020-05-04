#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

typedef struct {
	int nProx;
	TCHAR matricula[12];
}SharedMSG;
