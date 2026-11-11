#include "StdAfx.h"
#include "uifontitemkeeper.h"
#include "Application.h"

cUIFontItemKeeper::cUIFontItemPool cUIFontItemKeeper::mPool( 50 );

cUIFontItemKeeper::cUIFontItemKeeper()
{
}

cUIFontItemKeeper::~cUIFontItemKeeper()
{
	if( !mList.IsEmpty() )
	{
		cUIFontItemList::cIterator i = mList.Begin();
		cUIFontItemList::cIterator end = mList.End();

		sUIFontItem* pData;
		for( ; i != end; ++i )
		{
			pData = (sUIFontItem*)(*i);
			delete pData;
		}
		mList.Clear();
	}
}

void* cUIFontItemKeeper::operator new(size_t /*n*/)
{ 
	return mPool.Alloc();
} 

void cUIFontItemKeeper::operator delete(void* ptr)
{
	mPool.Free( (sUIFontItem*)ptr );
}

void cUIFontItemKeeper::AddFontItem( unsigned short fontIdx, const wchar_t* str, int posX, int posY, unsigned long color )
{
	sUIFontItem* pData = new sUIFontItem;
	if( !pData )
	{
		assert(0);
		return;
	}

	pData->mFontIdx = fontIdx;
	Sstrncpy( pData->mStr, 256, str, _tcslen(str) );

	pData->mStrLen = _tcslen(pData->mStr);
	pData->mPos.x = posX;
	pData->mPos.y = posY;
	pData->mColor = color;

	mList.PushBack( pData );
}

void cUIFontItemKeeper::DrawAll()
{
	if( mList.IsEmpty() )
		return;

	cUIFontItemList::cIterator i = mList.Begin();
	cUIFontItemList::cIterator end = mList.End();

	sUIFontItem* pData;
	for( ; i != end; ++i )
	{
		pData = (sUIFontItem*)(*i);
// 		FONTAGENT->DrawText( pData->mFontIdx, pData->mStr, pData->mStrLen, &pData->mPos, pData->mColor );

		delete pData;
	}

	mList.Clear();
}
