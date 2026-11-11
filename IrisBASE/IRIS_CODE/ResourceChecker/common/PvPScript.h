/* ==========================================================================
* 파    일 : PvP.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-11-14
* 주의사항 :
*===========================================================================*/
#pragma once
#include "PVP_Common.h"
#include "Lexer.h"
#include "Parser.h"

class cPvPScript
{
public:

	static cPvPScript*	 mpPvPScript;

public:

	/// 생성 소멸
	cPvPScript();
	virtual ~cPvPScript();

	///	초기화
	bool				Init();
	/// 전체 사용 메모리 제거
	void				Release();

	sDMInfo*			GetDmInfo( unsigned int type );
	tPointerArray<void*>*	GetStartTimeAry()	{ return &mPvPStarttimeAry;	}

protected:
	bool	LoadDMInfo();
	bool	LoadMemberGive();


	bool	LoadTime( unsigned short pvpType );

private:

	typedef tPointerHashMap<unsigned long, void*> cHashMap;
	typedef tPointerArray<void*> cPArray;

	/// <unsigned long, sDMInfo* pDMInfo>
	cHashMap			mDMInfoMap;

	/// <sPvPStartTime*>
	cPArray				mPvPStarttimeAry;

};


#define PVPSCRIPT cPvPScript::mpPvPScript
