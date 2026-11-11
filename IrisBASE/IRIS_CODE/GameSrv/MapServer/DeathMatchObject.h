/* ==========================================================================
* 파    일 : DeathMatchObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-09-04
* 주의사항 :
*===========================================================================*/
#pragma once

#include "BaseDeathMatch.h"

struct sLeaderInfo
{
	unsigned long		mLeaderIdx;			/// 수장 idx
	unsigned long		mLeaderChangeTime;	/// 수장 변경 시간
	unsigned long		mLeaderDieCnt;		/// 수장 죽은 수
};

struct sStoneInfo
{
	unsigned long		mStoneIdx;			/// 석상 idx
	unsigned long		mStoneRegenTime;	/// 석상 리젠
};


class cDeathMatchObject : public cBaseDeathMatch
{
public:
	cDeathMatchObject();
	virtual ~cDeathMatchObject();

	/// 메모리풀에서 객체 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 객체를 제거해줌
	void operator delete( void* ptr, size_t n );

	virtual bool Init( unsigned short DMIdx, sLevelForceInfo* pLevelForceInfo );

	void Process( unsigned long elapsedTime, unsigned long accumTime );
	void LeaderProcess( unsigned long elapsedTime, unsigned long accumTime );

	void StoneProcess( unsigned long elapsedTime, unsigned long accumTime );

	virtual void OutMember( unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType );

	/// 점수 변경
	virtual void ScoreUpdate( unsigned long killIdx, ePVPDM_TEAM_TYPE killerTeam, unsigned long deadIdx, ePVPDM_TEAM_TYPE deadTeam );
	void StoneScoreUpdate( ePVPDM_TEAM_TYPE killerTeam, unsigned long killerIdx );

	/// 결과 처리
	void				Result();

	void SyncStoneHP( unsigned long teamType, unsigned long hp );
	void DeleteStone( unsigned long teamType, unsigned long lastAttacker );

private:
	/// 프로세스 부하 방지
	unsigned long		mLeaderTimePass;

	sLeaderInfo			mLeaderInfo[ePVPDM_TEAMTYPE_MAX];
	sStoneInfo			mStoneInfo[ePVPDM_TEAMTYPE_MAX];

	unsigned long		mNextLogTime;
};

const int FLAG_MAX = 7;

struct sFlagInfo
{
	/// 리젠된 깃발 정보
	unsigned long		mFlagIndex;
	unsigned long		mFlagPointIndex;

	/// 현재 깃발 소유자
	unsigned long		mFlagOwnerIndex;
	ePVPDM_TEAM_TYPE	mFlagOwnerTeam;

	/// 깃발을 게양해서 리젠해야되는지 체크
	unsigned long		mNewFlagWaitTime;
	bool				mNewFlagGen;

	/// 유저들에게 깃발위치를 주기적 알려주는시간
	unsigned long mSendFlagPosTime;
};

struct sHoistInfo
{
	unsigned long		mObjectIdx;
	unsigned long		mRegenIdx;

	unsigned long		mNextRegenTime;
};

//////////////////////////////////////////////////////////////////////////

class cDeathMatch2Object : public cBaseDeathMatch
{
public:
	cDeathMatch2Object();
	virtual ~cDeathMatch2Object();

	/// 메모리풀에서 객체 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 객체를 제거해줌
	void operator delete( void* ptr, size_t n );

	virtual bool Init( unsigned short DMIdx, sLevelForceInfo* pLevelForceInfo );

	virtual void Process( unsigned long elapsedTime, unsigned long accumTime );

	virtual void OutMember( unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType );

	virtual void SendFight( unsigned long fightTime );

	/// 점수 변경
	void ScoreUpdate( unsigned long killIdx, ePVPDM_TEAM_TYPE killerTeam, unsigned long deadIdx, ePVPDM_TEAM_TYPE deadTeam );

	/// 채집 객체 상태 변경시 호출
	void ChangeGatheringState( unsigned long gIdx, unsigned long playerIdx );

	/// 결과 처리
	void Result();

	void RegenFlag( unsigned char aryPos, float x, float y );
	bool DelInfRegenFlag( unsigned long playerIdx, float x, float y );

	bool IsGathering( unsigned long playerIdx, unsigned long gatheringIdx );

protected:

	/// 깃발 & 깃발포인트 갱신 처리
	void FlagProcess( unsigned long accumTime );

	/// 젠 주기 갱신
	void EnableNewGen();
	void EnableNewGen( unsigned char aryPos );

	///
	void RegenNewFlag( unsigned char aryPos );
	void RegenHoist( bool first );

	/// msg 발송
	void SendFlagInfo( unsigned char aryPos, bool newGen );
	void SendFlagPos( unsigned char aryPos );

protected:

	typedef tHashMap<unsigned long,unsigned long> cLongMap;

	/// 최초 게양팀
	ePVPDM_TEAM_TYPE	mFirstFlagTeam;

	unsigned char		mFlagCnt;
	sFlagInfo			mFlagInfo[FLAG_MAX];

	/// 게양 포인트(채집)팀별 리젠 시간
	unsigned char		mAHoistCnt;
	sHoistInfo			mAteamHoist[FLAG_MAX];
	unsigned char		mBHoistCnt;
	sHoistInfo			mBteamHoist[FLAG_MAX];
};
