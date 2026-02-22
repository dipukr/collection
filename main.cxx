#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

struct object
{
	object() {std::cout << "contructor called" << std::endl;}
	~object() {std::cout << "destructor called" << std::endl;}
	object(const object &obj) {std::cout << "copy constructor called" << std::endl;}
	void operator=(const object &obj) {std::cout << "assignment operator called" << std::endl;}
};

std::vector<object> call() {
	object obj;
	std::vector<object> data;
	data.reserve(10);
	std::cout << data.capacity() << std::endl;
	data.emplace_back(obj);
	return data;
}

int main() {
	std::cout << __cplusplus << std::endl;
}