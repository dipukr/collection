/*
 * SVM - built-in ("internal") native methods.
 *
 * SVM uses a single lightweight native calling convention (no JNI):
 *
 *     Slot *native(Class *class, MethodBlock *mb, Slot *args);
 *
 * `args` points at the method's arguments on the caller's operand stack
 * (args[0] is `this` for instance methods).  A native reads its arguments
 * from there, writes any return value into args[0], and returns the new
 * operand-stack top: `args` for a void method, `args + 1` for a value.
 *
 * Native methods are bound lazily: linkClass installs resolveNativeWrapper as
 * the invoker; on the first call it looks the method up in native_methods,
 * caches the real function pointer in the MethodBlock, and tail-calls it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "svm.h"
#include "thread.h"

typedef Slot *(*NativeInvoker)(Class *, MethodBlock *, Slot *);

/* ---- saral/lang/SVM : console output & runtime ------------------------- */

static Slot *n_printString(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m;
    if(args[0].o) { char *s = string2Cstr(args[0].o); fputs(s, stdout); free(s); }
    else fputs("null", stdout);
    return args;
}
static Slot *n_printInt(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; printf("%lld", (long long)args[0].i); return args;
}
static Slot *n_printUInt(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; printf("%llu", (unsigned long long)args[0].u); return args;
}
static Slot *n_printNum(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; printf("%g", args[0].d); return args;
}
static Slot *n_printChar(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; putchar((int)args[0].i); return args;
}
static Slot *n_printBool(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; fputs(args[0].i ? "true" : "false", stdout); return args;
}
static Slot *n_gc(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; gc(); return args;
}
static Slot *n_freeMemory(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; args[0].i = freeHeapMem(); return args + 1;
}

/* ---- saral/lang/Object ------------------------------------------------- */

static Slot *n_getClass(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; args[0].o = (Object *)args[0].o->class; return args + 1;
}
static Slot *n_hashCode(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; args[0].i = (int)(intptr_t)args[0].o; return args + 1;
}
static Slot *n_objWait(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; objectWait(args[0].o, args[1].i, 0); return args;
}
static Slot *n_objNotify(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; objectNotify(args[0].o); return args;
}
static Slot *n_objNotifyAll(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; objectNotifyAll(args[0].o); return args;
}

/* ---- saral/lang/Thread ------------------------------------------------- */

static Slot *n_threadStart(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; createSaralThread(args[0].o); return args;
}
static Slot *n_threadSleep(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; threadSleep(threadSelf(), args[0].i, 0); return args;
}
static Slot *n_threadYield(Class *c, MethodBlock *m, Slot *args) {
    (void)c; (void)m; sched_yield(); return args;
}

/* ---- registration table ------------------------------------------------- */

typedef struct {
    const char *cls;
    const char *name;
    const char *sig;
    NativeInvoker fn;
} NativeEntry;

static NativeEntry native_methods[] = {
    { "saral/lang/SVM", "printString", "(Lsaral/lang/String;)V", n_printString },
    { "saral/lang/SVM", "printInt",    "(J)V",                   n_printInt    },
    { "saral/lang/SVM", "printUInt",   "(K)V",                   n_printUInt   },
    { "saral/lang/SVM", "printNum",    "(D)V",                   n_printNum    },
    { "saral/lang/SVM", "printChar",   "(C)V",                   n_printChar   },
    { "saral/lang/SVM", "printBool",   "(Z)V",                   n_printBool   },
    { "saral/lang/SVM", "gc",          "()V",                    n_gc          },
    { "saral/lang/SVM", "freeMemory",  "()J",                    n_freeMemory  },

    { "saral/lang/Object", "getClass",  "()Lsaral/lang/Class;",  n_getClass    },
    { "saral/lang/Object", "hashCode",  "()I",                   n_hashCode    },
    { "saral/lang/Object", "wait",      "(J)V",                  n_objWait     },
    { "saral/lang/Object", "notify",    "()V",                   n_objNotify   },
    { "saral/lang/Object", "notifyAll", "()V",                   n_objNotifyAll},

    { "saral/lang/Thread", "start",     "()V",                   n_threadStart },
    { "saral/lang/Thread", "sleep",     "(J)V",                  n_threadSleep },
    { "saral/lang/Thread", "yield",     "()V",                   n_threadYield },

    { NULL, NULL, NULL, NULL }
};

void *lookupInternalNative(MethodBlock *mb) {
    const char *cls = class_cb(mb->class)->name;
    int i;
    for(i = 0; native_methods[i].cls != NULL; i++)
        if(strcmp(native_methods[i].cls, cls) == 0 &&
           strcmp(native_methods[i].name, mb->name) == 0 &&
           strcmp(native_methods[i].sig, mb->type) == 0)
            return (void *)native_methods[i].fn;
    return NULL;
}

Slot *resolveNativeWrapper(Class *class, MethodBlock *mb, Slot *args) {
    NativeInvoker fn = (NativeInvoker)lookupInternalNative(mb);
    if(fn == NULL) {
        signalException("saral/lang/UnsatisfiedLinkError", mb->name);
        return args;
    }
    mb->native_invoker = (void *)fn;
    return fn(class, mb, args);
}

void initialiseNatives(void) { /* table is static; nothing to do */ }
