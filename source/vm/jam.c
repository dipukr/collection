#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "jam.h"


static int noasyncgc = FALSE;
static int verbosegc = FALSE;
static int verboseclass = FALSE;

Property *commandline_props;
int commandline_props_count = 0;

#define KB 1024
#define MB (KB*KB)
#define MIN_HEAP 4*KB
#define MIN_STACK 2*KB

static int java_stack = 64*KB;
static int min_heap   = 256*KB;
static int max_heap   = 16*MB;

char VM_initing = TRUE;

void initVM() {
 
    initialiseAlloc(min_heap, max_heap, verbosegc);
    initialiseClass(verboseclass);
    initialiseDll();
    initialiseUtf8();
    initialiseMonitor();
    initialiseMainThread(java_stack);
    initialiseException();
    initialiseString();
    initialiseGC(noasyncgc);
    initialiseJNI();

    /* No need to check for exception - if one occurs, signalException aborts VM */
    findSystemClass("java/lang/NoClassDefFoundError");
    findSystemClass("java/lang/ClassFormatError");
    findSystemClass("java/lang/NoSuchFieldError");
    findSystemClass("java/lang/NoSuchMethodError");

    VM_initing = FALSE;
}
    
void showUsage(char *name) {
    printf("Usage: %s [-options] class [arg1 arg2 ...]\n", name);
    printf("\nwhere options include:\n");
    printf("    -help\tprint out this message\n");
    printf("    -version\tprint out version number and copyright information\n");
    printf("    -verbose\tprint out information about class loading, etc.\n");
    printf("    -verbosegc\tprint out results of garbage collection\n");
    printf("    -noasyncgc\tturn off asynchronous garbage collection\n");
    printf("    -D<name>=<value>\n\t\tset a system property\n");
    printf("    -ms<size>\tset the initial size of the heap (default = %dK)\n", min_heap/KB);
    printf("    -mx<size>\tset the maximum size of the heap (default = %dM)\n", max_heap/MB);
    printf("    -ss<size>\tset the Java stack size for each thread (default = %dK)\n",java_stack/KB);
    printf("\t\tsize may be followed by K,k or M,m (e.g. 2M)\n");
}

void showVersionAndCopyright() {
    printf("JamVM version %s\n", VERSION);
    printf("Copyright (C) 2003 Robert Lougher <rob@lougher.demon.co.uk>\n\n");
    printf("This program is free software; you can redistribute it and/or\n");
    printf("modify it under the terms of the GNU General Public License\n");
    printf("as published by the Free Software Foundation; either version 2,\n");
    printf("or (at your option) any later version.\n\n");
    printf("This program is distributed in the hope that it will be useful,\n");
    printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    printf("GNU General Public License for more details.\n");
}

int parseMemValue(char *str) {
    char *end;
    long n = strtol(str, &end, 0);

    switch(end[0]) {
        case '\0':
            break;

        case 'M':
        case 'm':
            n *= MB;
	    break;

        case 'K':
        case 'k':
            n *= KB;
            break;

        default:
             n = 0;
    } 

    return n;
}

int parseCommandLine(int argc, char *argv[]) {
    int status = 1;
    int i;

    Property props[argc-1];

    for(i = 1; i < argc; i++) {
        if(*argv[i] != '-') {
            if(min_heap > max_heap) {
                printf("Minimum heap size greater than max!\n");
                goto exit;
	    }
            if(commandline_props_count) {
                commandline_props = (Property*)sysMalloc(commandline_props_count * sizeof(Property));
                memcpy(commandline_props, &props[0], commandline_props_count * sizeof(Property));
            }
            return i;
	}

	if(strcmp(argv[i], "-help") == 0) {
            status = 0;
            break;
        }

	else if(strcmp(argv[i], "-version") == 0) {
            showVersionAndCopyright();
            status = 0;
            goto exit;

	} else if(strcmp(argv[i], "-verbose") == 0)
            verboseclass = TRUE;

        else if(strcmp(argv[i], "-verbosegc") == 0)
            verbosegc = TRUE;

        else if(strcmp(argv[i], "-noasyncgc") == 0)
            noasyncgc = TRUE;

        else if(strncmp(argv[i], "-ms", 3) == 0) {
            min_heap = parseMemValue(argv[i]+3);
	    if(min_heap < MIN_HEAP) {
                printf("Invalid minimum heap size: %s (min %dK)\n", argv[i], MIN_HEAP/KB);
	        goto exit;
            }

	} else if(strncmp(argv[i], "-mx", 3) == 0) {
            max_heap = parseMemValue(argv[i]+3);
	    if(max_heap < MIN_HEAP) {
                printf("Invalid maximum heap size: %s (min is %dK)\n", argv[i], MIN_HEAP/KB);
	        goto exit;
            }

	} else if(strncmp(argv[i], "-ss", 3) == 0) {
            java_stack = parseMemValue(argv[i]+3);
	    if(java_stack < MIN_STACK) {
                printf("Invalid Java stack size: %s\n", argv[i]);
	        goto exit;
            }

	} else if(strncmp(argv[i], "-D", 2) == 0) {
            char *pntr, *key = strcpy(sysMalloc(strlen(argv[i]+2) + 1), argv[i]+2);
            for(pntr = key; *pntr && (*pntr != '='); pntr++);
            if(pntr == key) {
                printf("Bad property: %s\n", argv[i]);
                goto exit;
            }
            *pntr++ = '\0';
            props[commandline_props_count].key = key;
            props[commandline_props_count++].value = pntr;
	} else {
            printf("Unrecognised command line option: %s\n", argv[i]);
	    break;
        }
    }

    showUsage(argv[0]);

exit:
    exit(status);
}

int main(int argc, char *argv[]) {
    Class *array_class, *class;
    MethodBlock *mb;
    Object *array;
    char *cpntr;
    int status;
    int i;

    int class_arg = parseCommandLine(argc, argv);

    initVM();

    for(cpntr = argv[class_arg]; *cpntr; cpntr++)
        if(*cpntr == '.')
            *cpntr = '/';

    class = findSystemClass(argv[class_arg]);
    if(exceptionOccured()) {
        printException();
        exitVM(1);
    }

    mb = lookupMethod(class, "main", "([Ljava/lang/String;)V");
    if(!mb || !(mb->access_flags & ACC_STATIC)) {
        printf("Static method \"main\" not found in %s\n", argv[class_arg]);
        exitVM(1);
    }

    /* Create the String array holding the command line args */

    if((array_class = findArrayClass("[Ljava/lang/String;")) &&
           (array = allocArray(array_class, argc-class_arg-1, 4)))  {
        u4 *args = INST_DATA(array)-class_arg;

        for(i = class_arg+1; i < argc; i++)
            if(!(args[i] = (u4)Cstr2String(argv[i])))
                break;

        /* Call the main method */
        if(i == argc)
            executeStaticMethod(class, mb, array);
    }

    /* ExceptionOccured returns the exception or NULL, which is OK
       for normal conditionals, but not here... */
    if(status = (exceptionOccured() ? 1 : 0))
        printException();

    mainThreadWaitToExitVM();
    exitVM(status);
}
