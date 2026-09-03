/*
 * SVM - modified-UTF-8 handling and a de-duplicating UTF-8 pool.
 *
 * All UTF-8 strings that come out of the constant pool are interned here so
 * that class/field/method names can be compared by pointer identity.
 */

#include <stdlib.h>
#include <string.h>
#include "svm.h"
#include "hash.h"

static HashTable utf8_table;

/* Decode one modified-UTF-8 character, advancing *pntr. */
static int getUtf8Char(const unsigned char **pntr) {
    const unsigned char *p = *pntr;
    int x = *p++;
    if(x < 0x80) {                       /* 1 byte */
        *pntr = p;
        return x;
    }
    if((x & 0xe0) == 0xc0) {             /* 2 bytes */
        int y = *p++;
        *pntr = p;
        return ((x & 0x1f) << 6) | (y & 0x3f);
    }
    /* 3 bytes */
    {
        int y = *p++;
        int z = *p++;
        *pntr = p;
        return ((x & 0xf) << 12) | ((y & 0x3f) << 6) | (z & 0x3f);
    }
}

/* Number of UTF-16 code units in a modified-UTF-8 C string. */
int utf8Len(const char *utf8) {
    const unsigned char *p = (const unsigned char *)utf8;
    int count = 0;
    while(*p) {
        getUtf8Char(&p);
        count++;
    }
    return count;
}

/* Decode a modified-UTF-8 string into a caller-supplied 16-bit buffer. */
void convertUtf8(const char *utf8, short *buff) {
    const unsigned char *p = (const unsigned char *)utf8;
    while(*p)
        *buff++ = (short)getUtf8Char(&p);
}

/* '/' -> '.' for display of class names.  Returns a malloc'd copy. */
char *slash2dots(const char *utf8) {
    char *copy = strdup(utf8);
    char *p;
    for(p = copy; *p; p++)
        if(*p == '/') *p = '.';
    return copy;
}

/* ---- interning ---------------------------------------------------------- */

static int utf8Hash(void *key) {
    const char *s = (const char *)key;
    int hash = 0;
    while(*s)
        hash = hash * 37 + (unsigned char)*s++;
    return hash;
}

static int utf8Compare(void *key, void *data, int kh, int dh) {
    return kh == dh && strcmp((const char *)key, (const char *)data) == 0;
}

static void *utf8Prepare(void *key) {
    return strdup((const char *)key);   /* owned copy stored in table */
}

char *internUtf8(const char *s) {
    return (char *)findHashEntry(&utf8_table, (void *)s,
                                 utf8Hash, utf8Compare, utf8Prepare, TRUE);
}

void initialiseUtf8(void) {
    initHashTable(&utf8_table, 1 << 10);
}
