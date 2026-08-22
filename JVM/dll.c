#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jam.h"

#ifndef NO_JNI
#include <dlfcn.h>
#include "hash.h"
#include "jni.h"

/* Trace library loading and method lookup */
#ifdef TRACEDLL
#define TRACE(x) printf x
#else
#define TRACE(x)
#endif

#define HASHTABSZE 1<<4
static HashTable hash_table;

void *lookupLoadedDlls(MethodBlock *mb);
#endif

int nativeExtraArg(MethodBlock *mb) {
    return (mb->access_flags & ACC_STATIC ? mb->args_count + 1 : mb->args_count) + 1;
}

u4 *callJNIMethod(void *env, Class *class, char *sig, int extra, u4 *ostack, unsigned char *f) {
    u4 args[extra];
    u4 *opntr = ostack;
    u4 *apntr = &args[2];

    args[0] = (u4) env;
    args[1] = class ? (u4) class : *opntr++;

    SCAN_SIG(sig, *((u8 *) apntr)++ = *((u8 *) opntr)++, *apntr++ = *opntr++);

    switch (*sig) {
        case 'V':
            (*(void (*)()) f)();
            break;

        case 'D':
            *((double *) ostack)++ = (*(double (*)()) f)();
            break;

        case 'F':
            *((float *) ostack)++ = (*(float (*)()) f)();
            break;

        case 'J':
            *((long long *) ostack)++ = (*(long long (*)()) f)();
            break;

        default:
            *ostack++ = (*(u4 (*)()) f)();
            break;
    }

    return ostack;
}

char *mangleString(unsigned char *utf8) {
    int len = utf8Len(utf8);
    unsigned short *unicode = (unsigned short *) sysMalloc(len * 2);
    char *mangled, *mngldPtr;
    int i, mangledLen = 0;

    convertUtf8(utf8, unicode);

    /* Work out the length of the mangled string */

    for (i = 0; i < len; i++) {
        unsigned short c = unicode[i];
        if (c > 255)
            mangledLen += 6;
        else
            switch (c) {
                case '_':
                case ';':
                case '[':
                    mangledLen += 2;
                    break;

                default:
                    mangledLen++;
                    break;
            }
    }

    mangled = mngldPtr = (char *) sysMalloc(mangledLen + 1);

    /* Construct the mangled string */

    for (i = 0; i < len; i++) {
        unsigned short c = unicode[i];
        if (c > 255)
            mngldPtr += sprintf(mngldPtr, "_0%04X", c);
        else
            switch (c) {
                case '_':
                    *mngldPtr++ = '_';
                    *mngldPtr++ = '1';
                    break;
                case ';':
                    *mngldPtr++ = '_';
                    *mngldPtr++ = '2';
                    break;
                case '[':
                    *mngldPtr++ = '_';
                    *mngldPtr++ = '3';
                    break;

                case '/':
                    *mngldPtr++ = '_';
                    break;

                default:
                    *mngldPtr++ = c;
                    break;
            }
    }

    *mngldPtr = '\0';

    free(unicode);
    return mangled;
}

char *mangleClassAndMethodName(MethodBlock *mb) {
    char *classname = CLASS_CB(mb->class)->name;
    char *methodname = mb->name;
    char *nonMangled = (char *) sysMalloc(strlen(classname) + strlen(methodname) + 7);
    char *mangled;

    sprintf(nonMangled, "Java/%s/%s", classname, methodname);

    mangled = mangleString(nonMangled);
    free(nonMangled);
    return mangled;
}

char *mangleSignature(MethodBlock *mb) {
    char *type = mb->type;
    char *nonMangled;
    char *mangled;
    int i;

    /* find ending ) */
    for (i = strlen(type) - 1; type[i] != ')'; i--);

    nonMangled = (char *) sysMalloc(i);
    strncpy(nonMangled, type + 1, i - 1);
    nonMangled[i - 1] = '\0';

    mangled = mangleString(nonMangled);
    free(nonMangled);
    return mangled;
}

void *lookupInternal(MethodBlock *mb) {
    int i;

    if (strncmp(CLASS_CB(mb->class)->name, "java/lang/", 10) != 0)
        return NULL;

    TRACE(("<DLL: Looking up %s internally>\n", mb->name));

    for (i = 0; native_methods[i][0] &&
                (strcmp(mb->name, native_methods[i][0]) != 0); i++);

    if (native_methods[i][0])
        return mb->native_invoker = (void *) native_methods[i][1];
    else
        return NULL;
}

void *resolveNativeMethod(MethodBlock *mb) {
    void *func = lookupInternal(mb);

#ifndef NO_JNI
    if (func == NULL)
        func = lookupLoadedDlls(mb);
#endif

    return func;
}

u4 *resolveNativeWrapper(Class *class, MethodBlock *mb, u4 *ostack) {
    void *func = resolveNativeMethod(mb);

    if (func == NULL) {
        signalException("java/lang/UnsatisfiedLinkError", mb->name);
        return ostack;
    }
    return (*(u4 *(*)(Class *, MethodBlock *, u4 *)) func)(class, mb, ostack);
}

void initialiseDll() {
#ifndef NO_JNI
    initHashTable(hash_table, HASHTABSZE);
#endif
}

#ifndef NO_JNI
typedef struct {
    char *name;
    void *handle;
} DllEntry;

int dllNameHash(char *name) {
    int hash = 0;

    while (*name)
        hash = hash * 37 + *name++;

    return hash;
}

int resolveDll(char *name) {
    DllEntry *dll;

    TRACE(("<DLL: Attempting to resolve library %s>\n", name));

#define HASH(ptr) dllNameHash(ptr)
#define COMPARE(ptr1, ptr2, hash1, hash2) \
                  ((hash1 == hash2) && (strcmp(ptr1, ptr2->name) == 0))
#define PREPARE(ptr) ptr
#define SCAVENGE(ptr) FALSE
#define FOUND(ptr)

    findHashEntry(hash_table, name, dll, FALSE, FALSE);

    if (dll == NULL) {
        DllEntry *dll2;
        void *handle = dlopen(name, RTLD_LAZY);

        if (handle == NULL)
            return 0;

        TRACE(("<DLL: Successfully opened library %s>\n",name));

        dll = (DllEntry *) sysMalloc(sizeof(DllEntry));
        if (dll == NULL)
            return -1;

        dll->name = strcpy((char *) sysMalloc(strlen(name) + 1), name);
        dll->handle = handle;

#undef HASH
#undef COMPARE
#define HASH(ptr) dllNameHash(ptr->name)
#define COMPARE(ptr1, ptr2, hash1, hash2) \
                  ((hash1 == hash2) && (strcmp(ptr1->name, ptr2->name) == 0))

        findHashEntry(hash_table, dll, dll2, TRUE, FALSE);
    }

    return 1;
}

char *getDllPath() {
    return getenv("LD_LIBRARY_PATH");
}

char *getDllName(char *path, char *name) {
    char *buff = sysMalloc(strlen(path) + strlen(name) + 8);

    sprintf(buff, "%s/lib%s.so", path, name);
    return buff;
}

void *lookupLoadedDlls0(char *name) {
    TRACE(("<DLL: Looking up %s in loaded DLL's>\n", name));

#define ITERATE(ptr)                                   \
{                                                      \
    void *sym = dlsym(((DllEntry*)ptr)->handle, name); \
    if(sym) return sym;                                \
}

    hashIterate(hash_table);
    return NULL;
}

extern int nativeExtraArg(MethodBlock *mb);

extern u4 *callJNIMethod(void *env, Class *class, char *sig, int extra, u4 *ostack, unsigned char *native_func,
                         int args);

extern struct _JNINativeInterface Jam_JNINativeInterface;

extern void initJNILrefs();

u4 *callJNIWrapper(Class *class, MethodBlock *mb, u4 *ostack) {
    void *env = &Jam_JNINativeInterface;

    TRACE(("<DLL: Calling JNI method %s%s>\n", mb->name, mb->type));

    initJNILrefs();
    return callJNIMethod(&env, (mb->access_flags & ACC_STATIC) ? class : NULL,
                         mb->type, mb->native_extra_arg, ostack, mb->code, mb->args_count);
}

void *lookupLoadedDlls(MethodBlock *mb) {
    char *mangled = mangleClassAndMethodName(mb);
    void *func = lookupLoadedDlls0(mangled);

    if (func == NULL) {
        char *mangledSig = mangleSignature(mb);
        if (*mangledSig != '\0') {
            char *fullyMangled = (char *) sysMalloc(strlen(mangled) + strlen(mangledSig) + 3);
            sprintf(fullyMangled, "%s__%s", mangled, mangledSig);

            func = lookupLoadedDlls0(fullyMangled);
            free(fullyMangled);
        }
        free(mangledSig);
    }
    free(mangled);

    if (func) {
        mb->code = (unsigned char *) func;
        mb->native_extra_arg = nativeExtraArg(mb);
        return mb->native_invoker = (void *) callJNIWrapper;
    }

    return NULL;
}
#endif
