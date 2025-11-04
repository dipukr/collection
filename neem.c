#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define null NULL

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

#define CONSTANT_INT    1
#define CONSTANT_LONG   2
#define CONSTANT_FLOAT  3
#define CONSTANT_DOUBLE 4
#define CONSTANT_STRING 5
#define CONSTANT_SYMBOL 6

size_t allocated = 0;

void runtime_error(const char *what)
{
	fprintf(stdout, "%s\n", what);
	exit(EXIT_FAILURE);
}


union Constant {
	int32_t i32;
	int64_t i64;
	float f32;
	double f64;
	uint32_t sym;
};


void* mem_alloc(size_t sz)
{
	allocated += sz;
	void *mem = malloc(sz);
	if (mem == null) runtime_error("Out of memory");
	return mem;
}

struct Class {
	uint32_t id;
	uint32_t datac;
	uint32_t functionc;
	struct Function *functions;
};

struct Function {
	uint32_t id;
	uint16_t argc;
	uint16_t localc;
	uint32_t *offset;
	struct Class *class;
};

struct Frame {
	uint64_t *locals;
	uint64_t *stack;
	struct Function *fun;
	struct Frame *parent;
};


struct Object {
	struct Class *class;
};

struct Context {
	struct Function *func;
	struct Class *class;
	uint64_t *sp;
	uint64_t *bp;
	uint64_t *ip; 
};


size_t constantc;
struct Constant *constants;

size_t symbolc;
struct char **symbols;

size_t classc;
struct Class *classes;

size_t functionc;
struct Function *functions;

void file_reader::read(char *dest, U64 sz) 
{
	U64 rd = fread(dest, sz, 1, stream);
	assert(rd == 1);
}


uint8_t read_uint8_t(FILE *file)
{
	uint8_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

uint16_t read_uint16_t(FILE *file)
{
	uint16_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

uint32_t read_uint32_t(FILE *file)
{
	uint32_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

uint64_t read_uint64_t(FILE *file)
{
	uint64_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

int8_t read_int8_t(FILE *file)
{
	int8_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

int16_t read_int16_t(FILE *file)
{
	int16_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

int32_t read_int32_t(FILE *file)
{
	int32_t v;
	size_t sz = fread((char*) &v, sizeof(v), 1, stream);
	if (sz != 1) runtime_error("File IO error");
	return v;
}

int64_t read_int64_t(FILE *file);
float read_float(FILE *file);
double read_double(FILE *file);
struct Function* read_function(FILE *file);
struct Class* read_class(FILE *file);

void write_uint8_t(FILE *file, uint8_t v)
void write_uint16_t(FILE *file, uint16_t v)
void write_uint32_t(FILE *file, uint32_t v)
void write_uint64_t(FILE *file, uint64_t v)
void write_int8_t(FILE *file, int8_t v)
void write_int16_t(FILE *file, int16_t v)
void write_int32_t(FILE *file, int32_t v)
void write_int64_t(FILE *file, uint64_t v)


void Processor()
{
	Frame *frame = null;
	uint64_t *locals = frame->locals;
	uint64_t *sp = frame->stack;
	uint8_t *ip = fun->code;
	struct Function *func = frame->func;
	struct Object *self = (Object*) locals[0];

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
		case OPC_LDC_I:
		case OPC_LDC_L:
		case OPC_LDC_F:
		case OPC_LDC_D:	
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
		case OPC_CALL:
				
		}
	}
}

void main(int argc, const char *argv)
{
	printf("helll.world\n");
}
























