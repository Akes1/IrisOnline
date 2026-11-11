#include "stdafx.h"
#include "WebManager.h"
#include "resource.h"
#include "RenderSystem.h"
#include <Mshtml.h>

#include "oaidl.h"
#include "oleauto.h"
#include "ole2.h"

//#include <SHLGUID.h>

#define WEBSITE L"http://iris.enpang.com/community/free/" //L"http://iris.eyainteractive.com/launcher/iris_launcher.asp"

cWebManager* cWebManager::mpSingleton = 0;

cWebManager::cWebManager()
: mParentWnd( 0 )
, mMainDlg( 0 )
, mInstance( 0 )
, mpWebBrowser( 0 )
{
	mpSingleton = this;
}

cWebManager::~cWebManager()
{
	Release();

	mpSingleton = 0;
}

void cWebManager::Release()
{
	//if( mpWebBrowser )
	//	mpWebBrowser->Quit();

	//OleUninitialize();

	//DestroyWindow( mMainDlg );
}

bool cWebManager::Init( HINSTANCE hinst, HWND hwnd )
{
	mInstance  = hinst;
	mParentWnd = hwnd;

	//if( FAILED(OleInitialize(NULL)) )
	//{
	//	assert(0);
	//	return false;
	//}

	//if( FAILED( CoCreateInstance( CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID*)&mpWebBrowser ) ))
	//{
	//	assert(0);
	//	return false;
	//}

	//mMainDlg = CreateDialog( hinst, MAKEINTRESOURCE(IDD_DIALOG_WEB), mParentWnd, DlgProc );

	//if( !mpWebBrowser )
	//{
	//	assert(0);
	//	return false;
	//}

	//// 웹페이지 - 다이얼로그 부모 연결
	//HWND wepHwnd;
	//mpWebBrowser->get_HWND( (long*)&wepHwnd );
	//SetParent( wepHwnd, mMainDlg );

	//// 속성 설정
	//mpWebBrowser->put_AddressBar( VARIANT_FALSE );
	//mpWebBrowser->put_ToolBar( VARIANT_FALSE );
	//mpWebBrowser->put_MenuBar( VARIANT_FALSE );
	//mpWebBrowser->put_Resizable( VARIANT_FALSE );
	//mpWebBrowser->put_StatusBar( VARIANT_FALSE );
	//mpWebBrowser->put_FullScreen( VARIANT_TRUE );
	return true;
}

void cWebManager::UpdateRect( unsigned int x, unsigned int y, unsigned int w, unsigned int h )
{
	x, y, w, h;
	//RECT rect;
	//GetClientRect( mParentWnd, &rect );

	//rect.left += x;
	//rect.top += y;
	//rect.right = rect.left + w;
	//rect.bottom = rect.top + h;

	//::SetWindowPos( mMainDlg, 0, rect.left, rect.top, w, h, SWP_NOZORDER | SWP_NOACTIVATE );
	////MoveWindow( mMainDlg, rect.left, rect.top, rect.right, rect.bottom, TRUE );

	//if( mpWebBrowser )
	//{
	//	mpWebBrowser->put_Width( w );
	//	mpWebBrowser->put_Height( h );	
	//}
}

void cWebManager::Show()
{
	//// modal로 띄우기
	//ShowWindow( mMainDlg, SW_SHOWNORMAL );

	//if( mpWebBrowser )
	//{
	//	// 웹페이지 연결
	//	VARIANT v;
	//	VARIANT vTarget;
	//	VariantInit( &v );
	//	VariantInit( &vTarget );

	//	BSTR URL = SysAllocString( WEBSITE );

	//	/*	BSTR target = SysAllocString( L"_self" );
	//	V_VT( &vTarget ) = VT_BSTR;
	//	V_BSTR( &vTarget ) = target;
	//	*/

	//	/*V_VT( &vtHeader ) = VT_BSTR;
	//	V_BSTR( &vtHeader ) = bstrHeader;*/


	//	//VARIANT_BOOL bBusy;
	//	//mpWebBrowser->get_Busy( &bBusy );

	//	HRESULT hr = mpWebBrowser->Navigate( URL, &v, &vTarget, &v, &v );
	//	if( SUCCEEDED( hr ) )
	//	{
	//		mpWebBrowser->put_Visible( VARIANT_TRUE );
	//	}
	//	else
	//	{
	//		mpWebBrowser->Quit();
	//	}

	//	SysFreeString( URL );

	//	////
	//	//IDispatch* pDisp = NULL;
	//	//if( FAILED( mpWebBrowser->get_Document( &pDisp )) )
	//	//{
	//	//	int a;
	//	//	a = 1;
	//	//}

	//	//IHTMLDocument2* pHTMLDocument2;
	//	//if( FAILED(pDisp->QueryInterface( IID_IHTMLDocument2, (void**)&pHTMLDocument2 )) )
	//	//{
	//	//	int a;
	//	//	a = 1;

	//	//}

	//	//pHTMLDocument2->PUT_

	//	//*///pHTMLDocument2->get_frames()
	//}
}

void cWebManager::Hide()
{
	//ShowWindow( mMainDlg, SW_HIDE );
}

INT_PTR CALLBACK cWebManager::DlgProc( HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
	case WM_MOVE:
		{
		}
		break;
	case WM_COMMAND:
		{
		}
		break;
	case WM_SYSCOMMAND:
		{
			/*switch( wparam )
			{
			case SC_CLOSE:
			{
			}
			break;
			}*/
		}
		break;
	case WM_CLOSE:
		{
			//WEBMAN->Hide();
		}
		break;
	case WM_SHOWWINDOW:
		{
			if( wparam == TRUE )
			{

			}
			else
			{
				//	WEBMAN->Hide();
			}
		}
	}
	return FALSE;
}

