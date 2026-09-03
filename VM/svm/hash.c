/*
 * SVM - generic hash table implementation.
 */

#include <stdlib.h>
#include <string.h>
#include "hash.h"

void initHashTable(HashTable *table, int initial_size) {
    table->entries = (HashEntry *)calloc(initial_size, sizeof(HashEntry));
    table->size    = initial_size;
    table->count   = 0;
    initVMLock(&table->lock);
}

void resizeHash(HashTable *table, int new_size) {
    HashEntry *new_entries = (HashEntry *)calloc(new_size, sizeof(HashEntry));
    int i;

    for(i = 0; i < table->size; i++) {
        void *data = table->entries[i].data;
        if(data != NULL) {
            int hash = table->entries[i].hash;
            int j = hash & (new_size - 1);
            while(new_entries[j].data != NULL)
                j = (j + 1) & (new_size - 1);
            new_entries[j].hash = hash;
            new_entries[j].data = data;
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->size    = new_size;
}

void *findHashEntry(HashTable *table, void *key, HashFn hash_fn,
                    CompareFn cmp, PrepareFn prepare, int add_if_absent) {
    Thread *self;
    int hash = hash_fn(key);
    int i;
    void *data;

    disableSuspend(self = threadSelf());
    lockVMLock(&table->lock, self);

    i = hash & (table->size - 1);
    for(;;) {
        data = table->entries[i].data;
        if(data == NULL || cmp(key, data, hash, table->entries[i].hash))
            break;
        i = (i + 1) & (table->size - 1);
    }

    if(data == NULL && add_if_absent) {
        table->entries[i].hash = hash;
        data = table->entries[i].data = prepare(key);
        table->count++;

        /* keep load factor below 3/4 */
        if(table->count * 4 > table->size * 3)
            resizeHash(table, table->size * 2);
    }

    unlockVMLock(&table->lock, self);
    enableSuspend(self);
    return data;
}

void hashIterate(HashTable *table, IterateFn fn) {
    int i;
    for(i = table->size - 1; i >= 0; i--) {
        void *data = table->entries[i].data;
        if(data != NULL)
            fn(data);
    }
}
