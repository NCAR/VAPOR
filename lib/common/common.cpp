// common.cpp : Defines the entry point for the DLL application.
//

#ifdef WIN32
#include <vapor/common.h>
#include "windows.h"

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
/*
// This is an example of an exported variable
COMMON_API int ncommon=0;

// This is an example of an exported function.
COMMON_API int fncommon(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see common.h for the class definition
Ccommon::Ccommon()
{ 
	return; 
}
*/
#endif
