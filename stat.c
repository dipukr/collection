#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <nonstd.h>
#include <stdbool.h>

void file_write(const int argc, char const **argv)
{
	long x = 100;
	FILE *file = fopen("test.db", "wb");
	assert(file != null);
	U8 *mem = (U8*) malloc(GB4);
	assert(mem != null);
	memset(mem, 65, GB4);
	assert(fwrite(mem, GB4, 1, file) == 1);
	fclose(file);
}

void file_read0()
{
	FILE *file = fopen("test.db", "rb");
	assert(file != null);
	U8 *mem = (U8*) malloc(GB);
	assert(mem != null);
	U64 a = 0;
	while (true) {
		mem[a++] = (U8) fgetc(file);
		if (a == GB) break;
	}
	printf("Read %u bytes.\n", a);
	fclose(file);
}

void file_read1()
{
	FILE *file = fopen("test.db", "rb");
	assert(file != null);
	U8 *mem = (U8*) malloc(GB4);
	assert(mem != null);
	assert(fread(mem, GB4, 1, file) == 1);
	fclose(file);
}

void file_write0()
{
	FILE *file = fopen("test.db", "wb");
	assert(file != null);
	U8 *mem = (U8*) malloc(GB);
	assert(mem != null);
	memset(mem, 65, GB);
	U64 sz = fwrite(mem, 1, GB, file);
	printf("Wrote %u bytes.\n", sz);
	fclose(file);
}

void main(const int argc, char const **argv)
{
	eval(file_read1());
	eval(file_read2());
}