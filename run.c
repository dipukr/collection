#include <stdlib.h>
#include <assert.h>
#include <nonstd.h>
#include <stdio.h>

void main(const int argc, const char **argv)
{
	FILE *file = fopen("cmd", "r");
	assert(file != null);
	char cmd[1024];
	U64 sz = fread(cmd, 1, 1024, file);
	cmd[sz] = 0;
	fclose(file);
	system(cmd);
}