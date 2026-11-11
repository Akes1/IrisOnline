/* ====================================================================
* 파    일 :	D3DXFont.h
* 목    적 :	캐쉬폰트를 사용하기위한 성능 비교 및 테스트 진행용
* 작 성 자 :	이법석
* 작 성 일 :	2006.10.17
*
* 추가내용 :
*		추가날짜 	작업자		추가내용
*		06.10.17	이법석		생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once

#include "FontObject.h"

class cD3DXFont : public cFontObject
{
public:
	cD3DXFont();
	virtual ~cD3DXFont();

	/// 폰트의 생성
	virtual bool	CreateFont( HWND hwnd, LOGFONT* pLogFont, sCacheInfo& cacheInfo );

	/// 장치 소실시 처리 루틴
	virtual void	Invalidate();
	virtual void	Restore();

	/// 글자 찍기
	virtual bool	DrawText( LPTSTR str, int strLen, RECT* pRect, unsigned long color, LPD3DXSPRITE pSprite, bool outerLine );

protected:

	/// DX font 객체
	LPD3DXFONT			mpD3dxFont;
};
