#include <stdio.h>
#include <stdlib.h>
#include <nonstd.h>




class User {

	mut Str name;
	mut int age;
	mut int aadhar;
	mut num salary;

	construct(Str name, int age, int aadhar) {
		this.name = name;
		this.age = age;
		this.aadhar = aadhar;
	}
}


bool User::operator==(User rhs)
{
	return this.name == rhs.name and
		this.age == rhs.age and 
		this.aadharNo = rhs.aadharNo;
}

void main(const int argc, const char **argv)
{
	U64 a = 100;
	printf("%d\n", a);
}