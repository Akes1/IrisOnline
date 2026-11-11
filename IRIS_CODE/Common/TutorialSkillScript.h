/* ==========================================================================
* 파    일 : TutorialSkillScript.h
* 목    적 : 
* 작 성 자 : 
* 작 성 일 : 2009-10-29
* 주의사항 :
*===========================================================================*/
#pragma once

struct sTutorialSkill
{
	unsigned long	skillIndex;
	unsigned char	skillStep;

	sTutorialSkill()
	{
		skillIndex = 0;
		skillStep = 0;
	}
};

// 튜토리얼에서 제공하는 스킬 리스트
class cTutorialSkillScript
{
public:
	static cTutorialSkillScript* mpSingleton;

public:
	cTutorialSkillScript();
	~cTutorialSkillScript();

	bool Init();
	void Release();

	inline tPointerArray<void*>* GetTutorialSkillArr() { return &mTutorialSkillArr; }

public:
	static cTutorialSkillScript* GetSingleton();

private:
	bool LoadScript( const cString& pathName );

private:
	// 튜토리얼 스킬 저장
	typedef tPointerArray<void*> cTutorialSkillArr;
	cTutorialSkillArr mTutorialSkillArr;
};

inline cTutorialSkillScript* cTutorialSkillScript::GetSingleton()
{
	return mpSingleton;
}

#define TUTORIALSKILLSCRIPT cTutorialSkillScript::GetSingleton()
