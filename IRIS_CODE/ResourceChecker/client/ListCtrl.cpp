#include "Stdafx.h"
#include "ListCtrl.h"
#include "UIImage.h"

cListCtrl::cListCtrl( eUINodeType type )
: cUIWindow( type )
{
}

cListCtrl::~cListCtrl()
{
	Clear();
}

/// 주의사항 : 전체를 지운다
void cListCtrl::Clear()
{
	cRowDataArray::cIterator i = mRows.Begin();
	cRowDataArray::cIterator end = mRows.End();

	sRowData* p = 0;
	for( ; i != end; ++i )
	{
		p = (sRowData*)(*i);
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				sData* data = (sData*)p->mColumes[b];
				SAFE_DELETE( data );
			}
			p->mColumes.Clear();

			p->mExtraData = 0;
			SAFE_DELETE(p);
		}
	}
	mRows.Clear();
}
