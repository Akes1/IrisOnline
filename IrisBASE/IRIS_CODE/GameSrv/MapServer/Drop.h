/* ==========================================================================
* 파    일 : Drop.h
* 목    적 : 드랍 시스템
* 작 성 자 : 박경희
* 작 성 일 : 2007-02-20
* 주의사항 :
*===========================================================================*/
#pragma once

#include <map>
#include "RangeCheck.h"
#include "Drop_Common.h"

class cDropScript;
class cMonster;
class cPlayer;
class cGathering;

struct sDropNType;
struct sDropSWType;
struct sPlayerDamage;
struct sDropNTypeGather;

const unsigned long		DROP_LOG_WRITE_NORMAL = 10 * MINUTE;	/// 정상적인 로그 기록시간
const unsigned long		DROP_LOG_WRITE_CONTINUE = 1 * MINUTE;	/// 로그양이 많아 한번에 기록안된경우 다음 저장시간
const unsigned long		DROP_LOG_WRITE_PROCESS_CNT = 400;		/// 한프로세스에서 db에 기록할 로그 최대기준량 - 넘는양까지 고려해서 설정

struct sDropItemLog
{
	unsigned long		mMonIdx;
	unsigned long		mMonDieCnt;
	unsigned long		mMonMoneyCnt;

	tHashMap<unsigned long, unsigned long> mItemMap;
	tHashMap<unsigned long, unsigned long> mItemNoDropMap;

	sDropItemLog()
	{
		mMonIdx = 0;
		mMonDieCnt = 0;
		mMonMoneyCnt = 0;
	}
};

class cDrop
{

public:

	static cDrop*	mpDrop;

	cDrop(void);
	~cDrop(void);

	bool			Init();
	void			Release();
	void			ProcessDrop( unsigned long elapsedTime, unsigned long accumTime );

	/// 몬스터
	void			MonsterDrop( unsigned long monsterIdx, unsigned long playerIdx, unsigned long partyIdx );
	void			MonsterQuestDrop( unsigned long monsterIdx, unsigned long playerIdx, unsigned long partyIdx );
	/// 레벨차에 따른 exp, sxp 증감치 포함된 실제 경험치
	float			MonsterDropExpIncrease( float takeExp, unsigned char playerLevel, unsigned char monsterLevel );

	/// 채집
	void			GatherDrop( cGathering* pGathering, unsigned long playerIdx );

	bool			InsertDropMonCantTime( unsigned long itemIdx, sDropLimit* pDropLimit )	{ return mDropMonCantTime.Insert( itemIdx, pDropLimit ); }
	sDropLimit*		GetDropMonCantTime( unsigned long itemIdx )	{ return (sDropLimit*)mDropMonCantTime.GetAt( itemIdx ); }
	bool			InsertDropGatherCantTime( unsigned long itemIdx, unsigned long cantTime )	{ return mDropGatherCantTime.Insert( itemIdx, cantTime ); }

	void			InsertItemGroupRandSeed( unsigned long itemGroup, unsigned long randSeed );

private:

	/// 몬스터 
	void			DropTwitter( cMonster* pMonster, unsigned long playerIdx, unsigned long partyIdx );									/// 트위터 처리
	void			DropMonsterValue( cMonster* pMonster );								/// money, exp, sxp 드랍
	void			DropMonsterClass( unsigned long monsterIdx, float penalty );						/// 종류별(일반) 드랍
	void			DropMonsterLevelType( unsigned char level, unsigned char type, float penalty );	/// 레벨/타입별(월드) 드랍
	void			DropMonQuestType( cMonster* pMonster, cPlayer* pPlayer );			/// 퀘스트 아이템 드랍
	bool			DropMonCheck( cMonster* pMonster, cPlayer* pPlayer );				/// 드랍 조건 체크
	void			DropMonItemSelect( sDropItemListScript* pDropItemList, float penalty );			/// 아이템선택

	/// 채집
	void			DropGatherValue( cGathering* pGathering, unsigned long playerIdx );	/// money 드랍
	void			DropGatherClass( cGathering* pGathering, unsigned long playerIdx );	/// 종류별(일반) 드랍
	void			DropGatherQuest( cGathering* pGathering, unsigned long playerIdx );	/// 퀘스트 아이템 드랍
	void			DropGatherItemSelect( sDropItemListScript* pDropItemList );			/// 아이템선택

private:

	cDropScript*	mpDropScript;

	typedef tArray<void*> cAry;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned long, unsigned long> cHashMap;
	typedef tPointerHashMap<unsigned long, void*> cPHashMap;

	cHashMap		mDropItemCntMap;
	cHashMap		mDropNoItemCntMap;

	/// 몬스터를 공격한 유저들의 파티 리스트
	cHashSet		partyIdxSet;

	/// 충돌 범위 체크
	cRangeCheck		rangechk;

	/// 드랍 내역
	/// <unsigned long monsterIdx, sDropItemLog* pDropLog>
	cPHashMap		mDropLogMap;

	unsigned long	mLogWriteTime;

	typedef tPointerHashSet<sDropItemScript*> cSelectSet;
	cSelectSet		mTempSelectItemSet;

	/// 아이템별 드랍 제약(시간)
	cPHashMap		mDropMonCantTime;
	cHashMap		mDropGatherCantTime;

	unsigned long	mQuestSeed;

	/// <unsigned long itemGroup, unsigned long randSeed>
	cHashMap		mItemGroupRandSeed;
		
public:

	unsigned long	mDropCnt;
};

#define DROP cDrop::mpDrop
