#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
void main(int argc, const char **argv) {
	int **data = (int**) malloc(sizeof(int*) * 100);
	for (int i = 0; i < 100; i++) {
		data[i] = (int*) malloc(sizeof(int)*10);
		if ((unsigned)data[i] & 15)
			printf("%p\n", data[i]);
	}
}

