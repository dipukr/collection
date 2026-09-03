/*
 * SVM - the Saral Virtual Machine
 * -------------------------------
 * A compact, stack-based virtual machine for the Saral object-oriented
 * language.  The design closely follows JamVM 1.0.0 (Robert Lougher, 2003)
 * -- mark/sweep GC, thin locks, pthread threading with signal-based
 * stop-the-world suspension -- but with the following deliberate changes:
 *
 *   1. 64-bit stack/local/field slots (JamVM assumed 32-bit pointers and
 *      used two slots for long/double; SVM uses one 64-bit `Slot` for every
 *      value, which is correct on modern x86-64 and simplifies the
 *      interpreter enormously).
 *   2. `static inline` functions are used in place of function-like macros
 *      wherever practical (the sole unavoidable macro is the computed-goto
 *      DISPATCH in the interpreter).
 *   3. Its own binary module format (`.svm`) and a textual assembler (`sasm`).
 *
 * Target platform: POSIX (Linux / WSL), gcc.
 *
 * Copyright (C) 2026.  Distributed under the GNU GPL v2 (as JamVM is).
 */

#ifndef SVM_H
#define SVM_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/* ---- Fundamental slot types --------------------------------------------- */

/* Every operand-stack entry, local variable and object field is one 64-bit
 * Slot.  A Slot can hold any Saral primitive or an object reference. */
typedef union slot {
    int64_t   i;   /* bool, byte, ubyte, char, int, uint, long (sign chosen by op) */
    uint64_t  u;   /* unsigned view */
    double    d;   /* num */
    void     *o;   /* object reference */
    uintptr_t p;   /* raw pointer / return-address view */
} Slot;

typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

/* ---- Saral primitive type tags (used by arrays / reflection) ------------ */

enum PrimType {
    T_VOID  = 0,
    T_BOOL  = 1,
    T_BYTE  = 2,   /* signed 8  */
    T_UBYTE = 3,   /* unsigned 8 */
    T_CHAR  = 4,   /* unsigned 16 */
    T_INT   = 5,   /* signed 32 */
    T_UINT  = 6,   /* unsigned 32 */
    T_LONG  = 7,   /* signed 64 */
    T_ULONG = 8,   /* unsigned 64 */
    T_NUM   = 9,   /* double 64 */
    T_REF   = 10   /* object reference (array of objects) */
};

/* ---- Access flags -------------------------------------------------------- */

#define ACC_PUBLIC       0x0001
#define ACC_PRIVATE      0x0002
#define ACC_PROTECTED    0x0004
#define ACC_STATIC       0x0008
#define ACC_FINAL        0x0010
#define ACC_SYNCHRONIZED 0x0020
#define ACC_NATIVE       0x0100
#define ACC_INTERFACE    0x0200
#define ACC_ABSTRACT     0x0400

/* ---- Class lifecycle states --------------------------------------------- */

#define CLASS_LOADED   0
#define CLASS_LINKED   1
#define CLASS_BAD      2
#define CLASS_INITING  3
#define CLASS_INITED   4
#define CLASS_INTERNAL 5   /* array / primitive classes synthesised by the VM */

/* ---- Constant-pool entry tags ------------------------------------------- */

enum CpTag {
    CONSTANT_Utf8               = 1,
    CONSTANT_Integer            = 3,  /* int / uint / bool / byte / char literal */
    CONSTANT_Long               = 5,  /* long / ulong literal */
    CONSTANT_Double             = 6,  /* num literal */
    CONSTANT_String             = 8,
    CONSTANT_Class              = 7,
    CONSTANT_Fieldref           = 9,
    CONSTANT_Methodref          = 10,
    CONSTANT_InterfaceMethodref = 11,
    CONSTANT_NameAndType        = 12,

    CONSTANT_Resolved           = 20, /* entry has been resolved to a pointer */
    CONSTANT_Locked             = 21  /* entry is being resolved by some thread */
};

/* ========================================================================= */
/*  Instruction set                                                          */
/* ========================================================================= */
/*
 * SVM bytecode.  Operand-stack based.  Because slots are 64-bit, there is a
 * single load/store family and single-width arithmetic per logical type.
 * Signedness is encoded in the opcode where it matters (division, shift,
 * comparison, widening).  Branch offsets are 2-byte signed, relative to the
 * start of the branch instruction.  Constant-pool / local indices are 2-byte.
 */
enum Opcode {
    OP_NOP           = 0,

    /* -- constants -- */
    OP_ACONST_NULL   = 1,
    OP_ICONST        = 2,   /* i4  : push signed 32-bit immediate (sign-extended) */
    OP_LDC           = 3,   /* u2  : push constant-pool constant (int/long/num/string) */

    /* -- locals -- */
    OP_LOAD          = 4,   /* u2  : push local[index] */
    OP_STORE         = 5,   /* u2  : local[index] = pop */
    OP_IINC          = 6,   /* u2,i2 : local[index] += signed immediate */

    /* -- stack manipulation -- */
    OP_POP           = 7,
    OP_DUP           = 8,
    OP_DUP_X1        = 9,
    OP_SWAP          = 10,

    /* -- arithmetic (integer, 64-bit two's complement on .i) -- */
    OP_IADD          = 11,
    OP_ISUB          = 12,
    OP_IMUL          = 13,
    OP_IDIV          = 14,  /* signed */
    OP_IREM          = 15,  /* signed */
    OP_UDIV          = 16,  /* unsigned */
    OP_UREM          = 17,  /* unsigned */
    OP_INEG          = 18,

    /* -- floating point (num / double) -- */
    OP_DADD          = 19,
    OP_DSUB          = 20,
    OP_DMUL          = 21,
    OP_DDIV          = 22,
    OP_DREM          = 23,
    OP_DNEG          = 24,

    /* -- bitwise / shift -- */
    OP_IAND          = 25,
    OP_IOR           = 26,
    OP_IXOR          = 27,
    OP_INOT          = 28,  /* ~ */
    OP_ISHL          = 29,
    OP_ISHR          = 30,  /* arithmetic (signed) */
    OP_IUSHR         = 31,  /* logical  (unsigned) */

    /* -- conversions -- */
    OP_I2D           = 32,
    OP_D2I           = 33,
    OP_U2D           = 34,  /* unsigned int -> double */
    OP_I2B           = 35,  /* truncate to signed 8  */
    OP_I2UB          = 36,  /* truncate to unsigned 8 */
    OP_I2C           = 37,  /* truncate to unsigned 16 */
    OP_I2S           = 38,  /* truncate to signed 16 */
    OP_I2I           = 39,  /* truncate to signed 32 */
    OP_I2U           = 40,  /* truncate to unsigned 32 */

    /* -- comparisons producing -1/0/1 -- */
    OP_LCMP          = 41,  /* signed 64-bit compare */
    OP_ULCMP         = 42,  /* unsigned 64-bit compare */
    OP_DCMPL         = 43,  /* NaN -> -1 */
    OP_DCMPG         = 44,  /* NaN -> +1 */

    /* -- unconditional / computed branches -- */
    OP_GOTO          = 45,  /* i2 */

    /* -- conditional branches on top-of-stack vs 0 (signed) -- */
    OP_IFEQ          = 46,
    OP_IFNE          = 47,
    OP_IFLT          = 48,
    OP_IFGE          = 49,
    OP_IFGT          = 50,
    OP_IFLE          = 51,

    /* -- conditional branches comparing two ints -- */
    OP_IF_ICMPEQ     = 52,
    OP_IF_ICMPNE     = 53,
    OP_IF_ICMPLT     = 54,
    OP_IF_ICMPGE     = 55,
    OP_IF_ICMPGT     = 56,
    OP_IF_ICMPLE     = 57,

    /* -- reference equality / null tests -- */
    OP_IF_ACMPEQ     = 58,
    OP_IF_ACMPNE     = 59,
    OP_IFNULL        = 60,
    OP_IFNONNULL     = 61,

    /* -- switch -- */
    OP_TABLESWITCH   = 62,
    OP_LOOKUPSWITCH  = 63,

    /* -- returns -- */
    OP_RETURN        = 64,  /* void */
    OP_IRETURN       = 65,  /* value (any single slot) */

    /* -- fields -- */
    OP_GETSTATIC     = 66,  /* u2 cp */
    OP_PUTSTATIC     = 67,
    OP_GETFIELD      = 68,
    OP_PUTFIELD      = 69,

    /* -- method invocation -- */
    OP_INVOKEVIRTUAL   = 70,  /* u2 cp */
    OP_INVOKESPECIAL   = 71,  /* <init> / private / super */
    OP_INVOKESTATIC    = 72,
    OP_INVOKEINTERFACE = 73,

    /* -- object / array creation -- */
    OP_NEW           = 74,  /* u2 cp class */
    OP_NEWARRAY      = 75,  /* u1 primtype ; count on stack */
    OP_ANEWARRAY     = 76,  /* u2 cp class ; count on stack */
    OP_ARRAYLENGTH   = 77,
    OP_MULTIANEWARRAY= 78,  /* u2 cp class, u1 dims */

    /* -- array element access -- */
    OP_IALOAD        = 79,  /* generic 64-bit-slot load for int/uint/long/ulong/num/ref  */
    OP_IASTORE       = 80,
    OP_BALOAD        = 81,  /* byte[]  (signed 8) */
    OP_BASTORE       = 82,
    OP_UBALOAD       = 83,  /* ubyte[] (unsigned 8) */
    OP_UBASTORE      = 84,
    OP_CALOAD        = 85,  /* char[]  (unsigned 16) */
    OP_CASTORE       = 86,
    OP_SALOAD        = 87,  /* (signed 16, used for the 'short' storage class) */
    OP_SASTORE       = 88,
    OP_WALOAD        = 89,  /* 32-bit int/uint element */
    OP_WASTORE       = 90,

    /* -- type checks -- */
    OP_CHECKCAST     = 91,  /* u2 cp */
    OP_INSTANCEOF    = 92,

    /* -- exceptions / monitors -- */
    OP_ATHROW        = 93,
    OP_MONITORENTER  = 94,
    OP_MONITOREXIT   = 95,

    OP_MAX           = 96
};

/* ========================================================================= */
/*  Core runtime structures                                                  */
/* ========================================================================= */

typedef struct constant_pool {
    volatile u1 *type;   /* CpTag per entry */
    uintptr_t   *info;   /* raw value or resolved pointer per entry */
} ConstantPool;

typedef struct exception_table_entry {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;   /* cp index of caught class, or 0 = finally / catch-all */
} ExceptionTableEntry;

typedef struct line_no_table_entry {
    u2 start_pc;
    u2 line_no;
} LineNoTableEntry;

/* Minimal object header: lock word + class pointer.  Instance data follows
 * immediately (see inst_data()).  A Class is layout-compatible so that a
 * Class object can itself be treated as an Object during GC. */
typedef struct object {
    volatile uintptr_t lock;
    struct class      *class;
} Object;

typedef struct class {
    volatile uintptr_t lock;
    struct class      *class;   /* meta-class: always svm_class_Class */
} Class;

typedef struct methodblock {
    Class *class;
    char  *name;
    char  *type;               /* signature, e.g. "(II)I" */
    u2     access_flags;
    u2     max_stack;
    u2     max_locals;
    u2     args_count;         /* argument slots incl. `this` for instance methods */
    u2     exception_table_size;
    u2     line_no_table_size;
    u1    *code;
    u4     code_length;
    ExceptionTableEntry *exception_table;
    LineNoTableEntry    *line_no_table;
    int    method_table_index; /* vtable slot, -1 for static/private/<init> */
    void  *native_invoker;     /* fn ptr for native dispatch (see natives) */
} MethodBlock;

typedef struct fieldblock {
    Class *class;
    char  *name;
    char  *type;
    u2     access_flags;
    u2     constant;           /* cp index of ConstantValue, 0 if none */
    int    offset;             /* instance: slot index into object body     */
    Slot   static_value;       /* static:   the value itself                */
} FieldBlock;

typedef struct classblock {
    /* NOTE: the first two words mirror Object (lock, class) because the
     * enclosing Class* is what callers hold; CLASS_CB() steps past them. */
    char  *name;
    char  *source_file_name;
    Class *super;
    char  *super_name;
    u2     access_flags;
    u2     flags;              /* CLASS_LOADED ... CLASS_INTERNAL */
    u2     interfaces_count;
    u2     fields_count;
    u2     methods_count;
    u2     constant_pool_count;
    int    object_size;        /* number of instance Slots */
    FieldBlock  *fields;
    MethodBlock *methods;
    Class      **interfaces;
    ConstantPool constant_pool;
    int          method_table_size;
    MethodBlock **method_table;
    MethodBlock *finalizer;
    Class       *element_class; /* array element type (arrays only) */
    int          initing_tid;
    int          dim;           /* array dimensionality */
    Object      *class_loader;  /* NULL = bootstrap loader */
} ClassBlock;

/* A Java(=Saral)-level activation record on the interpreter stack. */
typedef struct frame {
    MethodBlock  *mb;
    u1           *last_pc;      /* pc of the in-progress call/throw site */
    Slot         *lvars;
    Slot         *ostack;
    struct frame *prev;
} Frame;

typedef struct exec_env {
    Object *exception;
    char   *stack;             /* base of malloc'd interpreter stack */
    char   *stack_end;         /* soft limit (overflow guard) */
    Frame  *last_frame;
    Object *thread;            /* the saral Thread object */
} ExecEnv;

/* ---- Inline accessors (replacing JamVM's macros) ------------------------ */

static inline ClassBlock *class_cb(Class *cls) {
    return (ClassBlock *)(cls + 1);
}
static inline Slot *inst_data(Object *ob) {
    return (Slot *)(ob + 1);
}
static inline int is_interface(ClassBlock *cb) {
    return (cb->access_flags & ACC_INTERFACE) != 0;
}
static inline int is_array_class(ClassBlock *cb) {
    return cb->name[0] == '[';
}
static inline int is_primitive(ClassBlock *cb) {
    return cb->flags == CLASS_INTERNAL && cb->name[0] != '[';
}

/* Array layout: [ header | int length | packed elements... ].  The length
 * lives in the first instance slot; element storage begins after it. */
static inline int array_length(Object *array) {
    return (int)inst_data(array)[0].i;
}
static inline void *array_body(Object *array) {
    return (void *)(inst_data(array) + 1);
}

/* Constant-pool readers (packed two u2 into one word for composite refs). */
static inline u1 cp_type(ConstantPool *cp, int i)      { return cp->type[i]; }
static inline uintptr_t cp_info(ConstantPool *cp, int i){ return cp->info[i]; }
static inline u2 cp_class(ConstantPool *cp, int i)      { return (u2)cp->info[i]; }
static inline u2 cp_ntype_idx(ConstantPool *cp, int i)  { return (u2)(cp->info[i] >> 16); }
static inline u2 cp_nt_name(ConstantPool *cp, int i)    { return (u2)cp->info[i]; }
static inline u2 cp_nt_type(ConstantPool *cp, int i)    { return (u2)(cp->info[i] >> 16); }
static inline char *cp_utf8(ConstantPool *cp, int i)    { return (char *)cp->info[i]; }

/* ========================================================================= */
/*  Function prototypes                                                       */
/* ========================================================================= */

/* --- heap / gc (heap.c) --- */
extern void   initialiseAlloc(size_t min, size_t max, int verbose);
extern void   initialiseGC(int noasyncgc);
extern Class *allocClass(void);
extern Object *allocObject(Class *class);
extern Object *allocTypeArray(int type, int size);
extern Object *allocArray(Class *class, int size, int el_size);
extern Object *allocMultiArray(Class *array_class, int dim, int *counts);
extern Object *cloneObject(Object *ob);
extern int  gc(void);                 /* run a synchronous collection */
extern long freeHeapMem(void);
extern long totalHeapMem(void);
extern long maxHeapMem(void);
/* GC-internal marking hooks (implemented across modules) */
extern void markObject(Object *ob);
extern void markClass(Class *class);
extern void markChildren(Object *ob);
extern void markClasses(void);
extern void markInternedStrings(void);
extern void scanThreads(void);
extern void scanThread(struct thread *thread);
extern int  isObjectRef(void *ptr);   /* conservative pointer test */

/* --- class / resolve / cast (class.c) --- */
extern Class *svm_class_Class;
extern Class *defineClass(char *name, char *data, int len, Object *loader);
extern Class *findSystemClass(char *name);
extern Class *findSystemClass0(char *name);   /* load+link, no init */
extern Class *initClass(Class *class);
extern Class *findArrayClassFromClassLoader(char *name, Object *loader);
extern Class *findClassFromClassLoader(char *name, Object *loader);
extern void   linkClass(Class *class);
extern FieldBlock  *findField(Class *class, char *name, char *type);
extern MethodBlock *findMethod(Class *class, char *name, char *type);
extern FieldBlock  *lookupField(Class *class, char *name, char *type);
extern MethodBlock *lookupMethod(Class *class, char *name, char *type);
extern Class       *resolveClass(Class *class, int index, int init);
extern MethodBlock *resolveMethod(Class *class, int index);
extern MethodBlock *resolveInterfaceMethod(Class *class, int index);
extern FieldBlock  *resolveField(Class *class, int index);
extern uintptr_t    resolveSingleConstant(Class *class, int index);
extern char isInstanceOf(Class *class, Class *test); /* is `test` a subtype of `class`? */
extern void setClassPath(char *path);
extern void initialiseClass(int verbose);

#define findArrayClass(name) findArrayClassFromClassLoader(name, NULL)
static inline Class *findArrayClassFromClass(char *name, Class *cls) {
    return findArrayClassFromClassLoader(name, class_cb(cls)->class_loader);
}
static inline Class *findClassFromClass(char *name, Class *cls) {
    return findClassFromClassLoader(name, class_cb(cls)->class_loader);
}

/* --- execution (execute.c / interp.c) --- */
extern Slot  executeJava(void);       /* interpreter loop; returns top result slot */
extern Slot  executeMethodArgs(Object *ob, Class *class, MethodBlock *mb, ...);
extern Slot  executeMethodVaList(Object *ob, Class *class, MethodBlock *mb, va_list args);
extern Slot  executeMethodList(Object *ob, Class *class, MethodBlock *mb, Slot *args);
/* Convenience: call a method whose class is the receiver's class. */
#define executeMethod(ob, mb, ...) executeMethodArgs(ob, (ob)->class, mb, ##__VA_ARGS__)

/* --- exceptions (excep.c) --- */
extern Object *exceptionOccured(void);
extern void    signalException(char *excep_name, char *message);
extern void    setException(Object *excep);
extern void    clearException(void);
extern void    printException(void);
extern u1     *findCatchBlock(Class *exception);
extern void    setStackTrace(Object *excep);
extern void    printStackTrace(Object *excep);

/* --- strings / utf8 (string.c) --- */
extern Object *createString(const char *utf8);
extern Object *findInternedString(Object *string);
extern char   *string2Cstr(Object *string);
extern Object *cstr2String(const char *cstr);
extern void    initialiseString(void);
extern char   *internUtf8(const char *s);   /* dedup a raw C string */
extern void    initialiseUtf8(void);

/* --- threading (thread.c) --- */
extern void      initialiseMainThread(int java_stack);
extern ExecEnv  *getExecEnv(void);
extern void      createSaralThread(Object *thread_obj);
extern void      createVMThread(char *name, void (*start)(struct thread *));
extern void      mainThreadWaitToExitVM(void);
extern void      suspendAllThreads(struct thread *self);
extern void      resumeAllThreads(struct thread *self);
extern void      initialiseSignals(void);

/* --- monitors (lock.c) --- */
extern void initialiseMonitor(void);
extern void objectLock(Object *ob);
extern void objectUnlock(Object *ob);
extern void objectWait(Object *ob, long long ms, int ns);
extern void objectNotify(Object *ob);
extern void objectNotifyAll(Object *ob);

/* --- natives (natives.c) --- */
extern void  initialiseNatives(void);
extern void *lookupInternalNative(MethodBlock *mb);

/* --- VM globals --- */
extern char VM_initing;

#endif /* SVM_H */
