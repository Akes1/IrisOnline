#include "StdAfx.h"
#include "ArcGauge.h"

#include "PlaneObject.h"


cArcGauge::cArcGauge()
: mpPlane(0)
, mCurValue(0)
, mMaxValue(0)
, mStartAlpahRef(0)
{
	mMaxAlphaRef = 255;
}

cArcGauge::~cArcGauge()
{
	SAFE_DELETE(mpPlane);
}

/// 주의사항 : 아이콘 크기와 맞게 고정한다.
bool cArcGauge::Create( NiTexture* pTexture, unsigned short texLeft, unsigned short texTop, unsigned short texRight,unsigned short texBottom )
{
	/// 객체 생성
	mpPlane = new cPlaneObject;
	if( mpPlane->Create( pTexture, 0, 0, texRight - texLeft, texBottom - texTop, texLeft, texTop, texRight, texBottom ) == false )
		return false;

	/// 알파 옵션 조절
	NiAlphaProperty* pAlphaProp = mpPlane->GetAlphaProp();
	if( pAlphaProp )
	{
		pAlphaProp->SetAlphaTesting(true);
		pAlphaProp->SetTestMode(NiAlphaProperty::TEST_LESS );
		pAlphaProp->SetSrcBlendMode( NiAlphaProperty::ALPHA_ONE );
		pAlphaProp->SetDestBlendMode( NiAlphaProperty::ALPHA_ZERO );

	}

	return true;
}

void cArcGauge::CalculateAlphaRef()
{
	if( mMaxValue <= 0 )
		return;
	
	NiAlphaProperty* pAlphaProp = mpPlane->GetAlphaProp();
	if( pAlphaProp )
	{
		unsigned char curRef = (unsigned char)( (mMaxAlphaRef - mStartAlpahRef) * mCurValue/mMaxValue);
		pAlphaProp->SetTestRef(curRef);
	}
}

void cArcGauge::Draw()
{
	if( mpPlane )
		mpPlane->Draw();
}


void cArcGauge::SetPos( short x, short y )
{
	if( mpPlane )
		mpPlane->SetScreenXY( x, y );
}
