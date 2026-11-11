/* ==========================================================================
* 파    일 : SkillScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-06-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "./common/Skill_Common.h"
#include "Lexer.h"
#include "Parser.h"

class cSkillTree
{
public:
	cSkillTree()
	{
		mIconCount = 0;
		mIconList = 0;
	}
	~cSkillTree()
	{
		SAFE_DELETE_ARRAY(mIconList);
	}

	struct sTreeIcon
	{
		unsigned int	skillIdx;
		unsigned int	uiPosX;
		unsigned int	uiPosY;
	};

	cString			mImageName;

	unsigned int	mIconCount;

	sTreeIcon*		mIconList;
};

class cSkillTreeLexer : public cLexer
{
public:
	cSkillTreeLexer( const char* buffer, unsigned int size );

	enum
	{
		/// 글자색
		eTOKEN_SKILLTREE = 100,
		eTOKEN_IMAGEFILE,
		eTOKEN_JOBIDX,
		eTOKEN_SKILLPOS,
	};
};

typedef tPointerHashMap<unsigned long, void*> cSkillHashMap;

class cSkillScript
{
public:

	static cSkillScript*	 mpSkillScript;

public:

	/// 생성 소멸
	cSkillScript(void);
	~cSkillScript(void){}

	///	초기화
	bool					Init();
	/// 전체 사용 메모리 제거
	void					Release();

	/// 스킬 정보를 알려줌
	sPlayerSkillBaseInfo*	GetPlayerSkillInfo( unsigned long skillNumber );
	sMonsterSkillScript*	GetMonsterSkillInfo( unsigned long monsterClassIdx, eMONSTERATTACK_TYPE attackType );

	cSkillHashMap*			GetMonsterSkillTotalMap();
	void*					GetMonsterBaseSkillArr( unsigned long monsterClassIdx );

	/// 효과 정보를 알려줌
	sInfluenceScript*		GetInfluenceInfo( unsigned long influenceClassIdx );

	/// 발동 효과 정보
	sActInfluenceScript*	GetActInfluenceInfo( unsigned long actIdx );

	bool					IsCastSkill( unsigned long skillIdx, unsigned char skillStep );

	/// 직업index와 플레이어 직업으로 사용 가능한지 체크
	bool					IsPlayerSkillJob( unsigned long skillJobIdx, unsigned long playerJob );

	tArray<unsigned long>*	GetFreeSkill( unsigned long jobstep ) { return (tArray<unsigned long>*)mFreeSkillMap.GetAt(jobstep); }

	tPointerHashMap<unsigned long, void*>* GetPlayerSkillInfoMap() { return &mPlayerSkillInfoMap; }

	/// 스킬을 사용할수 있는 직업 배열을 얻어옴
	tArray<unsigned long>*	GetPlayerSkillJob( unsigned long jobIdx );

	/// 효과 로드
	bool					InfluenceLoad();

	bool					ActInfluenctLoad();

	cSkillTree*				GetSkillTree( unsigned long jobIdx );

private:

	/// 스킬 정보 로드.
	bool					PlayerSkillScriptLoad();
	bool					MonsterSkillScriptLoad();

	/// 스킬 단계 로드.
	bool					PlayerSkillStepLoad();

	/// 스킬 습득/사용 직업코드 로드
	bool					PlayerSkillJobScriptLoad();

	void					PlayerFreeSkillList();

	/// 스킬 습득 npc 정보
	bool					LoadSkillTree( const cString& pathName );
	bool					LoadTree( cParser& parser, cSkillTree* pTree );

private:

	typedef tArray<void*>	cArray;

	///플레이어스킬정보테이블 <스크립트 상의 스킬 Index, 스킬 정보>
	///typedef tPointerHashMap<unsigned long,sSkillScript*> cPlayerSkillInfoMap;
	cSkillHashMap			mPlayerSkillInfoMap;

	///몬스터스킬정보테이블 <스크립트 상의 스킬 Index, 스킬 정보>
	///typedef tPointerHashMap<unsigned long, tArray<sMonsterSkillScript*>> cMonsterSkillInfoMap;
//	cSkillHashMap			mMonsterSkillInfoMap;

	cSkillHashMap			mMonsterBaseSkillMap;
	cSkillHashMap			mMonsterSkillTotalMap;


	/// 스킬 습득/사용 직업코드
	///typedef tPointerHashMap<unsigned long, tArray<unsigned short>*> cSkillJobMap;
	cSkillHashMap			mSkillJobMap;

	/// 플레이어 효과 정보
	///typedef tPointerHashMap<unsigned long, sInfluenceScript*> cPlayerInfluenceMap;
	cSkillHashMap			mInfluenceMap;

	///typedef tPointerHashMap<unsigned long, sActInfluenceScript*> cPlayerActInfluenceMap;
	cSkillHashMap			mActInfluenceMap;

	cSkillHashMap			mFreeSkillMap;

	typedef	tPointerHashMap<unsigned long, void*> cSkillTreeMap;
	cSkillTreeMap		mSkillTreeMap;

};

inline cSkillTree*	cSkillScript::GetSkillTree( unsigned long jobIdx )
{
	return (cSkillTree*)mSkillTreeMap.GetAt( jobIdx );
}

#define SKILLSCRIPT cSkillScript::mpSkillScript
