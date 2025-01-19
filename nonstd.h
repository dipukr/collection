#ifndef NONSTD_H
#define NONSTD_H

#include <stdint.h>
#include <time.h>

#define null NULL
#define KB 1024
#define MB KB*KB
#define GB KB*KB*KB

#define eval(x) time_t start = clock();\
    x;\
    time_t end = clock();\
    printf("Time elapsed: %d micros.\n", (end - start));

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;
typedef uint32_t uint;
typedef float dec32;
typedef double dec64;

#endif