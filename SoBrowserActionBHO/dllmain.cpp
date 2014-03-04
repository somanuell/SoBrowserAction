// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "SoBrowserActionBHO_i.h"
#include "dllmain.h"

#include "CSoBABHO.h"

CSoBrowserActionBHOModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{

	if ( dwReason == DLL_PROCESS_ATTACH ) {

		CCSoBABHO::sm_dwTlsIndex = TlsAlloc();

		CCSoBABHO::sm_hInstance = hInstance;
		DWORD dwCopied = GetModuleFileName( hInstance,
		                                    CCSoBABHO::sm_szModulePath,
		                                    sizeof( CCSoBABHO::sm_szModulePath ) / sizeof( wchar_t ) );
		if ( dwCopied == 0 ) {
			wchar_t szError[ 16 ];
			_ltow_s( GetLastError(), szError, 10 );
			MessageBox( NULL, szError, L"SO BHO GetModuleFileName Error", NULL );
		} else {
			wchar_t * pLastAntiSlash = wcsrchr( CCSoBABHO::sm_szModulePath, L'\\' );
			if ( pLastAntiSlash ) *( pLastAntiSlash ) = 0;
		}
		CCSoBABHO::RegisterPrivateClass();

	} else if ( dwReason == DLL_THREAD_DETACH ) {

		CCSoBABHO::UnhookIfHooked();

	} else if ( dwReason == DLL_PROCESS_DETACH ) {

		CCSoBABHO::UnhookIfHooked();
		if ( CCSoBABHO::sm_dwTlsIndex != TLS_OUT_OF_INDEXES ) TlsFree( CCSoBABHO::sm_dwTlsIndex );
		CCSoBABHO::UnregisterPrivateClass();

	}
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
