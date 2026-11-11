/* ==========================================================================
* 파    일 : PartyManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Player_Common.h"
#include "Party_Common.h"

const unsigned long PARTY_WAIT_TIME = 10000;
const unsigned long PARTY_POSUPDATE = 1000;
const unsigned long PARTY_DATAUPDATE = 100;

/// 네트워크 락
enum 
{
	eLOCK_PARTYADD,
	eLOCK_PARTYOUT,
	eLOCK_PARTYCHANGELEADER,
	eLOCK_PARTYCHANGEDIVIDE,
	eLOCK_PARTYTHROW,
};

/// 파티 매니저
class cPlayer;
class cPartyManager
{
public:
	static cPartyManager* mpSingleton;

public:
	cPartyManager();
	~cPartyManager();

	void Open();
	void Close();
	void Clear();
	void Process( unsigned long accumTime );
	
	/// 네트워크 메시지
	void NetworkMsgParser( char* msg );

	void RequestPartyAdd( LPCTSTR targetName );
	void RequestChangeLeader();
	void RequestChangeLeader( unsigned long playerIdx );
	void RequestPartyOut();
	void RequestThrow();
	void RequestThrow( unsigned long playerIdx );

	void RequestChangeDivide( PARTY_DIVIDE_TYPE type );

	//
	bool IsHeroLeader();
	bool IsHeroParty( unsigned long userIndex );

	//
	void UpdateLevel( unsigned long userIndex, unsigned char level );

	// ui 클릭으로 인한 타겟팅
	void SetTargetByUI( unsigned long userIndex );
	void SetTargetByArrIndex( unsigned int i );

	// hero의 타겟팅으로 인함
	void SetTargetByHero( unsigned long userIndex );

	// 타겟팅 클리어
	void ClearTarget();

	//
	void ShowUI();
	void HideUI();

	//
	void ChangeDisplayType( unsigned char type );

	void ClearAllBuff( unsigned long objectIdx );
	void AddBuff( unsigned long objectIdx, unsigned long uniqueIdx, unsigned long influenceIdx );
	void DeleteBuff( unsigned long objectIdx, unsigned long uniqueIdx );
	void DeleteDeBuff( unsigned long objectIdx, unsigned long uniqueIdx );

	LPCTSTR GetSelectUserName();
	inline unsigned long GetLeaderIndex() { return mLeaderIndex; }

	bool IsDice() { return mIsDice; }

	sPartyUserData*		GetPartyInfo( unsigned int index );
	sPartyUserData*		GetPartyInfoByUserIndex( unsigned long userIndex );
	int					GetPartyCount(){ return mCount; }
	PARTY_DIVIDE_TYPE	GetPartyDivide() { return mDivideType; }

public:
	static cPartyManager* GetSingleton();

private:
	bool AddPlayer( int i, sPartyUserData* data );
	bool DeletePlayer( int i );
	void UpdatePlayer( unsigned int i, unsigned long userIndex, unsigned char level, unsigned int maxhp, unsigned int maxmp,
					   unsigned int hp, unsigned int mp, float posX, float posY );

	bool UpdateBuff( unsigned int i, sInfluenceClient* data, long count );
	bool UpdateBuff( unsigned int i, cPlayer* player );

	void UpdateMap( unsigned int i, unsigned short mapnum, unsigned short roomnum, DWORD channelnum );
	void UpdateJob( unsigned int i, ePLAYER_JOB job );

	void UpdateMouseHover();
	void UpdateMinimap();

	int FindUser( unsigned long playerIndex );
	
	void ShowStateUI( int i );
	void HideStateUI( int i );

private:
	//
	sPartyUserData	mParty[MAX_PARTY];

	///
	unsigned long mPartyIndex;

	/// 파티장 유저 인덱스
	unsigned long mLeaderIndex;

	/// Hero를 제외하고 셋팅된 파티 인원수
	int mCount;

	/// 파티 분배방식
	PARTY_DIVIDE_TYPE mDivideType;

	///
	unsigned long mLastAccumTime;

	/// HP, MP 갱신 시간
	unsigned long mHPMPAccumTime;

	// 파티신청 임시 유저이름
	cStringT	mTempTargetName;

	// 선택되어있는 정보
	int mSelectArrIndex;

	// 주사위 굴리기 중인지
	bool mIsDice;
};

inline cPartyManager* cPartyManager::GetSingleton()
{
	return mpSingleton;
}

#define PARTYMAN cPartyManager::GetSingleton()
