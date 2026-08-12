//
// shell.cpp : Defines the entry point for the DLL application.
//

#include "shell.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

CSmile  csr;
void    init_package();

CSNIEXPORT int CSNICALL c_smile_ni_attach(c_smile_interface *csrt)
{
  csr.init(csrt);
  init_package();
  return 1;
}


