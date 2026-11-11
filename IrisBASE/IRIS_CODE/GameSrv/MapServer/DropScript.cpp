#include "gamesrv.h"

#include "StdAfx.h"
#include "DropScript.h"
#include "Drop_Common.h"
#include "GameFile.h"
#include "AppTimer.h"
#include "FileLoader.h"
#include "Tokenizer.h"
#include "MonsterScript.h"
#include "Monster_Common.h"
#include "Gathering_Common.h"
#include "GatheringScript.h"
#include "Drop.h"
#include "RandomTable.h"


cDropScript* cDropScript::mpDropScript = NULL;


cDropScript::cDropScript(void)
{
	/// 싱글톤
	if( mpDropScript )
	{
		Verbose->WriteLog( "DropScript new error");
	}
	else
	{
		mpDropScript = this;
	}

}


bool cDropScript::Init( bool dbInsertDataOnly )
{
	if( dbInsertDataOnly == false )
	{
		/// 파티 경험치 증가 계산 정보
		if( !LoadPartyCalcInfo() )
			return false;

		/// 몬스터
		if( LoadDropMonMoney() == false )
			return false;
		if( LoadDropMonClass() == false )
			return false;
		if( LoadDropMonLevelType() == false )
			return false;
		if( LoadDropMonQuest() == false )
			return false;
		if( LoadDropMonItemGroup() == false )
			return false;
		if( LoadDropMonItemCantTime() == false )
			return false;

		/// 레벨차에 따른 경험치 패널티
		if( !LoadDropExpIncrease() )
			return false;

		/// 레벨차에 따른 Drop 패널티
		if( LoadDropIncreadse() == false )
			return false;
	 
		/// 채집
		if( LoadDropGatherMoney() == false )
			return false;
		if( LoadDropGatherClass() == false )
			return false;
		if( LoadDropGatherQuest() == false )
			return false;
		if( LoadDropGatherItemGroup() == false )
			return false;
		if( LoadDropGatherItemCantTime() == false )
			return false;
	}
	else
	{
		if( LoadDropMonItemCantTime() == false )
			return false;
	}
	
	return true;
}


void cDropScript::Release()
{
	cPointrHashMap::cIterator i;

	for( i = mDropGatherClass.Begin() ; i != mDropGatherClass.End() ; ++i )
	{
		cAry* pHead = (cAry*)((*i).mSecond);
		if( pHead == NULL )
			continue;

		for( unsigned long x = 0 ; x < pHead->GetSize() ; ++x )
		{
			sDropClassScript* pInfo = (sDropClassScript*)(*pHead)[x];

			cAry* pAry = &pInfo->mItemGroup;
			if( pAry == NULL )
				continue;

			for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
			{
				sDropItemGroupScript* pItem = (sDropItemGroupScript*)(*pAry)[j];
				SAFE_DELETE( pItem );
			}
			SAFE_DELETE( pInfo );
		}
		SAFE_DELETE( pHead );
	}

	for( i = mDropGatherMoney.Begin() ; i != mDropGatherMoney.End() ; ++i )
	{
		sDropMoneyScript* pInfo = (sDropMoneyScript*)((*i).mSecond);
		SAFE_DELETE( pInfo );
	}

	for( i = mDropGatherItem.Begin() ; i != mDropGatherItem.End() ; ++i )
	{
		sDropItemListScript* pList = (sDropItemListScript*)(*i).mSecond;
		if( pList == NULL )
			continue;

		cAry* pAry = &pList->mItem;
		if( pAry == NULL )
			continue;

		for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
		{
			sDropItemScript* pItem = (sDropItemScript*)(*pAry)[j];
			SAFE_DELETE( pItem );
		}
		SAFE_DELETE( pList );
	}

	for( unsigned long j = 0 ; j < mShutDownDeleteAry.GetSize() ; ++j )
	{
		sDropLevelTypeScript* pInfo = (sDropLevelTypeScript*)(mShutDownDeleteAry[j]);

		cAry* pAry = &pInfo->mItemGroup;
		if( pAry == NULL )
			continue;

		for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
		{
			sDropItemGroupScript* pItem = (sDropItemGroupScript*)(*pAry)[j];
			SAFE_DELETE( pItem );
		}

		SAFE_DELETE( pInfo );
	}
	
	for( i = mDropMonLevelType.Begin() ; i != mDropMonLevelType.End() ; ++i )
	{
		cAry* pAry = (cAry*)((*i).mSecond);
		if( pAry == NULL )
			continue;

		SAFE_DELETE( pAry );
	}

	for( i = mDropMonClass.Begin() ; i != mDropMonClass.End() ; ++i )
	{
		cAry* pHead = (cAry*)((*i).mSecond);
		if( pHead == NULL )
			continue;

		for( unsigned long x = 0 ; x < pHead->GetSize() ; ++x )
		{
			sDropClassScript* pInfo = (sDropClassScript*)(*pHead)[x];

			cAry* pAry = &pInfo->mItemGroup;
			if( pAry == NULL )
				continue;

			for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
			{
				sDropItemGroupScript* pItem = (sDropItemGroupScript*)(*pAry)[j];
				SAFE_DELETE( pItem );
			}
			SAFE_DELETE( pInfo );
		}
		SAFE_DELETE( pHead );
	}

	for( i = mDropMonMoney.Begin() ; i != mDropMonMoney.End() ; ++i )
	{
		sDropMoneyScript* pInfo = (sDropMoneyScript*)((*i).mSecond);
		SAFE_DELETE( pInfo );
	}

	for( i = mDropMonItem.Begin() ; i != mDropMonItem.End() ; ++i )
	{
		sDropItemListScript* pList = (sDropItemListScript*)(*i).mSecond;
		if( pList == NULL )
			continue;

		cAry* pAry = &pList->mItem;
		if( pAry == NULL )
			continue;

		for( unsigned long j = 0 ; j < pAry->GetSize() ; ++j )
		{
			sDropItemScript* pItem = (sDropItemScript*)(*pAry)[j];
			SAFE_DELETE( pItem );
		}
		SAFE_DELETE( pList );
	}

	///
	mNormalDropExpIncreaseMap.Clear();
	mThemeDropExpIncreaseMap.Clear();

	///
	mNormalDropIncreaseMap.Clear();
	mThemeDropIncreaseMap.Clear();
}


unsigned long cDropScript::GetDropMonCantTime( unsigned long itemIdx )
{ 
	unsigned long cantTime = 0;
	mDropMonCantTime.GetAt( &cantTime, itemIdx );
	return cantTime;
}


tArray<void*>* cDropScript::GetDropMonLevelType( unsigned char monsterLevel, unsigned char type )
{ 
	return (tArray<void*>*)mDropMonLevelType.GetAt( type * DROP_MONSTER_TYPE_UNIT + monsterLevel ); 
}


tArray<void*>* cDropScript::GetDropMonQuest( unsigned long monsterClassIdx, unsigned long questIdx )
{
	sMonsterScript* pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( monsterClassIdx );
	if( pMonsterScript == NULL )
		return NULL;

	cPointrHashMap* pMap = (cPointrHashMap*)pMonsterScript->mpQuestItemDropMap;
	if( pMap == NULL )
		return NULL;

	cAry* pAry = (cAry*)pMap->GetAt( questIdx );

	return pAry;
}



tArray<void*>* cDropScript::GetDropGatherQuest( unsigned long classIdx, unsigned long questIdx )
{
	sGatheringList* pGatheringInfo = GATHERINGSCRIPT->GetGatheringInfo( classIdx );
	if( pGatheringInfo == NULL )
		return NULL;

	cPointrHashMap* pMap = (cPointrHashMap*)pGatheringInfo->mpQuestItemDropMap;
	if( pMap == NULL )
		return NULL;

	cAry* pAry = (cAry*)pMap->GetAt( questIdx );

	return pAry;
}



bool cDropScript::LoadDropMonMoney()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/item_dropmoney.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_dropmoney.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{

		/// 몬스터 종류
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long classIdx = str.ToInt();	

		/// 돈 드랍 확률
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long moneyPercent = str.ToInt();					

		/// 돈 드랍 최소 금액
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long minMoney = str.ToInt();						

		/// 돈 드랍 최대 금액
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long maxMoney = str.ToInt();						

		sDropMoneyScript* pList = new sDropMoneyScript;

		pList->mClassIdx = classIdx;	
		pList->mDropPercent = moneyPercent;
		pList->mMinMoney = minMoney;
		pList->mMaxMoney = maxMoney;

		/// 해쉬에 몬스터 별로 기록
		if( mDropMonMoney.Insert( pList->mClassIdx, pList ) == false )
		{
			assert(NULL);
			return false;
		}
	}

	return true;
}


bool cDropScript::LoadDropMonClass()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/Item_DropN.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load Item_DropN.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	/// 시작 컬럼 확인
	if( tokenizer.GetNext( &str ) == false )
		return true;

	while( tokenizer.IsEnd() == false )
	{
		/// 시작컬럼 확인
		if( str.CompareNoCase( "monster" ) != 0 )
			return true;

		/// 몬스터 종류
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long classIdx = str.ToInt();

		/// 추가드랍1 횟수
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char addDrop1Cnt = (unsigned char)str.ToInt();

		/// 추가드랍1 선택 비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long addDrop1Percent = str.ToInt();

		/// 추가드랍2 횟수
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char addDrop2Cnt = (unsigned char)str.ToInt();

		/// 추가드랍2 선택 비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long addDrop2Percent = str.ToInt();						

		sDropClassScript* pList = new sDropClassScript;

		pList->mClassIdx = classIdx;

		pList->mAddDrop1Cnt = addDrop1Cnt;
		pList->mAddDrop1Rate = addDrop1Percent;
		pList->mAddDrop2Cnt = addDrop2Cnt;
		pList->mAddDrop2Rate = addDrop2Percent;

		cAry* pItemGroupAry = &pList->mItemGroup;

		bool check = true;
		while( check )
		{
			/// 시작컬럼 확인
			if( tokenizer.GetNext( &str ) == false )
				break;

			if( str.CompareNoCase( "monster" ) == 0 )
				break;

			/// 아이템 그룹 인덱스
			unsigned long itemGroupIdx = str.ToInt();

			/// 아이템 그룹 선택 비율
			if( tokenizer.GetNext( &str ) == false )	{ return false; }
			unsigned long itemGroupPer = str.ToInt();

			sDropItemGroupScript* pItemGroupScript = new sDropItemGroupScript;

			pItemGroupScript->mItemGroupIdx = itemGroupIdx;
			pItemGroupScript->mItemGroupRate = itemGroupPer;

			pItemGroupAry->PushBack( pItemGroupScript );
		}

		cAry* pAry = (cAry*)mDropMonClass.GetAt( pList->mClassIdx );
		if( pAry == NULL )
		{
			pAry = new cAry;

			if( mDropMonClass.Insert( pList->mClassIdx, pAry ) == false )
				return false;
		}

		pAry->PushBack( pList );
		
	}

	return true;
}


bool cDropScript::LoadDropMonLevelType()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/Item_DropW.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load Item_DropW.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	/// 시작 컬럼 확인
	if( tokenizer.GetNext( &str ) == false )
		return true;

	while( tokenizer.IsEnd() == false )
	{
		/// 시작컬럼 확인
		if( str.CompareNoCase( "world" ) != 0 )
			return true;

		/// 몬스터 최소 레벨
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char monsterMinLevel = (unsigned char)str.ToInt();

		/// 몬스터 최대 레벨
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char monsterMaxLevel = (unsigned char)str.ToInt();

		/// 인던 필드 구분
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char serverTypeCheck = (unsigned char)str.ToInt();

		/// 몬스터 유형
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char type = (unsigned char)str.ToInt();

		/// 추가드랍1 횟수
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char addDrop1Cnt = (unsigned char)str.ToInt();

		/// 추가드랍1 선택 비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long addDrop1Percent = str.ToInt();

		/// 추가드랍2 횟수
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char addDrop2Cnt = (unsigned char)str.ToInt();

		/// 추가드랍2 선택 비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long addDrop2Percent = str.ToInt();							

		sDropLevelTypeScript* pList = new sDropLevelTypeScript;
		mShutDownDeleteAry.PushBack( pList );

		cAry* pItemGroupAry = &pList->mItemGroup;

		bool check = true;
		while( check )
		{
			/// 시작컬럼 확인
			if( tokenizer.GetNext( &str ) == false )
				break;

			if( str.CompareNoCase( "world" ) == 0 )
				break;

			/// 아이템 그룹 인덱스
			unsigned long itemGroupIdx = str.ToInt();

			/// 아이템 그룹 선택 비율
			if( tokenizer.GetNext( &str ) == false )	{ return false; }
			unsigned long itemGroupPer = str.ToInt();

			sDropItemGroupScript* pItemGroupScript = new sDropItemGroupScript;

			pItemGroupScript->mItemGroupIdx = itemGroupIdx;
			pItemGroupScript->mItemGroupRate = itemGroupPer;

			pItemGroupAry->PushBack( pItemGroupScript );
		}

		for( unsigned char i = monsterMinLevel ; i <= monsterMaxLevel ; ++i )
		{
			//pList->mLevelMin = monsterMinLevel;
			//pList->mLevelMax = monsterMaxLevel;

			pList->mMonsterType = type;

			pList->mServerTypeCheck = serverTypeCheck;
			pList->mAddDrop1Cnt = addDrop1Cnt;
			pList->mAddDrop1Rate = addDrop1Percent;
			pList->mAddDrop2Cnt = addDrop2Cnt;
			pList->mAddDrop2Rate = addDrop2Percent;

			cAry* pAry = (cAry*)mDropMonLevelType.GetAt( type * DROP_MONSTER_TYPE_UNIT + i );
			if( pAry == NULL )
			{
				pAry = new cAry;

				if( mDropMonLevelType.Insert( type * DROP_MONSTER_TYPE_UNIT + i, pAry ) == false )
					return false;
			}

			pAry->PushBack( pList );
		}
	}

	return true;
}


bool cDropScript::LoadDropMonQuest()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/Item_DropQ.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load Item_DropQ.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sMonsterScript* pMonsterScript = NULL;
	/// 몬스터 스크립트에 정의된 퀘스트아이템드롭 해쉬맵 포인터의 포인터
	cPointrHashMap** pMap = NULL;
	cAry* pAry = NULL;
	sDropQType* pList = NULL;

	while( tokenizer.IsEnd() == false )
	{
		/// 퀘스트 번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long questIdx = str.ToInt();

		/// 몬스터 종류 index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long monsterClassIdx = str.ToInt();

		/// 아이템 index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemIdx = str.ToInt();

		/// 드랍 확률
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long dropPer = str.ToInt();

		/// 해당 몬스터를 찾아 퀘스트 아이템 정보를 넣는다.
		pMonsterScript = MONSTERSCRIPT->GetMonsterListInfo( monsterClassIdx );
		if( pMonsterScript == NULL )
			return false;

		/// 퀘스트 인덱스를 키로하는 해쉬 맵을 만든다.
		pMap = &((cPointrHashMap*)pMonsterScript->mpQuestItemDropMap);
		if( *pMap == NULL )
			*pMap = new cPointrHashMap( 999 );

		/// 퀘스트 인덱스를 키로하는 해쉬맵에 드롭 item 정보를 추가한다.
		pAry = (cAry*)((*pMap)->GetAt( questIdx ));
		if( pAry == NULL )
		{
			pAry = new cAry;
			if( (*pMap)->Insert( questIdx, pAry ) == false )
				return false;
		}

		/// 퀘스트 아이템 드롭 정보 스트럭처
		pList = new sDropQType;

		pList->mItemIdx = itemIdx;
		pList->mDropPer = dropPer;

		pAry->PushBack( pList );
	}

	return true;
}


bool cDropScript::LoadDropMonItemGroup()
{

	cFileLoader loader;
	cString pathName = "./Script/resource/item_droplist.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_droplist.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{

		/// 아이템 그룹 idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemGroupIdx = str.ToInt();	

		/// 아이템 종류 idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemIdx = str.ToInt();

		/// 아이템 선택비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemRate = str.ToInt();						

		/// 아이템 수량
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned short itemCount = (unsigned short)str.ToInt();

		sDropItemListScript* pList = (sDropItemListScript*)mDropMonItem.GetAt( itemGroupIdx );
		if( pList == NULL )
		{
			pList = new sDropItemListScript;

			if( mDropMonItem.Insert( itemGroupIdx, pList ) == false )
			{
				assert(NULL);
				return false;
			}

			DROP->InsertItemGroupRandSeed( itemGroupIdx, RANDOMTABLE->GetSeed() );
		}

		sDropItemScript* pItem = new sDropItemScript;
		pItem->mItemIdx = itemIdx;
		pItem->mItemRate = itemRate;
		pItem->mItemCount = itemCount;

		pList->mItemGroupIdx = itemGroupIdx;
		pList->mItem.PushBack( pItem );
	}

	return true;
}


bool cDropScript::LoadDropMonItemCantTime()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/item_droplimit.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_droplimit.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 아이템 idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemIdx = str.ToInt();	

		/// 제약시간
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long cantTime = str.ToInt();

		/// 드랍 제약 최소(%) Minimum drop constraints (%)
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long mindrop = str.ToInt();

		/// 드랍 제약 최대(%)
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long maxdrop = str.ToInt();

		/// 가중치(오전)
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long AM = str.ToInt();

		/// 가중치(오후)
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long PM = str.ToInt();


		if( mDropMonCantTime.Insert( itemIdx, cantTime ) == false )
			return false;
		
		//if( dbInsertDataOnly == false )
		//{
		//	unsigned long randValue = cantTime;
		//	if( cantTime > HOUR )
		//	{
		//		randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % ( cantTime / MINUTE ) + 1;
		//		randValue *= MINUTE;
		//	}
		//	else if( cantTime > MINUTE )
		//	{
		//		randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % ( cantTime / SECOND ) + 1;
		//		randValue *= SECOND;
		//	}
		//	else
		//	{
		//		randValue = (unsigned long)( RANDOMTABLE->Get( ) * 1000000 ) % cantTime + 1;
		//	}

		//	if( DROP->InsertDropMonCantTime( itemIdx, cantTime + NETWORK2->GetAccumTime() ) == false )
		//		return false;
		//}
	}

	return true;
}


bool cDropScript::LoadDropGatherMoney()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/item_gatheringmoney.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_gatheringmoney.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{

		/// 채집 종류
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long classIdx = str.ToInt();	

		/// 돈 드랍 확률
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long moneyPercent = str.ToInt();					

		/// 돈 드랍 최소 금액
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long minMoney = str.ToInt();						

		/// 돈 드랍 최대 금액
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long maxMoney = str.ToInt();						

		sDropMoneyScript* pList = new sDropMoneyScript;

		pList->mClassIdx = classIdx;	
		pList->mDropPercent = moneyPercent;
		pList->mMinMoney = minMoney;
		pList->mMaxMoney = maxMoney;

		/// 해쉬에 몬스터 별로 기록
		if( mDropGatherMoney.Insert( pList->mClassIdx, pList ) == false )
		{
			assert(NULL);
			return false;
		}
	}

	return true;
}


bool cDropScript::LoadDropGatherClass()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/item_gatheringn.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_gatheringn.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	/// 시작 컬럼 확인
	if( tokenizer.GetNext( &str ) == false )
		return true;

	while( tokenizer.IsEnd() == false )
	{
		/// 시작컬럼 확인
		if( str.CompareNoCase( "gathering" ) != 0 )
			return true;

		/// 채집 종류
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long classIdx = str.ToInt();

		/// 추가드랍1 횟수
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char addDrop1Cnt = (unsigned char)str.ToInt();

		/// 추가드랍1 선택 비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long addDrop1Percent = str.ToInt();

		/// 추가드랍2 횟수
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned char addDrop2Cnt = (unsigned char)str.ToInt();

		/// 추가드랍2 선택 비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long addDrop2Percent = str.ToInt();						

		sDropClassScript* pList = new sDropClassScript;

		pList->mClassIdx = classIdx;

		pList->mAddDrop1Cnt = addDrop1Cnt;
		pList->mAddDrop1Rate = addDrop1Percent;
		pList->mAddDrop2Cnt = addDrop2Cnt;
		pList->mAddDrop2Rate = addDrop2Percent;

		cAry* pItemGroupAry = &pList->mItemGroup;

		bool check = true;
		while( check )
		{
			/// 시작컬럼 확인
			if( tokenizer.GetNext( &str ) == false )
				break;

			if( str.CompareNoCase( "gathering" ) == 0 )
				break;

			/// 아이템 그룹 인덱스
			unsigned long itemGroupIdx = str.ToInt();

			/// 아이템 그룹 선택 비율
			if( tokenizer.GetNext( &str ) == false )	{ return false; }
			unsigned long itemGroupPer = str.ToInt();

			sDropItemGroupScript* pItemGroupScript = new sDropItemGroupScript;

			pItemGroupScript->mItemGroupIdx = itemGroupIdx;
			pItemGroupScript->mItemGroupRate = itemGroupPer;

			pItemGroupAry->PushBack( pItemGroupScript );
		}

		cAry* pAry = (cAry*)mDropGatherClass.GetAt( pList->mClassIdx );
		if( pAry == NULL )
		{
			pAry = new cAry;

			if( mDropGatherClass.Insert( pList->mClassIdx, pAry ) == false )
				return false;
		}

		pAry->PushBack( pList );
	}

	return true;
}


bool cDropScript::LoadDropGatherQuest()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/item_gatheringq.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_gatheringq.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	sGatheringList* pGatheringInfo = NULL;
	/// 채집 스크립트에 정의된 퀘스트아이템드롭 해쉬맵 포인터의 포인터
	cPointrHashMap** pMap = NULL;
	cAry* pAry = NULL;
	sDropQTypeGather* pList = NULL;

	while( tokenizer.IsEnd() == false )
	{
		/// 퀘스트 번호
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long questIdx = str.ToInt();

		/// 종류 index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long classIdx = str.ToInt();

		/// 아이템 index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemIdx = str.ToInt();

		/// 드랍 확률
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long dropPer = str.ToInt();

		/// 해당 채집오브젝트를 찾아 퀘스트 아이템 정보를 넣는다.
		pGatheringInfo = GATHERINGSCRIPT->GetGatheringInfo( classIdx );
		if( pGatheringInfo == NULL )
			return false;

		/// 퀘스트 인덱스를 키로하는 해쉬 맵을 만든다.
		pMap = &((cPointrHashMap*)pGatheringInfo->mpQuestItemDropMap);
		if( *pMap == NULL )
			*pMap = new cPointrHashMap( 999 );

		/// 퀘스트 인덱스를 키로하는 해쉬맵에 드롭 item 정보를 추가한다.
		pAry = (cAry*)((*pMap)->GetAt( questIdx ));
		if( pAry == NULL )
		{
			pAry = new cAry;
			if( (*pMap)->Insert( questIdx, pAry ) == false )
				return false;
		}

		/// 퀘스트 아이템 드롭 정보 스트럭처
		pList = new sDropQTypeGather;

		pList->mItemIdx = itemIdx;
		pList->mDropPer = dropPer;

		pAry->PushBack( pList );
	}

	return true;
}


bool cDropScript::LoadDropGatherItemGroup()
{

	cFileLoader loader;
	cString pathName = "./Script/resource/item_gatheringlist.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_gatheringlist.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{

		/// 아이템 그룹 idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemGroupIdx = str.ToInt();	

		/// 아이템 종류 idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemIdx = str.ToInt();

		/// 아이템 선택비율
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemRate = str.ToInt();						

		/// 아이템 수량
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned short itemCount = (unsigned short)str.ToInt();

		sDropItemListScript* pList = (sDropItemListScript*)mDropGatherItem.GetAt( itemGroupIdx );
		if( pList == NULL )
		{
			pList = new sDropItemListScript;

			if( mDropGatherItem.Insert( itemGroupIdx, pList ) == false )
			{
				assert(NULL);
				return false;
			}
		}

		sDropItemScript* pItem = new sDropItemScript;
		pItem->mItemIdx = itemIdx;
		pItem->mItemRate = itemRate;
		pItem->mItemCount = itemCount;

		pList->mItemGroupIdx = itemGroupIdx;
		pList->mItem.PushBack( pItem );
	}

	return true;
}


bool cDropScript::LoadDropGatherItemCantTime()
{
	cFileLoader loader;
	cString pathName = "./Script/resource/item_gatheringlimit.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load item_gatheringlimit.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 아이템 idx
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long itemIdx = str.ToInt();	

		/// 제약시간
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long cantTime = str.ToInt();

		if( mDropGatherCantTime.Insert( itemIdx, cantTime ) == false )
			return false;

		if( DROP->InsertDropGatherCantTime( itemIdx, cantTime + NETWORK2->GetAccumTime() ) == false )
			return false;
	}

	return true;
}


bool cDropScript::LoadDropExpIncrease()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/DropExpIncrease.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load DropExpIncrease.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	mNormalScriptMinLevelDifferent = 120;
	mNormalScriptMaxLevelDifferent = -120;

	mThemeScriptMinLevelDifferent = 120;
	mThemeScriptMaxLevelDifferent = -120;

	char levelDifference = 0;
	float expIncrease = 0;

	while( tokenizer.IsEnd() == false )
	{
		/// 괄호상단 서버 분류
		tokenizer.GetNext( &str );

		unsigned char type = (unsigned char)str.ToInt();

		if( type == 0 )
		{
			/// 그룹 번호 안의 세부항목 시작
			tokenizer.GetNext( &str );
			if( str != '{' )
			{
				assert( 0 && "wrong script" );
				return false;
			}

			tokenizer.GetNext( &str );
			if( str == "}" )
				continue;

			while( tokenizer.IsEnd() == false )
			{
				/// 레벨차
				levelDifference = (unsigned char)str.ToInt();
				if( levelDifference <= -120 && 120 <= levelDifference )
					return false;

				/// 경험치 증가치
				if( tokenizer.GetNext( &str ) == false )	{ return false; }
				expIncrease = str.ToFloat();

				/// 해쉬에 몬스터 별로 기록
				if( mNormalDropExpIncreaseMap.Insert( levelDifference, expIncrease ) == NULL )
				{
					assert(0);
					return false;
				}

				/// 최소,최대값을 기록
				if( mNormalScriptMinLevelDifferent > levelDifference )	{ mNormalScriptMinLevelDifferent = levelDifference; }
				if( mNormalScriptMaxLevelDifferent < levelDifference )	{ mNormalScriptMaxLevelDifferent = levelDifference; }

				tokenizer.GetNext( &str );
				if( str == "}" )
					break;
			}
		}
		else if( type == 1 )
		{
			/// 그룹 번호 안의 세부항목 시작
			tokenizer.GetNext( &str );
			if( str != '{' )
			{
				assert( 0 && "wrong script" );
				return false;;
			}

			tokenizer.GetNext( &str );
			if( str == "}" )
				continue;

			while( tokenizer.IsEnd() == false )
			{
				/// 레벨차
				levelDifference = (unsigned char)str.ToInt();
				if( levelDifference <= -120 && 120 <= levelDifference )
					return false;

				/// 경험치 증가치
				if( tokenizer.GetNext( &str ) == false )	{ return false; }
				expIncrease = str.ToFloat();

				/// 해쉬에 몬스터 별로 기록
				if( mThemeDropExpIncreaseMap.Insert( levelDifference, expIncrease ) == NULL )
				{
					assert(0);
					return false;
				}

				/// 최소,최대값을 기록
				if( mThemeScriptMinLevelDifferent > levelDifference )	{ mThemeScriptMinLevelDifferent = levelDifference; }
				if( mThemeScriptMaxLevelDifferent < levelDifference )	{ mThemeScriptMaxLevelDifferent = levelDifference; }

				tokenizer.GetNext( &str );
				if( str == "}" )
					break;
			}
		}
	}

	return true;
}

bool cDropScript::LoadDropIncreadse()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/DropIncrease.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load DropIncrease.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	mNormalDropMinLevelDifferent = 120;
	mThemeDropMinLevelDifferent = 120;

	char levelDifference = 0;
	float Increase = 0;

	while( tokenizer.IsEnd() == false )
	{
		/// 괄호상단 서버 분류
		tokenizer.GetNext( &str );

		unsigned char type = (unsigned char)str.ToInt();

		if( type == 0 )
		{
			/// 그룹 번호 안의 세부항목 시작
			tokenizer.GetNext( &str );
			if( str != '{' )
			{
				assert( 0 && "wrong script" );
				return false;
			}

			tokenizer.GetNext( &str );
			if( str == "}" )
				continue;

			while( tokenizer.IsEnd() == false )
			{
				/// 레벨차
				levelDifference = (unsigned char)str.ToInt();
				if( levelDifference <= -120 )
					return false;

				/// 증가치
				if( tokenizer.GetNext( &str ) == false )	{ return false; }
				Increase = str.ToFloat();

				/// 해쉬에 몬스터 별로 기록
				if( mNormalDropIncreaseMap.Insert( levelDifference, Increase ) == NULL )
				{
					assert(0);
					return false;
				}

				/// 최소,최대값을 기록
				if( mNormalDropMinLevelDifferent > levelDifference )	{ mNormalDropMinLevelDifferent = levelDifference; }

				tokenizer.GetNext( &str );
				if( str == "}" )
					break;
			}
		}
		else if( type == 1 )
		{
			/// 그룹 번호 안의 세부항목 시작
			tokenizer.GetNext( &str );
			if( str != '{' )
			{
				assert( 0 && "wrong script" );
				return false;;
			}

			tokenizer.GetNext( &str );
			if( str == "}" )
				continue;

			while( tokenizer.IsEnd() == false )
			{
				/// 레벨차
				levelDifference = (unsigned char)str.ToInt();
				if( levelDifference <= -120  )
					return false;

				/// 경험치 증가치
				if( tokenizer.GetNext( &str ) == false )	{ return false; }
				Increase = str.ToFloat();

				/// 해쉬에 몬스터 별로 기록
				if( mThemeDropIncreaseMap.Insert( levelDifference, Increase ) == NULL )
				{
					assert(0);
					return false;
				}

				/// 최소,최대값을 기록
				if( mThemeDropMinLevelDifferent > levelDifference )	{ mThemeDropMinLevelDifferent = levelDifference; }

				tokenizer.GetNext( &str );
				if( str == "}" )
					break;
			}
		}
	}

	return true;
}

unsigned long cDropScript::GetDropGatherCantTime( unsigned long itemIdx )
{ 
	unsigned long cantTime = 0;
	mDropGatherCantTime.GetAt( &cantTime, itemIdx );
	return cantTime;
}


float cDropScript::GetDropExpIncrease( bool isTheme, char levelDifference )
{	
	char diff = levelDifference;
	if( isTheme == true )
	{
		if( diff < mThemeScriptMinLevelDifferent )
			return 0.0f;
		if( diff > mThemeScriptMaxLevelDifferent )
			diff = mThemeScriptMaxLevelDifferent;

		cDropExpIncrease::cIterator key = mThemeDropExpIncreaseMap.Find( diff );
		if( key == mThemeDropExpIncreaseMap.End() )
			return 0.0f;

		return (*key).mSecond;
	}
	else
	{
		if( diff < mNormalScriptMinLevelDifferent )
			return 0.0f;
		if( diff > mNormalScriptMaxLevelDifferent )
			diff = mNormalScriptMaxLevelDifferent;

		cDropExpIncrease::cIterator key = mNormalDropExpIncreaseMap.Find( diff );
		if( key == mNormalDropExpIncreaseMap.End() )
			return 0.0f;

		return (*key).mSecond;
	}
}

float cDropScript::GetDropIncrease( bool isTheme, char levelDifference )
{
	char diff = levelDifference;
	if( isTheme == true )
	{
		if( diff < mThemeDropMinLevelDifferent )
			diff = mThemeDropMinLevelDifferent;

		cDropIncrease::cIterator key = mThemeDropIncreaseMap.Find( diff );
		if( key == mThemeDropIncreaseMap.End() )
			return 1.0f;

		return (*key).mSecond;
	}
	else
	{
		if( diff < mNormalDropMinLevelDifferent )
			diff = mNormalDropMinLevelDifferent;

		cDropIncrease::cIterator key = mNormalDropIncreaseMap.Find( diff );
		if( key == mNormalDropIncreaseMap.End() )
			return 1.0f;

		return (*key).mSecond;
	}
}


bool cDropScript::LoadPartyCalcInfo()
{
	cFileLoader loader;
	cString pathName = "./Script/Resource/Party_Exp.txt";

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load Party_Exp.txt" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// index
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		unsigned long index = (unsigned long)str.ToInt();	

		/// value
		if( tokenizer.GetNext( &str ) == false )	{ return false; }
		float value = str.ToFloat();

		if( mPartyCalcMap.Insert( index, value ) == false )
			return false;
	}
	return true;
}


float cDropScript::GetPartyCalcInfo( unsigned long idx )
{
	cHashMap::cIterator i = mPartyCalcMap.Find( idx );
	if( i == mPartyCalcMap.End() )
	{
		assert( NULL );
		return 0.0f;
	}

	return (float)((*i).mSecond);
}
