/* ==========================================================================
* 파    일 : QuestManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-01-18
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"
#include "BaseStage.h"
#include "QuestDefine.h"

/// 퀘스트 팁보기 0.5초 간격
const unsigned long QUESTTIP_SHOWTIME = 500;

/// 퀘스트 매니저
class cParser;
class cQuestDefine;
class cUIFuncText;
class cUIFuncTextNode;
class cTrigger;
class cNpcWindow;

/// 네트워크 락
enum
{
	eLOCK_QUESTNONE,
	eLOCK_QUESTADD,
	eLOCK_QUESTADDBYITEM,
	eLOCK_QUESTCOMPLETE,
	eLOCK_QUESTREWARD,
	eLOCK_QUESTDELETE,
	eLOCK_QUESTDELETEAUTO,
};

/// 퀘스트 남은시간
struct sQuestTime
{
	long restTime;
	long startTime;
	
	sQuestTime()
	{
		restTime = 0;
		startTime = 0;
	}
};

// 
struct sNpcStausData
{
	// npc 종류 인덱스
	unsigned long	npcClassIndex;

	// 상태
	eNpcQuestStatus	status;
};

struct MSG_RES_NPC_QUEST_REWARD;
struct MSG_RES_NPC_QUEST_COMPLETE;
struct MSG_RES_NPC_QUEST_FAIL;

class cQuestManager
{
public:
	static cQuestManager* mpSingleton;

public:
	cQuestManager();
	~cQuestManager();

	bool Init();
 	cQuestDefine* GetQuestDefine( unsigned long questIdx );

 
 public:
 	static cQuestManager* GetSingleton();

private:
	bool Load( const cString& pathname );
	bool LoadNpcQuest( const cString& pathname );
	bool LoadQuests( const cString& pathname );
	bool LoadQuestData( cParser& parser, cQuestDefine* p, cTrigger* trigger );

	/// 
	bool LoadTrigger( cParser& parser, cTrigger* trigger );
	bool LoadInfo( cParser& parser, cQuestInfo* info );
	bool LoadDuty( cParser& parser, cQuestDuty* duty, __int64& complete );
	bool LoadLimit( cParser& parser, cQuestLimit* limit );
	bool LoadEnd( cParser& parser, cQuestEnd* end );
	bool LoadDefault( cParser& parser, cQuestReward* reward );
	bool LoadSelect( cParser& parser, cQuestReward* select );
	bool LoadFail( cParser& parser, cQuestFail* fail );

// 	/// 유효성 검사
 	bool IsValid( cQuestDefine* define );
 
 	bool CheckAura( unsigned long influenceIndex );
 
 	sNpcQuestList* GetNpcQuestMap( unsigned long npcClassIdx );
 
 	/// 현재 맵에 존재하는 NPC의 상태 갱신하기
 	eNpcQuestStatus UpdateNpcStatus( unsigned long npcIndex, unsigned long npcClassIndex );

private:
	/// 퀘스트 스크립트 정보 맵
	typedef tPointerHashMap<unsigned long, void*> cQuestDefineMap;
	cQuestDefineMap mQuestDefine;

	/// NPC 보유 전체퀘스트 맵  ( npc 인덱스, sNpcQuestList ) 
	typedef tPointerHashMap<unsigned long, void*> cNpcQuestMap;
	cNpcQuestMap mNpcQuestMap;

	/// 퀘스트 남은 시간 저장 (전부저장)
	typedef tPointerHashMap<unsigned long, void*> cTimeMap;
	cTimeMap	mTimeMap;

	/// 신규 퀘스트 리스트
	typedef tHashSet<unsigned long> cNewQuestSet;
	cNewQuestSet mNewQuestSet;

	/// 신규 이벤트 퀘스트 리스트
	typedef tHashSet<unsigned long> cNewEventQuestSet;
	cNewEventQuestSet mNewEventQuestSet;

	/// 퀘스트 정보 문자열
	cUIFuncText* mpQuestText;

	/// 퀘스트 정보 저장
	sQuestList mQuest[MAX_KEEPQUEST];
	unsigned int mTotalKeepCount;

	/// 완료가능목록 갯수
	unsigned int mCompleteCount;

	/// 진행중 목록 갯수
	unsigned int mProgressCount;

	/// 삭제나 완료로 인해, check 상태 풀어야하는 퀘스트
	unsigned long mCheckQuestIndex;

	/// 임시저장소
	unsigned long mTempQuestIndex;
	unsigned long mTempNpcIndex;
	unsigned int mTempArrIndex;

	/// 추적중인 퀘스트 
	unsigned long mChaseQuestIndex;

	/// 네트워크 관련 락
	bool	mNetLock;
	int		mLockTry;

	/// 선택된 퀘스트 정보 저장
	typedef tHashSet<unsigned long> cSelectSet;
	cSelectSet mSelectSet;

	/// 맵단위 퀘스트 갯수 저장 ( 맵인덱스, 갯수 )
	typedef tHashMap<unsigned int, unsigned int> cCountMap;
	cCountMap mCountMap;

	/// 각 npc 상태 저장
	typedef tPointerHashMap<unsigned long, void*> cNpcStatusMap;
	cNpcStatusMap	mNpcStatusMap;
};

inline cQuestManager* cQuestManager::GetSingleton()
{
	return mpSingleton;
}

#define QUESTMAN cQuestManager::GetSingleton()


//////////////////////////////////////////////////////////////////
// 퀘스트 스크립트 토큰
enum
{
	/// 퀘스트 선언
	eTOKEN_QUESTDATA = 100,
	eTOKEN_TITLE,
	eTOKEN_TYPE,
	eTOKEN_DESC,
	eTOKEN_REPEAT,
	eTOKEN_GROUP,
	eTOKEN_SHARETYPE,
	
	eTOKEN_GIVENPC,
	eTOKEN_TAKENPC,
	eTOKEN_TAKEMAP,
	eTOKEN_GIVEITEM,
	eTOKEN_TAKEITEM,
	eTOKEN_SELECTCOUNT,
	eTOKEN_LEVEL,
	
	eTOKEN_CHALLEANGE,
	eTOKEN_REPETITION,

	eTOKEN_ALL,
	eTOKEN_ORDER,

	eTOKEN_INFO,
	eTOKEN_DUTY,
	eTOKEN_LIMIT,
	eTOKEN_END,
	eTOKEN_DEFAULTREWARD,
	eTOKEN_SELECTREWARD,
	eTOKEN_FAIL,

	eTOKEN_TALKNPC,
	eTOKEN_HUNT,
	eTOKEN_COLLECT,
	eTOKEN_PVP,
	eTOKEN_ACTIVESKILL,
	eTOKEN_ENHANCE,
	eTOKEN_PUTCARD,
	eTOKEN_INFOMAP,
	
	eTOKEN_BUFF,
	eTOKEN_DEBUFF,
	eTOKEN_EQUIPITEM,
	eTOKEN_TIMETYPE,
	eTOKEN_TIME,
	eTOKEN_MAPCHANGEPOS,
	eTOKEN_MONSTER,
	eTOKEN_MONSTERTIME, 
	eTOKEN_TAKEMONEY,
	eTOKEN_TAKEFIRE,
	eTOKEN_TAKEWATER,
	eTOKEN_TAKEWIND,
	eTOKEN_TAKEEARTH,
	eTOKEN_TUTORIALMODE,

	eTOKEN_EXP,
	eTOKEN_SXP,
	eTOKEN_MONEY,
	eTOKEN_SKILLPOINT,
	eTOKEN_ITEM,
	eTOKEN_CHANGEJOB,
	eTOKEN_FORCETYPE,
	eTOKEN_GIVETITLE,

	/// 트리거 이벤트 관련
	eTOKEN_TRIGGER,
	eTOKEN_TE_LEVEL,
	eTOKEN_TE_RACE,
	eTOKEN_TE_JOB,
	eTOKEN_TE_GENDER,
	eTOKEN_TE_HAVEITEM,
	eTOKEN_TE_LINKQUEST,
	eTOKEN_TE_DAYEVENT,
	eTOKEN_TE_TIMEEVENT,
	eTOKEN_TE_TRANSMON,
	eTOKEN_TE_SETTITLE,
	eTOKEN_TE_SJOBSKILL,

	eTOKEN_HUMAN,
	eTOKEN_ELF,
	eTOKEN_BEAST,

	// 요일 지정
	eTOKEN_NONE,
	eTOKEN_MON,
	eTOKEN_TUE,
	eTOKEN_WED,
	eTOKEN_THU,
	eTOKEN_FRI,
	eTOKEN_SAT,
	eTOKEN_SUN,

	eTOKEN_TRUE,
	eTOKEN_FALSE,
};

/// 퀘스트 스크립트 어휘 분석기
class cQuestLexer : public cLexer
{
public:
	cQuestLexer( const char* buffer, unsigned int size );
};
