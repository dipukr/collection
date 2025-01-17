#include <cstdio>
#include <cstdlib>
#include <cstdint>

int main(const int argc, const char **argv)
{
	FILE *in_stream = fopen(argv[0], "rb");

	fseek(in_stream, 0, SEEK_END);
	size_t file_sz = (size_t) ftell(in_stream);
	
	printf("file_sz=%d\n", file_sz);
	
	fseek(in_stream, file_sz - 8, SEEK_SET);
	size_t exe_sz = 0;
	fread((uint8_t*) &exe_sz, sizeof(exe_sz), 1, in_stream);
	
	printf("exe_sz=%d\n", exe_sz);

	size_t str_sz = file_sz - exe_sz - 8;
	printf("str_sz=%d\n", str_sz);
	
	fseek(in_stream, exe_sz, SEEK_SET);
	
	char *str = new char[str_sz + 1];
	fread(str, str_sz, 1, in_stream);
	str[str_sz] = 0;
	
	printf("%s\n", str);
	fclose(in_stream);

	return EXIT_SUCCESS;
}