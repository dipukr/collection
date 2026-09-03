/*
 * SVM - the bytecode interpreter.
 *
 * A stack-based, direct-threaded interpreter (gcc computed goto), with a
 * portable switch-based fallback when THREADED is not defined.  Method
 * invocation and return are handled in-line (no C recursion): a new frame is
 * laid out above the caller's operand stack and the interpreter's working
 * registers (pc/mb/lvars/ostack/cp) are re-pointed at it.  This is JamVM's
 * model, adapted to 64-bit slots.
 *
 * The one place SVM keeps macros rather than inline functions is instruction
 * dispatch (DISPATCH/NEXT/DEF): computed-goto dispatch cannot be expressed as
 * a function.  Everything else uses the inline operand readers below.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "svm.h"
#include "thread.h"

typedef Slot *(*NativeInvoker)(Class *, MethodBlock *, Slot *);

/* ---- inline operand readers -------------------------------------------- */

static inline int     idx2(const u1 *pc) { return (pc[1] << 8) | pc[2]; }
static inline int     br2 (const u1 *pc) { return (int16_t)((pc[1] << 8) | pc[2]); }
static inline int32_t imm4(const u1 *pc) { return (int32_t)(((u4)pc[1]<<24)|((u4)pc[2]<<16)|((u4)pc[3]<<8)|pc[4]); }
static inline int32_t be32(const u1 *p)  { return (int32_t)(((u4)p[0]<<24)|((u4)p[1]<<16)|((u4)p[2]<<8)|p[3]); }

#ifdef THREADED
#define DEF(op)   L_##op
#define DISPATCH  goto *handlers[*pc]
#else
#define DEF(op)   case OP_##op
#define DISPATCH  break
#endif

#define NEXT(n)   do { pc += (n); DISPATCH; } while(0)

#define NULLCHECK(o)                                                       \
    if((o) == NULL) {                                                      \
        frame->last_pc = (u1 *)pc;                                         \
        signalException("saral/lang/NullPointerException", NULL);          \
        goto throwException;                                               \
    }

#define BOUNDSCHECK(arr, i)                                                \
    if((unsigned)(i) >= (unsigned)array_length(arr)) {                     \
        char b[32]; sprintf(b, "%d", (int)(i));                           \
        frame->last_pc = (u1 *)pc;                                         \
        signalException("saral/lang/ArrayIndexOutOfBoundsException", b);   \
        goto throwException;                                               \
    }

#define ZEROCHECK(v)                                                       \
    if((v) == 0) {                                                         \
        frame->last_pc = (u1 *)pc;                                         \
        signalException("saral/lang/ArithmeticException", "/ by zero");    \
        goto throwException;                                               \
    }

Slot executeJava(void) {
    ExecEnv *ee = getExecEnv();
    Frame  *frame = ee->last_frame;
    MethodBlock *mb = frame->mb;
    Slot   *lvars  = frame->lvars;
    Slot   *ostack = frame->ostack;
    ConstantPool *cp = &class_cb(mb->class)->constant_pool;
    const u1 *pc = mb->code;

    MethodBlock *new_mb;
    Slot *arg1;

#ifdef THREADED
    static const void *handlers[] = {
        &&L_NOP, &&L_ACONST_NULL, &&L_ICONST, &&L_LDC, &&L_LOAD, &&L_STORE,
        &&L_IINC, &&L_POP, &&L_DUP, &&L_DUP_X1, &&L_SWAP, &&L_IADD, &&L_ISUB,
        &&L_IMUL, &&L_IDIV, &&L_IREM, &&L_UDIV, &&L_UREM, &&L_INEG, &&L_DADD,
        &&L_DSUB, &&L_DMUL, &&L_DDIV, &&L_DREM, &&L_DNEG, &&L_IAND, &&L_IOR,
        &&L_IXOR, &&L_INOT, &&L_ISHL, &&L_ISHR, &&L_IUSHR, &&L_I2D, &&L_D2I,
        &&L_U2D, &&L_I2B, &&L_I2UB, &&L_I2C, &&L_I2S, &&L_I2I, &&L_I2U,
        &&L_LCMP, &&L_ULCMP, &&L_DCMPL, &&L_DCMPG, &&L_GOTO, &&L_IFEQ,
        &&L_IFNE, &&L_IFLT, &&L_IFGE, &&L_IFGT, &&L_IFLE, &&L_IF_ICMPEQ,
        &&L_IF_ICMPNE, &&L_IF_ICMPLT, &&L_IF_ICMPGE, &&L_IF_ICMPGT,
        &&L_IF_ICMPLE, &&L_IF_ACMPEQ, &&L_IF_ACMPNE, &&L_IFNULL, &&L_IFNONNULL,
        &&L_TABLESWITCH, &&L_LOOKUPSWITCH, &&L_RETURN, &&L_IRETURN,
        &&L_GETSTATIC, &&L_PUTSTATIC, &&L_GETFIELD, &&L_PUTFIELD,
        &&L_INVOKEVIRTUAL, &&L_INVOKESPECIAL, &&L_INVOKESTATIC,
        &&L_INVOKEINTERFACE, &&L_NEW, &&L_NEWARRAY, &&L_ANEWARRAY,
        &&L_ARRAYLENGTH, &&L_MULTIANEWARRAY, &&L_IALOAD, &&L_IASTORE,
        &&L_BALOAD, &&L_BASTORE, &&L_UBALOAD, &&L_UBASTORE, &&L_CALOAD,
        &&L_CASTORE, &&L_SALOAD, &&L_SASTORE, &&L_WALOAD, &&L_WASTORE,
        &&L_CHECKCAST, &&L_INSTANCEOF, &&L_ATHROW, &&L_MONITORENTER,
        &&L_MONITOREXIT
    };
    DISPATCH;
#else
    while(1) {
        switch(*pc) {
#endif

    DEF(NOP):          NEXT(1);
    DEF(ACONST_NULL):  (ostack++)->o = NULL; NEXT(1);
    DEF(ICONST):       (ostack++)->i = imm4(pc); NEXT(5);

    DEF(LDC): {
        int idx = idx2(pc);
        u1 t = cp_type(cp, idx);
        Slot s;
        if(t == CONSTANT_String || t == CONSTANT_Resolved)
            s.o = (Object *)resolveSingleConstant(mb->class, idx);
        else if(t == CONSTANT_Double) {
            u8 bits = (u8)cp_info(cp, idx);
            memcpy(&s.d, &bits, sizeof(double));
        } else
            s.i = (int64_t)(intptr_t)cp_info(cp, idx);
        *ostack++ = s;
        NEXT(3);
    }

    DEF(LOAD):   *ostack++ = lvars[idx2(pc)]; NEXT(3);
    DEF(STORE):  lvars[idx2(pc)] = *--ostack; NEXT(3);
    DEF(IINC):   lvars[idx2(pc)].i += (int16_t)((pc[3] << 8) | pc[4]); NEXT(5);

    DEF(POP):    ostack--; NEXT(1);
    DEF(DUP):    *ostack = ostack[-1]; ostack++; NEXT(1);
    DEF(DUP_X1): {
        Slot v1 = ostack[-1], v2 = ostack[-2];
        ostack[-2] = v1; ostack[-1] = v2; *ostack++ = v1; NEXT(1);
    }
    DEF(SWAP): { Slot t = ostack[-1]; ostack[-1] = ostack[-2]; ostack[-2] = t; NEXT(1); }

    DEF(IADD): ostack[-2].i = ostack[-2].i + ostack[-1].i; ostack--; NEXT(1);
    DEF(ISUB): ostack[-2].i = ostack[-2].i - ostack[-1].i; ostack--; NEXT(1);
    DEF(IMUL): ostack[-2].i = ostack[-2].i * ostack[-1].i; ostack--; NEXT(1);
    DEF(IDIV): ZEROCHECK(ostack[-1].i); ostack[-2].i = ostack[-2].i / ostack[-1].i; ostack--; NEXT(1);
    DEF(IREM): ZEROCHECK(ostack[-1].i); ostack[-2].i = ostack[-2].i % ostack[-1].i; ostack--; NEXT(1);
    DEF(UDIV): ZEROCHECK(ostack[-1].u); ostack[-2].u = ostack[-2].u / ostack[-1].u; ostack--; NEXT(1);
    DEF(UREM): ZEROCHECK(ostack[-1].u); ostack[-2].u = ostack[-2].u % ostack[-1].u; ostack--; NEXT(1);
    DEF(INEG): ostack[-1].i = -ostack[-1].i; NEXT(1);

    DEF(DADD): ostack[-2].d = ostack[-2].d + ostack[-1].d; ostack--; NEXT(1);
    DEF(DSUB): ostack[-2].d = ostack[-2].d - ostack[-1].d; ostack--; NEXT(1);
    DEF(DMUL): ostack[-2].d = ostack[-2].d * ostack[-1].d; ostack--; NEXT(1);
    DEF(DDIV): ostack[-2].d = ostack[-2].d / ostack[-1].d; ostack--; NEXT(1);
    DEF(DREM): ostack[-2].d = fmod(ostack[-2].d, ostack[-1].d); ostack--; NEXT(1);
    DEF(DNEG): ostack[-1].d = -ostack[-1].d; NEXT(1);

    DEF(IAND): ostack[-2].i = ostack[-2].i & ostack[-1].i; ostack--; NEXT(1);
    DEF(IOR):  ostack[-2].i = ostack[-2].i | ostack[-1].i; ostack--; NEXT(1);
    DEF(IXOR): ostack[-2].i = ostack[-2].i ^ ostack[-1].i; ostack--; NEXT(1);
    DEF(INOT): ostack[-1].i = ~ostack[-1].i; NEXT(1);
    DEF(ISHL): ostack[-2].i = ostack[-2].i << (ostack[-1].i & 63); ostack--; NEXT(1);
    DEF(ISHR): ostack[-2].i = ostack[-2].i >> (ostack[-1].i & 63); ostack--; NEXT(1);
    DEF(IUSHR):ostack[-2].u = ostack[-2].u >> (ostack[-1].i & 63); ostack--; NEXT(1);

    DEF(I2D):  ostack[-1].d = (double)ostack[-1].i; NEXT(1);
    DEF(D2I):  ostack[-1].i = (int64_t)ostack[-1].d; NEXT(1);
    DEF(U2D):  ostack[-1].d = (double)ostack[-1].u; NEXT(1);
    DEF(I2B):  ostack[-1].i = (int8_t)ostack[-1].i;  NEXT(1);
    DEF(I2UB): ostack[-1].i = (uint8_t)ostack[-1].i; NEXT(1);
    DEF(I2C):  ostack[-1].i = (uint16_t)ostack[-1].i; NEXT(1);
    DEF(I2S):  ostack[-1].i = (int16_t)ostack[-1].i; NEXT(1);
    DEF(I2I):  ostack[-1].i = (int32_t)ostack[-1].i; NEXT(1);
    DEF(I2U):  ostack[-1].i = (uint32_t)ostack[-1].i; NEXT(1);

    DEF(LCMP): {
        int64_t v2 = ostack[-1].i, v1 = ostack[-2].i; ostack--;
        ostack[-1].i = v1 < v2 ? -1 : (v1 > v2 ? 1 : 0); NEXT(1);
    }
    DEF(ULCMP): {
        uint64_t v2 = ostack[-1].u, v1 = ostack[-2].u; ostack--;
        ostack[-1].i = v1 < v2 ? -1 : (v1 > v2 ? 1 : 0); NEXT(1);
    }
    DEF(DCMPL): {
        double v2 = ostack[-1].d, v1 = ostack[-2].d; ostack--;
        ostack[-1].i = v1 < v2 ? -1 : (v1 > v2 ? 1 : (v1 == v2 ? 0 : -1)); NEXT(1);
    }
    DEF(DCMPG): {
        double v2 = ostack[-1].d, v1 = ostack[-2].d; ostack--;
        ostack[-1].i = v1 > v2 ? 1 : (v1 < v2 ? -1 : (v1 == v2 ? 0 : 1)); NEXT(1);
    }

    DEF(GOTO): pc += br2(pc); DISPATCH;

    DEF(IFEQ): if((--ostack)->i == 0) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFNE): if((--ostack)->i != 0) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFLT): if((--ostack)->i <  0) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFGE): if((--ostack)->i >= 0) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFGT): if((--ostack)->i >  0) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFLE): if((--ostack)->i <= 0) { pc += br2(pc); DISPATCH; } NEXT(3);

    DEF(IF_ICMPEQ): ostack -= 2; if(ostack[0].i == ostack[1].i) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ICMPNE): ostack -= 2; if(ostack[0].i != ostack[1].i) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ICMPLT): ostack -= 2; if(ostack[0].i <  ostack[1].i) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ICMPGE): ostack -= 2; if(ostack[0].i >= ostack[1].i) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ICMPGT): ostack -= 2; if(ostack[0].i >  ostack[1].i) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ICMPLE): ostack -= 2; if(ostack[0].i <= ostack[1].i) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ACMPEQ): ostack -= 2; if(ostack[0].o == ostack[1].o) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IF_ACMPNE): ostack -= 2; if(ostack[0].o != ostack[1].o) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFNULL):    if((--ostack)->o == NULL) { pc += br2(pc); DISPATCH; } NEXT(3);
    DEF(IFNONNULL): if((--ostack)->o != NULL) { pc += br2(pc); DISPATCH; } NEXT(3);

    DEF(TABLESWITCH): {
        const u1 *a = mb->code + (((pc - mb->code) + 1 + 3) & ~3);
        int deflt = be32(a), low = be32(a + 4), high = be32(a + 8);
        int index = (int)(--ostack)->i;
        if(index < low || index > high) pc += deflt;
        else pc += be32(a + 12 + (index - low) * 4);
        DISPATCH;
    }
    DEF(LOOKUPSWITCH): {
        const u1 *a = mb->code + (((pc - mb->code) + 1 + 3) & ~3);
        int deflt = be32(a), npairs = be32(a + 4);
        int key = (int)(--ostack)->i, i, off = deflt;
        for(i = 0; i < npairs; i++)
            if(be32(a + 8 + i * 8) == key) { off = be32(a + 8 + i * 8 + 4); break; }
        pc += off;
        DISPATCH;
    }

    /* ---- returns ---- */
    DEF(IRETURN): {
        Slot retval = *--ostack;
        Frame *cur = frame;
        if(cur->mb->access_flags & ACC_SYNCHRONIZED)
            objectUnlock(cur->mb->access_flags & ACC_STATIC ? (Object *)cur->mb->class : cur->lvars[0].o);
        frame = frame->prev;
        if(frame->mb == NULL) return retval;
        mb = frame->mb; lvars = frame->lvars;
        cp = &class_cb(mb->class)->constant_pool;
        ostack = cur->lvars;
        *ostack++ = retval;
        pc = frame->last_pc + 3;
        ee->last_frame = frame;
        DISPATCH;
    }
    DEF(RETURN): {
        Frame *cur = frame;
        if(cur->mb->access_flags & ACC_SYNCHRONIZED)
            objectUnlock(cur->mb->access_flags & ACC_STATIC ? (Object *)cur->mb->class : cur->lvars[0].o);
        frame = frame->prev;
        if(frame->mb == NULL) { Slot z; z.p = 0; return z; }
        mb = frame->mb; lvars = frame->lvars;
        cp = &class_cb(mb->class)->constant_pool;
        ostack = cur->lvars;
        pc = frame->last_pc + 3;
        ee->last_frame = frame;
        DISPATCH;
    }

    /* ---- fields ---- */
    DEF(GETSTATIC): {
        FieldBlock *fb;
        frame->last_pc = (u1 *)pc;
        fb = resolveField(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        *ostack++ = fb->static_value;
        NEXT(3);
    }
    DEF(PUTSTATIC): {
        FieldBlock *fb;
        frame->last_pc = (u1 *)pc;
        fb = resolveField(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        fb->static_value = *--ostack;
        NEXT(3);
    }
    DEF(GETFIELD): {
        FieldBlock *fb;
        Object *o;
        frame->last_pc = (u1 *)pc;
        fb = resolveField(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        o = ostack[-1].o;
        NULLCHECK(o);
        ostack[-1] = inst_data(o)[fb->offset];
        NEXT(3);
    }
    DEF(PUTFIELD): {
        FieldBlock *fb;
        Object *o;
        Slot v;
        frame->last_pc = (u1 *)pc;
        fb = resolveField(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        v = *--ostack;
        o = (--ostack)->o;
        NULLCHECK(o);
        inst_data(o)[fb->offset] = v;
        NEXT(3);
    }

    /* ---- invocation ---- */
    DEF(INVOKESTATIC):
        frame->last_pc = (u1 *)pc;
        new_mb = resolveMethod(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        arg1 = ostack - new_mb->args_count;
        goto invokeMethod;

    DEF(INVOKESPECIAL):
        frame->last_pc = (u1 *)pc;
        new_mb = resolveMethod(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        arg1 = ostack - new_mb->args_count;
        NULLCHECK(arg1[0].o);
        goto invokeMethod;

    DEF(INVOKEVIRTUAL):
        frame->last_pc = (u1 *)pc;
        new_mb = resolveMethod(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        arg1 = ostack - new_mb->args_count;
        NULLCHECK(arg1[0].o);
        new_mb = class_cb(((Object *)arg1[0].o)->class)->method_table[new_mb->method_table_index];
        goto invokeMethod;

    DEF(INVOKEINTERFACE):
        frame->last_pc = (u1 *)pc;
        new_mb = resolveInterfaceMethod(mb->class, idx2(pc));
        if(ee->exception) goto throwException;
        arg1 = ostack - new_mb->args_count;
        NULLCHECK(arg1[0].o);
        new_mb = lookupMethod(((Object *)arg1[0].o)->class, new_mb->name, new_mb->type);
        goto invokeMethod;

    /* ---- allocation ---- */
    DEF(NEW): {
        Class *class;
        Object *ob;
        frame->last_pc = (u1 *)pc;
        class = resolveClass(mb->class, idx2(pc), TRUE);
        if(ee->exception) goto throwException;
        ob = allocObject(class);
        if(ob == NULL) goto throwException;
        (ostack++)->o = ob;
        NEXT(3);
    }
    DEF(NEWARRAY): {
        int type = pc[1];
        int count = (int)(--ostack)->i;
        Object *ob;
        frame->last_pc = (u1 *)pc;
        ob = allocTypeArray(type, count);
        if(ob == NULL) goto throwException;
        (ostack++)->o = ob;
        NEXT(2);
    }
    DEF(ANEWARRAY): {
        Class *class, *array_class;
        char name[512];
        int count = (int)ostack[-1].i;
        Object *ob;
        frame->last_pc = (u1 *)pc;
        class = resolveClass(mb->class, idx2(pc), FALSE);
        if(ee->exception) goto throwException;
        if(class_cb(class)->name[0] == '[')
            snprintf(name, sizeof(name), "[%s", class_cb(class)->name);
        else
            snprintf(name, sizeof(name), "[L%s;", class_cb(class)->name);
        array_class = findArrayClass(name);
        ob = allocArray(array_class, count, sizeof(Slot));
        if(ob == NULL) goto throwException;
        ostack[-1].o = ob;
        NEXT(3);
    }
    DEF(ARRAYLENGTH): {
        Object *array = ostack[-1].o;
        NULLCHECK(array);
        ostack[-1].i = array_length(array);
        NEXT(1);
    }
    DEF(MULTIANEWARRAY): {
        Class *class;
        int dim = pc[3], i;
        int counts[255];
        Object *ob;
        frame->last_pc = (u1 *)pc;
        class = resolveClass(mb->class, idx2(pc), FALSE);
        if(ee->exception) goto throwException;
        ostack -= dim;
        for(i = 0; i < dim; i++) counts[i] = (int)ostack[i].i;
        ob = allocMultiArray(class, dim, counts);
        if(ob == NULL) goto throwException;
        (ostack++)->o = ob;
        NEXT(4);
    }

    /* ---- array element access ---- */
    DEF(IALOAD): {
        int i = (int)ostack[-1].i; Object *a = ostack[-2].o; ostack--;
        NULLCHECK(a); BOUNDSCHECK(a, i);
        ostack[-1] = ((Slot *)array_body(a))[i]; NEXT(1);
    }
    DEF(IASTORE): {
        Slot v = ostack[-1]; int i = (int)ostack[-2].i; Object *a = ostack[-3].o;
        ostack -= 3; NULLCHECK(a); BOUNDSCHECK(a, i);
        ((Slot *)array_body(a))[i] = v; NEXT(1);
    }
    DEF(WALOAD): {
        int i = (int)ostack[-1].i; Object *a = ostack[-2].o; ostack--;
        NULLCHECK(a); BOUNDSCHECK(a, i);
        ostack[-1].i = ((int32_t *)array_body(a))[i]; NEXT(1);
    }
    DEF(WASTORE): {
        int32_t v = (int32_t)ostack[-1].i; int i = (int)ostack[-2].i; Object *a = ostack[-3].o;
        ostack -= 3; NULLCHECK(a); BOUNDSCHECK(a, i);
        ((int32_t *)array_body(a))[i] = v; NEXT(1);
    }
    DEF(BALOAD): {
        int i = (int)ostack[-1].i; Object *a = ostack[-2].o; ostack--;
        NULLCHECK(a); BOUNDSCHECK(a, i);
        ostack[-1].i = ((int8_t *)array_body(a))[i]; NEXT(1);
    }
    DEF(BASTORE): {
        int8_t v = (int8_t)ostack[-1].i; int i = (int)ostack[-2].i; Object *a = ostack[-3].o;
        ostack -= 3; NULLCHECK(a); BOUNDSCHECK(a, i);
        ((int8_t *)array_body(a))[i] = v; NEXT(1);
    }
    DEF(UBALOAD): {
        int i = (int)ostack[-1].i; Object *a = ostack[-2].o; ostack--;
        NULLCHECK(a); BOUNDSCHECK(a, i);
        ostack[-1].i = ((uint8_t *)array_body(a))[i]; NEXT(1);
    }
    DEF(UBASTORE): {
        uint8_t v = (uint8_t)ostack[-1].i; int i = (int)ostack[-2].i; Object *a = ostack[-3].o;
        ostack -= 3; NULLCHECK(a); BOUNDSCHECK(a, i);
        ((uint8_t *)array_body(a))[i] = v; NEXT(1);
    }
    DEF(CALOAD): {
        int i = (int)ostack[-1].i; Object *a = ostack[-2].o; ostack--;
        NULLCHECK(a); BOUNDSCHECK(a, i);
        ostack[-1].i = ((uint16_t *)array_body(a))[i]; NEXT(1);
    }
    DEF(CASTORE): {
        uint16_t v = (uint16_t)ostack[-1].i; int i = (int)ostack[-2].i; Object *a = ostack[-3].o;
        ostack -= 3; NULLCHECK(a); BOUNDSCHECK(a, i);
        ((uint16_t *)array_body(a))[i] = v; NEXT(1);
    }
    DEF(SALOAD): {
        int i = (int)ostack[-1].i; Object *a = ostack[-2].o; ostack--;
        NULLCHECK(a); BOUNDSCHECK(a, i);
        ostack[-1].i = ((int16_t *)array_body(a))[i]; NEXT(1);
    }
    DEF(SASTORE): {
        int16_t v = (int16_t)ostack[-1].i; int i = (int)ostack[-2].i; Object *a = ostack[-3].o;
        ostack -= 3; NULLCHECK(a); BOUNDSCHECK(a, i);
        ((int16_t *)array_body(a))[i] = v; NEXT(1);
    }

    /* ---- type checks ---- */
    DEF(CHECKCAST): {
        Object *o = ostack[-1].o;
        Class *class;
        frame->last_pc = (u1 *)pc;
        class = resolveClass(mb->class, idx2(pc), TRUE);
        if(ee->exception) goto throwException;
        if(o != NULL && !isInstanceOf(class, o->class)) {
            signalException("saral/lang/ClassCastException", class_cb(o->class)->name);
            goto throwException;
        }
        NEXT(3);
    }
    DEF(INSTANCEOF): {
        Object *o = ostack[-1].o;
        Class *class;
        frame->last_pc = (u1 *)pc;
        class = resolveClass(mb->class, idx2(pc), FALSE);
        if(ee->exception) goto throwException;
        ostack[-1].i = (o != NULL) && isInstanceOf(class, o->class);
        NEXT(3);
    }

    /* ---- monitors / throw ---- */
    DEF(MONITORENTER): { Object *o = (--ostack)->o; NULLCHECK(o); objectLock(o);   NEXT(1); }
    DEF(MONITOREXIT):  { Object *o = (--ostack)->o; NULLCHECK(o); objectUnlock(o); NEXT(1); }

    DEF(ATHROW): {
        Object *o = ostack[-1].o;
        frame->last_pc = (u1 *)pc;
        NULLCHECK(o);
        ee->exception = o;
        goto throwException;
    }

#ifndef THREADED
        default:
            fprintf(stderr, "Unknown opcode %d in %s.%s\n", *pc,
                    class_cb(mb->class)->name, mb->name);
            exit(1);
#endif

    /* =================== shared inline call/return paths ================= */

invokeMethod: {
    Frame *new_frame = (Frame *)(arg1 + new_mb->max_locals);
    Object *sync = NULL;

    if((char *)(new_frame + 1) + new_mb->max_stack * sizeof(Slot) > ee->stack_end) {
        frame->last_pc = (u1 *)pc;
        signalException("saral/lang/StackOverflowError", NULL);
        goto throwException;
    }

    new_frame->mb = new_mb;
    new_frame->lvars = arg1;
    new_frame->ostack = (Slot *)(new_frame + 1);
    new_frame->prev = frame;
    frame->last_pc = (u1 *)pc;
    ee->last_frame = new_frame;

    if(new_mb->access_flags & ACC_SYNCHRONIZED) {
        sync = (new_mb->access_flags & ACC_STATIC) ? (Object *)new_mb->class : arg1[0].o;
        objectLock(sync);
    }

    if(new_mb->access_flags & ACC_NATIVE) {
        Slot *top = ((NativeInvoker)new_mb->native_invoker)(new_mb->class, new_mb, arg1);
        if(sync) objectUnlock(sync);
        ee->last_frame = frame;
        ostack = top;
        if(ee->exception) goto throwException;
        NEXT(3);
    } else {
        frame  = new_frame;
        mb     = new_mb;
        lvars  = arg1;
        ostack = new_frame->ostack;
        cp     = &class_cb(mb->class)->constant_pool;
        pc     = mb->code;
        DISPATCH;
    }
}

throwException: {
    Object *excep = ee->exception;
    u1 *handler;
    clearException();
    handler = findCatchBlock(excep->class);

    if(handler == NULL) {
        ee->exception = excep;   /* propagate out of this executeJava */
        { Slot z; z.p = 0; return z; }
    }

    frame  = ee->last_frame;
    mb     = frame->mb;
    lvars  = frame->lvars;
    cp     = &class_cb(mb->class)->constant_pool;
    ostack = frame->ostack;
    (ostack++)->o = excep;
    pc = handler;
    DISPATCH;
}

#ifndef THREADED
        }   /* switch */
    }       /* while  */
#endif
}
