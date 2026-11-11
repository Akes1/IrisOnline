/* ==========================================================================
* 파    일 : Drop_Common.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-02-21
* 주의사항 :
*===========================================================================*/
#pragma once

#include "CommonNetworkMsgStruct.h"
#include "sqltypes.h"

#pragma pack(push,1)


const unsigned short EXP_DROP_RANGE = 4000;
const unsigned long	DROP_PER_FLOAT_CONVERT = 10;
const unsigned long RANDOM_MAX = 1000000;

enum eDROP_SERVERTYPE
{
	eDROPSERVERTYPE_ALL = 0,		/// 인던, 필드 모두 드랍
	eDROPSERVERTYPE_FIELD,			/// 필드만 드랍
	eDROPSERVERTYPE_INDUN,			/// 인던만 드랍
};

/// money 드랍 정보
struct sDropMoneyScript
{
	unsigned long	mClassIdx;						/// 종류idx
	unsigned long	mDropPercent;					/// 드랍 확률
	unsigned long	mMinMoney;						/// 드롭 최소 금액
	unsigned long	mMaxMoney;						/// 드롭 최대 금액
};

/// 몬스터 종류별 드랍 정보중 아이템그룹 목록
struct sDropItemGroupScript
{
	unsigned long	mItemGroupIdx;					/// 아이템 그룹 인덱스
	unsigned long	mItemGroupRate;					/// 아이템 그룹 선택 비율
};

/// 종류별(몬스터/채집) 드랍 정보
struct sDropClassScript
{
	unsigned long	mClassIdx;						/// 종류(몬스터/채집) idx
	
	//unsigned long	mNoAddDropRate;					/// 추가드랍 선택 안할 비율
	unsigned char	mAddDrop1Cnt;					/// 추가드랍1 횟수
	unsigned long	mAddDrop1Rate;					/// 추가드랍1 선택 비율
	unsigned char	mAddDrop2Cnt;					/// 추가드랍2 횟수
	unsigned long	mAddDrop2Rate;					/// 추가드랍2 선택 비율

	//unsigned long	mItemGroupRateTotal;			/// 아이템그룹 비율 총합
	/// <sDropItemGroupScript*>
	tArray<void*>	mItemGroup;						/// 아이템그룹

	sDropClassScript()
	{
		memset( this, 0, sizeof(sDropClassScript) );
	}
};

/// 몬스터 레벨,타입별 드랍 정보
struct sDropLevelTypeScript
{
	//unsigned char	mLevelMin;
	//unsigned char	mLevelMax;
	unsigned char	mMonsterType;					/// 몬스터 타입

	unsigned char	mServerTypeCheck;				/// 서버별 드랍 체크
	//unsigned long	mNoAddDropRate;					/// 추가드랍 선택 안할 확률
	unsigned char	mAddDrop1Cnt;					/// 추가드랍1 횟수
	unsigned long	mAddDrop1Rate;					/// 추가드랍1 선택 비율
	unsigned char	mAddDrop2Cnt;					/// 추가드랍2 횟수
	unsigned long	mAddDrop2Rate;					/// 추가드랍2 선택 비율

	//unsigned long	mItemGroupRateTotal;			/// 아이템그룹 비율 총합
	/// <sDropMonsterItemGroup*>
	tArray<void*>	mItemGroup;						/// 아이템그룹

	sDropLevelTypeScript()
	{
		memset( this, 0, sizeof(sDropLevelTypeScript) );
	}
};

/// 드랍 아이템 정보
struct sDropItemScript
{
	unsigned long	mItemIdx;						/// 아이템 종류 인덱스
	unsigned long	mItemRate;						/// 선택 비율
	unsigned long	mItemCount;						/// 아이템 수량
};

/// 드랍 아이템 목록 정보
struct sDropItemListScript
{
	unsigned long	mItemGroupIdx;					/// 아이템 그룹 인덱스	

	//unsigned long	mItemRateTotal;					/// 아이템그룹 비율 총합
	/// <sDropItemScript*>
	tArray<void*>	mItem;							/// 아이템그룹

	sDropItemListScript()
	{
		memset( this, 0, sizeof(sDropItemListScript) );
	}
};

/// 퀘스트 드랍 
struct sDropQType
{
	unsigned long	mItemIdx;		/// 드랍 아이템 인덱스
	unsigned long	mDropPer;		/// 드랍 확률
};

/// 채집 
struct sDropQTypeGather : public sDropQType				/// 퀘스트 드랍
{};

/// 플레이어가 몬스터에 준 데미지
struct sPlayerDamage
{
	unsigned long	mMonsterIdx;
	unsigned long	mPlayerIdx;
	long			mDamage;
};


/// 파티원에 따른 증가치
struct sPartyExp
{
	unsigned char	mPartyMemberCnt;
	float			mExpIncrease;
	float			mSxpIncrease;
};

struct sDropLimit
{
	unsigned long		mItemIdx;
	unsigned long		mDropTerm;
	TIMESTAMP_STRUCT	mEndTime;
	bool				mIsDrop;
};
#pragma pack(pop)
