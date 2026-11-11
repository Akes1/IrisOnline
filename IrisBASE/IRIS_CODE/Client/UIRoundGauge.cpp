#include "StdAfx.h"
#include "uiroundgauge.h"
#include "PlaneObject.h"

#include "Application.h"

cUIRoundGauge::cUIRoundGauge()
: mpPlane(0)
, mStartTime(0)
, mMaintainTime(0)
, mElapsedTime(0)
, mActivate( false )
, mGaugeState( eGaugeState_None )
{
}

cUIRoundGauge::~cUIRoundGauge()
{
	SAFE_DELETE(mpPlane);
}

/// 주의사항 : 아이콘 크기와 맞게 고정한다.
bool cUIRoundGauge::Create( NiTexture* pTexture, unsigned short texLeft, unsigned short texTop, unsigned short texRight,unsigned short texBottom )
{
	/// 0324 웅주
	if( mpPlane )
	{
		SAFE_DELETE( mpPlane );
	}

	/// 객체 생성
	mpPlane = new cPlaneObject;
	if( !mpPlane->Create( pTexture, 0, 0, texRight - texLeft, texBottom - texTop,
							texLeft, texTop, texRight, texBottom ) )
	{
		return false;
	}

	/// 알파 옵션 조절
	NiAlphaProperty* pAlphaProp = mpPlane->GetAlphaProp();
	pAlphaProp->SetAlphaTesting(true);
	pAlphaProp->SetTestMode(NiAlphaProperty::TEST_GREATER);
	pAlphaProp->SetSrcBlendMode( NiAlphaProperty::ALPHA_SRCCOLOR );
	pAlphaProp->SetDestBlendMode( NiAlphaProperty::ALPHA_INVSRCCOLOR );
	pAlphaProp->SetTestRef(255);

	return true;
}

unsigned int cUIRoundGauge::Update( unsigned long accumtime )
{
	/// 비활성화 상태이면 
	if( !mActivate )
		return eGaugeState_None;

	if( mStartTime == 0 || accumtime < mStartTime )
	{
		/// 진행을 계속 유지 할수 없다.
		mActivate	= false;
		mGaugeState = eGaugeState_None;
		return mGaugeState;
	}

	NiAlphaProperty* pAlphaProp = mpPlane->GetAlphaProp();
	unsigned char curRef;
	unsigned int state;

	/// 시작후 경과 시간
	unsigned long elapsedTime = accumtime - mStartTime + mElapsedTime;
	if( elapsedTime > mMaintainTime )
	{
		/// 종료
		curRef = 255;
		state = eGaugeState_End;
		DeActivate();
	}
	else
	{
		/// 진행
		curRef = (unsigned char)((255 * elapsedTime) / mMaintainTime);
		state = eGaugeState_Process;
		mGaugeState = state;
	}

	pAlphaProp->SetTestRef(curRef);
	return state;
}

void cUIRoundGauge::Draw()
{
	if( mpPlane )
	{
		mpPlane->Draw();
	}
}

void cUIRoundGauge::SetPos( short x, short y )
{
	if( mpPlane )
	{
		mpPlane->SetScreenXY( x, y );
	}
}

void cUIRoundGauge::UpdateGauge( unsigned long startTime, unsigned long elapsedTime, unsigned long maintainTime, bool active )
{
	mStartTime		= startTime;
	mElapsedTime	= elapsedTime;
	mMaintainTime	= maintainTime;
	mActivate		= active;
}

void cUIRoundGauge::SetElapsedTime( unsigned long elapsedTime )
{
	mElapsedTime = elapsedTime;
}

void cUIRoundGauge::Activate( unsigned long maintainTime )
{
	if( maintainTime == 0 )
		return;

	mActivate = true;
	mMaintainTime = maintainTime;
	mGaugeState = eGaugeState_Start;

	mStartTime = THEAPP->GetWorldAccumTime();
}

void cUIRoundGauge::DeActivate()
{
	mActivate = false;
	mStartTime = 0;
	mMaintainTime = 0;
	mElapsedTime = 0;
	mGaugeState = eGaugeState_None;
}

void cUIRoundGauge::operator=( const cUIRoundGauge& other )
{
	mActivate = other.mActivate;
	mGaugeState = other.mGaugeState;
	mStartTime = other.mStartTime;
	mMaintainTime = other.mMaintainTime;
	mElapsedTime = other.mElapsedTime;
}

unsigned long cUIRoundGauge::GetRestTime()
{
	if( mStartTime == 0 || mMaintainTime == 0 )
		return 0;

	long restTime = (long)(mMaintainTime - ( THEAPP->GetWorldAccumTime() - mStartTime + mElapsedTime ));
	return ( restTime > 0 )? restTime : 0;
}