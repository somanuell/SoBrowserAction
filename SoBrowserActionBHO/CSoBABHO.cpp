// CSoBABHO.cpp : Implementation of CCSoBABHO

#include "stdafx.h"
#include "CSoBABHO.h"
#include <WinError.h>


HMODULE CCSoBABHO::sm_hInstance;
wchar_t CCSoBABHO::sm_szModulePath[ MAX_PATH ];
DWORD CCSoBABHO::sm_dwTlsIndex;
UINT CCSoBABHO::sm_uiRegisteredMsg;
wchar_t * CCSoBABHO::sm_pszPrivateClassName = L"SoBrowserActionClassName";

HWND FindThreadToolBar( HWND hWndIEFrame ) {

	HWND hWndWorker = FindWindowEx( hWndIEFrame, NULL, L"WorkerW", NULL );
	if ( hWndWorker ) {
		HWND hWndRebar= FindWindowEx( hWndWorker, NULL, L"ReBarWindow32", NULL );
		if ( hWndRebar ) {
			HWND hWndBand = FindWindowEx( hWndRebar, NULL, L"ControlBandClass", NULL );
			if ( hWndBand ) {
				return FindWindowEx( hWndBand, NULL, L"ToolbarWindow32", NULL );
			}
		}
	}
	return 0;

}

void AddBrowserAction( HWND hWndIEFrame, HWND hWndToolBar ) {

	LRESULT lr = SendMessage( hWndToolBar, TB_BUTTONCOUNT, 0, 0 );
	UINT ButtonCount = (UINT)lr;
	for ( WPARAM index = 0; index < ButtonCount; ++index ) {
		TBBUTTON tbb;
		LRESULT lr = SendMessage( hWndToolBar, TB_GETBUTTON, index, reinterpret_cast<LPARAM>( &tbb ) );
		if ( lr == TRUE ) {
			if ( tbb.idCommand == 4242 ) return;
		}
	}
	HIMAGELIST hImgList = (HIMAGELIST)SendMessage( hWndToolBar, TB_GETIMAGELIST, 0, 0 );
	HIMAGELIST hImgListHot = (HIMAGELIST)SendMessage( hWndToolBar, TB_GETHOTIMAGELIST, 0, 0 );
	HIMAGELIST hImgListPressed = (HIMAGELIST)SendMessage( hWndToolBar, TB_GETPRESSEDIMAGELIST, 0, 0 );
	int cx, cy;
	BOOL bRetVal = ImageList_GetIconSize( hImgList, &cx, &cy );
	HBITMAP hBitMap = LoadBitmap( CCSoBABHO::sm_hInstance, MAKEINTRESOURCE( IDB_BITMAP_SO_LITTLE ) );
	int iImage = -1;
	if ( hImgList ) {
		iImage = ImageList_Add( hImgList, hBitMap, NULL );
	}
	if ( hImgListHot ) {
		ImageList_Add( hImgListHot, hBitMap, NULL );
	}
	if ( hImgListPressed ) {
		ImageList_Add( hImgListPressed, hBitMap, NULL );
	}
	TBBUTTON tbb;
	memset( &tbb, 0, sizeof( TBBUTTON ) );
	tbb.idCommand = 4242;
	tbb.iBitmap = iImage;
	tbb.fsState = TBSTATE_ENABLED;
	tbb.fsStyle = BTNS_BUTTON;
	lr = SendMessage( hWndToolBar, TB_INSERTBUTTON, 0, reinterpret_cast<LPARAM>( &tbb ) );
	if ( lr == TRUE ) {
		HWND hWndBand = GetParent( hWndToolBar );
		RECT rectBand;
		GetWindowRect( hWndBand, &rectBand );
		HWND hWndReBar = GetParent( hWndBand );
		POINT ptNew = { rectBand.left - cx, rectBand.top };
		ScreenToClient( hWndReBar, &ptNew );
		MoveWindow( hWndBand, ptNew.x, ptNew.y, rectBand.right - rectBand.left + cx, rectBand.bottom - rectBand.top, FALSE );
		RECT rect;
		GetWindowRect( hWndIEFrame, &rect );
		SetWindowPos( hWndIEFrame, NULL, rect.left, rect.top, rect.right - rect.left + 1, rect.bottom - rect.top, SWP_NOZORDER );
		SetWindowPos( hWndIEFrame, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER );
	}
	if ( hBitMap ) DeleteObject( hBitMap );
	return;
}

STDMETHODIMP CCSoBABHO::SetSite( IUnknown* pUnkSite ) {

	if ( pUnkSite ) {

		HRESULT hr = pUnkSite->QueryInterface( IID_IWebBrowser2, (void**)&m_spIWebBrowser2 );
		if ( SUCCEEDED( hr ) && m_spIWebBrowser2 ) {
			SHANDLE_PTR hWndIEFrame;
			hr = m_spIWebBrowser2->get_HWND( &hWndIEFrame );
			if ( SUCCEEDED( hr ) ) {
				bool bSuccess = CreatePrivateWindow();
				if ( bSuccess ) {
					m_hWndIEFrame = reinterpret_cast<HWND>( hWndIEFrame );
					wchar_t szFullPath[ MAX_PATH ];
					wcscpy_s( szFullPath, sm_szModulePath );
					wcscat_s( szFullPath, L"\\SoBrowserActionInjector.exe" );
					STARTUPINFO si;
					memset( &si, 0, sizeof( si ) );
					si.cb = sizeof( si );
					PROCESS_INFORMATION pi;
					wchar_t szCommandLine[ 128 ];
					swprintf_s( szCommandLine, L"SoBrowserActionInjector.exe %u", (DWORD32)hWndIEFrame );
					BOOL bWin32Success = CreateProcess( szFullPath, szCommandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
					if ( bWin32Success ) {
						CloseHandle( pi.hThread );
						CloseHandle( pi.hProcess );
					}
				}
			}
		}

	} else {

		DestroyPrivateWindow();
		m_spIWebBrowser2.Release();

	}

	return S_OK;
}


void CCSoBABHO::HookIfNotHooked( void ) {

	if ( sm_dwTlsIndex == TLS_OUT_OF_INDEXES ) return;

	HHOOK hHook = reinterpret_cast<HHOOK>( TlsGetValue( sm_dwTlsIndex ) );
	if ( hHook ) return;

	hHook = SetWindowsHookEx( WH_CALLWNDPROCRET, HookCallWndProcRet, sm_hInstance, GetCurrentThreadId() );
	TlsSetValue( sm_dwTlsIndex, hHook );

}

void CCSoBABHO::UnhookIfHooked( void ) {

	if ( sm_dwTlsIndex == TLS_OUT_OF_INDEXES ) return;
	HHOOK hHook = reinterpret_cast<HHOOK>( TlsGetValue( sm_dwTlsIndex ) );
	if ( UnhookWindowsHookEx( hHook ) ) TlsSetValue( sm_dwTlsIndex, 0 );

}

LRESULT CALLBACK CCSoBABHO::HookCallWndProcRet( int nCode, WPARAM wParam, LPARAM lParam ) {

	if ( nCode == HC_ACTION ) {
		PCWPRETSTRUCT pcwprets = reinterpret_cast<PCWPRETSTRUCT>( lParam );
		if ( pcwprets && ( pcwprets->message == WM_COMMAND ) ) {
			if ( LOWORD( pcwprets->wParam ) == 4242 ) {
				NotifyActiveBhoIE9( pcwprets->hwnd );
			}
		} else {
			if ( sm_uiRegisteredMsg == 0 ) sm_uiRegisteredMsg = RegisterWindowMessage( L"SOBA_MSG" );
			if ( sm_uiRegisteredMsg ) {
				if ( pcwprets && ( pcwprets->message == sm_uiRegisteredMsg ) ) {
					HookIfNotHooked();
					HWND hWndTB = FindThreadToolBar( pcwprets->hwnd );
					if ( hWndTB ) {
						AddBrowserAction( pcwprets->hwnd, hWndTB );
					}
				}
			}
		}
	}

	return CallNextHookEx( 0, nCode, wParam, lParam);

}

void CCSoBABHO::NotifyActiveBhoIE9( HWND hWndFromIEMainProcess ) {

	// Up to Main Frame
	HWND hWndChild = hWndFromIEMainProcess;
	while ( HWND hWndParent = GetParent( hWndChild ) ) {
		hWndChild = hWndParent;
	}
	HWND hwndIEFrame = hWndChild;

	// down to : find first "visible" FrameTab"
	struct ew {
		static BOOL CALLBACK ewp( HWND hWnd, LPARAM lParam ) {
			if ( ( GetWindowLongPtr( hWnd, GWL_STYLE ) & WS_VISIBLE ) == 0 ) return TRUE;
			wchar_t szClassName[ 32 ];
			if ( GetClassName( hWnd, szClassName, _countof( szClassName ) ) ) {
				if ( wcscmp( szClassName, L"Frame Tab" ) == 0 ) {
					*reinterpret_cast<HWND*>( lParam ) = hWnd;
					return FALSE;
				}
			}
			return TRUE;
		}
	};
	HWND hWndFirstVisibleTab = 0;
	EnumChildWindows( hwndIEFrame, ew::ewp, reinterpret_cast<LPARAM>( &hWndFirstVisibleTab ) );
	if ( hWndFirstVisibleTab == 0 ) return;

	// down to : find thread ID of first child
	HWND hWndThreaded = GetWindow( hWndFirstVisibleTab, GW_CHILD );
	if ( hWndThreaded == 0 ) return;
	DWORD dwTID = GetWindowThreadProcessId( hWndThreaded, NULL );
	wchar_t szWindowText[ 64 ];
	HWND hWndPrivate = FindWindow( sm_pszPrivateClassName, MakeWindowText( szWindowText, sizeof( szWindowText ), dwTID ) );
	if ( hWndPrivate ) SendMessage( hWndPrivate, WM_USER, 0, 0 );

}

void CCSoBABHO::RegisterPrivateClass( void ) {

	WNDCLASS wndclass;
	memset( &wndclass, 0, sizeof( wndclass ) );
	wndclass.hInstance = sm_hInstance;
	wndclass.lpszClassName = sm_pszPrivateClassName;
	wndclass.lpfnWndProc = wpPrivate;
	RegisterClass( &wndclass );
	return;

}

wchar_t * CCSoBABHO::MakeWindowText( wchar_t * pszBuffer, size_t cbBuffer, DWORD dwTID ) {

	swprintf( pszBuffer, cbBuffer / sizeof( wchar_t ), L"TID_%.04I32x", dwTID );
	return pszBuffer;

}

void CCSoBABHO::UnregisterPrivateClass( void ) {

	UnregisterClass( sm_pszPrivateClassName, sm_hInstance );
	return;

}

bool CCSoBABHO::CreatePrivateWindow( void ) {

	wchar_t szWindowText[ 64 ];
	m_hWndPrivate = CreateWindow( sm_pszPrivateClassName,
	                              MakeWindowText( szWindowText, sizeof( szWindowText ),
                                                 GetCurrentThreadId() ),
                                 0, 0, 0,0 ,0 ,NULL, 0, sm_hInstance, this );
	return m_hWndPrivate ? true : false;

}

LRESULT CALLBACK CCSoBABHO::wpPrivate( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {

	switch( uMsg ) {
		case WM_CREATE: {
			CREATESTRUCT * pCS = reinterpret_cast<CREATESTRUCT*>( lParam );
			SetWindowLongPtr( hWnd, GWLP_USERDATA,
			                  reinterpret_cast<LONG_PTR>( pCS->lpCreateParams ) );
			return 0;
		}
		case WM_USER: {
			CCSoBABHO * pThis = reinterpret_cast<CCSoBABHO*>( GetWindowLongPtr( hWnd, GWLP_USERDATA ) );
			if ( pThis ) pThis->OnActionClick( wParam, lParam );
			break;
		}
		default: return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
	return 0;

}

void CCSoBABHO::OnActionClick( WPARAM wParam, LPARAM lParam ) {

	OutputDebugString( L"In CCSoBABHO::OnActionClick\n" );

	return;

}

// CCSoBABHO

