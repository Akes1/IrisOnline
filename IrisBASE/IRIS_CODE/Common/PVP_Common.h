/* ==========================================================================
* 파    일 : PVP_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-09-04
* 주의사항 :
*===========================================================================*/
#pragma once 

#include "CommonNetworkMsgStruct.h" 

#pragma pack(push,1) 

const int				PVP_DM_TEAM_CNT = 30;						/// 데스매치 팀인원
const int				PVP_DM_NOTICE_CNT = 5;						/// 데스매치 공지 발송 카운트
const int				PVP_DM_NOTICE_TIME = PVP_DM_NOTICE_CNT * MINUTE;	/// 데스매치 공지 발송 시간
const int				PVP_DM_JOIN_TIME = 1 * MINUTE;				/// 데스매치 입장 시간
const int				PVP_DM_READY_TIME = 3 * MINUTE;				/// 입장 시작부터 전투 시작까지 시간
const int				PVP_DM_READY_PRINT1_TIME = 30 * SECOND;		/// 시작전 남은 시간 알림1
const int				PVP_DM_READY_PRINT2_TIME = 10 * SECOND;		/// 시작전 남은 시간 알림2 (1초간격)
const int				PVP_DM_READY_OUT_TIME = 1 * MINUTE;			/// 데스매치 전투시작 카운트 불가시 게임 out 시간
const int				PVP_DM_TEAM_MINCNT_TIME = 5 * MINUTE;		/// 데스매치시 팀인원 부족시 시간 제한
const int				PVP_DM_OUTWAIT_TIME = 1 * MINUTE;			/// 데스매치 게임종료후 자동 종료 대기 시간

const int				PVP_DM_LEVEL_START_TERM = 10 * MINUTE;		/// 전장 시작 레벨 간격 텀
const int				PVP_DM_LEVEL_SECTION_MAX = 5;

const int				PVP_DM_MEMBER_DIE_POINT = 1;				/// 데스매치팀원 죽은경우 포인트
const int				PVP_DM_LEADER_DIE_POINT = 5;				/// 데스매치수장 죽은경우 포인트
const int				PVP_DM_LEADER_CHANGETIME = 1 * MINUTE;		/// 수장이 선택된 후 다음 선택 시간
const int				PVP_DM_LEADER_DIECHANGETIME = 5 * SECOND;	/// 수장이 죽은 후 다음 선택 시간
const int				PVP_DM_RESURRECTION_TIME = 10 * SECOND;		/// 데스매치시 죽은경우 부활대기 시간
const int				PVP_DM_LEADER_SKILL = 300001;				/// 리더 전용 효과
const int				PVP_DM_LEADER_INFLUENCE = 100001;			/// 리더 전용 효과
const int				PVP_DM_LEADER_INFLUENCE2 = 100008;			/// 리더 전용 효과
const int				PVP_DM2_FLAG_INFLUENCE = 100009;			/// 리더 전용 효과
const int				PVP_DM_NODAMAGE_INFLUENCE = 100002;			/// 무적 효과
const int				PVP_DM_WAIT_NODAMAGE_INFLUENCE = 100010;	/// 전장 진입 로딩전 무적 효과
const int				PVP_DM_CANTPVP_INFLUENCE = 100003;			/// PVP 진입 불가 효과
const int				PVP_DM_WIN_INFLUENCE = 100006;				/// 승리 보상 효과
const int				PVP_DM_LOSE_INFLUENCE = 100007;				/// 패배 보상 효과
const int				PVP_DM_ADD_TOTEMIDX_BUFF = 1001;			/// 자신의 지역에 이로운 효과를 주는 토템인덱스
const int				PVP_DM_ADD_TOTEMIDX_DEBUFF = 1002;			/// 자신의 지역에 해로운 효과를 주는 토템인덱스

const int				PVP_REPORT_TOTAL = 5;						/// 전장 자리비움 신고 강제 퇴장 누적점수

const int				PVP_DM_STONE_DIE_POINT = 10;				/// 석상 파괴시 점수
const int				PVP_DM_STONE_REGEN_TIME = 3 * MINUTE;		/// 석상 리젠 시간

/// unsigned char
enum ePVPDM_TEAM_TYPE
{
	ePVPDM_TEAMTYPE_A = 0,
	ePVPDM_TEAMTYPE_B,

	ePVPDM_TEAMTYPE_MAX,
};

enum ePVPDM_END_TYPE
{
	ePVPDM_ENDTYPE_POINT,					/// 포인트 달성
	ePVPDM_ENDTYPE_TIME,					/// 시간 초과(포인트량)
	ePVPDM_ENDTYPE_LEADERDIE,				/// 수장 사망 횟수
	ePVPDM_ENDTYPE_FIRSTKILL,				/// 최초 킬러 보유
	ePVPDM_ENDTYPE_KILLTOTAL,				/// Kill 총합
	ePVPDM_ENDTYPE_RANDOM,					/// random

	ePVPDM_ENDTYPE_FIRSTFLAG,				/// 최초 게양팀
	ePVPDM_ENDTYPE_LASTFLAG,				/// 마지막 깃발 보유팀
	ePVPDM_ENDTYPE_MEMBER,					/// 최소인원 부족
};


/// 구간별 정보
struct sDMBlockInfo
{
	unsigned char	mLevelMin;				/// 최소레벨
	unsigned char	mLevelMax;				/// 최대레벨

	unsigned long	mGoalPoint;				/// 목표 포인트
	
	unsigned long	mStoneAIdx;				/// 석상 몬스터 번호
	unsigned long	mStoneBIdx;				/// 석상 몬스터 번호

	unsigned long	mLoseDMBasePoint;		/// 패배시 전장포인트 보상기준
	unsigned long	mLoseFriendlyBasePoint;	/// 패배시 친밀도 보상기준
};

/// 최종 보상
struct sDMGiveInfo
{
	unsigned char		mLevel;						// 캐릭터 레벨

	unsigned long		mWinMoney;					// 돈 보상-승리
	unsigned long		mLoseMoney;					// 돈 보상-패배

	float				mWinExpPer;					// EXP 보상-승리(%)
	float				mLoseExpPer;				// EXP 보상-패배(%)

	float				mWinSxpPer;					// SXP 보상-승리(%)
	float				mLoseSxpPer;				// SXP 보상-패배(%)	

	unsigned long		mKillPoint;					// kill-일반
	unsigned long		mLeaderKillPoint;			// kill-수장

	unsigned long		mWinMyForcePoint;			// 자기세력포인트-승리
	unsigned long		mWinMyForceFriendly;		// 자기세력친밀도-승리

	unsigned long		mLoseMyForcePoint;			// 자기세력포인트-패배
	unsigned long		mLoseMyForceFriendly;		// 자기세력친밀도-패배

	unsigned long		mWinFamilyForcePoint;		// 동맹세력포인트-승리
	unsigned long		mWinFamilyForceFriendly;	// 동맹세력친밀도-승리
};

/// 전장 시작 시간
struct sPvPStartTime
{
	unsigned char	mHour;		/// 시작 시
	unsigned char	mMinute;	/// 시작 분
};

struct sDMInfo
{
	unsigned char	mType;			/// 전장 종류
	unsigned long	mTotalDMTime;	/// 총진행시간
	unsigned char	mTeamPlayerMin;	/// 진영최소인원
	unsigned char	mTeamPlayerMax;	/// 진영최대인원
	bool			mStone;			/// 수호석생성 여부
	NiPoint2		mStoneAPos;		/// A진영 수호석 위치
	NiPoint2		mStoneBPos;		/// B진영 수호석 위치

	/// 레벨 구간별 리스트
	tPointerArray<sDMBlockInfo*>	mLevelBlockInfo;

	/// 레벨별 보상정보
	tPointerHashMap<unsigned long, sDMGiveInfo*> mGiveInfo;

	~sDMInfo()
	{
		unsigned int count = mLevelBlockInfo.GetSize();
		for( unsigned int i=0;i<count;i++ )
		{
			delete mLevelBlockInfo[i];
		}
		mLevelBlockInfo.Clear();

		///
		tPointerHashMap<unsigned long, sDMGiveInfo*>::cIterator b = mGiveInfo.Begin();
		tPointerHashMap<unsigned long, sDMGiveInfo*>::cIterator end = mGiveInfo.End();
		for( ; b!= end; b++ )
		{
			sDMGiveInfo* info = (sDMGiveInfo*)(*b).mSecond;
			SAFE_DELETE(info);
		}
		mGiveInfo.Clear();
	}

	sDMBlockInfo* GetBlockInfo( unsigned short level )	
	{
		unsigned int count = mLevelBlockInfo.GetSize();
		for( unsigned int i=0;i<count;i++ )
		{
			sDMBlockInfo* info = (sDMBlockInfo*)mLevelBlockInfo[i];
			if( info )
			{
				if( level >= info->mLevelMin && level <= info->mLevelMax )
					return info;
			}
		}
		return 0;
	}

	sDMGiveInfo*	GetGiveInfo( unsigned short level )
	{
		return (sDMGiveInfo*)mGiveInfo.GetAt( level );
	}
};

struct sPvPPlayerList
{
	short			mCharacterChannel;	/// 플레이어의 신청시 채널
	unsigned long	mPlayerIdx;			/// 플레이어idx
	unsigned char	mLevel;				/// 플레이어레벨
	unsigned char	mForceType;			/// 세력타입
	bool			mSelection;			/// 전장참여에 선정/비선정 여부
};

struct MSG_SYN_PVP_DM_NOTICE : public MSGROOT
{
	unsigned short		mPvPType;			/// pvp 종류
	unsigned char		mLeftTime;			/// 남은 시간(분)
};

//struct MSG_SYN_PVP_DM_JOIN : public MSGROOT
//{
//	unsigned char		mMinLevel;
//	unsigned char		mMaxLevel;
//	unsigned long		mLeftTime;			/// 남은 시간
//};

struct MSG_SYN_PVP_READYQUESTION : public MSGROOT
{
	unsigned short		mPvPType;			/// pvp 종류
	unsigned char		mMinLevel;			/// 레벨 구간
	unsigned char		mMaxLevel;
	unsigned long		mLeftTime;			/// 남은 시간
};

struct MSG_RES_PVP_DM_JOIN : public MSGROOT
{
	unsigned long		mDMIdx;
	unsigned char		mTeamType;		/// ePVPDM_TEAM_TYPE
};

struct sMsgMemberInfo
{
	unsigned long		mCharacterIdx;
	wchar_t				mName[MAX_NAME_BUFFER_SIZE];
	unsigned char		mForceType;						/// eFORCE_TYPE
};

struct MSG_SYN_PVP_DM_MEMBERIN : public MSGROOT
{
	sMsgMemberInfo		mMemberInfo;
};

struct MSG_RES_PVP_DM_MEMBERIN : public MSGROOT
{
	bool				mFireForce;			/// 팀세력 유무
	bool				mWaterForce;
	bool				mWindForce;
	bool				mEarthForce;
	unsigned char		mMemberCnt;			/// 접속중인 유저 카운트
	sMsgMemberInfo		mMemberInfo[1];		/// 점속중인 유저 정보

	MSG_RES_PVP_DM_MEMBERIN()
	{
		memset( this, 0, sizeof(MSG_RES_PVP_DM_MEMBERIN) );
	}

	unsigned long GetMsgLength() { return sizeof(MSG_RES_PVP_DM_MEMBERIN) - sizeof(mMemberInfo) + (mMemberCnt * sizeof(sMsgMemberInfo));} 
};

struct MSG_SYN_PVP_DM_MEMBEROUT : public MSGROOT
{
	unsigned long		mCharacterIdx;
};

struct MSG_SYN_PVP_DM_LEADERCHANGE : public MSGROOT
{
	unsigned long		mCharacterIdx;		/// 수장 플레이어IDX
};

struct MSG_SYN_PVP_JOINTIME : public MSGROOT
{
	unsigned long		mFightLeftTime;		/// 전투까지 남은시간
};

struct MSG_SYN_PVP_FIGHT : public MSGROOT
{
	unsigned long		mFightTime;		/// 전투 시간
};

struct MSG_SYN_PVP_DM_SCORE : public MSGROOT
{
	unsigned char		mTeamType;		/// ePVPDM_TEAM_TYPE
	unsigned long		mTotalScore;
	unsigned long		mPlusScore;
	unsigned long		mPlayerIdx;
	unsigned long		mPlayerScore;
};

struct MSG_SYN_PVP_DM2_SCORE : public MSGROOT
{
	unsigned long		mDelFlagIdx;
	unsigned long		mDelHoistIdx;
	unsigned char		mTeamType;		/// ePVPDM_TEAM_TYPE
	unsigned long		mTotalScore;
};

struct MSG_SYN_PVP_DM2_PLAYER_SCORE : public MSGROOT
{
	unsigned char		mTeamType;		/// ePVPDM_TEAM_TYPE
	unsigned long		mPlayerIdx;
	unsigned long		mPlayerScore;
};


struct sPvPDMTeamScore
{
	unsigned long		mCharacterIdx;
	unsigned long		mScore;
};

struct MSG_SYN_PVP_DM_FIGHTEND : public MSGROOT
{
	unsigned char		mEndType;
	unsigned long		mFireFriendly;
	unsigned long		mFirePoint;
	unsigned long		mWaterFriendly;
	unsigned long		mWaterPoint;
	unsigned long		mWindFriendly;
	unsigned long		mWindPoint;
	unsigned long		mEarthFriendly;
	unsigned long		mEarthPoint;
	unsigned long		mFireFriendlyTotal;
	unsigned long		mFirePointTotal;
	unsigned long		mWaterFriendlyTotal;
	unsigned long		mWaterPointTotal;
	unsigned long		mWindFriendlyTotal;
	unsigned long		mWindPointTotal;
	unsigned long		mEarthFriendlyTotal;
	unsigned long		mEarthPointTotal;
	unsigned long		mMoney;
	unsigned long		mSxp;
	unsigned long		mExp;
	unsigned long		mTeamCnt;
	sPvPDMTeamScore		mTeam[1];

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_PVP_DM_FIGHTEND) - sizeof(mTeam) + (mTeamCnt * sizeof(sPvPDMTeamScore)); }
};

struct MSG_RES_PVP_DM_RANKINFO : public MSGROOT
{
	unsigned long		mTeamCnt;
	sPvPDMTeamScore		mTeam[1];

	/// 실제보내는 메세지 크기
	unsigned long GetMsgLength() { return sizeof(MSG_RES_PVP_DM_RANKINFO) - sizeof(mTeam) + (mTeamCnt * sizeof(sPvPDMTeamScore)); }
};


struct MSG_SYN_PVP_DM2_INFO : public MSGROOT
{
	unsigned long	mGatherIdx;
	unsigned long	mFlagOwnerIdx;	// 깃발 소유자
	NiPoint2		mFlagPointPos;	// 게양 포인트 위치

	bool			mPrintNewGenMsg;// 신규 젠에 대한 메제시 출력여부
};

struct MSG_SYN_PVP_DM2_FLAGPOS : public MSGROOT
{
	unsigned long	mGatherIdx;
	NiPoint2		mFlagPos;		// 깃발 위치
};

struct MSG_SYN_PVP_DM2_HOIST : public MSGROOT
{
	unsigned long	mGatherIdx;
	NiPoint2		mPos;		// 위치
};

// 전장 자리비움 신고
struct MSG_REQ_PVP_REPORT : public MSGROOT
{
	wchar_t			mName[MAX_NAME_BUFFER_SIZE];	// 신고할 유저명
};

#define ERROR_PVP_REPORT_SUCCESS	0	/// 성공.
#define ERROR_PVP_REPORT_FAIL		1	/// 실패
#define ERROR_PVP_REPORT_OTHERTEAM	2	/// 실패 - 다른 진영
#define ERROR_PVP_REPORT_EXIST		3	/// 실패 - 중복신고

struct MSG_RES_PVP_REPORT : public MSG_ERROR
{};

struct MSG_SYN_PVP_REPORT : public MSGROOT
{
	wchar_t	mName[MAX_NAME_BUFFER_SIZE];		// 신고한 유저명
};


//
struct MSG_SYN_PVP_REPORT_OUT : public MSGROOT
{
	wchar_t	mName[MAX_NAME_BUFFER_SIZE];		// 강제퇴장 캐릭터
};

struct MSG_SYN_PVP_FORCEOUT : public MSGROOT	// 강제퇴장 처리
{};


struct MSG_SYN_PVP_DM_STONE_CREATE : public MSGROOT
{
	unsigned char		mTeamType;		/// ePVPDM_TEAM_TYPE
	unsigned long		mMaxHP;
	NiPoint2			mPos;			// 위치
};

struct MSG_SYN_PVP_DM_STONE_HP : public MSGROOT
{
	unsigned long		mHP;
};

struct MSG_SYN_PVP_DM_STONE_DIE : public MSGROOT
{
	unsigned char		mTeamType;		// ePVPDM_TEAM_TYPE 킬러 팀타입
	unsigned long		mTotalScore;	// 킬러 팀 총 점수
	unsigned long		mKillerIndex;	// 스톤 죽인 유저
};

struct MSG_SYN_DM_ENEMYLEADERDIE : public MSGROOT
{
	unsigned long		mPlayerIdx;		/// 적군 수장
};

struct MSG_SYN_DM_LEADERDIE : public MSGROOT
{
	unsigned long		mPlayerIdx;		/// 아군 수장
};

struct MSG_SYN_PVP_DM2_FLAGDROP : public MSGROOT
{
	unsigned long		mPlayerIdx;
};

struct MSG_SYN_PVP_DM2_FLAGCOMPLETE_SYN : public MSG_SYN_PVP_DM2_SCORE
{
	unsigned long		mPlayerIdx;
};

struct MSG_SYN_PVP_FLAGCOMPLETE_SCORE : public MSG_SYN_PVP_DM2_SCORE
{
	unsigned long		mPlayerIdx;
};

#pragma pack(pop) 

