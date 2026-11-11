/* ==========================================================================
* 파    일 : CardImage.h
* 목    적 : 카드이미지 출력용 
* 작 성 자 : 김가람
* 작 성 일 : 2007-07-31
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Label.h"

const int CARD_WIDTH = 120;
const int CARD_HEIGHT = 200;

class cEffetAniPlane;
class cCardParam
{
public:
	/// 텍스쳐
	NiTexture*	mpTexture;

	/// 좌표
	cUIPos	mTexPos;
};

class cCardImage : public cLabel
{
public:
	cCardImage( eUINodeType type = eUINODE_LABEL );
	virtual ~cCardImage();

	enum
	{
		eFade_None = 0,
		eFade_In,
		eFade_Out,

		/// 확장!!
		eFade_In_Out, /// fadein -> fadeout
		eFade_Out_In, /// fadeout -> fadein
	};


private:
	/// 이미지 뒤집기 
	bool mShowImage;

	unsigned char mFadeState;
	unsigned long mAccumTime;
	unsigned long mFadeTime;

	/// 뒤집기 락
	bool mLock;

	/// 카드 이미지 저장 
	cCardParam* mpOpenParam;

	/// 뒷면 이미지 저장 
	cCardParam* mpCloseParam;

	unsigned long mTarotItemIdx;

	cEffetAniPlane* mOpenImage;
	cEffetAniPlane* mCloseImage;

	cEffetAniPlane*	mOverImage;
};