/* ==========================================================================
* 파    일 : MakeSkillScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2009-01-05
* 주의사항 :
*===========================================================================*/
#pragma once

#include "MakeSkill_Common.h"

class cMakeSkillScript
{
public:

	static cMakeSkillScript*	 mpMakeSkillScript;

public:

	/// 생성 소멸
	cMakeSkillScript(void);
	~cMakeSkillScript(void);

	///	초기화
	bool					Init();

	sMakeSkillScript*		GetMakeSkill( unsigned long recipeIdx );
	sGroupInfo*			    GetGroupInfo( unsigned long groupIdx );

#ifdef _CLIENT
	tHashSet<unsigned long>* GetMixedTypeSet( unsigned long itemIdx ) { return (tHashSet<unsigned long>*)mItemMixedType.GetAt(itemIdx); }
#endif

private:

	bool					MakeSkillScriptLoad();

private:

	typedef tPointerHashMap<unsigned long, void*> cPHashMap;

	typedef tArray<void*> cPAry;
	typedef tArray<unsigned long> cLongAry;

	// sMakeSkillScript*
	cPHashMap				mMakeSkillMap;

	// sGroupInfo*
	cPHashMap				mGroupMap;

#ifdef _CLIENT
	/// 재료 아이템이 사용되는 조합 정보를 찍기 위해
	typedef tPointerHashMap<unsigned long, void*> cItemMixedTypeMap;
	typedef tHashSet<unsigned long> cMixedTypeSet;
	cItemMixedTypeMap mItemMixedType;
#endif
};

#define MAKESKILLSCRIPT cMakeSkillScript::mpMakeSkillScript
