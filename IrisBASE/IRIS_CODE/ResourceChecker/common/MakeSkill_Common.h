/* ==========================================================================
* 파    일 : MakeSkill_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2009-01-05
* 주의사항 :
*===========================================================================*/
#pragma once

#include "CommonNetworkMsgStruct.h"

#pragma pack(push,1)

const int MIN_MAKESKILL             = 1;
const int MAX_MAKESKILL             = 2;
const int MAKESKILL_MAKE_ITEM_CNT   = 3;
const int MAKESKILL_GROUP_MAX       = 100000;

/// 최대 조합 숙련도
#define MIXSKILL_EXP1	70
#define MIXSKILL_EXP2	200
#define MIXSKILL_EXP3	320
#define MIXSKILL_EXP4	450

/// unsigned char
enum eMAKESKILL
{
	eMAKESKILL_NONE = 0,
	eMAKESKILL_COOK,		/// 요리
	eMAKESKILL_PAINTCARD,	/// 카드그리기
	eMAKESKILL_AlCHEMY,		/// 연금술
};

/// 제조스킬
struct sMakeSkillScript
{
	unsigned long		mRecipeIdx;								/// 레시피idx
	unsigned char		mMakeSkill;								/// 전문 기술 eMAKESKILL

	unsigned char		mMakeStep;								/// 단계
	unsigned short		mMinExp;								/// 최소숙련도 
	unsigned short		mMinDefaultExp;							/// 기본최소숙련도
	unsigned short		mMaxDefaultExp;							/// 기본최대숙련도
	unsigned short		mMaxExp;								/// 최대업가능숙련도

	unsigned long		mType;									/// 분류 타입
	unsigned long		mGroupIdx;								/// 습득 그룹
	unsigned long		mMakeItemIdx[MAKESKILL_MAKE_ITEM_CNT];	/// 완성된 아이템
	unsigned char		mMakeCnt[MAKESKILL_MAKE_ITEM_CNT];		/// 완성된 수량
	unsigned short		mMakeRate[MAKESKILL_MAKE_ITEM_CNT];		/// 선택 확률(비율)
	unsigned long		mMakeTime;								/// 조합시간
	unsigned long		mNextMakeTime;							/// 재조합시간

	/// tArray<sMaterialScript*>
	tArray<void*>*		mpMaterial;								/// 재료아이템

	sMakeSkillScript()
	{
		memset( this, 0, sizeof(sMakeSkillScript) );
	}
};

/// 재료아이템
struct sMaterialScript
{
    unsigned long		mItemIdx;	/// 재료아이템
	unsigned char		mItemCnt;	/// 보유수량
};


struct sRecipeInfo
{
	unsigned long		mRecipeIdx;
	unsigned long		mCoolTime;
};

struct sRecipeGroupInfo
{
	unsigned long		mRecipeIdx;
	unsigned char		mMakeSkill;
};

struct sGroupInfo
{
    unsigned char       mStep;
    unsigned long       mExp;
    tArray<void*>		mRecipeArr;
};

/// 제조스킬
struct MSG_RES_MAKESKILL_LIST : public MSGROOT
{
	unsigned short	mCount;
	sRecipeInfo		mRecipeInfo[1];

	unsigned long	GetMsgLength() { return sizeof(MSG_RES_MAKESKILL_LIST) - sizeof(mRecipeInfo) + 
		(mCount * sizeof(sRecipeInfo)); } 
};

#define ERROR_MAKESKILL_ADD_SUCCESS     0
#define ERROR_MAKESKILL_ADD_DBUSED      1
#define ERROR_MAKESKILL_ADD_DBERROR     2
#define ERROR_MAKESKILL_ADD_STATE       3
#define ERROR_MAKESKILL_ADD_ITEM        4
#define ERROR_MAKESKILL_ADD_NOTEMPTY    5           /// 2개의 보조직업을 모두 배웠음.
#define ERROR_MAKESKILL_ADD_ALREADY     6
#define ERROR_MAKESKILL_ADD_STEP        7

struct MSG_REQ_MAKESKILL_ADD : public MSGROOT
{
	unsigned short	mSlotNum;				        /// 인벤슬롯 번호
};

struct MSG_RES_MAKESKILL_ADD : public MSG_ERROR
{
	unsigned char	mMakeSkill;
	unsigned char	mMakeSkillStep;
};


#define ERROR_MAKESKILL_DEL_DBUSED      0
#define ERROR_MAKESKILL_DEL_SUCCESS     1
#define ERROR_MAKESKILL_DEL_DBERROR     2
#define ERROR_MAKESKILL_DEL_NOTHAVE     3

struct MSG_REQ_MAKESKILL_DELETE : public MSGROOT
{
	unsigned char	mMakeSkill;
};

struct MSG_RES_MAKESKILL_DELETE : public MSG_ERROR
{
	unsigned char	mMakeSkill;
};


#define ERROR_MAKESKILL_RECIPE_ADD_SUCCESS  0
#define ERROR_MAKESKILL_RECIPE_ADD_DBUSED   1
#define ERROR_MAKESKILL_RECIPE_ADD_DBERROR  2
#define ERROR_MAKESKILL_RECIPE_ADD_STATE    3
#define ERROR_MAKESKILL_RECIPE_ADD_ITEM     4
#define ERROR_MAKESKILL_RECIPE_ADD_SCRIPT   5
#define ERROR_MAKESKILL_RECIPE_ADD_ALREADY  6
#define ERROR_MAKESKILL_RECIPE_ADD_NOTHAVE  7       /// 습득한 보조직업이 없음
#define ERROR_MAKESKILL_RECIPE_ADD_STEP     8
#define ERROR_MAKESKILL_RECIPE_ADD_EXP      9

struct MSG_REQ_MAKESKILL_RECIPE_ADD : public MSG_REQ_MAKESKILL_ADD
{};

struct MSG_RES_MAKESKILL_RECIPE_ADD : public MSG_ERROR
{
	unsigned short		mCount;
	sRecipeGroupInfo	mGroupInfo[1];

	unsigned long	GetMsgLength() { return sizeof(MSG_RES_MAKESKILL_RECIPE_ADD) - sizeof(mGroupInfo) + (mCount * sizeof(sRecipeGroupInfo)); } 
};


#define ERROR_MAKESKILL_START_SUCCESS       0
#define ERROR_MAKESKILL_START_SCRIPT        1
#define ERROR_MAKESKILL_START_COOLTIME      2
#define ERROR_MAKESKILL_START_HAVESKILL     3
#define ERROR_MAKESKILL_START_STATE         4
#define ERROR_MAKESKILL_START_DBERROR       5
#define ERROR_MAKESKILL_START_COUNT         6
#define ERROR_MAKESKILL_START_STEP          7
#define ERROR_MAKESKILL_START_EXP           8
/// MSG_REQ_MAKESKILL_ITEMMIX_START = MSG_ERROR

struct MSG_REQ_MAKESKILL_ITEMMIX_START : public MSGROOT
{
	unsigned char	mMakeSkill;
	unsigned long	mRecipeIdx;
	unsigned short	mMakeCnt;
};

struct MSG_SYN_MAKESKILL_ITEMMIX_START : public MSGROOT
{
	unsigned long	mCharacterIdx;
};


/// MSG_RES_MAKESKILL_ITEMMIX_END : public MSG_ERROR
#define ERROR_MAKESKILL_END_SUCCESS     0
#define ERROR_MAKESKILL_END_ITEM        1
#define ERROR_MAKESKILL_END_STATE       2
#define ERROR_MAKESKILL_END_TIME        3

struct MSG_SYN_MAKESKILL_ITEMMIX_MIDDLEEND : public MSGROOT
{
	unsigned long	mCharacterIdx;
};

struct MSG_SYN_MAKESKILL_ITEMMIX_END : public MSG_ERROR
{
	unsigned long	mCharacterIdx;
};


#define ERROR_MAKESKILL_CANCEL_SUCCESS    0
#define ERROR_MAKESKILL_CANCEL_DBWORK     1

struct MSG_SYN_MAKESKILL_ITEMMIX_CANCEL : public MSGROOT
{
	unsigned long	mCharacterIdx;
};

struct MSG_SYN_MAKESKILL_ITEMMIX_CANCELFORCE : public MSGROOT
{};

struct MSG_SYN_MAKESKILL_ITEMMIX_EXP : public MSGROOT
{
	unsigned char	mMakeSkill;
	unsigned long	mMakeSkillExp;
};

#pragma pack(pop)
