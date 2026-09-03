/*
 * SVM - threading.
 *
 * Each Saral thread is a native POSIX thread.  Stop-the-world GC is achieved
 * exactly as in JamVM: the collector sends SIGUSR1 to every other thread; the
 * handler captures the thread's register/stack state with sigsetjmp and then
 * self-suspends in sigsuspend until released.  Threads about to block on a
 * native lock mark themselves "blocking" (a safe point) so they need not be
 * signalled.  This avoids any suspension polling on the interpreter hot path.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include "svm.h"
#include "thread.h"

#define SUSPEND_SIG SIGUSR1

extern Object *allocObject(Class *class);   /* from heap.c */

static pthread_key_t self_key;

static Thread   main_thread;
static ExecEnv  main_ee;
static Thread  *thread_list;          /* circular? no: NULL-terminated, head=main */
static VMLock   thread_lock;
static pthread_mutex_t exit_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  exit_cv   = PTHREAD_COND_INITIALIZER;
static int      non_daemon_thrds = 0;

/* thread-id bitmap */
static unsigned int *tid_bitmap;
static int           tid_bitmap_size;   /* in words */

static int java_stack_size;

/* ---- thread-id allocation ---------------------------------------------- */

static int genThreadID(void) {
    int i;
    for(i = 0; i < tid_bitmap_size; i++) {
        unsigned int w = tid_bitmap[i];
        if(w != 0xffffffffu) {
            int bit = __builtin_ffs((int)~w) - 1;
            tid_bitmap[i] |= 1u << bit;
            return i * 32 + bit + 1;   /* ids start at 1 */
        }
    }
    /* grow */
    tid_bitmap = realloc(tid_bitmap, (tid_bitmap_size + 1) * sizeof(unsigned int));
    tid_bitmap[tid_bitmap_size] = 1;
    return (tid_bitmap_size++) * 32 + 1;
}

static void freeThreadID(int id) {
    id--;
    tid_bitmap[id / 32] &= ~(1u << (id % 32));
}

/* ---- self / accessors --------------------------------------------------- */

Thread *threadSelf(void)          { return (Thread *)pthread_getspecific(self_key); }
void    setThreadSelf(Thread *t)  { pthread_setspecific(self_key, t); }
ExecEnv *getExecEnv(void)         { return threadSelf()->ee; }
void   *getStackTop(Thread *t)    { return t->stack_top; }
void   *getStackBase(Thread *t)   { return t->stack_base; }

/* ---- Java interpreter stack -------------------------------------------- */

/* dummy bottom-of-stack method; its max_stack must be 0 so the first real
 * frame is placed immediately above it (see execute.c CREATE_TOP_FRAME). */
MethodBlock dummy_mb;   /* zero-initialised: name/code NULL, max_stack 0 */

static void initialiseJavaStack(ExecEnv *ee) {
    char *stack = malloc(java_stack_size);
    Frame *dummy = (Frame *)stack;

    dummy->mb = &dummy_mb;
    dummy->lvars = (Slot *)(dummy + 1);
    dummy->ostack = (Slot *)(dummy + 1);
    dummy->prev = NULL;

    ee->stack = stack;
    ee->last_frame = dummy;
    ee->stack_end = stack + java_stack_size - sizeof(Frame) - 1024;
}

/* ---- suspension --------------------------------------------------------- */

static void suspendLoop(Thread *thread) {
    sigjmp_buf env;
    sigset_t mask;
    char old_state = thread->state;

    sigsetjmp(env, FALSE);        /* flush registers onto the stack */
    thread->stack_top = (void *)&env;
    thread->state = THREAD_SUSPENDED;

    sigfillset(&mask);
    sigdelset(&mask, SUSPEND_SIG);
    sigdelset(&mask, SIGTERM);

    while(thread->suspend)
        sigsuspend(&mask);

    thread->state = old_state;
}

static void suspendHandler(int sig) {
    (void)sig;
    suspendLoop(threadSelf());
}

void disableSuspend0(Thread *thread, void *stack_top) {
    sigset_t mask;
    thread->stack_top = stack_top;
    thread->blocking = TRUE;
    sigemptyset(&mask);
    sigaddset(&mask, SUSPEND_SIG);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void enableSuspend(Thread *thread) {
    sigset_t mask;
    thread->blocking = FALSE;
    if(thread->suspend)
        suspendLoop(thread);
    sigemptyset(&mask);
    sigaddset(&mask, SUSPEND_SIG);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}

void suspendAllThreads(Thread *self) {
    Thread *thread;
    lockVMLock(&thread_lock, self);

    for(thread = thread_list; thread != NULL; thread = thread->next) {
        if(thread == self) continue;
        thread->suspend = TRUE;
        if(!thread->blocking)
            pthread_kill(thread->tid, SUSPEND_SIG);
    }

    /* wait until every other thread is parked (suspended or blocking) */
    for(thread = thread_list; thread != NULL; thread = thread->next) {
        if(thread == self) continue;
        while(!thread->blocking && thread->state != THREAD_SUSPENDED)
            sched_yield();
    }
}

void resumeAllThreads(Thread *self) {
    Thread *thread;
    for(thread = thread_list; thread != NULL; thread = thread->next) {
        if(thread == self) continue;
        thread->suspend = FALSE;
    }
    for(thread = thread_list; thread != NULL; thread = thread->next) {
        if(thread == self) continue;
        if(!thread->blocking)
            pthread_kill(thread->tid, SUSPEND_SIG);
    }
    for(thread = thread_list; thread != NULL; thread = thread->next) {
        if(thread == self) continue;
        while(thread->state == THREAD_SUSPENDED)
            sched_yield();
    }
    unlockVMLock(&thread_lock, self);
}

/* ---- GC hooks ----------------------------------------------------------- */

void scanThread(Thread *thread) {
    ExecEnv *ee = thread->ee;
    char *slot, *end;

    if(ee->thread) markObject(ee->thread);

    /* conservatively scan the native C stack (top..base) */
    slot = (char *)getStackTop(thread);
    end  = (char *)getStackBase(thread);
    if(slot && end) {
        for(; slot < end; slot += sizeof(void *)) {
            void *ref = *(void **)slot;
            if(isObjectRef(ref)) markObject((Object *)ref);
        }
    }

    /* conservatively scan the used portion of the Java stack */
    {
        Frame *top = ee->last_frame;
        char *jslot = (char *)ee->stack;
        char *jend  = (char *)(top->ostack + top->mb->max_stack);
        for(; jslot < jend; jslot += sizeof(Slot)) {
            void *ref = ((Slot *)jslot)->o;
            if(isObjectRef(ref)) markObject((Object *)ref);
        }
    }
}

void scanThreads(void) {
    Thread *thread;
    for(thread = thread_list; thread != NULL; thread = thread->next)
        scanThread(thread);
}

/* ---- idle detection ----------------------------------------------------- */

int systemIdle(Thread *self) {
    Thread *thread;
    for(thread = thread_list; thread != NULL; thread = thread->next)
        if(thread != self && thread->state < THREAD_WAITING)
            return FALSE;
    return TRUE;
}

/* ---- list management ---------------------------------------------------- */

static void linkThread(Thread *thread) {
    thread->next = thread_list;
    thread->prev = NULL;
    if(thread_list) thread_list->prev = thread;
    thread_list = thread;
}
static void unlinkThread(Thread *thread) {
    if(thread->prev) thread->prev->next = thread->next; else thread_list = thread->next;
    if(thread->next) thread->next->prev = thread->prev;
}

/* ---- creating Saral threads -------------------------------------------- */

static void *threadStart(void *arg) {
    Thread *thread = (Thread *)arg;
    ExecEnv *ee = thread->ee;
    Object *thread_obj = ee->thread;
    MethodBlock *run;
    char stack_marker;

    thread->stack_base = &stack_marker;
    initialiseJavaStack(ee);
    setThreadSelf(thread);

    disableSuspend(thread);
    lockVMLock(&thread_lock, thread);
    linkThread(thread);
    thread->state = THREAD_RUNNING;
    unlockVMLock(&thread_lock, thread);
    enableSuspend(thread);

    pthread_mutex_lock(&exit_lock);
    non_daemon_thrds++;
    pthread_mutex_unlock(&exit_lock);

    run = lookupMethod(thread_obj->class, "run", "()V");
    if(run != NULL)
        executeMethodArgs(thread_obj, thread_obj->class, run);

    if(exceptionOccured())
        printException();

    disableSuspend(thread);
    lockVMLock(&thread_lock, thread);
    unlinkThread(thread);
    thread->state = 0;
    freeThreadID(thread->id);
    unlockVMLock(&thread_lock, thread);
    enableSuspend(thread);

    pthread_mutex_lock(&exit_lock);
    if(--non_daemon_thrds == 0)
        pthread_cond_signal(&exit_cv);
    pthread_mutex_unlock(&exit_lock);

    free(ee->stack);
    free(ee);
    free(thread);
    return NULL;
}

void createSaralThread(Object *thread_obj) {
    Thread  *thread = calloc(1, sizeof(Thread));
    ExecEnv *ee     = calloc(1, sizeof(ExecEnv));
    Thread  *self   = threadSelf();

    thread->ee = ee;
    ee->thread = thread_obj;
    thread->state = THREAD_CREATING;

    disableSuspend(self);
    lockVMLock(&thread_lock, self);
    thread->id = genThreadID();
    unlockVMLock(&thread_lock, self);
    enableSuspend(self);

    if(pthread_create(&thread->tid, NULL, threadStart, thread) != 0) {
        free(ee); free(thread);
        signalException("saral/lang/OutOfMemoryError", "cannot create thread");
    }
}

/* ---- internal VM (daemon) threads -------------------------------------- */

static void *vmThreadStart(void *arg) {
    Thread *thread = (Thread *)arg;
    char stack_marker;

    thread->stack_base = &stack_marker;
    initialiseJavaStack(thread->ee);
    setThreadSelf(thread);

    disableSuspend(thread);
    lockVMLock(&thread_lock, thread);
    linkThread(thread);
    thread->state = THREAD_RUNNING;
    unlockVMLock(&thread_lock, thread);
    enableSuspend(thread);

    thread->start(thread);
    return NULL;
}

void createVMThread(char *name, void (*start)(Thread *)) {
    Thread  *thread = calloc(1, sizeof(Thread));
    ExecEnv *ee     = calloc(1, sizeof(ExecEnv));
    (void)name;
    thread->ee = ee;
    thread->start = start;
    thread->id = genThreadID();
    pthread_create(&thread->tid, NULL, vmThreadStart, thread);
}

/* ---- misc thread ops ---------------------------------------------------- */

void threadSleep(Thread *thread, long long ms, int ns) {
    struct timespec ts;
    (void)thread;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000 + ns;
    nanosleep(&ts, NULL);
}

void threadInterrupt(Thread *thread) {
    thread->interrupted = TRUE;
    if(thread->wait_mon)
        pthread_cond_broadcast(&thread->wait_mon->cv);
}

/* ---- signals ------------------------------------------------------------ */

void initialiseSignals(void) {
    struct sigaction act;
    act.sa_handler = suspendHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(SUSPEND_SIG, &act, NULL);
}

/* ---- bootstrap / shutdown ---------------------------------------------- */

void initialiseMainThread(int stack_size) {
    char stack_marker;
    java_stack_size = stack_size;

    pthread_key_create(&self_key, NULL);
    initVMLock(&thread_lock);

    tid_bitmap = calloc(1, sizeof(unsigned int));
    tid_bitmap_size = 1;

    main_thread.state = THREAD_RUNNING;
    main_thread.tid = pthread_self();
    main_thread.id = genThreadID();
    main_thread.ee = &main_ee;
    main_thread.stack_base = &stack_marker;

    initialiseJavaStack(&main_ee);
    setThreadSelf(&main_thread);
    linkThread(&main_thread);
    non_daemon_thrds = 1;

    initialiseSignals();
}

void mainThreadWaitToExitVM(void) {
    Thread *self = threadSelf();
    self->state = THREAD_WAITING;

    pthread_mutex_lock(&exit_lock);
    non_daemon_thrds--;   /* main thread is leaving */
    while(non_daemon_thrds > 0)
        pthread_cond_wait(&exit_cv, &exit_lock);
    pthread_mutex_unlock(&exit_lock);
}
