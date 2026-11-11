/* ==========================================================================
* 파    일 : NpcScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-04-25
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"

struct sNPCList;
struct sNPCRegen;
struct sNpcFuncData;

//////////////////////////////////////////////////////
/// Npc 토큰 타입
enum
{
	/// Npc 대화
	eTOKEN_NPC_TELLING = 100,

	/// 기능
	eTOKEN_NPC_STORE,
	eTOKEN_NPC_WAREHOUSE,
	eTOKEN_NPC_DISJOINT,
	eTOKEN_NPC_GUILD,
	eTOKEN_NPC_FORTUNETAROT,
	eTOKEN_NPC_DEAL,
	eTOKEN_NPC_TAROT,

	eTOKEN_NPC_FORTUNESPREAD,
	eTOKEN_NPC_SPREAD,
	eTOKEN_NPC_THEME,
	eTOKEN_NPC_SKILLPREVIEW,
	eTOKEN_NPC_ENHANCE_RESET,
	eTOKEN_NPC_ENHANCE_EXCHANGE,
	eTOKEN_NPC_EVENT,

	eTOKEN_NPC_TEXT,
	eTOKEN_NPC_PAGE,

	/// 기본 대사
	eTOKEN_NPC_DEFAULT,

	/// 퀘스트
	eTOKEN_NPC_QUEST,
	eTOKEN_NPC_NEW,
	eTOKEN_NPC_PLAY,
	eTOKEN_NPC_COMPLETE,

	eTOKEN_NPC_ANSWER,
	eTOKEN_NPC_SOUND,
	eTOKEN_NPC_SCENE,

	eTOKEN_NPC_NEXT,
	eTOKEN_NPC_EVENTYES,
	eTOKEN_NPC_EVENTREWARD,
	eTOKEN_NPC_EVENTCOMPLETE,

	eTOKEN_NPC_MAX,
};


/// Npc 어휘 분석기
class cNpcLexer : public cLexer
{
public:
	cNpcLexer( const char* buffer, unsigned int size );
};


class cParser;
class cNPCScript
{
public:
	static cNPCScript*	 mpNPCScript;

public:

	cNPCScript();
	virtual ~cNPCScript();

	bool						Init();
	bool						StageInit( unsigned short mapNumber, unsigned char levelMode );
	void						Release();


	/*--------------- NPC 리스트 ---------------*/
	/// 스크립트 로드
	bool						LoadNPCList();
	/// 정보 조회
	sNPCList*					GetNPCList( unsigned long idx );
	/// 정보 제거
	void						ReleaseNPCList();
	/// 해쉬맵 포인터를 넘김
	const tPointerHashMap<unsigned long, sNPCList*>* GetNPCListMap() { return (tPointerHashMap<unsigned long, sNPCList*>*)mpNPCListMap; }


	/*--------------- NPC 리젠 리스트 ---------------*/
	/// 스크립트 로드
	bool						LoadNPCRegen( unsigned short mapNumber, unsigned char levelMode,
		tPointerHashMap<unsigned long, void*>* pNPCRegenMap );
	/// 정보 조회
	sNPCRegen*					GetNPCRegen( unsigned char levelMode, unsigned long idx );
	/// 해당 맵에 있는 모든 NPC 정보 얻기
	void	  				    GetNPCRegen( unsigned char levelMode, unsigned long mapidx,  tList<void*>* pNPCList );

	/// NPC 리젠 사용 메모리 제거
	void						ReleaseNPCRegen();
	/// 리젠 리스트 해쉬맵 포인터를 넘김
	const tPointerHashMap<unsigned long, sNPCRegen*>* GetNPCRegenMap( unsigned char levelMode ) { return (tPointerHashMap<unsigned long, sNPCRegen*>*)mpNPCRegenMap->GetAt( levelMode ); }


	/*--------------- 모델 정보 ---------------*/
	/// 모델 정보 
	tHashSet<unsigned long>*	GetNpcModelNumber( unsigned long map ) { return (tHashSet<unsigned long>*)mpModelNumberMap->GetAt( map ); }
	/// 모델 정보 제거
	void						ReleaseModelNumber();

	/*--------------- 상점 정보 ---------------*/
	/// 스크립트 로드
	bool						LoadNPCStore();
	/// 스크립트 세부항목 로드
	bool						LoadNpcStoreDetail( cParser& parser, sNpcFuncData* data );
	/// 정보 조회
	bool						IsHaveNpcFunc( unsigned long classIdx, unsigned int func );

	sNpcFuncData*				GetNpcFuncData( unsigned long classIdx );

private:
	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;

	/// 모델 리스트
	typedef tPointerHashSet<unsigned long> cHashSet;

	/// NPC 리스트
	//typedef tPointerHashMap<unsigned int, sNPCList*> cNPCListMap;
	cPointHashMap*				mpNPCListMap;

	/// NPC 리스트
	//typedef tPointerHashMap<unsigned int, sNPCRegen*> cNPCRegenMap;
	cPointHashMap*				mpNPCRegenMap;

	/// 맵별 모델 넘버 해쉬맵
	// typedef tPointerHashMap<unsigned long, cHashSet*> cModelNumberMap;
	cPointHashMap*				mpModelNumberMap;


	/// npc 종류별 기능(상점,창고...)eNpcFunc
	//typedef tHashMap<unsigned long, unsigned long> cHashMap;
	//cHashMap*					mpNpcStore;

	/// npc 종류별 기능(상점,창고...) (sNpcFuncData)
	typedef tPointerHashMap<unsigned long, void*> cNpcFuncMap;
	cNpcFuncMap*		mpNpcFuncMap;
};

#define NPCSCRIPT cNPCScript::mpNPCScript
