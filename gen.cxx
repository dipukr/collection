#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <sys/stat.h>

int main(const int argc, const char **argv)
{
	FILE *in_stream = fopen("/home/dkumar/RESEARCH/untitled", "rb");
	FILE *out_stream = fopen("/home/dkumar/RESEARCH/rad", "wb");
	
	fseek(in_stream, 0, SEEK_END);
	size_t untitled_sz = (size_t) ftell(in_stream);
	fseek(in_stream, 0, SEEK_SET);

	uint8_t *untitled_bytes = new uint8_t[untitled_sz];
	fread(untitled_bytes, untitled_sz, 1, in_stream);
	fclose(in_stream);

	const char *str = "life_is_beautiful.";
	size_t str_sz = strlen(str);
	
	fwrite(untitled_bytes, untitled_sz, 1, out_stream);
	fwrite(str, str_sz, 1, out_stream);
	fwrite((uint8_t*) &untitled_sz, sizeof(untitled_sz), 1, out_stream);
	
	fclose(out_stream);
	chmod("/home/dkumar/RESEARCH/rad", strtol("777", nullptr, 8));

	printf("File generated successfully.\n");
	return EXIT_SUCCESS;
}