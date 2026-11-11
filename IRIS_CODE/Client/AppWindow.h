/* ====================================================================
 * 파    일 :	AppWindow.h
 * 목    적 :	윈도우 창에대한 관리 및 정보 제공
 * 작 성 자 :	이법석
 * 작 성 일 :	2005.12
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		 2005.12	이법석	     생성
 * 
 * 주의사항 :	1. 절대로 임의로 width, height의 변경 함수를 만들지 말것..
 *				   변경은 항상 RenderSystem을 통해서 UpdateWindow함수 내부에서만 
 *				   가능하도록 설정한다.
 * =================================================================== */

#pragma once

class cAppWindow
{
public:
	cAppWindow();
	~cAppWindow();

	/// window mutex handle
	static HANDLE	mMutexHandle;

	static LPCTSTR			GetWindowClassName()	{ return mWindowClassName; }

	inline unsigned int		GetWidth() const	{ return mWidth; }
	inline unsigned int		GetHeight() const	{ return mHeight; }

	inline NiWindowRef		GetHWND() const		{ return mWnd; }

	/// 윈도우 생성
	NiWindowRef				Create( NiInstanceRef hi, unsigned int width, unsigned int height );

	//	/// window mode 변경
	void AdjustWindowForChange( bool isFullScreen );
	void AdjustWindowRect( unsigned int width, unsigned int height, bool Init = false );

	void ChangeWindowOrder( bool top );

public:
	/// window cStageManager name
	static TCHAR		mWindowClassName[];

private:

	/// window handle
	NiWindowRef		mWnd;

	/// window width
	unsigned int	mWidth;

	/// window height
	unsigned int	mHeight;

	/// window style
	unsigned long	mWindowStyle;
};
