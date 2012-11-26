#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef WINVER							// Allow use of features specific to Windows Vista or later.
#define WINVER 0x06000000				// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT					// Allow use of features specific to Windows Vista or later.
#define _WIN32_WINNT 0x0600				// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS					// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0601			// Change this to the appropriate value to target Windows Me or later.
#endif

#include <afx.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <Windows.h>
#include <iostream>
#include <cstdlib>

// Safe release for interfaces
template<class Interface>
inline void SafeRelease( Interface *& pInterfaceToRelease )
{
    if ( pInterfaceToRelease != NULL )
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = NULL;
    }
}