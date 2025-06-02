#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>

void error0()
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

void error1()
{
	int *ints = new int[100];
	memset(ints, sizeof(int) * 100, 'a');
	delete ints[];
	std::cout << ints[0];
}