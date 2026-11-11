#pragma once

class cAppWindow
{
public:
	cAppWindow();
	~cAppWindow();

	static LPCTSTR			GetWindowClassName()	{ return mWindowClassName; }

	HWND Create( HINSTANCE hi, unsigned int width, unsigned int height );

private:

	/// window cStageManager name
	static TCHAR		mWindowClassName[];

	/// window handle
	HWND		mWnd;

	/// window width
	unsigned int	mWidth;

	/// window height
	unsigned int	mHeight;

	/// window style
	unsigned long	mWindowStyle;

};
