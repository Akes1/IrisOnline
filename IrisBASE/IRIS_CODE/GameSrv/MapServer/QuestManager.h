/* ==========================================================================
* 파    일 : QuestManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-03-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"
#include "Quest_Common.h"
#include "QuestDefine.h"

const DWORD	QUEST_SAVETIME = 900000;
const DWORD QUEST_TIMEEVENT = 60 * 60 * 1000;	/// 1시간

struct sQuestMonster
{
	/// 몬스터 오브젝트 인덱스
	tArray<unsigned long> mMonsterArr;
};

/// 플레이어가 소환한 몬스터 정보
struct sQuestMonsterInfo
{
	/// 퀘스트인덱스, sQuestMonster*
	tPointerHashMap<unsigned long, void*> mQuestMonInfoMap;
};

/// 퀘스트 타입에 따른 관리구조
struct sQuestTypeArr
{
	/// 퀘스트 인덱스
	tArray<unsigned long> mTypeArr;
};

/// 퀘스트 매니저
class cParser;
class cQuestDefine;
class cTrigger;
class cPlayer;
class cQuestManager
{
public:
	static cQuestManager* mpSingleton;

public:
	cQuestManager();
	~cQuestManager();

	bool Init();
	void Clear();
	void Process( long elasedTime, long accumTime );

	/// 유효성 검사
	bool IsValid( cQuestDefine* define );

	/// 완료 가능한지 검사 ( 플레이어, 퀘스트 배열인덱스 )
	bool IsPossibleComplete( unsigned long playerIdx, int arrIdx );

	/// 선택보상에 있는 아이템인지 검사
	unsigned int IsSelectRewardItem( unsigned long questIdx, unsigned long ItemIdx );		/// 개수 리턴

	/// 선택보상에 있는 버프인지 검사
	bool IsSelectRewardBuff( unsigned long questIdx, unsigned long buffIdx );				/// 개수 리턴

	/// 해당 퀘스트를 실제로 npc가 소유했는지 검사
	bool IsHaveQuest( unsigned long questIdx, unsigned long npcClassIdx );

	/// 몬스터 소환정보 기록
	bool AddQuestMonster( unsigned long playerIdx, unsigned long questIdx, unsigned long monsterIdx );
	void DeleteQuestMonster( unsigned long playerIdx, unsigned long questIdx );
	void DeleteAllQuestMonster( unsigned long playerIdx );

	cQuestDefine* GetQuestDefine( unsigned long questIdx );
	cQuestReward* GetDefaultReward( unsigned long questIdx );

	///
	tPointerHashMap<unsigned long, void*>* GetQuestDefineMap() { return &mQuestDefine; }
	sNpcQuestList* GetNpcQuestMap( unsigned long npcClassIdx );

	/// 
	tArray<unsigned long>* GetQuestTypeArr( int type );

public:
	static cQuestManager* GetSingleton();
	
protected:
	bool Load( const cString& pathname );
	bool LoadNpcQuest( const cString& pathname );
	bool LoadQuests( const cString& pathname );
	bool LoadQuestData( cParser& parser, cQuestDefine* p, cTrigger* trigger );

	/// 
	bool LoadTrigger( cParser& parser, cTrigger* trigger, unsigned char& type );
	bool LoadDuty( cParser& parser, cQuestDuty* duty, __int64& complete );
	bool LoadLimit( cParser& parser, cQuestLimit* limit );
	bool LoadEnd( cParser& parser, cQuestEnd* end );
	bool LoadDefault( cParser& parser, cQuestReward* reward );
	bool LoadSelect( cParser& parser, cQuestReward* select );
	bool LoadFail( cParser& parser, cQuestFail* fail );

	bool CheckAura( unsigned long influenceIndex );

private:
	/// 퀘스트 스크립트 정보 맵
	typedef tPointerHashMap<unsigned long, void*> cQuestDefineMap;
	cQuestDefineMap mQuestDefine;

	/// NPC 보유 전체퀘스트 맵  ( npc 인덱스, sNpcQuestList ) 
	typedef tPointerHashMap<unsigned long, void*> cNpcQuestMap;
	cNpcQuestMap mNpcQuestMap;

	/// PLYAER 별 몬스터 소환 정보 ( player 인덱스, sQuestMonsterInfo )
	typedef tPointerHashMap<unsigned long, void*> cQuestMonsterMap;
	cQuestMonsterMap	mQuestMonsterMap;

	/// 상황별 퀘스트 리스트 ( 검색을 위한 관리구조 )
	sQuestTypeArr	mQuestType[eQUEST_MAX];

	long mAccumTime;
	long mElapsedTime;
};

inline tArray<unsigned long>* cQuestManager::GetQuestTypeArr( int type )
{
	if( type >= eQUEST_MAX )
	{
		assert(0);
		return 0;
	}
	return &mQuestType[type].mTypeArr;
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
	eTOKEN_QUEST = 100,
	eTOKEN_TITLE,
	eTOKEN_TYPE,
	eTOKEN_DESC,
	eTOKEN_REPEAT,
	eTOKEN_GROUP,

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
	eTOKEN_DEFAULT,
	eTOKEN_SELECT,
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

	eTOKEN_CHANGEJOB,
	eTOKEN_FORCETYPE,
	eTOKEN_GIVETITLE,

	eTOKEN_EXP,
	eTOKEN_SXP,
	eTOKEN_MONEY,
	eTOKEN_SKILLPOINT,
	eTOKEN_ITEM,

	/// 트리거 이벤트 관련
	eTOKEN_TRIGGER,
	eTOKEN_TE_LEVEL,
	eTOKEN_TE_RACE,
	eTOKEN_TE_GENDER,
	eTOKEN_TE_JOB,
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

	eTOKEN_TRUE,
	eTOKEN_FALSE,

	// 요일 지정
	eTOKEN_NONE,
	eTOKEN_MON,
	eTOKEN_TUE,
	eTOKEN_WED,
	eTOKEN_THU,
	eTOKEN_FRI,
	eTOKEN_SAT,
	eTOKEN_SUN,

	/// NPC 퀘스트 리스트 스크립트용
	eTOKEN_NPC,
};

/// 퀘스트 스크립트 어휘 분석기
class cQuestLexer : public cLexer
{
public:
	cQuestLexer( const char* buffer, unsigned int size );
};
