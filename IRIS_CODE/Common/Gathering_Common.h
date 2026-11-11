/* ==========================================================================
* 파    일 : Gathering_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-12-11
* 주의사항 :
*===========================================================================*/
#pragma once
#pragma pack(push,1)

#include "CommonNetworkMsgStruct.h"

/// unsigned char
enum eGATHERING_TYPE
{
	eGATHERINGTYPE_COMMON = 0,		/// 타입
	eGATHERINGTYPE_MINING,


	eGATHERINGTYPE_FLAG = 101,
	eGATHERINGTYPE_FLAGPOINT,
};

/// 채집 상태값 
enum eACTION_GAT_ID 
{ 
	eACT_GAT_IDLE,					/// 일반 대기 상태
	eACT_GAT_DIE = 4,				/// 제거
};

/// 상태 변경 값 
enum eACTION_GAT_CHANGE 
{ 
	eACTCH_GAT_CONTINUE	= 0,		/// 프로세스 계속 수행		 
	eACTCH_GAT_ERROR,				/// 스크립트 오류

	eACTCH_GAT_IDLE_END,			/// 대기 종료
	eACTCH_GAT_DIE_END,				/// 제거 프로세스 종료 
};


struct sGatheringList			/// 채집 정보
{
	unsigned long		mGatheringIdx;		/// 채집IDX
	unsigned long		mNameIdx;			/// 이름IDX
	unsigned long		mNameHeight;
	cString				mFileName;			/// 
	unsigned long		mIconIndex;
	cString				mEffectFileName;
	eGATHERING_TYPE		mType;				/// 타입(종류)
	unsigned long		mNeedItemIdx;		/// 필요아이템
	unsigned long		mNeedBuffIdx;		/// 필요버프
	unsigned char		mNeedLevel;			/// 캐릭터레벨
	unsigned char		mNeedMakeIdx;		/// 요구 제조스킬
	long				mNeedQuestIdx;		/// 요구 퀘스트
	unsigned long		mInfluenceIdx;		/// 습득시 부여할 효과번호
	unsigned long		mGatheringTime;		/// 채집 시간

	tPointerHashMap<unsigned long, void*>*	mpQuestItemDropMap;

	sGatheringList()
	{
		mpQuestItemDropMap = NULL;
	}
};

struct sGatheringRegen			/// 리젠 좌표 정보
{
	unsigned long		mGatheringIdx;		/// 채집IDX
	unsigned short		mMapNumber;			/// 맵번호
    float				mPosX;				/// 좌표
	float				mPosY;
	float				mDirection;			/// 회전값
	float				mHeight;			/// 높이 확장값
};

struct sGatheringRegenHeader	/// 채집 리젠 헤더
{
	unsigned long		mRegenWaitTime;		/// 리젠 대기시간
	unsigned long		mRandTime;			/// 대기시간 이후 랜덤 리젠 시간
	/// <sGatheringRegen*>
	tArray<void*>		mRegenDetail;		/// 리젠 위치 정보 포인터
};

struct MSG_GATHERING_INFO : public MSGROOT
{
	unsigned long		mObjectIdx;			/// 채집 오브젝트 IDX
	unsigned long		mGatheringIdx;		/// 채집 종류 IDX
	float				mPosX;				/// 생성 좌표
	float				mPosY;
	float				mDirection;			/// 방향값
	float				mAppendZ;			/// 높이 확장값
};

struct MSG_GATHERING_HOIST_INFO : public MSGROOT
{
	unsigned long		mObjectIdx;			/// 채집 오브젝트 IDX
	unsigned long		mGatheringIdx;		/// 채집 종류 IDX
	float				mPosX;				/// 생성 좌표
	float				mPosY;
	float				mDirection;			/// 방향값
	float				mAppendZ;			/// 높이 확장값
	unsigned char		mTeamType;			/// pvp 게양팀 정보
};

const int ERROR_GATHERING_SUCCESS = 0;		/// 성공
const int ERROR_GATHERING_FAIL = 1;			/// 실패
const int ERROR_GATHERING_LEVEL = 2;		/// 레벨
const int ERROR_GATHERING_ITEM = 3;			/// 장비
const int ERROR_GATHERING_QUEST = 4;		/// 퀘스트
const int ERROR_GATHERING_MAKESKILL = 5;	/// 전문기술
const int ERROR_GATHERING_RANGE = 6;		/// 거리
const int ERROR_GATHERING_BUFF = 7;			/// 버프
const int ERROR_GATHERING_PVP = 8;			/// 전장에서 불가능한 채집방법


struct MSG_REQ_GATHERING_START : public MSGROOT
{
	unsigned long		mObjectIdx;			/// 채집 오브젝트 IDX
};

struct MSG_SYN_GATHERING_CANCEL: public MSGROOT
{
	unsigned long		mCharacterIdx;		/// 유저 IDX
};

struct MSG_SYN_GATHERING_START : public MSGROOT
{
	unsigned long		mObjectIdx;			/// 채집 오브젝트 IDX
	unsigned long		mClassIdx;			/// 채집 종류 IDX
	unsigned long		mCharacterIdx;		/// 유저 IDX
};

struct MSG_SYN_GATHERING_END : public MSGROOT
{
	unsigned long		mObjectIdx;			/// 채집 오브젝트 IDX
	unsigned long		mCharacterIdx;		/// 유저 IDX
};


typedef MSG_REQ_GATHERING_START MSG_GATHERINGIDX;

#pragma pack(pop)
