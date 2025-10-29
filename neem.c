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
	byte type;
	union {
		int i32;
		long i64;
		float f32;
		double f64;
	};

} Constant;

typedef struct {
	sym_t id;
	short argc;
	short localc;
	byte *code;
} Function;

typedef struct {
	Function *fun;
	ulong *locals;
	ulong *stack;
	struct Frame *parent;
} Frame;

typedef struct {

} Class;


typedef struct {
	Class *class;
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
		case OPC_NULL:
		case OPC_BCONST_0:
		case OPC_BCONST_1:
		case OPC_ICONST_0:
		case OPC_ICONST_1:
		case OPC_ICONST_2:
		case OPC_ICONST_3:
		case OPC_ICONST_4:
		case OPC_ICONST_5:
		case OPC_LCONST_0:
		case OPC_LCONST_1:
		case OPC_LCONST_2:
		case OPC_FCONST_0:
		case OPC_FCONST_1:
		case OPC_FCONST_2:
		case OPC_DCONST_0:
		case OPC_DCONST_1:
		case OPC_DCONST_2:
		case OPC_ILDC:
		case OPC_LLDC:
		case OPC_FLDC:
		case OPC_DLDC:	
		case OPC_I2L:
		case OPC_I2F:
		case OPC_I2D:
		case OPC_L2I:
		case OPC_L2F:
		case OPC_L2D:
		case OPC_F2I:
		case OPC_F2L:
		case OPC_F2D:
		case OPC_D2I:
		case OPC_D2L:
		case OPC_D2F:

		}
	}
}

void main(int argc, const char *argv)
{
	printf("helll.world\n");
}
























