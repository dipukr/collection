#ifndef __SHELL_H

#include "c-smile-ni.h"

#ifdef _WIN32

#define   WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include  <windows.h>

#else
// KDE? stuff goes here
#endif

extern CSmile csr;
extern void   init_package(); 


#endif