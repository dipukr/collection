/*
 * SVM - VM bootstrap and command-line entry point.
 *
 * Usage: svm [-options] main.class [args...]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "svm.h"

char VM_initing = TRUE;

#define KB 1024
#define MB (KB * KB)

static size_t min_heap   = 1 * MB;
static size_t max_heap   = 64 * MB;
static int    java_stack = 256 * KB;
static int    verbosegc  = FALSE;
static int    verbosecl  = FALSE;

static void initVM(void) {
    initialiseAlloc(min_heap, max_heap, verbosegc);
    initialiseUtf8();
    initialiseClass(verbosecl);
    initialiseMonitor();
    initialiseMainThread(java_stack);   /* threadSelf() valid after this */

    /* Load the meta-class first so every subsequently-loaded class object
     * gets a valid class->class pointer (needed by the GC to recognise class
     * objects and scan their statics). */
    findSystemClass("saral/lang/Class");

    /* Core classes / subsystems (still under VM_initing: load errors fatal) */
    initialiseString();
    initialiseNatives();
    initialiseGC(FALSE);

    VM_initing = FALSE;
}

static void usage(const char *me) {
    printf("Usage: %s [-options] class [args...]\n", me);
    printf("  -cp <path>     colon-separated class path (default '.')\n");
    printf("  -verbose       trace class linking\n");
    printf("  -verbosegc     trace garbage collection\n");
    printf("  -ms<size>      initial heap (e.g. -ms2m)\n");
    printf("  -mx<size>      maximum heap (e.g. -mx128m)\n");
    printf("  -ss<size>      thread stack size\n");
    exit(0);
}

static size_t parseSize(const char *s) {
    char *end;
    long n = strtol(s, &end, 0);
    if(*end == 'm' || *end == 'M') n *= MB;
    else if(*end == 'k' || *end == 'K') n *= KB;
    return n;
}

int main(int argc, char *argv[]) {
    char cp_buf[4096] = ".";
    int i;
    Class *class;
    MethodBlock *mb;
    Object *args_array;
    Slot *body;

    for(i = 1; i < argc && argv[i][0] == '-'; i++) {
        if(!strcmp(argv[i], "-cp") && i + 1 < argc)
            strncpy(cp_buf, argv[++i], sizeof(cp_buf) - 1);
        else if(!strcmp(argv[i], "-verbose"))   verbosecl = TRUE;
        else if(!strcmp(argv[i], "-verbosegc")) verbosegc = TRUE;
        else if(!strncmp(argv[i], "-ms", 3))    min_heap = parseSize(argv[i] + 3);
        else if(!strncmp(argv[i], "-mx", 3))    max_heap = parseSize(argv[i] + 3);
        else if(!strncmp(argv[i], "-ss", 3))    java_stack = parseSize(argv[i] + 3);
        else if(!strcmp(argv[i], "-help"))      usage(argv[0]);
        else { printf("Unknown option: %s\n", argv[i]); usage(argv[0]); }
    }
    if(i >= argc) usage(argv[0]);

    setClassPath(cp_buf);
    initVM();

    /* main class: accept dotted or slashed name */
    {
        char name[512];
        char *p;
        strncpy(name, argv[i], sizeof(name) - 1);
        for(p = name; *p; p++) if(*p == '.') *p = '/';
        class = findSystemClass(name);
    }
    if(exceptionOccured()) { printException(); return 1; }

    mb = lookupMethod(class, "main", "([Lsaral/lang/String;)V");
    if(mb == NULL || !(mb->access_flags & ACC_STATIC)) {
        fprintf(stderr, "No static main([Lsaral/lang/String;)V in %s\n", argv[i]);
        return 1;
    }

    /* build the String[] argument array */
    args_array = allocArray(findArrayClass("[Lsaral/lang/String;"), argc - i - 1, sizeof(Slot));
    body = (Slot *)array_body(args_array);
    {
        int j;
        for(j = i + 1; j < argc; j++)
            body[j - i - 1].o = cstr2String(argv[j]);
    }

    executeMethodArgs(NULL, class, mb, args_array);

    if(exceptionOccured())
        printException();

    mainThreadWaitToExitVM();
    return 0;
}
