/* ==========================================================================
* 파    일 : DeathMatchObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-09-04
* 주의사항 :
*===========================================================================*/
#pragma once

#include "PVP_Common.h"
#include "Player_Common.h"
#include "MemoryPool.h"
#include "RangeCheck.h"

class cPlayer;
struct sLevelForceInfo;

enum ePVPDM_PROCESS
{
	ePVPDM_PROCESS_WAIT,					/// 유저 입장 대기
	ePVPDM_PROCESS_READY,					/// 전투전 1분 카운트 시작
	ePVPDM_PROCESS_READY_OUT,				/// 카운트 시작 조건 비만족 - 전투불가
	ePVPDM_PROCESS_FIGHT_MINIMUM,			/// 시작후 5분간 인원 체크 안함
	ePVPDM_PROCESS_FIGHT,					/// 모든 조건을 체크하는 전투 상태
	ePVPDM_PROCESS_FIGHTEND,				/// 전투 끝 결과 처리
	ePVPDM_PROCESS_END,						/// 객체 제거
};

enum ePVPDM_OBJECTTYPE
{
	ePVPDM_OBJECTTYPE_1 = 101,				/// pvp전장1
	ePVPDM_OBJECTTYPE_2,					/// pvp전장2
};


//////////////////////////////////////////////////////////////////////////
/// 개인 정보
class cPvPDMMemberInfo
{
public:

	cPvPDMMemberInfo();
	~cPvPDMMemberInfo();

	void* operator new( size_t n );
	void operator delete( void* p );

	void CalcScore();

public:
	unsigned long		mPlayerIdx;			/// 고유번호
	unsigned char		mForceType;

	unsigned long		mLeaderKillCnt;		/// 수장 죽인수
	unsigned long		mKillCnt;			/// 죽인수
	unsigned long		mDeathCnt;			/// 죽은수

	/// 결과값 변수
	unsigned long		mScore;

	// 자신을 신고한 유저 인덱스 셋
	typedef tHashSet<unsigned long> cReportSet;
	cReportSet	mReportSet;

private:
	static tPool<cPvPDMMemberInfo>	mMemberInfoPool;

};


//////////////////////////////////////////////////////////////////////////
/// 팀 정보
class cPvPDMTeamInfo
{
public:
	cPvPDMTeamInfo();
	~cPvPDMTeamInfo();

	bool AddMember( unsigned long idx, unsigned char forceType );
	bool DelMember( unsigned long idx );

	unsigned long CalcScore( sPvPDMTeamScore* arr );

	bool AddReport( unsigned long objectIndex, unsigned long targetIndex );
	int GetReportCount( unsigned long targetIndex );

public:
	unsigned char		mMemberCnt;			/// 팀인원
	unsigned long		mPoint;				/// 습득 포인트

	unsigned long		mTotalKillCnt;

	/// 연합 관리
	unsigned long		mPartyUnionIndex;

	/// 세력별 동맹팀
	bool				mForce[eFORCETYPE_MAX];

	//cPvPDMMemberInfo
	tPointerHashMap<unsigned long, void*> mMemberMap;
};

//////////////////////////////////////////////////////////////////////////

class cBaseDeathMatch : public PerNode
{
public:
	cBaseDeathMatch( ePVPDM_OBJECTTYPE type );
	virtual ~cBaseDeathMatch();

	virtual bool		Init( unsigned short DMIdx, sLevelForceInfo* pLevelForceInfo );
	virtual void		Process( unsigned long elapsedTime, unsigned long accumTime ) = 0;
	int					GetProcess()	{ return mProcess; }

	/// 데스매치 맴버 추가
	int					JoinTeam( cPlayer* pPlayer );
	void				SendTeamList( cPlayer* pPlayer );

	virtual void		OutMember( unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType );

	unsigned short		GetDMIdx()	{ return mDMIdx; }

	void				SendReadyLeftTime( cPlayer* pPlayer );

	bool				IsFight();
	unsigned char		TeamCnt( ePVPDM_TEAM_TYPE teamType )	{ return mTeamInfo[teamType].mMemberCnt; }
	unsigned long		TeamKillTotal( ePVPDM_TEAM_TYPE teamType );

	void				SetPartyUnionIndex( ePVPDM_TEAM_TYPE teamType, unsigned long partyUnionIndex );
	unsigned long		GetPartyUnionIndex( ePVPDM_TEAM_TYPE teamType );

	/// 카운트 시작
	void				Ready();

	/// pvp 시작 메세지 발송
	virtual void		SendFight( unsigned long fightTime );
	bool				SendFight( cPlayer* pPlayer );

	void				EndNpcTalk();

	void				AddTeleportPortal();
	
	/// 점수 변경
	virtual void		ScoreUpdate( unsigned long killIdx, ePVPDM_TEAM_TYPE killerTeam, unsigned long deadIdx, ePVPDM_TEAM_TYPE deadTeam ) = 0;

	/// 결과 처리
	virtual void		Result();
	void				SendResult( ePVPDM_TEAM_TYPE winTeam, ePVPDM_END_TYPE endType );
	virtual void		SendTeamResult( cPvPDMTeamInfo* pTeam, ePVPDM_END_TYPE endType, char protocol );

	/// pvp 끝남 처리
	void				EndDM();
	bool			OrderEndDM();
	void				EndDMReq( unsigned long playerIdx, unsigned long teamType );

	/// msg발송
	void				DMQuickSend( char* msg, unsigned long length );
	void				TeamQuickSend( ePVPDM_TEAM_TYPE teamType, char* msg, unsigned long length );
	void				TeamQuickSendExcept( ePVPDM_TEAM_TYPE teamType, unsigned long exceptIdx, char* msg, unsigned long length );
	void				TeamChatQuickSend( cPlayer* pPlayer, ePVPDM_TEAM_TYPE teamType, char* msg, unsigned long length );

	ePVPDM_OBJECTTYPE	GetType() { return mObjectType; }

	bool				IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos );
	void				SendObstruction( cPlayer* pPlayer );

	unsigned long		GetProcessEndTime()	{ return mProcessEndTime; }

	int					UseTeleportPortal( cPlayer* pPlayer, unsigned long mapChangeIdx );

	// 전장 신고 추가
	int					AddReport( cPlayer* hero, unsigned long targetIndex );

protected:

	typedef tArray<unsigned long> cArray;
	typedef tPointerArray<void*> cPArray;
	typedef tPointerHashMap<unsigned long, void*> cPHashMap;
	typedef tHashSet<unsigned long> cHashSet;

	unsigned short		mDMIdx;					/// 고유 번호
	ePVPDM_OBJECTTYPE	mObjectType;	

	ePVPDM_PROCESS		mProcess;

	unsigned long		mFightStartTime;		/// 전투 시작시간

	unsigned long		mProcessEndTime;		/// 현재 프로세스 종료 시간

	sDMBlockInfo*		mpDMBlockInfo;			/// 레벨 구간 정보

	/// 팀 정보
	cPvPDMTeamInfo		mTeamInfo[ePVPDM_TEAMTYPE_MAX];	
	ePVPDM_TEAM_TYPE	mFirstKillTeam;

	bool				mScoreEnd;

	/// 토템 관리자
	cArray				mDMTotemArr;

	/// npc관리자
	cArray				mNpcAry;
	
	cArray				mTempChatAry;

	cRangeCheck			mRangeCheck;

	unsigned long		mWaitEndTime;

	unsigned long		mAutoOutCheckTime;

	//////////////////////////////////////////////////////////////////////////
	/// 장애물
	/// <unsigned long, sDoorInfo* pDoorInfo>
	cPHashMap			mObstructionMap;

	//////////////////////////////////////////////////////////////////////////
	/// 텔레포트 위치이동 포탈
	cHashSet			mPvPTeleportSet;
};
