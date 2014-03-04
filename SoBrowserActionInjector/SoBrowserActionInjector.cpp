#include <Windows.h>
#include <stdlib.h>

typedef LRESULT (CALLBACK *PHOOKCALLWNDPROCRET)( int nCode, WPARAM wParam, LPARAM lParam );
PHOOKCALLWNDPROCRET g_pHookCallWndProcRet;
HMODULE g_hDll;
UINT g_uiRegisteredMsg;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, char * pszCommandLine, int ) {

	//MessageBoxA( NULL, pszCommandLine, "Injector.exe", 0 );
	HWND hWndIEFrame = (HWND)atoi( pszCommandLine );

	wchar_t szFullPath[ MAX_PATH ];
	DWORD dwCopied = GetModuleFileName( NULL, szFullPath, sizeof( szFullPath ) / sizeof( wchar_t ) );
	if ( dwCopied == 0 ) {
		wchar_t szError[ 16 ];
		_ltow_s( GetLastError(), szError, 10 );
		MessageBox( NULL, szError, L"SO EXE Injector GetModuleFileName Error", NULL );
	} else {
		wchar_t * pLastAntiSlash = wcsrchr( szFullPath, L'\\' );
		if ( pLastAntiSlash ) *( pLastAntiSlash + 1 ) = 0;
		wcscat_s( szFullPath, L"SoBrowserActionBHO.dll" );
		g_hDll = LoadLibrary( szFullPath );
		if ( g_hDll == 0 ) {
			wchar_t szError[ 16 ];
			_ltow_s( GetLastError(), szError, 10 );
			MessageBox( NULL, szError, L"SO EXE Injector LoadLibrary Error", NULL );
		} else {
			g_pHookCallWndProcRet = (PHOOKCALLWNDPROCRET)GetProcAddress( g_hDll, "HookCallWndProcRet" );
			if ( g_pHookCallWndProcRet == 0 ) {
				wchar_t szError[ 16 ];
				_ltow_s( GetLastError(), szError, 10 );
				MessageBox( NULL, szError, L"SO EXE Injector GetProcAddress Error", NULL );
			} else {
				g_uiRegisteredMsg = RegisterWindowMessage( L"SOBA_MSG" );
				if ( g_uiRegisteredMsg == 0 ) {
					MessageBox( NULL, L"RegisterWindowMessage Failed", L"SO EXE Injector", NULL );
				} else {
					DWORD dwTID = GetWindowThreadProcessId( hWndIEFrame, NULL );
					HHOOK hHook = SetWindowsHookEx( WH_CALLWNDPROCRET, g_pHookCallWndProcRet, g_hDll, dwTID );
					if ( hHook == 0 ) {
						MessageBox( NULL, L"SetWindowsHookEx Failed", L"SO EXE Injector", NULL );
					} else {
						SendMessage( hWndIEFrame, g_uiRegisteredMsg, 0, reinterpret_cast<LPARAM>( hWndIEFrame ) );
						UnhookWindowsHookEx( hHook );
					}
				}
			}
		}
	}

	if ( g_hDll ) FreeLibrary( g_hDll );
	return 0;

}
