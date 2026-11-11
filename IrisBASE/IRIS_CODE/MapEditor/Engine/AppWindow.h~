/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.20
* 내    용 : 어플리케이션 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 어플리케이션 윈도우
class cAppWindow
{
public:
	cAppWindow( HWND wnd );
	virtual ~cAppWindow();

	// 주의: 임의로 width, height의 변경 함수를 만들지 말것.
	// 변경은 항상 RenderSystem을 통해서 ToggleWindow함수 내부에서만 가능하도록 설정한다.
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;

	/// 핸들을 리턴
	NiWindowRef GetHandle() const;

	/// 스크린 모드를 변경
	void ToggleWindow( bool fullScreen );

private:
	/// 핸들
	NiWindowRef mHandle;
	
	/// 크기
	unsigned int mWidth;
	unsigned int mHeight;
	
	/// 윈도우 스타일
	unsigned long mWindowStyle;
};

inline unsigned int	cAppWindow::GetWidth() const
{
	return mWidth;
}

inline unsigned int	cAppWindow::GetHeight() const
{
	return mHeight;
}

inline NiWindowRef cAppWindow::GetHandle() const
{
	return mHandle;
}
