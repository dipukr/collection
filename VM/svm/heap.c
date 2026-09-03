/*
 * SVM - mark & sweep garbage collector and object allocation.
 *
 * A single contiguous heap is reserved with mmap and carved up with an
 * address-ordered free list.  Collection is stop-the-world: all other
 * threads are suspended (via signals, see thread.c), the reachable graph is
 * marked recursively from the roots, then the heap is swept and the free
 * list rebuilt.  This mirrors JamVM's alloc.c, adapted to 64-bit slots and
 * inline helpers.  (Finalizers are intentionally omitted - see README.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "svm.h"
#include "heap.h"
#include "thread.h"

extern void suspendAllThreads(Thread *self);
extern void resumeAllThreads(Thread *self);

#define HDR_SIZE(hdr)    ((hdr) & ~(uintptr_t)(ALLOC_BIT | FLC_BIT))
#define HDR_ALLOCED(hdr) ((hdr) & ALLOC_BIT)

typedef struct chunk {
    uintptr_t     header;
    struct chunk *next;
} Chunk;

static Chunk  *freelist;
static char   *heapbase;
static char   *heaplimit;
static char   *heapmax;
static long    heapfree;

static unsigned int *markBits;
static int           markBitSize;

static VMLock heap_lock;
static int    verbosegc;

static Class *oom;   /* pre-allocated OutOfMemoryError */

/* ---- mark bitmap -------------------------------------------------------- */

static inline int markEntry(void *ptr) {
    return (int)(((char *)ptr - heapbase) >> (3 /*log grain*/ + 5 /*log 32*/));
}
static inline int markOffset(void *ptr) {
    return (int)((((char *)ptr - heapbase) >> 3) & 31);
}
static inline void markBit(void *ptr) {
    markBits[markEntry(ptr)] |= 1u << markOffset(ptr);
}
static inline int isMarked(void *ptr) {
    return (markBits[markEntry(ptr)] & (1u << markOffset(ptr))) != 0;
}

static void allocMarkBits(void) {
    int no_of_bits = (heaplimit - heapbase) >> 3;
    markBitSize = (no_of_bits + 31) >> 5;
    markBits = (unsigned int *)malloc(markBitSize * sizeof(unsigned int));
}
static void clearMarkBits(void) {
    memset(markBits, 0, markBitSize * sizeof(unsigned int));
}

/* Conservative test: does `ptr` look like a heap object reference? */
int isObjectRef(void *ptr) {
    return (char *)ptr > heapbase
        && (char *)ptr < heaplimit
        && (((uintptr_t)ptr & (OBJECT_GRAIN - 1)) == 0);
}

void initialiseAlloc(size_t min, size_t max, int verbose) {
    char *mem = (char *)mmap(0, max, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(mem == MAP_FAILED) {
        fprintf(stderr, "Couldn't allocate the heap.  Aborting.\n");
        exit(1);
    }

    /* align so that heapbase + HEADER_SIZE is object-aligned */
    heapbase = (char *)(((uintptr_t)mem + HEADER_SIZE + OBJECT_GRAIN - 1)
                        & ~(uintptr_t)(OBJECT_GRAIN - 1)) - HEADER_SIZE;
    heaplimit = heapbase + ((min - (heapbase - mem)) & ~(uintptr_t)(OBJECT_GRAIN - 1));
    heapmax   = heapbase + ((max - (heapbase - mem)) & ~(uintptr_t)(OBJECT_GRAIN - 1));

    freelist = (Chunk *)heapbase;
    freelist->header = heapfree = heaplimit - heapbase;
    freelist->next = NULL;

    allocMarkBits();
    initVMLock(&heap_lock);
    verbosegc = verbose;
}

/* ---- marking ------------------------------------------------------------ */

void markObject(Object *ob) {
    if(isObjectRef(ob) && !isMarked(ob))
        markChildren(ob);
}
void markClass(Class *class) {
    markChildren((Object *)class);
}

static void markStatics(Class *class) {
    ClassBlock *cb = class_cb(class);
    int i;
    for(i = 0; i < cb->fields_count; i++) {
        FieldBlock *fb = &cb->fields[i];
        if((fb->access_flags & ACC_STATIC) &&
           (fb->type[0] == 'L' || fb->type[0] == '[')) {
            Object *ref = (Object *)fb->static_value.o;
            if(isObjectRef(ref) && !isMarked(ref))
                markChildren(ref);
        }
    }
}

void markChildren(Object *ob) {
    ClassBlock *cb;

    if(!isObjectRef(ob) || isMarked(ob))
        return;
    markBit(ob);

    if(ob->class == NULL)
        return;

    /* a Class object: mark its static reference fields */
    if(ob->class == svm_class_Class) {
        markStatics((Class *)ob);
        return;
    }

    cb = class_cb(ob->class);

    if(cb->name[0] == '[') {
        /* array: only object-element arrays contain references */
        if(cb->name[1] == 'L' || cb->name[1] == '[') {
            int len = array_length(ob);
            Slot *body = (Slot *)array_body(ob);
            int i;
            for(i = 0; i < len; i++) {
                Object *ref = (Object *)body[i].o;
                if(isObjectRef(ref) && !isMarked(ref))
                    markChildren(ref);
            }
        }
    } else {
        /* ordinary object: walk ref fields across the superclass chain */
        Class *c = ob->class;
        Slot *body = inst_data(ob);
        while(c != NULL) {
            ClassBlock *ccb = class_cb(c);
            int i;
            for(i = 0; i < ccb->fields_count; i++) {
                FieldBlock *fb = &ccb->fields[i];
                if(!(fb->access_flags & ACC_STATIC) &&
                   (fb->type[0] == 'L' || fb->type[0] == '[')) {
                    Object *ref = (Object *)body[fb->offset].o;
                    if(isObjectRef(ref) && !isMarked(ref))
                        markChildren(ref);
                }
            }
            c = ccb->super;
        }
    }
}

static void doMark(Thread *self) {
    (void)self;
    clearMarkBits();
    if(oom) markBit(oom);
    markClasses();
    markInternedStrings();
    scanThreads();
}

/* Rebuild the free list; returns size of the largest free chunk. */
static long doSweep(Thread *self) {
    char *ptr;
    Chunk newlist;
    Chunk *last = &newlist;
    long largest = 0;
    int marked = 0, unmarked = 0;
    (void)self;

    heapfree = 0;

    for(ptr = heapbase; ptr < heaplimit; ) {
        uintptr_t hdr = *(uintptr_t *)ptr;
        long size = HDR_SIZE(hdr);

        if(HDR_ALLOCED(hdr)) {
            Object *ob = (Object *)(ptr + HEADER_SIZE);
            if(isMarked(ob)) { marked++; ptr += size; continue; }
            unmarked++;
        }

        /* start a run of free space here and merge following free blocks */
        last->next = (Chunk *)ptr;
        last = last->next;
        last->header &= ~(uintptr_t)(ALLOC_BIT | FLC_BIT);

        for(;;) {
            ptr += size;
            if(ptr >= heaplimit) goto done;
            hdr  = *(uintptr_t *)ptr;
            size = HDR_SIZE(hdr);
            if(HDR_ALLOCED(hdr)) {
                Object *ob = (Object *)(ptr + HEADER_SIZE);
                if(isMarked(ob)) break;
                unmarked++;
            }
            last->header += size;
        }

        if((long)last->header > largest) largest = last->header;
        heapfree += last->header;
        marked++;
        ptr += size;   /* skip the marked block that ended the run */
        if(ptr >= heaplimit) goto out;
    }

done:
    if((long)last->header > largest) largest = last->header;
    heapfree += last->header;
out:
    last->next = NULL;
    freelist = newlist.next;

    if(verbosegc)
        printf("<GC: live=%d freed=%d free=%ld/%ld largest=%ld>\n",
               marked, unmarked, heapfree, (long)(heaplimit - heapbase), largest);
    return largest;
}

static long gc0(Thread *self) {
    long largest;
    suspendAllThreads(self);
    doMark(self);
    largest = doSweep(self);
    resumeAllThreads(self);
    return largest;
}

int gc(void) {
    Thread *self;
    disableSuspend(self = threadSelf());
    lockVMLock(&heap_lock, self);
    gc0(self);
    unlockVMLock(&heap_lock, self);
    enableSuspend(self);
    return 0;
}

static void expandHeap(long min) {
    Chunk *chunk, *new_chunk;
    long delta = (heaplimit - heapbase) / 2;

    if(delta < min) delta = min;
    if(heaplimit + delta > heapmax) delta = heapmax - heaplimit;
    delta &= ~(uintptr_t)(OBJECT_GRAIN - 1);

    if(delta == 0) return;
    if(verbosegc) printf("<GC: expanding heap by %ld bytes>\n", delta);

    for(chunk = freelist; chunk && chunk->next != NULL; chunk = chunk->next);

    new_chunk = (Chunk *)heaplimit;
    new_chunk->header = delta;
    new_chunk->next = NULL;
    if(chunk) chunk->next = new_chunk; else freelist = new_chunk;

    heaplimit += delta;
    heapfree  += delta;
    free(markBits);
    allocMarkBits();
}

/* Carve `n` bytes out of the free list; returns the payload pointer or NULL
 * if no chunk is large enough. */
static void *carve(long n) {
    Chunk **chunkpp;
    for(chunkpp = &freelist; *chunkpp; chunkpp = &(*chunkpp)->next) {
        Chunk *found = *chunkpp;
        long clen = HDR_SIZE(found->header);
        void *ret;

        if(clen < n)
            continue;

        if(clen == n) {
            *chunkpp = found->next;
        } else {
            Chunk *rem = (Chunk *)((char *)found + n);
            rem->header = clen - n;
            rem->next = found->next;
            *chunkpp = rem;
        }
        found->header = n | ALLOC_BIT;
        heapfree -= n;
        ret = (char *)found + HEADER_SIZE;
        memset(ret, 0, n - HEADER_SIZE);
        return ret;
    }
    return NULL;
}

/* Core allocator.  `len` is the payload size (excluding the header). */
static void *gcMalloc(long len) {
    long n = (len + HEADER_SIZE + OBJECT_GRAIN - 1) & ~(uintptr_t)(OBJECT_GRAIN - 1);
    Thread *self;
    void *ret;

    disableSuspend(self = threadSelf());
    lockVMLock(&heap_lock, self);

    /* fast path */
    if((ret = carve(n)) == NULL) {
        /* try a collection, then expansion, then give up */
        if(gc0(self) >= n)
            ret = carve(n);
        while(ret == NULL && heaplimit < heapmax) {
            expandHeap(n);
            ret = carve(n);
        }
    }

    unlockVMLock(&heap_lock, self);
    enableSuspend(self);

    if(ret == NULL) {
        if(oom) setException((Object *)oom);
        else    signalException("saral/lang/OutOfMemoryError", NULL);
    }
    return ret;
}

/* ---- public allocation entry points ------------------------------------ */

Class *allocClass(void) {
    Class *class = (Class *)gcMalloc(sizeof(ClassBlock) + sizeof(Class));
    return class;
}

Object *allocObject(Class *class) {
    ClassBlock *cb = class_cb(class);
    long size = (long)cb->object_size * sizeof(Slot);
    Object *ob = (Object *)gcMalloc(size + sizeof(Object));
    if(ob != NULL)
        ob->class = class;
    return ob;
}

Object *allocArray(Class *class, int size, int el_size) {
    Object *ob;
    if(size < 0) {
        signalException("saral/lang/NegativeArraySizeException", NULL);
        return NULL;
    }
    /* header | Object | length(Slot) | size*el_size bytes */
    ob = (Object *)gcMalloc(sizeof(Object) + sizeof(Slot) + (long)size * el_size);
    if(ob != NULL) {
        ob->class = class;
        inst_data(ob)[0].i = size;
    }
    return ob;
}

Object *allocTypeArray(int type, int size) {
    Class *class;
    int el_size;
    switch(type) {
        case T_BOOL:  case T_BYTE:  case T_UBYTE: class = findArrayClass("[B"); el_size = 1; break;
        case T_CHAR:                              class = findArrayClass("[C"); el_size = 2; break;
        case T_INT:   case T_UINT:                class = findArrayClass("[I"); el_size = 4; break;
        case T_LONG:  case T_ULONG:               class = findArrayClass("[J"); el_size = 8; break;
        case T_NUM:                               class = findArrayClass("[D"); el_size = 8; break;
        default:
            fprintf(stderr, "Invalid array type %d - aborting.\n", type);
            exit(1);
    }
    if(class == NULL) return NULL;
    return allocArray(class, size, el_size);
}

Object *allocMultiArray(Class *array_class, int dim, int *counts) {
    int i, count = counts[0];
    Object *array;

    if(dim > 1) {
        Class *inner = findArrayClassFromClass(class_cb(array_class)->name + 1, array_class);
        array = allocArray(array_class, count, sizeof(Slot));
        if(array == NULL) return NULL;
        for(i = 0; i < count; i++)
            ((Slot *)array_body(array))[i].o = allocMultiArray(inner, dim - 1, counts + 1);
    } else {
        int el_size;
        switch(class_cb(array_class)->name[1]) {
            case 'B': case 'Z': el_size = 1; break;
            case 'C':           el_size = 2; break;
            case 'I':           el_size = 4; break;
            case 'J': case 'D': el_size = 8; break;
            default:            el_size = sizeof(Slot); break; /* refs */
        }
        array = allocArray(array_class, count, el_size);
    }
    return array;
}

Object *cloneObject(Object *ob) {
    uintptr_t hdr = *objectHeader(ob);
    long size = HDR_SIZE(hdr) - HEADER_SIZE;
    Object *clone = (Object *)gcMalloc(size);
    if(clone != NULL) {
        memcpy(clone, ob, size);
        clone->lock = 0;
    }
    return clone;
}

long freeHeapMem(void)  { return heapfree; }
long totalHeapMem(void) { return heaplimit - heapbase; }
long maxHeapMem(void)   { return heapmax - heapbase; }

void initialiseGC(int noasyncgc) {
    /* Pre-allocate an OutOfMemoryError so we can still throw when the heap is
     * exhausted.  (No async GC thread in this core build; parameter kept for
     * interface parity with JamVM.) */
    (void)noasyncgc;
    oom = (Class *)allocObject(findSystemClass("saral/lang/OutOfMemoryError"));
}
