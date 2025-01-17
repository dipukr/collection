#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>

int main(const int argc, const char **argv)
{
	std::vector<int> vec;
	vec.push_back(100);
	vec.push_back(200);
	vec.push_back(300);
	std::cout << vec[0] << std::endl;
	std::cout << vec.size() << std::endl;
}