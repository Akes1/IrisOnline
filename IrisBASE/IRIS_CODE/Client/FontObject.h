/* ====================================================================
* 파    일 :	FontObject.h
* 목    적 :	폰트 객체의 기본형 제공
* 작 성 자 :	이법석
* 작 성 일 :	2006.10.10
*
* 추가내용 :
*		추가날짜 	작업자		추가내용
*		06.10.10	이법석		생성 (묵향 엔진 참조)
* 
* 주의사항 :	
* =================================================================== */

#pragma once

struct sCacheInfo
{
	unsigned int maxCacheItemNum;
	unsigned int texWidth;
	unsigned int texHeight;
	unsigned int texNum;

	sCacheInfo()
	{
		maxCacheItemNum = 1024;
		texWidth = 1024;
		texHeight = 256;
		texNum = 2;
	}
};

class cFontObject
{
public:
	virtual ~cFontObject();

	/// 폰트의 생성
	virtual bool	CreateFont( HWND hwnd, LOGFONT* pLogFont, sCacheInfo& cacheInfo ) = 0;

	/// 장치 소실시 처리 루틴
	virtual void	Invalidate()	{ }
	virtual void	Restore()		{ }

	/// 글자 찍기
	virtual bool	DrawText( LPTSTR str, int strLen, RECT* pRect, unsigned long color, LPD3DXSPRITE pSprite, bool outerLine ) = 0;

	/// Font를 이용한 문자열에 대한 정보
	virtual long	GetTextExtent( HWND hwnd, LPCTSTR text, unsigned int textLen );

	inline long		GetTextWidth()	{ return mFontWidth; }
	inline long		GetTextHeight()	{ return mFontHeight; }

	/// 주의사항 : offscreen에 폰트를 그리는 경우는 등록 해라!!
	virtual void SetAlpha( bool /*bAlpha*/ ) {}

protected:
	cFontObject();

	virtual bool	SetFontInfo( HWND hwnd, LOGFONT* pLogFont );

protected:
	///	게임브리오 엔진에서 제공하는 Dx9객체
	NiDX9Renderer*		mpRenderer;

	/// 폰트핸들
	HFONT				mhFont;

	bool				mOuter;

	///	폰트 생성시 기본 정보
	LOGFONT				mLogFont;
	unsigned short		mFontWidth;
	unsigned short		mFontHeight;
};