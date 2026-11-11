#include "stdafx.h"
#include "MakeStage.h"

cMakeStage::cMakeStage()
: mpMakeUIManager(0)
, mpMakeCharacter(0)
, mCardDramaObj(0)
, mViewState(0)
, mCurrentSelectIndex(0)
, mUpdateDisplayObj(false)
{
}

cMakeStage::~cMakeStage()
{
//	SAFE_DELETE(mpMakeCharacter);

	for( unsigned int i=0; i< mViewItemArray.GetSize(); i++ )
	{
		sViewItem* p = (sViewItem*)mViewItemArray[i];
		SAFE_DELETE(p);
	}
	mViewItemArray.Clear();
}

void cMakeStage::Init( cStageManager* pStageManager )
{
	mpStageManager = pStageManager;
}

void cMakeStage::InitStage()
{
	if( mInit )	return;

	/// UI
// 	mpMakeUIManager = new cMakeUIManager;
// 	mpMakeUIManager->Init();

	mInit = true;

	if( LoadDisplayItemInfo() == false )
	{
		assert(0);
	}
}

bool cMakeStage::LoadDisplayItemInfo()
{
	/// 파일 열기
	cFileLoader loader;
	cString pathName = "./Script/Resource/MakeStageViewItem.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load MakeStageViewItem.txt" );
		cString msg;
		msg.Format("[%s]", pathName.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "fail to open file", MB_OK | MB_ICONERROR );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;
	sViewItem* pViewItem = 0;

	while( tokenizer.IsEnd() == false )
	{
		pViewItem = new sViewItem;

		/// 직업 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto error; }
		pViewItem->mJobIdx = str.ToInt();

		// Item 번호
		if( tokenizer.GetNext( &str ) == false )	{ goto error; }
		pViewItem->mItemIdx = str.ToInt();

		/// 인벤번호
		if( tokenizer.GetNext( &str ) == false )	{ goto error; }
		pViewItem->mInvenNum = (unsigned short)str.ToInt();

		mViewItemArray.PushBack( pViewItem );
		pViewItem = 0;
	}
	return true;

error:
	SAFE_DELETE(pViewItem);
	cString msg;
	msg.Format("[%s] [Line:%d]", pathName.Cstr(), tokenizer.mLine );
	MessageBoxA( NULL, msg.Cstr(), "fail to parse", MB_OK | MB_ICONERROR );
	return false;
}
