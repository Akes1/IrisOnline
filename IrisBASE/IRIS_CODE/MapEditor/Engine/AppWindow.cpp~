#include "stdafx.h"
#include "AppWindow.h"

cAppWindow::cAppWindow( HWND wnd )
: mHandle( wnd )
{
	mWidth = 0;
	mHeight = 0;
}

cAppWindow::~cAppWindow()
{
}

void cAppWindow::ToggleWindow( bool fullScreen )
{
	if( fullScreen )
	{
		mWindowStyle = WS_POPUP|WS_SYSMENU|WS_VISIBLE;
		::SetWindowPos( mHandle, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE );
	}
	else
	{
		mWindowStyle = WS_CHILD|WS_VISIBLE;
		::SetWindowPos( mHandle, HWND_NOTOPMOST, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE );
	}

	::SetWindowLong(mHandle, GWL_STYLE, mWindowStyle);
	::SetWindowPos(mHandle, 0, 0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
}
