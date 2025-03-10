#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

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

void main(int argc, const char **argv)
{
	printf("hello.world\n");
	list_files_recursively("/home/dkumar/RESEARCH");
}