/* ==========================================================================
* 파    일 : UIImageBase.h
* 목    적 : ScreenTexture 와 ScreenElements를 구분해서 사용하기 위한 구조를 위해서
* 작 성 자 : 이법석
* 작 성 일 : 2007.02.26
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIRect.h"
typedef tUIRect<int> cUIRect;

class cUIImageBase
{
public:
	virtual ~cUIImageBase() {}

	virtual void Draw() = 0;

	/// 텍스처를 설정
	virtual void SetTexture( NiTexture* ptex ) = 0;

	/// 이미지만을 변경한다.
	virtual void ChangeImage( NiTexture* /*ptex*/ ) {}

	/// 스크린 상의 위치를 설정 [0, 1]
	virtual void SetPos( int x, int y ) = 0;

	/// 스크린 상의 크기를 설정 [0, 1]
	virtual void SetSize( float /*w*/, float /*h*/ ) {}

	/// 스크린 상의 영역을 설정 [0, 1]
	virtual void SetScreenRect( const cUIRect& rect ) = 0;

	virtual void SetTextureRect( unsigned int tl, unsigned int tt, unsigned int tr, unsigned int tb ) = 0;
	virtual void SetTextureRect( const cUIRect& /*rect*/ ) {}

	/// 색상을 설정
	virtual void SetColor( const NiColorA& color ) = 0;
	virtual NiColorA& GetColor() { return mColor; }

	/// 알파 설정
	virtual void SetAlpha( float /*a*/ ) {}

	/// 회전
	virtual void SetTranslate( float /*x*/, float /*y*/ ) {}
	virtual void SetRotate( float /*angle*/ ) {}

	/// 적합 여부를 리턴
	virtual bool IsValid() const;

	/// 텍스처 영역을 리턴
	const cUIRect& GetTextureRect() const;

	/// 이미지의 넓이 리턴
	unsigned int GetWidth() const;

	/// 이미지의 높이 리턴 
	unsigned int GetHeight() const;

	virtual void SetRenderTargetWH( unsigned int /*width*/, unsigned int /*height*/ ) {}

protected:
	cUIImageBase() 
	{
		mTextureWidth = 0;
		mTextureHeight = 0;
	
		mWidthOnScreen = 1;
		mHeightOnScreen = 1;

		mTexRect.Set( 0, 0, 1, 1 );
		mColor = NiColorA::WHITE;
	}

protected:
	/// 텍스처 크기
	unsigned int mTextureWidth;
	unsigned int mTextureHeight;

	NiColorA mColor;

	/// 텍스처 영역 [0, 1]
	cUIRect mTexRect;

	unsigned int mWidthOnScreen;
	unsigned int mHeightOnScreen;
};

inline bool cUIImageBase::IsValid() const
{
	if( mTextureWidth == 0 || mTextureHeight == 0 )
	{
		assert( 0 && "invalid base image size" );
		return false;
	}

	return true;
}

inline const cUIRect& cUIImageBase::GetTextureRect() const
{
	return mTexRect;
}

inline unsigned int cUIImageBase::GetWidth() const
{
	return mWidthOnScreen;
}

inline unsigned int cUIImageBase::GetHeight() const
{
	return mHeightOnScreen;
}
