#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>
struct color {
    uint8_t r, g, b, a;
    color(uint8_t rr=0, uint8_t gg=0, uint8_t bb=0, uint8_t aa=255)
        : r(rr), g(gg), b(bb), a(aa) {}
};
struct object
{
	object() {std::cout << "contructor called" << std::endl;}
	~object() {std::cout << "destructor called" << std::endl;}
	object(const object &obj) {std::cout << "copy constructor called" << std::endl;}
	void operator=(const object &obj) {std::cout << "assignment operator called" << std::endl;}
};
std::vector<object> call() {
	std::vector<object> data;
	object obj;
	data.push_back(obj);
	data.push_back(obj);
	data.push_back(obj);
	return data;
}

int main(int argc, const char **argv)
{
	std::cout << sizeof(color) << std::endl;
}