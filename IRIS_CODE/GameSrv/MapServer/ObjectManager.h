/* ====================================================================
* 파    일 :	
* 목    적 :	
* 작 성 자 :	정성대
* 작 성 일 :	2006.08.29
*
* 추가내용 :
*		추가날짜	작업자	추가내용
*		2006.08.29	정성대	생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once

#include "Player.h"
#include "Npc_Common.h"
#include "IndexGenerator.h"
#include "Gathering_Common.h"
#include "Monster_Common.h"
#include "PlayerNamePool.h"

class cBaseObject;
class cMonster;
class cNpc;
class cItem;
class cTotem;
class cGridManager;
class cStatusCalc_Server;
class cDamageCalc;
class cStatusScript;
class cMonsterScript;
class cLevelScript;
class cNPCScript;
class cTotemScript;
class cVehicleScript;
class cGatheringScript;
class cGathering;
class cFlag;
class cFlagPoint;
class cShortCutScript;
class cUserPortal;
class cTutorialScript;
class cTutorialSkillScript;

struct PerSocketContext;
struct sObject;

enum eOBJECTTYPE;

/// 추적 위치 번호별 위치값
const NiPoint2 FOLLOW_POS_VALUE[FOLLOW_POS_MAX] =
{
	NiPoint2( -1, 1 ),	NiPoint2( 0, 1 ),	NiPoint2( 1, 1 ),
	NiPoint2( -1, 0 ),	NiPoint2( 0, 0 ),	NiPoint2( 1, 0 ),
	NiPoint2( -1, -1 ),	NiPoint2( 0, -1 ),	NiPoint2( 1, -1 )
};

class cObjectManager
{
	/// pool
	typedef	tPool<cPlayer>	cPlayerPool;
	typedef	tPool<cMonster>	cMonsterPool;
	typedef	tPool<cNpc>		cNpcPool;
	typedef	tPool<cItem>	cItemPool;
	typedef	tPool<cTotem>	cTotemPool;
	typedef	tPool<cGathering>	cGatheringPool;
	typedef	tPool<cUserPortal>	cUserPortalPool;

	/// hashmap
	typedef tPointerHashMap<unsigned long, cPlayer*>	cPlayerMap;
	typedef tPointerHashMap<unsigned long, void*>		cMonsterMap;
	typedef tPointerHashMap<unsigned long, cNpc*>		cNpcMap;
	typedef tPointerHashMap<unsigned long, cItem*>		cItemMap;
	typedef tPointerHashMap<unsigned long, cTotem*>		cTotemMap;
	typedef tPointerHashMap<unsigned long, void*>		cGatheringMap;
	typedef tPointerHashMap<unsigned long, void*>		cPointHashMap;

public:
	static cObjectManager*	mpObjectManager;

public:
	cObjectManager(void);
	~cObjectManager(void);

	bool			Init( int serverType );
	void			StageScriptInit( unsigned short mapType );
	void			Release();
	void			ThemeDataClear();
	void			Process( unsigned long elapsedTime, unsigned long accumTime );
	
	/// pool alloc & free
	cPlayer*		AllocPlayer();
	void			FreePlayer( cPlayer* ptr );

	cMonster*		AllocMonster();
	void		    FreeMonster( cMonster* ptr );

	cNpc*			AllocNpc();
	void			FreeNpc( cNpc* ptr );

	cItem*          AllocItem( );
	void			FreeItem( cItem* ptr );

	cTotem*         AllocTotem( );
	void			FreeTotem( cTotem* ptr );

	cGathering*     AllocGathering( );
	void			FreeGathering( cGathering* ptr );

	cUserPortal*	AllocUserPortal( );
	void			FreeUserPortal( cUserPortal* ptr );

	/// add & remove functions
	cPlayer*		GetPlayer    ( unsigned long playerIdx ) { return mpPlayerMap->GetAt( playerIdx ); }
	cPlayer*		GetPlayer    ( wchar_t* charName );
	cPlayer*		AddPlayer    ( unsigned long connectionIdx, unsigned long playerIdx, wchar_t* playerName, unsigned long money, unsigned long deposit );
	bool			RemovePlayer ( unsigned long playerIdx );

	/// 061201 PKH 몬스터 추가
	cMonster*		GetMonster    ( unsigned long monsterIdx ) { return (cMonster*)mpMonsterMap->GetAt( monsterIdx ); }
	cMonster*		AddMonster    ( unsigned long monsterIdx, unsigned long calssIdx, unsigned long regenIdx, unsigned short mapNumber, unsigned char levelMode );
	bool			RemoveMonster ( unsigned long monsterIdx );
	void			InsertDeleteMonster( unsigned long monsterIdx );
	void			MonJobProcess( cMonster* pMonster, eACTION_ID actionid, eACTION_CHANGE actionstate );
	cMonster*		GetDeathMonster ( unsigned long monsterIdx )	{ return (cMonster*)mpDeathMonsterMap->GetAt( monsterIdx ); }
	void			AddDeathMonster( cMonster* pMonster );
	void			RecoverDeathMonster( cMonster* pMonster );

	cNpc*			GetNpc        ( unsigned long npcIdx ) { return mpNpcMap->GetAt( npcIdx ); }
	cNpc*			AddNpc        ( int uniqueIdx, unsigned long classIdx, unsigned short mapNumber, float x, float y, float dir );
	bool			RemoveNpc     ( unsigned long npcIdx );

	cItem*			GetItem       ( unsigned long itemIdx ) { return mpItemMap->GetAt( itemIdx ); }
	cItem*			AddItem       ( unsigned long itemIdx, cBaseObject* baseObject, bool getAuto=false );
	bool			RemoveItem    ( unsigned long itemIdx );

	cTotem*			GetTotem		( unsigned long totemIdx ) { return mpTotemMap->GetAt( totemIdx ); }
	cTotem*			AddTotem		( unsigned long calssIdx, unsigned long partyIdx, unsigned long partyUnionIdx, NiPoint2 pos, sObject attacker,
		eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType, unsigned long skillIdx, float direction, unsigned char applyCnt );
	cTotem*			PvPAddTotem		( unsigned char teamType, unsigned long calssIdx, unsigned short pvpIdx, NiPoint2 pos, eAPPLYTYPE applyType, eATTRIBUTETYPE attributeType );
	bool			RemoveTotem		( unsigned long totemIdx );
	void			InsertDeleteTotem( unsigned long totemIdx );

	cGathering*		GetGathering		( unsigned long gatheringIdx ) { return (cGathering*)mpGatheringMap->GetAt( gatheringIdx ); }
	cGathering*		AddGathering		( unsigned long regenIdx, unsigned long gatheringIdx, unsigned short mapNumber,
		float posX, float posY, float direction, float height );
	bool			RemoveGathering		( unsigned long gatheringIdx );
	void			InsertDeleteGathering( unsigned long gatheringIdx );

	cUserPortal*	GetUserPortal		( unsigned long objectIdx ) { return (cUserPortal*)mpUserPortalMap->GetAt( objectIdx ); }
	cUserPortal*	AddUserPortal		( unsigned long mapChangePosIdx, unsigned long creatorIdx, unsigned short mapNumber, float posX, float posY );
	bool			RemoveUserPortal	( unsigned long objectIdx );
	void			InsertDeleteUserPortal( unsigned long objectIdx );

	/// 070622 PKH 접속한 플레이어에게만 몬스터 모델 정보를 보내준다.
	void			SendMsgMonsterModelNumberToPlayer( ULONG_PTR socketContext, unsigned long mapNum );
	/// 070622 PKH 접속한 플레이어에게만 NPC 모델 정보를 보내준다.
	void			SendMsgNpcModelNumberToPlayer( ULONG_PTR socketContext, unsigned long mapNum );

	cBaseObject*	GetObject( unsigned char objectType, unsigned long idx );

	/// 071029 KCJ NPC맵 반환
	void* GetNpcMap  ( ) { return mpNpcMap;  }

	/// 071113 KCJ ITEM맵 반환
	void* GetItemMap ( ) { return mpItemMap; }

	/// 070627 PKH NPC데이터를 pool에 등록한다.
	void			LoadNpcInfo();

	/// 070817 PKH 목표 지점을 기준으로 범위안에 해당하는 플레이어와 갯수를 알려줌
	//void			PlayerInCenterPoint( unsigned short centerMapNum, NiPoint3 centerPoint, float range, tArray<sObject>* targetAry, bool applyHP, 
	//	unsigned char maxTargetCnt = TARGET_CNT_MAX );
	///// 080429 PKH 결투 시스템등 player가 player를 공격하는 형태
	//void			PlayerInCenterPoint( unsigned short centerMapNum, NiPoint3 centerPoint, float range, tArray<sMultiTarget>* targetAry,
	//	unsigned long attackerIdx, bool applyHP, unsigned char maxTargetCnt = TARGET_CNT_MAX );
	/// 070817 PKH 목표 지점을 기준으로 범위안에 해당하는 몬스터와 갯수를 알려줌
	//void			MonsterInCenterPoint( unsigned short centerMapNum, NiPoint3 centerPoint, float range, tArray<sObject>* targetAry, bool applyHP, 
	//	unsigned char maxTargetCnt = TARGET_CNT_MAX );

	//void			MonsterInBox( unsigned short centerMapNum, float direction, NiPoint3 centerPos3, 
	//	float rangeX, float rangeY,  tArray<sMultiTarget>* targetAry, bool applyHP, unsigned char maxTargetCnt = TARGET_CNT_MAX );

	float			ObjectSizeRange( cBaseObject* object1, cBaseObject* object2, float range );

	/// 기준점으로 추적 퍼짐위치 좌표값 계산
	NiPoint2		CalcFollowPos( unsigned char followPos, NiPoint2 centerPos, float range );

	/// npc와 대화 가능 여부를 판별해 준다.
	unsigned long	IsNpcTalk( unsigned long npcIdx, cPlayer* pPlayer, eNPC_TALK_TYPE npcTalkType, unsigned long value, ULONG_PTR socketContext = 0 );

	void			SendNotice( char* msg, unsigned long size );

	/// 치트
	void			CheatRemoveItemRange( cBaseObject* obj, float radius );
	void			CheatRemoveMonsterRange( cBaseObject* pCenter, float size, bool isCheatKill = false );
	void			CheatRemoveMonster( unsigned long monsterIdx );
	void			CheatKillMonster( unsigned long monsterIdx, unsigned long playerIdx );

	void			QuestRemoveMonster( unsigned long monsterIdx );

	//
	void			SendPVPNotice( unsigned char leftMinute, unsigned short PvPType, unsigned long aryPos );
	void            SendMsgSynPVPReadyQuestion( unsigned short pvpType, unsigned long aryPos );
	//void            SendMsgSynPVPReadyClose( unsigned long aryPos );

	//
	void			SendTimeEventQuest();

	/// GM 이벤트
	void		    AddGMEvent( sGMEventInfo eventInfo );
	void			DelGMEvent( unsigned long DBIndex, unsigned long influenceIdx );

protected:
	// Memory Pool - cPlayerNamePool class.
	cPlayerNamePool*    mpPlayerNamePool;

	/// Objects Pool & Map
	cPlayerPool*		mpPlayerPool;
	cMonsterPool*		mpMonsterPool;
	cNpcPool*			mpNpcPool;
	cItemPool*			mpItemPool;
	cTotemPool*			mpTotemPool;
	cGatheringPool*		mpGatheringPool;
	cUserPortalPool*	mpUserPortalPool;

	
	cPlayerMap*			mpPlayerMap;
	cMonsterMap*		mpMonsterMap;
	cNpcMap*			mpNpcMap;
	cItemMap*			mpItemMap;
	cTotemMap*			mpTotemMap;
	cGatheringMap*		mpGatheringMap;
	cPointHashMap*		mpUserPortalMap;

	cMonsterMap*		mpDeathMonsterMap;

	typedef tArray<unsigned long> cArray;
	typedef tHashSet<unsigned long> cHashSet;

	/// 070115 PKH 데미지 계산기
	cStatusCalc_Server*	mpStatusCalc;
	cDamageCalc*		mpDamageCalc;
	cStatusScript*		mpStatusScript;

	/// 070130 PKH 몬스터
	cMonsterScript*		mpMonsterScript;
	cArray				mDelMonster;
	cArray				mDelMonsterMapOnly;

	/// 레벨 스크립트
	cLevelScript*		mpLevelScript;

	cNPCScript*			mpNpcScript;

	/// 081212 PKH 토템
	cTotemScript*		mpTotemScript;
	cIndexGenerator		mTotemIdxGen;
	cArray				mDelTotem;

	/// 081212 PKH 채집
	cGatheringScript*	mpGatheringScript;
	cIndexGenerator		mGatheringIdxGen;
	cArray				mDelGathering;

	cIndexGenerator		mUserPortalIdxGen;
	cArray				mDelUserPortal;

	/// 090106 PKH 탈것
	cVehicleScript*		mpVehicleScript;

	/// 퀵슬롯 디폴트 세팅
	cShortCutScript*	mpShortCutScript;

	// 튜토리얼 스크립트
	cTutorialScript*	mpTutorialScript;

	// 튜토리얼 스킬 스크립트
	cTutorialSkillScript* mpTutorialSkillScript;
};

#define OBJECTMANAGER	cObjectManager::mpObjectManager
