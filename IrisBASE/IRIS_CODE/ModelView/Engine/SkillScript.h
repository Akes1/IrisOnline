/* ==========================================================================
* 파    일 : SkillScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-06-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Skill_Common.h"


typedef tPointerHashMap<unsigned long, void*> cSkillHashMap;
typedef tArray<void*>	cArray;

class cSkillScript
{
public:

	static cSkillScript*	 mpSkillScript;

public:

	/// 생성 소멸
	cSkillScript(void);
	~cSkillScript(void) {}

	///	초기화
	bool					Init();
	/// 전체 사용 메모리 제거
	void					Release();

	/// 스킬 정보를 알려줌
	sPlayerSkillBaseInfo*	GetPlayerSkillInfo( unsigned long skillNumber );
	sMonsterSkillScript*	GetMonsterSkillInfo( unsigned long monsterClassIdx, eMONSTERATTACK_TYPE attackType );

	cSkillHashMap*			GetMonsterSkillTotalMap();
	void*					GetMonsterBaseSkillArr( unsigned long monsterClassIdx );
	void*					GetMonsterEliteSkillArr( unsigned long monsterClassIdx );


	/// 효과 정보를 알려줌
	sInfluenceScript*		GetInfluenceInfo( unsigned long influenceClassIdx );

	bool					IsCastSkill( unsigned long skillIdx, unsigned long skillStep );

	tArray<unsigned long>*	GetFreeSkill( unsigned long jobIdx ) { return (tArray<unsigned long>*)mFreeSkillMap.GetAt(jobIdx); }

	tPointerHashMap<unsigned long, void*>* GetPlayerSkillInfoMap() { return &mPlayerSkillInfoMap; }

	tPointerHashMap<unsigned long, void*>* GetInfluenceInfoMap() { return &mInfluenceMap; }

	/// 스킬을 사용할수 있는 직업 배열을 얻어옴
	tArray<unsigned long>*	GetPlayerSkillJob( unsigned long jobIdx );

	/// 효과 로드
	bool					InfluenceLoad();

private:

	/// 스킬 정보 로드.
	bool					PlayerSkillScriptLoad();
	bool					MonsterSkillScriptLoad();

	/// 스킬 단계 로드.
	bool					PlayerSkillStepLoad();

	/// 스킬 습득/사용 직업코드 로드
	bool					PlayerSkillJobScriptLoad();

	void					PlayerFreeSkillList();


private:

	///플레이어스킬정보테이블 <스크립트 상의 스킬 Index, 스킬 정보>
	///typedef tPointerHashMap<unsigned long,sSkillScript*> cPlayerSkillInfoMap;
	cSkillHashMap			mPlayerSkillInfoMap;

	///몬스터스킬정보테이블 <스크립트 상의 스킬 Index, 스킬 정보>
	///typedef tPointerHashMap<unsigned long, tArray<sMonsterSkillScript*>> cMonsterSkillInfoMap;
//	cSkillHashMap			mMonsterSkillInfoMap;

	cSkillHashMap			mMonsterBaseSkillMap;
	cSkillHashMap			mMonsterSkillTotalMap;
	cSkillHashMap			mMonsterEliteSkillMap;


	/// 스킬 습득/사용 직업코드
	///typedef tPointerHashMap<unsigned long, tArray<unsigned short>*> cSkillJobMap;
	cSkillHashMap			mSkillJobMap;

	/// 플레이어 효과 정보
	///typedef tPointerHashMap<unsigned long, sInfluenceScript*> cPlayerInfluenceMap;
	cSkillHashMap			mInfluenceMap;


	cSkillHashMap			mFreeSkillMap;
};


#define SKILLSCRIPT cSkillScript::mpSkillScript
