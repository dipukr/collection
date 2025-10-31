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
	printf("%d\n", sizeof(pthread_cond_t));
}

