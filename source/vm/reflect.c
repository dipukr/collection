#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jam.h"
#include "frame.h"
#include "lock.h"

Class *convertSigElement2Class(char **sig_pntr, Class *declaring_class) {
    char *sig = *sig_pntr;
    Class *class;

    switch (*sig) {
        case '[': {
            char next;
            while (*++sig == '[');
            if (*sig == 'L')
                while (*++sig != ';');
            next = *++sig;
            *sig = '\0';
            class = findArrayClassFromClass(*sig_pntr, declaring_class);
            *sig = next;
            break;
        }

        case 'L':
            while (*++sig != ';');
            *sig++ = '\0';
            class = findClassFromClass((*sig_pntr)+1, declaring_class);
            break;

        default:
            class = findPrimitiveClass(*sig++);
            break;
    }
    *sig_pntr = sig;
    return class;
}

Object *convertSig2ClassArray(char **sig_pntr, Class *declaring_class) {
    char *sig = *sig_pntr;
    int no_params, i = 0;

    Class **params, *array_class = findArrayClass("[Ljava/lang/Class;");
    Object *array;

    for (no_params = 0; *++sig != ')'; no_params++) {
        if (*sig == '[')
            while (*++sig == '[');
        if (*sig == 'L')
            while (*++sig != ';');
    }

    if ((array = allocArray(array_class, no_params, 4)) == NULL)
        return NULL;

    params = (Class **) &INST_DATA(array)[1];

    *sig_pntr += 1;
    while (**sig_pntr != ')')
        if ((params[i++] = convertSigElement2Class(sig_pntr, declaring_class)) == NULL)
            return NULL;

    return array;
}

Object *getExceptionTypes(MethodBlock *mb) {
    int i;
    Object *array;
    Class **excps, *array_class = findArrayClass("[Ljava/lang/Class;");

    if ((array = allocArray(array_class, mb->throw_table_size, 4)) == NULL)
        return NULL;

    excps = (Class **) &INST_DATA(array)[1];

    for (i = 0; i < mb->throw_table_size; i++)
        if ((excps[i] = resolveClass(mb->class, mb->throw_table[i], FALSE)) == NULL)
            return NULL;

    return array;
}

Object *getClassConstructors(Class *class, int public) {
    Class *array_class = findArrayClass("[Ljava/lang/reflect/Constructor;");
    Class *reflect_class = findSystemClass("java/lang/reflect/Constructor");
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **cons;
    MethodBlock *init_mb;
    int count = 0;
    int i, j;

    if (!array_class || !reflect_class)
        return NULL;

    if (!(init_mb = findMethod(reflect_class, "<init>", "(Ljava/lang/Class;[Ljava/lang/Class;[Ljava/lang/Class;I)V")))
        return NULL;

    for (i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if ((strcmp(mb->name, "<init>") == 0) && (!public || (mb->access_flags & ACC_PUBLIC)))
            count++;
    }

    if ((array = allocArray(array_class, count, 4)) == NULL)
        return NULL;
    cons = (Object **) &INST_DATA(array)[1];

    for (i = 0, j = 0; j < count; i++) {
        MethodBlock *mb = &cb->methods[i];

        if ((strcmp(mb->name, "<init>") == 0) && (!public || (mb->access_flags & ACC_PUBLIC))) {
            Object *reflect_ob;

            if ((reflect_ob = allocObject(reflect_class))) {
                char *signature = sysMalloc(strlen(mb->type) + 1);
                char *sig = signature;
                Object *classes, *exceps;

                strcpy(sig, mb->type);
                classes = convertSig2ClassArray(&sig, mb->class);
                exceps = getExceptionTypes(mb);
                free(signature);

                if ((classes == NULL) || (exceps == NULL))
                    return NULL;

                executeMethod(reflect_ob, init_mb, mb->class, classes, exceps, mb);
                cons[j++] = reflect_ob;
            } else
                return NULL;
        }
    }
    return array;
}

Object *getClassMethods(Class *class, int public) {
    Class *array_class = findArrayClass("[Ljava/lang/reflect/Method;");
    Class *reflect_class = findSystemClass("java/lang/reflect/Method");
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **methods;
    MethodBlock *init_mb;
    int count = 0;
    int i, j;

    if (!array_class || !reflect_class)
        return NULL;

    if (!(init_mb = findMethod(reflect_class, "<init>",
                               "(Ljava/lang/Class;[Ljava/lang/Class;[Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/String;I)V")))
        return NULL;

    for (i = 0; i < cb->methods_count; i++) {
        MethodBlock *mb = &cb->methods[i];
        if ((mb->name[0] != '<') && (!public || (mb->access_flags & ACC_PUBLIC)))
            count++;
    }

    if ((array = allocArray(array_class, count, 4)) == NULL)
        return NULL;
    methods = (Object **) &INST_DATA(array)[1];

    for (i = 0, j = 0; j < count; i++) {
        MethodBlock *mb = &cb->methods[i];

        if ((mb->name[0] != '<') && (!public || (mb->access_flags & ACC_PUBLIC))) {
            Object *reflect_ob;

            if ((reflect_ob = allocObject(reflect_class))) {
                char *signature = sysMalloc(strlen(mb->type) + 1);
                char *sig = signature;
                Object *classes, *exceps, *name;
                Class *ret;

                strcpy(sig, mb->type);
                classes = convertSig2ClassArray(&sig, mb->class);
                exceps = getExceptionTypes(mb);
                name = createString(mb->name);

                sig++;
                ret = convertSigElement2Class(&sig, mb->class);
                free(signature);

                if ((classes == NULL) || (exceps == NULL) || (name == NULL) || (ret == NULL))
                    return NULL;

                executeMethod(reflect_ob, init_mb, mb->class, classes, exceps, ret, name, mb);
                methods[j++] = reflect_ob;
            } else
                return NULL;
        }
    }
    return array;
}

Object *getClassFields(Class *class, int public) {
    Class *array_class = findArrayClass("[Ljava/lang/reflect/Field;");
    Class *reflect_class = findSystemClass("java/lang/reflect/Field");
    ClassBlock *cb = CLASS_CB(class);
    Object *array, **fields;
    MethodBlock *init_mb;
    int count = 0;
    int i, j;

    if (!array_class || !reflect_class)
        return NULL;

    if (!(init_mb = findMethod(reflect_class, "<init>", "(Ljava/lang/Class;Ljava/lang/Class;Ljava/lang/String;I)V")))
        return NULL;

    if (!public)
        count = cb->fields_count;
    else
        for (i = 0; i < cb->fields_count; i++)
            if (cb->fields[i].access_flags & ACC_PUBLIC)
                count++;

    if ((array = allocArray(array_class, count, 4)) == NULL)
        return NULL;
    fields = (Object **) &INST_DATA(array)[1];

    for (i = 0, j = 0; j < count; i++) {
        FieldBlock *fb = &cb->fields[i];

        if (!public || (fb->access_flags & ACC_PUBLIC)) {
            Object *reflect_ob;

            if ((reflect_ob = allocObject(reflect_class))) {
                char *signature = sysMalloc(strlen(fb->type) + 1);
                char *sig = signature;
                Object *name;
                Class *type;

                strcpy(signature, fb->type);
                type = convertSigElement2Class(&sig, class);
                free(signature);
                name = createString(fb->name);

                if ((type == NULL) || (name == NULL))
                    return NULL;

                executeMethod(reflect_ob, init_mb, class, type, name, fb);
                fields[j++] = reflect_ob;
            } else
                return NULL;
        }
    }
    return array;
}

Object *getClassInterfaces(Class *class) {
    Class *array_class = findArrayClass("[Ljava/lang/Class;");
    ClassBlock *cb = CLASS_CB(class);
    Object *array;

    if ((array = allocArray(array_class, cb->interfaces_count, 4)) == NULL)
        return NULL;

    memcpy(&INST_DATA(array)[1], cb->interfaces, cb->interfaces_count * 4);
    return array;
}

Object *getClassClasses(Class *class, int public) {
    Class *array_class = findArrayClass("[Ljava/lang/Class;");
    ClassBlock *cb = CLASS_CB(class);
    int i, j, count = 0;
    Object *array;

    for (i = 0; i < cb->inner_class_count; i++) {
        Class *iclass;
        if ((iclass = resolveClass(class, cb->inner_classes[i], FALSE)) == NULL)
            return NULL;
        if (!public || (CLASS_CB(iclass)->access_flags & ACC_PUBLIC))
            count++;
    }

    if ((array = allocArray(array_class, count, 4)) == NULL)
        return NULL;

    for (i = 0, j = 1; j <= count; i++) {
        Class *iclass = resolveClass(class, cb->inner_classes[i], FALSE);
        if (!public || (CLASS_CB(iclass)->access_flags & ACC_PUBLIC))
            INST_DATA(array)[j++] = (u4) iclass;
    }

    return array;
}

Class *getDeclaringClass(Class *class) {
    ClassBlock *cb = CLASS_CB(class);
    return cb->declaring_class ? resolveClass(class, cb->declaring_class, FALSE) : NULL;
}

int getWrapperPrimTypeIndex(Object *arg) {
    ClassBlock *cb;

    if (arg == NULL) return 0;

    cb = CLASS_CB(arg->class);
    if (strncmp(cb->name, "java/lang/", 10) != 0)
        return 0;
    if (strcmp(&cb->name[10], "Boolean") == 0)
        return 1;
    if (strcmp(&cb->name[10], "Byte") == 0)
        return 2;
    if (strcmp(&cb->name[10], "Character") == 0)
        return 3;
    if (strcmp(&cb->name[10], "Short") == 0)
        return 4;
    if (strcmp(&cb->name[10], "Integer") == 0)
        return 5;
    if (strcmp(&cb->name[10], "Float") == 0)
        return 6;
    if (strcmp(&cb->name[10], "Long") == 0)
        return 7;
    if (strcmp(&cb->name[10], "Double") == 0)
        return 8;
    return 0;
}

Object *createWrapperObject(Class *type, u4 *pntr) {
    static char *wrapper_suffix[] = {
        "Boolean", "Byte", "Character", "Short",
        "Integer", "Float", "Long", "Double"
    };
    static char wrapper_name[20] = "java/lang/";
    ClassBlock *type_cb = CLASS_CB(type);

    if (IS_PRIMITIVE(type_cb)) {
        int idx = type_cb->flags - CLASS_PRIM - 1;
        if (idx == -1) /* void */
            return NULL;
        else {
            Class *wrapper_type;
            Object *wrapper = NULL;

            strncpy(&wrapper_name[10], wrapper_suffix[idx], 10);
            if ((wrapper_type = findSystemClass(wrapper_name)) &&
                (wrapper = allocObject(wrapper_type))) {
                INST_DATA(wrapper)[0] = pntr[0];
                if (idx > 5) /* i.e. long or double */
                    INST_DATA(wrapper)[1] = pntr[1];
            }
            return wrapper;
        }
    } else
        return (Object *) *pntr;
}

u4 *widenPrimitiveValue(int src_idx, int dest_idx, u4 *src, u4 *dest) {
#define err 0
#define U4 1
#define U8 2
#define I2F 3
#define I2D 4
#define I2J 5
#define J2F 6
#define J2D 7
#define F2D 8

    static char conv_table[9][8] = {
        /*  bool byte char shrt int  flt long  dbl             */
        {err, err, err, err, err, err, err, err}, /* !prim */
        {U4, err, err, err, err, err, err, err}, /* bool  */
        {err, U4, err, U4, U4, I2F, I2J, I2D}, /* byte  */
        {err, err, U4, err, U4, I2F, I2J, I2D}, /* char  */
        {err, err, err, U4, U4, I2F, I2J, I2D}, /* short */
        {err, err, err, err, U4, I2F, I2J, I2D}, /* int   */
        {err, err, err, err, err, U4, err, F2D}, /* float */
        {err, err, err, err, err, J2F, U8, J2D}, /* long  */
        {err, err, err, err, err, err, err, U8}
    }; /* dbl   */

    static void *handlers[] = {&&illegal_arg, &&u4, &&u8, &&i2f, &&i2d, &&i2j, &&j2f, &&j2d, &&f2d};

    int handler = conv_table[src_idx][dest_idx - 1];
    goto *handlers[handler];

u4:
    *dest++ = *src;
    return dest;
u8:
    *((u8 *) dest)++ = *(u8 *) src;
    return dest;
i2f:
    *((float *) dest)++ = (float) *(int *) src;
    return dest;
i2d:
    *((double *) dest)++ = (double) *(int *) src;
    return dest;
i2j:
    *((long long *) dest)++ = (long long) *((int *) src);
    return dest;
j2f:
    *((float *) dest)++ = (float) *(long long *) src;
    return dest;
j2d:
    *((double *) dest)++ = (double) *(long long *) src;
    return dest;
f2d:
    *((double *) dest)++ = (double) *(float *) src;
    return dest;

illegal_arg:
    return NULL;
}

u4 *unwrapAndWidenObject(Class *type, Object *arg, u4 *pntr) {
    ClassBlock *type_cb = CLASS_CB(type);

    if (IS_PRIMITIVE(type_cb)) {
        int formal_idx = getPrimTypeIndex(type_cb);
        int actual_idx = getWrapperPrimTypeIndex(arg);
        u4 *data = INST_DATA(arg);

        return widenPrimitiveValue(actual_idx, formal_idx, data, pntr);
    }

    if ((arg == NULL) || isInstanceOf(type, arg->class)) {
        *pntr++ = (u4) arg;
        return pntr;
    }

    return NULL;
}

Object *invoke(Object *ob, MethodBlock *mb, Object *arg_array, Object *param_types) {
    Object **args = (Object **) (INST_DATA(arg_array) + 1);
    Class **types = (Class **) (INST_DATA(param_types) + 1);
    int args_len = arg_array ? *INST_DATA(arg_array) : 0;
    int types_len = *INST_DATA(param_types);

    ExecEnv *ee = getExecEnv();
    void *ret;
    u4 *sp;
    int i;

    Object *excep;

    if (args_len != types_len) {
        signalException("java/lang/IllegalArgumentException", "wrong number of args");
        return NULL;
    }

    CREATE_TOP_FRAME(ee, mb->class, mb, sp, ret);

    if (ob) *sp++ = (u4) ob;

    for (i = 0; i < args_len; i++)
        if ((sp = unwrapAndWidenObject(*types++, *args++, sp)) == NULL) {
            POP_TOP_FRAME(ee);
            signalException("java/lang/IllegalArgumentException", "arg type mismatch");
            return NULL;
        }

    if (mb->access_flags & ACC_SYNCHRONIZED)
        objectLock(ob ? ob : (Object *) mb->class);

    if (mb->access_flags & ACC_NATIVE)
        (*(u4 *(*)(Class *, MethodBlock *, u4 *)) mb->native_invoker)(mb->class, mb, ret);
    else
        executeJava();

    if (mb->access_flags & ACC_SYNCHRONIZED)
        objectUnlock(ob ? ob : (Object *) mb->class);

    POP_TOP_FRAME(ee);

    if (excep = exceptionOccured()) {
        Object *ite_excep;
        MethodBlock *init;
        Class *ite_class;

        clearException();
        ite_class = findSystemClass("java/lang/reflect/InvocationTargetException");

        if (!exceptionOccured() && (ite_excep = allocObject(ite_class)) &&
            (init = lookupMethod(ite_class, "<init>", "(Ljava/lang/Throwable;)V"))) {
            executeMethod(ite_excep, init, excep);
            setException(ite_excep);
        }
        return NULL;
    }

    return ret;
}
