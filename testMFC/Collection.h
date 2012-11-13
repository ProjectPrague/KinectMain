#pragma once
// TO DO add all global used header files to tidy it up.
//This file needs to be renamed to stdafx.h for precompiling
//This includes the AFX.h file and the WIN_NT defenition

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