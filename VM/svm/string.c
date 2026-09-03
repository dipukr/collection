/*
 * SVM - java.lang.String equivalent: saral/lang/String.
 *
 * Contract expected of the core library:
 *   class saral/lang/String {
 *       char[] value;   // field "value", type "[C"
 *       int    count;   // field "count", type "I"
 *   }
 * createString() builds instances directly (without running a constructor),
 * exactly as JamVM's createString does.  String literals (OP_LDC of a
 * CONSTANT_String) are interned.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svm.h"
#include "hash.h"

extern int   utf8Len(const char *utf8);
extern void  convertUtf8(const char *utf8, short *buff);

static Class     *string_class;
static int        value_offset;
static int        count_offset;
static HashTable  intern_table;

static short *stringChars(Object *string) {
    Object *value = (Object *)inst_data(string)[value_offset].o;
    return (short *)array_body(value);
}
static int stringLen(Object *string) {
    return (int)inst_data(string)[count_offset].i;
}

Object *createString(const char *utf8) {
    int len = utf8Len(utf8);
    Object *value = allocTypeArray(T_CHAR, len);
    Object *string;

    if(value == NULL) return NULL;
    convertUtf8(utf8, (short *)array_body(value));

    string = allocObject(string_class);
    if(string == NULL) return NULL;

    inst_data(string)[count_offset].i = len;
    inst_data(string)[value_offset].o = value;
    return string;
}

Object *cstr2String(const char *cstr) {
    return createString(cstr);
}

char *string2Cstr(Object *string) {
    int len = stringLen(string);
    short *chars = stringChars(string);
    char *cstr = (char *)malloc(len + 1);
    int i;
    for(i = 0; i < len; i++)
        cstr[i] = (char)chars[i];
    cstr[len] = '\0';
    return cstr;
}

/* ---- interning ---------------------------------------------------------- */

static int stringHash(void *key) {
    Object *s = (Object *)key;
    int len = stringLen(s);
    short *chars = stringChars(s);
    int hash = 0, i;
    for(i = 0; i < len; i++)
        hash = hash * 31 + chars[i];
    return hash;
}

static int stringCompare(void *key, void *data, int kh, int dh) {
    Object *a = (Object *)key, *b = (Object *)data;
    int len;
    if(kh != dh) return FALSE;
    len = stringLen(a);
    if(len != stringLen(b)) return FALSE;
    return memcmp(stringChars(a), stringChars(b), len * sizeof(short)) == 0;
}

static void *stringPrepare(void *key) { return key; }

Object *findInternedString(Object *string) {
    return (Object *)findHashEntry(&intern_table, string,
                                   stringHash, stringCompare, stringPrepare, TRUE);
}

void markInternedStrings(void) {
    hashIterate(&intern_table, (IterateFn)markObject);
}

void initialiseString(void) {
    FieldBlock *value_fb, *count_fb;

    string_class = findSystemClass("saral/lang/String");
    if(string_class == NULL) {
        printException();
        exit(1);
    }
    value_fb = findField(string_class, "value", "[C");
    count_fb = findField(string_class, "count", "I");
    if(value_fb == NULL || count_fb == NULL) {
        fprintf(stderr, "Malformed saral/lang/String\n");
        exit(1);
    }
    value_offset = value_fb->offset;
    count_offset = count_fb->offset;

    initHashTable(&intern_table, 1 << 8);
}
