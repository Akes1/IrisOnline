#include "StdAfx.h"
#include "UINode.h"

#include "UINodeData.h"
#include "UINodeProperty.h"
#include "UIEvent.h"

cUINode::cUINode( eUINodeType type )
: mType( type )
, mpParent( 0 )
, mID( ULONG_MAX )
, mFrontID( ULONG_MAX )
, mpImage( 0 )
, mVisible( true )
, mEnabled( true )
, mPressed( false )
, mMove( false )
, mAlpha( 1.0f )
, mNetLock( false )
, mLockTry( -1 )
, mBackupVisible( true )
{
	mSnapType = eSNAP_NONE;

	mRelativeRect.Set(0, 0, 0, 0);
	mAbsoluteRect.Set(0, 0, 0, 0);

	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();

	mDefaultColor = COLOR_DEFAULT;

	/// 스크립트의 좌표는 해당 사이즈를 기준으로 한다
	mDefaultWidth = DEFAULT_SCREENWIDTH;
	mDefaultHeight = DEFAULT_SCREENHEIGHT;

	mExceptionEvent = false;
	mLClickNotifyStage = false;
	mProcessEnable = false;

	mUseBoldText = false;

	mHoverTipIndex = 0;
}

cUINode::~cUINode()
{
	Destroy();
}
