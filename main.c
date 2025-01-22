#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonstd.h>
#define N 11

void main(const int argc, char const **argv)
{
	char a[] = "helloworld";
	char b[N];
	memset(b, 0, N);
	for(int i=0;i<strlen(a);i++) {
		b[i] = a[i] ^ 4;
	}
	printf("%s\n", b);
	for(int i=0;i<strlen(b);i++) {
		b[i] = b[i] ^ 4;
	}
	printf("%s\n", b);
}











