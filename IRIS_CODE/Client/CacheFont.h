/* ====================================================================
 * 파    일 :	CacheFont.h
 * 목    적 :	실시간 이미지 폰트를 생성해서 사용
 * 작 성 자 :	이법석
 * 작 성 일 :	2006.11.10
 *
 * 추가내용 :
 *		추가날짜 	작업자		추가내용
 *		06.11.10	이법석		묵향 소스 참조
 * 
 * 주의사항 :	
 * =================================================================== */
#pragma once

#include "FontObject.h"
#include "CacheAgent.h"

class cCacheFont : public cFontObject
{
public:
	cCacheFont();
	virtual ~cCacheFont();

	/// 폰트의 생성
	virtual bool CreateFont( HWND hwnd, LOGFONT* pLogFont, sCacheInfo& cacheInfo );

	/// 글자 찍기
	virtual bool DrawText( LPTSTR str, int strLen, RECT* pRect, unsigned long color, LPD3DXSPRITE pSprite, bool outerLine );

	void	Invalidate();
	void	Restore();

protected:
	/// 캐쉬 관리자
	cCacheAgent*			mpCacheAgent;

	sCacheInfo				mCacheInfo;

	/// 캐쉬 관리자용 전달자
	sFontPropertyDesc		mFontPropertyDesc;
};