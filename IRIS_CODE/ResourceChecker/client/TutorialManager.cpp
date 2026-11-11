#include "stdafx.h"
#include "TutorialManager.h"
#include "Protocol.h"
#include "UIMsgboxEvent.h"

cTutorialMode::cTutorialMode()
: mModeIndex(UINT_MAX)
, mStartPosIndex(0)
, mDieTextIndex(0)
{

}

cTutorialMode::~cTutorialMode()
{
	//
	cTutorialObjectImageMap::cIterator ob = mObjectImageMap.Begin();
	cTutorialObjectImageMap::cIterator oend = mObjectImageMap.End();
	for( ; ob != oend; ++ob )
	{
		sTutorialBaseImage* p = (sTutorialBaseImage*)(ob->mSecond);
		SAFE_DELETE( p );
	}
	mObjectImageMap.Clear();

	//
	for( unsigned int i = 0; i < mObjectArr.GetSize(); ++i )
	{
		sTutorialObject* p = (sTutorialObject*)mObjectArr[i];
		SAFE_DELETE( p );
	}
	mObjectArr.Clear();

	//
	for( unsigned int i = 0; i < mImageArr.GetSize(); ++i )
	{
		sTutorialImage* p = (sTutorialImage*)mImageArr[i];
		SAFE_DELETE( p );
	}
	mImageArr.Clear();
}

///////////////////////////////////////////////////////////
cTutorialManager* cTutorialManager::mpSingleton = NULL;

cTutorialManager::cTutorialManager()
: mModeIndex(-1)
{
	mpSingleton = this;
}

cTutorialManager::~cTutorialManager()
{
	cModeMap::cIterator i = mModeMap.Begin();
	cModeMap::cIterator end = mModeMap.End();

	for( ; i != end; ++i )
	{
		cTutorialMode* define = (cTutorialMode*)(i->mSecond);
		SAFE_DELETE( define );
	}
	mModeMap.Clear();

	//
	mTextureMap.Clear();

	//
	mpSingleton = 0;
}

bool cTutorialManager::Init()
{
	if( LoadScript( "./Script/Resource/tutorialmode.txt"  ) == false )
	{
		assert( 0 && "failed to read tutorial script" );
		cString msg;
		msg.Format( "[./Script/Resource/tutorialmode.txt]" );
		MessageBoxA( NULL, msg.Cstr(), "failed to read tutorial script", MB_OK | MB_ICONERROR );
		return false;
	}
	return true;
}
