/*
 * SVM - heap layout constants and object-header bit helpers.
 *
 * Every heap block is preceded by a one-word header:
 *
 *   bits: [ ...... block size (8-aligned) ...... | flc | alloc ]
 *                                                   ^b1    ^b0
 *
 * The FLC ("flush/contended") bit is used by the thin-lock code in lock.c to
 * record that some thread is waiting on a currently thin-locked object.
 */

#ifndef SVM_HEAP_H
#define SVM_HEAP_H

#include <stdint.h>

#define OBJECT_GRAIN 8
#define HEADER_SIZE  ((int)sizeof(uintptr_t))
#define ALLOC_BIT    1
#define FLC_BIT      2

static inline uintptr_t *objectHeader(void *ob) {
    return (uintptr_t *)((char *)ob - HEADER_SIZE);
}
static inline void setFlcBit(void *ob)   { *objectHeader(ob) |=  FLC_BIT; }
static inline void clearFlcBit(void *ob) { *objectHeader(ob) &= ~FLC_BIT; }
static inline int  testFlcBit(void *ob)  { return (*objectHeader(ob) & FLC_BIT) != 0; }

#endif /* SVM_HEAP_H */
