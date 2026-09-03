/*
 * SVM - the VM -> Saral call boundary.
 *
 * executeMethodArgs / executeMethodVaList / executeMethodList build a fresh
 * top frame on the current thread's interpreter stack, marshal the arguments
 * into the local-variable slots, and either invoke a native method directly
 * or run the bytecode via executeJava().  Because SVM slots are 64-bit, every
 * argument (including long/num) is a single slot.
 */

#include <stdio.h>
#include "svm.h"
#include "thread.h"

typedef Slot *(*NativeInvoker)(Class *, MethodBlock *, Slot *);

/* Build the dummy + real top frame; returns the local-variable base (sp). */
static Slot *createTopFrame(ExecEnv *ee, MethodBlock *mb) {
    Frame *last = ee->last_frame;
    Frame *dummy = (Frame *)(last->ostack + last->mb->max_stack);
    Slot  *sp = (Slot *)(dummy + 1);
    Frame *new_frame = (Frame *)(sp + mb->max_locals);

    dummy->mb = NULL;
    dummy->ostack = sp;
    dummy->prev = last;

    new_frame->mb = mb;
    new_frame->lvars = sp;
    new_frame->ostack = (Slot *)(new_frame + 1);
    new_frame->prev = dummy;

    ee->last_frame = new_frame;
    return sp;
}

static void popTopFrame(ExecEnv *ee) {
    ee->last_frame = ee->last_frame->prev->prev;
}

static Slot runFrame(ExecEnv *ee, Object *ob, Class *class,
                     MethodBlock *mb, Slot *sp) {
    Slot result;

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : (Object *)class);

    if(mb->access_flags & ACC_NATIVE) {
        NativeInvoker fn = (NativeInvoker)mb->native_invoker;
        Slot *top = fn(class, mb, sp);
        result = (top > sp) ? sp[0] : (Slot){ .p = 0 };
    } else {
        result = executeJava();
    }

    if(mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : (Object *)class);

    popTopFrame(ee);
    return result;
}

Slot executeMethodVaList(Object *ob, Class *class, MethodBlock *mb, va_list args) {
    ExecEnv *ee = getExecEnv();
    Slot *sp = createTopFrame(ee, mb);
    char *p = mb->type + 1;   /* first param */

    if(ob) (sp++)->o = ob;    /* receiver */

    while(*p != ')') {
        Slot s;
        switch(*p) {
            case 'D': s.d = va_arg(args, double); break;
            case 'J': case 'K': s.i = va_arg(args, int64_t); break;
            case 'L': case '[': s.o = va_arg(args, void *);
                      while(*p == '[') p++;
                      if(*p == 'L') while(*p != ';') p++;
                      break;
            default:  s.i = va_arg(args, int); break;   /* Z B C I U */
        }
        *sp++ = s;
        p++;
    }
    return runFrame(ee, ob, class, mb, ee->last_frame->lvars);
}

Slot executeMethodArgs(Object *ob, Class *class, MethodBlock *mb, ...) {
    va_list args;
    Slot ret;
    va_start(args, mb);
    ret = executeMethodVaList(ob, class, mb, args);
    va_end(args);
    return ret;
}

Slot executeMethodList(Object *ob, Class *class, MethodBlock *mb, Slot *args) {
    ExecEnv *ee = getExecEnv();
    Slot *sp = createTopFrame(ee, mb);
    int i, n = mb->args_count - (ob ? 1 : 0);

    if(ob) (sp++)->o = ob;
    for(i = 0; i < n; i++)
        *sp++ = args[i];

    return runFrame(ee, ob, class, mb, ee->last_frame->lvars);
}
