#include "StdAfx.h"
#include "Icon.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"
#include "UIContainer.h"
#include "Item_Common.h"

cIcon::cIcon( eUINodeType type )
: cBaseIcon( type )
, mImageFont( 0 )
, mNumber( 0 )
, mShowGauge( true )
, mShowNumber( true )
, mpEdgeImage(0)
, mpCountImage(0)
, mpFlashImage(0)
, mAniEnable( true )
, mpTipImage(0)
{
	mTipType = eTOOLTIP_NONE;
	mProcessEnable = true;
}

cIcon::~cIcon()
{
//	SAFE_DELETE( mpFlashImage );
	SAFE_DELETE( mpTipImage );
//	SAFE_DELETE( mpCountImage );
//	SAFE_DELETE( mpEdgeImage );
}