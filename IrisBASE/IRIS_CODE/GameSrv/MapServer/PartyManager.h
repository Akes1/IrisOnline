/* ==========================================================================
* 파    일 : PartyManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-05
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Monster.h"
#include "Party.h"
#include "PartyJoinPool.h"

const unsigned long PARTY_UPDATETIME = 10000;
const unsigned long PARTY_DELETETIME = 60000;

class cPartyManager
{
	static cPartyManager* mpSingleton;

public:
	cPartyManager( );
	~cPartyManager( );

	bool    Init               ( );
	void    Process            ( unsigned long elapsedTime, unsigned long accumTime );
	void    Release            ( );

	cParty* AllocParty         ( ) { return mPartyPool.Alloc(); }
	void    FreeParty          ( cParty* party ) { mPartyPool.Free( party ); }

	int     CheckPlayer        ( cPlayer* player );

	// 파티생성
	cParty*	AddParty           ( unsigned long partyIndex, unsigned long leaderIdx, unsigned long playerIdx );
	cParty*	GetParty           ( unsigned int partyIdx );
	void    DelParty           ( cParty** party );								// 파티 삭제

	void    AddRestore         ( unsigned long playerIdx, unsigned int partyIdx );
	void    SetRestore         ( cPlayer* player );
	void    DelRestore         ( unsigned long playerIdx );

	// 파티신청응답시 제대로 된 연결인지 검사하기 위함.
	// 신청정보 저장 ( 신청받은 유저, 신청한 유저 인덱스 )
	PartyJoin* SearchJoin      ( wchar_t* name );
	PartyJoin* GetJoin         ( wchar_t* name );
	void       ReleaseJoin     ( PartyJoin* join );
	void       ClearRequest    ( cPlayer* player );	// 신청받는 유저 신청 자동 취소

	// 몬스터 사냥 : 파티원 갱신
	void    SendToPartyHunt    ( cMonster* monster, cPlayer* player, unsigned int racegender );
	void    SendToJobChange    ( cPlayer* player );

	// 주사위 굴리기
	void    PartyDiceStart     ( cPlayer* player );
	void    PartyDiceResult    ( cPlayer* player, unsigned short value );

	void    SendDiceResult     ( cParty* party );
	void    SendToPartyAll     ( cParty* party, char* msg, unsigned long length );

	bool    ResPartyList       ( cPlayer* player, cParty* party );
	bool    ResPartyAddOne     ( cPlayer* player, cParty* party );
	
	// 파티원 탈퇴처리 (파티탈퇴, 게임종료)
	void    PartyDelPlayer     ( cParty* party, unsigned long playerIdx );
	void    PartyOutPlayer     ( cPlayer* player );
	void    PartyThrowPlayer   ( cPlayer* leader, unsigned long playerIdx );
	void    PartyChangeLeader  ( cPlayer* leader, unsigned long playerIdx );
	void    PartyChangeDivide  ( cPlayer* leader, PARTY_DIVIDE_TYPE type );
	void    PartyMapIn         ( cPlayer* player );

	void    PostSynParty       ( cParty* party );
	void    PostAskParty       ( unsigned long partyIdx, cPlayer* player, wchar_t* playerName );
	void    PostRepParty       ( unsigned long leaderIdx, wchar_t* playerName, unsigned long error, unsigned char replay );
	void    PostAddParty       ( unsigned long partyIdx, sPartyUserData* leader, sPartyUserData* player );
	void    PostDelParty       ( unsigned long partyIdx );
	void    PostAddUser        ( unsigned long partyIdx, cPlayer* player );
	void    PostDelUser        ( unsigned long partyIdx, unsigned long leaderIdx, unsigned long playerIdx );
	void    PostOutUser        ( unsigned long partyIdx, unsigned long leaderIdx, unsigned long playerIdx );
	void    PostSynLeader      ( cParty* party );
	void    PostSynDivideType  ( cParty* party );
	//void    PostSynLevelData   ( unsigned long partyIdx, cPlayer* player );
	void    PostSynMapData     ( unsigned long partyIdx, cPlayer* player );
	void    PostSynUser        ( unsigned long partyIdx, unsigned long playerIdx, long gotoCID );

	void    PostSynDiceStart   ( unsigned long partyIdx );
	void    PostSynDiceResult  ( unsigned long partyIdx, unsigned long playerIdx, unsigned short value );

private:
	typedef tPool<cParty>                           cPartyPool;
	typedef tPointerHashMap<unsigned long, cParty*> cPartyMap;
	typedef tHashMap<unsigned long, unsigned long>  cJoinMap;

	cPartyPool      mPartyPool;		// 전체파티 풀
	cPartyMap       mPartyMap;		// 전체 파티정보 저장
	cJoinMap        mRestore;		// 파티정보 복구

	// Memory Pool - cPartyJoinPool class.
	cPartyJoinPool* mpPartyJoinPool;

public:
	static cPartyManager* Get ( ) { return mpSingleton; }
};

#define PARTYMAN cPartyManager::Get( )
