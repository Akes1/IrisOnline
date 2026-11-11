#include "StdAfx.h"
#include "gamesrv.h"

#include "Protocol.h"

#include "DropScript.h"
#include "Drop.h"

#include "Monster.h"
#include "ObjectManager.h"
#include "AppTimer.h"
#include "Monster.h"
#include "Player.h"
#include "MonsterScript.h"
#include "item.h"
#include "TakeDamagePool.h"
#include "Gathering.h"
#include "logProtocol.h"
#include "AIManager.h"
#include "Party.h"
#include "PartyManager.h"
#include "ItemManager.h"
#include "GridManager.h"
#include "RandomTable.h"
#include "SkillManager.h"
#include "Twitter_Common.h"
#include "TwitterScript.h"

cDrop* cDrop::mpDrop = NULL;



cDrop::cDrop(void)
{
	/// 싱글톤
	if( mpDrop )
	{
		Verbose->WriteLog( "Drop new error");
	}
	else
	{
		mpDrop = this;
	}
}



cDrop::~cDrop(void)
{
}



bool cDrop::Init()
{

	mLogWriteTime = DROP_LOG_WRITE_NORMAL;

	mpDropScript = new cDropScript;
	if( mpDropScript->Init() == false )
	{
		assert(NULL);
		return false;
	}	

	/// 몬스터 드랍정보 통계
	const cPHashMap* pMonsterMap = MONSTERSCRIPT->GetMonsterMap();
	if( pMonsterMap != NULL )
	{
		cPHashMap::cConstIterator i = pMonsterMap->Begin();
		cPHashMap::cConstIterator e = pMonsterMap->End();

		for( ; i != e ; ++i )
		{
			unsigned long monsterIdx = (*i).mFirst;
			const sMonsterScript* pScript = (sMonsterScript*)(*i).mSecond;

			if( pScript == NULL )
				continue;

			sDropItemLog* pLog = new sDropItemLog;
			pLog->mMonIdx = monsterIdx;

			if( mDropLogMap.Insert( monsterIdx, pLog ) == false )
			{
				SAFE_DELETE( pLog );
			}
		}
	}
	
	mQuestSeed = RANDOMTABLE->GetSeedx100();

	return true;
}



void cDrop::Release()
{
	unsigned long maxSize = MAX_UDP_PACKET_SIZE;
	if( maxSize > MAX_IO_CONTEXT_BUFFER_LEN )
		maxSize = MAX_IO_CONTEXT_BUFFER_LEN ;

	/// 메세지에 담을 내용을 메세지 사이즈보다 크지 않게 설정
	unsigned long maxCnt = 0;
	maxCnt = ( maxSize - sizeof( DROPITEM_EVENT ) ) / sizeof(sDropItem);

	/// 종료시점이라 메모리 단편화 부분 무시
	/// ※ iocontextpool이 이미 제거되 pool을 못얻어옴
	sDropItem* pDropItem = new sDropItem[maxCnt];
	unsigned long count = 0;

	cPHashMap::cIterator i = mDropLogMap.Begin();
	cPHashMap::cIterator e = mDropLogMap.End();
	for( ; i != e ; ++i )
	{
		unsigned long monsterIdx = (*i).mFirst;
		sDropItemLog* pLog = (sDropItemLog*)(*i).mSecond;
		if( pLog == NULL )
			continue;

		/// 죽은 적이 없는 몬스터 통과
		if( pLog->mMonDieCnt == 0 )
		{
			SAFE_DELETE( pLog );
			continue;
		}

		/// 아이템 정보 없이 몬스터 죽은 정보만 기록할지 체크
		bool dropItemMap = true;
		bool dropItemNoMap = true;

		/// 드랍했던 아이템 정보 로그패킷에 기록
		cHashMap* pMap = &pLog->mItemMap;
		if( pMap != NULL )
		{
			cHashMap::cIterator mi = pMap->Begin();
			cHashMap::cIterator me = pMap->End();
			for( ; mi != me ; ++mi )
			{
				unsigned long itemIdx = (*mi).mFirst;
				unsigned long itemCnt = (*mi).mSecond;

				if( itemCnt == 0 )
					continue;

				pDropItem[count].mMonsterIdx = monsterIdx;
				pDropItem[count].mDieCnt = pLog->mMonDieCnt;
				pDropItem[count].mMoneyCnt = pLog->mMonMoneyCnt;
				pDropItem[count].mItemIdx = itemIdx;
				pDropItem[count].mItemCnt = itemCnt;
				pDropItem[count].mDropType = 0;		/// 선정후 드랍함

				dropItemMap = false;

				++count;
				if( count >= maxCnt )
					break;
			}

			//SAFE_DELETE( pMap );
		}
		else
			dropItemMap = false;

		/// 선정은 됐으나 드랍하지 않은 아이템 정보 로그패킷에 기록
		cHashMap* pNoMap = &pLog->mItemNoDropMap;
		if( pNoMap != NULL )
		{
			cHashMap::cIterator mi = pNoMap->Begin();
			cHashMap::cIterator me = pNoMap->End();
			for( ; mi != me ; ++mi )
			{
				unsigned long itemIdx = (*mi).mFirst;
				unsigned long itemCnt = (*mi).mSecond;

				if( itemCnt == 0 )
					continue;

				pDropItem[count].mMonsterIdx = monsterIdx;
				pDropItem[count].mDieCnt = pLog->mMonDieCnt;
				pDropItem[count].mMoneyCnt = pLog->mMonMoneyCnt;
				pDropItem[count].mItemIdx = itemIdx;
				pDropItem[count].mItemCnt = itemCnt;
				pDropItem[count].mDropType = 1;		/// 선정후 제약조건에 걸려 드랍안함

				dropItemNoMap = false;

				++count;
				if( count >= maxCnt )
					break;
			}

			//SAFE_DELETE( pNoMap );
		}
		else
			dropItemNoMap = false;

		/// 아이템 드랍이 없는 몬스터
		if( dropItemMap == true && dropItemNoMap == true )
		{
			pDropItem[count].mMonsterIdx = monsterIdx;
			pDropItem[count].mDieCnt = pLog->mMonDieCnt;
			pDropItem[count].mMoneyCnt = pLog->mMonMoneyCnt;
			pDropItem[count].mItemIdx = 0;
			pDropItem[count].mItemCnt = 0;

			++count;
		}

		SAFE_DELETE( pLog );
	}

	/// 버퍼사이즈꽉 채운 경우 로그 기록
	if( count >= maxCnt )
	{
		NETWORK2->PostServerEvent("cDrop::Release count[%d] >= maxCnt[%d]", count, maxCnt );
	}

	NETWORK2->PostDropItemEvent( pDropItem, count );

	SAFE_DELETE_ARRAY( pDropItem );

	SAFE_RELEASE_DELETE( mpDropScript );
}


void cDrop::ProcessDrop( unsigned long /*elapsedTime*/, unsigned long accumTime )
{
	if( mLogWriteTime > accumTime )
		return;

	/// 다음 기록 시간설정
	mLogWriteTime = accumTime + DROP_LOG_WRITE_NORMAL;

	PerIoContext* perIoContext = NETWORK2->GetIoContext( IOCP_REQUEST_WRITE );
	sDropItem* pDropItem = (sDropItem*)perIoContext->buffer;

	unsigned long count = 0;

	cPHashMap::cIterator i = mDropLogMap.Begin();
	cPHashMap::cIterator e = mDropLogMap.End();

	for( ; i != e ; ++i )
	{
		unsigned long monsterIdx = (*i).mFirst;
		sDropItemLog* pLog = (sDropItemLog*)(*i).mSecond;
		if( pLog == NULL )
			continue;

		/// 죽은 적이 있는 몬스터만 기록
		if( pLog->mMonDieCnt == 0 )
			continue;

		/// 아이템 정보 없이 몬스터 죽은 정보만 기록할지 체크
		bool dropItemMap = true;
		bool dropItemNoMap = true;

		/// 드랍했던 아이템 정보 로그패킷에 기록
		cHashMap* pMap = &pLog->mItemMap;
		if( pMap != NULL )
		{
			/// 아이템 드랍 내역 확인
			/// 한몬스터에 해당하는 아이템 목록은 count가 최대치를 넘어서더라도 다 기록한다. 
			/// ※ 최대치는 위항목을 반영해 설정한다.
			cHashMap::cIterator mi = pMap->Begin();
			cHashMap::cIterator me = pMap->End();
			for( ; mi != me ; ++ mi )
			{
				unsigned long itemIdx = (*mi).mFirst;
				unsigned long itemCnt = (*mi).mSecond;

				if( itemCnt == 0 )
					continue;

				pDropItem[count].mMonsterIdx = monsterIdx;
				pDropItem[count].mDieCnt = pLog->mMonDieCnt;
				pDropItem[count].mMoneyCnt = pLog->mMonMoneyCnt;
				pDropItem[count].mItemIdx = itemIdx;
				pDropItem[count].mItemCnt = itemCnt;
				pDropItem[count].mDropType = 0;		/// 선정후 드랍함

				dropItemMap = false;

				/// 아이템 겟수 초기화
				(*mi).mSecond = 0;
				
				++count;			
			}
		}
		else
			dropItemMap = false;

		/// 선정은 됐으나 드랍하지 않은 아이템 정보 로그패킷에 기록
		cHashMap* pNoMap = &pLog->mItemNoDropMap;
		if( pNoMap != NULL )
		{
			cHashMap::cIterator noI = pNoMap->Begin();
			cHashMap::cIterator noE = pNoMap->End();
			for( ; noI != noE ; ++ noI )
			{
				unsigned long itemIdx = (*noI).mFirst;
				unsigned long itemCnt = (*noI).mSecond;

				if( itemCnt == 0 )
					continue;

				pDropItem[count].mMonsterIdx = monsterIdx;
				pDropItem[count].mDieCnt = pLog->mMonDieCnt;
				pDropItem[count].mMoneyCnt = pLog->mMonMoneyCnt;
				pDropItem[count].mItemIdx = itemIdx;
				pDropItem[count].mItemCnt = itemCnt;
				pDropItem[count].mDropType = 1;		/// 선정후 제약조건에 걸려 드랍안함

				dropItemNoMap = false;

				/// 아이템 겟수 초기화
				(*noI).mSecond = 0;

				++count;			
			}
		}
		else
			dropItemNoMap = false;

		/// 아이템 드랍이 없는 몬스터
		if( dropItemMap == true && dropItemNoMap == true )
		{
			pDropItem[count].mMonsterIdx = monsterIdx;
			pDropItem[count].mDieCnt = pLog->mMonDieCnt;
			pDropItem[count].mMoneyCnt = pLog->mMonMoneyCnt;
			pDropItem[count].mItemIdx = 0;
			pDropItem[count].mItemCnt = 0;
			pDropItem[count].mDropType = 0;

			++count;
		}

		/// 초기화
		pLog->mMonDieCnt = 0;
		pLog->mMonMoneyCnt = 0;

		/// 
		if( count >= DROP_LOG_WRITE_PROCESS_CNT )
		{
			mLogWriteTime = accumTime + DROP_LOG_WRITE_CONTINUE;
			break;
		}
	}

	if( count != 0 )
		NETWORK2->PostDropItemEvent( pDropItem, count );

	perIoContext->offset = sizeof( sDropItem ) * count;

	if( perIoContext->offset > 40000 )
	{
		NETWORK2->PostServerEvent( "cDrop::ProcessDrop perIoContext->offset[%d] > 40000", perIoContext->offset );
	}
	NETWORK2->ReleaseIoContext( perIoContext, "cDrop::ProcessDrop(" );
}


void cDrop::MonsterDrop( unsigned long monsterIdx, unsigned long playerIdx, unsigned long partyIdx )
{
	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL )  
	{ 
		assert(NULL);
		NETWORK2->PostServerEvent("drop monster idx error[%d]", monsterIdx );
		return; 
	}

	unsigned long monsterClassIdx = pMonster->GetRaceGender();

	sMonsterScript* pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( monsterClassIdx );
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("drop monsterScript idx error[%d]", monsterClassIdx );
		return;
	}

	sDropItemLog* pDropLog = (sDropItemLog*)mDropLogMap.GetAt( monsterClassIdx );
	if( pDropLog != NULL )
	{
		++pDropLog->mMonDieCnt;
	}

	//Verbose->WriteLog("몬스터죽음");

	/// 드랍 목록 임시공간 초기화
	mDropItemCntMap.Clear();
	mDropNoItemCntMap.Clear();

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;
	
	sGameOption* toOption1 =pPlayer->GetOptionData();	
	if( toOption1->option1.autoSendTwitterMsg == true )
	{
		DropTwitter( pMonster, playerIdx, partyIdx );
	}

	/// 저렙 몬스터를 잡은 경우에 대한 패널티 변수 선언..
	int div = 0;
	unsigned char level = pPlayer->GetLevel();
	cParty* pParty = PARTYMAN->GetParty( partyIdx );
	if( pParty )
	{
		unsigned long cnt = pParty->GetCount();
		unsigned long* userArr = pParty->GetUserArr();
		for( unsigned int i=0;i<cnt;i++ )
		{
			if( i >= MAX_PARTYUSER )
			{
				assert(0);
				break;
			}

			cPlayer* p = OBJECTMANAGER->GetPlayer( userArr[i] );
			if( p == 0 )
				continue;

			if( DropMonCheck( pMonster, p ) == false )
				continue;

			if( level < p->GetLevel() )
				level = p->GetLevel();
		}
	}
	div = pMonster->GetLevel() - level;

	/// money, exp, sxp 드랍
	DropMonsterValue( pMonster );

	/// 아이템 드랍
	float penalty = 1.0f;
	if( div < 0)
	{
		/// script
		bool isTheme = NETWORK2->GetServerType() == _E_ST_ID_THEME_;
		penalty = DROPSCRIPT->GetDropIncrease( isTheme, (char)div );
	}

	DropMonsterClass( monsterClassIdx, penalty );
	DropMonsterLevelType( pMonster->GetLevel(), 0, penalty );
	DropMonsterLevelType( pMonster->GetLevel(), pMonster->GetMonsterInfo()->mType, penalty );

	if( mDropNoItemCntMap.GetSize() != 0 )
	{
		cHashMap::cConstIterator begin = mDropNoItemCntMap.Begin();
		cHashMap::cConstIterator end = mDropNoItemCntMap.End();

		for( ; begin != end ; ++begin )
		{
			sDropItemLog* pDropLog = (sDropItemLog*)mDropLogMap.GetAt( monsterClassIdx );
			if( pDropLog != NULL )
			{
				cHashMap* pMap = (cHashMap*)&pDropLog->mItemNoDropMap;
				if( pMap != NULL )
				{
					unsigned long itemIdx = (*begin).mFirst;
					unsigned long itemCnt = (*begin).mSecond;
					cHashMap::cIterator find = pMap->Find( itemIdx );
					cHashMap::cIterator end = pMap->End();

					if( find != end )
						(*find).mSecond += itemCnt;
					else
						pMap->Insert( itemIdx, itemCnt );
				}			
			}
		}
	}

	if( mDropItemCntMap.GetSize() == 0 )
		return;

	unsigned long playerOwnerIdx = 0;
	unsigned long partyOwnerIdx[MAX_PARTYUSER] = {0,};
	unsigned char partyOwnerCnt = 0;

	/// 선공한 유저 파티 유/무 
	if( pParty == NULL )
	{
		/// 파티가 사라졌으면 선공유저에게 우선권을 준다.
		if( OBJECTMANAGER->GetPlayer( playerIdx ) != NULL )
		{
			playerOwnerIdx = playerIdx;
		}
	}
	else
	{
		switch( pParty->GetDivideType() )
		{
		case PARTY_DIVIDE_TURN:		/// 순서대로
			{
				playerOwnerIdx = pParty->GetTurnUser( pMonster->GetMapNumber(), pMonster->GetXPos(), pMonster->GetYPos() );
			}
			break;
		case PARTY_DIVIDE_RANDOM:	/// 랜덤
			{
				playerOwnerIdx = pParty->GetRandomUser( pMonster->GetMapNumber(), pMonster->GetXPos(), pMonster->GetYPos() );
			}
			break;
		case PARTY_DIVIDE_DAMAGE:	/// 데미지
			{
				unsigned long* pUser = pParty->GetUserArr();
				if( pUser == NULL )
					break;

				unsigned int partyCnt = pParty->GetCount();
				unsigned long maxDamagePlayerIdx = playerIdx;
				unsigned long maxDamage = 0;
				unsigned long tempDamage = 0;

				for( unsigned int i = 0; i < partyCnt; ++i )
				{
					cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pUser[i] );
					if( pPlayer )
					{
						/// 조건 체크
						if( DropMonCheck( pMonster, pPlayer ) == false )
							continue;

						tempDamage = pMonster->GetTakeDamage( pUser[i] );
						if( tempDamage > maxDamage )
						{
							maxDamagePlayerIdx = pUser[i];
							maxDamage = tempDamage;
						}
					}
				}

				playerOwnerIdx = maxDamagePlayerIdx;
			}
			break;
        case PARTY_DIVIDE_LEADER:
            playerOwnerIdx = pParty->GetLeader();
            break;
		}

		unsigned long* pUser = pParty->GetUserArr();
		unsigned int partyCnt = pParty->GetCount();

		if( pUser != NULL )
		{
			for( unsigned int i = 0; i < partyCnt; ++i )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pUser[i] );
				if( pPlayer )
				{
					/// 조건 체크
					if( DropMonCheck( pMonster, pPlayer ) == false )
						continue;

					partyOwnerIdx[partyOwnerCnt] = pUser[i];
					++partyOwnerCnt;
				}
			}
		}
	}

	unsigned short applyCnt;
	bool isApply = false;

	unsigned itemIdx = ITEMMANAGER->MakeItemIdx( );
	cItem*   item    = OBJECTMANAGER->AddItem( itemIdx, pMonster );
	if ( item != NULL )
	{
		if( playerOwnerIdx != 0 )
			item->PushOwner( playerOwnerIdx );

        bool partyLeader = false;
        if ( pParty )
        {
            partyLeader = (pParty->GetDivideType() == PARTY_DIVIDE_LEADER) ? true : false;
        }

		for( unsigned char i = 0 ; i < partyOwnerCnt ; ++i )
			item->PushParty( partyOwnerIdx[i], partyLeader );

		cHashMap::cConstIterator begin = mDropItemCntMap.Begin();
		cHashMap::cConstIterator end = mDropItemCntMap.End();

		for( ; begin != end ; ++begin )
		{
			applyCnt = 0;

			if( (*begin).mSecond == 0 )
				NETWORK2->PostServerEvent("cDrop::Drop (*begin).mSecond(%d,%d) == 0", (*begin).mFirst, (*begin).mSecond );

			//Verbose->WriteLog("아이템드랍 [%d,%d]", (*begin).mFirst, (*begin).mSecond);

			/// 통계자료 출력용
			//printf("\n%d\t%d",(*begin).mFirst, (*begin).mSecond);
			
			cPlayer* owner = pPlayer;
			if( playerOwnerIdx != 0 )
			{
				// 소유자 기준
				owner = OBJECTMANAGER->GetPlayer( playerOwnerIdx );
			}

			if( owner )
			{
				unsigned long itemDefineIndex = (unsigned long)(*begin).mFirst;

				TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefineByIndex( itemDefineIndex );
				if( itemDefine && itemDefine->type == ITEM_ETC1 )
				{
					if( itemDefine->subType == ITEM_ETC1_QUEST )
					{
						// 퀘스트 시작아이템 생성 제한
						// 고유 번호로 검색
						TB_ITEM_ABILITY* itemAbility = ITEMMANAGER->GetItemAbility( itemDefine->idx );
						if( itemAbility && itemAbility->influence_idx > 0 )
						{
							unsigned long questIndex = itemAbility->influence_idx;
							if( owner->IsKeepGroupQuest( questIndex ) != -1 )
							{
								if( owner == pPlayer )
									continue;
								else
								{
									if( pPlayer->IsKeepGroupQuest( questIndex ) != -1)
										continue;
								}
							}

							if( owner->IsCompleteQuest( questIndex ) == true )
							{
								if( owner == pPlayer )
									continue;
								else
								{
									if( pPlayer->IsCompleteQuest( questIndex ) == true )
										continue;
								}
							}
						}	
					}
					else if( itemDefine->subType == ITEM_ETC1_TITLE )
					{
						// 호칭 생성 제한
						if( owner->IsHaveTitle( itemDefine->titleIndex ) == true )
						{
							if( owner == pPlayer )
								continue;
							else
							{
								if( pPlayer->IsHaveTitle( itemDefine->titleIndex ) == true )
									continue;
							}
						}
					}
				}
			}

			while( applyCnt < (*begin).mSecond )
			{
				unsigned short cnt = item->PushItem( (*begin).mFirst, (unsigned short)(*begin).mSecond );
				if( cnt == 0 )
				{
					NETWORK2->PostServerEvent("cDrop::Drop item(%d,%d)->PushItem cnt == 0", (*begin).mFirst, (*begin).mSecond);
					break;
				}

				applyCnt = applyCnt + cnt;
				isApply = true;
			}

			sDropItemLog* pDropLog = (sDropItemLog*)mDropLogMap.GetAt( monsterClassIdx );
			if( pDropLog != NULL )
			{
				cHashMap* pMap = (cHashMap*)&pDropLog->mItemMap;
				if( pMap != NULL )
				{
					unsigned long itemIdx = (*begin).mFirst;
					unsigned long itemCnt = (*begin).mSecond;
					cHashMap::cIterator find = pMap->Find( itemIdx );
					cHashMap::cIterator end = pMap->End();

					if( find != end )
						(*find).mSecond += itemCnt;
					else
						pMap->Insert( itemIdx, itemCnt );
				}			
			}
		}
	}

	if( isApply == true )
	{
		/// 몬스터의 현재 좌표에 떨어트릴수 없는 경우
		if( AIMANAGER->IsPossible( item->GetMapNumber(), item->GetXPos(), item->GetYPos(), item->GetObject() ) == false )
		{
			/// 첫번째 소유자한테 떨어트림
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerOwnerIdx );
			if( pPlayer != NULL && AIMANAGER->IsPossible( pPlayer->GetMapNumber(), pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->GetObject()  ) == true )
			{
				item->SetMapNumber( pPlayer->GetMapNumber() );
				item->SetPos( pPlayer->GetXPos(), pPlayer->GetYPos() );
			}
			else
			{
				for( unsigned char i = 0 ; i < partyOwnerCnt ; ++i )
				{
					/// 파티 소유자중 가능한 위치에 떨어트린다.
					pPlayer = OBJECTMANAGER->GetPlayer( partyOwnerIdx[i] );
					if( pPlayer != NULL && AIMANAGER->IsPossible( pPlayer->GetMapNumber(), pPlayer->GetXPos(), pPlayer->GetYPos(), pPlayer->GetObject()  ) == true )
					{
						item->SetMapNumber( pPlayer->GetMapNumber() );
						item->SetPos( pPlayer->GetXPos(), pPlayer->GetYPos() );
						break;
					}
				}
			}
		}
		GRIDMANAGER->AddItem( item );
	}
	else
		OBJECTMANAGER->RemoveItem( itemIdx );

	mDropItemCntMap.Clear();
	mDropNoItemCntMap.Clear();

	/// 통계자료 출력용
	//printf("\n-----------------------------------");
}


void cDrop::MonsterQuestDrop( unsigned long monsterIdx, unsigned long playerIdx, unsigned long partyIdx )
{

	cMonster* pMonster = OBJECTMANAGER->GetMonster( monsterIdx );
	if( pMonster == NULL )  
	{ 
		assert(NULL);
		NETWORK2->PostServerEvent("QuestDrop monster idx error[%d]", monsterIdx );
		return; 
	}

	unsigned long monsterClassIdx = pMonster->GetRaceGender();

	sMonsterScript* pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( monsterClassIdx );
	if( pMonsterScript == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("QuestDrop monsterScript idx error[%d]", monsterClassIdx );
		return;
	}

		/// 파티 유/무
	cParty* pParty = PARTYMAN->GetParty( partyIdx );
	if( pParty == NULL )
	{
		/// 파티가 사라졌으면 선공한 플레이어에게 드랍을 한다.
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
		if( pPlayer != NULL )
			DropMonQuestType( pMonster, pPlayer );
	}
	else
	{
		unsigned long* pUser = pParty->GetUserArr();
		unsigned int partyCnt = pParty->GetCount();

		if( pUser != NULL )
		{
			/// 파티원별 퀘스트 드랍
			for( unsigned int i = 0; i < partyCnt; ++i )
			{
				cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pUser[i] );
				if( pPlayer != NULL )
				{
					/// 조건 체크
					if( DropMonCheck( pMonster, pPlayer ) == false )
						continue;

					DropMonQuestType( pMonster, pPlayer );
				}
			}
		}
	}
}

void cDrop::DropTwitter( cMonster* pMonster, unsigned long playerIdx, unsigned long partyIdx )
{
	if( pMonster == NULL )
		return;

	unsigned long monsterClassIdx = pMonster->GetRaceGender();

	/// 트위터로 발송한 몬스터 목록인지 체크
	if( TWITTERSCRIPT->IsMonsterEvnet( monsterClassIdx ) == false )
		return;

	/// 몬스터 스크립트 정보
	sMonsterScript* pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( monsterClassIdx );
	if( pMonsterScript == NULL )
	{
		NETWORK2->PostServerEvent("cDrop::DropTwitter pMonsterScript[%d] == NULL", monsterClassIdx );
		return;
	}

	/// 몬스터를 공격한 데미지 맵
	TakeDamageRoot* pTakeDamageRoot = pMonster->GetTakeDamageRoot();
	if( pTakeDamageRoot == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDrop::DropTwitter pTakeDamageMap == NULL");
		return;
	}
	
	/// 선공한 유저 파티 유/무 
	cPlayer* pPlayer = NULL;
	cParty* pParty = PARTYMAN->GetParty( partyIdx );
	if( pParty == NULL )
	{
		/// 파티가 사라졌으면 선공유저에게만 알린다.
		pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
		if( pPlayer != NULL )
		{
			HANDLE handle  = NULL;
			MSG_SYN_TWITTER_MONSTER_DIE* sendMsg = (MSG_SYN_TWITTER_MONSTER_DIE*)NETWORK2->GetMsgRoot( &handle, 
				pPlayer->GetConnectionIdx(), NM_TWITTER, NM_TWITTER_MONSTER_DIE_SYN );
			if ( sendMsg != NULL )
			{
				sendMsg->mMonsterIdx = monsterClassIdx;
				NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_TWITTER_MONSTER_DIE) );
			}

			return;
		}
	}
	else
	{
		unsigned long* pUser = pParty->GetUserArr();
		if( pUser == NULL )
			return;

		unsigned int partyCnt = pParty->GetCount();
		for( unsigned int i = 0; i < partyCnt; ++i )
		{
			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pUser[i] );
			if( pPlayer )
			{
				/// 조건 체크
				if( DropMonCheck( pMonster, pPlayer ) == false )
					continue;

				HANDLE handle  = NULL;
				MSG_SYN_TWITTER_MONSTER_DIE* sendMsg = (MSG_SYN_TWITTER_MONSTER_DIE*)NETWORK2->GetMsgRoot( &handle, 
					pPlayer->GetConnectionIdx(), NM_TWITTER, NM_TWITTER_MONSTER_DIE_SYN );
				if ( sendMsg != NULL )
				{
					sendMsg->mMonsterIdx = monsterClassIdx;
					NETWORK2->SendMsgRoot( handle, sizeof(MSG_SYN_TWITTER_MONSTER_DIE) );
				}
			}
		}
	}
}

void cDrop::DropMonsterValue( cMonster* pMonster )
{
	if( pMonster == NULL )
		return;

	unsigned long monsterKind = pMonster->GetRaceGender();

	/// 몬스터 스크립트 정보
	sMonsterScript* pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( monsterKind );
	if( pMonsterScript == NULL )
	{
		NETWORK2->PostServerEvent("cDrop::DropValue pMonsterScript[%d] == NULL", monsterKind);
		return;
	}

	unsigned long maxMoney = 0;
	unsigned long minMoney = 0;
	int moneyDropPer = 0;

	/// 드랍 money 정보
	sDropMoneyScript* pMoney = mpDropScript->GetDropMonMoney( monsterKind );
	if( pMoney != NULL ) 
	{
		maxMoney = pMoney->mMaxMoney;
		minMoney = pMoney->mMinMoney;
		moneyDropPer = pMoney->mDropPercent;
	}

	/// 최소 최대 금액 수치확인
	long moneyGap = maxMoney - minMoney;
	if( moneyGap > RANDOM_MAX )
	{
		NETWORK2->PostServerEvent("cDrop::MonsterDropValue[%d] moneyGap > RANDOM_MAX", monsterKind, moneyGap );
		moneyGap = RANDOM_MAX;
	}

	/// 몬스터를 공격한 데미지 맵
	TakeDamageRoot* pTakeDamageRoot = pMonster->GetTakeDamageRoot();
	if( pTakeDamageRoot == NULL )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDrop::DropValue pTakeDamageMap == NULL");
		return;
	}

	/// 랜덤값 추출
	unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );
	int dropMoney = 0;
	unsigned long dropExp = pMonsterScript->mExp;
	unsigned long dropSxp = pMonsterScript->mSxp;

	/// money드랍 체크 성공시 랜덤금액 결정
	int droprand = ( randValue % 100 ) + 1;
	if( droprand <= moneyDropPer ) 
	{
		randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );

		if( moneyGap > 0 )
			dropMoney = minMoney + ( randValue % moneyGap ); 
		else
			dropMoney = minMoney;

		/// 로그 정보 갱신
		sDropItemLog* pDropLog = (sDropItemLog*)mDropLogMap.GetAt( monsterKind );
		if( pDropLog != NULL )
			++pDropLog->mMonMoneyCnt;
	}

	unsigned long playerIdx;		/// 공격자 정보
	unsigned long damage;			/// 공격자가준 데미지
	float partyTotalMoney;			/// 개인/파티 분배 할 전체량
	float partyTotalExp;
	float partyTotalSxp;

	unsigned long partyIdx;			/// 파티 고유번호

	unsigned long calcExp;
	unsigned long calcSxp;

	/// 공격한 플레이어(데미지)전체 합산
	unsigned long totalDamage = 0;

	/// 몬스터가 공격받은 전체 데미지 값
	PerTakeDamage* per = (PerTakeDamage*)pTakeDamageRoot->pool;
	while( per )
	{
		totalDamage += per->takeDamage;
		per = (PerTakeDamage*)per->next;
	}

	partyIdxSet.Clear();

	per = (PerTakeDamage*)pTakeDamageRoot->pool;
	while( per )
	{
		playerIdx = per->playerIdx;
		damage = per->takeDamage;

		/// 데미지를 주지않는 형태의 디버프
		if( damage == 0 )
		{
			per = (PerTakeDamage*)per->next;
			continue;
		}

		/// 플레이어 객체가 없으면 money를 주지 않음
		cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
		if( pPlayer == NULL )
		{
			per = (PerTakeDamage*)per->next;
			continue;
		}

		/// 데미지 비율로 개인/파티(전체) 에게 돌아갈 money 계산
		partyTotalMoney = (float)(dropMoney * damage / totalDamage);

		/// 증감치 계산을 하기전 순수 경험치
		partyTotalExp = (float)(dropExp * damage / totalDamage);
		partyTotalSxp = (float)(dropSxp * damage / totalDamage);

		bool IsParty = false;

		/// 파티정보를 읽어옴
		cParty* pParty = NULL;
		partyIdx = pPlayer->GetPartyIndex();
		if( partyIdx != 0 )
		{
			pParty = PARTYMAN->GetParty( partyIdx );
			if( pParty != NULL )
				IsParty = true;
		}

		/// 파티가 아닌 경우
		if( IsParty == false )
		{
			if( pPlayer->GetStateDie() == true )
			{
				per = (PerTakeDamage*)per->next;
				continue;
			}

			/// 공격 데미지가 1% 미만인경우 보상 없음
			if( pMonster->GetMaxHP() * (float)0.01 > damage )	
			{
				per = (PerTakeDamage*)per->next;
				continue; 
			}

			/// 드랍 조건이 타당한지 체크
			if( DropMonCheck( pMonster, pPlayer ) == false )
			{
				per = (PerTakeDamage*)per->next;
				continue;
			}

			calcExp = FloatToInt( MonsterDropExpIncrease( partyTotalExp, pPlayer->GetLevel(), pMonsterScript->mLevel ) );
			calcSxp = FloatToInt( MonsterDropExpIncrease( partyTotalSxp, pPlayer->GetLevel(), pMonsterScript->mLevel ) );

			///------ 드랍 통계자료 출력시 주석
			pPlayer->AddExp( calcExp, true, false );
			pPlayer->AddSxp( calcSxp, true );

			if( partyTotalMoney >= 1.0f )
			{
				float addMoneyPer = pPlayer->GetStatusEtc()->mMoneyAddPer;
				if( addMoneyPer < 0.0f )
				{
					NETWORK2->PostServerEvent("cDrop::DropMonsterValue addMoneyPer < 0.0f[%f]", partyTotalMoney );					
					pPlayer->AddMoney( pMonster->GetObject(), FloatToInt(partyTotalMoney) );
				}
				else
				{
					unsigned long addMoney = (unsigned long)(partyTotalMoney + ( partyTotalMoney * addMoneyPer / 100.0f ));
					pPlayer->AddMoney( pMonster->GetObject(), addMoney );
				}
			}
			///---------------------------------

		}
		else
		{
			/// 파티 인경우 파티처리용 Set에 기록한 후 처리
			/// ※ 한파티 여러명이 친경우 insert 시도는 여러 번 있을 수 있지만 1개만 들어감
			partyIdxSet.Insert( partyIdx );

			/// 파티 객체에 드랍 값 누적 입력
			if( pParty != NULL )
				pParty->AddDropValue( pMonster->GetMaxHP(), damage, partyTotalMoney,  partyTotalExp, partyTotalSxp );
		}

		per = (PerTakeDamage*)per->next;
	}

	cHashSet::cIterator setIter = partyIdxSet.Begin();
	cHashSet::cIterator setIterEnd = partyIdxSet.End();

	/// 따로 처리 할 파티 경험치 분배 처리
	while( setIter != setIterEnd )
	{
		partyIdx = (*setIter++);
		cParty* pParty = PARTYMAN->GetParty( partyIdx );
		if( pParty != NULL )
			pParty->DivideDropValue( pMonster->GetObjectID(), pMonsterScript->mLevel );	/// 파티원에게 money, exp, sxp를 분배
	}
}


void cDrop::DropMonsterClass( unsigned long monsterIdx, float penalty )
{

	/// 드랍 스크립트에서 몬스터 종류에 해당하는 드랍정보 읽어 옴
	cAry* pAry = DROPSCRIPT->GetDropMonClass( monsterIdx );
	if( pAry == NULL )
		return;

	for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
	{
		sDropClassScript* pClass = (sDropClassScript*)(*pAry)[i];
		if( pClass == NULL )
		{
			NETWORK2->PostServerEvent("cDrop::DropMonsterClass pClass[%d] == NULL", monsterIdx );
			continue;
		}

		unsigned long dropCnt = 1;
		unsigned long dropAddRate = pClass->mAddDrop1Rate + pClass->mAddDrop2Rate;

		unsigned long classIdx = pClass->mClassIdx;

		/// 드랍 반복 횟수 결정
		if( dropAddRate != 0 )
		{
			/// 랜덤값 추출
			unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;

			if( randValue <= pClass->mAddDrop1Rate )
				dropCnt += pClass->mAddDrop1Cnt;
			else if( randValue <= dropAddRate )
				dropCnt += pClass->mAddDrop2Cnt;
		}

		/// 같은 아이템목록 그룹에 중복으로 떨어지는지 체크하는set 초기화
		mTempSelectItemSet.Clear();

		/// 선택된 드랍 횟수만큼 반복
		for( unsigned char j = 0 ; j < dropCnt ; ++j )
		{
			/// 랜덤값 추출
			unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;

			/// 아이템 그룹중 하나를 선택
			cAry* pItemAry = (cAry*)&pClass->mItemGroup;
			if( pItemAry == NULL )
				continue;

			unsigned long rateSum = 0;
			for( unsigned long i = 0 ; i < pItemAry->GetSize() ; ++i )
			{
				sDropItemGroupScript* pItemGroup = (sDropItemGroupScript*)(*pItemAry)[i];
				if( pItemGroup == NULL )
				{
					NETWORK2->PostServerEvent("cDrop::DropMonsterClass pItemGroup[%d] == NULL", classIdx );
					continue;
				}

				rateSum += pItemGroup->mItemGroupRate;

				if( rateSum < randValue )
					continue;

				sDropItemListScript* pItemList = DROPSCRIPT->GetDropMonItem( pItemGroup->mItemGroupIdx );
				if( pItemList == NULL )
				{
					NETWORK2->PostServerEvent("cDrop::DropMonsterClass pItemList[%d,%d] == NULL", classIdx, pItemGroup->mItemGroupIdx );
					continue;
				}

				++mDropCnt;

				/// 선택된 그룹의 아이템목록을 드랍
				DropMonItemSelect( pItemList, penalty );
				break;
			}
		}
	}
}


void cDrop::DropMonsterLevelType( unsigned char level, unsigned char type, float penalty )
{

	/// 드랍 스크립트에서 몬스터 종류에 해당하는 드랍정보 읽어 옴
	cAry* pAry = DROPSCRIPT->GetDropMonLevelType( level, type );
	if( pAry == NULL )
		return;

	for( unsigned long i1 = 0 ; i1 < pAry->GetSize() ; ++i1 )
	{
		sDropLevelTypeScript* pLevelType = (sDropLevelTypeScript*)(*pAry)[i1];
		if( pLevelType == NULL )
		{
			NETWORK2->PostServerEvent("cDrop::DropMonsterLevelType pLevelType[%d,%d] == NULL", level, type );
			continue;
		}

		if( pLevelType->mServerTypeCheck == eDROPSERVERTYPE_FIELD && NETWORK2->GetServerType() != _E_ST_NORMAL_MAP_ )
			continue;
		else if( pLevelType->mServerTypeCheck == eDROPSERVERTYPE_INDUN && NETWORK2->GetServerType() == _E_ST_NORMAL_MAP_ )
			continue;			

		unsigned long dropCnt = 1;
		unsigned long dropAddRate = pLevelType->mAddDrop1Rate + pLevelType->mAddDrop2Rate;

		/// 확률 토탈값 오류
		if( dropAddRate > RANDOM_MAX )
		{
			NETWORK2->PostServerEvent("cDrop::DropMonsterLevelType dropAddRate[%d,%d,%d] > RANDOM_MAX", level, type, dropAddRate );
			dropAddRate = RANDOM_MAX;
		}

		/// 드랍 반복 횟수 결정
		if( dropAddRate != 0 )
		{
			/// 랜덤값 추출
			unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;

			if( randValue <= pLevelType->mAddDrop1Rate )
				dropCnt += pLevelType->mAddDrop1Cnt;
			else if( randValue <= dropAddRate )
				dropCnt += pLevelType->mAddDrop2Cnt;
		}

		/// 같은 아이템목록 그룹에 중복으로 떨어지는지 체크하는set 초기화
		mTempSelectItemSet.Clear();

		/// 드랍 반복
		for( unsigned char i2 = 0 ; i2 < dropCnt ; ++i2 )
		{
			/// 랜덤값 추출
			unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;

			cAry* pItemAry = (cAry*)&pLevelType->mItemGroup;
			if( pItemAry == NULL )
				continue;

			unsigned long rateSum = 0;
			for( unsigned long i3 = 0 ; i3 < pItemAry->GetSize() ; ++i3 )
			{
				sDropItemGroupScript* pItemGroup = (sDropItemGroupScript*)(*pItemAry)[i3];
				if( pItemGroup == NULL )
				{
					NETWORK2->PostServerEvent("cDrop::DropMonsterLevelType pItemGroup[%d,%d] == NULL", level, type );
					continue;
				}

				rateSum += pItemGroup->mItemGroupRate;

				if( rateSum < randValue )
					continue;

				sDropItemListScript* pItemList = DROPSCRIPT->GetDropMonItem( pItemGroup->mItemGroupIdx );
				if( pItemList == NULL )
				{
					NETWORK2->PostServerEvent("cDrop::DropMonsterLevelType pItemList[%d,%d,%d] == NULL", level, type, pItemGroup->mItemGroupIdx );
					continue;
				}

				DropMonItemSelect( pItemList, penalty );
				break;
			}
		}
	}
}


void cDrop::DropMonItemSelect( sDropItemListScript* pDropItemList, float penalty )
{
	if( pDropItemList == NULL )
	{
		NETWORK2->PostServerEvent( "cDrop::DropItemSelect pDropItemList == NULL" );
		return;
	}

	cAry* pItemAry = (cAry*)&pDropItemList->mItem;
	if( pItemAry == NULL )
	{
		NETWORK2->PostServerEvent("cDrop::DropMonsterClass pItemAry[%d] == NULL", pDropItemList->mItemGroupIdx );
		return;
	}

	//unsigned char Data[4];
	//Data[0]=(unsigned char)(rand()%256);
	//Data[1]=(unsigned char)(rand()%256);
	//Data[2]=(unsigned char)(rand()%256);
	//Data[3]=(unsigned char)(rand()%256);

	//unsigned long *dwRandData=(unsigned long*)Data;
	//randValue = ((*dwRandData)%1000000);

	/// 랜덤값 추출
	cHashMap::cIterator f = mItemGroupRandSeed.Find( pDropItemList->mItemGroupIdx );
	cHashMap::cIterator e = mItemGroupRandSeed.End();
	unsigned long randValue = 0;
	if( f != e )
		randValue = (unsigned long)( RANDOMTABLE->Get( (*f).mSecond ) * 1000000 ) + 1;
	else
	{
		NETWORK2->PostServerEvent("cDrop::DropMonItemSelect f == e [%d]", pDropItemList->mItemGroupIdx );
		randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) + 1;
	}

	unsigned long rateSum = 0;
	for( unsigned long i = 0 ; i < pItemAry->GetSize() ; ++i )
	{
		sDropItemScript* pItem = (sDropItemScript*)(*pItemAry)[i];
		if( pItem == NULL )
		{
			NETWORK2->PostServerEvent("cDrop::DropMonsterClass pItem[%d] == NULL", pDropItemList->mItemGroupIdx );
			continue;
		}
		
		rateSum += (unsigned long)(pItem->mItemRate * penalty);

		if( rateSum < randValue )
			continue;

		if( pItem->mItemIdx == 0 )
		{
			/// 스크립트 오류
			NETWORK2->PostServerEvent("cDrop::DropMonItemSelect pItem->mItemIdx == 0");
			continue;
		}

		if( pItem->mItemCount == 0 )
		{
			NETWORK2->PostServerEvent("cDrop::DropMonsterClass pItem->mItemCount[%d,%d,%d] == 0", 
				pDropItemList->mItemGroupIdx, pItem->mItemIdx, pItem->mItemCount );
			return;
		}

		/// 이미 한번 선택됐던 아이템이다
		if( mTempSelectItemSet.GetCount( pItem ) != 0 )
		{
			if( mDropNoItemCntMap.Insert( pItem->mItemIdx, pItem->mItemCount ) == false )
			{
				/// 입력을 했는데 이미 있는 아이템이면 값을 + 시킨다
				cHashMap::cIterator find = mDropNoItemCntMap.Find( pItem->mItemIdx );
				if( find != mDropNoItemCntMap.End() )
					(*find).mSecond += pItem->mItemCount;
			}

			//Verbose->WriteLog("이미선택");
			return;
		}

		cPHashMap::cIterator find = mDropMonCantTime.Find( pItem->mItemIdx );
		if( find != mDropMonCantTime.End() )
		{
			sDropLimit* pDropLimit = (sDropLimit*)(*find).mSecond;
			if( pDropLimit != NULL )
			{
				/// 드랍시간 구간이 현재와 맞는지 체크
				if( TIMER->DiffRealTime( pDropLimit->mEndTime ) > 0 )
				{
					/// 현재 시간과 구간이 맞는 경우 드랍 유무 판단
					if( pDropLimit->mIsDrop == true )
					{
						/// 이미 드랍 한경우임 - 드랍 안함
						if( mDropNoItemCntMap.Insert( pItem->mItemIdx, pItem->mItemCount ) == false )
						{
							/// 입력을 했는데 이미 있는 아이템이면 값을 + 시킨다
							cHashMap::cIterator find = mDropNoItemCntMap.Find( pItem->mItemIdx );
							if( find != mDropNoItemCntMap.End() )
								(*find).mSecond += pItem->mItemCount;

							// Verbose->WriteLog("시간 제약");
						}

						return;
					}
					else
					{
						/// 정상구간에 드랍되지 않은 경우이므로 드랍시키고 드랍했음 플래그만 킨다
						pDropLimit->mIsDrop = true;

						/// 네트워크 발송
						NETWORK2->PostItemDropLimitUpdate( pDropLimit->mItemIdx, pDropLimit->mEndTime );

						/// DB기록
						HANDLE        handle = NULL;
						ITEM_DROPLIMIT_UPDATE* pUpdate = (ITEM_DROPLIMIT_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE );
						if( pUpdate != NULL )
						{
							pUpdate->mItemIdx = pDropLimit->mItemIdx;
							pUpdate->mEndTime = pDropLimit->mEndTime;
							NETWORK2->SendSQL( handle, sizeof(ITEM_DROPLIMIT_UPDATE) );
						}
					}
				}
				else
				{
					/// 현재 시간구간이 안맞으므로 이전 드랍 시간기간지나서 처음으로 떨어지는 경우임
					/// 시간 설정을 현재를 기준으로 시간 설정하고 드랍 플래그를 킨다
					TIMESTAMP_STRUCT nextTime = pDropLimit->mEndTime;
					
					unsigned long errorCheck = 0;
					while( TIMER->DiffRealTime( nextTime ) <= 0 )
					{
						struct tm tmNextTime = TIMER->ConvertTStoTM( nextTime );
						time_t result;

						tmNextTime.tm_sec = tmNextTime.tm_sec + ( pDropLimit->mDropTerm / 1000 );

						if( (result = mktime( &tmNextTime )) != (time_t)-1 )
						{
							//datetime = *localtime( &result );
							errno_t err = localtime_s( &tmNextTime , &result );
							if( err != 0 )
								NETWORK2->PostServerEvent("cDrop::DropMonItemSelect errno_t err[%d]", err );
						}

						nextTime.year	= (SQLSMALLINT)tmNextTime.tm_year+1900;
						nextTime.month  = (SQLSMALLINT)tmNextTime.tm_mon+1;
						nextTime.day    = (SQLSMALLINT)tmNextTime.tm_mday;
						nextTime.hour   = (SQLSMALLINT)tmNextTime.tm_hour;
						nextTime.minute = (SQLSMALLINT)tmNextTime.tm_min;
						nextTime.second = (SQLSMALLINT)tmNextTime.tm_sec;
						nextTime.fraction = 0;

						/// 반복문이 너무 많이 도는 경우에 대비(버그체크)
						++errorCheck;
						if( errorCheck > 500 )
						{
							pDropLimit->mEndTime = nextTime;
							NETWORK2->PostServerEvent("cDrop::DropMonItemSelect errorCheck > 500" );
							return;
						}							
					}

					pDropLimit->mIsDrop = true;
					pDropLimit->mEndTime = nextTime;

					/// 네트워크 발송
					NETWORK2->PostItemDropLimitUpdate( pDropLimit->mItemIdx, pDropLimit->mEndTime );

					///db에 요청
					HANDLE        handle = NULL;
					ITEM_DROPLIMIT_UPDATE* pUpdate = (ITEM_DROPLIMIT_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE );
					if( pUpdate != NULL )
					{
						pUpdate->mItemIdx = pDropLimit->mItemIdx;
						pUpdate->mEndTime = pDropLimit->mEndTime;
						NETWORK2->SendSQL( handle, sizeof(ITEM_DROPLIMIT_UPDATE) );
					}
				}
			}
		}

		mTempSelectItemSet.Insert( pItem );

		if( pItem->mItemCount == 0 )
			NETWORK2->PostServerEvent("cDrop::DropMonItemSelect itemIdx[%d], itemCnt[%d]", pItem->mItemIdx, pItem->mItemCount );

		/// 드랍 목록에 등록
		if( mDropItemCntMap.Insert( pItem->mItemIdx, pItem->mItemCount ) == false )
		{
			/// 입력을 했는데 이미 있는 아이템이면 값을 + 시킨다
			cHashMap::cIterator find = mDropItemCntMap.Find( pItem->mItemIdx );
			if( find == mDropItemCntMap.End() )
			{
				NETWORK2->PostServerEvent("cDrop::DropMonsterClass find[%d,%d,%d] == mDropItemCntMap.End()",
					pDropItemList->mItemGroupIdx, pItem->mItemIdx, pItem->mItemCount );   
				return;
			}

			(*find).mSecond += pItem->mItemCount;

			if( (*find).mSecond == 0 )
				NETWORK2->PostServerEvent("cDrop::DropMonItemSelect itemIdx[%d], itemCnt[%d]", pItem->mItemIdx, (*find).mSecond );
		}

		return;
	}
}


void cDrop::DropMonQuestType( cMonster* pMonster, cPlayer* pPlayer )
{
	if( pMonster == NULL )
		return;
	if( pPlayer == NULL )
		return;

	unsigned long monsterClassIdx = pMonster->GetRaceGender();

	/// 퀘스트 전체 검사
	for( unsigned long i = 0 ; i < MAX_KEEPQUEST ; ++i )
	{
		/// 퀘스트 정보 로드
		TB_QUEST_PROGRESS* pQuest = pPlayer->GetQuest( i );
		if( pQuest == NULL )
			break;

		if( pQuest->idx == 0 )
			return;

		cQuestDefine* questDefine = QUESTMAN->GetQuestDefine( pQuest->questIdx );
		if( !questDefine )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("DropQType questDefine[%d] == NULL", pQuest->questIdx );
			return;
		}

		/// 퀘스트 진행 중 체크
		if( pQuest->check == questDefine->mComplete )
			continue;

		/// 몬스터가 플레이어의 퀘스트에 해당하는 드랍아이템 종류를 읽어옴
		cAry* pAry = DROPSCRIPT->GetDropMonQuest( monsterClassIdx, pQuest->questIdx );
		if( pAry == NULL )
			continue;

		/// 드랍 아이템 비교
		for( unsigned long itemI = 0 ; itemI < pAry->GetSize() ; ++itemI )
		{
			sDropQType* pDropQ = (sDropQType*)(*pAry)[itemI];

			if( pDropQ == NULL )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("DropQType pDropQ == NULL [%d]", itemI );
				continue;
			}

			/// 드랍 확률 비교
			//double randvalue = RANDOMTABLE->Get( mQuestSeed );
			/*unsigned long selectRand = ( randvalue * 100 ) + 1;*/
			//unsigned long selectRand = ( rand() % 100 ) + 1;
			double randvalue = RANDOMTABLE->Getx100( mQuestSeed ) + 1;

			double dropPer = (double)pDropQ->mDropPer;

			if( randvalue > dropPer )
				continue;

			/// 드랍 필요 수량 확인
			unsigned long dropNeedCnt = pPlayer->GetDropItemCount( i, pDropQ->mItemIdx );
			if( dropNeedCnt == 0 )
				continue;

			/// 퀘스트 제한 검사 - 착용정보 검사
			if( pPlayer->IsEquipQuestItem( pQuest->questIdx ) == false )
				continue;

			/// 진행시 필요한 조건 만족하는지 검사
			if( TRIGGERMAN->CheckDuty( pPlayer->GetObjectID(), pPlayer->GetObjectID(), pQuest->questIdx ) == false )
				continue;

			/// 플레이어 자동 습득
			if( pPlayer->ItemGetQuest( pDropQ->mItemIdx, 1 ) == true )
			{
				/// 퀘스트 아이템 수집 관련 업데이트
				pPlayer->UpdateDutyItem();
			}
			else
			{
				assert(NULL);
				NETWORK2->PostServerEvent("Drop::DropQType ItemGetQuest[%d] ERROR", pDropQ->mItemIdx );
			}
		}
	}
}


float cDrop::MonsterDropExpIncrease( float takeExp, unsigned char playerLevel, unsigned char monsterLevel )
{
	char levelDifferent;
	float increase;
	float exp = 0.0f;

	/// 레벨차에 따른 증감치를 얻어옴
	levelDifferent = monsterLevel - playerLevel;

	bool isTheme = NETWORK2->GetServerType() == _E_ST_ID_THEME_;
	increase = DROPSCRIPT->GetDropExpIncrease( isTheme, levelDifferent );

	/// 경험치 * 증가치
	exp = takeExp * increase;
	if( exp < 0.0f )
	{
		assert(NULL);
		NETWORK2->PostServerEvent("cDrop::DropExpIncrease exp < 0.0f");
		return 0.0f;
	}
	return exp;
}



bool cDrop::DropMonCheck( cMonster* pMonster, cPlayer* pPlayer )
{
	if( pMonster == NULL )
		return false;
	if( pPlayer == NULL )
		return false;

	if( pMonster->GetMapNumber() != pPlayer->GetMapNumber() )
		return false;

	NiPoint2 monsterPos = pMonster->GetPos();
	NiPoint2 playerPos = pPlayer->GetPos();

	cRangeCheck rangeCheck( PARTY_RANGE );
	if( rangeCheck.IsRange( monsterPos, playerPos ) == false )
		return false;

	return true;
}


void cDrop::GatherDrop( cGathering* pGathering, unsigned long playerIdx )
{
	if( pGathering == NULL )
		return;

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;

	/// 드랍 목록 임시공간 초기화
	mDropItemCntMap.Clear();

	///// money 드랍
	DropGatherValue( pGathering, playerIdx );
	/// 퀘스트드랍
	DropGatherQuest( pGathering, playerIdx );
	/// 종류별(일반) 드랍
	DropGatherClass( pGathering, playerIdx );

	/// 효과 추가
	if( pGathering->GetInfluence() !=0 )
		SKILLMANAGER->AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), pGathering->GetInfluence(), 0, true );

	if( mDropItemCntMap.GetSize() == 0 )
		return;

	/// 드랍 아이템 처리
	cHashMap::cConstIterator begin = mDropItemCntMap.Begin();
	cHashMap::cConstIterator end = mDropItemCntMap.End();
	bool                     apply = false;

	for( ; begin != end ; ++begin )
	{
		unsigned long itemIdx = (*begin).mFirst;
		unsigned long itemCnt = (*begin).mSecond;

		unsigned short applyCnt = 0;
		while( applyCnt < itemCnt )
		{
			unsigned short cnt = pPlayer->ItemGetAutoPush( itemIdx, (unsigned short)itemCnt );
			if( cnt == 0 )
			{
				assert(0);
				NETWORK2->PostServerEvent("cDrop::GatherDrop item(%d,%d)->ItemGetAutoPush cnt == 0", itemIdx, itemCnt);
				break;
			}

			applyCnt = applyCnt + cnt;
			apply    = true;
		}
	}
	pPlayer->ItemGetAutoEnd( apply );

	mDropItemCntMap.Clear();
}


void cDrop::InsertItemGroupRandSeed( unsigned long itemGroup, unsigned long randSeed )
{
	if( mItemGroupRandSeed.Insert( itemGroup, randSeed ) == false )
	{
		NETWORK2->PostServerEvent("cDrop::InsertItemGroupRandSeed Insert Duplicate[%d,%d]", itemGroup, randSeed );
	}
}


void cDrop::DropGatherValue( cGathering* pGathering, unsigned long playerIdx )
{
	if( pGathering == NULL )
		return;

	/// 플레이어 객체가 없으면 money를 주지 않음
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL || pPlayer->GetStateDie() == true )
		return;

	unsigned long classIdx = pGathering->GetClassIdx();

	sDropMoneyScript* pMoney = mpDropScript->GetDropGatherMoney( classIdx );
	if( pMoney == NULL )
		return;

	unsigned long maxMoney = pMoney->mMaxMoney;
	unsigned long minMoney = pMoney->mMinMoney;

	/// 최소 최대 금액 수치확인
	long moneyGap = maxMoney - minMoney;
	if( moneyGap > RANDOM_MAX )
	{
		NETWORK2->PostServerEvent("cDrop::GatherDropValue moneyGap[%d,%d] > RANDOM_MAX", classIdx, moneyGap );
		moneyGap = RANDOM_MAX;
	}

	/// money드랍 체크 
	unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );
	unsigned long droprand = ( randValue % 100 ) + 1;
	if( droprand > pMoney->mDropPercent ) 
		return;

	/// 랜덤금액 결정
	randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );
	int dropMoney = 0;
	if( moneyGap > 0 )
		dropMoney = minMoney + ( randValue % moneyGap ); 
	else
		dropMoney = minMoney;

	pPlayer->AddMoney( pGathering->GetObject(), dropMoney );
}


void cDrop::DropGatherClass( cGathering* pGathering, unsigned long playerIdx )
{
	if( pGathering == NULL )
		return;

	/// 플레이어 객체가 없으면 money를 주지 않음
	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL || pPlayer->GetStateDie() == true )
		return;

	unsigned long classIdx = pGathering->GetClassIdx();

	cAry* pAry = DROPSCRIPT->GetDropGatherClass( classIdx );
	if( pAry == NULL )
		return;

	for( unsigned long i = 0 ; i < pAry->GetSize() ; ++i )
	{
		sDropClassScript* pClass = (sDropClassScript*)(*pAry)[i];
		if( pClass == NULL )
		{
			NETWORK2->PostServerEvent("cDrop::DropGatherClass pClass[%d] == NULL", classIdx );
			continue;
		}

		unsigned long dropCnt = 1;
		unsigned long dropAddRate = pClass->mAddDrop1Rate + pClass->mAddDrop2Rate;

		unsigned long classIdx = pClass->mClassIdx;

		/// 드랍 반복 횟수 결정
		if( dropAddRate != 0 )
		{
			/// 랜덤값 추출
			unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );
			unsigned long selectRand = ( randValue % dropAddRate ) + 1;

			if( selectRand <= pClass->mAddDrop1Rate )
				dropCnt += pClass->mAddDrop1Cnt;
			else if( selectRand < dropAddRate )
				dropCnt += pClass->mAddDrop2Cnt;
		}

		/// 같은 아이템목록 그룹에 중복으로 떨어지는지 체크하는set 초기화
		mTempSelectItemSet.Clear();

		/// 선택된 드랍 횟수만큼 반복
		for( unsigned char j = 0 ; j < dropCnt ; ++j )
		{
			/// 랜덤값 추출
			unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;

			/// 아이템 그룹중 하나를 선택
			cAry* pItemAry = (cAry*)&pClass->mItemGroup;
			if( pItemAry == NULL )
				continue;

			unsigned long rateSum = 0;
			for( unsigned long i = 0 ; i < pItemAry->GetSize() ; ++i )
			{
				sDropItemGroupScript* pItemGroup = (sDropItemGroupScript*)(*pItemAry)[i];
				if( pItemGroup == NULL )
				{
					NETWORK2->PostServerEvent("cDrop::DropGatherClass pItemGroup[%d] == NULL", classIdx );
					continue;
				}

				rateSum += pItemGroup->mItemGroupRate;

				if( rateSum < randValue )
					continue;

				sDropItemListScript* pItemList = DROPSCRIPT->GetDropGatherItem( pItemGroup->mItemGroupIdx );
				if( pItemList == NULL )
				{
					NETWORK2->PostServerEvent("cDrop::DropGatherClass pItemList[%d,%d] == NULL", classIdx, pItemGroup->mItemGroupIdx );
					continue;
				}

				/// 선택된 그룹의 아이템목록을 드랍
				DropGatherItemSelect( pItemList );
				break;
			}
		}
	}
}


void cDrop::DropGatherQuest( cGathering* pGathering, unsigned long playerIdx )
{
	if( pGathering == NULL )
		return;

	unsigned long gatherClassIdx = pGathering->GetClassIdx();

	cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( playerIdx );
	if( pPlayer == NULL )
		return;

	/// 퀘스트 전체 검사
	for( unsigned long i = 0 ; i < MAX_KEEPQUEST ; ++i )
	{
		/// 퀘스트 정보 로드
		TB_QUEST_PROGRESS* pQuest = pPlayer->GetQuest( i );
		if( pQuest == NULL )
			break;

		if( pQuest->idx == 0 )
			return;

		cQuestDefine* questDefine = QUESTMAN->GetQuestDefine( pQuest->questIdx );
		if( !questDefine )
		{
			assert(NULL);
			NETWORK2->PostServerEvent("GatherDropQType questDefine[%d] == NULL", pQuest->questIdx );
			return;
		}

		/// 퀘스트 진행 중 체크
		if( pQuest->check == questDefine->mComplete )
			continue;

		/// 몬스터가 플레이어의 퀘스트에 해당하는 드랍아이템 종류를 읽어옴
		cAry* pAry = DROPSCRIPT->GetDropGatherQuest( gatherClassIdx, pQuest->questIdx );
		if( pAry == NULL )
			continue;

		/// 드랍 아이템 비교
		for( unsigned long itemI = 0 ; itemI < pAry->GetSize() ; ++itemI )
		{
			sDropQType* pDropQ = (sDropQTypeGather*)(*pAry)[itemI];
			if( pDropQ == NULL )
			{
				assert(NULL);
				NETWORK2->PostServerEvent("GatherDropQType pDropQ == NULL [%d]", itemI );
				continue;
			}

			/// 드랍 확률 비교
			unsigned long randvalue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 );
			unsigned long selectRand = ( randvalue % 100 ) + 1;
			if( selectRand > pDropQ->mDropPer )
				continue;

			/// 드랍 필요 수량 확인
			unsigned long dropNeedCnt = pPlayer->GetDropItemCount( i, pDropQ->mItemIdx );
			if( dropNeedCnt == 0 )
				continue;

			/// 퀘스트 제한 검사 - 착용정보 검사
			if( pPlayer->IsEquipQuestItem( pQuest->questIdx ) == false )
				continue;

			/// 진행시 필요한 조건 만족하는지 검사
			if( TRIGGERMAN->CheckDuty( pPlayer->GetObjectID(), pPlayer->GetObjectID(), pQuest->questIdx ) == false )
				continue;

			/// 플레이어 자동 습득
			if( pPlayer->ItemGetQuest( pDropQ->mItemIdx, 1 ) == true )
			{
				/// 퀘스트 아이템 수집 관련 업데이트
				pPlayer->UpdateDutyItem();
			}
			else
			{
				assert(NULL);
				NETWORK2->PostServerEvent("Drop::GatherDropQType ItemGetQuest[%d] ERROR", pDropQ->mItemIdx );
			}
		}
	}
}


void cDrop::DropGatherItemSelect( sDropItemListScript* pDropItemList )
{
	if( pDropItemList == NULL )
	{
		NETWORK2->PostServerEvent( "cDrop::DropGatherItemSelect pDropItemList == NULL" );
		return;
	}

	cAry* pItemAry = (cAry*)&pDropItemList->mItem;
	if( pItemAry == NULL )
	{
		NETWORK2->PostServerEvent("cDrop::DropGatherItemSelect pItemAry[%d] == NULL", pDropItemList->mItemGroupIdx );
		return;
	}

	/// 랜덤값 추출
	unsigned long randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % 1000000 + 1;

	unsigned long rateSum = 0;
	for( unsigned long i = 0 ; i < pItemAry->GetSize() ; ++i )
	{
		sDropItemScript* pItem = (sDropItemScript*)(*pItemAry)[i];
		if( pItem == NULL )
		{
			NETWORK2->PostServerEvent("cDrop::DropGatherItemSelect pItem[%d] == NULL", pDropItemList->mItemGroupIdx );
			continue;
		}

		rateSum += pItem->mItemRate;

		if( rateSum < randValue )
			continue;

		/// 선택 안될확률
		if( pItem->mItemIdx == 0 )
			return;

		if( pItem->mItemCount == 0 )
		{
			NETWORK2->PostServerEvent("cDrop::DropGatherItemSelect pItem->mItemCount[%d,%d,%d] == 0", 
				pDropItemList->mItemGroupIdx, pItem->mItemIdx, pItem->mItemCount );
			return;
		}

		/// 이미 한번 선택됐던 아이템이다
		if( mTempSelectItemSet.GetCount( pItem ) != 0 )
			return;

		unsigned long cantTime = 0;
		cHashMap::cIterator find = mDropGatherCantTime.Find( pItem->mItemIdx );
		if( find != mDropItemCntMap.End() )
		{
			if( (*find).mSecond > NETWORK2->GetAccumTime() )
				return;

			cantTime = DROPSCRIPT->GetDropGatherCantTime( pItem->mItemIdx );
			cantTime += NETWORK2->GetAccumTime();
			(*find).mSecond = cantTime;
		}

		mTempSelectItemSet.Insert( pItem );

		/// 드랍 목록에 등록
		if( mDropItemCntMap.Insert( pItem->mItemIdx, pItem->mItemCount ) == false )
		{
			/// 입력을 했는데 이미 있는 아이템이면 값을 + 시킨다
			cHashMap::cIterator find = mDropItemCntMap.Find( pItem->mItemIdx );
			if( find == mDropItemCntMap.End() )
			{
				NETWORK2->PostServerEvent("cDrop::DropGatherItemSelect find[%d,%d,%d] == mDropItemCntMap.End()",
					pDropItemList->mItemGroupIdx, pItem->mItemIdx, pItem->mItemCount );   
				return;
			}

			(*find).mSecond += pItem->mItemCount;
		}

		return;
	}
}
