#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonstd.h>

void hex(U32 val) {printf("%x\n", val);}
void binary(U32 value)
{
    U32 mask = 0xff000000; // start with a mask for the highest byte.
    U32 shift = 256*256*256; // start with a shift for the highest byte.
    U32 byte, byte_iterator, bit_iterator;
    for (byte_iterator = 0; byte_iterator < 4; byte_iterator++) {
        byte = (value & mask) / shift; // isolate each byte.
        for (bit_iterator = 0; bit_iterator < 8; bit_iterator++) { // print the byte's bits.
            if (byte & 0x80) // if the highest bit in the byte isn't 0,
                printf("1"); // print a 1.
            else printf("0"); // print a 0.
            byte *= 2; // move all the bits to the left by 1.
        }
        if (byte_iterator == 3) printf("\n");
        else printf(" ");
        mask /= 256; // move the bits in mask right by 8.
        shift /= 256; // move the bits in shift right by 8.
    }
}


struct Person
{
	val name;
	var age;
	val sal;
}

void main(const int argc, char const **argv)
{
	U8 data[2] = {0};
	U16 val = 0xabcd;
	binary(0xabcd);
	hex(val);
	

	val x = 1000;

	data[0] = val;
	data[1] = val >> 8;
	printf("%x\n", *(U16*)data);
}











