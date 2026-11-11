#include "stdafx.h"
#include "QuestManager.h"

#include "UIFuncText.h"
#include "Protocol.h"
#include "Application.h"

int SortQuest( const void *arg1, const void *arg2 )
{
	sQuestList* quest1 = (sQuestList*)arg1;
	sQuestList* quest2 = (sQuestList*)arg2;

	if( quest1 && quest2 )
	{
		if( quest1->questIdx > quest2->questIdx )
		{
			return 1;
		}
		else if( quest1->questIdx < quest2->questIdx )
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	
	assert(0);
	return 0;
}

int BoardQuestSort( unsigned long arg1, unsigned long arg2 )
{
	cQuestDefine* define1 = QUESTMAN->GetQuestDefine( arg1 );
	cQuestDefine* define2 = QUESTMAN->GetQuestDefine( arg2 );
	if( define1 && define2 )
	{
		if( define1->mRepeatType < define2->mRepeatType )
		{
			return 1;
		}
		else if( define1->mRepeatType < define2->mRepeatType )
		{
			return -1;
		}
		else
		{
			return ( arg1 < arg2 ) ? 1 : 0;
		}
	}
	assert(0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
cQuestManager* cQuestManager::mpSingleton = 0;

cQuestManager::cQuestManager()
: mpQuestText(0)
, mTotalKeepCount(0)
, mCompleteCount(0)
, mProgressCount(0)
, mCheckQuestIndex(ULONG_MAX)
, mTimeMap( 64 )
, mNetLock( false )
, mLockTry( eLOCK_QUESTNONE )
, mTempQuestIndex(0)
, mTempNpcIndex(0)
, mTempArrIndex(0)
, mChaseQuestIndex(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	memset( mQuest, 0, sizeof(sQuestList) * MAX_KEEPQUEST );
}

cQuestManager::~cQuestManager()
{
	SAFE_DELETE( mpQuestText );

	/// 
	cQuestDefineMap::cIterator i = mQuestDefine.Begin();
	cQuestDefineMap::cIterator end = mQuestDefine.End();

	for( ; i != end; ++i )
	{
		cQuestDefine* p = (cQuestDefine*)(i->mSecond);

		SAFE_DELETE( p );
	}
	mQuestDefine.Clear();

	/// npc 보유 퀘스트 맵 삭제
	{
		cNpcQuestMap::cIterator i = mNpcQuestMap.Begin();
		cNpcQuestMap::cIterator end = mNpcQuestMap.End();

		for( ; i != end; ++i )
		{
			sNpcQuestList* p = (sNpcQuestList*)(i->mSecond);
			if( p )
				SAFE_DELETE( p );
		}
		mNpcQuestMap.Clear();
	}

	/// 시간맵 클리어
	//ClearQuestTime();
}

///// 시간맵 클리어
//void cQuestManager::ClearQuestTime()
//{
//	cTimeMap::cIterator b = mTimeMap.Begin();
//	cTimeMap::cIterator end = mTimeMap.End();
//
//	for( ; b != end; ++b )
//	{
//		sQuestTime* data = (sQuestTime*)(b->mSecond);
//		SAFE_DELETE( data );
//	}
//	mTimeMap.Clear();
//}
//
//void cQuestManager::Close()
//{
//	/// 맵체인지시
//	memset( mQuest, 0, sizeof(mQuest) );
//	mTotalKeepCount = 0;
//	mCompleteCount = 0;
//	mProgressCount = 0;
//
//	mNetLock = false;
//	mLockTry = eLOCK_QUESTNONE;
//
//	/// 시간맵 클리어
//	{
//		cTimeMap::cIterator b = mTimeMap.Begin();
//		cTimeMap::cIterator end = mTimeMap.End();
//
//		for( ; b != end; ++b )
//		{
//			sQuestTime* data = (sQuestTime*)(b->mSecond);
//			SAFE_DELETE( data );
//		}
//		mTimeMap.Clear();
//	}
//
//	/// npc 상태 클리어
//	{
//		cNpcStatusMap::cIterator b = mNpcStatusMap.Begin();
//		cNpcStatusMap::cIterator end = mNpcStatusMap.End();
//		for( ; b != end; ++b )
//		{
//			sNpcStausData* data = (sNpcStausData*)(b->mSecond);
//			SAFE_DELETE( data );
//		}
//		mNpcStatusMap.Clear();
//	}
//	
//	///
//	mCountMap.Clear();
//	mNewQuestSet.Clear();
//	mNewEventQuestSet.Clear();
//}

bool cQuestManager::Init()
{
	mpQuestText = new cUIFuncText;

	/// 퀘스트 설명 문자열 로드
	cString path;
	path.Format("./language/%s/Quest_Note.txt", cApplication::mLangaugeFolder.Cstr() );
	if( mpQuestText->Load( path.Cstr() ) == false )
	{
		return false;
	}

	/// 퀘스트 데이터 로드
	if( Load( "./Script/Resource/List_Quest.txt"  ) == false )
	{
		assert( 0 && "failed to load quest script" );
		return false;
	}

	/// NPC 퀘스트 리스트 로드
	if( LoadNpcQuest( "./Script/Resource/NpcQuestList.txt" ) == false )
	{
		assert( 0 && "failed to load npc quest list" );
		return false;
	}

	return true;
}
