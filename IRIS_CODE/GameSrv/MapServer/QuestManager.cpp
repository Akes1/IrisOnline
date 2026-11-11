#include "stdafx.h"
#include "gamesrv.h"
#include "QuestManager.h"
#include "Player.h"
#include "SkillManager.h"
#include "Npc.h"
#include "ObjectManager.h"

cQuestManager* cQuestManager::mpSingleton = 0;

cQuestManager::cQuestManager()
: mAccumTime(0)
, mElapsedTime(0)
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;
}

cQuestManager::~cQuestManager()
{
	Clear();
}

void cQuestManager::Clear()
{
	/// 몬스터 소환정보 삭제
	{
		typedef tPointerHashMap<unsigned long, void*> cQuestMonInfoMap;
		cQuestMonsterMap::cIterator i = mQuestMonsterMap.Begin();
		cQuestMonsterMap::cIterator end = mQuestMonsterMap.End();

		for( ; i != end; ++i )
		{
			sQuestMonsterInfo* questInfo = (sQuestMonsterInfo*)(i->mSecond);
			if( questInfo )
			{
				cQuestMonInfoMap& infoMap = questInfo->mQuestMonInfoMap;
				cQuestMonInfoMap::cIterator qi = infoMap.Begin();
				cQuestMonInfoMap::cIterator qend = infoMap.End();
				for( ; qi != qend; ++qi )
				{
					sQuestMonster* monInfo = (sQuestMonster*)(qi->mSecond);
					if( monInfo )
					{
						monInfo->mMonsterArr.Clear();
						SAFE_DELETE( monInfo );
					}
				}

				infoMap.Clear();
				SAFE_DELETE( questInfo );
			}
		}
		mQuestMonsterMap.Clear();
	}

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

	/// 퀘스트 맵 삭제
	{
		cQuestDefineMap::cIterator i = mQuestDefine.Begin();
		cQuestDefineMap::cIterator end = mQuestDefine.End();

		for( ; i != end; ++i )
		{
			cQuestDefine* p = (cQuestDefine*)(i->mSecond);
			if( p )
			{
				SAFE_DELETE( p->mQuestDuty );
				SAFE_DELETE( p->mDefaultReward );
				SAFE_DELETE( p->mSelectReward );
				SAFE_DELETE( p->mQuestLimit );
				SAFE_DELETE( p->mQuestFail );

				SAFE_DELETE( p );
			}
		}
		mQuestDefine.Clear();
	}
}

bool cQuestManager::Init()
{
	Clear();

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
	
	/// 다음 정각 까지의 차이 구하기
	time_t curTime;
	time_t validTime;

	time( &curTime );

	tm valid;
	::memset( &valid, 0, sizeof(valid) );

	localtime_s( &valid, &curTime );

	valid.tm_hour++;
	valid.tm_min = 0;
	valid.tm_sec = 0;

	validTime = mktime( &valid );
	long diff = (long)difftime( validTime, curTime );

	mAccumTime = diff * 1000;
	return true;
}

void cQuestManager::Process( long, long accumTime )
{
	/// 시간 이벤트 발생
	if( accumTime - mAccumTime > mElapsedTime )
	{
		OBJECTMANAGER->SendTimeEventQuest();

		mAccumTime = accumTime;
		mElapsedTime = QUEST_TIMEEVENT;
	}
}

cQuestDefine* cQuestManager::GetQuestDefine( unsigned long questIdx )
{
	return (cQuestDefine*)mQuestDefine.GetAt( questIdx );
}

cQuestReward* cQuestManager::GetDefaultReward( unsigned long questIdx )
{
	cQuestDefine* define = GetQuestDefine( questIdx );
	return ( define == 0 )? 0 : define->mDefaultReward;
}

/// 완료가능한 퀘스트인지 검사
bool cQuestManager::IsPossibleComplete( unsigned long playerIdx, int arrIdx )
{
	cPlayer* player = OBJECTMANAGER->GetPlayer( playerIdx );
	if( !player )
	{
		assert(0);
		return false;
	}

	TB_QUEST_PROGRESS* quest = player->GetQuest( arrIdx );
	if( !quest )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsPossibleComplete() - failed to search have quest", playerIdx, arrIdx );
		return false;
	}
	
	cQuestDefine* questDefine = GetQuestDefine( quest->questIdx );
	if( !questDefine )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsPossibleComplete() - failed to search questIdx", quest->questIdx );
		assert(0);
		return false;
	}

	/// 의무가 없는 퀘스트는 없다
	cQuestDuty* questDuty = questDefine->mQuestDuty;
	if( !questDuty )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsPossibleComplete() - not exist quest duty", quest->questIdx );
		assert(0);
		return false;
	}
	
	if( questDefine->mComplete == 0 )
		return false;

    /// 완료조건이 동일하면 완료
	if( quest->check == questDefine->mComplete )
		return true;
	return false;
}

/// 선택보상에 있는 아이템인지 검사
unsigned int cQuestManager::IsSelectRewardItem( unsigned long questIdx, unsigned long ItemIdx )
{
	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsSelectRewardItem() - failed to search questIdx", questIdx );
		assert(0);
		return 0;
	}

	cQuestReward* select = define->mSelectReward;
	if( !select )
		return 0;

	for( unsigned int i = 0; i < QUEST_ITEM_MAX; ++i )
	{
		if( select->mRewardItem[i].itemIndex > 0 && ItemIdx == select->mRewardItem[i].itemIndex )
			return select->mRewardItem[i].count;
	}

	return 0;
}

bool cQuestManager::IsSelectRewardBuff( unsigned long questIdx, unsigned long buffIdx )
{
	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsSelectRewardBuff() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	cQuestReward* select = define->mSelectReward;
	if( !select )
		return false;

	for( unsigned int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		if( select->mRewardBuff[i] > 0 && buffIdx == select->mRewardBuff[i] )
			return true;
	}

	return true;
}

bool cQuestManager::IsHaveQuest( unsigned long questIdx, unsigned long npcClassIdx )
{
	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
	{
		NETWORK2->PostServerEvent( "Error(%d): QUEST IsHaveQuest() - failed to search questIdx", questIdx );
		assert(0);
		return false;
	}

	sNpcQuestList* list = GetNpcQuestMap( npcClassIdx );
	if( !list )
		return false;

	/// 해당 퀘스트 소유했는지 검사
	typedef tHashSet<unsigned long> cQuestSet;
	cQuestSet::cIterator i = list->mQuestSet.Find( questIdx );
	if( i != list->mQuestSet.End() )
		return true;
	return false;
}

sNpcQuestList* cQuestManager::GetNpcQuestMap( unsigned long npcClassIdx )
{
	if( npcClassIdx == 0 )
		return 0;

	return (sNpcQuestList*)mNpcQuestMap.GetAt( npcClassIdx );
}

bool cQuestManager::AddQuestMonster( unsigned long playerIdx, unsigned long questIdx, unsigned long monsterIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cQuestMonInfoMap;

	sQuestMonsterInfo* questInfo = (sQuestMonsterInfo*)mQuestMonsterMap.GetAt( playerIdx );
	if( questInfo == NULL )
	{
		questInfo = new sQuestMonsterInfo;

		if( mQuestMonsterMap.Insert( playerIdx, questInfo ) == false )
		{
			NETWORK2->PostServerEvent( "Error(playerIdx : %d): QUEST AddQuestMonster() - failed to mQuestMonsterMap insert", playerIdx );
			SAFE_DELETE( questInfo );
			return false;
		}
	}

	cQuestMonInfoMap& infoMap = questInfo->mQuestMonInfoMap;
	sQuestMonster* monsterInfo = (sQuestMonster*)infoMap.GetAt( questIdx );
	if( monsterInfo == NULL )
	{
		monsterInfo = new sQuestMonster;
		if( infoMap.Insert( questIdx, monsterInfo ) == false )
		{
			NETWORK2->PostServerEvent( "Error(questIdx : %d): QUEST AddQuestMonster() - failed to questMap insert", questIdx );
			SAFE_DELETE( monsterInfo );
			return false;
		}
	}

	monsterInfo->mMonsterArr.PushBack( monsterIdx );
	return true;
}

/// 해당 퀘랑 관련된 몬스터만 삭제
void cQuestManager::DeleteQuestMonster( unsigned long playerIdx, unsigned long questIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cQuestMonInfoMap;

	sQuestMonsterInfo* questInfo = (sQuestMonsterInfo*)mQuestMonsterMap.GetAt( playerIdx );
	if( questInfo )
	{
		cQuestMonInfoMap& infoMap = questInfo->mQuestMonInfoMap;
		sQuestMonster* monInfo = (sQuestMonster*)infoMap.GetAt( questIdx );
		if( monInfo )
		{
			for( unsigned int i = 0; i < monInfo->mMonsterArr.GetSize(); ++i )
			{
				unsigned long monsterIdx = monInfo->mMonsterArr[i];
				OBJECTMANAGER->QuestRemoveMonster( monsterIdx );
			}

			monInfo->mMonsterArr.Clear();
			SAFE_DELETE( monInfo );
		}

		infoMap.Erase( questIdx );
	}
}

/// 해당 유저가 소환한 모든 몬스터 삭제 - 게임종료시
void cQuestManager::DeleteAllQuestMonster( unsigned long playerIdx )
{
	typedef tPointerHashMap<unsigned long, void*> cQuestMonInfoMap;

	sQuestMonsterInfo* questInfo = (sQuestMonsterInfo*)mQuestMonsterMap.GetAt( playerIdx );
	if( questInfo )
	{
		cQuestMonInfoMap& infoMap = questInfo->mQuestMonInfoMap;
		cQuestMonInfoMap::cIterator qi = infoMap.Begin();
		cQuestMonInfoMap::cIterator qend = infoMap.End();
		for( ; qi != qend; ++qi )
		{
			sQuestMonster* monInfo = (sQuestMonster*)(qi->mSecond);
			if( monInfo )
			{
				for( unsigned int i = 0; i < monInfo->mMonsterArr.GetSize(); ++i )
				{
					unsigned long monsterIdx = monInfo->mMonsterArr[i];
					OBJECTMANAGER->QuestRemoveMonster( monsterIdx );
				}

				monInfo->mMonsterArr.Clear();
				SAFE_DELETE( monInfo );
			}
		}

		infoMap.Clear();
		SAFE_DELETE( questInfo );
	}

	mQuestMonsterMap.Erase( playerIdx );
}

