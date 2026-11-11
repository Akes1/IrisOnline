#pragma once

class cTwitterScript
{
public:
	static cTwitterScript*		mpTwitterScript;

public:
	cTwitterScript();
	~cTwitterScript();

	bool Init();

	bool IsMonsterEvnet( unsigned long monsterIdx );	

protected:

	bool LoadTwitterEventList();
	
protected:

	typedef tHashSet<unsigned long> cMonsterSet;
	cMonsterSet				mEventMonsterSet;
};


#define TWITTERSCRIPT cTwitterScript::mpTwitterScript