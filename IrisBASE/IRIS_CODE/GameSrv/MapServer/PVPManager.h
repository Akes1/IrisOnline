/* ==========================================================================
* 파    일 : PVPManager.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-09-04
* 주의사항 :
*===========================================================================*/
#pragma once

#include "DeathMatchObject.h"
#include "RangeCheck.h"
#include "PvPObjectPool.h"
#include "IndexGenerator.h"
#include "PvPPlayerListPool.h"
#include "GameSrv.h"
#include "PvPScript.h"

enum ePVPMANAGER_PROCESS
{
	ePVPPROCESS_WAIT,				/// pvp 대기상태
	ePVPPROCESS_RUN,				/// 전장 개별 수행
};

enum ePVPSECTION_PROCESS
{
	ePVPSECTION_WAIT,
	ePVPPROCESS_NOTICE,			/// 공지 발송상태
	ePVPPROCESS_READY,			/// 입장 신청 받는 상태
	ePVPPROCESS_RUNOBJECT,		/// 방별 프로세스로 넘김
};

struct sLevelForceInfo
{
	///
	sDMBlockInfo* mpBlockInfo;

	/// 인던 인덱스 목록
	tArray<unsigned long>	mPvPObjectIdxAry;

	///
	unsigned long	mForceCnt[eFORCETYPE_MAX];			/// 세력별 인원 카운트
	bool			mATeam[eFORCETYPE_MAX];				/// A팀 true / b팀 false
	unsigned long	mCreateMaxRoom;						/// 생성될 인던 갯수

	unsigned long	mTeamCnt[ePVPDM_TEAMTYPE_MAX];		/// 입장/불가 인원 체크시 입장가능 처리된 인원 카운트

	sLevelForceInfo()
	{
		memset( this, 0, sizeof(sLevelForceInfo) );
		mPvPObjectIdxAry.Reserve( 10 );
	}

	void Init()
	{
		memset( mForceCnt, 0, sizeof(mForceCnt) );
		memset( mATeam, 0, sizeof(mATeam) );
		memset( mTeamCnt, 0, sizeof(mTeamCnt) );
		mCreateMaxRoom = 0;
		mPvPObjectIdxAry.Clear();
	}
};

struct sSecionInfo
{
	ePVPSECTION_PROCESS mSecProcess;
	unsigned long		mNextEventTime;
	unsigned char		mMinLevel;
	unsigned char		mMaxLevel;
	unsigned char		mNoticeCount;
	unsigned long		mNoticeSendTime;

	/// 채널서버 레디 상태인 플레이어명단
	/// <unsigned long playerIdx>
	tHashSet<unsigned long> mReadyPlayerSet;
};

class cPVPManager
{	
public:

	static cPVPManager* mpPVPManager;

public:

	cPVPManager();
	~cPVPManager();

	bool				Init( SERVER_TYPE type, unsigned short mapNumber );
	void				Release();
	void				Process( unsigned long elapsedTime, unsigned long accumTime );
	void				ObjectProcess( unsigned long elapsedTime, unsigned long accumTime );


	/// 공용 기본 변수 조회
	sDMInfo*			GetDMInfo() { return mpDMInfo; }
	unsigned long		GetMapIndex() { return mMapNumber; }


	/// 인던 관리부
	void				TeamDivision( PVP_USER_SELECT* select );
	unsigned short		JoinDeathMatch( cPlayer* pPlayer, float* x, float*y );
	void				MemberOut( unsigned long pvpIdx, unsigned long playerIdx, ePVPDM_TEAM_TYPE teamType );
	void				DelAryInsertPVP( unsigned long pvpDMIdx );


	/// 전장 인던 객체관련 참조
	sPvPObjectRoot*		GetPvPObjectRoot()	{ return &mPvPObjectRoot; }
	cBaseDeathMatch*	GetPvPDMObject( unsigned long dmIdx );


	/// 객체 인덱스 생성 / 제거
	unsigned long		GenNpcIndex()	{ return mNpcIdxGen.GeneratIdx(); }
	void				DelNpcIndex( unsigned long npcIdx )	{ mNpcIdxGen.DelIdx( npcIdx); }

	unsigned long		GetTeamPlayerMax();
	unsigned long		GetTeamPlayerMin();

	unsigned long		GetTeamGenPosIdx( ePVPDM_TEAM_TYPE teamType );

	//void				SendReadyLeftTime( cPlayer* pPlayer );

	/// 게임서버 입장대기 관리목록
	void				AddReadyPlayer( cPlayer* pPlayer );
	void				DelReadyPlayer( cPlayer* pPlayer );
	void				SendNotPvPIn( unsigned long aryPos );

	void				AddGmPlayer( unsigned long playerIdx )	{ mGmPlayer.Insert( playerIdx ); }
	bool				IsGmPlayer( unsigned long playerIdx );

	unsigned long		GetSectionAryPos( unsigned char level );
	sSecionInfo*		GetSectionInfo( unsigned long aryPos );

	void				SendMsgSynPvPReadyClose( unsigned long aryPos );
private:

	typedef tPointerHashMap<unsigned long, void*> cPHashMap;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tArray<unsigned long> cAry;
	typedef tPointerArray<void*> cPAry;


	ePVPMANAGER_PROCESS	mProcess;			/// 프로세스 상태
	unsigned long		mNextEventTime;		/// 다음 이벤트 시작 시간

	sSecionInfo			mSectionInfo[PVP_DM_LEVEL_SECTION_MAX];

	/// pvp 객체 리스트
	sPvPObjectRoot		mPvPObjectRoot;


	///  객체 삭제 배열
	cAry				mDelDMAry;

	unsigned char		mDMMaxCnt;

	cRangeCheck			mRangeCheck;


	sDMInfo*			mpDMInfo;

	unsigned short		mMapNumber;

	unsigned long		mTeamGenPos[ePVPDM_TEAMTYPE_MAX];


	cIndexGenerator		mNpcIdxGen;


	/// 구간별 세력정보 객체
	cPAry				mLevelForceAry;

	/// 레벨별 참조정보
	/// <unsigned char level, sLevelForceInfo*>
	cPHashMap			mLevelForceMap;

	///// 레디 상태인 플레이어명단
	///// <unsigned long playerIdx>
	//cHashSet			mReadyPlayerSet;

	/// 다음 시작 시간 설정
	TIMESTAMP_STRUCT	mNextStartTime;
	unsigned long		mStartTimeScriptPos;

	/// 081114 PKH PvP스크립트
	cPvPScript*			mpPvPScript;

	cHashSet			mGmPlayer;
};

#define PVPMANAGER	cPVPManager::mpPVPManager

