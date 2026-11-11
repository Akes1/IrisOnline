/* ==========================================================================
* 파    일 : ShortcutScript.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-02-10
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Player_Common.h"

const unsigned int MAX_JOBKIND = 6;

class cShortCutScript
{
public:
	static cShortCutScript* mpSingleton;
	static cShortCutScript* GetSingleton();

public:
	cShortCutScript();
	~cShortCutScript();

	bool Init();

	sShortcut* GetShortCut( unsigned long firstjob );

private:
	bool LoadShortCutScript( const cString& pathName );

private:
	sShortcut	mShortCut[MAX_JOBKIND][MAX_SHORTCUT_SIZE];
};

inline cShortCutScript* cShortCutScript::GetSingleton()
{
	return mpSingleton;
}

#define SHORTCUTSCRIPT cShortCutScript::GetSingleton()