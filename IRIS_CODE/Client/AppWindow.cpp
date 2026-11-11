#include "StdAfx.h"
#include "appwindow.h"

TCHAR cAppWindow::mWindowClassName[] = _T("IRIS Client");
HANDLE cAppWindow::mMutexHandle = NULL;

cAppWindow::cAppWindow()
{
}

cAppWindow::~cAppWindow()
{
	if( mMutexHandle )
		::CloseHandle(mMutexHandle);

	::DestroyWindow(mWnd);
}

/* ------------------------------------------------------------------
 * 함수이름 :	Create( NiInstanceRef hi, unsigned int width, unsigned int height )
 * 목    적 :	윈도우 생성
 * 주의사항 :	
 * ------------------------------------------------------------------ */
NiWindowRef cAppWindow::Create( NiInstanceRef hi, unsigned int width, unsigned int height ) 
{
	/// set window style (window mode로 생성)
	mWidth = width;
	mHeight = height;

	RECT rect = { 0, 0, mWidth, mHeight };
	mWindowStyle = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN;

	/// Create Window
//	::AdjustWindowRect(&rect, mWindowStyle, FALSE);
	::AdjustWindowRect( &rect, mWindowStyle, FALSE );

	mWnd = ::CreateWindow( GetWindowClassName(), GetWindowClassName(), mWindowStyle, 
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hi, NULL );

	if( mWnd )
	{
		::SetWindowPos( mWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED|SWP_HIDEWINDOW|SWP_NOACTIVATE );

		HMENU hMenu = GetSystemMenu( mWnd, FALSE );
		EnableMenuItem( hMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED );

		mWindowStyle = ::GetWindowLong( mWnd, GWL_STYLE );
		::ShowWindow( mWnd, SW_HIDE );
	}

	return mWnd;
}

void cAppWindow::AdjustWindowForChange( bool isFullScreen )
{
	HWND order = HWND_TOPMOST;
	if( isFullScreen )
	{
		mWindowStyle = WS_POPUP|WS_SYSMENU|WS_VISIBLE;
	}
	else
	{
		mWindowStyle = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN;

#if (defined(_DEVSYS) || defined(_DEBUG))
		order = HWND_NOTOPMOST;
#endif
	}
	::SetWindowLong( mWnd, GWL_STYLE, mWindowStyle );

	mWindowStyle = ::GetWindowLong( mWnd, GWL_STYLE );
	::SetWindowPos( mWnd, order, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE );
}

void cAppWindow::AdjustWindowRect( unsigned int width, unsigned int height, bool Init )
{
	mWidth = width;
	mHeight = height;

	if( mWidth >= (unsigned int)GetSystemMetrics(SM_CXSCREEN) || 
		mHeight >= (unsigned int)GetSystemMetrics(SM_CYSCREEN) )
	{
		mWindowStyle = WS_OVERLAPPED|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN;
		::SetWindowLong( mWnd, GWL_STYLE, mWindowStyle );

		mWindowStyle = ::GetWindowLong( mWnd, GWL_STYLE );
	}

	RECT rect = { 0, 0, mWidth, mHeight };
	::AdjustWindowRect( &rect, mWindowStyle, FALSE );

	if( Init )
		::SetWindowPos( mWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED|SWP_HIDEWINDOW|SWP_NOACTIVATE );
	else
		::SetWindowPos( mWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW|SWP_NOACTIVATE );

	// 전체 화면을 갱신한다.
	::InvalidateRect( NULL, 0, TRUE );  
}

#if (defined(_DEVSYS) || defined(_DEBUG))
	void cAppWindow::ChangeWindowOrder( bool /*top*/ )
	{
	}
#else
	void cAppWindow::ChangeWindowOrder( bool top )
	{
		if( top )
			::SetWindowPos( GetHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED );
		else
			::SetWindowPos( GetHWND(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED|SWP_NOACTIVATE );
	}
#endif
