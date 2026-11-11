/* ==========================================================================
* 파    일 : LevelScript.h
* 목    적 : 레벨에 관련된 테이블들 정보를 로드함
* 작 성 자 : 박경희
* 작 성 일 : 2007-05-15
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Level_Common.h"

class cLevelScript
{
public:

	static cLevelScript* mpLevelScript;

public:
	cLevelScript(void);
	~cLevelScript(void);

public:

	bool				Init();
	void				Release();

	/// 스크립트에서 경험치 테이블 정보를 읽어온다.
	bool				LoadExpTable();
	/// 해쉬맵에 위치한 경험치 테이블 정보를 읽어온다.
	sExpTable*			GetExpTable( unsigned char level );

	/// 스크립트에서 스킬경험치 정보를 읽어온다.
	bool				LoadSxpTable();
	/// 해쉬맵에 위치한 스킬경험치 정보를 읽어온다.
	sSxpTable*			GetSxpTable( unsigned char level );

	unsigned char		GetMaxLevel()	{ return mMaxLevel; }
	unsigned char		GetMaxSkill()	{ return mMaxSkill; }

private:

	typedef tPointerHashMap<unsigned long, void*> cHashMap;

	///	typedef tPointerHashMap<unsigned long, sExpTable*> cHashMap;
	cHashMap*			mpExpTableMap;
	///	typedef tPointerHashMap<unsigned long, sSxpTable*> cHashMap;
	cHashMap*			mpSxpTableMap;

	unsigned char		mMaxLevel;
	unsigned char		mMaxSkill;

};


#define LEVELSCRIPT cLevelScript::mpLevelScript