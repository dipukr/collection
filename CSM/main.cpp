/*
*
* main.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
// main.cpp - TIL main routine

#include <stdio.h>
#include <stdarg.h>


#include "c-smile.h"

//|
//| Revision history:
//|
//| 17.04.2001 - initial implementation completed
//|
//|

using namespace c_smile;

bool  initialize_ext ( )
{
    return true;
}

//|

//| main - the main routine
//|
int main ( int argc,  char *argv [ ] ) 
{
    return cs_main ( argc, argv );
}
