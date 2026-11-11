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
	
};

#define MAKESKILLSCRIPT cMakeSkillScript::mpMakeSkillScript
