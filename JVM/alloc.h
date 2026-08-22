#define LOG_OBJECT_GRAIN	3
#define HEADER_SIZE		4
#define FLC_BIT			2

#define clear_flc_bit(o) { \
	unsigned int *hdr = (unsigned int*)(((char*)o)-HEADER_SIZE); \
        *hdr  &= ~FLC_BIT; \
}

#define set_flc_bit(o) { \
	unsigned int *hdr = (unsigned int*)(((char*)o)-HEADER_SIZE); \
        *hdr  |= FLC_BIT; \
}

#define test_flc_bit(o) *(unsigned int*)(((char*)o)-HEADER_SIZE) & FLC_BIT
