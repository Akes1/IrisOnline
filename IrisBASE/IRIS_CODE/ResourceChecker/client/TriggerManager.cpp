#include "stdafx.h"
#include "TriggerManager.h"
//#include "ObjectManager.h"
//#include "Hero.h"

cTriggerManager* cTriggerManager::mpSingleton = 0;

/////
cTrigger::cTrigger()
: mQuestIndex(0)
, mLowLevel(0)
, mHighLevel(0)
, mDayCount(0)
, mStartHour(-1)
, mEndHour(-1)
, mTransmonIndex(0)
, mTitleIndex(0)
, mGender(-1)
, mMakeSkillIndex(0)
, mMakeSkillExp(0)
, mMaxMakeSkillExp(0)
{
	mRace[0] = -1;
	mRace[1] = -1;

	for( unsigned int i = 0; i < QUEST_ITEM_MAX; ++i )
	{
		mHaveItem[i].itemIndex = 0;
		mHaveItem[i].count = 0;
	}

	for( unsigned int i = 0; i < eDAY_MAX; ++i )
	{
		mDayType[i] = eDAY_MAX;
	}
}

////////////////
cTriggerManager::cTriggerManager()
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;
}

cTriggerManager::~cTriggerManager()
{
	Clear();
}

bool cTriggerManager::Init()
{
	Clear();
	return true;
}

void cTriggerManager::Clear()
{
	cTriggerMap::cIterator i = mTriggerMap.Begin();
	cTriggerMap::cIterator end = mTriggerMap.End();

	for( ; i != end; ++i )
	{
		cTrigger* trigger = (cTrigger*)(i->mSecond);
		SAFE_DELETE( trigger );
	}
	mTriggerMap.Clear();
}

/// 트리거 삽입
bool cTriggerManager::AddTrigger( unsigned long questIdx, cTrigger* trigger )
{
	if( !trigger )
		return false;

	if( mTriggerMap.Insert( questIdx, trigger ) == false )
	{
		assert( 0 && "failed to add trigger" );
		return false;
	}
	return true;
}

/// 트리거삭제
void cTriggerManager::DeleteTrigger( unsigned long questIdx )
{
	cTrigger* trigger = (cTrigger*)mTriggerMap.GetAt( questIdx );
	if( trigger )
	{
		mTriggerMap.Erase( questIdx );
		SAFE_DELETE( trigger );
	}
}

cTrigger* cTriggerManager::GetTrigger( unsigned long questIdx )
{
	return (cTrigger*)mTriggerMap.GetAt( questIdx );
}
