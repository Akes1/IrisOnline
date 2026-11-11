/* ==========================================================================
* 파    일 : QuestDefine.h
* 목    적 : 퀘스트스크립트
* 작 성 자 : 김가람
* 작 성 일 : 2008-01-18
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Quest_Common.h"

/// 수행 정보 ( 대상 인덱스, (수행타입, 갯수) )
/// ( 조건은 하위비트부터 채워진다 )
struct sDuty
{
	eDutyType dutyType;
	unsigned long targetIdx;
	unsigned int count;
	unsigned long skillIndex;
	
	bool enhance;
	bool putcard;

	sDuty::sDuty()
	{
		dutyType = eDUTY_NONE;
		targetIdx = 0;
		count = 0;
		skillIndex = 0;
		enhance = false;
		putcard = false;
	}
};

// 퀘스트 이미지 정보
struct sQuestInfoData
{
	unsigned short	mapIndex;		// 표시 맵 인덱스
	unsigned short	x;				// 화면상 x
	unsigned short	y;				// 화면상 y
	float 			scale;			// 비율 ( 기본 1.0 )
	unsigned char	showName;		// 이름표시( 1: 표시, 0: 미표시 )

	sQuestInfoData()
	{
		mapIndex = 0;
		x = 0;
		y = 0;
		scale = 1.0f;
		showName = true;
	}
};

// 퀘스트 도움 정보
class cQuestInfo
{
	friend class cQuestManager;

public:
	cQuestInfo() {}
	~cQuestInfo() {}

public:
	sQuestInfoData mData[MAX_DUTY];
};

/// 퀘스트 임무
class cQuestDuty
{
	friend class cQuestManager;

public:
	cQuestDuty() {}
	~cQuestDuty() {}

public:
    /// 수행정보
	sDuty mDuty[MAX_DUTY];
};

/// 퀘스트 수행 제약 및 수행시 제공
class cQuestLimit
{
	friend class cQuestManager;

public:
	cQuestLimit();

public:
	/// 제약 버프
	unsigned long mLimitBuff[QUEST_BUFF_MAX];

	/// 제약 디버프
	unsigned long mLimitDeBuff[QUEST_BUFF_MAX];

	/// 제약 사용 아이템 (장착)
	unsigned long mLimitEquipItem[QUEST_ITEM_MAX];

	/// 제한시간타입 ( 0 :무제한, 1:실시간, 2:게임시간 )
	unsigned char mTimeType;

	/// 제한시간
	unsigned long mTime;

	/// 맵이동 인덱스
	unsigned long mMapChangeIndex;

	/// 소환 몬스터 고유번호
	unsigned long mMonsterClassIndex;
	unsigned int mMonsterCount;
	unsigned long mMonsterTime;

	/// 퀘스트 부여시 주는 아이템 정보
	sQuestItem mGiveItem[QUEST_ITEM_MAX];

	/// 지불할 금액
	unsigned long mTakeMoney;

	/// 지불할 포인트
	unsigned long mTakeFire;
	unsigned long mTakeWater;
	unsigned long mTakeWind;
	unsigned long mTakeEarth;

	/// 변경할 튜토리얼 모드
	int			mTutorialModeIndex;
};

inline cQuestLimit::cQuestLimit()
: mTime(0)
, mTimeType(0)
, mMapChangeIndex(0)
, mMonsterClassIndex(0)
, mMonsterCount(0)
, mMonsterTime(0)
, mTakeMoney(0)
, mTakeFire(0)
, mTakeWater(0)
, mTakeWind(0)
, mTakeEarth(0)
, mTutorialModeIndex(-1)
{
	memset( &mLimitBuff, 0, sizeof(mLimitBuff) );
	memset( &mLimitDeBuff, 0, sizeof(mLimitDeBuff) );
	memset( &mLimitEquipItem, 0, sizeof(mLimitEquipItem) );
	memset( &mGiveItem, 0, sizeof(mGiveItem) );
}

/// 퀘스트수행 완료( 실패, 포기, 완료 )
class cQuestEnd
{
	friend class cQuestManager;

public:
	cQuestEnd();

public:
	/// 종료할 버프
	unsigned long mEndBuff[QUEST_BUFF_MAX];

	/// 종료할 디버프
	unsigned long mEndDeBuff[QUEST_BUFF_MAX];

	/// 종료시 가져가는 아이템 정보
	sQuestItem mTakeItem[QUEST_ITEM_MAX];

	/// 변경할 튜토리얼 모드
	int			mTutorialModeIndex;
};

inline cQuestEnd::cQuestEnd()
: mTutorialModeIndex(-1)
{
	memset( &mEndBuff, 0, sizeof(mEndBuff) );
	memset( &mEndDeBuff, 0, sizeof(mEndDeBuff) );
	memset( &mTakeItem, 0, sizeof(mTakeItem) );
}

/// 퀘스트 보상
class cQuestReward
{
	friend class cQuestManager;

public:
	cQuestReward();

public:
	/// 경험치 (기본보상)
	unsigned int mExp;

	/// 스킬 경험치 (기본보상)
	unsigned int mSxp;

	/// 금액 (기본보상)
	unsigned long mMoney;

	/// 스킬포인트 (기본보상)
	unsigned int mSkillPoint;

	/// 맵이동 인덱스 (기본보상)
	unsigned long mMapChangeIndex;

	/// 전직인덱스 
	unsigned short mChangeJobIndex;

	/// 세력종류 (eFORCE_TYPE)
	unsigned char mForceType;

	/// 호칭
	unsigned long mGiveTitleIndex;

	/// 버프
	unsigned long mRewardBuff[QUEST_BUFF_MAX];

	/// 아이템 인덱스, 갯수
	sQuestItem mRewardItem[QUEST_ITEM_MAX];
};

inline
cQuestReward::cQuestReward()
: mExp(0)
, mSxp(0)
, mMoney(0)
, mSkillPoint(0)
, mChangeJobIndex(0)
, mForceType(0)
, mGiveTitleIndex(0)
, mMapChangeIndex(0)
{
	for( unsigned int i = 0; i < QUEST_BUFF_MAX; ++i )
		mRewardBuff[i] = 0;

	for( unsigned int i = 0; i < QUEST_ITEM_MAX; ++i )
	{
		mRewardItem[i].itemIndex = 0;
		mRewardItem[i].count = 0;
	}
}

/// 퀘스트 실패시
class cQuestFail
{
	friend class cQuestManager;

public:
	cQuestFail();

public:
	/// 실패시 버프
	unsigned long mFailBuff[QUEST_BUFF_MAX];

	/// 실패시 디버프
	unsigned long mFailDeBuff[QUEST_BUFF_MAX];
};

inline
cQuestFail::cQuestFail()
{
	for( unsigned int i = 0; i < QUEST_BUFF_MAX; ++i )
	{
		mFailBuff[i] = 0;
		mFailDeBuff[i] = 0;
	}
}

enum eQuestType
{
	eQUEST_TYPE_NORMAL = 0,
	eQUEST_TYPE_TUTORIAL,
	eQUEST_TYPE_THEME,
	eQUEST_TYPE_REPEAT,
};

/// 퀘스트 정의
class cQuestDefine
{
	friend class cQuestManager;

public:
	cQuestDefine();
	~cQuestDefine();

public:
	// 퀘스트 타입 ( eQUESTTYPE )
	unsigned char mType;

	/// 퀘스트 인덱스
	unsigned long mIndex;

	/// 퀘스트명
	unsigned long mTitleIndex;
	
	/// 설명 텍스트 인덱스
	unsigned long mDestIndex;

	/// 설명 문자열 줄수
	unsigned int mDestLineNum;

	/// 퀘스트 타입 (eQUESTTYPE)
	unsigned char mEventType;

	/// 반복 타입 
	unsigned char mRepeatType;

	/// 그룹번호 
	unsigned short mGroup;

	/// 부여 npc 인덱스
	unsigned long mGiveNpcIndex;

	/// 보상 npc 인덱스
	unsigned long mTakeNpcIndex;

	/// 보상 맵 인덱스 
	unsigned short mTakeMapIndex;

	/// 선택보상 선택 가능 개수
	unsigned int mSelectCount;

	/// 퀘스트 적정 레벨 표시용 
	unsigned int mLevel;

	// 퀘스트 도움 정보
	cQuestInfo* mQuestInfo;

	/// 퀘스트 임무 정보
	cQuestDuty* mQuestDuty;

	/// 퀘스트 기본 보상정보
	cQuestReward* mDefaultReward;

	/// 퀘스트 선택 보상정보
	cQuestReward* mSelectReward;

	/// 퀘스트 수행 제약정보
	cQuestLimit* mQuestLimit;

	/// 퀘스트 종료시 정보(완료,포기,실패)
	cQuestEnd*	mQuestEnd;

	/// 퀘스트 실패시 
	cQuestFail* mQuestFail;

	/// 완료 조건 검사용 비트
	__int64 mComplete;
};

inline
cQuestDefine::cQuestDefine()
: mTitleIndex(0)
, mDestIndex(0)
, mDestLineNum(0)
, mEventType( eQUEST_MAX )
, mRepeatType( eQUEST_CHALLENGE )
, mGroup(0)
, mGiveNpcIndex(0)
, mTakeNpcIndex(0)
, mTakeMapIndex(USHRT_MAX)
, mSelectCount(0)
, mQuestInfo(0)
, mQuestDuty(0)
, mDefaultReward(0)
, mSelectReward(0)
, mQuestLimit(0)
, mQuestEnd(0)
, mQuestFail(0)
, mComplete(0)
, mLevel(1)
, mType(eQUEST_TYPE_NORMAL)
{
}

inline 
cQuestDefine::~cQuestDefine()
{
	SAFE_DELETE( mQuestInfo );
	SAFE_DELETE( mQuestDuty );
	SAFE_DELETE( mDefaultReward );
	SAFE_DELETE( mSelectReward );
	SAFE_DELETE( mQuestLimit );
	SAFE_DELETE( mQuestFail );
	SAFE_DELETE( mQuestEnd );
}
