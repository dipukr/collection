#include <iostream>

int add_binary(int a, int b)
{
	int c = 0;
	int f = 1;
	int v = 0;
	while (a > 0 || b > 0) {
		int r = (a > 0 ? a % 10 : 0) + (b > 0 ? b % 10 : 0);
		cout << "r: " << r << endl;
		r = (r == 2) ? 0 : r;
		c = (r > 1) ? 1 : 0;
		cout << "c: " << c << endl;
		v += v * f + (r == 1) ? 1 : 0;
		cout << "v: " << v << endl;
		a = a / 10;
		b = b / 10;
		f = f * 10;
	}
	return v;
}

void binary_print(uint32_t value)
{
	uint32_t mask = 0xff000000; // start with a mask for the highest byte.
	uint32_t shift = 256*256*256; // start with a shift for the highest byte.
	uint32_t byte, byte_iterator, bit_iterator;
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
