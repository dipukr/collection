#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

void handler() {
	write(STDOUT_FILENO, "hello.world\n", 13);
}

int main(int argc, const char **argv) {
	signal(SIGTERM, handler);
	while (true) {
		printf("Wasting your cycles: %d\n", getpid());
		sleep(1);
	}
	return EXIT_SUCCESS;
}

