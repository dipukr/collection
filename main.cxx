#include <iostream>
#define logger(a) std::cout << a << std::endl

struct Address {
	Address() {
		logger("Address created");
	}
	~Address() {
		logger("Address destroyed");
	}
};

struct User {
	Address addr;
	User() {
		logger("User created");
	}
	~User() {
		logger("User destroyed");
	}
};

User getUser() {
	User u;
	return u;
}


void log(uint a) {
	logger(a);
}


int main(int argc, const char **argv) { 
	log(-100);
	return EXIT_SUCCESS;
}