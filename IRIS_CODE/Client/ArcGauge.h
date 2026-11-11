/* ==========================================================================
* 파    일 : ArcGauge.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-02-16
* 주의사항 : 반 호로 된 게이지 구현 ( 히어로 원형게이지 )
*===========================================================================*/
#pragma once

class cPlaneObject;
class cArcGauge
{
public:
	cArcGauge();
	virtual ~cArcGauge();

	/// 생성부 (모두 절대 좌표다!!)
	bool Create( NiTexture* pTexture, unsigned short texLeft, unsigned short texTop, unsigned short texRight,unsigned short texBottom );

	/// 출력부
	void Draw();

	/// 위치 셋팅 (절대 좌표를 넣자!!)
	void SetPos( short x, short y );

	/// 게이지 현재 값
	void SetCurrentValue( float curvalue );

	/// 게이지 최고 값
	void SetMaxValue( float maxvalue );

	/// 알파 시작 값
	void SetStartRef( unsigned int alphaRef );

protected:
	/// 알파값 계산 및 세팅 
	void CalculateAlphaRef();

protected:
	///
	cPlaneObject*	mpPlane;

	/// 현재 값
	float mCurValue;
	/// 최고 값
	float mMaxValue;
	/// 게이지 이미지에 설정된 최고 알파 값
	float mMaxAlphaRef;

	/// 시작 알파 값 
	unsigned int mStartAlpahRef;
};

inline void cArcGauge::SetCurrentValue( float curvalue )
{
	mCurValue = curvalue;

	if( mCurValue >= mMaxValue )
		mCurValue = mMaxValue;

	/// 현재 알파 재세팅
	CalculateAlphaRef();
}

inline void cArcGauge::SetMaxValue( float maxvalue )
{
	mMaxValue = maxvalue;
	/// 현재 알파 재세팅
	CalculateAlphaRef();
}

inline void cArcGauge::SetStartRef( unsigned int alphaRef )
{
	mStartAlpahRef = alphaRef;
}