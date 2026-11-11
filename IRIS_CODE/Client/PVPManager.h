/* ==========================================================================
* 작 성 자 : 김가람
* 작 성 일 : 2008.09.05
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "PVP_Common.h"

const unsigned long PVP_WAIT_TIME = 1000 * 60;

class cPlayer;

enum ePVPState
{
	ePVPState_None,

	ePVPState_Notice,	// 입장알림
	ePVPState_Count,	// 입장 대기
	ePVPState_Wait,		// 대기		
	ePVPState_MapChangeWait,		// 맵 체인지 대기

	
	//
	ePVPState_Enable,	// 전투 가능(맵이동후)
	ePVPState_Ready,	// 전투 대기
	ePVPState_Fight,	// 전투
	ePVPState_Result,	// 결과
};

struct sMyTeamInfo
{
	unsigned long	mPlayerIdx;
	TCHAR			mName[MAX_NAME_BUFFER_SIZE];
	unsigned char	mForceType;
	unsigned long	mKillCnt;
	unsigned long	mDeathCnt;

	unsigned long	mScoreResult;
};

class cPVPManager
{
public:
	static cPVPManager* mpSingleton;

public:
	cPVPManager();
	~cPVPManager();

	void Open();
	void Close();

	void Process( unsigned long deltaTime, unsigned long accumTime );

	bool IsEnableAttack( cPlayer* player );
	bool IsMyTeam( cPlayer* player );

	inline bool IsEnableBattle() { return (mPVPState >= ePVPState_Enable && mPVPState <= ePVPState_Result); }
	inline void SetEnableBattle( bool set ) { mPVPState = (set)? ePVPState_Enable:ePVPState_None; }

	// 전장 자리비움 신고
	void RequestReport( cStringT name );

	//
	void ChangeState( ePVPState state );

	void NetworkMsgParser( char* pMsg );

	inline unsigned char GetMinLevel() { return mMinLevel; }
	inline unsigned char GetMaxLevel() { return mMaxLevel; }
	inline ePVPState GetPVPState() { return mPVPState; }
	inline unsigned short GetPVPType() { return mPVPType; }
	inline unsigned long GetWaitLeftTime() { return mWaitLeftTime; }
	inline unsigned long GetGoalPoint() { return mGoalPoint; }
	inline unsigned long GetTeamAScore() { return mTeamAScore; }
	inline unsigned long GetTeamBScore() { return mTeamBScore; }

	void LinkPvpForceEffect( cPlayer* player, unsigned char forceType );

public:
	static cPVPManager* GetSingleton();

	void AddTeamPlayerInfo( sMsgMemberInfo* memberInfo );
	void UpdateTeamPlayerInfo( unsigned long playerIdx, unsigned long value, bool kill );
	void DelTeamPlayerInfo( unsigned long playerIdx );

	void OpenRank( bool update );
	void OpenResult( MSG_SYN_PVP_DM_FIGHTEND* pmsg, bool win );

protected:
	// 전장 타입
	unsigned short mPVPType;

	// 상태 
	ePVPState mPVPState;

	// 정보
	unsigned long mLeaderIdx;

	unsigned char mMinLevel;
	unsigned char mMaxLevel;
	unsigned long mFightLeftTime;
	unsigned long mReadyPrintTime;
	unsigned long mWaitLeftTime;		// 대기 시간

	bool mUpdateRankInfo;
	typedef tPointerHashMap<unsigned long, void*> cMyTeamInfoMap;
	cMyTeamInfoMap  mTeamInfoMap;

	// 점수
	unsigned long mGoalPoint;
	unsigned long mTeamAScore;
	unsigned long mTeamBScore;

	// 신고 락
	bool mRequestLock;
};

inline cPVPManager* cPVPManager::GetSingleton()
{
	return mpSingleton;
}

#define PVPMAN cPVPManager::GetSingleton()
