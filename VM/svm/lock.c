/*
 * SVM - object monitors and thin locks.
 *
 * Uncontended locking uses a single compare-and-swap on the object's lock
 * word (a "thin" lock holding the owner thread id and a recursion count).
 * On contention (or wait/notify) the lock is "inflated" to a heavyweight
 * Monitor (a pthread mutex + condition variable) whose pointer is stored in
 * the lock word with the shape bit set.  This is JamVM's scheme; for
 * simplicity SVM never deflates monitors (see README).
 *
 * A hash table (keyed by object address) hands every thread contending on
 * the same not-yet-inflated object the same Monitor during the thin->fat
 * transition.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <errno.h>
#include <sys/time.h>
#include "svm.h"
#include "thread.h"
#include "hash.h"
#include "heap.h"
#include "atomic.h"

/* lock-word layout:
 *   thin:  [ tid | count | 0 ]      (shape bit clear)
 *   fat:   [ Monitor*        | 1 ]  (shape bit set)
 */
#define SHAPE_BIT   ((uintptr_t)1)
#define COUNT_SHIFT 1
#define COUNT_SIZE  8
#define COUNT_MASK  ((((uintptr_t)1 << COUNT_SIZE) - 1) << COUNT_SHIFT)
#define COUNT_MAX   (COUNT_MASK)
#define TID_SHIFT   (COUNT_SHIFT + COUNT_SIZE)
#define TID_MASK    (~(uintptr_t)0 << TID_SHIFT)

static HashTable mon_cache;

/* ---- Monitor primitives ------------------------------------------------- */

void monitorInit(Monitor *mon) {
    pthread_mutex_init(&mon->lock, NULL);
    pthread_cond_init(&mon->cv, NULL);
    mon->owner = NULL;
    mon->count = mon->waiting = mon->notifying = mon->interrupting = mon->entering = 0;
    mon->in_use = TRUE;
}

void monitorLock(Monitor *mon, Thread *self) {
    if(mon->owner == self) {
        mon->count++;
        return;
    }
    mon->entering++;
    disableSuspend(self);
    self->state = THREAD_WAITING;
    pthread_mutex_lock(&mon->lock);
    self->state = THREAD_RUNNING;
    enableSuspend(self);
    mon->entering--;
    mon->owner = self;
}

static int monitorTryLock(Monitor *mon, Thread *self) {
    if(mon->owner == self) { mon->count++; return TRUE; }
    if(pthread_mutex_trylock(&mon->lock)) return FALSE;
    mon->owner = self;
    return TRUE;
}

void monitorUnlock(Monitor *mon, Thread *self) {
    if(mon->owner != self) return;
    if(mon->count == 0) {
        mon->owner = NULL;
        pthread_mutex_unlock(&mon->lock);
    } else
        mon->count--;
}

int monitorWait(Monitor *mon, Thread *self, long long ms, int ns) {
    char interrupted = FALSE;
    int old_count;
    char timed = (ms != 0) || (ns != 0);
    struct timespec ts;

    if(mon->owner != self)
        return FALSE;

    disableSuspend(self);
    old_count = mon->count;
    mon->count = 0;
    mon->owner = NULL;
    mon->waiting++;

    if(timed) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        ts.tv_sec  = tv.tv_sec + ms / 1000;
        ts.tv_nsec = (tv.tv_usec + (ms % 1000) * 1000) * 1000 + ns;
        if(ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
    }

    self->wait_mon = mon;
    self->state = THREAD_WAITING;

    if(self->interrupted)
        interrupted = TRUE;
    else
        for(;;) {
            if(timed) {
                if(pthread_cond_timedwait(&mon->cv, &mon->lock, &ts) == ETIMEDOUT)
                    break;
            } else
                pthread_cond_wait(&mon->cv, &mon->lock);

            if(self->interrupting) {
                interrupted = TRUE;
                self->interrupting = FALSE;
                mon->interrupting--;
                break;
            }
            if(mon->notifying) { mon->notifying--; break; }
        }

    self->state = THREAD_RUNNING;
    self->wait_mon = NULL;
    mon->owner = self;
    mon->count = old_count;
    mon->waiting--;
    enableSuspend(self);

    if(interrupted) {
        self->interrupted = FALSE;
        signalException("saral/lang/InterruptedException", NULL);
    }
    return TRUE;
}

int monitorNotify(Monitor *mon, Thread *self) {
    if(mon->owner != self) return FALSE;
    if(mon->notifying + mon->interrupting < mon->waiting) {
        mon->notifying++;
        pthread_cond_signal(&mon->cv);
    }
    return TRUE;
}

int monitorNotifyAll(Monitor *mon, Thread *self) {
    if(mon->owner != self) return FALSE;
    mon->notifying = mon->waiting - mon->interrupting;
    pthread_cond_broadcast(&mon->cv);
    return TRUE;
}

/* ---- monitor cache (object -> Monitor) ----------------------------------
 * The table is keyed on the object address.  A 64-bit address does not fit in
 * the table's int hash, so two objects can share a hash bucket; therefore the
 * comparator must test the real object identity, not just the hash.  We keep
 * that identity by storing the object pointer next to the Monitor.  (Monitors
 * are never freed - SVM does not deflate - so the Monitor pointer parked in an
 * inflated lock word stays valid.) */

typedef struct { Object *obj; Monitor mon; } MonNode;

static int monHash(void *key)  { return (int)((uintptr_t)key >> 3 /*log2 grain*/); }
static int monCompare(void *key, void *data, int kh, int dh) {
    return kh == dh && ((MonNode *)data)->obj == (Object *)key;
}
static void *monPrepare(void *key) {
    MonNode *node = (MonNode *)malloc(sizeof(MonNode));
    node->obj = (Object *)key;
    monitorInit(&node->mon);
    return node;
}

static Monitor *findMonitor(Object *obj) {
    uintptr_t lockword = obj->lock;
    MonNode *node;
    if(lockword & SHAPE_BIT)
        return (Monitor *)(lockword & ~SHAPE_BIT);
    node = (MonNode *)findHashEntry(&mon_cache, obj, monHash, monCompare, monPrepare, TRUE);
    return &node->mon;
}

static void inflate(Object *obj, Monitor *mon, Thread *self) {
    clearFlcBit(obj);
    monitorNotifyAll(mon, self);
    obj->lock = (uintptr_t)mon | SHAPE_BIT;
}

/* ---- object-level locking ---------------------------------------------- */

void objectLock(Object *obj) {
    Thread *self = threadSelf();
    uintptr_t thin = (uintptr_t)self->id << TID_SHIFT;
    Monitor *mon;

    if(compareAndSwap(&obj->lock, 0, thin))
        return;

    if((obj->lock & (TID_MASK | SHAPE_BIT)) == thin) {
        /* recursive thin lock */
        if((obj->lock & COUNT_MASK) < COUNT_MAX)
            obj->lock += (uintptr_t)1 << COUNT_SHIFT;
        else {
            mon = findMonitor(obj);
            monitorLock(mon, self);
            inflate(obj, mon, self);
            mon->count = (uintptr_t)1 << COUNT_SIZE;
        }
        return;
    }

    /* contended: get the shared monitor and spin/inflate */
    mon = findMonitor(obj);
    monitorLock(mon, self);
    while((obj->lock & SHAPE_BIT) == 0) {
        setFlcBit(obj);
        if(compareAndSwap(&obj->lock, 0, (uintptr_t)self))
            inflate(obj, mon, self);
        else
            monitorWait(mon, self, 0, 0);
    }
}

void objectUnlock(Object *obj) {
    Thread *self = threadSelf();
    uintptr_t thin = (uintptr_t)self->id << TID_SHIFT;

    if(obj->lock == thin) {
        /* releasing an uncontended thin lock */
        obj->lock = 0;
        if(testFlcBit(obj)) {
            Monitor *mon = findMonitor(obj);
            if(monitorTryLock(mon, self)) {
                if(testFlcBit(obj))
                    monitorNotify(mon, self);
                monitorUnlock(mon, self);
            }
        }
    } else if((obj->lock & (TID_MASK | SHAPE_BIT)) == thin) {
        obj->lock -= (uintptr_t)1 << COUNT_SHIFT;   /* recursive thin */
    } else if(obj->lock & SHAPE_BIT) {
        Monitor *mon = (Monitor *)(obj->lock & ~SHAPE_BIT);
        monitorUnlock(mon, self);
    }
}

static Monitor *ownedMonitor(Object *obj, Thread *self) {
    uintptr_t lockword = obj->lock;
    Monitor *mon;
    if(lockword & SHAPE_BIT) {
        mon = (Monitor *)(lockword & ~SHAPE_BIT);
        if(mon->owner == self) return mon;
        return NULL;
    }
    if(((lockword & TID_MASK) >> TID_SHIFT) == (uintptr_t)self->id) {
        /* inflate so wait/notify have a real monitor */
        mon = findMonitor(obj);
        monitorLock(mon, self);
        mon->count = (lockword & COUNT_MASK) >> COUNT_SHIFT;
        inflate(obj, mon, self);
        return mon;
    }
    return NULL;
}

void objectWait(Object *obj, long long ms, int ns) {
    Thread *self = threadSelf();
    Monitor *mon = ownedMonitor(obj, self);
    if(mon == NULL) {
        signalException("saral/lang/IllegalMonitorStateException", "not owner");
        return;
    }
    monitorWait(mon, self, ms, ns);
}

void objectNotify(Object *obj) {
    Thread *self = threadSelf();
    Monitor *mon = ownedMonitor(obj, self);
    if(mon == NULL)
        signalException("saral/lang/IllegalMonitorStateException", "not owner");
    else
        monitorNotify(mon, self);
}

void objectNotifyAll(Object *obj) {
    Thread *self = threadSelf();
    Monitor *mon = ownedMonitor(obj, self);
    if(mon == NULL)
        signalException("saral/lang/IllegalMonitorStateException", "not owner");
    else
        monitorNotifyAll(mon, self);
}

void initialiseMonitor(void) {
    initHashTable(&mon_cache, 1 << 5);
}
