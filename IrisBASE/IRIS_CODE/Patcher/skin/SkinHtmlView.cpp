#include "stdafx.h"
#include "SkinHtmlView.h"
#include "SkinResource.h"
#include <Mshtml.h>

BEGIN_MESSAGE_MAP(cSkinHtmlView, CHtmlView)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE( cSkinHtmlView, CHtmlView )

cSkinHtmlView::cSkinHtmlView()
{
	mConnected = true;
}

cSkinHtmlView::~cSkinHtmlView()
{
}

BOOL cSkinHtmlView::Create( cSkinComponentResource* resource, CWnd* parent, UINT id )
{
	/// 리소스 설정
	ASSERT( resource );
	mResource = (cSkinHtmlViewResource*)resource;
	mUrl = mResource->mUrl;

	return CHtmlView::Create( 0, 0, WS_CHILD | WS_VISIBLE, mResource->mRect, parent, id );
}

void cSkinHtmlView::Go()
{
	Navigate2( mUrl.Cstr() );
}

int cSkinHtmlView::OnCreate( LPCREATESTRUCT cs ) 
{
	if(CHtmlView::OnCreate( cs ) == -1)
		return -1;

	return 0;
}

void cSkinHtmlView::OnPaint() 
{
	CPaintDC dc(this);
	// Do not call CHtmlView::OnPaint() for painting messages
}

void cSkinHtmlView::OnDocumentComplete( LPCTSTR lpszURL ) 
{
	if( mConnected )
	{
		ShowWindow(SW_SHOW);
	}

	// HtmlView의 스크롤바 없애기
	IDispatch *pDisp = this->GetHtmlDocument();
	if ( pDisp != NULL )
	{
		IHTMLDocument2* pHTMLDocument2;
		HRESULT hr;
		hr = pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pHTMLDocument2);
		if (hr == S_OK)
		{
			IHTMLElement *pIElement;
			hr = pHTMLDocument2->get_body(&pIElement);

			IHTMLBodyElement *pIBodyElement;
			hr = pIElement->QueryInterface(IID_IHTMLBodyElement,(void**)&pIBodyElement);

			BSTR bstr;
			CString strTmp;
			strTmp = "no";

			bstr = strTmp.AllocSysString();

			pIBodyElement->put_scroll(L"no");

			pIBodyElement->Release();
			pIElement->Release();
			::SysFreeString( bstr );
		}

		pHTMLDocument2->Release();
		pDisp->Release();

	}

	CHtmlView::OnDocumentComplete(lpszURL);
}

void cSkinHtmlView::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD flags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel) 
{
	if(strncmp(lpszURL, "res", 3) == 0)
	{
		mConnected = false;
		ShowWindow( SW_HIDE );
	}
	CHtmlView::OnBeforeNavigate2(lpszURL, flags, lpszTargetFrameName, baPostedData, lpszHeaders, pbCancel);
}

void cSkinHtmlView::OnDownloadBegin() 
{
	CHtmlView::OnDownloadBegin();
}

void cSkinHtmlView::OnDownloadComplete() 
{
	CHtmlView::OnDownloadComplete();
}

HRESULT cSkinHtmlView::OnTranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID) 
{ 
	if ( (lpMsg->message == WM_KEYDOWN) )// && (lpMsg->wParam == VK_F5) ) 
		return S_OK; 

	return CHtmlView::OnTranslateAccelerator(lpMsg, pguidCmdGroup, nCmdID); 
} 

void cSkinHtmlView::OnNavigateComplete2(LPCTSTR strURL) 
{
	CHtmlView::OnNavigateComplete2(strURL);
}

HRESULT cSkinHtmlView::OnGetHostInfo(DOCHOSTUIINFO * pInfo)
{
	pInfo->dwFlags = DOCHOSTUIFLAG_NO3DBORDER;
	pInfo->dwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT;

	return S_OK;
}
