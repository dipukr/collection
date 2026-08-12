#include "thread.h"

#define COMPARE_AND_SWAP(addr, old_val, new_val)   \
({                                                 \
    char result;                                   \
    int read_val;                                  \
    __asm__ __volatile__ ("                        \
	lock;                                      \
	cmpxchgl %5, %1;                           \
	sete %0"                                   \
    : "=q" (result), "=m" (*addr), "=a" (read_val) \
    : "m" (*addr), "a" (old_val), "r" (new_val)    \
    : "memory");                                   \
    result;                                        \
})

#define ATOMIC_READ(addr) *addr
#define ATOMIC_WRITE(addr, value) *addr = value

#define SCAN_SIG(p, D, S)           \
   p++;               /* skip start ( */    \
   while(*p != ')') {               \
       if((*p == 'J') || (*p == 'D')) {     \
          D;                    \
          p++;                  \
      } else {                  \
          S;                    \
          if(*p == '[')             \
              for(p++; *p == '['; p++);     \
          if(*p == 'L')             \
              while(*p++ != ';');       \
          else                  \
              p++;              \
      }                     \
   }                        \
   p++;

extern void monitorInit(Monitor *mon);
extern void monitorLock(Monitor *mon, Thread *self);
extern void monitorUnlock(Monitor *mon, Thread *self);
extern int monitorWait(Monitor *mon, Thread *self, long long ms, int ns);
extern int monitorNotify(Monitor *mon, Thread *self);
extern int monitorNotifyAll(Monitor *mon, Thread *self);

extern void objectLock(Object *ob);
extern void objectUnlock(Object *ob);
extern void objectNotify(Object *ob);
extern void objectNotifyAll(Object *ob);
extern void objectWait(Object *ob, long long ms, int ns);
extern int objectLockedByCurrent(Object *ob);

