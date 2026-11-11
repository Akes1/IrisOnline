#include "stdafx.h"
#include "QuestManager.h"

#include "UIFuncText.h"
#include "Protocol.h"
#include "ChatManager.h"
#include "GameUIManager.h"
#include "QuestWindow.h"
#include "UIMsgboxEvent.h"
#include "NpcWindow.h"
#include "NpcRewardWindow.h"
#include "QuestSimpleWindow.h"
#include "ItemManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Npc.h"
#include "Application.h"
#include "StageManager.h"
#include "SoundSystem.h"
#include "GameResourceManager.h"
#include "InputSystem.h"
#include "SimpleTipWindow.h"
#include "SmallMiniMapWindow.h"
#include "MiniMapWindow.h"
#include "NpcScript.h"
#include "BoardWindow.h"
#include "NpcTutorialWindow.h"
#include "TriggerManager.h"
#include "OptionManager.h"
#include "TwitterWindow.h"
#include "TwitterScript.h"

/// 배열용 퀵소트 정렬 함수
int SortQuest( const void *arg1, const void *arg2 )
{
	sQuestList* quest1 = (sQuestList*)arg1;
	sQuestList* quest2 = (sQuestList*)arg2;

	if( quest1 && quest2 )
	{

		cQuestDefine* define1 = QUESTMAN->GetQuestDefine( quest1->questIdx );
		cQuestDefine* define2 = QUESTMAN->GetQuestDefine( quest2->questIdx );

		if( define1 && define2 )
		{
			if( define1->mLevel < define2->mLevel )
				return 1;
			else if( define1->mLevel > define2->mLevel )
				return -1;
			else
				return 0;
		}
	}

	assert(0);
	return 0;
}

/// 리스트용 정렬 함수
bool QuestLevelSort( unsigned long arg1, unsigned long arg2 )
{
	cQuestDefine* define1 = QUESTMAN->GetQuestDefine( arg1 );
	cQuestDefine* define2 = QUESTMAN->GetQuestDefine( arg2 );

	if( define1 && define2 )
	{
		if( define1->mLevel > define2->mLevel )
			return true;
		else
			return false;
	}
	assert(0);
	return false;
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
	ClearQuestTime();
}

/// 시간맵 클리어
void cQuestManager::ClearQuestTime()
{
	cTimeMap::cIterator b = mTimeMap.Begin();
	cTimeMap::cIterator end = mTimeMap.End();

	for( ; b != end; ++b )
	{
		sQuestTime* data = (sQuestTime*)(b->mSecond);
		SAFE_DELETE( data );
	}
	mTimeMap.Clear();
}

void cQuestManager::Close()
{
	/// 맵체인지시
	memset( mQuest, 0, sizeof(mQuest) );
	mTotalKeepCount = 0;
	mCompleteCount = 0;
	mProgressCount = 0;

	mNetLock = false;
	mLockTry = eLOCK_QUESTNONE;

	/// 시간맵 클리어
	{
		cTimeMap::cIterator b = mTimeMap.Begin();
		cTimeMap::cIterator end = mTimeMap.End();

		for( ; b != end; ++b )
		{
			sQuestTime* data = (sQuestTime*)(b->mSecond);
			SAFE_DELETE( data );
		}
		mTimeMap.Clear();
	}

	/// npc 상태 클리어
	{
		cNpcStatusMap::cIterator b = mNpcStatusMap.Begin();
		cNpcStatusMap::cIterator end = mNpcStatusMap.End();
		for( ; b != end; ++b )
		{
			sNpcStausData* data = (sNpcStausData*)(b->mSecond);
			SAFE_DELETE( data );
		}
		mNpcStatusMap.Clear();
	}
	
	///
	mCountMap.Clear();
	mNewQuestSet.Clear();
	mNewEventQuestSet.Clear();
}

bool cQuestManager::Init()
{
	mpQuestText = new cUIFuncText;

	/// 퀘스트 설명 문자열 로드
	cStringT path;
	path.Format( L"./language/%s/Quest_Note.txt", cApplication::mLangaugeFolderW.Cstr() );
	if( mpQuestText->Load( path.Cstr() ) == false )
	{
		assert( 0 && "failed to load quest description" );
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

cUIFuncTextNode* cQuestManager::GetNpcTextNode( unsigned int textid )
{
	return mpQuestText->GetNode( textid );
}

sNpcQuestList* cQuestManager::GetNpcQuestMap( unsigned long npcClassIdx )
{
	if( npcClassIdx == 0 )
		return 0;

	return (sNpcQuestList*)mNpcQuestMap.GetAt( npcClassIdx );
}

void cQuestManager::OpenLoad()
{
	/// 선택해논 정보 삭제
	mChaseQuestIndex = 0;
	mSelectSet.Clear();

	UpdateUIAll();

	GAMEUI->ApplyWinodwOption_CharacterQuest();
}

void cQuestManager::OpenMapChange()
{
	mSelectSet.Clear();

	UpdateUIAll();
	GAMEUI->ApplyWinodwOption_CharacterQuest();
	//
	UpdateChaseQuest( mChaseQuestIndex );
}

/// 퀘스트 윈도우만 갱신 - quest add
void cQuestManager::UpdateUI()
{
	cQuestWindow* window = GAMEUI->GetQuestWindow();
	if( !window )
	{
		assert( 0 && "error quest ui" );
		return;
	}

	/// UI 에 정보 세팅
	window->Clear();

	bool newQuest = false;
	///
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( mQuest[i].questIdx );
		if( define )
		{
			/// 마지막 위치의 퀘스트가 새로 추가한 퀘스트
			if( i == mTotalKeepCount -1 )
				newQuest = true;
			else
				newQuest = false;

			window->AddQuest( i, mQuest[i].questIdx, (mQuest[i].check == define->mComplete), newQuest );
		}
	}

	/// 갯수 갱신
	CheckQuestCount();
}

/// 퀘스트 UI 업데이트  - quest list
void cQuestManager::UpdateUIAll() 
{
	cQuestWindow* questWin = GAMEUI->GetQuestWindow();
	cQuestSimpleWindow* questSimpleWin = GAMEUI->GetQuestSimpleWindow();
	cMiniMapWindow* minimap = GAMEUI->GetMinimapWindow();

	if( !questWin || !questSimpleWin || !minimap )
	{
		assert( 0 && "error quest ui" );
		return;
	}
	
	mClosedCategory.Clear();
	mClosedSimpleCategory.Clear();
	questWin->GetHideMapCategorySet( mClosedCategory );
	questSimpleWin->GetHideMapCategorySet( mClosedSimpleCategory );
	cUIRect questSimpleWinPos = questSimpleWin->GetRelativeRect();
	

	/// UI 에 정보 세팅
	questWin->Clear();
	questSimpleWin->Clear();
	questSimpleWin->Hide();
	minimap->UpdateQuestChase();

	///
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( mQuest[i].questIdx );
		if( !define )
		{
			assert(0);
			continue;
		}

		questWin->AddQuest( i, mQuest[i].questIdx, (mQuest[i].check == define->mComplete) );

		if( IsSelectQuest( mQuest[i].questIdx ) == true )
			questSimpleWin->AddQuestInfo( mQuest[i].questIdx, &mQuest[i].check );
	}

	/// 접혀진 퀘스트 카테고리 맵 인덱스 적용
	cMapIndexSet::cConstIterator pos = mClosedCategory.Begin();
	cMapIndexSet::cConstIterator end = mClosedCategory.End();
	for( ; pos != end ; ++pos )
	{
		questWin->SetHideMapCategory( *pos );
	}
	pos = mClosedSimpleCategory.Begin();
	end = mClosedSimpleCategory.End();
	for( ; pos != end ; ++pos )
	{
		questSimpleWin->SetHideMapCategory( *pos );
	}

	///접혀진 상태의 윈도우 위치값 적용
	questSimpleWin->SetRelativePos( cUIPos(questSimpleWinPos.mLeft, questSimpleWinPos.mTop) );

	/// 갯수 갱신
	CheckQuestCount();
}

/// 퀘스트 진행중, 완료가능 갯수 갱신
void cQuestManager::CheckQuestCount()
{
	mCompleteCount = 0;
	mProgressCount = 0;
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( mQuest[i].questIdx );
		if( !define )
		{
			assert(0);
			continue;
		}

		/// 완료가능, 진행중 갯수 체크
		if( mQuest[i].check == define->mComplete )
			mCompleteCount++;
		else
			mProgressCount++;
	}

	/// 갯수 오류 검사
	if( mTotalKeepCount < mCompleteCount + mProgressCount )
	{
		assert(0);
		return;
	}

	/// UI 업데이트
	cQuestWindow* window = GAMEUI->GetQuestWindow();
	if( window )
		window->UpdateCount( mProgressCount, mCompleteCount );
}

void cQuestManager::AddNpcStatus( unsigned long npcIndex, unsigned long npcClassIndex )
{
	sNpcStausData* p = new sNpcStausData;
	p->npcClassIndex = npcClassIndex;
	p->effectType	 = eNPCQUEST_EFFECT_NONE;

	if( mNpcStatusMap.Insert( npcIndex, p ) == false )
	{
		SAFE_DELETE( p );
		assert(0);
	}
}

eNpcQuestEffect cQuestManager::GetNpcStatus( unsigned long npcIndex )
{
	sNpcStausData* p = (sNpcStausData*)mNpcStatusMap.GetAt( npcIndex );
	return ( p ) ? p->effectType : eNPCQUEST_EFFECT_NONE;
}

void cQuestManager::UpdateNpcStatus()
{
	if( mNpcStatusMap.IsEmpty() == true )
		return;

	/// 각 npc 상태 정보 갱신
	cNpcStatusMap::cIterator i = mNpcStatusMap.Begin();
	cNpcStatusMap::cIterator end = mNpcStatusMap.End();
	for( ; i != end; ++i )
	{
		unsigned long npcIndex = (unsigned long)(i->mFirst);
		sNpcStausData* p	   = (sNpcStausData*)(i->mSecond);
		if( p )
		{
			// 일반 npc 일때만 퀘스트 상태 처리
			sNPCList* npcInfo = NPCSCRIPT->GetNPCList( p->npcClassIndex );
			if( npcInfo && npcInfo->mType <= eNPCTYPE_TUTORIALBOOK)
			{
				eNpcQuestEffect type = UpdateNpcEffect( npcIndex, p->npcClassIndex );
				p->effectType = type;

				/// NPC 갱신
				cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIndex );
				if( pNpc )
				{
					pNpc->SetQuestEffect( type );
				}

				/// 미니맵 갱신
				cSmallMiniMapWindow* smini = GAMEUI->GetSmallMinimapWindow();
				cMiniMapWindow* mini = GAMEUI->GetMinimapWindow();
				if( smini && mini )	
				{
					smini->UpdateNpcEffect( npcIndex, type );
					mini->UpdateNpcEffect( npcIndex, type );
				}
			}
		}
		else
			assert(0);
	}
}

// 각 npc들의 이펙트 상태 결정하기
eNpcQuestEffect cQuestManager::UpdateNpcEffect( unsigned long npcIndex, unsigned long npcClassIndex )
{
	// 상태 우선순위 : 완료(테마>일반>반복) > 신규(테마>일반>반복) > 진행 > 고레벨 퀘스트 신규 > 저레벨 퀘스트 신규
	eNpcQuestEffect npcEffectType = eNPCQUEST_EFFECT_NONE;

	// 현재 소유한 퀘스트중
	sQuestList* quest = mQuest;
	for( unsigned int i = 0; i < mTotalKeepCount; ++i, ++quest )
	{
		cQuestDefine* define = GetQuestDefine( quest->questIdx );
		if( !define )
		{
			assert(0);
			continue;
		}
		
		if( define->mTakeNpcIndex != npcClassIndex )
			continue;

		// 보상가능
		if( quest->status == eQUEST_COMPLETE )
		{
			///우선 순쉬 계산 후 리턴( 테마 > 일반 > 반복 )
			switch( define->mType )
			{
			case eQUEST_TYPE_THEME:				///테마가 우선순위가 가장 높음. 바로 리턴
				return eNPCQUEST_EFFECT_THEME_REWARD;
			case eQUEST_TYPE_NORMAL:
			case eQUEST_TYPE_TUTORIAL:
				npcEffectType = eNPCQUEST_EFFECT_REWARD;
				break;
			case eQUEST_TYPE_REPEAT:
				if( npcEffectType != eNPCQUEST_EFFECT_REWARD )
					npcEffectType = eNPCQUEST_EFFECT_REPEAT_REWARD;
				break;
			default:
				{
					assert(0);
					continue;
				}
			}
		}

		// 진행중일때
		if( quest->status == eQUEST_PLAYING )
		{
			// 완료가능
			if( quest->check == define->mComplete )
			{
				///우선 순위 계산 후 리턴
				switch( define->mType )
				{
				///우선 순쉬 계산 후 리턴( 테마 > 일반 > 반복 )
				case eQUEST_TYPE_THEME:
					return eNPCQUEST_EFFECT_THEME_COMPLETE; ///테마가 우선순위가 가장 높음. 바로 리턴
				case eQUEST_TYPE_NORMAL:
				case eQUEST_TYPE_TUTORIAL:
					npcEffectType = eNPCQUEST_EFFECT_COMPLETE;
					break;
				case eQUEST_TYPE_REPEAT:
					if( npcEffectType != eNPCQUEST_EFFECT_COMPLETE )
						npcEffectType = eNPCQUEST_EFFECT_REPEAT_COMPLETE;
					break;
				default:
					{
						assert(0);
						continue;
					}
				}
			}
			else
			{
				// 그 외는 진행
				if( npcEffectType == eNPCQUEST_EFFECT_NONE )
					npcEffectType = eNPCQUEST_EFFECT_PLAYING;
			}
		}
	}

	///우선 순위에 맞게 리턴
	if( npcEffectType == eNPCQUEST_EFFECT_REWARD   || npcEffectType == eNPCQUEST_EFFECT_REPEAT_REWARD || 
		npcEffectType == eNPCQUEST_EFFECT_COMPLETE || npcEffectType == eNPCQUEST_EFFECT_REPEAT_COMPLETE )
		return npcEffectType;
	
	// 신규 검사
	if( mNewEventQuestSet.IsEmpty() == true && mNewQuestSet.IsEmpty() == true )
		return npcEffectType;

	sNpcQuestList* list = GetNpcQuestMap( npcClassIndex );
	if( !list )
		return npcEffectType;

	eNpcQuestEffect tempEffectType = eNPCQUEST_EFFECT_NONE;
	typedef tHashSet<unsigned long> cQuestSet;

	if( mNewEventQuestSet.IsEmpty() == false )
	{
		cQuestSet::cIterator s = list->mQuestSet.Begin();
		cQuestSet::cIterator send = list->mQuestSet.End();
		for( ; s != send; ++s )
		{
			unsigned long newIndex = (unsigned long)(*s);

			// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
			if( mNewEventQuestSet.Find( newIndex ) != mNewEventQuestSet.End() )
			{
				/// 저 레벨 퀘스트 걸러내기
				short lowLevel = TRIGGERMAN->GetLowLevel( newIndex );
				if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP ) )
				{
					tempEffectType = eNPCQUEST_EFFECT_LOW_LEVEL_NEW;
					continue;
				}

				/// 퀘스트 타입별로 구분하기
				cQuestDefine* define = GetQuestDefine( newIndex );
				if( !define )
				{
					assert(0);
					continue;
				}
				
				switch( define->mType )
				{
				///우선 순쉬 계산 후 리턴( 테마 > 일반 > 반복 )
				case eQUEST_TYPE_THEME:
					return eNPCQUEST_EFFECT_THEME_NEW; ///테마가 우선순위가 가장 높음. 바로 리턴
				case eQUEST_TYPE_NORMAL:
				case eQUEST_TYPE_TUTORIAL:
					npcEffectType =  eNPCQUEST_EFFECT_NEW;
					break;
				case eQUEST_TYPE_REPEAT:
					if( npcEffectType != eNPCQUEST_EFFECT_NEW )
						npcEffectType = eNPCQUEST_EFFECT_REPEAT_NEW;
					break;
				default:
					{
						assert(0);
						continue;
					}
				}
			}
		}
	}

	if( mNewQuestSet.IsEmpty() == false )
	{
		cQuestSet::cIterator s = list->mQuestSet.Begin();
		cQuestSet::cIterator send = list->mQuestSet.End();
		for( ; s != send; ++s )
		{
			unsigned long newIndex = (unsigned long)(*s);

			// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
			if( mNewQuestSet.Find( newIndex ) != mNewQuestSet.End() )
			{	
				/// 저 레벨 퀘스트 걸러내기
				short lowLevel = TRIGGERMAN->GetLowLevel( newIndex );
				if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP ) )
				{
					tempEffectType = eNPCQUEST_EFFECT_LOW_LEVEL_NEW;
					continue;
				}

				cQuestDefine* define = GetQuestDefine( newIndex );
				if( !define )
				{
					assert(0);
					continue;
				}

				switch( define->mType )
				{
				///우선 순위 계산 후 리턴( 테마 > 일반 > 반복 )
				case eQUEST_TYPE_THEME:
					return eNPCQUEST_EFFECT_THEME_NEW; ///테마가 우선순위가 가장 높음. 바로 리턴
				case eQUEST_TYPE_NORMAL:
				case eQUEST_TYPE_TUTORIAL:
					npcEffectType =  eNPCQUEST_EFFECT_NEW;
					break;
				case eQUEST_TYPE_REPEAT:
					if( npcEffectType != eNPCQUEST_EFFECT_NEW )
						npcEffectType = eNPCQUEST_EFFECT_REPEAT_NEW;
					break;
				default:
					{
						assert(0);
						continue;
					}
				}
			}
		}
	}

	// 상태 우선순위 : 진행 > 저레벨 퀘스트 신규
	if( npcEffectType == eNPCQUEST_EFFECT_NONE && tempEffectType == eNPCQUEST_EFFECT_LOW_LEVEL_NEW )
		 npcEffectType = eNPCQUEST_EFFECT_LOW_LEVEL_NEW;
	
	return npcEffectType;
}

void cQuestManager::SetNpcQuestList( cNpcWindow* win, unsigned long npcIndex, unsigned long npcClassIndex )
{
	if( win == 0 )
	{
		assert(0);
		return;
	}

	cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIndex );
	if( !pNpc )
	{
		assert(0);
		return;
	}

	/// npc의 퀘스트 상태가 없으면 리턴
	if( pNpc->GetQuestEffect() == eNPCQUEST_EFFECT_NONE )
		return;

	// 정렬
	sQuestList temp[MAX_KEEPQUEST];
	::memset( temp, 0, sizeof(temp) );

	sQuestList* data = mQuest;
	sQuestList* p = temp;
	for( unsigned int i = 0; i < mTotalKeepCount; ++i, ++data, ++p )
	{
		*p = *data;
	}

	if( mTotalKeepCount > 1 )
	{
		::qsort( (void*)temp, mTotalKeepCount, sizeof(sQuestList), SortQuest );
	}

	/// 완료 삽입
	/// 보상가능
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( temp[i].questIdx );
		if( define && define->mTakeNpcIndex == npcClassIndex )
		{
			if( temp[i].status == eQUEST_COMPLETE )
				win->AddQuest( temp[i].questIdx, eNPCQUEST_REWARD );
		}
	}

	/// 진행완료가능
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( temp[i].questIdx );
		if( define && define->mTakeNpcIndex == npcClassIndex )
		{
			if( temp[i].status == eQUEST_PLAYING && temp[i].check == define->mComplete )
				win->AddQuest( temp[i].questIdx, eNPCQUEST_COMPLETE );
		}
	}

	//// 신규 삽입
	typedef tHashSet<unsigned long> cQuestSet;
	typedef tList<unsigned long> cTempQuestList;
	cTempQuestList	tempQuestList;

	sNpcQuestList* list = GetNpcQuestMap( npcClassIndex );
	if( list )
	{
		if( mNewEventQuestSet.IsEmpty() == false )
		{
			cQuestSet::cIterator s = list->mQuestSet.Begin();
			cQuestSet::cIterator send = list->mQuestSet.End();
			for( ; s != send; ++s )
			{
				unsigned long newIndex = (unsigned long)(*s);

				// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
				if( mNewEventQuestSet.Find( newIndex ) != mNewEventQuestSet.End() )
				{
					tempQuestList.PushBack( newIndex );
				}
			}
		}

		if( mNewQuestSet.IsEmpty() == false )
		{
			cQuestSet::cIterator s = list->mQuestSet.Begin();
			cQuestSet::cIterator send = list->mQuestSet.End();
			for( ; s != send; ++s )
			{
				unsigned long newIndex = (unsigned long)(*s);

				// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
				if( mNewQuestSet.Find( newIndex ) != mNewQuestSet.End() )
				{
					tempQuestList.PushBack( newIndex );
				}
			}
		}
	}
	
	cTempQuestList	lowLevelQuestList;
	unsigned int size = tempQuestList.GetSize();
	if( size > 0 )
	{
		if( size > 1 )
			tempQuestList.Sort( QuestLevelSort );

		cTempQuestList::cIterator i = tempQuestList.Begin();
		cTempQuestList::cIterator end = tempQuestList.End();
		for( ; i != end; ++i )
		{
			unsigned long questIdx = (*i);

			win->AddQuest( questIdx, eNPCQUEST_NEW );
		}
	}

	// 진행중 삽입
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( temp[i].questIdx );
		if( define && define->mTakeNpcIndex == npcClassIndex )
		{
			if( temp[i].status == eQUEST_PLAYING && temp[i].check < define->mComplete )
				win->AddQuest( temp[i].questIdx, eNPCQUEST_PLAYING );
		}
	}
}

/// 신규 퀘스트 리스트 세팅, 반복과 일반 구분
bool cQuestManager::SetBoardQuestList()
{
	/// npc 윈도우에 퀘스트 분류후 삽입
	cBoardWindow* boardWin = GAMEUI->GetBoardWindow();
	if( !boardWin )
	{
		assert(0);
		return false;
	}

	// 퀘스트 목록 요청
	unsigned long npcIndex = HERO->GetNpcIdx();
	if( npcIndex == 0 )
	{
		assert(0);
		return false;
	}

	cNpc* npc = OBJECTMANAGER->GetNpc( npcIndex );
	if( !npc )
	{
		assert(0);
		return false;
	}

	unsigned long npcClassIndex = npc->GetClassId();

	//// 신규 삽입
	typedef tHashSet<unsigned long> cQuestSet;
	typedef tList<unsigned long> cTempQuestList;
	cTempQuestList	tempQuestList;
	cTempQuestList	tempRepeatQuestList;

	sNpcQuestList* list = GetNpcQuestMap( npcClassIndex );
	if( list )
	{
		if( mNewEventQuestSet.IsEmpty() == false )
		{
			cQuestSet::cIterator s = list->mQuestSet.Begin();
			cQuestSet::cIterator send = list->mQuestSet.End();
			for( ; s != send; ++s )
			{
				unsigned long newIndex = (unsigned long)(*s);

				// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
				if( mNewEventQuestSet.Find( newIndex ) != mNewEventQuestSet.End() )
				{
					cQuestDefine* define = QUESTMAN->GetQuestDefine( newIndex );
					if( define )
					{
						if( define->mRepeatType == eQUEST_REPEAT )
							tempRepeatQuestList.PushBack( newIndex );
						else
							tempQuestList.PushBack( newIndex );
					}
					else
						assert(0);
				}
			}
		}

		if( mNewQuestSet.IsEmpty() == false )
		{
			cQuestSet::cIterator s = list->mQuestSet.Begin();
			cQuestSet::cIterator send = list->mQuestSet.End();
			for( ; s != send; ++s )
			{
				unsigned long newIndex = (unsigned long)(*s);

				// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
				if( mNewQuestSet.Find( newIndex ) != mNewQuestSet.End() )
				{
					cQuestDefine* define = QUESTMAN->GetQuestDefine( newIndex );
					if( define )
					{
						if( define->mRepeatType == eQUEST_REPEAT )
							tempRepeatQuestList.PushBack( newIndex );
						else
							tempQuestList.PushBack( newIndex );
					}
					else
						assert(0);
				}
			}
		}
	}

	//
	boardWin->Clear();

	/// eQUEST_CHALLENGE 형 넣기.
	unsigned int size = tempQuestList.GetSize();
	if( size > 0 )
	{
		if( size > 1 )
			tempQuestList.Sort( QuestLevelSort );

		cTempQuestList::cIterator i   = tempQuestList.Begin();
		cTempQuestList::cIterator end = tempQuestList.End();
		for( ; i != end; ++i )
		{
			unsigned long questIdx = (*i);
			boardWin->AddQuest( questIdx );
		}
	}

	/// eQUEST_REPEAT 형 넣기.
	size = tempRepeatQuestList.GetSize();
	if( size > 0 )
	{
		if( size > 1 )
			tempRepeatQuestList.Sort( QuestLevelSort );
		
		cTempQuestList::cIterator i   = tempRepeatQuestList.Begin();
		cTempQuestList::cIterator end = tempRepeatQuestList.End();
		for( ; i != end; ++i )
		{
			unsigned long questIdx = (*i);
			boardWin->AddQuest( questIdx );
		}
	}


	boardWin->ChangePage( eBOARD_LIST );
	return true;
}

void cQuestManager::SetQuestTip( unsigned long npcclassIndex )
{
	cQuestTipWindow* tip = GAMEUI->GetQuestTip();
	if( !tip )
	{
		assert(0);
		return;
	}
	
	UIMAN->HideTip();

	// 정보 세팅
	sQuestList temp[MAX_KEEPQUEST];
	::memset( temp, 0, sizeof(temp) );

	sQuestList* data = mQuest;
	sQuestList* p = temp;
	for( unsigned int i = 0; i < mTotalKeepCount; ++i, ++data, ++p )
	{
		*p = *data;
	}

	if( mTotalKeepCount > 1 )
	{
		::qsort( (void*)temp, mTotalKeepCount, sizeof(sQuestList), SortQuest );
	}

	/// 완료가능
	bool completeQuest = false;
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( temp[i].questIdx );
		if( define && define->mTakeNpcIndex == npcclassIndex )
		{
			if( temp[i].status == eQUEST_COMPLETE || (temp[i].status == eQUEST_PLAYING && temp[i].check == define->mComplete) )
			{
				if( completeQuest == false )
				{
					tip->AddQuestInfo( UIMAN->GetUIText( 231 ), COLOR_WHITE, true );
					completeQuest = true;
				}

				LPCTSTR title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );

				unsigned long textColor = COLOR_GREEN;
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 474 ), define->mLevel, title );
//				if( define->mRepeatType == eQUEST_REPEAT )
//					str.Append( GAMERESOURCEMAN->GetGameText( 472 ) );
				
				/// 저 레벨 퀘스트 체크
				short lowLevel = TRIGGERMAN->GetLowLevel( temp[i].questIdx );
				if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
				{
					str.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
					textColor = COLOR_LOW_LEVEL_QUEST;
				}

				tip->AddQuestInfo( (LPCTSTR)str.Cstr(), textColor );
			}
		}
	}


	//// 신규 삽입
	typedef tHashSet<unsigned long> cQuestSet;
	typedef tList<unsigned long> cTempQuestList;
	cTempQuestList	tempQuestList;

	bool newQuest = false;
	sNpcQuestList* list = GetNpcQuestMap( npcclassIndex );
	if( list != NULL )
	{
		if( mNewEventQuestSet.IsEmpty() == false )
		{
			cQuestSet::cIterator s = list->mQuestSet.Begin();
			cQuestSet::cIterator send = list->mQuestSet.End();
			for( ; s != send; ++s )
			{
				unsigned long newIndex = (unsigned long)(*s);

				// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
				if( mNewEventQuestSet.Find( newIndex ) != mNewEventQuestSet.End() )
				{
					tempQuestList.PushBack( newIndex );
				}
			}
		}

		if( mNewQuestSet.IsEmpty() == false )
		{
			cQuestSet::cIterator s = list->mQuestSet.Begin();
			cQuestSet::cIterator send = list->mQuestSet.End();
			for( ; s != send; ++s )
			{
				unsigned long newIndex = (unsigned long)(*s);

				// 신규 퀘스트 목록중 해당 npc가 제공하는 퀘스트가 있으면
				if( mNewQuestSet.Find( newIndex ) != mNewQuestSet.End() )
				{
					tempQuestList.PushBack( newIndex );
				}
			}
		}
	}

	cTempQuestList	lowLevelQuestList;
	unsigned int size = tempQuestList.GetSize();
	if( size > 0 )
	{
		if( size > 1 )
			tempQuestList.Sort( QuestLevelSort );

		cTempQuestList::cIterator i = tempQuestList.Begin();
		cTempQuestList::cIterator end = tempQuestList.End();
		for( ; i != end; ++i )
		{
			unsigned long questIdx = (*i);
			
			cQuestDefine* define = GetQuestDefine( questIdx );
			if( define )
			{
				unsigned long questColor = COLOR_GREEN;
				short lowLevel = TRIGGERMAN->GetLowLevel( questIdx );
				if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
					questColor = COLOR_LOW_LEVEL_QUEST;

				if( newQuest == false )
				{
					tip->AddQuestInfo( UIMAN->GetUIText( 229 ), COLOR_WHITE, true );
					newQuest = true;
				}

				LPCTSTR title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );

				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 474 ), define->mLevel, title );
//				if( define->mRepeatType == eQUEST_REPEAT )
//					str.Append( GAMERESOURCEMAN->GetGameText( 472 ) );
				if( questColor == COLOR_LOW_LEVEL_QUEST )
					str.Append( GAMERESOURCEMAN->GetGameText( 476 ) );

				tip->AddQuestInfo( (LPCTSTR)str.Cstr(), questColor );
			}

		}
	}

	// 진행중 삽입
	bool progressQuest = false;
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( temp[i].questIdx );
		if( define && define->mTakeNpcIndex == npcclassIndex )
		{
			if( temp[i].status == eQUEST_PLAYING && temp[i].check < define->mComplete )
			{
				if( progressQuest == false )
				{
					tip->AddQuestInfo( UIMAN->GetUIText( 230 ), COLOR_WHITE, true );
					progressQuest = true;
				}

 				LPCTSTR title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );
 
				unsigned long textColor = COLOR_GREEN;
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 474 ), define->mLevel, title );
//				if( define->mRepeatType == eQUEST_REPEAT )
//					str.Append( GAMERESOURCEMAN->GetGameText( 472 ) );

				/// 저 레벨 퀘스트 체크
				short lowLevel = TRIGGERMAN->GetLowLevel( temp[i].questIdx );
				if( lowLevel != 0 && lowLevel <= (HERO->GetLevel() - LOW_LEVEL_QUEST_GAP) )
				{
					str.Append( GAMERESOURCEMAN->GetGameText( 476 ) );
					textColor = COLOR_LOW_LEVEL_QUEST;
				}

				tip->AddQuestInfo( (LPCTSTR)str.Cstr(), textColor );
			}
		}
	}

	if( completeQuest == true || progressQuest == true || newQuest == true ) 
	{
		// 팁보기
		UIMAN->ShowQuestTip( tip, cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
	}
}

/// 퀘스트 신규 추가
bool cQuestManager::QuestAddByNpc( unsigned long questIdx, unsigned long npcIndex )
{
	if( IsNetLock() )
		return false;

	if( npcIndex == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 447 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		assert(0);
		return false;	
	}

	/// 유효성 검사
	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
	{
		assert(0);
		return false;	
	}

	/// 이미 보유중인 퀘스트인지 검사
	int arrIdx = GetQuestArrIdx( questIdx );
	if( arrIdx != -1 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 402 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		return false;
	}

	MSG_REQ_QUEST_ADD msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_QUEST_ADD_REQ;
	msg.questIndex = questIdx;
	msg.npcIndex = npcIndex;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 락걸기
	NetLock( eLOCK_QUESTADD );
	return true;
}

bool cQuestManager::QuestAddByItem( unsigned short number )
{
	if( IsNetLock() )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, GAMERESOURCEMAN->GetGameText( 223 ) );
		return false;
	}
	
	cItem* item = ITEMMAN->GetItem( number );
	if( !item )
	{
		assert(0);
		return false;
	}

	cItemDefine* itemDefine = ITEMMAN->GetItemDefine( item->GetIndex() );
	if( !itemDefine )
	{
		assert(0);
		return false;
	}

	cItemAbility* itemAbil = itemDefine->GetAbililty();
	if( !itemAbil )
	{
		assert(0);
		return false;
	}

	long questIndex = itemAbil->GetInfluenceIndex();
	if( questIndex <= 0 )
	{
		assert(0);
		return false;
	}

	/// 
	cQuestDefine* define = GetQuestDefine( questIndex );
	if( !define )
	{
		assert(0);
		return false;	
	}

	/// 이미 보유중인 퀘스트인지 검사
	int arrIdx = GetQuestArrIdx( questIndex );
	if( arrIdx != -1 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 402 ),
			GAMERESOURCEMAN->GetGameText( 301 ),
			0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
		return false;
	}

	/// 의뢰아이템을 이용한 퀘스트 부여
	MSG_REQ_QUEST_ADDBYITEM msg;
	::memset( &msg, 0, sizeof(msg) );

	msg.Category = NM_QUEST;
	msg.Protocol = NM_QUEST_ADDBYITEM_REQ;
	msg.number = (unsigned short)number;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 퀘스트 락
	NetLock( eLOCK_QUESTADDBYITEM, number );
	return true;
}

/// 퀘스트 포기 - 퀘스트 ui에서 실행함
void cQuestManager::QuestDelete( unsigned int arrIdx )
{
	if( IsNetLock() )
		return;

	if( mTotalKeepCount == 0 )
		return;

	/// 배열보다 크면 에러
	if( arrIdx >= mTotalKeepCount )
	{
		assert(0);
		return;
	}

	/// 소유한 퀘스트가 아님
	if( mQuest[arrIdx].questIdx == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 407 ),
			GAMERESOURCEMAN->GetGameText( 301 ),
			0, GAMEUI->GetQuestWindow() );
		return;
	}

	cQuestDefine* define = GetQuestDefine( mQuest[arrIdx].questIdx );
	if( !define )
	{
		assert(0);
		return;
	}

	if( define->mType == eQUEST_TYPE_TUTORIAL )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 430 ),
			GAMERESOURCEMAN->GetGameText( 301 ),
			0, GAMEUI->GetQuestWindow() );
		return;
	}

	MSG_REQ_QUEST_DEL msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_QUEST;
	msg.Protocol = NM_QUEST_DEL_REQ;
	msg.arrayIdx = arrIdx;
	msg.questIndex = mQuest[arrIdx].questIdx;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 저장
	mCheckQuestIndex = mQuest[arrIdx].questIdx;
	NetLock( eLOCK_QUESTDELETE );
}

void cQuestManager::QuestDeleteAuto( unsigned int arrIdx )
{
	if( mLockTry == eLOCK_QUESTDELETEAUTO )
		return;

	if( mTotalKeepCount == 0 )
		return;

	/// 배열보다 크면 에러
	if( arrIdx >= mTotalKeepCount )
	{
		assert(0);
		return;
	}

	/// 소유한 퀘스트가 아님
	if( mQuest[arrIdx].questIdx == 0 )
	{
		assert(0);
		DeleteQuestTime( mQuest[arrIdx].questIdx );
		return;
	}

	MSG_REQ_QUEST_DELAUTO msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_QUEST;
	msg.Protocol = NM_QUEST_DELAUTO_REQ;
	msg.arrayIdx = arrIdx;
	msg.questIndex = mQuest[arrIdx].questIdx;

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 저장
	mCheckQuestIndex = mQuest[arrIdx].questIdx;
	NetLock( eLOCK_QUESTDELETEAUTO );
}

/// 퀘스트 완료요청
bool cQuestManager::QuestComplete( unsigned long questIdx, unsigned long npcIndex )
{
	if( IsNetLock() )
		return false;

	if( npcIndex == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 447 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		assert(0);
		return false;	
	}

	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
	{
		assert(0);
		return false;	
	}

	/// 소유중인 퀘스트인지 검사
	int arrIdx = GetQuestArrIdx( questIdx );
	if( arrIdx == -1 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 407 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 진행완료가능 상태인지 검사
	if( mQuest[arrIdx].status != eQUEST_PLAYING )
	{
		/// 이미 진행완료한 퀘스트
		if( mQuest[arrIdx].status == eQUEST_COMPLETE )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 428 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		/// 그 외의 잘못된 상태
		else
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 426 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		return false;
	}

	///
	cQuestReward* reward = define->mDefaultReward;
	if( reward )
	{
		TCHAR temp[256]={0,};

		/// 전직 퀘인지 검사
		if( reward->mChangeJobIndex > 0 )
		{
			LPCTSTR jobStr = GAMERESOURCEMAN->GetJobName( reward->mChangeJobIndex );
			::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 435 ), jobStr );

			mTempQuestIndex = questIdx;
			mTempNpcIndex = npcIndex;
			mTempArrIndex = arrIdx;

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_QUEST_REWARDJOB, eSKIN_YESNO,
				temp,
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			return true;
		}
		/// 세력 퀘인지 검사
		else if( reward->mForceType != eFORCETYPE_NONE )
		{
			int textIdx = 0;
			switch( reward->mForceType )
			{
			case eFORCETYPE_NONE:	textIdx = 119;	break;
			case eFORCETYPE_FIRE:	textIdx = 120;	break;
			case eFORCETYPE_WATER:  textIdx = 121;	break;
			case eFORCETYPE_WIND:	textIdx = 122;	break;
			case eFORCETYPE_EARTH:	textIdx = 123;	break;
			default: assert(0); return false;
			}

			LPCTSTR forceStr = UIMAN->GetUIText( textIdx );
			::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 436 ), forceStr );
			
			mTempQuestIndex = questIdx;
			mTempNpcIndex = npcIndex;
			mTempArrIndex = arrIdx;

			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_QUEST_REWARDJOB, eSKIN_YESNO,
				temp,							
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			return true;
		}
	}
	
	/// 보내기
	MSG_REQ_NPC_QUEST_COMPLETE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_QUEST_COMPLETE_REQ;
	msg.arrayIdx = arrIdx;
	msg.questIndex = questIdx;
	msg.npcIndex = npcIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 저장
	mCheckQuestIndex = questIdx;

	NetLock( eLOCK_QUESTCOMPLETE );
	return true;
}

bool cQuestManager::QuestComplete()
{
	if( !(mTempQuestIndex > 0 && mTempNpcIndex > 0) )
	{
		assert(0);
		return false;
	}

	/// 보내기
	MSG_REQ_NPC_QUEST_COMPLETE msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_QUEST_COMPLETE_REQ;
	msg.arrayIdx = mTempArrIndex;
	msg.questIndex = mTempQuestIndex;
	msg.npcIndex = mTempNpcIndex;
	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

	/// 저장
	mCheckQuestIndex = mTempQuestIndex;

	mTempQuestIndex = 0;
	mTempNpcIndex = 0;
	mTempArrIndex = 0;

	NetLock( eLOCK_QUESTCOMPLETE );
	return true;
}

bool cQuestManager::QuestReward( unsigned long questIdx, unsigned long npcIndex )
{
	if( IsNetLock() )
		return false;

	if( npcIndex == 0 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 447 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
		assert(0);
		return false;	
	}

	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
	{
		assert(0);
		return false;	
	}

	/// 소유중인 퀘스트인지 검사
	int arrIdx = GetQuestArrIdx( questIdx );
	if( arrIdx == -1 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 407 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 진행완료를 거친 퀘스트인지 검사
	if( mQuest[arrIdx].status != eQUEST_COMPLETE )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 427 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	cNpcRewardWindow* pReward = GAMEUI->GetNpcRewardWindow();
	if( !pReward )
	{
		assert(0);
		return false;
	}

	/// 보상 갯수 검사
	if( define->mSelectCount != pReward->GetSelectCount() )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 413 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 슬롯 검사용
	sItemData	itemData[10];
	::memset( itemData, 0, sizeof(itemData) );
	unsigned int needCnt = 0;
	unsigned int totalWeight = 0;

	cQuestReward* defaultReward = define->mDefaultReward;
	if( defaultReward )
	{
		for( int i = 0; i < 5; ++i, ++needCnt )
		{
			if( defaultReward->mRewardItem[i].itemIndex == 0 )
				break;

			itemData[needCnt].itemDefineIndex	= defaultReward->mRewardItem[i].itemIndex;
			itemData[needCnt].count				= (unsigned short)defaultReward->mRewardItem[i].count;

			/// 보상 아이템 무게 구하기.
			cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( itemData[needCnt].itemDefineIndex );
			if( pItemDefine )
				totalWeight += pItemDefine->GetWeight() * itemData[needCnt].count;
			else
			{
				assert( 0 && "bad item" );
				return false;
			}
		}		
	}

	/// 선택 보상인 경우
	sRewardData selectData[5];
	::memset( selectData, 0, sizeof(selectData) );
	unsigned int rowCount = 0;

	if( define->mSelectCount > 0 )
	{
		/// 선택 보상인 경우
		sRewardIcon* data = pReward->GetSelectData();
		for( unsigned int i = 0; i < 5; ++i, ++data )
		{
			cIcon* icon = data->icon;
			if( icon && data->select == true )
			{
				selectData[rowCount].type = ( icon->GetIconType() == eICON_ITEM )? eREWARD_ITEM : eREWARD_SKILL;
				selectData[rowCount].index = data->index;

				itemData[needCnt].itemDefineIndex	= data->index;
				itemData[needCnt].count				= (unsigned short)icon->GetNumber();
				
				/// 보상 아이템 무게 구하기.
				if( selectData[rowCount].type == eREWARD_ITEM )
				{
					cItemDefine* pItemDefine = ITEMMAN->GetItemDefine( itemData[needCnt].itemDefineIndex );
					if( pItemDefine )
						totalWeight += pItemDefine->GetWeight() * itemData[needCnt].count;
					else
					{
						assert( 0 && "bad item" );
						return false;
					}
				}

				//
				needCnt++;

				if( rowCount < define->mSelectCount )
					rowCount++;
			}
		}
	}

	// 보상관련 아이템이 있을때만 검사하기
	if( totalWeight > 0 )
	{
		// 무게검사
		if( HERO->GetMaxWeight() < ITEMMAN->GetWeight() + totalWeight )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 208 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
			return false;
		}

		/// 빈슬롯 검사
		unsigned int emptySlotCnt = 0;
		for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
		{
			/// 빈슬롯 검사
			cItem* item = ITEMMAN->GetItem( slot );
			if( item && item->GetIndex() == 0 )
			{
				emptySlotCnt++;

				/// 빈슬롯이 더 많으면 통과
				if( emptySlotCnt >= needCnt )
					break;
			}
		}

		/// 슬롯이 부족할때 겹치기 검사도 한다
		if( emptySlotCnt < needCnt )
		{
			for( unsigned int i = 0; i < needCnt; ++i )
			{
				unsigned long itemIndex = itemData[i].itemDefineIndex;
				unsigned int  count		= itemData[i].count;

				cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
				if( !define )
				{
					assert(0);
					continue;
				}

				for( unsigned short slot = INVENTORY_BAG0_BEGIN; slot <= HERO->GetBagEndSlot(); ++slot )
				{
					/// 겹치는 슬롯
					cItem* item = ITEMMAN->GetItem( slot );
					if( item && item->GetIndex() == itemIndex )
					{
						int remainCnt = define->GetCapacity() - item->GetCount();
						count -= remainCnt;

						if( count <= 0 )
							break;
					}
				}

				/// 다 못찾으면 빈슬롯 사용하기
				if( count > 0 )
				{
					emptySlotCnt--;
					if( emptySlotCnt < 0 )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 202 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
						return false;
					}
				}
			}
		}
	}

	/// 보내기
	MSG_REQ_NPC_QUEST_REWARD msg;
	::memset( &msg, 0, sizeof( msg ) );

	msg.Category = NM_NPC;
	msg.Protocol = NM_NPC_QUEST_REWARD_REQ;
	msg.arrayIdx = arrIdx;
	msg.questIndex = questIdx;
	msg.npcIndex = npcIndex;
	msg.rowCount = pReward->GetSelectCount();

	if( define->mSelectCount > 0 )
	{
		for( unsigned int i = 0; i < rowCount; ++i )
		{
			msg.table[i].type = selectData[i].type;
			msg.table[i].index = selectData[i].index;
		}
	}

	NETWORK->SendNetworkMsg( (char*)&msg, sizeof(MSG_REQ_NPC_QUEST_REWARD) );

	/// 저장
	mCheckQuestIndex = questIdx;
	NetLock( eLOCK_QUESTREWARD );
	return true;
}

/// 신규 퀘스트 승인 결과
void cQuestManager::ResQuestAdd( MSG_RES_QUEST_ADD* pmsg )
{
	switch( pmsg->ErrorCode )
	{
	case ERROR_QUEST_ADD_SUCCESS:
		{
			sQuestList* table = &pmsg->table;
			sQuestList* quest = mQuest + mTotalKeepCount;

			/// 저장
			(*quest) = (*table);
			mTotalKeepCount++;

			/// UI 업데이트
			UpdateUI();

			/// 각 맵에 포함된 퀘스트 갯수 갱신
			CheckCountMap();

			/// 새로 추가된 퀘스트를 기본선택 처리
			AddQuestSelect( quest->questIdx, true );
			
			//
			mChaseQuestIndex = quest->questIdx;
			UpdateChaseQuest( mChaseQuestIndex );

			///
			cQuestDefine* define = GetQuestDefine( quest->questIdx );
			if( define )
			{
				sNPCList* npcInfo = NPCSCRIPT->GetNPCList( define->mGiveNpcIndex );
				if( npcInfo )
				{
					if( npcInfo->mType == eNPCTYPE_NORMAL )
					{
						/// NPC 장면윈도우 갱신
						cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
						if( npcWin )
							npcWin->UpdateDialog();
					}
					else if( npcInfo->mType == eNPCTYPE_TUTORIALBOOK )
					{
						cNpcTutorialWindow* win = GAMEUI->GetNpcTutorialWindow();
						if( win )
							win->UpdateDialog();
					}
				}

				//
				LPCTSTR title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );
			
				cStringT str;
				str.Format( GAMERESOURCEMAN->GetGameText( 400 ), title );
				CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, (LPCTSTR)str.Cstr() );
			}

			/// 시간정보 저장
			if( quest->status == eQUEST_PLAYING )
				AddQuestTime( quest->questIdx, quest->restTime );

			if( mChaseQuestIndex == quest->questIdx )
				mChaseQuestIndex = 0;
		}
		break;
	case ERROR_QUEST_ADD_FAIL:		// 실패 - 알수 없는 오류
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 401 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_NOTYET:	// 실패 - DB 사용중
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 401 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_EXIST:		// 실패 - 이미 해당 퀘스트를 가지고있음
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 402 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;	
	case ERROR_QUEST_ADD_COMPLETE:	// 실패 - 이미 완료한 퀘스트
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 403 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_TRIGGER:	// 실패 - 퀘스트 받을 조건이 안됨
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 404 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_MAX:		// 실패 - 퀘스트 목록이 FULL 
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 405 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_ITEM:		/// 실패 - 인벤 부족
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 409 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_NPC:
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 420 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_TIME:
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 437 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_MONEY:
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 438 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_POINT:
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 439 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_ADD_MAX_ITEM:
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 220 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	default:
		assert(0);
		break;
	}

	NetUnLock( eLOCK_QUESTADD );
}

/// 퀘스트 진행완료 요청 결과
void cQuestManager::ResQuestComplete( MSG_RES_NPC_QUEST_COMPLETE* pmsg )
{
	switch( pmsg->ErrorCode )
	{
	case ERROR_QUEST_COMPLETE_SUCCESS:		// 성공 - 진행 완료 요청 성공
		{
			/// 상태 정보 갱신
			sQuestList* quest = mQuest + pmsg->arrayIdx;
			if( quest->questIdx > 0 )
			{
				quest->status = pmsg->status;

				/// 진행완료
				CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, GAMERESOURCEMAN->GetGameText( 406 ) );

				cQuestDefine* define = GetQuestDefine( quest->questIdx );
				if( define )
				{
					sNPCList* npcInfo = NPCSCRIPT->GetNPCList( define->mTakeNpcIndex );
					if( npcInfo )
					{
						if( npcInfo->mType == eNPCTYPE_NORMAL )
						{
							/// NPC 장면윈도우 갱신
							cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
							if( npcWin )
								npcWin->UpdateDialog();
						}
						else if( npcInfo->mType == eNPCTYPE_TUTORIALBOOK )
						{
							cNpcTutorialWindow* win = GAMEUI->GetNpcTutorialWindow();
							if( win )
								win->UpdateDialog();
						}
					}
				}

				if( OPTIONMAN->IsAutoSendTwitterMsg()	== true ) 
				{
					cQuestReward* reward	= define->mDefaultReward;
					if( reward  )
					{
						if( reward->mChangeJobIndex > 0 )
						{

							cTwitterWindow* pTwitWin = GAMEUI->GetTwitterWindow();
							if( pTwitWin )
							{
								if( pTwitWin->IsLogin() == true )
								{
									sTwitterEvent tEvent;
									tEvent.mEvent	= eTWITEVENT_QUEST;
									tEvent.mExData	= (void*)reward->mChangeJobIndex;

									pTwitWin->AddEvent( tEvent );									
								}							
							}
							else
							{
								assert(0);
							}
						}
					}
				}

				/// 퀘스트 완료 효과
				NiTransform trans;
				trans.MakeIdentity();
				trans.m_Translate = NiPoint3( 0.0f, 0.0f, 50.0f );
				HERO->SetLinkdEffect( eLINK_HEAD2, "./Data/Effect/eff_pc_get_05.nif",&trans, false, true );
 				SOUNDSYS->Play2DSound( 2012 );

				if( mChaseQuestIndex == quest->questIdx )
					mChaseQuestIndex = 0;
			}
		}
		break;
	case ERROR_QUEST_COMPLETE_NOTYET:		// 실패 - DB 사용중
	case ERROR_QUEST_COMPLETE_FAIL:			// 실패 - 오류
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 424 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_COMPLETE_POSSIBLE:		// 실패 - 완료조건 아님
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 421 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_COMPLETE_TAKE:			// 실패 - 아이템 회수 실패
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 423 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_COMPLETE_NPC:			// 실패 - NPC 인증
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 420 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_COMPLETE_TIME:			// 실패 - 시간제한 오버
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 425 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_COMPLETE_ALREADY:		// 실패 - 이미 완료한 퀘스트
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 424 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	case ERROR_QUEST_COMPLETE_TUTORIALMODE:
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 417 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		break;
	}

	NetUnLock( eLOCK_QUESTCOMPLETE );
	
	/// 초기화
	mCheckQuestIndex = ULONG_MAX;
}

/// 퀘스트 보상 요청 결과
void cQuestManager::ResQuestReward( MSG_RES_NPC_QUEST_REWARD* pmsg )
{
	switch( pmsg->ErrorCode )
	{
	case ERROR_QUEST_REWARD_SUCCESS:	// 성공 - 완료 요청 성공
		{
			cQuestDefine* define = GetQuestDefine( mCheckQuestIndex );
			if( define )
			{
				sNPCList* npcInfo = NPCSCRIPT->GetNPCList( define->mTakeNpcIndex );
				if( npcInfo )
				{
					if( npcInfo->mType == eNPCTYPE_NORMAL )
					{
						/// NPC 장면윈도우 갱신
						cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
						if( npcWin )
							npcWin->UpdateDialog();
					}
					else if( npcInfo->mType == eNPCTYPE_TUTORIALBOOK )
					{
						cNpcTutorialWindow* win = GAMEUI->GetNpcTutorialWindow();
						if( win )
							win->UpdateDialog();
					}
				}
			}

			CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, GAMERESOURCEMAN->GetGameText( 429 ) );
		}
		break;
	case ERROR_QUEST_REWARD_NOTYET:		// 실패 - DB 사용중
	case ERROR_QUEST_REWARD_FAIL:		// 실패 - 오류
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 424 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case ERROR_QUEST_REWARD_ITEM:		// 실패 - 아이템 보상 지급 실패
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 422 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case ERROR_QUEST_REWARD_MAX_ITEM:		// 실패 - 오류
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 220 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case ERROR_QUEST_REWARD_MONEY:		// 실패 - 보상 금액 지급 실패
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 414 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case ERROR_QUEST_REWARD_COUNT:		// 실패 - 선택갯수 적음
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 413 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case ERROR_QUEST_REWARD_NPC:		// 실패 - NPC 인증
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 420 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	case ERROR_QUEST_REWARD_COMPLETE:		// 실패 - 진행완료상태 아님
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 427 ),
				GAMERESOURCEMAN->GetGameText( 301 )
				);
		}
		break;
	default:
		assert(0);
		break;
	}

	NetUnLock( eLOCK_QUESTREWARD );
	
	/// 초기화
	mCheckQuestIndex = ULONG_MAX;
}

/// Load, MapChange 시에 오는 메세지 처리
void cQuestManager::NetworkMsgParserLoad( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	if( tempMsg->Protocol == NM_QUEST_LIST_RES )
	{
		MSG_RES_QUEST_LIST* pmsg = (MSG_RES_QUEST_LIST*)msg;

		/// 초기화
		memset( mQuest, 0, sizeof(mQuest) );
		ClearQuestTime();

		mTotalKeepCount = 0;
		mCompleteCount = 0;
		mProgressCount = 0;

		if( pmsg->rowCount > 0 )
		{
			/// 임시삽입
			sQuestList* quest = mQuest;
			sQuestList* table = pmsg->table;
			for( unsigned char i = 0; i < pmsg->rowCount; i++, table++, quest++ )
			{
				(*quest) = (*table);

				/// 시간맵에 저장
				if( quest->status == eQUEST_PLAYING )
					AddQuestTime( quest->questIdx, quest->restTime );
			}

			mTotalKeepCount = pmsg->rowCount;

			/// 각 맵에 포함된 퀘스트 갯수 갱신
			CheckCountMap();
		}
	}
}

///
void cQuestManager::NetworkMsgParser( char* msg )
{
	MSGROOT* tempMsg = (MSGROOT*)msg;

	switch( tempMsg->Protocol )
	{
	case NM_QUEST_LIST_RES:		/// 퀘스트리스트, 퀘스트삭제시 새로 온다
		{
			MSG_RES_QUEST_LIST* pmsg = (MSG_RES_QUEST_LIST*)msg;

			/// 초기화
			memset( mQuest, 0, sizeof(mQuest) );
			mTotalKeepCount = 0;

			if( pmsg->rowCount > 0 )
			{
				/// 임시삽입
				sQuestList* quest = mQuest;
				sQuestList* table = pmsg->table;
				for( unsigned char i = 0; i < pmsg->rowCount; i++, table++ )
				{
					(*quest) = (*table);
					quest++;
				}

				mTotalKeepCount = pmsg->rowCount;
			}

			/// 각 맵에 포함된 퀘스트 갯수 갱신
			CheckCountMap();

			/// UI 업데이트
			UpdateUIAll();
		}
		break;
	case NM_QUEST_UPDATE_SYN:
		{
			/// 퀘스트 진행정보 업데이트 받음
			MSG_RES_QUEST_UPDATE* pmsg = (MSG_RES_QUEST_UPDATE*)msg;

			cQuestWindow* window = GAMEUI->GetQuestWindow();

			sQuestList* table = pmsg->table;
			for( unsigned char i = 0; i < pmsg->rowCount; i++, table++ )
			{
				sQuestList* quest = mQuest + pmsg->table[i].arrIdx;

				/// 업데이트 된 내용 출력
				ActiveQuestText( quest, table );

				/// 업데이트
				(*quest) = (*table);

				/// UI 갱신
				cQuestDefine* define = GetQuestDefine( quest->questIdx );
				if( window && define )
				{
					window->UpdateQuestInfo();

					/// 완료된 퀘스트이면 퀘스트윈도우의 리스트도 갱신
					if( quest->check == define->mComplete )
						window->SetCompleteQuest( quest->arrIdx, quest->questIdx );
				}
			}

			/// 갯수 갱신
			CheckQuestCount();

			if( pmsg->rowCount > 0 )
			{
				UpdateNpcStatus();
			}
		}
		break;
	case NM_QUEST_DEL_RES:
		{
			/// 퀘스트 삭제 요청에 대한 결과
			MSG_RES_QUEST_DEL* pmsg = (MSG_RES_QUEST_DEL*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_QUEST_DEL_SUCCESS:
				if( mCheckQuestIndex != ULONG_MAX )
				{
					cQuestDefine* define = GetQuestDefine( mCheckQuestIndex );
					if( define )
					{
						cStringT str;
						str.Format( GAMERESOURCEMAN->GetGameText( 410 ), GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ));
						CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, (LPCTSTR)str.Cstr() );
					}
					else
						assert(0);

					if( mChaseQuestIndex == mCheckQuestIndex )
						mChaseQuestIndex = 0;

					/// 시간정보 삭제
					DeleteQuestTime( mCheckQuestIndex );
				}
				else
					assert(0);
				break;
			case ERROR_QUEST_DEL_NOTYET:	// 실패 - DB 사용중
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 411 ),
					GAMERESOURCEMAN->GetGameText( 301 ), 0, GAMEUI->GetQuestWindow() );
				break;
			case ERROR_QUEST_DEL_FAIL:		// 실패 - 오류
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 411 ),
					GAMERESOURCEMAN->GetGameText( 301 ), 0, GAMEUI->GetQuestWindow() );
				break;
			case ERROR_QUEST_DEL_QUEST:		// 실패 - 잘못된 퀘스트
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 412 ),
					GAMERESOURCEMAN->GetGameText( 301 ), 0, GAMEUI->GetQuestWindow() );
				break;
			default:
				assert(0);
				break;
			}

			NetUnLock( eLOCK_QUESTDELETE );
			/// 초기화
			mCheckQuestIndex = ULONG_MAX;
		}
		break;
	case NM_QUEST_DELAUTO_RES:	/// 남은시간 없을때 자동삭제
		{
			/// 퀘스트 삭제 요청에 대한 결과
			MSG_RES_QUEST_DEL* pmsg = (MSG_RES_QUEST_DEL*)msg;

			/// 자동 삭제 결과 알림
			if( pmsg->ErrorCode == ERROR_QUEST_DELAUTO_SUCCESS )
			{
				/// 시간정보 삭제
				if( mCheckQuestIndex != ULONG_MAX )
				{
					DeleteQuestTime( mCheckQuestIndex );

					cQuestDefine* define = GetQuestDefine( mCheckQuestIndex );
					if( define )
					{
						cStringT str;
						str.Format( GAMERESOURCEMAN->GetGameText( 415 ), GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex ));
						CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, (LPCTSTR)str.Cstr() );

						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
							eMBEVENT_NONE, eSKIN_OK,
							(LPCTSTR)str.Cstr(),
							GAMERESOURCEMAN->GetGameText( 301 )
							);
					}
					else
						assert(0);

					if( mChaseQuestIndex == mCheckQuestIndex )
						mChaseQuestIndex = 0;
				}
				else
					assert(0);

				cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
				if( npcWin && npcWin->IsVisible() == true )
					npcWin->Hide();

				/// 퀘스트 실패 효과
				HERO->SetLinkdEffect( eLINK_FOOT, "./Data/Effect/Eff_Mon_Regen_01.nif", 0 );
			}
			else
			{
				assert(0);
			}

			NetUnLock( eLOCK_QUESTDELETEAUTO );
			/// 초기화
			mCheckQuestIndex = ULONG_MAX;
		}
		break;
	case NM_QUEST_ADDBYITEM_RES:
		{
			/// 의뢰아이템으로 퀘스트 부여
			MSG_RES_QUEST_ADDBYITEM* pmsg = (MSG_RES_QUEST_ADDBYITEM*)msg;

			switch( pmsg->ErrorCode )
			{
			case ERROR_QUEST_ADD_SUCCESS:	// 성공 - 의뢰아이템 사용
				{
					sQuestList* table = &pmsg->table;
					sQuestList* quest = mQuest + mTotalKeepCount;

					/// 저장
					(*quest) = (*table);
					mTotalKeepCount++;

					UpdateUI();

					/// 각 맵에 포함된 퀘스트 갯수 갱신
					CheckCountMap();

					/// 새로 추가된 퀘스트를 기본선택 처리
					AddQuestSelect( table->questIdx, true );                 

					//
					mChaseQuestIndex = table->questIdx;
					UpdateChaseQuest( mChaseQuestIndex );

					cQuestDefine* define = GetQuestDefine( quest->questIdx );
					if( define )
					{
						LPCTSTR title = GAMERESOURCEMAN->GetQuestTitle( define->mTitleIndex );

						cStringT str;
						str.Format( GAMERESOURCEMAN->GetGameText( 400 ), title );
						CHATMANAGER->AddSystemMsg( eSYSTEM_QUEST, (LPCTSTR)str.Cstr() );
					}

					/// 시간정보 저장
					if( quest->status == eQUEST_PLAYING )
						AddQuestTime( quest->questIdx, quest->restTime );

					// 창 열기
					cUIWindow* win = (cUIWindow*)GAMEUI->GetQuestWindow();
					if( win && win->IsVisible() == false )
						win->Show();
				}
				break;
			case ERROR_QUEST_ADD_FAIL:	// 실패 - 퀘스트
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 401 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_NOTYET:	// 실패 - DB 사용중
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 401 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_EXIST:		// 실패 - 이미 해당 퀘스트를 가지고있음
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 402 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_COMPLETE:	// 실패 - 이미 완료한 퀘스트
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 403 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_TRIGGER:		// 실패 - 퀘스트 받을 조건이 안됨
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 404 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_MAX:			// 실패 - 퀘스트 목록이 FULL 
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 405 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_ITEM:		// 실패 - 퀘스트 제공아이템 받을수 없음
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 409 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_TIME:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 437 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_MONEY:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 438 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_POINT:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 439 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_ITEMFAIL:	// 실패 - 아이템
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 225 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_COOLTIME:	// 실패 - 아이템 쿨타임
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 226 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_TUTORIALMODE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 417 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			case ERROR_QUEST_ADD_MAX_ITEM:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 220 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
            case ERROR_QUEST_ADD_LICENSE:
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 293 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, (cUIWindow*)GAMEUI->GetInventoryWindow() );
				break;
			default:
				assert(0);
				break;
			}

			/// 퀘스트 언락
			NetUnLock( eLOCK_QUESTADDBYITEM );
		}
		break;
	case NM_QUEST_NEWLIST_SYN:
		{
			// 신규 퀘스트 목록
			MSG_SYN_NEWQUESTLIST* pmsg = (MSG_SYN_NEWQUESTLIST*)msg;

			// 초기화
			mNewQuestSet.Clear();
			mNewEventQuestSet.Clear();

			// 신규 퀘스트 목록 저장
			// - 이벤트용과 아닌것을 구분한다
			if( pmsg->rowCount > 0 )
			{
				for( unsigned short i = 0; i < pmsg->rowCount; ++i )	
				{
					unsigned long questIndex = pmsg->questIdxList[i];
					cQuestDefine* define = GetQuestDefine( questIndex );
					if( define )
					{
						// 일반
						if( define->mEventType == eQUEST_MAX )
						{
							mNewQuestSet.Insert( questIndex );
						}
						// 이벤트용
						else
						{
							mNewEventQuestSet.Insert( questIndex );
						}
					}
					else
						assert(0);
				}
			}

			// npc들의 상태를 갱신한다
			UpdateNpcStatus();

			// 퀘스트 관련 UI 갱신
			if( HERO->GetNpcFunc() == NPC_FUNC_BOARD )
			{
				SetBoardQuestList();
			}
		}
		break;
	case NM_QUEST_NEWLISTEVENT_SYN:
		{
			// 이벤트용 신규 퀘스트 목록
			MSG_SYN_NEWQUESTLIST* pmsg = (MSG_SYN_NEWQUESTLIST*)msg;

			// 초기화
			mNewEventQuestSet.Clear();

			// 신규 이벤트 퀘스트 목록 저장
			if( pmsg->rowCount > 0 )
			{
				for( unsigned short i = 0; i < pmsg->rowCount; ++i )	
				{
					unsigned long questIndex = pmsg->questIdxList[i];
					cQuestDefine* define = GetQuestDefine( questIndex );
					if( define )
					{
						// 이벤트용
						if( define->mEventType != eQUEST_MAX )
						{
							mNewEventQuestSet.Insert( questIndex );
						}
						else
							assert(0);	
					}
					else
						assert(0);
				}
			}

			// npc들의 상태를 갱신한다
			UpdateNpcStatus();

			// 퀘스트 관련 UI 갱신
			if( HERO->GetNpcFunc() == NPC_FUNC_BOARD )
			{
				SetBoardQuestList();
			}
		}
		break;
	case NM_QUEST_NEXT_LEVEL_NEWLIST_SYN:
	case NM_QUEST_NEXT_LEVEL_NEWLISTEVENT_SYN:
		{
			// 고레벨 퀘스트 목록
			MSG_SYN_NEXT_LEVEL_NEWQUESTLIST* pmsg = (MSG_SYN_NEXT_LEVEL_NEWQUESTLIST*)msg;

			typedef tHashSet<unsigned long> cQuestSet;
			cQuestSet tempQuestSet;
			
			// 고레벨 퀘스트 목록 저장
			for( unsigned short i = 0; i < pmsg->rowCount; ++i )	
			{
				unsigned long questIndex = pmsg->questIdxList[i];
				cQuestDefine* define = GetQuestDefine( questIndex );
				if( define )
				{
					tempQuestSet.Insert( questIndex );	
				}
				else
					assert(0);
			}
			

			/// 각 npc 상태 정보 갱신
			cNpcStatusMap::cIterator i = mNpcStatusMap.Begin();
			cNpcStatusMap::cIterator end = mNpcStatusMap.End();
			for( ; i != end; ++i )
			{
				unsigned long npcIndex = (unsigned long)(i->mFirst);
				sNpcStausData* p	   = (sNpcStausData*)(i->mSecond);
				if( p )
				{
					// 일반 npc 일때만 퀘스트 상태 처리
					sNPCList* npcInfo = NPCSCRIPT->GetNPCList( p->npcClassIndex );
					if( npcInfo && npcInfo->mType <= eNPCTYPE_TUTORIALBOOK )
					{
						sNpcQuestList* list = GetNpcQuestMap( p->npcClassIndex );
						if( !list )
							continue;

						/// npc 상태가 없거나 저레벨 퀘스트 상태일때만 고레벨 퀘스트 이펙트 표시가 가능하다(다른 상태에서는 우선순위에서 밀림)
						if( p->effectType == eNPCQUEST_EFFECT_NONE || p->effectType == eNPCQUEST_EFFECT_LOW_LEVEL_NEW )
						{
							cQuestSet::cIterator s = list->mQuestSet.Begin();
							cQuestSet::cIterator send = list->mQuestSet.End();
							for( ; s != send; ++s )
							{
								unsigned long newIndex = (unsigned long)(*s);

								// 고레벨 퀘스트 목록 중 해당 npc가 제공하는 퀘스트가 있으면
								if( tempQuestSet.Find( newIndex ) != tempQuestSet.End() )
								{
									p->effectType = eNPCQUEST_EFFECT_NEXT_LEVEL_NEW;
									
									/// NPC 갱신
									cNpc* pNpc = OBJECTMANAGER->GetNpc( npcIndex );
									if( pNpc )
									{
										pNpc->SetQuestEffect( eNPCQUEST_EFFECT_NEXT_LEVEL_NEW );		
									}
									
									/// 미니맵 갱신
									cSmallMiniMapWindow* smini = GAMEUI->GetSmallMinimapWindow();
									cMiniMapWindow* mini = GAMEUI->GetMinimapWindow();
									if( smini && mini )	
									{
										smini->UpdateNpcEffect( npcIndex, eNPCQUEST_EFFECT_NEXT_LEVEL_NEW );
										mini->UpdateNpcEffect( npcIndex, eNPCQUEST_EFFECT_NEXT_LEVEL_NEW );
									}
								}
							}
						}
						else
							continue;
					}
				}
				else
					assert(0);
			}
		
		}
		break;
	default:
		assert(0);
		break;
	}
}

/// 갱신
void cQuestManager::ActiveQuestText( sQuestList* oldQuest, sQuestList* newQuest )
{
	if( !oldQuest || !newQuest )
	{
		assert(0);
		return;
	}

	if( oldQuest->questIdx != newQuest->questIdx )
	{
		assert(0);
		return;
	}

	/// 퀘스트 정보
	cQuestDefine* define = GetQuestDefine( oldQuest->questIdx );
	if( !define )
	{
		assert(0);
		return;
	}

	cQuestDuty* duty = define->mQuestDuty;
	if( !duty )
		return;

	/// 달라진 내용을 출력한다
	if( oldQuest->check == newQuest->check )
		return;

	__int64 changedata = newQuest->check - oldQuest->check;
	__int64 end = 0xFF;

	cStringT str;

	/// 바뀐데이터의 바뀐 부분을 출력한다
	for( unsigned int i = 0; i < MAX_DUTY; ++i )
	{
		eDutyType type = duty->mDuty[i].dutyType;
		unsigned long targetIdx = duty->mDuty[i].targetIdx;
		unsigned int count = duty->mDuty[i].count;
		unsigned long skillIndex = duty->mDuty[i].skillIndex;

		/// 의무가 없는게 나오면 뒤에도 없으므로 끝
		if( type == eDUTY_NONE )
			break;

		/// 갱신이 된 내용만 출력
		if( (changedata & end) > 0 )
		{
			switch( type )
			{
			case eDUTY_HUNT:
				{
					LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );
					str.Format( GAMERESOURCEMAN->GetGameText( 451 ), monName, count );

					/// 완료
					if( (newQuest->check & end) == (define->mComplete & end) )
					{
						str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
					}
					/// 진행중
					else
					{
						__int64 cnt = (newQuest->check & end);
						for( unsigned int b = 0; b < i; ++b)
							cnt = cnt >> 8;

						cStringT str1;
						str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

						str += str1;
					}

					HERO->ActiveQuestText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
				}
				break;
			case eDUTY_COLLECT:
				{
					LPCTSTR itemName = ITEMMAN->GetItemName( targetIdx );
					str.Format( GAMERESOURCEMAN->GetGameText( 453 ), itemName, count );

					/// 완료
					if( (newQuest->check & end) == (define->mComplete & end) )
						str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
					/// 진행중
					else
					{
						__int64 cnt = (newQuest->check & end);
						for( unsigned int b = 0; b < i; ++b)
							cnt = cnt >> 8;

						cStringT str1;
						str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

						str += str1;
					}

					HERO->ActiveQuestText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
				}
				break;
			case eDUTY_ACTIVESKILL:
				{
					LPCTSTR skillName = GAMERESOURCEMAN->GetSkillNameText( skillIndex );
					LPCTSTR monName = GAMERESOURCEMAN->GetMonsterName( targetIdx );

					str.Format( GAMERESOURCEMAN->GetGameText( 456 ), skillName, monName );

					/// 완료
					if( (newQuest->check & end) == (define->mComplete & end) )
					{
						str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
					}
					
					HERO->ActiveQuestText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
				}
				break;
			case eDUTY_ENHANCE:
				{
					str.Format( GAMERESOURCEMAN->GetGameText( 457 ) );

					/// 완료
					if( (newQuest->check & end) == (define->mComplete & end) )
					{
						str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
					}

					HERO->ActiveQuestText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
				}
				break;
			case eDUTY_PUTCARD:
				{
					str.Format( GAMERESOURCEMAN->GetGameText( 458 ) );

					/// 완료
					if( (newQuest->check & end) == (define->mComplete & end) )
					{
						str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
					}

					HERO->ActiveQuestText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
				}
				break;
			case eDUTY_PVPJOIN:
			case eDUTY_PVPWIN:
			case eDUTY_PVPLOSE:
				{
					unsigned int pvpIndex = 0;
					switch( targetIdx )
					{
					case MAP_DEATHMATCH:		pvpIndex = 859;			break;
					case MAP_FLAG:				pvpIndex = 860;			break;
					}

					unsigned int textIndex = 0;
					switch( type )
					{
					case eDUTY_PVPJOIN:		textIndex = 465;			break;
					case eDUTY_PVPWIN:		textIndex = 466;			break;
					case eDUTY_PVPLOSE:		textIndex = 467;			break;
					}

					str.Format( GAMERESOURCEMAN->GetGameText( textIndex ), GAMERESOURCEMAN->GetGameText( pvpIndex ), count );

					/// 완료
					if( (newQuest->check & end) == (define->mComplete & end) )
					{
						str.Append( GAMERESOURCEMAN->GetGameText( 445 ) );
					}
					/// 진행중
					else
					{
						__int64 cnt = (newQuest->check & end);
						for( unsigned int b = 0; b < i; ++b)
							cnt = cnt >> 8;

						cStringT str1;
						str1.Format( GAMERESOURCEMAN->GetGameText( 459 ), (unsigned int)cnt, count );

						str += str1;
					}

					HERO->ActiveQuestText( (LPCTSTR)str.Cstr(), COLOR_WHITE );
				}
				break;
			}
		}

		/// 완료 체크용
		end = end << 8;
	}
}

/// 남은시간 획득
long cQuestManager::GetQuestRestTime( unsigned long questIdx )
{
	sQuestTime* p = (sQuestTime*)mTimeMap.GetAt( questIdx );
	return ( p ) ? p->restTime : 0;
}

/// 퀘스트 남은시간저장
void cQuestManager::AddQuestTime( unsigned long questIdx, unsigned long restTime )
{
	cQuestDefine* define = GetQuestDefine( questIdx );
	if( !define )
		return;

	/// 시간체크용 퀘스트만 저장
	cQuestLimit* limit = define->mQuestLimit;
	if( limit && limit->mTimeType != eTIME_NONE )
	{
		sQuestTime* p = new sQuestTime;
		p->restTime = restTime;
		p->startTime = THEAPP->GetWorldAccumTime();

		/// 시간맵에 저장
		if( mTimeMap.Insert( questIdx, p ) == false )
		{
			assert(0);
			return;
		}
	}
}

/// 삭제
void cQuestManager::DeleteQuestTime( unsigned long questIdx )
{
	sQuestTime* p = (sQuestTime*)mTimeMap.GetAt( questIdx );
	if( p )
	{
		mTimeMap.Erase( questIdx );
		SAFE_DELETE( p );
	}
}

void cQuestManager::Process( unsigned long accumTime )
{
	if( mTimeMap.GetSize() == 0 )
		return;

	/// 시간 체크
	cTimeMap::cIterator i = mTimeMap.Begin();
	cTimeMap::cIterator end = mTimeMap.End();
	for( ; i != end; )
	{
		sQuestTime* p = (sQuestTime*)(i->mSecond);
		unsigned long questIdx = (unsigned long)(i->mFirst);
		if( !p )
			continue;

		++i;

		long elasedTime = accumTime - p->startTime;
		
		/// 데이터 갱신
		p->restTime -= elasedTime;
		p->startTime = accumTime;

		if( p->restTime < 0 )
			p->restTime = 0;

		/// 한번만 쏘도록 조절
		if( p->restTime <= 0 && STAGEMAN->GetCurrentStage() == eStage_Game )
		{
			int arrIdx = GetQuestArrIdx( questIdx );
			if( arrIdx > -1 )
				QuestDeleteAuto( arrIdx );
		}
	}
}

bool cQuestManager::IsSelectQuest( unsigned long questIdx )
{
	cSelectSet::cIterator i = mSelectSet.Find( questIdx );
	return ( i == mSelectSet.End() ) ? false : true;
}

bool cQuestManager::IsCompleteGathering( unsigned long questIndex )
{
	cQuestDefine* define = GetQuestDefine( questIndex );
	if( !define )
	{
		assert(0);
		return true;
	}

	cQuestDuty* duty = define->mQuestDuty;
	if( !duty )
	{
		assert(0);
		return true;
	}

	int arrIdx = GetQuestArrIdx( questIndex );
	if( arrIdx == -1 )
		return true;

	/// 퀘스트 자체가 완료되었으면 리턴
	if( mQuest[arrIdx].check == define->mComplete )
		return true;

	/// 채집 관련부분 완료되었는지 검사
	__int64 mask = 0x00;
	for( unsigned int i = 0; i < MAX_DUTY; ++i )
	{
		eDutyType type = duty->mDuty[i].dutyType;

		/// 의무가 없는게 나오면 뒤에도 없으므로 끝
		if( type == eDUTY_NONE )
			break;
	
		if( type == eDUTY_COLLECT )
		{
			__int64 bit = 0xFF << ( i * 8 );
			mask = mask | bit;
		}
	}

	/// 모든 채집 조건을 완료하였으면
	if( (mQuest[arrIdx].check & mask) == (define->mComplete & mask) )
		return true;
	return false;
}

void cQuestManager::AddQuestSelect( unsigned long questIndex, bool setQuestWindow )
{
	int arrIdx = GetQuestArrIdx( questIndex );

	if( arrIdx == -1 )
		return;

	mSelectSet.Insert( questIndex );

	/// UI 세팅
	cQuestSimpleWindow* simple = GAMEUI->GetQuestSimpleWindow();
	if( simple )
		simple->AddQuestInfo( questIndex, &mQuest[arrIdx].check );

	/// 체크 세팅
	if( setQuestWindow == true )
	{
		cQuestWindow* win = GAMEUI->GetQuestWindow();
		if( win )
			win->SetCheck( arrIdx, questIndex, true );
	}
}

void cQuestManager::DeleteQuestSelect( unsigned long questIndex, bool setQuestWindow )
{
	int arrIdx = GetQuestArrIdx( questIndex );

	if( arrIdx == -1 )
		return;

	mSelectSet.Erase( questIndex );

	/// UI 세팅
	cQuestSimpleWindow* simple = GAMEUI->GetQuestSimpleWindow();
	if( simple )
		simple->DeleteQuestInfo( questIndex );

	/// 체크 풀기
	if( setQuestWindow == true )
	{
		cQuestWindow* win = GAMEUI->GetQuestWindow();
		if( win )
			win->SetCheck( arrIdx, questIndex, false );
	}
}

/// 각 맵별로 퀘스트 총 갯수 검사
void cQuestManager::CheckCountMap()
{
	mCountMap.Clear();

	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		cQuestDefine* define = GetQuestDefine( mQuest[i].questIdx );
		if( !define )
		{
			assert(0);
			continue;
		}

		/// 이미 갯수를 세어놓았으면 더하기
		cCountMap::cIterator b = mCountMap.Find( define->mTakeMapIndex );
		if( b != mCountMap.End() )
		{
			/// 있으면 더하기
			unsigned int& total = (unsigned int)(b->mSecond);
			total += 1;
		}
		else
		{
			/// 없으면 새로삽입
			if( mCountMap.Insert( define->mTakeMapIndex, 1 ) == false )
				assert(0 && "error to insert quest count");
		}
	}
}

void cQuestManager::UpdateChaseQuest( unsigned long questIndex )
{
	if( questIndex == 0 )
		return;

	cQuestDefine* define = GetQuestDefine( questIndex );
	if( !define )
	{
		assert(0);
		return;
	}

	mChaseQuestIndex = questIndex;

	// UI 갱신
	cQuestSimpleWindow* simple = GAMEUI->GetQuestSimpleWindow();
	if( simple )
	{
		simple->UpdateSelect();
	}

	cQuestWindow* questWin = GAMEUI->GetQuestWindow();
	if( questWin )
	{
		questWin->UpdateSelect();
	}

	cMiniMapWindow* minimap = GAMEUI->GetMinimapWindow();
	if( minimap )
	{
		minimap->UpdateQuestChase();
	}
}

void cQuestManager::ClearChaseQuest()
{
	mChaseQuestIndex = 0;

	// UI 갱신
	cQuestSimpleWindow* simple = GAMEUI->GetQuestSimpleWindow();
	if( simple )
	{
		simple->UpdateSelect();
	}

	cQuestWindow* questWin = GAMEUI->GetQuestWindow();
	if( questWin )
	{
		questWin->UpdateSelect();
	}

	cMiniMapWindow* minimap = GAMEUI->GetMinimapWindow();
	if( minimap )
	{
		minimap->UpdateQuestChase();
	}
}

unsigned int cQuestManager::GetCount( unsigned short mapIdx )
{
	cCountMap::cIterator i = mCountMap.Find( mapIdx );
	return ( i == mCountMap.End() ) ? 0 : (unsigned int)(i->mSecond);
}

void cQuestManager::NetLock( int lockTry, unsigned short slotIndex0 )
{
	switch( lockTry )
	{
	case eLOCK_QUESTADD:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetLock( eLOCK_QUESTADD );

			cBoardWindow* boardWin = GAMEUI->GetBoardWindow();
			if( boardWin )
				boardWin->NetLock( eLOCK_QUESTADD );

			ITEMMAN->LockItemTry( ITEM_TRY_QUEST_REWARDTAKE );
		}
		break;
	case eLOCK_QUESTADDBYITEM:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetLock( eLOCK_QUESTADDBYITEM );

			ITEMMAN->LockItemTry( ITEM_TRY_QUEST_ADDBYITEM, slotIndex0 );
		}
		break;
	case eLOCK_QUESTCOMPLETE:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetLock( eLOCK_QUESTCOMPLETE );

			ITEMMAN->LockItemTry( ITEM_TRY_QUEST_REWARDTAKE );
		}
		break;
	case eLOCK_QUESTREWARD:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetLock( eLOCK_QUESTREWARD );

			ITEMMAN->LockItemTry( ITEM_TRY_QUEST_REWARDTAKE );
		}
		break;
	case eLOCK_QUESTDELETE: 
		{
			cQuestWindow* questWin = GAMEUI->GetQuestWindow();
			if( questWin )
				questWin->NetLock( eLOCK_QUESTDELETE );

			ITEMMAN->LockItemTry( ITEM_TRY_QUEST_TAKE );
		}
		break;
	case eLOCK_QUESTDELETEAUTO:		/// 맵로딩 중에 요청이 들어올수 있으므로 
		if( STAGEMAN->GetCurrentStage() == eStage_Game )
		{
			cQuestWindow* questWin = GAMEUI->GetQuestWindow();
			if( questWin )
				questWin->NetLock( eLOCK_QUESTDELETEAUTO );

			ITEMMAN->LockItemTry( ITEM_TRY_QUEST_TAKE );
		}
		break;
	default:
		assert(0);
		return;
	}

	mNetLock = true;
	mLockTry = lockTry;
}

void cQuestManager::NetUnLock( int lockTry )
{
	if( mNetLock == false )
		return;

	if( mLockTry != lockTry )
	{
		assert(0);
		return;
	}

	switch( lockTry )
	{
	case eLOCK_QUESTADD:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetUnLock( eLOCK_QUESTADD );

			cBoardWindow* boardWin = GAMEUI->GetBoardWindow();
			if( boardWin )
				boardWin->NetUnLock( eLOCK_QUESTADD );

			ITEMMAN->UnlockItemTry( ITEM_TRY_QUEST_REWARDTAKE );
		}
		break;
	case eLOCK_QUESTADDBYITEM:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetUnLock( eLOCK_QUESTADDBYITEM );

			ITEMMAN->UnlockItemTry( ITEM_TRY_QUEST_ADDBYITEM );
		}
		break;
	case eLOCK_QUESTCOMPLETE:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetUnLock( eLOCK_QUESTCOMPLETE );

			ITEMMAN->UnlockItemTry( ITEM_TRY_QUEST_REWARDTAKE );
		}
		break;
	case eLOCK_QUESTREWARD:
		{
			cNpcWindow* npcWin = GAMEUI->GetNpcWindow();
			if( npcWin )
				npcWin->NetUnLock( eLOCK_QUESTREWARD );

			ITEMMAN->UnlockItemTry( ITEM_TRY_QUEST_REWARDTAKE );
		}
		break;
	case eLOCK_QUESTDELETE:	
		{
			cQuestWindow* questWin = GAMEUI->GetQuestWindow();
			if( questWin )
				questWin->NetUnLock( eLOCK_QUESTDELETE );

			ITEMMAN->UnlockItemTry( ITEM_TRY_QUEST_TAKE );
		}
		break;
	case eLOCK_QUESTDELETEAUTO:		/// 맵로딩 중에 요청이 들어올수 있으므로
		if( STAGEMAN->GetCurrentStage() == eStage_Game )
		{
			cQuestWindow* questWin = GAMEUI->GetQuestWindow();
			if( questWin )
				questWin->NetUnLock( eLOCK_QUESTDELETEAUTO );

			ITEMMAN->UnlockItemTry( ITEM_TRY_QUEST_TAKE );
		}
		break;
	default:
		assert(0);
		return;
	}

	mLockTry = eLOCK_QUESTNONE;
	mNetLock = false;
}
