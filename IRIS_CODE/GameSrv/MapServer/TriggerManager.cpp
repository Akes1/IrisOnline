#include "stdafx.h"
#include "gamesrv.h"
#include "TriggerManager.h"
#include "ObjectManager.h"
#include "AppTimer.h"

cTriggerManager* cTriggerManager::mpSingleton = 0;

/////
cTrigger::cTrigger()
: mQuestIndex(0)
, mLowLevel(0)
, mHighLevel(0)
, mStartHour(-1)
, mEndHour(-1)
, mDayCount(0)
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

/// 퀘스트 조건 체크
eCheckTriggerResult cTriggerManager::CheckTrigger( unsigned long playerIdx, unsigned long questIdx )
{
	eCheckTriggerResult ret = eCHECK_TRIGGER_SUCCESS;
	/// 발생조건이 없으면 그냥 true
	cTrigger* trigger = GetTrigger( questIdx );
	if( !trigger )
		return eCHECK_TRIGGER_SUCCESS;

	cPlayer* player = OBJECTMANAGER->GetPlayer( playerIdx );
	if( !player )
		return eCHECK_TRIGGER_ERROR;

	/// 레벨 범위 체크
	int level = player->GetLevel();
	if( trigger->mLowLevel > 0 && trigger->mLowLevel > level )
	{
		if( trigger->mLowLevel > 0 && trigger->mLowLevel - NEXT_LEVEL_QUEST_GAP > level )
			return eCHECK_TRIGGER_ERROR;
		else
			ret = eCHECK_TRIGGER_NEXT_LEVEL_QUEST;
	}

	if( trigger->mHighLevel > 0 && trigger->mHighLevel < level )
		return eCHECK_TRIGGER_ERROR;

	/// 종족 체크
	char race = player->GetRace();
	if( trigger->mRace[0] != -1 && (trigger->mRace[0] != race && trigger->mRace[1] != race)  )
		return eCHECK_TRIGGER_ERROR;

	/// 성별 체크
	if( trigger->mGender != -1 )
	{
		if( trigger->mGender != player->GetGender() )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 직업 체크
	unsigned long jobIdx = player->GetJob();
	if( !trigger->mJobSet.IsEmpty() && trigger->mJobSet.Find( jobIdx ) == trigger->mJobSet.End() )
		return eCHECK_TRIGGER_ERROR;

	/// 아이템 목록 체크
	sQuestItem* item = trigger->mHaveItem;
	for( unsigned int i = 0; i < QUEST_ITEM_MAX; i++, item++ )
	{
		int			  remainCount = item->count;
		TB_INVENTORY* inven = player->SelectInventory( INVENTORY_CHARACTER_BEGIN );

		/// 인벤토리 +착용정보 아이템 필요갯수 검사
		for( int i = INVENTORY_CHARACTER_BEGIN; i <= INVENTORY_CHARACTER_END; i++, inven++ )
		{
			if( player->IsInventory( inven ) == true )
			{
				if( (long)item->itemIndex == inven->itemDefineIndex )
				{
					/// 남은 체크 갯수 줄이기
					remainCount -= inven->count;

					if( remainCount <= 0 )
						break;
				}
			}
		}
		/// 갯수가 부족하면 조건 불충족
		if( remainCount > 0 )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 연계 퀘스트 체크
	typedef tArray<unsigned long> cLinkQuestArr;
	cLinkQuestArr& linkQuest = trigger->mLinkQuestArr;

	for( unsigned int i = 0; i < linkQuest.GetSize(); ++i )
	{
		/// 필요한 연계퀘스트가 모두 완료되었는지 검사
		unsigned long linkIdx = (unsigned long)linkQuest[i];
		if( player->IsCompleteQuest( linkIdx, false ) == false )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 몬스터 변신 검사
	if( trigger->mTransmonIndex > 0 )
	{
		if( trigger->mTransmonIndex != player->GetChgMonsterIdx() )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 타이틀 세팅 검사
	if( trigger->mTitleIndex > 0 )
	{
		if( trigger->mTitleIndex != player->GetTitleIndex() )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 요일검사
	if( trigger->mDayCount > 0 )
	{
		time_t ltime;
		time( &ltime );

		/// 오늘날짜로 변경하기
		tm datetime		 = *localtime( &ltime );
		datetime.tm_year = datetime.tm_year+1900;
		datetime.tm_mon  = datetime.tm_mon+1;

		/// 혹시 모르니까 갱신
		mktime( &datetime );

		/// 동일 요일이 아니면 리턴
		bool search = false;
		for( int i = 0; i < trigger->mDayCount; ++i )
		{
			if( trigger->mDayType[i] == datetime.tm_wday )
			{
				search = true;
				break;
			}
		}

		if( search == false )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 시간검사
	if( trigger->mStartHour > -1 && trigger->mEndHour > -1 )
	{
		TIMESTAMP_STRUCT now = TIMER->GetRealTime();

		if( !(now.hour >= trigger->mStartHour && now.hour < trigger->mEndHour) )
			return eCHECK_TRIGGER_ERROR;
	}

	/// 보조직업 검사 ( 숙련도 최소, 최대 검사 )
	if( trigger->mMakeSkillIndex != 0 )
	{
		sHeroInfo* playerInfo = player->GetHeroInfo();

		if( playerInfo->MakeSkill1.mIndex == trigger->mMakeSkillIndex )
		{
			if( !(playerInfo->MakeSkill1.mExp >= trigger->mMakeSkillExp && 
				  playerInfo->MakeSkill1.mExp <= trigger->mMaxMakeSkillExp) )
				return eCHECK_TRIGGER_ERROR;
		}
		else if( playerInfo->MakeSkill2.mIndex == trigger->mMakeSkillIndex )
		{
			if( !(playerInfo->MakeSkill2.mExp >= trigger->mMakeSkillExp && 
				  playerInfo->MakeSkill2.mExp <= trigger->mMaxMakeSkillExp) )
				return eCHECK_TRIGGER_ERROR;
		}
		else
			return eCHECK_TRIGGER_ERROR;
	}

	return ret;
}

/// 트리거중 퀘스트 진행시 지켜져야할 의무 조건 체크
bool cTriggerManager::CheckDuty( unsigned long heroIdx, unsigned long otherIdx, unsigned long questIdx )
{
	cTrigger* trigger = GetTrigger( questIdx );
	if( !trigger )
		return true;

	cPlayer* hero = OBJECTMANAGER->GetPlayer( heroIdx );
	if( !hero )
	{
		assert(0);
		return false;
	}

	cPlayer* other = OBJECTMANAGER->GetPlayer( otherIdx );
	if( !other )
	{
		assert(0);
		return false;
	}

	/// 몬스터 변신상태 체크
	if( trigger->mTransmonIndex > 0 )
	{
		if( !(trigger->mTransmonIndex == hero->GetChgMonsterIdx() &&
			trigger->mTransmonIndex == other->GetChgMonsterIdx()) )
			return false;
	}

	/// 타이틀 세팅 체크
	if( trigger->mTitleIndex > 0 )
	{
		if( !(trigger->mTitleIndex == hero->GetTitleIndex() &&
			trigger->mTitleIndex == other->GetTitleIndex()) )
			return false;
	}

	return true;
}

TIMESTAMP_STRUCT cTriggerManager::GetValidTime( unsigned long questIdx )
{
	TIMESTAMP_STRUCT result;
	::memset( &result, 0, sizeof(result) );

	cTrigger* trigger = GetTrigger( questIdx );
	if( trigger )
	{
		time_t curTime;
		time_t validTime;

		time( &curTime );

		tm valid;
		::memset( &valid, 0, sizeof(valid) );
		valid = *localtime( &curTime );
		valid.tm_hour = QUEST_INITHOUR;
		valid.tm_min  = 0;
		valid.tm_sec  = 0;

		/// 하루단위
		validTime = mktime( &valid );
		if( difftime( validTime, curTime ) >= 0 )
		{
			result = TIMER->ConvertTMtoTS( valid );
		}
		else
		{
			validTime += (60*60*24);
			tm newValid = *localtime( &validTime );

			result = TIMER->ConvertTMtoTS( newValid );
		}
	}
	return result;
}