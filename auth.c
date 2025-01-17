#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void withdraw_money()
{
	printf("Money withdrawn.\n");
}

int main()
{
	char password[32];
	printf("Enter your password: ");
	gets(password);
	if (strcmp(password, "1Matthew") == 0)
		withdraw_money();
	else printf("Wrong password.\n");
	return EXIT_SUCCESS;
}
