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
	vec.push_back(400);
	vec.push_back(500);
	vec.push_back(600);
	vec.push_back(700);
	vec.push_back(800);
	int &a = vec[3];
	std::cout << a << std::endl;
	vec.push_back(900);
	std::cout << a << std::endl;
	std::cout << KEY << std::endl;
}