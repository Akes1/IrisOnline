/* ==========================================================================
* 파    일 : Party.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-05
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Party_Common.h"

const float PARTY_RANGE = 4000; // 40M

// 주사위 굴리기 참여 멤버
struct sDiceMember
{
	wchar_t         mCharName[MAX_NAME_BUFFER_SIZE];
	unsigned long   mUserIdx;
	unsigned short  mRandom;                                // 랜덤값
	bool            mRoll;                                  // 굴리기/포기 선택 여부

	sDiceMember()
	{
		memset(this, 0, sizeof(sDiceMember));
	}
};

class cParty
{
public:
	cParty( );
	~cParty( );

	// 메모리풀을 이용하므로 new, delete 오버로딩
	void* operator new                      ( size_t n );
	void  operator delete                   ( void* ptr, size_t n );

	void              Init                  ( unsigned long index, unsigned long leaderIdx, PARTY_DIVIDE_TYPE type, unsigned long accumTime );

	void              SetIndex              ( unsigned long index ) { mIndex = index; }
	void              SetDivideType         ( PARTY_DIVIDE_TYPE type ) { mDivideType = type; }

	unsigned long*    GetUserArr            ( ) { return mUserArr; }
	unsigned int      GetCount              ( ) { return mCount; }

	bool              IsUser                ( unsigned long userIdx );
	bool              AddUser               ( unsigned long userIdx );
	bool              DeleteUser            ( unsigned long userIdx );

	sPartyUserData*   GetUserData           ( ) { return mUserDat; }
	bool              SetUserData           ( sPartyUserData* userData, unsigned long tickCount );
	bool              SetUserData           ( cPlayer* player, unsigned long tickCount );

	bool              IsLeader              ( unsigned long userIdx ) { return (mLeaderIndex == userIdx); }
	void              SetLeader             ( unsigned long userIdx ) { mLeaderIndex = userIdx; }
	unsigned long     GetLeader             ( ) { return mLeaderIndex; }
	unsigned long     NextLeader            ( );

	// [순서분배]의 유저인덱스 획득
	unsigned long     GetTurnUser           ( unsigned short mapNumber, float x, float y );

	// [랜덤분배]의 유저인덱스 획득
	unsigned long     GetRandomUser         ( unsigned short mapNumber, float x, float y );

	void              SetOwnUserNum         ( unsigned int num ) { mOwnUserNum = num; }
	unsigned int      GetOwnUserNum         ( ) { return mOwnUserNum; }

	unsigned long     GetIndex              ( ) { return mIndex; }
	PARTY_DIVIDE_TYPE GetDivideType         ( ) { return mDivideType; }

	// 경험치 drop 처리
	void              AddDropValue          ( unsigned long maxHP, unsigned long damage, float money, float exp, float sxp );
	// 조건을 만족하는 파티원 중 모두에게 드랍 수치 분배
	void              DivideDropValue       ( unsigned long monsterIdx, unsigned char monsterLevel );

	// 갱신 시간 저장
	void              SetLastAccumTime      ( unsigned long accumTime ) { mLastAccumTime = accumTime; }
	unsigned long     GetLastAccumTime      ( ) { return mLastAccumTime; }

	// 주사위 굴리기
	unsigned long     GetDiceEndTime        ( ) { return mDiceEndTime; }
	unsigned int      GetDiceCount          ( ) { return mDiceCount; }
	sDiceMember*      GetDiceMember         ( unsigned int index ) { return &mDiceMember[index]; }
	sDiceMember*      GetDiceMember         ( unsigned long userIdx );
	bool              IsDiceRollEnd         ( ) { return ( mDiceCount > 0 && mDiceCount == mDiceRollCnt ) ? true : false; }

	bool              DiceStart             ( );
	void              DiceRelease           ( );
	int               DiceRollReq           ( unsigned long userIdx, unsigned short randVal );
	void              SetDiceRoll           ( unsigned long userIdx, unsigned short randVal );

	unsigned long     GetLostPlayer         ( unsigned long accumTime );

	void              SendPartyAddoneSyn    ( sPartyUserData* userData );
	void              SendPartyOutSyn       ( unsigned long playerIdx );
	void              SendPartyThrowSyn     ( unsigned long playerIdx );

	void              SendPartyLeaderSyn    ( unsigned long leaderIdx );
	void              SendPartyDivideType   ( char type );
	void              SendPartyDataLevelSyn ( unsigned long playerIdx, unsigned char level );
	void              SendPartyMapchangeSyn ( unsigned long playerIdx, short map, short room, DWORD channelnum );
	void              SendPartyDeleteSyn    ( );
	void              SendChat              ( sInventory* inventory, wchar_t* name, wchar_t* message, unsigned long characterIdx, unsigned short job );
	void              SendUpdate            ( );

private:
	unsigned long     mIndex;					// 파티 인덱스
	PARTY_DIVIDE_TYPE mDivideType;				// 파티 분배방식
	unsigned long     mUserArr[MAX_PARTYUSER];	// 파티유저 인덱스
	sPartyUserData    mUserDat[MAX_PARTYUSER];	// 파티유저 데이타
	unsigned long     mUserTsp[MAX_PARTYUSER];	// 파티유저 타임 - 윈도우즈 타이머 사용.
	unsigned int      mCount;					// 파티인원수
	unsigned long     mLeaderIndex;				// 파티장 인덱스
	int               mOwnUserNum;				// 아이템 소유권 유저 배열 번호

	/// 파티에 drop된 money
	float             mDropMoney;
	float             mDropExp;
	float             mDropSxp;

	unsigned long     mMonsterMaxHP;			// 드롭보상할 몬스터의 최대 hp
	unsigned long     mDamage;					// 몬스터가 받은 데미지
	unsigned long     mLastAccumTime;			// 파티 갱신 시간

	/// 주사위 굴리기
	sDiceMember       mDiceMember[MAX_PARTYUSER];
	unsigned short    mDiceCount;				// 참여 멤버 수
	unsigned short    mDiceRollCnt;				// 굴리기/포기 선택한 인원 수
	unsigned long     mDiceEndTime;
};
