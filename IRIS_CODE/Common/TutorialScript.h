/* ==========================================================================
* 파    일 : TutorialScript.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-10-13
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Lexer.h"

// 모드 구조체
class cParser;
class cTutorialMode
{
public:
	cTutorialMode();
	~cTutorialMode() {}

public:
	// 모드 인덱스
	unsigned long	mModeIndex;	

	// 시작좌표
	unsigned long	mStartPosIndex;

	// 부활관련 안내문 인덱스
	unsigned long	mDieTextIndex;
};

inline cTutorialMode::cTutorialMode()
: mModeIndex(UINT_MAX)
, mStartPosIndex(0)
, mDieTextIndex(0)
{
}

//
class cTutorialScript
{
public:
	static cTutorialScript* mpSingleton;

public:
	cTutorialScript();
	~cTutorialScript();

	bool Init();
	void Release();

	cTutorialMode* GetTutorialMode( unsigned long modeIdx );

public:
	static cTutorialScript* GetSingleton();

private:
	bool LoadScript( const cString& pathName );
	bool LoadImages( const cString& path, cParser& parser );
	bool LoadMode( cParser& parser, cTutorialMode* mode );

private:
	// 튜토리얼 모드 관리 (cTutorialMode*)
	typedef tPointerHashMap<unsigned long, void*> cModeMap;
	cModeMap	mModeMap;
};

inline cTutorialMode* cTutorialScript::GetTutorialMode( unsigned long modeIdx )
{
	return (cTutorialMode*)mModeMap.GetAt( modeIdx );
}

inline cTutorialScript* cTutorialScript::GetSingleton()
{
	return mpSingleton;
}

#define TUTORIALSCRIPT cTutorialScript::GetSingleton()

