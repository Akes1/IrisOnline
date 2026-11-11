/* ==========================================================================
* 파    일 : UIImage.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.25
* 주의사항 : 
*===========================================================================*/
#pragma once

/// UI 이미지 - 확대, 축소 가능
class cUIImage
{
public:
	cUIImage( NiTexture* tex, unsigned int x, unsigned int y, unsigned int tx, unsigned int ty, unsigned int tw, unsigned int th );
	virtual ~cUIImage();

	/// 렌더링
	virtual void Draw();

	virtual void SetAlpha( float a );

protected:
	/// 스크린 객체
	NiMeshScreenElements* mpScreenElement;

	/// 텍스쳐 정보의 변경을 용이하게 하기 위한 참조 및 관리 변수
	NiTexturingProperty*	mpTextureProp;

	/// 알파 블랜딩을 위한 참조 및 관리 변수
	NiAlphaProperty*		mpAlphaProp;

	/// 화면상에서의 위치 및 크기
	short			mScreenX;
	short			mScreenY;

	float mScaleX;
	float mScaleY;

	/// 회전 각도
	/// 회전 각도
	float mAngle;
	NiPoint2 mVertex[4];
	NiPoint2 mCenter;
};