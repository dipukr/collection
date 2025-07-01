#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonstd.h>
#define N 10000

void main(const int argc, const char **argv)
{
	register int v = 10;
	int *p = &v;
	printf("%d\n", v);
}



