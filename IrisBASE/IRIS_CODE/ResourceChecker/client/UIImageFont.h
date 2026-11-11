/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.11
* 내    용 : 이미지 폰트
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIImage.h"

enum eIMAGEFONT_TYPE
{
	eIAMGEFONT_ITEMCOUNT = 0,	// 인벤토리 아이템 갯수
	eIAMGEFONT_TAROTSTR,		// 타로 - 정방향, PVP 아군 포인트
	eIAMGEFONT_TAROTCALC,		// 타로 - 계산식
	eIAMGEFONT_TAROTINV,		// 타로 - 역방향, PVP 적군 포인트
	eIAMGEFONT_TAROTDEFAULT,	// 타로 - 기본
	eIAMGEFONT_PVPTIME,			// PVP - 시간
	eIAMGEFONT_PVPDESTPOINT,	// PVP - 목표 시간
	eIAMGEFONT_HEROLEVEL,		// 케릭터 정보창 - 래밸
};

/// 이미지 폰트
class cUIImageFont : cUIImage
{
public:
	cUIImageFont( NiTexture* tex, unsigned int texX, unsigned int texY, unsigned int charWidth, unsigned int charHeight );
	virtual ~cUIImageFont();

	/// 렌더링
	void DrawText( unsigned int x, unsigned int y, const char* text );

	/// 문자열
	unsigned int GetTextWidth( unsigned int num ) const;
	unsigned int GetTextHeight() const;

protected:
	unsigned int mTexX;
	unsigned int mTexY;
	unsigned int mCharWidth;
	unsigned int mCharHeight;
};

inline unsigned int cUIImageFont::GetTextHeight() const
{
	return mCharHeight;
}
