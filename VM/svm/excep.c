/*
 * SVM - exception support.
 *
 * The pending exception lives in ExecEnv->exception.  signalException raises
 * a VM-internal exception; findCatchBlock unwinds the frame chain looking for
 * a matching handler (catch_type 0 means finally / catch-all), releasing
 * synchronized monitors as it unwinds.  The interpreter drives propagation.
 *
 * Core library contract:
 *   class saral/lang/Throwable {
 *       saral/lang/String message;   // field "message", "Lsaral/lang/String;"
 *       long[]            backtrace;  // field "backtrace", "[J"
 *   }
 *   constructors: <init>()V  and  <init>(Lsaral/lang/String;)V
 */

#include <stdio.h>
#include <stdlib.h>
#include "svm.h"
#include "thread.h"

extern char *slash2dots(const char *utf8);

Object *exceptionOccured(void) { return getExecEnv()->exception; }
void    setException(Object *e) { getExecEnv()->exception = e; }
void    clearException(void)    { getExecEnv()->exception = NULL; }

void signalException(char *excep_name, char *message) {
    ExecEnv *ee = getExecEnv();
    Class *excep_class;
    Object *excep;
    MethodBlock *init;

    if(VM_initing) {
        fprintf(stderr, "Exception during VM init: %s (%s)\n",
                excep_name, message ? message : "");
        exit(1);
    }
    if(ee->exception)   /* don't overwrite a pending exception */
        return;

    excep_class = findSystemClass(excep_name);
    if(ee->exception) return;   /* class load itself failed */

    excep = allocObject(excep_class);
    if(excep == NULL) return;

    if(message != NULL) {
        Object *str = createString(message);
        init = lookupMethod(excep_class, "<init>", "(Lsaral/lang/String;)V");
        if(init) executeMethodArgs(excep, excep_class, init, str);
    } else {
        init = lookupMethod(excep_class, "<init>", "()V");
        if(init) executeMethodArgs(excep, excep_class, init);
    }

    setStackTrace(excep);
    ee->exception = excep;
}

/* ---- handler search ----------------------------------------------------- */

static u1 *findCatchBlockInMethod(MethodBlock *mb, Class *excep, u1 *pc) {
    int i;
    int cur = pc - mb->code;
    for(i = 0; i < mb->exception_table_size; i++) {
        ExceptionTableEntry *e = &mb->exception_table[i];
        if(cur < e->start_pc || cur >= e->end_pc)
            continue;
        if(e->catch_type == 0)
            return mb->code + e->handler_pc;   /* finally / catch-all */
        {
            Class *caught = resolveClass(mb->class, e->catch_type, FALSE);
            if(caught == NULL) { clearException(); continue; }
            if(isInstanceOf(caught, excep))
                return mb->code + e->handler_pc;
        }
    }
    return NULL;
}

u1 *findCatchBlock(Class *exception) {
    ExecEnv *ee = getExecEnv();
    Frame *frame = ee->last_frame;
    Frame *top = frame;

    while(frame->mb != NULL) {
        u1 *handler = findCatchBlockInMethod(frame->mb, exception, frame->last_pc);
        if(handler != NULL) {
            ee->last_frame = frame;
            return handler;
        }
        /* release monitor held by a synchronized method being unwound */
        if(frame->mb->access_flags & ACC_SYNCHRONIZED) {
            Object *sync = (frame->mb->access_flags & ACC_STATIC)
                         ? (Object *)frame->mb->class : frame->lvars[0].o;
            objectUnlock(sync);
        }
        top = frame;
        frame = frame->prev;
    }
    /* uncaught: leave last_frame at this invocation's top frame so the
     * VM->Saral caller can pop it cleanly */
    ee->last_frame = top;
    return NULL;
}

/* ---- stack traces ------------------------------------------------------- */

static int mapPC2LineNo(MethodBlock *mb, u1 *pc) {
    int i, line = -1;
    int off = pc - mb->code;
    for(i = 0; i < mb->line_no_table_size; i++)
        if(off >= mb->line_no_table[i].start_pc)
            line = mb->line_no_table[i].line_no;
    return line;
}

void setStackTrace(Object *excep) {
    ExecEnv *ee = getExecEnv();
    FieldBlock *bt_fb = findField(excep->class, "backtrace", "[J");
    Frame *frame;
    Object *array;
    Slot *body;
    int depth = 0, i;

    if(bt_fb == NULL) return;

    for(frame = ee->last_frame; frame->mb != NULL; frame = frame->prev)
        depth++;

    array = allocTypeArray(T_LONG, depth * 2);
    if(array == NULL) return;
    body = (Slot *)array_body(array);

    for(i = 0, frame = ee->last_frame; frame->mb != NULL; frame = frame->prev) {
        body[i++].p = (uintptr_t)frame->mb;
        body[i++].p = (uintptr_t)frame->last_pc;
    }
    inst_data(excep)[bt_fb->offset].o = array;
}

void printStackTrace(Object *excep) {
    ClassBlock *cb = class_cb(excep->class);
    FieldBlock *msg_fb = findField(excep->class, "message", "Lsaral/lang/String;");
    FieldBlock *bt_fb  = findField(excep->class, "backtrace", "[J");
    char *cname = slash2dots(cb->name);

    fprintf(stderr, "%s", cname);
    if(msg_fb) {
        Object *msg = inst_data(excep)[msg_fb->offset].o;
        if(msg) { char *m = string2Cstr(msg); fprintf(stderr, ": %s", m); free(m); }
    }
    fprintf(stderr, "\n");
    free(cname);

    if(bt_fb) {
        Object *array = inst_data(excep)[bt_fb->offset].o;
        if(array) {
            Slot *body = (Slot *)array_body(array);
            int len = array_length(array), i;
            for(i = 0; i < len; i += 2) {
                MethodBlock *mb = (MethodBlock *)body[i].p;
                u1 *pc = (u1 *)body[i + 1].p;
                char *mcname = slash2dots(class_cb(mb->class)->name);
                int line = mapPC2LineNo(mb, pc);
                fprintf(stderr, "\tat %s.%s", mcname, mb->name);
                if(mb->access_flags & ACC_NATIVE)
                    fprintf(stderr, "(native)\n");
                else if(class_cb(mb->class)->source_file_name)
                    fprintf(stderr, "(%s:%d)\n", class_cb(mb->class)->source_file_name, line);
                else
                    fprintf(stderr, "(unknown)\n");
                free(mcname);
            }
        }
    }
}

void printException(void) {
    ExecEnv *ee = getExecEnv();
    Object *excep = ee->exception;
    if(excep == NULL) return;
    ee->exception = NULL;
    printStackTrace(excep);
}
