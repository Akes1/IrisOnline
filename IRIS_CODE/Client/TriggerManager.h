/* ==========================================================================
* 파    일 : TriggerManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-08-01
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Quest_Common.h"

/// 트리거 정보
class cTrigger
{
	friend class cQuestManager;
	friend class cTriggerManager;

public:
	cTrigger();
	short GetLowLevel(){ return mLowLevel; }

private:
	/// 발생 퀘스트 인덱스
	unsigned long mQuestIndex;

	/// 최저 레벨
	short mLowLevel;

	/// 최고 레벨
	short mHighLevel;

	/// 종족
	short mRace[2];

	/// 성별
	short mGender;

	/// 가능한 직업
	typedef tHashSet<unsigned long> cJobSet;
	cJobSet mJobSet;

	/// 보유해야하는 아이템
	sQuestItem mHaveItem[QUEST_ITEM_MAX];

	/// 퀘스트매니저 - 완료 목록 참조
	/// 하위 연계 퀘스트 인덱스
	typedef tArray<unsigned long> cLinkQuestArr;
	cLinkQuestArr mLinkQuestArr;
 
	/// 요일 타입
	short mDayCount;
	short mDayType[eDAY_MAX];

	/// 시간 범위
	short mStartHour;
	short mEndHour;

	/// 변신 몬스터 인덱스
	unsigned long mTransmonIndex;

	/// 타이틀 인덱스
	unsigned long mTitleIndex;

	// 보조직업 인덱스 (eMAKESKILL)
	unsigned char mMakeSkillIndex;

	// 숙련도 
	unsigned long mMakeSkillExp;	// 최소
	unsigned long mMaxMakeSkillExp;	// 최대
};


/// 트리거 매니저
class cTriggerManager
{
	static cTriggerManager* mpSingleton;

public:	
	cTriggerManager();
	~cTriggerManager();

	bool Init();
	void Clear();
	
	bool AddTrigger( unsigned long questIdx, cTrigger* trigger );
	void DeleteTrigger( unsigned long questIdx );
	short GetLowLevel( unsigned long questIdx );

private:
	cTrigger* GetTrigger( unsigned long questIdx );

public:
	static cTriggerManager* GetSingleton();

private:
	/// 트리거 맵 (questidx, cTrigger*)
	typedef tPointerHashMap<unsigned long, void*> cTriggerMap;
	cTriggerMap mTriggerMap;
};

inline
cTriggerManager* cTriggerManager::GetSingleton()
{
	return mpSingleton;
}

#define TRIGGERMAN cTriggerManager::GetSingleton()

