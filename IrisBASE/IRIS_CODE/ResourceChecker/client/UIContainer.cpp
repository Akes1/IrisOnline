#include "StdAfx.h"
#include "UIContainer.h"

//#include "InputSystem.h"
//#include "TabWindow.h"
//#include "RadioButton.h"

#include "UIManager.h"
#include "UIRootNode.h"

#include "UIEvent.h"

cUIContainerSkin::cUIContainerSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cUIContainerSkin::~cUIContainerSkin()
{
}


/////////////////////////////////////////////////////////////////////////////////////////////
cUIContainer::cUIContainer( eUINodeType type )
: cUINode( type ) 
{
	mOnlyBackground = false;
	mHoldView = false;
	mExceptSnap = false;
}

/// 주의사항 : 
cUIContainer::~cUIContainer()
{
	/// 자식들에게 알림
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);
		if( n )
		{
			n->mpParent = 0;
			delete n;
		}
		else
		{
			assert(0);
		}
	}
}
