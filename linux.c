#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/input.h>

uint64_t virt_to_phys(void *virt_addr)
{
	int fd = open("/proc/self/pagemap", O_RDONLY);
	if (fd < 0) {
		perror("open");
		return 0;
	}

	uint64_t page_size = getpagesize();
	uint64_t vpn = (uint64_t) virt_addr / page_size;
	uint64_t offset = vpn * sizeof(uint64_t);

	uint64_t entry;
	if (pread(fd, &entry, sizeof(entry), offset) != sizeof(entry)) {
		perror("pread");
		close(fd);
		return 0;
	}
	close(fd);

	// Bit 63 = page present, bits 0–54 = PFN (Page Frame Number)
	if (!(entry & (1ULL << 63))) {
		printf("Page not present in RAM!\n");
		return 0;
	}

	uint64_t pfn = entry & ((1ULL << 55) - 1);
	uint64_t phys_addr = (pfn * page_size) + ((uint64_t)virt_addr % page_size);
	return phys_addr;
}


void list_files_recursively(const char *dir_name)
{
	DIR *dir = opendir(dir_name);
	if (dir == NULL) return;
	struct dirent *entry = readdir(dir);
	while (entry != NULL) {
		printf("%s\n", entry->d_name);
		if (entry->d_type == DT_DIR && 
			strcmp(entry->d_name, ".") != 0 &&
			strcmp(entry->d_name, "..") != 0) {
			char path[1024] = {0};
			strcat(path, dir_name);
			strcat(path, "/");
			strcat(path, entry->d_name);
			list_files_recursively(path);
		}
		entry = readdir(dir);
	}
	closedir(dir);
}

void key_logger()
{
	struct input_event ev;
	int fd = open("/dev/input/event5", O_RDONLY);
	printf("%d\n", fd);
	FILE *log = fopen("/tmp/key.log", "a");
	while (1) {
		read(fd, &ev, sizeof(ev));
		fflush(stdout);
		fclose(log);
		if (ev.type == EV_KEY && ev.value == 0)
			printf("%d\t", ev.code);
	}
}

void main(int argc, const char **argv)
{
	int a = 100;
	printf("%p\n", a);
	uint64_t addr = virt_to_phys(&a);
	printf("%p\n", addr);
}