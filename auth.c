#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, const char **argv) {

	char buf[100];
	memset(buf, 0, 100);
	printf("Enter password: ");
	scanf("%s", buf);
	if (strcmp(buf, "secret") == 0) printf("Access granted.\n");
	else printf("Access denied.\n");
	return EXIT_SUCCESS;
}
