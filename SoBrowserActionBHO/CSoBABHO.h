// CSoBABHO.h : Declaration of the CCSoBABHO

#pragma once
#include "resource.h"       // main symbols



#include "SoBrowserActionBHO_i.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;


// CCSoBABHO

class ATL_NO_VTABLE CCSoBABHO :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCSoBABHO, &CLSID_CSoBABHO>,
	public IObjectWithSiteImpl<CCSoBABHO>,
	public IDispatchImpl<ICSoBABHO, &IID_ICSoBABHO, &LIBID_SoBrowserActionBHOLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	CCSoBABHO() : m_hWndPrivate( 0 )
	{
	}

	//DECLARE_REGISTRY_RESOURCEID(IDR_CSOBABHO)
	static HRESULT WINAPI UpdateRegistry( BOOL bRegister ) throw() {
		ATL::_ATL_REGMAP_ENTRY regMapEntries[2];
		memset( &regMapEntries[1], 0, sizeof(ATL::_ATL_REGMAP_ENTRY));
		regMapEntries[0].szKey = L"MODULEPATH";
		regMapEntries[0].szData = sm_szModulePath;
		return ATL::_pAtlModule->UpdateRegistryFromResource(IDR_CSOBABHO, bRegister, regMapEntries);
	}

DECLARE_NOT_AGGREGATABLE(CCSoBABHO)

BEGIN_COM_MAP(CCSoBABHO)
	COM_INTERFACE_ENTRY(ICSoBABHO)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

	STDMETHOD(SetSite)(IUnknown *pUnkSite);

	static HMODULE sm_hInstance;
	static DWORD sm_dwTlsIndex;
	static wchar_t sm_szModulePath[ MAX_PATH ];
	static LRESULT CALLBACK HookCallWndProcRet( int nCode, WPARAM wParam, LPARAM lParam );
	static void UnhookIfHooked( void );
	static void RegisterPrivateClass( void );
	static void UnregisterPrivateClass( void );
	static wchar_t * MakeWindowText( wchar_t * pszBuffer, size_t cbBuffer, DWORD dwTID );

private:
	CComPtr<IWebBrowser2> m_spIWebBrowser2;
	HWND m_hWndIEFrame;
	HWND m_hWndPrivate;
	static void NotifyActiveBhoIE9( HWND hwndFromIEMainProcess );
	static UINT sm_uiRegisteredMsg;
	static void HookIfNotHooked( void );
	static LRESULT CALLBACK wpPrivate( HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam );
	bool CreatePrivateWindow( void );
	static wchar_t * sm_pszPrivateClassName;
	void OnActionClick( WPARAM wParam, LPARAM lParam );
	void DestroyPrivateWindow( void ) {
		if ( m_hWndPrivate ) DestroyWindow( m_hWndPrivate );
	};
};

OBJECT_ENTRY_AUTO(__uuidof(CSoBABHO), CCSoBABHO)
