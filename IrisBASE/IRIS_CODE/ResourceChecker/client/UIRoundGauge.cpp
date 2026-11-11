#include "StdAfx.h"
#include "uiroundgauge.h"
#include "UINode.h"

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
//	SAFE_DELETE(mpPlane);
}
