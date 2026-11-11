/* ==========================================================================
* 파    일 : MonsterScript.h
* 목    적 : 몬스터 리소스를 읽어와 몬스터에 셋팅해준다
* 작 성 자 : 박경희
* 작 성 일 : 2007-11-12 
* 주의사항 : 
*===========================================================================*/
#pragma once

struct sMonsterScript;
struct sRegenScript;
struct sRegenGroupScript;
struct sMonsterTalk;
struct sMonsterPattern;

class cModeAgent;

const long RAND_DIRECTION = 1000;	/// 스크립트에서 방향 랜덤 값

class cMonsterScript
{
public:

	static cMonsterScript*		mpMonsterScript;

public:

	/// 생성 소멸
	cMonsterScript(void);
	~cMonsterScript(void) {}


	bool				Init();
	bool				StageInit( unsigned short mapNumber, unsigned char levelMode );
	void				Release();


	/// 몬스터 리스트
	sMonsterScript*		GetMonsterListInfo( unsigned long monsterIdx );
	const tPointerHashMap<unsigned long, void*>* GetMonsterMap() { return (tPointerHashMap<unsigned long, void*>*)mpMonsterMap; }


	/// 몬스터 리젠 리스트
	sRegenScript*		GetRegenMonsterInfo( unsigned char modeNum, unsigned long monsterIdx );
	sRegenGroupScript*	GetRegenGroupInfo( unsigned char modeNum, unsigned long groupIdx );
	tPointerHashMap<unsigned long, void*>* GetRegenMonsterMap( unsigned char modeNum ) { return (tPointerHashMap<unsigned long, void*>*)mpRegenMonsterMap->GetAt( modeNum ); }
	tPointerHashMap<unsigned long, void*>* GetRegenGroupMap( unsigned char modeNum ) { return (tPointerHashMap<unsigned long, void*>*)mpRegenGroupMap->GetAt( modeNum ); }


	/// 모델 정보 
	tHashSet<unsigned long>*	GetMonsterModelNumber( unsigned long map ) { return (tHashSet<unsigned long>*)mpModelNumberMap->GetAt( map ); }


	/// 몬스터 대사
	sMonsterTalk*		GetMonsterTalkInfo( unsigned long monsterClassIdx, unsigned char state );


	/// 몬스터 리젠 스위치 그룹
	tPointerHashMap<unsigned long, void*>*	GetSwitchGroup( unsigned char mapNum );


	/// 몬스터 Mode
	void				LoadMonsterMode( unsigned long monsterIdx );
	void				ReleaseMonsterMode();

	cModeAgent*			GetMonsterModeAgent( unsigned long monsterIdx );

	/// 이동 패턴
	sMonsterPattern*	GetMonsterPattern( unsigned char levelMode, unsigned long patternIdx );

	/// 소환 패턴
	tArray<void*>*		GetSummonPattern( unsigned long summonPatternIdx )	{ return (tArray<void*>*)mpSummonPatternMap->GetAt( summonPatternIdx ); }

	// 오토 더미 랜덤 뽑기
	unsigned long		GetRandomDummy();

private:

	/// 몬스터 리스트
	bool				LoadMonsterList();
	void				ReleaseMonsterList();

	/// 몬스터 리젠 리스트
	bool				LoadRegenMonsterlist( unsigned short mapnum, unsigned char levelMode, 
		tPointerHashMap<unsigned long, void*>* pRegenMonsterMap,
		tPointerHashMap<unsigned long, void*>* pRegenGroupMap,
		tPointerHashMap<unsigned long, void*>* pRegenSwitchGroup );
	void				ReleaseRegenMonster();
	void				ReleaseRegenGroup();

	/// 모델 정보
	void				ReleaseModelNumber();

	/// 몬스터 대사
	bool				LoadMonsterTalkList();
	void				ReleaseMonsterTalk();

	/// 몬스터 리젠 스위치 그룹
	void				ReleaseRegenSwitchGroup();

	/// 몬스터 패턴
	bool				LoadMonsterPattern( unsigned short mapnum, unsigned char levelMode,
		tPointerHashMap<unsigned long, void*>* pMonsterPattern );
	void				ReleaseMonsterPattern();

	/// 소환 패턴
	bool				LoadSummonPattern();
	void				ReleaseSummonPattern();

private:

	typedef tArray<unsigned long> cArray;
	typedef tArray<void*> cAryVoid;
	typedef tPointerHashSet<unsigned long> cHashSet;
	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;

	/// 몬스터 리스트 해쉬맵
	//	typedef tPointerHashMap<unsigned long, sMonsterScript*> cMonsterMap;
	cPointHashMap*		mpMonsterMap;

	/// 리젠 몬스터 해쉬맵
	//	typedef tPointerHashMap<unsigned long, sRegenScript*> cRegenMonsterMap;
	cPointHashMap*		mpRegenMonsterMap;

	/// 리젠 그룹 해쉬맵
	//	typedef tPointerHashMap<unsigned long, sRegenGroupScript*> cRegenGroupMap;
	cPointHashMap*		mpRegenGroupMap;

	/// 맵별 모델 넘버 해쉬맵
	// typedef tPointerHashMap<unsigned long, cHashSet*> cModelNumberMap;
	cPointHashMap*		mpModelNumberMap;

	/// 몬스터 대사
	// typedef tPointerHashmap<unsigned long, sMonsterTalk[eMONSTERTALK_MAX]> cMonsterTalkMap;
	cPointHashMap*		mpMonsterTalkMap;

	/// 몬스터 Mode
	// typedef tPointerHashmap<unsigned long, cModeAgent*> mpMonsterModeAgentMap;
	cPointHashMap*		mpMonsterModeAgentMap;

	/// 몬스터리젠 선택 그룹 
	// typedef tArray<unsigned long> cRegenGroupAry;
	// typedef tPointerHashmap<unsigned long, cRegenGroupAry*> cMonsterTalkMap;
	cPointHashMap*		mpRegenSwitchGroup;

	/// 몬스터 이동 패턴
	/// <unsigned long patternIdx, sMonsterPattern*>
	cPointHashMap*		mpMonsterPattern;

	/// 소환 패턴
	/// <sSummonPatternScript*> 
	/// <unsigned long summonPatternIdx, void* >
	cPointHashMap*		mpSummonPatternMap;

	// 더미몬스터 소환가능 목록
	cArray			mDummyArray;
};

#define MONSTERSCRIPT cMonsterScript::mpMonsterScript
