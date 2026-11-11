#include "Stdafx.h"
#include "UIIMENode.h"
#include "Application.h"

HWND cUIIMENode::mHWnd = NULL;
bool cUIIMENode::mShiftKey = false;
bool cUIIMENode::mControlKey = false;

cUIIMENode::cUIIMENode( eUINodeType type )
: cUINode( type )
, mTextLength( 0 )
, mTextCaretX( 0 )
, mScrollIndex( 0 )
, mScrollCaretX( 0 )
, mOneLineWidth( 0 )
, mStartDragCaretX( 0 )
, mEndDragCaretX( 0 )
, mMaxEditLength(0)
, mFixFlag( false )
, mCompFlag( false )
, mDraggingFlag( false )
, mChangeText( false )
, mImeFlag( false )
{
	mEditKind = eEDIT_NORMAL;

	::ZeroMemory( mText, sizeof(mText) );
	::ZeroMemory( mCompText, sizeof(mCompText) );
}

cUIIMENode::~cUIIMENode()
{

}
