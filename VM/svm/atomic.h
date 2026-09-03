/*
 * SVM - atomic compare-and-swap.
 *
 * JamVM used hand-written per-architecture assembly (i386 `cmpxchgl`,
 * PowerPC `lwarx`/`stwcx.`) in lock_md.h.  SVM instead uses the portable gcc
 * atomic builtins, which compile to the same instructions and work on both
 * 32- and 64-bit targets.  This keeps the whole VM architecture-independent.
 */

#ifndef SVM_ATOMIC_H
#define SVM_ATOMIC_H

#include <stdint.h>

/* Returns TRUE if *addr == old and was replaced with new_val. */
static inline int compareAndSwap(volatile uintptr_t *addr,
                                 uintptr_t old_val, uintptr_t new_val) {
    return __sync_bool_compare_and_swap(addr, old_val, new_val);
}

#endif /* SVM_ATOMIC_H */
