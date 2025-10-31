#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
struct Function {
	uint32_t id;
	uint16_t argc;
	uint16_t localc;
	uint32_t offset;
	void *class;
};
void main(int argc, const char **argv) {
<<<<<<< HEAD
	printf("%d\n", sizeof(pthread_cond_t));
=======
	uint32_t a = -100;
	printf("%d\n", sizeof(struct Function));
	printf("%d\n", a);
>>>>>>> 7329b88120371f71a010173abb909cfea16d42bb
}

