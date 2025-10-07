#include <stdio.h>
#include <stdlib.h>

struct User
{
	int x;
	int y;
	User(int a, int b) {
		x = a;
		y = b;
	}
};


int main() {

	User u(100, 200);
	return EXIT_SUCCESS;
}