#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define null NULL

typedef unsigned char byte;
typedef unsigned int uint;
typedef unsigned long ulong;

#define OPC_NOP 1
#define OPC_LOAD_N 2
#define OPC_LOAD_T 3
#define OPC_LOAD_F 4
#define OPC_LOAD_0 5
#define OPC_LOAD_1 6
#define OPC_LOAD_2 7
#define OPC_LOAD_3 8
#define OPC_LOAD_4 9
#define OPC_LOAD_5 10
#define OPC_LOAD_0L 11
#define OPC_LOAD_1L 12
#define OPC_LOAD_2L 13
#define OPC_LOAD_0_0 14
#define OPC_LOAD_1_0 15
#define OPC_LOAD_2_0 16
#define OPC_LOAD_M1  17
#define OPC_LOAD_M1_0 18


typedef struct {
	byte *code;
} Function;

typedef struct {
	Function *fun;
	ulong *locals;
	ulong *stack;
	struct Frame *parent;
} Frame;

typedef struct {
	uint lock;
	
} Object;


void Processor()
{
	Frame *frame = null;
	Function *fun = frame->fun;
	ulong *locals = frame->locals;
	ulong *sp = frame->stack;
	byte *ip = fun->code;
	Object *self = (Object*) locals[0];

	while (true) {
		switch (*ip) {
		case OPC_NOP:
			ip += 1;
			break;
		case OPC_LOAD_N:
			*sp++ = 0;
			ip += 1;
			break;
		case OPC_LOAD_T:
			*sp++ = 1;
			ip += 1;
			break;
		case OPC_LOAD_F:
		case OPC_LOAD_0:
			*sp++ = 0;
			ip += 1;
			break;
		case OPC_LOAD_1:
			*sp++ = 1;
			ip += 1;
			break;
		case OPC_LOAD_2:
			*sp++ = 2;
			ip += 1;
			break;
		case OPC_LOAD_3:
			*sp++ = 3;
			ip += 1;
			break;
		case OPC_LOAD_4:
			*sp++ = 4;
			ip += 1;
			break;
		case OPC_LOAD_5:
			*sp++ = 5;
			ip += 1;
			break;
		case OPC_LOAD_0L:
			*sp++ = 0;
			ip += 1;
			break;
		case OPC_LOAD_1L:
			*sp++ = 1;
			ip += 1;
			break;
		case OPC_LOAD_2L:
			*sp++ = 5;
			ip += 1;
			break;
		case OPC_LOAD_0_0:
			*(double*) sp++ = 0.0;
			ip += 1;
			break;
		case OPC_LOAD_1_0:
			*(double*) sp++ = 1.0;
			ip += 1;
			break;
		case OPC_LOAD_2_0:
			*(double*) sp++ = 2.0;
			ip += 1;
			break;
		case OPC_LOAD_M1:
			*sp++ = -1;
			ip += 1;
			break;
		case OPC_LOAD_M1_0:	
			*(double*) sp++ = -1.0;
			ip += 1;
			break;
			
		}
	}
}

void main(int argc, const char *argv)
{
	printf("helll.world\n");
}
























