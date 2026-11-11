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

	// 이펙트 상태 저장
	eNpcQuestEffect	effectType;
};

const int LOW_LEVEL_QUEST_GAP = 10;

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
	void Close();		/// 맵체인지
	void OpenLoad();	/// 각 스테이지에서의 오픈
	void OpenMapChange();
	void Process( unsigned long accumTime );

	//
	void SetQuestTip( unsigned long npcclassIndex ); // 퀘스트 툴팁 세팅

	// npc가 제공할 신규 퀘스트 목록 임시 저장소
	void SetNpcQuestList( cNpcWindow* win, unsigned long npcIndex, unsigned long npcClassIndex );		/// NPC에 퀘스트 리스트 세팅
	bool SetBoardQuestList();		/// 게시판용 퀘스트 리스트 세팅

	void AddQuestSelect( unsigned long questIndex, bool setQuestWindow = false );
	void DeleteQuestSelect( unsigned long questIndex, bool setQuestWindow = false );
	bool IsSelectQuest( unsigned long questIdx );

	/// 해당 채집아이템 관련 퀘스트가 완료되었는지 검사
	bool IsCompleteGathering( unsigned long questIndex );

	// npc 퀘스트 상태 저장
	void AddNpcStatus( unsigned long npcIndex, unsigned long npcClassIndex );
	void UpdateNpcStatus();
	eNpcQuestEffect GetNpcStatus( unsigned long npcIndex );

	// 추적중 퀘스트 세팅
	void UpdateChaseQuest( unsigned long questIndex );
	unsigned long GetChangeQuestIndex() { return mChaseQuestIndex; }
	void ClearChaseQuest();

	__int64& GetProgress( unsigned int arrIdx );		/// 퀘스트 진행정보
	unsigned long GetQuestIndex( unsigned int arrIdx );
	unsigned char GetQuestStatus( unsigned int arrIdx );
	int GetQuestArrIdx( unsigned long questIdx );
	cQuestDefine* GetQuestDefine( unsigned long questIdx );
	long GetQuestRestTime( unsigned long questIdx );
	unsigned int GetCount( unsigned short mapIdx );		/// 맵에 소유한 퀘스트 총 갯수 리턴

	void NetUnLock( int lockTry );

	/// 퀘스트 설명용 텍스트 획득
	cUIFuncTextNode* GetNpcTextNode( unsigned int textid );

	/// 네트워크 메시지
	void NetworkMsgParserLoad( char* msg );			/// Load, MapChange 시 오는 패킷
	void NetworkMsgParser( char* msg );
	void ResQuestAdd( MSG_RES_QUEST_ADD* pmsg );
	void ResQuestComplete( MSG_RES_NPC_QUEST_COMPLETE* pmsg );
	void ResQuestReward( MSG_RES_NPC_QUEST_REWARD* pmsg );

	/// 패킷 쏘기
	bool QuestAddByNpc( unsigned long questIdx, unsigned long npcIndex );	/// 퀘스트 추가 (NPC에 의함)
	bool QuestAddByItem( unsigned short number );								/// 퀘스트 추가 (아이템에 의함)
	bool QuestComplete( unsigned long questIdx, unsigned long npcIndex );	/// 퀘스트 진행 완료
	bool QuestComplete();
	bool QuestReward( unsigned long questIdx, unsigned long npcIndex );		/// 퀘스트 보상
	void QuestDelete( unsigned int arrIdx );		/// 퀘스트 포기
	void QuestDeleteAuto( unsigned int arrIdx );	/// 퀘스트 자동 포기

	tHashSet<unsigned long>& GetSelectedSet() { return mSelectSet; }
	
public:
	static cQuestManager* GetSingleton();

private:
	void UpdateUIAll();						/// 전체 퀘스트 리스트 갱신
	void UpdateUI();						/// 퀘스트 윈도우만 갱신

	/// 보유, 완료가능 개수 갱신
	void CheckQuestCount();

	///
	void ActiveQuestText( sQuestList* oldQuest, sQuestList* newQuest );

	///
	void AddQuestTime( unsigned long questIdx, unsigned long restTime );
	void DeleteQuestTime( unsigned long questIdx );
	void ClearQuestTime();

	/// 각 맵별로 소유한 퀘스트 갯수 저장
	void CheckCountMap();
	
	/// 락걸기
	void NetLock( int lockTry, unsigned short slotIndex0 = USHRT_MAX);
	bool IsNetLock();

private:
	bool Load( const cString& pathname );
	bool LoadNpcQuest( const cString& pathname );
	bool LoadQuests( const cString& pathname );
	bool LoadQuestData( cParser& parser, cQuestDefine* p, cTrigger* trigger );

	/// 
	bool LoadTrigger( cParser& parser, cTrigger* trigger, unsigned char& type );
	bool LoadInfo( cParser& parser, cQuestInfo* info );
	bool LoadDuty( cParser& parser, cQuestDuty* duty, __int64& complete );
	bool LoadLimit( cParser& parser, cQuestLimit* limit );
	bool LoadEnd( cParser& parser, cQuestEnd* end );
	bool LoadDefault( cParser& parser, cQuestReward* reward );
	bool LoadSelect( cParser& parser, cQuestReward* select );
	bool LoadFail( cParser& parser, cQuestFail* fail );

	/// 유효성 검사
	bool IsValid( cQuestDefine* define );

	bool CheckAura( unsigned long influenceIndex );

	sNpcQuestList* GetNpcQuestMap( unsigned long npcClassIdx );

	/// 현재 맵에 존재하는 NPC의 이펙트 상태 갱신하기
	eNpcQuestEffect UpdateNpcEffect( unsigned long npcIndex, unsigned long npcClassIndex );

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

	typedef tHashSet<unsigned int> cMapIndexSet;
	/// Quest Window에서 접혀저 있는 맵 인덱스 저장
	cMapIndexSet mClosedCategory;
	/// simple Quest Window에서 접혀저 있는 맵 인덱스 저장
	cMapIndexSet mClosedSimpleCategory;

	/// 맵단위 퀘스트 갯수 저장 ( 맵인덱스, 갯수 )
	typedef tHashMap<unsigned int, unsigned int> cCountMap;
	cCountMap mCountMap;

	/// 각 npc 상태 저장
	typedef tPointerHashMap<unsigned long, void*> cNpcStatusMap;
	cNpcStatusMap	mNpcStatusMap;
};

inline bool cQuestManager::IsNetLock()
{
	if( mNetLock == true )
		assert(0);

	return mNetLock;
}

inline __int64& cQuestManager::GetProgress( unsigned int arrIdx )
{
	return mQuest[arrIdx].check;
}

inline int cQuestManager::GetQuestArrIdx( unsigned long questIdx )
{
	for( unsigned int i = 0; i < mTotalKeepCount; ++i )
	{
		if( mQuest[i].questIdx == questIdx )
			return i;
	}
	return -1;
}

inline unsigned long cQuestManager::GetQuestIndex( unsigned int arrIdx )
{
	return mQuest[arrIdx].questIdx;
}

inline unsigned char cQuestManager::GetQuestStatus( unsigned int arrIdx )
{
	return mQuest[arrIdx].status;
}

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
