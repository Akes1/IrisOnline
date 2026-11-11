/* ==========================================================================
* 파    일 : TriggerManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-01-24
* 주의사항 :
*===========================================================================*/
#pragma once

enum eCheckTriggerResult
{
	eCHECK_TRIGGER_SUCCESS = 0,		 ///체크 성공
	eCHECK_TRIGGER_ERROR,			 ///체크 에러.(현재 고레벨 퀘스트를 제외한 나머지 에러 값을 구분하지 않는다.)
	eCHECK_TRIGGER_NEXT_LEVEL_QUEST, /// 고레벨 퀘스트임
};

const int NEXT_LEVEL_QUEST_GAP = 2;

/// 트리거 정보
class cTrigger
{
	friend class cQuestManager;
	friend class cTriggerManager;

public:
	cTrigger();

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

	/// 퀘스트 추가 조건 가능여부 판단
	eCheckTriggerResult CheckTrigger( unsigned long playerIdx, unsigned long questIdx );

	/// 퀘스트 진행시 조건 판단
	bool CheckDuty( unsigned long heroIdx, unsigned long otherIdx, unsigned long questIdx );

	bool AddTrigger( unsigned long questIdx, cTrigger* trigger );
	void DeleteTrigger( unsigned long questIdx );

	/// 만기일계산
	TIMESTAMP_STRUCT GetValidTime( unsigned long questIdx );

protected:
	cTrigger* GetTrigger( unsigned long questIdx );

public:
	static cTriggerManager* GetSingleton();

private:
	/// 트리거 맵 
	typedef tPointerHashMap<unsigned long, void*> cTriggerMap;
	cTriggerMap mTriggerMap;
};

inline
cTriggerManager* cTriggerManager::GetSingleton()
{
	return mpSingleton;
}

#define TRIGGERMAN cTriggerManager::GetSingleton()

