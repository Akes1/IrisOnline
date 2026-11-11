#include "StdAfx.h"
#include "AppWindow.h"


TCHAR cAppWindow::mWindowClassName[] = _T("Dummy Client");

cAppWindow::cAppWindow()
{
	mWnd = 0;
}

cAppWindow::~cAppWindow()
{
	if( mWnd )
		::DestroyWindow(mWnd);
}

HWND cAppWindow::Create( HINSTANCE hi, unsigned int width, unsigned int height )
{
	mWidth = width;
	mHeight = height;

	RECT rect = { 0, 0, mWidth, mHeight };
	mWindowStyle = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN;

	::AdjustWindowRect( &rect, mWindowStyle, FALSE );

	mWnd = ::CreateWindow( GetWindowClassName(), GetWindowClassName(), mWindowStyle, 
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hi, NULL );

	if( mWnd )
	{
		::SetWindowPos( mWnd, 0, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER|SWP_FRAMECHANGED );
		mWindowStyle = ::GetWindowLong( mWnd, GWL_STYLE );

		::ShowWindow( mWnd, SW_SHOWNORMAL );
	}

	return mWnd;
}