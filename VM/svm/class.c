/*
 * SVM - class loading, linking, initialization, constant-pool resolution
 *       and runtime type checks (isInstanceOf).
 *
 * Binary module format (.svm), all integers big-endian:
 *
 *   u4  magic            = 0x5341524C  ("SARL")
 *   u2  version
 *   u2  constant_pool_count            (entries indexed 1 .. count-1)
 *   cp[count-1]:
 *       u1 tag
 *       Utf8(1):        u2 len, bytes
 *       Integer(3):     u4
 *       Long(5):        u8
 *       Double(6):      u8
 *       String(8):      u2 utf8_index
 *       Class(7):       u2 name_index
 *       NameAndType(12):u2 name_index, u2 type_index
 *       Fieldref(9)/Methodref(10)/InterfaceMethodref(11):
 *                       u2 class_index, u2 name_and_type_index
 *   u2  access_flags
 *   u2  this_class                     (Class cp index)
 *   u2  super_class                    (Class cp index; 0 = no super / Object)
 *   u2  interfaces_count
 *   u2  interfaces[]                   (Class cp index each)
 *   u2  fields_count
 *   field[]:  u2 access, u2 name_idx, u2 type_idx, u2 constval_idx
 *   u2  methods_count
 *   method[]: u2 access, u2 name_idx, u2 type_idx, u2 max_stack, u2 max_locals,
 *             u4 code_len, code bytes,
 *             u2 etab_count, (u2 start,u2 end,u2 handler,u2 catch)*,
 *             u2 lnt_count,  (u2 start_pc,u2 line_no)*
 *   u2  source_file_index              (Utf8 index, 0 = none)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svm.h"
#include "hash.h"
#include "thread.h"

extern char *internUtf8(const char *s);
extern char *slash2dots(const char *utf8);
extern Slot *resolveNativeWrapper(Class *class, MethodBlock *mb, Slot *args); /* natives.c */

Class *svm_class_Class = NULL;

static HashTable loaded_classes;
static char classpath[16][512];
static int  classpath_count;
static int  verbose;

/* ---- big-endian readers ------------------------------------------------- */

typedef struct { unsigned char *p; unsigned char *end; } Reader;

static u1 rd_u1(Reader *r) { return *r->p++; }
static u2 rd_u2(Reader *r) { u2 v = (r->p[0] << 8) | r->p[1]; r->p += 2; return v; }
static u4 rd_u4(Reader *r) { u4 v = ((u4)r->p[0]<<24)|((u4)r->p[1]<<16)|((u4)r->p[2]<<8)|r->p[3]; r->p += 4; return v; }
static u8 rd_u8(Reader *r) { u8 hi = rd_u4(r); u8 lo = rd_u4(r); return (hi << 32) | lo; }

/* ---- signature scanning ------------------------------------------------- */

/* Advance past one type token in a descriptor.  Every token is one slot. */
static void skipType(char **p) {
    while(**p == '[') (*p)++;
    if(**p == 'L') { while(*(*p)++ != ';'); }
    else (*p)++;
}

/* Count argument slots in a method signature "(...)ret". */
static int argSlots(char *sig) {
    int count = 0;
    char *p = sig + 1;   /* skip '(' */
    while(*p != ')') { skipType(&p); count++; }
    return count;
}

static int returnsValue(char *sig) {
    char *p = sig;
    while(*p != ')') p++;
    return p[1] != 'V';
}

/* ---- hash table hooks --------------------------------------------------- */

/* We key the class table on the (name, loader) pair.  Since names are
 * interned, name comparison is pointer equality. */
typedef struct { char *name; Object *loader; Class *class; } ClassKey;

static int classHash(void *key) {
    ClassKey *k = (ClassKey *)key;
    const char *s = k->name;
    int hash = 0;
    while(*s) hash = hash * 37 + (unsigned char)*s++;
    return hash;
}
static int classCompare(void *key, void *data, int kh, int dh) {
    ClassKey *k = (ClassKey *)key;
    ClassBlock *cb = class_cb((Class *)data);
    return kh == dh && strcmp(k->name, cb->name) == 0 && k->loader == cb->class_loader;
}

/* the value stored is the Class* carried in the key */
static void *classPrepare(void *key) { return ((ClassKey *)key)->class; }

static Class *addClassToHash(Class *class) {
    ClassBlock *cb = class_cb(class);
    ClassKey key; key.name = cb->name; key.loader = cb->class_loader; key.class = class;
    return (Class *)findHashEntry(&loaded_classes, &key,
                                  classHash, classCompare, classPrepare, TRUE);
}

static Class *findHashedClass(char *name, Object *loader) {
    ClassKey key; key.name = internUtf8(name); key.loader = loader; key.class = NULL;
    return (Class *)findHashEntry(&loaded_classes, &key,
                                  classHash, classCompare, classPrepare, FALSE);
}

/* ---- .svm parsing ------------------------------------------------------- */

Class *defineClass(char *expected_name, char *data, int len, Object *loader) {
    Reader r; r.p = (unsigned char *)data; r.end = (unsigned char *)data + len;
    Class *class;
    ClassBlock *cb;
    ConstantPool *cp;
    int i, cp_count;
    u4 magic;

    magic = rd_u4(&r);
    if(magic != 0x5341524C) {
        signalException("saral/lang/ClassFormatError", "bad magic");
        return NULL;
    }
    rd_u2(&r);  /* version */

    class = allocClass();
    cb = class_cb(class);
    memset(cb, 0, sizeof(ClassBlock));
    cb->class_loader = loader;
    cp = &cb->constant_pool;

    /* constant pool */
    cp_count = rd_u2(&r);
    cb->constant_pool_count = cp_count;
    cp->type = calloc(cp_count, sizeof(u1));
    cp->info = calloc(cp_count, sizeof(uintptr_t));

    for(i = 1; i < cp_count; i++) {
        u1 tag = rd_u1(&r);
        switch(tag) {
            case CONSTANT_Utf8: {
                int l = rd_u2(&r);
                char *buff = malloc(l + 1);
                memcpy(buff, r.p, l); buff[l] = '\0'; r.p += l;
                cp->info[i] = (uintptr_t)internUtf8(buff);
                free(buff);
                cp->type[i] = CONSTANT_Utf8;
                break;
            }
            case CONSTANT_Integer:
                cp->info[i] = (uintptr_t)(int)rd_u4(&r);
                cp->type[i] = CONSTANT_Integer;
                break;
            case CONSTANT_Long:
                cp->info[i] = (uintptr_t)rd_u8(&r);
                cp->type[i] = CONSTANT_Long;
                break;
            case CONSTANT_Double: {
                u8 bits = rd_u8(&r);
                cp->info[i] = (uintptr_t)bits;
                cp->type[i] = CONSTANT_Double;
                break;
            }
            case CONSTANT_String:
                cp->info[i] = rd_u2(&r);         /* utf8 index, resolved lazily */
                cp->type[i] = CONSTANT_String;
                break;
            case CONSTANT_Class:
                cp->info[i] = rd_u2(&r);         /* name index, resolved lazily */
                cp->type[i] = CONSTANT_Class;
                break;
            case CONSTANT_NameAndType: {
                u2 name_idx = rd_u2(&r), type_idx = rd_u2(&r);
                cp->info[i] = ((uintptr_t)type_idx << 16) | name_idx;
                cp->type[i] = CONSTANT_NameAndType;
                break;
            }
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref: {
                u2 class_idx = rd_u2(&r), nt_idx = rd_u2(&r);
                cp->info[i] = ((uintptr_t)nt_idx << 16) | class_idx;
                cp->type[i] = tag;
                break;
            }
            default:
                signalException("saral/lang/ClassFormatError", "bad constant tag");
                return NULL;
        }
    }

    cb->access_flags = rd_u2(&r);

    /* this / super */
    {
        u2 this_idx = rd_u2(&r);
        u2 super_idx = rd_u2(&r);
        cb->name = cp_utf8(cp, cp_class(cp, this_idx));
        cb->super_name = super_idx ? cp_utf8(cp, cp_class(cp, super_idx)) : NULL;
    }

    (void)expected_name;

    /* interfaces */
    cb->interfaces_count = rd_u2(&r);
    cb->interfaces = calloc(cb->interfaces_count, sizeof(Class *));
    for(i = 0; i < cb->interfaces_count; i++) {
        u2 idx = rd_u2(&r);
        cb->interfaces[i] = resolveClass(class, idx, FALSE);
    }

    /* fields */
    cb->fields_count = rd_u2(&r);
    cb->fields = calloc(cb->fields_count, sizeof(FieldBlock));
    for(i = 0; i < cb->fields_count; i++) {
        FieldBlock *fb = &cb->fields[i];
        fb->class = class;
        fb->access_flags = rd_u2(&r);
        fb->name = cp_utf8(cp, rd_u2(&r));
        fb->type = cp_utf8(cp, rd_u2(&r));
        fb->constant = rd_u2(&r);
    }

    /* methods */
    cb->methods_count = rd_u2(&r);
    cb->methods = calloc(cb->methods_count, sizeof(MethodBlock));
    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        int j, code_len, etab, lnt;
        mb->class = class;
        mb->access_flags = rd_u2(&r);
        mb->name = cp_utf8(cp, rd_u2(&r));
        mb->type = cp_utf8(cp, rd_u2(&r));
        mb->max_stack = rd_u2(&r);
        mb->max_locals = rd_u2(&r);
        code_len = rd_u4(&r);
        mb->code_length = code_len;
        if(code_len) {
            mb->code = malloc(code_len);
            memcpy(mb->code, r.p, code_len); r.p += code_len;
        }
        etab = rd_u2(&r);
        mb->exception_table_size = etab;
        if(etab) {
            mb->exception_table = calloc(etab, sizeof(ExceptionTableEntry));
            for(j = 0; j < etab; j++) {
                mb->exception_table[j].start_pc   = rd_u2(&r);
                mb->exception_table[j].end_pc     = rd_u2(&r);
                mb->exception_table[j].handler_pc = rd_u2(&r);
                mb->exception_table[j].catch_type = rd_u2(&r);
            }
        }
        lnt = rd_u2(&r);
        mb->line_no_table_size = lnt;
        if(lnt) {
            mb->line_no_table = calloc(lnt, sizeof(LineNoTableEntry));
            for(j = 0; j < lnt; j++) {
                mb->line_no_table[j].start_pc = rd_u2(&r);
                mb->line_no_table[j].line_no  = rd_u2(&r);
            }
        }
        mb->method_table_index = -1;
    }

    /* source file */
    {
        u2 sf = rd_u2(&r);
        cb->source_file_name = sf ? cp_utf8(cp, sf) : NULL;
    }

    cb->flags = CLASS_LOADED;

    /* register (dedup by name+loader) */
    {
        Class *found = addClassToHash(class);
        if(found != class)
            return found;
    }

    /* meta-class pointer */
    if(strcmp(cb->name, "saral/lang/Class") == 0)
        svm_class_Class = class;
    class->class = svm_class_Class;   /* may be NULL until Class is loaded */

    return class;
}

/* ---- loading from the class path --------------------------------------- */

static Class *loadSystemClass(char *name) {
    char path[600];
    int i;
    FILE *f = NULL;

    for(i = 0; i < classpath_count && f == NULL; i++) {
        snprintf(path, sizeof(path), "%s/%s.svm", classpath[i], name);
        f = fopen(path, "rb");
    }
    if(f == NULL) {
        signalException("saral/lang/NoClassDefFoundError", name);
        return NULL;
    }
    {
        long len;
        char *data;
        Class *class;
        fseek(f, 0, SEEK_END); len = ftell(f); fseek(f, 0, SEEK_SET);
        data = malloc(len);
        if(fread(data, 1, len, f) != (size_t)len) { fclose(f); free(data); return NULL; }
        fclose(f);
        class = defineClass(name, data, len, NULL);
        free(data);
        return class;
    }
}

Class *findSystemClass0(char *name) {
    Class *class = findHashedClass(name, NULL);
    if(class == NULL)
        class = loadSystemClass(name);
    if(class != NULL)
        linkClass(class);
    return class;
}

Class *findSystemClass(char *name) {
    Class *class = findSystemClass0(name);
    if(class != NULL)
        class = initClass(class);
    return class;
}

/* ---- array classes ------------------------------------------------------ */

static Class *createArrayClass(char *name, Object *loader) {
    Class *class = allocClass();
    ClassBlock *cb = class_cb(class);
    Class *object_class = findSystemClass("saral/lang/Object");
    ClassBlock *obj_cb = class_cb(object_class);
    Class *found;

    memset(cb, 0, sizeof(ClassBlock));
    cb->name = internUtf8(name);
    cb->super = object_class;
    cb->super_name = obj_cb->name;
    cb->class_loader = loader;
    cb->access_flags = ACC_PUBLIC | ACC_FINAL;
    cb->flags = CLASS_INTERNAL;
    cb->object_size = obj_cb->object_size;
    cb->method_table = obj_cb->method_table;
    cb->method_table_size = obj_cb->method_table_size;

    /* dimension + element class for reference arrays */
    { char *p = name; int dim = 0; while(*p == '[') { dim++; p++; }
      cb->dim = dim;
      if(*p == 'L' || *p == '[')
          cb->element_class = findClassFromClassLoader(name + 1, loader);
    }

    class->class = svm_class_Class;
    found = addClassToHash(class);
    return found;
}

Class *findArrayClassFromClassLoader(char *name, Object *loader) {
    Class *class = findHashedClass(name, loader);
    if(class == NULL)
        class = createArrayClass(name, loader);
    return class;
}

Class *findClassFromClassLoader(char *name, Object *loader) {
    if(name[0] == '[')
        return findArrayClassFromClassLoader(name, loader);
    /* user-defined loaders are not supported in the core build */
    return findSystemClass0(name);
}

/* ---- linking ------------------------------------------------------------ */

void linkClass(Class *class) {
    ClassBlock *cb = class_cb(class);
    Class *super = NULL;
    int i, offset, mtbl_size, new_methods = 0;
    MethodBlock **mtbl;

    if(cb->flags >= CLASS_LINKED)
        return;

    if(cb->super_name != NULL) {
        super = findSystemClass0(cb->super_name);
        cb->super = super;
        if(super) linkClass(super);
    }

    /* field layout: every field occupies exactly one 64-bit slot */
    offset = super ? class_cb(super)->object_size : 0;
    for(i = 0; i < cb->fields_count; i++) {
        FieldBlock *fb = &cb->fields[i];
        if(fb->access_flags & ACC_STATIC) {
            fb->offset = -1;
            fb->static_value.i = 0;
        } else {
            fb->offset = offset++;
        }
    }
    cb->object_size = offset;

    /* method preparation: arg slots, native trampoline, vtable indices */
    mtbl_size = super ? class_cb(super)->method_table_size : 0;
    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        mb->args_count = argSlots(mb->type) + ((mb->access_flags & ACC_STATIC) ? 0 : 1);
        if(mb->access_flags & ACC_NATIVE) {
            mb->native_invoker = (void *)resolveNativeWrapper;
            /* place the (unused) native frame just past the arguments */
            mb->max_locals = mb->args_count;
            mb->max_stack = 0;
        }

        if((mb->access_flags & (ACC_STATIC | ACC_PRIVATE)) || mb->name[0] == '<') {
            mb->method_table_index = -1;
        } else {
            MethodBlock *sm = super ? lookupMethod(super, mb->name, mb->type) : NULL;
            if(sm != NULL && sm->method_table_index >= 0)
                mb->method_table_index = sm->method_table_index;   /* override */
            else
                mb->method_table_index = mtbl_size + new_methods++;
        }
    }

    /* build vtable: copy super's, then install this class's virtuals */
    cb->method_table_size = mtbl_size + new_methods;
    mtbl = calloc(cb->method_table_size, sizeof(MethodBlock *));
    if(super)
        memcpy(mtbl, class_cb(super)->method_table, mtbl_size * sizeof(MethodBlock *));
    for(i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if(mb->method_table_index >= 0)
            mtbl[mb->method_table_index] = mb;
        if(strcmp(mb->name, "finalize") == 0 && strcmp(mb->type, "()V") == 0)
            cb->finalizer = mb;
    }
    cb->method_table = mtbl;

    cb->flags = CLASS_LINKED;
    if(verbose)
        printf("[Linked %s]\n", cb->name);
}

/* ---- initialization ----------------------------------------------------- */

Class *initClass(Class *class) {
    ClassBlock *cb = class_cb(class);
    MethodBlock *clinit;
    Object *co = (Object *)class;
    Thread *self = threadSelf();
    int i;

    linkClass(class);

    objectLock(co);
    while(cb->flags == CLASS_INITING) {
        if(cb->initing_tid == self->id) { objectUnlock(co); return class; }
        objectWait(co, 0, 0);
    }
    if(cb->flags == CLASS_INITED) { objectUnlock(co); return class; }
    if(cb->flags == CLASS_BAD) {
        objectUnlock(co);
        signalException("saral/lang/NoClassDefFoundError", cb->name);
        return NULL;
    }
    cb->flags = CLASS_INITING;
    cb->initing_tid = self->id;
    objectUnlock(co);

    /* init super first */
    if(cb->super && !is_interface(cb))
        initClass(cb->super);

    /* set static final constants */
    for(i = 0; i < cb->fields_count; i++) {
        FieldBlock *fb = &cb->fields[i];
        if((fb->access_flags & ACC_STATIC) && fb->constant) {
            ConstantPool *cp = &cb->constant_pool;
            switch(fb->type[0]) {
                case 'D': fb->static_value.i = (int64_t)cp_info(cp, fb->constant); break;
                case 'J': case 'K': fb->static_value.i = (int64_t)cp_info(cp, fb->constant); break;
                case 'L': fb->static_value.o = (void *)resolveSingleConstant(class, fb->constant); break;
                default:  fb->static_value.i = (int)cp_info(cp, fb->constant); break;
            }
        }
    }

    /* run <clinit> */
    clinit = findMethod(class, "<clinit>", "()V");
    if(clinit != NULL)
        executeMethodArgs(NULL, class, clinit);

    objectLock(co);
    cb->flags = exceptionOccured() ? CLASS_BAD : CLASS_INITED;
    objectNotifyAll(co);
    objectUnlock(co);
    return class;
}

/* ---- member lookup ------------------------------------------------------ */

MethodBlock *findMethod(Class *class, char *name, char *type) {
    ClassBlock *cb = class_cb(class);
    int i;
    for(i = 0; i < cb->methods_count; i++)
        if(strcmp(cb->methods[i].name, name) == 0 &&
           strcmp(cb->methods[i].type, type) == 0)
            return &cb->methods[i];
    return NULL;
}

FieldBlock *findField(Class *class, char *name, char *type) {
    ClassBlock *cb = class_cb(class);
    int i;
    for(i = 0; i < cb->fields_count; i++)
        if(strcmp(cb->fields[i].name, name) == 0 &&
           strcmp(cb->fields[i].type, type) == 0)
            return &cb->fields[i];
    return NULL;
}

MethodBlock *lookupMethod(Class *class, char *name, char *type) {
    for(; class != NULL; class = class_cb(class)->super) {
        MethodBlock *mb = findMethod(class, name, type);
        if(mb) return mb;
    }
    return NULL;
}

FieldBlock *lookupField(Class *class, char *name, char *type) {
    for(; class != NULL; class = class_cb(class)->super) {
        FieldBlock *fb = findField(class, name, type);
        if(fb) return fb;
    }
    return NULL;
}

/* ---- constant-pool resolution ------------------------------------------ */

Class *resolveClass(Class *class, int index, int init) {
    ConstantPool *cp = &class_cb(class)->constant_pool;
    Class *ref;

    if(cp_type(cp, index) == CONSTANT_Resolved)
        ref = (Class *)cp_info(cp, index);
    else {
        char *name = cp_utf8(cp, (u2)cp_info(cp, index));
        ref = findClassFromClass(name, class);
        if(ref == NULL) return NULL;
        cp->info[index] = (uintptr_t)ref;
        cp->type[index] = CONSTANT_Resolved;
    }
    if(init) initClass(ref);
    return ref;
}

MethodBlock *resolveMethod(Class *class, int index) {
    ConstantPool *cp = &class_cb(class)->constant_pool;
    if(cp_type(cp, index) == CONSTANT_Resolved)
        return (MethodBlock *)cp_info(cp, index);
    {
        int cl_idx = cp_class(cp, index);
        int nt_idx = cp_ntype_idx(cp, index);
        Class *cl = resolveClass(class, cl_idx, TRUE);
        char *name, *type;
        MethodBlock *mb;
        if(cl == NULL) return NULL;
        name = cp_utf8(cp, cp_nt_name(cp, nt_idx));
        type = cp_utf8(cp, cp_nt_type(cp, nt_idx));
        mb = lookupMethod(cl, name, type);
        if(mb == NULL) { signalException("saral/lang/NoSuchMethodError", name); return NULL; }
        cp->info[index] = (uintptr_t)mb;
        cp->type[index] = CONSTANT_Resolved;
        return mb;
    }
}

MethodBlock *resolveInterfaceMethod(Class *class, int index) {
    return resolveMethod(class, index);
}

FieldBlock *resolveField(Class *class, int index) {
    ConstantPool *cp = &class_cb(class)->constant_pool;
    if(cp_type(cp, index) == CONSTANT_Resolved)
        return (FieldBlock *)cp_info(cp, index);
    {
        int cl_idx = cp_class(cp, index);
        int nt_idx = cp_ntype_idx(cp, index);
        Class *cl = resolveClass(class, cl_idx, TRUE);
        char *name, *type;
        FieldBlock *fb;
        if(cl == NULL) return NULL;
        name = cp_utf8(cp, cp_nt_name(cp, nt_idx));
        type = cp_utf8(cp, cp_nt_type(cp, nt_idx));
        fb = lookupField(cl, name, type);
        if(fb == NULL) { signalException("saral/lang/NoSuchFieldError", name); return NULL; }
        cp->info[index] = (uintptr_t)fb;
        cp->type[index] = CONSTANT_Resolved;
        return fb;
    }
}

uintptr_t resolveSingleConstant(Class *class, int index) {
    ConstantPool *cp = &class_cb(class)->constant_pool;
    u1 type = cp_type(cp, index);

    if(type == CONSTANT_String) {
        char *utf8 = cp_utf8(cp, (u2)cp_info(cp, index));
        Object *str = findInternedString(createString(utf8));
        cp->info[index] = (uintptr_t)str;
        cp->type[index] = CONSTANT_Resolved;
    }
    /* Integer/Long/Double already hold their value; Resolved returns cached */
    return cp_info(cp, index);
}

/* ---- runtime type checks (was cast.c) ---------------------------------- */

static int isSubClassOf(Class *class, Class *test) {
    for(; test != NULL; test = class_cb(test)->super)
        if(test == class) return TRUE;
    return FALSE;
}

static int implementsInterface(Class *class, Class *test) {
    for(; test != NULL; test = class_cb(test)->super) {
        ClassBlock *cb = class_cb(test);
        int i;
        for(i = 0; i < cb->interfaces_count; i++)
            if(cb->interfaces[i] == class ||
               implementsInterface(class, cb->interfaces[i]))
                return TRUE;
    }
    return FALSE;
}

static int isInstOfArray(Class *class, Class *test) {
    ClassBlock *ccb = class_cb(class), *tcb = class_cb(test);
    if(isSubClassOf(class, test)) return TRUE;
    if(is_array_class(ccb) && is_array_class(tcb) && ccb->dim == tcb->dim &&
       ccb->element_class && tcb->element_class)
        return isInstanceOf(ccb->element_class, tcb->element_class);
    return FALSE;
}

char isInstanceOf(Class *class, Class *test) {
    if(class == test) return TRUE;
    if(is_interface(class_cb(class))) return implementsInterface(class, test);
    if(is_array_class(class_cb(test))) return isInstOfArray(class, test);
    return isSubClassOf(class, test);
}

/* ---- GC root: all loaded classes --------------------------------------- */

void markClasses(void) {
    hashIterate(&loaded_classes, (IterateFn)markClass);
}

/* ---- setup -------------------------------------------------------------- */

void setClassPath(char *path) {
    char *tok = strtok(path, ":");
    classpath_count = 0;
    while(tok && classpath_count < 16) {
        strncpy(classpath[classpath_count++], tok, 511);
        tok = strtok(NULL, ":");
    }
    if(classpath_count == 0)
        strcpy(classpath[classpath_count++], ".");
}

void initialiseClass(int verboseclass) {
    verbose = verboseclass;
    initHashTable(&loaded_classes, 1 << 8);
}
