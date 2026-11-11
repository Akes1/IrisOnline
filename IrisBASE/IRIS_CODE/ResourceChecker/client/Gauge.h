/* ==========================================================================
* 파    일 : Gauge.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UINodeData.h"

/// 게이지에 변화가 있을때, 해당수치를 기준으로 그라데이션 컬러를 넣는다.
const float GRADATION_VALUE = 10.0f;

//class cPlaneObject;
/// 기본 게이지바
class cGauge : public cUINode
{
public:
	enum
	{
		eGaugeState_Start = 0,
		eGaugeState_Process,
		eGaugeState_End,

		eGaugeState_None,
	};

	cGauge( eUINodeType type = eUINODE_GAUGE );
	virtual ~cGauge();

protected:
	/// 게이지
//	cPlaneObject* mpGaugeImage;

	/// 게이지 값
	float mValue;

	/// 목표 값 (애니메이션목표)
	float mDestValue;

	/// 최대 값
	float mMaxValue;

	/// 게이지 퍼센트
	float mPercent;
	
	/// 게이지 상태
	unsigned int mState;

	/// 시간값
	unsigned long mStartTime;

	/// 게이지 차는 속도
	float mSpeed;

	/// 퍼센트용 라벨
	bool mShowPercent;

	/// 게이지 값 보기 여부
	bool mShowText;
	int mTextX, mTextY;
	cStringT mText;
	
	/// 게이지 텍스쳐
	NiTexture*	mpImageTexture;		

	/// 게이지 좌표
	cUIPos		mImagePos;

	/// 게이지 텍스쳐 사이즈
	cUISize		mImageSize;

	/// 게이지 텍스쳐 좌표
	cUIPos		mImageTexpos;

	/// 게이지 100% 기준 사이즈
	cUISize		mGaugeSize;

	/// 게이지 출력 위지
	bool		mImageStart;

	bool		mRightDecrease;
};


/////////////////////////////////////////////////////////////
/// 이중 게이지 바
class cDoubleGauge : public cGauge
{
public:
	cDoubleGauge( eUINodeType type = eUINODE_DOUBLEGAUGE );
	virtual ~cDoubleGauge();

protected:
	/// 게이지
//	cPlaneObject* mpDestGaugeImage;

	/// 게이지 텍스쳐
	NiTexture*	mpDestImageTexture;		

	/// 게이지 좌표
	cUIPos		mDestImagePos;

	/// 게이지 텍스쳐 사이즈
	cUISize		mDestImageSize;

	/// 게이지 텍스쳐 좌표
	cUIPos		mDestImageTexpos;

	/// 게이지 100% 기준 사이즈
	cUISize		mDestGaugeSize;

	/// 게이지 출력 위지
	bool		mDestImageStart;
};

class cGaugeTB : public cGauge
{
public:
	cGaugeTB( eUINodeType type = eUINODE_GAUGETB );
	virtual ~cGaugeTB();

};

//////////////////////////////////////////////////////////////////////////////////
class cGaugeSkin : public cUINodeSkin
{
public:
	cGaugeSkin( eUINodeType type = eUINODE_GAUGE );
	virtual ~cGaugeSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 퍼센트용 라벨
	bool mShowPercent;
	bool mShowText;

	/// 게이지 차는 속도
	float mGaugeSpeed;

	/// 게이지 정보 세팅
	NiTexture*	mpImageTexture;
	cUIPos		mImagePos;
	cUISize		mImageSize;
	cUIPos		mImageTexpos;
	cUISize		mGaugeSize;
	bool		mImageStart;

	NiTexture*	mpDestImageTexture;
	cUIPos		mDestImagePos;
	cUISize		mDestImageSize;
	cUIPos		mDestImageTexpos;
	cUISize		mDestGaugeSize;
	bool		mDestImageStart;
};
