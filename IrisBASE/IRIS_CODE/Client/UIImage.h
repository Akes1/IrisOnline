/* ==========================================================================
* 파    일 : UIImage.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.25
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIImageBase.h"

/// UI 이미지 - 확대, 축소 가능
class cUIImage : public cUIImageBase
{
public:
	cUIImage();
	cUIImage( NiTexture* pTex );
	cUIImage( NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th );
	virtual ~cUIImage();

	/// 텍스처를 설정
	virtual void SetTexture( NiTexture* ptex );

	/// 렌더링
	virtual void Draw();

	/// 스크린 좌표 셋팅
	void SetPos( int x, int y );

	/// 스크린 상에서 크기
	void SetScreenWH( unsigned int screenW, unsigned int screenH );
	void SetScreenRect( const cUIRect& rect );

	/// 갱신
	void UpdateImage();

	/// 텍스쳐 UV 셋팅
	void SetTextureRect( unsigned int tl, unsigned int tt, unsigned int tr, unsigned int tb );
	void SetTextureRect( const cUIRect& rect );

	/// 정점 컬러 정보 셋팅
	void SetColor( float r, float g, float b, float a );
	void SetColor( const NiColorA& color );

	virtual void SetAlpha( float a );

	/// 회전 및 이동
	/// 절대좌표.
	void SetTranslate( int x, int y );
	void SetRotate( float angle );

	inline NiAlphaProperty* GetAlphaProp() { return mpAlphaProp; }
	inline NiTexturingProperty* GetTexturingProp() { return mpTextureProp; }

	int GetScreenPosX(){ return mScreenX; }
	int GetScreenPosY(){ return mScreenY; }

	cUIRect GetScreenRect(){ return cUIRect( mScreenX, mScreenY, mScreenX + mWidthOnScreen, mScreenY + mHeightOnScreen ); }

	virtual void SetRenderTargetWH( unsigned int width, unsigned int height );

protected:
	/// 스크린 객체
	NiMeshScreenElementsPtr mpScreenElement;

	/// 텍스쳐 정보의 변경을 용이하게 하기 위한 참조 및 관리 변수
	NiTexturingProperty*	mpTextureProp;

	/// 알파 블랜딩을 위한 참조 및 관리 변수
	NiAlphaProperty*		mpAlphaProp;

	/// 화면상에서의 위치 및 크기
	int			mScreenX;
	int			mScreenY;

	float mScaleX;
	float mScaleY;

	unsigned int	mRenderTargetWidth;
	unsigned int	mRenderTargetHeight;

	/// 회전 각도
	/// 회전 각도
	float mAngle;
	NiPoint2 mVertex[4];
	NiPoint2 mCenter;
};