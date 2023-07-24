#ifndef REGISTROS_CPU
#define REGISTROS_CPU

#include <stdint.h>
#include <stdlib.h>

typedef struct{
char AX[4];
char BX[4];
char CX[4];
char DX[4];
char EAX[8];
char EBX[8];
char ECX[8];
char EDX[8];
char RAX[16];
char RBX[16];
char RCX[16];
char RDX[16];

}t_registros_cpu;


#endif

/*
Registros de 4 bytes: AX, BX, CX, DX. 
Registros de 8 bytes: EAX, EBX, ECX, EDX
Registros de 16 bytes: RAX, RBX, RCX, RDX
*/


