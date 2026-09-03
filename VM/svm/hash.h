/*
 * SVM - generic hash table (open addressing, linear probing).
 *
 * JamVM specialised its table per call-site with function-like macros
 * (HASH/COMPARE/PREPARE...).  SVM instead uses a single real function that
 * takes callbacks, in line with the "inline/functions over macros" style.
 * Sizes are powers of two so the modulo is a bit-mask.
 */

#ifndef SVM_HASH_H
#define SVM_HASH_H

#include "thread.h"

typedef struct hash_entry {
    int   hash;
    void *data;
} HashEntry;

typedef struct hash_table {
    HashEntry *entries;
    int        size;    /* power of two */
    int        count;
    VMLock     lock;
} HashTable;

/* key   : the lookup key supplied by the caller
 * data  : an entry already stored in the table
 * Returns non-zero if `key` matches `data`. */
typedef int   (*HashFn)(void *key);
typedef int   (*CompareFn)(void *key, void *data, int key_hash, int data_hash);
typedef void *(*PrepareFn)(void *key);      /* key -> value to store */
typedef void  (*IterateFn)(void *data);

extern void  initHashTable(HashTable *table, int initial_size);
extern void  resizeHash(HashTable *table, int new_size);

/* Find `key`; if absent and add_if_absent, insert prepare(key).
 * Returns the stored data (or NULL if absent and not adding). */
extern void *findHashEntry(HashTable *table, void *key, HashFn hash,
                           CompareFn cmp, PrepareFn prepare, int add_if_absent);

extern void  hashIterate(HashTable *table, IterateFn fn);

#endif /* SVM_HASH_H */
