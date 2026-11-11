#include "Stdafx.h"
#include "TextCtrl.h"

//#include "FontAgent.h"
#include "UIImage.h"

cTextCtrl::cTextCtrl( eUINodeType type )
: cUIWindow( type )
, mMaxTextWidth(0)
, mTotalMaxNum(0)
{
	mTextQueue.Reserve( 60 );
}

cTextCtrl::~cTextCtrl()
{
	Clear();
}

/// 주의사항 : 전체를 지운다
void cTextCtrl::Clear()
{
	sRowTextData* p = 0;
	for( unsigned int i = 0; i < mTextQueue.GetSize(); ++i )
	{
		p = (sRowTextData*)mTextQueue[i];
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				SAFE_DELETE( p->mColumes[b] );
			}
			p->mColumes.Clear();
			SAFE_DELETE(p);
		}
	}
	mTextQueue.Clear();
}
