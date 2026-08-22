#ifdef NO_JNI
#error to use classpath, Jam must be compiled with JNI!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

/* For system properties */
#include <sys/utsname.h>

#include "jam.h"
#include "thread.h"
#include "lock.h"

/* java.lang.VMObject */

u4 *getClass(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *ob = (Object*)*ostack;
    *ostack++ = (u4)ob->class;
    return ostack;
}

u4 *jamClone(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *ob = (Object*)*ostack;
    *ostack++ = (u4)cloneObject(ob);
    return ostack;
}

/* static method wait(Ljava/lang/Object;JI)V */
u4 *wait(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *obj = (Object *)ostack[0];
    long long ms = *((long long *)&ostack[1]);
    int ns = ostack[3];

    objectWait(obj, ms, ns);
    return ostack;
}

/* static method notify(Ljava/lang/Object;)V */
u4 *notify(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *obj = (Object *)*ostack;
    objectNotify(obj);
    return ostack;
}

/* static method notifyAll(Ljava/lang/Object;)V */
u4 *notifyAll(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *obj = (Object *)*ostack;
    objectNotifyAll(obj);
    return ostack;
}

/* java.lang.VMSystem */

/* arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V */
u4 *arraycopy(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *src = (Object *)ostack[0];
    int start1 = ostack[1];
    Object *dest = (Object *)ostack[2];
    int start2 = ostack[3];
    int length = ostack[4];


    if((src == NULL) || (dest == NULL))
	signalException("java/lang/NullPointerException", NULL);
    else {
        ClassBlock *scb = CLASS_CB(src->class);
        ClassBlock *dcb = CLASS_CB(dest->class);
        int *sdata = INST_DATA(src);            
        int *ddata = INST_DATA(dest);            

        if((scb->name[0] != '[') || (dcb->name[0] != '['))
            goto storeExcep; 

        if((start1 < 0) || (start2 < 0) || (length < 0)
                        || ((start1 + length) > sdata[0]) || ((start2 + length) > ddata[0])) {
            signalException("java/lang/ArrayIndexOutOfBoundsException", NULL);
            return ostack;
	}

        if(isInstanceOf(dest->class, src->class)) {
            int size;

            switch(scb->name[1]) {
                case 'B':
                    size = 1;
                    break;
                case 'C':
                case 'S':
                    size = 2;
                    break;
                case 'I':
                case 'F':
                case 'L':
                case '[':
                    size = 4;
                    break;
                case 'J':
                case 'D':
                    size = 8;
                    break;
            } 

            memmove(((char *)&ddata[1]) + start2*size,
                    ((char *)&sdata[1]) + start1*size,
                    length*size);
	} else {
	    Object **sob, **dob;
	    int i;

   	    if(!(((scb->name[1] == 'L') || (scb->name[1] == '[')) &&
   	                  ((dcb->name[1] == 'L') || (dcb->name[1] == '['))))
                goto storeExcep; 

	    /* Not compatible array types, but elements may be compatible...
	       e.g. src = [Ljava/lang/Object, dest = [Ljava/lang/String, but
               all src = Strings - check one by one...
	     */
	    
	    if(scb->dim != dcb->dim)
                goto storeExcep;

	    sob = (Object**)&sdata[start1+1];
	    dob = (Object**)&ddata[start2+1];

	    if(scb->dim == 1)
	        for(i = 0; i < length; i++) {
                    if(*sob && !isInstanceOf(dcb->element_class, (*sob)->class))
                        goto storeExcep;
                    *dob++ = *sob++;
	        }
	    else
	        for(i = 0; i < length; i++) {
                    if(*sob && !isInstanceOf(dcb->element_class, CLASS_CB((*sob)->class)->element_class))
                        goto storeExcep;
                    *dob++ = *sob++;
	        }
	}
    }
    return ostack;

storeExcep:
    signalException("java/lang/ArrayStoreException", NULL);
    return ostack;
}

u4 *identityHashCode(Class *class, MethodBlock *mb, u4 *ostack) {
    return ++ostack;
}

/* java.lang.Runtime */

u4 *freeMemory(Class *class, MethodBlock *mb, u4 *ostack) {
    *((u8*)ostack)++ = (u8) freeHeapMem();
    return ostack;
}

u4 *totalMemory(Class *class, MethodBlock *mb, u4 *ostack) {
    *((u8*)ostack)++ = (u8) totalHeapMem();
    return ostack;
}

u4 *maxMemory(Class *class, MethodBlock *mb, u4 *ostack) {
    *((u8*)ostack)++ = (u8) maxHeapMem();
    return ostack;
}

u4 *gc(Class *class, MethodBlock *mb, u4 *ostack) {
    gc1();
    return ostack;
}

u4 *runFinalization(Class *class, MethodBlock *mb, u4 *ostack) {
    return ostack;
}

u4 *exitInternal(Class *class, MethodBlock *mb, u4 *ostack) {
    exitVM(0);
}

u4 *nativeLoad(Class *class, MethodBlock *mb, u4 *ostack) {
    char *name = String2Cstr((Object*)ostack[1]);

    ostack[0] = resolveDll(name);
    free(name);

    return ostack+1;
}

u4 *nativeGetLibname(Class *class, MethodBlock *mb, u4 *ostack) {
    char *path = String2Cstr((Object*)ostack[0]);
    char *name = String2Cstr((Object*)ostack[1]);
    char *lib = getDllName(path, name);
    free(path);
    free(name);
    *ostack++ = (u4)Cstr2String(lib);
    free(lib);
    return ostack;
}

void setProperty(Object *this, char *key, char *value) {
    Object *k = Cstr2String(key);
    Object *v = Cstr2String(value ? value : "?");

    MethodBlock *mb = lookupMethod(this->class, "put",
                           "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    executeMethod(this, mb, k, v);
}

extern Property *commandline_props;
extern int commandline_props_count;

u4 *insertSystemProperties(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *this = (Object *)*ostack;
    struct utsname info;
    uname(&info);

    setProperty(this, "java.vm.name", "JamVM");
    setProperty(this, "java.vm.version", VERSION);
    setProperty(this, "java.vm.vendor", "Robert Lougher");
    setProperty(this, "java.vm.vendor.url", "http://jamvm.sourceforge.net");
    setProperty(this, "java.version", "1.4");
    setProperty(this, "java.vendor", "GNU Classpath");
    setProperty(this, "java.vendor.url", "http://gnu.classpath.org");
    setProperty(this, "java.home", INSTALL_DIR);
    setProperty(this, "java.specification.version", "1.4");
    setProperty(this, "java.specification.vendor", "Sun Microsystems, Inc.");
    setProperty(this, "java.specification.name", "Java Platform API Specification");
    setProperty(this, "java.vm.specification.version", "1.0");
    setProperty(this, "java.vm.specification.vendor", "Sun Microsystems, Inc.");
    setProperty(this, "java.vm.specification.name", "Java Virtual Machine Specification");
    setProperty(this, "java.class.version", "48.0");
    setProperty(this, "java.class.path", getClassPath());
    setProperty(this, "java.library.path", getDllPath());
    setProperty(this, "java.io.tmpdir", "/tmp");
    setProperty(this, "java.compiler", "");
    setProperty(this, "java.ext.dirs", "");
    setProperty(this, "os.name", info.sysname);
    setProperty(this, "os.arch", info.machine);
    setProperty(this, "os.version", info.release);
    setProperty(this, "file.separator", "/");
    setProperty(this, "path.separator", ":");
    setProperty(this, "line.separator", "\n");
    setProperty(this, "user.name", getenv("USER"));
    setProperty(this, "user.home", getenv("HOME"));
    setProperty(this, "user.dir", getenv("PWD"));

    /* Handle command line properties */

    if(commandline_props_count) {
        int i;
        for(i = 0; i < commandline_props_count; i++) {
            setProperty(this, commandline_props[i].key, commandline_props[i].value);
            free(commandline_props[i].key);
        }
        free(commandline_props);
    }

    return ostack;
}

/* java.lang.VMClass */

extern int clazz_offset;

#define GET_CLASS(vmClass) (Class*)(INST_DATA((Object*)vmClass)[clazz_offset])

u4 *isInstance(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    Object *ob = (Object*)ostack[1];

    *ostack++ = ob == NULL ? FALSE : (u4)isInstanceOf(clazz, ob->class);
    return ostack;
}

u4 *isAssignableFrom(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    Class *clazz2 = GET_CLASS(ostack[2]);

    if(clazz2 == NULL)
        signalException("java/lang/NullPointerException", NULL);
    else
        *ostack++ = (u4)isInstanceOf(clazz, clazz2);

    return ostack;
}

u4 *isInterface(Class *class, MethodBlock *mb, u4 *ostack) {
    ClassBlock *cb = CLASS_CB(GET_CLASS(ostack[0]));
    *ostack++ = IS_INTERFACE(cb) ? TRUE : FALSE;
    return ostack;
}

u4 *isPrimitive(Class *class, MethodBlock *mb, u4 *ostack) {
    ClassBlock *cb = CLASS_CB(GET_CLASS(ostack[0]));
    *ostack++ = IS_PRIMITIVE(cb) ? TRUE : FALSE;
    return ostack;
}

u4 *isArray(Class *class, MethodBlock *mb, u4 *ostack) {
    ClassBlock *cb = CLASS_CB(GET_CLASS(ostack[0]));
    *ostack++ = IS_ARRAY(cb) ? TRUE : FALSE;
    return ostack;
}

u4 *getSuperclass(Class *class, MethodBlock *mb, u4 *ostack) {
    ClassBlock *cb = CLASS_CB(GET_CLASS(ostack[0]));
    *ostack++ = (u4) (IS_PRIMITIVE(cb) || IS_INTERFACE(cb) ? NULL : cb->super);
    return ostack;
}

u4 *getComponentType(Class *clazz, MethodBlock *mb, u4 *ostack) {
    Class *class = GET_CLASS(ostack[0]);
    ClassBlock *cb = CLASS_CB(class);
    Class *componentType = NULL;

    if(IS_ARRAY(cb))
        switch(cb->name[1]) {
            case 'L':
                componentType = cb->element_class;
                break;

            case '[':
                componentType = findArrayClassFromClass(&cb->name[1], class);
                break;

            default:
                componentType = findPrimitiveClass(cb->name[1]);
                break;
        }
 
    *ostack++ = (u4) componentType;
    return ostack;
}

u4 *getName(Class *class, MethodBlock *mb, u4 *ostack) {
    unsigned char *dot_name = slash2dots(CLASS_CB((GET_CLASS(*ostack)))->name);
    Object *string = createString(dot_name);
    *ostack++ = (u4)string;
    free(dot_name);
    return ostack;
}

u4 *getDeclaredClasses(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    int public = ostack[1];
    *ostack++ = (u4) getClassClasses(clazz, public);
    return ostack;
}

u4 *getDeclaringClass0(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    *ostack++ = (u4) getDeclaringClass(clazz);
    return ostack;
}

u4 *getDeclaredConstructors(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    int public = ostack[1];
    *ostack++ = (u4) getClassConstructors(clazz, public);
    return ostack;
}

u4 *getDeclaredMethods(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    int public = ostack[1];
    *ostack++ = (u4) getClassMethods(clazz, public);
    return ostack;
}

u4 *getDeclaredFields(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    int public = ostack[1];
    *ostack++ = (u4) getClassFields(clazz, public);
    return ostack;
}

u4 *getInterfaces(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    *ostack++ = (u4) getClassInterfaces(clazz);
    return ostack;
}

u4 *getClassLoader(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(*ostack);
    *ostack++ = (u4)CLASS_CB(clazz)->class_loader;
    return ostack;
}

u4 *getClassModifiers(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(*ostack);
    *ostack++ = (u4)CLASS_CB(clazz)->access_flags;
    return ostack;
}

u4 *forName0(u4 *ostack, int resolve, Object *loader) {
    Object *string = (Object *)ostack[0];
    char *cstr = String2Cstr(string);
    int len = strlen(cstr);
    Class *class = NULL;
    int i = 0;
    
    if(cstr[0] == '[') {
        for(; cstr[i] == '['; i++);
        switch(cstr[i]) {
            case 'Z':
            case 'B':
            case 'C':
            case 'S':
            case 'I':
            case 'F':
            case 'J':
            case 'D':
                if(len-i != 1)
                    goto out;
                break;
            case 'L':
                if(cstr[len-1] != ';')
                    goto out;
                break;
            default:
                goto out;
                break;
        }
    }
    for(; i < len; i++)
        if(cstr[i]=='.') cstr[i]='/';

    class = findClassFromClassLoader(cstr, loader);

out:
    if(class == NULL) {
        clearException();
	signalException("java/lang/ClassNotFoundException", cstr);
    } else
        if(resolve)
            initClass(class);

    free(cstr);
    *ostack++ = (u4)class;
    return ostack;
}

u4 *forName(Class *clazz, MethodBlock *mb, u4 *ostack) {
    Frame *caller = getExecEnv()->last_frame->prev;
    Object *loader = caller->mb ? CLASS_CB(caller->mb->class)->class_loader : NULL;

    return forName0(ostack, TRUE, loader);
}

u4 *loadArrayClass(Class *clazz, MethodBlock *mb, u4 *ostack) {
    Object *loader = (Object*)ostack[1];
    return forName0(ostack, FALSE, loader);
}

u4 *initialize(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = GET_CLASS(ostack[0]);
    initClass(clazz);
    return ostack;
}

u4 *throwException(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *excep = (Object *)ostack[0];
    setException(excep);
    return ostack;
}

/* java.lang.VMThrowable */

u4 *fillInStackTrace(Class *class, MethodBlock *mb, u4 *ostack) {
    *ostack++ = (u4) setStackTrace();
    return ostack;
}

u4 *getStackTrace(Class *class, MethodBlock *m, u4 *ostack) {
    Object *this = (Object *)*ostack;
    *ostack++ = (u4) convertStackTrace(this);
    return ostack;
}

/* java.lang.VMSecurityManager */

u4 *currentClassLoader(Class *class, MethodBlock *mb, u4 *ostack) {
    *ostack++ = (u4)NULL;
    return ostack;
}

u4 *getClassContext(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *class_class = findArrayClass("[Ljava/lang/Class;");
    Frame *bottom, *last;
    int depth = 0;
    Object *array;
    int *data;

    if(class_class == NULL)
        return ostack;

    /* The top frame will be frame for VMSecurityManager - skip */
    last = getExecEnv()->last_frame->prev;

    /* The next frame _may_ be SecurityManager */
    if(last->mb && strcmp(CLASS_CB(last->mb->class)->name, "java/lang/SecurityManager") == 0)
        last = last->prev;

    bottom = last;
    do {
        for(; last->mb != NULL; last = last->prev, depth++);
    } while((last = last->prev)->prev != NULL);
    
    array = allocArray(class_class, depth, 4);

    if(array != NULL) {
        data = INST_DATA(array);

        depth = 1;
        do {
            for(; bottom->mb != NULL; bottom = bottom->prev)
                data[depth++] = (int)bottom->mb->class;
        } while((bottom = bottom->prev)->prev != NULL);
    }

    *ostack++ = (int)array;
    return ostack;
}

/* java.lang.VMClassLoader */

/* loadClass(Ljava/lang/String;I)Ljava/lang/Class; */
u4 *loadClass(Class *clazz, MethodBlock *mb, u4 *ostack) {
    int resolve = ostack[1];
    return forName0(ostack, resolve, NULL);
}

/* getPrimitiveClass(C)Ljava/lang/Class; */
u4 *getPrimitiveClass(Class *class, MethodBlock *mb, u4 *ostack) {
    char prim_type = *ostack;
    *ostack++ = (u4)findPrimitiveClass(prim_type);
    return ostack;
}

u4 *defineClass0(Class *clazz, MethodBlock *mb, u4 *ostack) {
    Object *class_loader = (Object *)ostack[0];
    Object *string = (Object *)ostack[1];
    Object *array = (Object *)ostack[2];
    int offset = ostack[3];
    int data_len = ostack[4];
    char *data = ((char*)INST_DATA(array)) + 4;
    char *cstr = string ? String2Cstr(string) : NULL;
    int len = string ? strlen(cstr) : 0;
    Class *class;
    int i;

    for(i = 0; i < len; i++)
        if(cstr[i]=='.') cstr[i]='/';

    if((class = defineClass(cstr, data, offset, data_len, class_loader)) != NULL)
        linkClass(class);

    free(cstr);
    *ostack++ = (int)class;
    return ostack;
}

u4 *resolveClass0(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *clazz = (Class *)*ostack;
    initClass(clazz);
    return ostack;
}

/* java.lang.reflect.Constructor */

u4 *constructNative(Class *class, MethodBlock *mb2, u4 *ostack) {
    Object *array = (Object*)ostack[1]; 
    Object *paramTypes = (Object*)ostack[3];
    MethodBlock *mb = (MethodBlock*)ostack[4]; 
    Object *ob = allocObject(mb->class);

    if(ob) invoke(ob, mb, array, paramTypes);

    *ostack++ = (u4) ob;
    return ostack;
}

u4 *getMethodModifiers(Class *class, MethodBlock *mb2, u4 *ostack) {
    MethodBlock *mb = (MethodBlock*)ostack[1]; 
    *ostack++ = (u4) mb->access_flags;
    return ostack;
}

u4 *getFieldModifiers(Class *class, MethodBlock *mb, u4 *ostack) {
    FieldBlock *fb = (FieldBlock*)ostack[1]; 
    *ostack++ = (u4) fb->access_flags;
    return ostack;
}

Object *getAndCheckObject(u4 *ostack, Class *type) {
    Object *ob = (Object*)ostack[1];

    if(ob == NULL) {
        signalException("java/lang/NullPointerException", NULL);
        return NULL;
    }
    if(!isInstanceOf(type, ob->class)) {
        signalException("java/lang/IllegalArgumentException", "object is not an instance of declaring class");
        return NULL;
    }
    return ob;
}

u4 *getPntr2Field(u4 *ostack) {
    Class *decl_class = (Class *)ostack[2];
    FieldBlock *fb = (FieldBlock*)ostack[4]; 
    Object *ob;

    if(fb->access_flags & ACC_STATIC)
        return &fb->static_value;

    if((ob = getAndCheckObject(ostack, decl_class)) == NULL)
        return NULL;

    return &(INST_DATA(ob)[fb->offset]);
}

u4 *getField(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *field_type = (Class *)ostack[3];
    u4 *field;

    if((field = getPntr2Field(ostack)) != NULL)
        *ostack++ = (u4) createWrapperObject(field_type, field);
    return ostack;
}

u4 *getPrimitiveField(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *field_type = (Class *)ostack[3];
    int type_no = ostack[5]; 

    ClassBlock *type_cb = CLASS_CB(field_type);
    u4 *field;

    if(((field = getPntr2Field(ostack)) != NULL) && (!(IS_PRIMITIVE(type_cb)) ||
                 ((ostack = widenPrimitiveValue(getPrimTypeIndex(type_cb), type_no, field, ostack)) == NULL)))
        signalException("java/lang/IllegalArgumentException", "field type mismatch");
    return ostack;
}

u4 *setField(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *field_type = (Class *)ostack[3];
    Object *value = (Object*)ostack[5];
    u4 *field;

    if(((field = getPntr2Field(ostack)) != NULL) &&
                     (unwrapAndWidenObject(field_type, value, field) == NULL))
        signalException("java/lang/IllegalArgumentException", "field type mismatch");

    return ostack;
}

u4 *setPrimitiveField(Class *class, MethodBlock *mb, u4 *ostack) {
    Class *field_type = (Class *)ostack[3];
    int type_no = ostack[5]; 

    ClassBlock *type_cb = CLASS_CB(field_type);
    u4 *field;

    if(((field = getPntr2Field(ostack)) != NULL) && (!(IS_PRIMITIVE(type_cb)) ||
                 (widenPrimitiveValue(type_no, getPrimTypeIndex(type_cb), &ostack[6], field) == NULL)))
        signalException("java/lang/IllegalArgumentException", "field type mismatch");
    return ostack;
}

/* java.lang.reflect.Method */

u4 *invokeNative(Class *class, MethodBlock *mb2, u4 *ostack) {
    Object *array = (Object*)ostack[2]; 
    Object *paramTypes = (Object*)ostack[4];
    Class *retType = (Class*)ostack[5];
    MethodBlock *mb = (MethodBlock*)ostack[6]; 
    Object *ob = NULL;
    u4 *ret;

    if(!(mb->access_flags & ACC_STATIC)) {
        if((ob = getAndCheckObject(ostack, mb->class)) == NULL)
            return ostack;
        mb = CLASS_CB(ob->class)->method_table[mb->method_table_index];
    }
 
    if((ret = (u4*) invoke(ob, mb, array, paramTypes)) != NULL)
        *ostack++ = (u4) createWrapperObject(retType, ret);
    return ostack;
}

/* java.lang.VMString */

/* static method - intern(Ljava/lang/String;)Ljava/lang/String; */
u4 *intern(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *string = (Object*)ostack[0];
    ostack[0] = (u4)findInternedString(string);
    return ostack+1;
}

/* java.lang.Thread */

/* static method currentThread()Ljava/lang/Thread; */
u4 *currentThread(Class *class, MethodBlock *mb, u4 *ostack) {
    *ostack++ = (u4)getExecEnv()->thread;
    return ostack;
}

/* instance method nativeInit(J)V */
u4 *nativeInit(Class *class, MethodBlock *mb, u4 *ostack) {
    return ostack;
}

/* instance method start()V */
u4 *start(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *this = (Object *)*ostack;
    createJavaThread(this);
    return ostack;
}

/* static method sleep(JI)V */
u4 *jamSleep(Class *class, MethodBlock *mb, u4 *ostack) {
    long long ms = *((long long *)&ostack[0]);
    int ns = ostack[2];
    Thread *thread = threadSelf();

    threadSleep(thread, ms, ns);

    return ostack;
}

/* instance method interrupt()V */
u4 *interrupt(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *this = (Object *)*ostack;
    Thread *thread = threadSelf0(this);
    if(thread)
        threadInterrupt(thread);
    return ostack;
}

/* instance method isAlive()Z */
u4 *isAlive(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *this = (Object *)*ostack;
    Thread *thread = threadSelf0(this);
    *ostack++ = thread ? threadIsAlive(thread) : FALSE;
    return ostack;
}

/* static method yield()V */
u4 *yield(Class *class, MethodBlock *mb, u4 *ostack) {
    Thread *thread = threadSelf();
    threadYield(thread);
    return ostack;
}

/* instance method isInterrupted()Z */
u4 *isInterrupted(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *this = (Object *)*ostack;
    Thread *thread = threadSelf0(this);
    *ostack++ = thread ? threadIsInterrupted(thread) : FALSE;
    return ostack;
}

/* static method interrupted()Z */
u4 *interrupted(Class *class, MethodBlock *mb, u4 *ostack) {
    Thread *thread = threadSelf();
    *ostack++ = threadInterrupted(thread);
    return ostack;
}

/* instance method nativeSetPriority(I)V */
u4 *nativeSetPriority(Class *class, MethodBlock *mb, u4 *ostack) {
    return ostack+1;
}

/* instance method holdsLock(Ljava/lang/Object;)Z */
u4 *holdsLock(Class *class, MethodBlock *mb, u4 *ostack) {
    Object *ob = (Object *)ostack[0];
    if(ob == NULL)
        signalException("java/lang/NullPointerException", NULL);
    else
        *ostack++ = objectLockedByCurrent(ob);
    return ostack;
}

char *native_methods[][2] = {
                             "arraycopy",		(char*)arraycopy,
                             "insertSystemProperties",	(char*)insertSystemProperties,
                             "getPrimitiveClass",	(char*)getPrimitiveClass,
			     "defineClass",             (char*)defineClass0,
			     "resolveClass",            (char*)resolveClass0,
                             "intern",			(char*)intern,
                             "loadClass",		(char*)loadClass,
                             "loadArrayClass",		(char*)loadArrayClass,
                             "initialize",		(char*)initialize,
                             "throwException",		(char*)throwException,
                             "forName",			(char*)forName,
			     "isInstance",		(char*)isInstance,
			     "isAssignableFrom",	(char*)isAssignableFrom,
			     "isInterface",		(char*)isInterface,
			     "isPrimitive",		(char*)isPrimitive,
			     "isArray",			(char*)isArray,
                             "getSuperclass",		(char*)getSuperclass,
                             "getName",			(char*)getName,
                             "getClass",		(char*)getClass,
			     "getComponentType",	(char*)getComponentType,
                             "identityHashCode",	(char*)identityHashCode,
			     "clone",			(char*)jamClone,
			     "wait",			(char*)wait,
			     "notify",			(char*)notify,
			     "notifyAll",		(char*)notifyAll,
                             "gc",			(char*)gc,
                             "runFinalization",		(char*)runFinalization,
                             "exitInternal",		(char*)exitInternal,
                             "fillInStackTrace",	(char*)fillInStackTrace,
                             "getStackTrace",		(char*)getStackTrace,
			     "currentClassLoader",	(char*)currentClassLoader,
			     "getClassContext",		(char*)getClassContext,
			     "getClassLoader",		(char*)getClassLoader,
			     "constructNative",		(char*)constructNative,
			     "invokeNative",		(char*)invokeNative,
                             "nativeLoad",		(char*)nativeLoad,
                             "nativeGetLibname",	(char*)nativeGetLibname,
			     "freeMemory",		(char*)freeMemory,
			     "totalMemory",		(char*)totalMemory,
			     "maxMemory",		(char*)maxMemory,
			     "currentThread",		(char*)currentThread,
			     "nativeInit",		(char*)nativeInit,
			     "start",			(char*)start,
			     "sleep",			(char*)jamSleep,
			     "nativeInterrupt",		(char*)interrupt,
			     "isAlive",			(char*)isAlive,
			     "yield",			(char*)yield,
			     "isInterrupted",		(char*)isInterrupted,
			     "interrupted",		(char*)interrupted,
			     "nativeSetPriority",	(char*)nativeSetPriority,
                             "holdsLock",		(char*)holdsLock,
			     "getDeclaredClasses",	(char*)getDeclaredClasses,
			     "getDeclaringClass",	(char*)getDeclaringClass0,
			     "getDeclaredConstructors",	(char*)getDeclaredConstructors,
			     "getDeclaredMethods",	(char*)getDeclaredMethods,
			     "getDeclaredFields",	(char*)getDeclaredFields,
			     "getInterfaces",		(char*)getInterfaces,
			     "getModifiers",		(char*)getClassModifiers,
			     "getConstructorModifiers",	(char*)getMethodModifiers,
			     "getMethodModifiers",	(char*)getMethodModifiers,
			     "getFieldModifiers",	(char*)getFieldModifiers,
                             "getField",		(char*)getField,
                             "setField",		(char*)setField,
                             "setZField",		(char*)setPrimitiveField,
                             "setBField",		(char*)setPrimitiveField,
                             "setCField",		(char*)setPrimitiveField,
                             "setSField",		(char*)setPrimitiveField,
                             "setIField",		(char*)setPrimitiveField,
                             "setFField",		(char*)setPrimitiveField,
                             "setJField",		(char*)setPrimitiveField,
                             "setDField",		(char*)setPrimitiveField,
                             "getZField",		(char*)getPrimitiveField,
                             "getBField",		(char*)getPrimitiveField,
                             "getCField",		(char*)getPrimitiveField,
                             "getSField",		(char*)getPrimitiveField,
                             "getIField",		(char*)getPrimitiveField,
                             "getFField",		(char*)getPrimitiveField,
                             "getJField",		(char*)getPrimitiveField,
                             "getDField",		(char*)getPrimitiveField,
                             NULL,			NULL};

