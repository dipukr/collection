/*
 * SVM - threading primitives and Thread / Monitor structures.
 * Modeled on JamVM's thread.h: native pthreads, condition-variable wait
 * locks, and signal-based suspension for stop-the-world GC.
 */

#ifndef SVM_THREAD_H
#define SVM_THREAD_H

#include <pthread.h>
#include <setjmp.h>
#include <alloca.h>
#include "svm.h"

/* Thread states.  Ordering matters: systemIdle() treats state < WAITING as
 * "possibly running work". */
#define THREAD_CREATING  0
#define THREAD_STARTED   1
#define THREAD_RUNNING   2
#define THREAD_WAITING   3
#define THREAD_SUSPENDED 5

typedef struct thread Thread;

typedef struct monitor {
    pthread_mutex_t lock;
    pthread_cond_t  cv;
    Thread         *owner;
    int             count;         /* recursion count */
    int             waiting;
    int             notifying;
    int             interrupting;
    int             entering;
    struct monitor *next;          /* free-list link */
    char            in_use;
} Monitor;

struct thread {
    char        state;
    char        interrupted;
    char        interrupting;
    char        suspend;           /* suspension requested */
    char        blocking;          /* in a native blocking region (safe point) */
    pthread_t   tid;
    int         id;                /* small dense id, used in thin lockword */
    ExecEnv    *ee;
    void       *stack_top;         /* captured at suspend for conservative scan */
    void       *stack_base;
    Monitor    *wait_mon;
    Object     *thread_obj;
    void      (*start)(struct thread *); /* VM-internal thread entry */
    Thread     *prev, *next;
};

extern Thread *threadSelf(void);
extern void    setThreadSelf(Thread *thread);
extern void   *getStackTop(Thread *thread);
extern void   *getStackBase(Thread *thread);
extern void    threadInterrupt(Thread *thread);
extern void    threadSleep(Thread *thread, long long ms, int ns);
extern int     systemIdle(Thread *self);
extern void    disableSuspend0(Thread *thread, void *stack_top);
extern void    enableSuspend(Thread *thread);

/* Record the current C-stack top (so a suspended thread's registers/locals
 * are conservatively scannable) then mark ourselves un-suspendable.
 *
 * This is the ONE place SVM must use a macro rather than an inline function:
 * the sigjmp_buf (into which sigsetjmp flushes the live registers) has to live
 * in the *caller's* stack frame so that it, and the stack_top it yields, stay
 * valid after this returns and while the caller subsequently blocks on a
 * native lock.  alloca() in a macro guarantees that; an inline function does
 * not.  (JamVM does exactly this.) */
#define disableSuspend(thread)                        \
    do {                                              \
        sigjmp_buf *_env = alloca(sizeof(sigjmp_buf));\
        sigsetjmp(*_env, FALSE);                      \
        disableSuspend0((thread), (void *)_env);      \
    } while(0)

/* ---- Simple VM-internal locks (not exposed to Saral code) --------------- */

typedef pthread_mutex_t VMLock;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t  cv;
} VMWaitLock;

static inline void initVMLock(VMLock *lock)         { pthread_mutex_init(lock, NULL); }
static inline void initVMWaitLock(VMWaitLock *wl)   {
    pthread_mutex_init(&wl->lock, NULL);
    pthread_cond_init(&wl->cv, NULL);
}
static inline void lockVMLock(VMLock *lock, Thread *self) {
    self->state = THREAD_WAITING;
    pthread_mutex_lock(lock);
    self->state = THREAD_RUNNING;
}
static inline void unlockVMLock(VMLock *lock, Thread *self) {
    (void)self;
    pthread_mutex_unlock(lock);
}
static inline void lockVMWaitLock(VMWaitLock *wl, Thread *self)   { lockVMLock(&wl->lock, self); }
static inline void unlockVMWaitLock(VMWaitLock *wl, Thread *self) { unlockVMLock(&wl->lock, self); }
static inline void waitVMWaitLock(VMWaitLock *wl, Thread *self) {
    self->state = THREAD_WAITING;
    pthread_cond_wait(&wl->cv, &wl->lock);
    self->state = THREAD_RUNNING;
}
static inline void notifyVMWaitLock(VMWaitLock *wl, Thread *self) {
    (void)self;
    pthread_cond_signal(&wl->cv);
}
static inline void notifyAllVMWaitLock(VMWaitLock *wl, Thread *self) {
    (void)self;
    pthread_cond_broadcast(&wl->cv);
}

#endif /* SVM_THREAD_H */
