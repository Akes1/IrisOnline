#include "StdAfx.h"
#include "UIRootNode.h"
#include "UIManager.h"

/// 주의사항 : 
cUIRootNode::cUIRootNode( eUINodeType type )
: cUIContainer( type )
, mFontItemKeeper( 0 )
, mTipWindow( 0 )
{
	mpParent = 0;
	mID = 0;
	mpImage = 0;

	mRelativeRect.Set( 0, 0, GetScreenWidth(), GetScreenHeight() );
	mAbsoluteRect.Set( 0, 0, GetScreenWidth(), GetScreenHeight() );

	mVisible = true;
	mEnabled = true;

	mFontItemKeeper = new cUIFontItemKeeper;
}

/// 주의사항 : 
cUIRootNode::~cUIRootNode()
{
	SAFE_DELETE(mFontItemKeeper);

	mProcessList.Clear();
	mChildVisArray.Clear();
	mFindChildMap.Clear();
}