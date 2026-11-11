/* ==========================================================================
* 파    일 : TotemScript.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-10-23
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Totem_Common.h"

class cTotemScript
{
public:

	static cTotemScript*		mpTotemScript;

public:

	/// 생성 소멸
	cTotemScript(void);
	~cTotemScript(void) {}

	bool						Init();
	void						Release();

	/// 스크립트 로드
	bool						LoadTotemList();
	/// 정보 조회
	sTotemScript*				GetTotemInfo( unsigned long totemIdx );

private:

	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;

	/// 몬스터 리스트 해쉬맵
	//	typedef tPointerHashMap<unsigned long, sTotemScript*> cMonsterMap;
	cPointHashMap*				mpTotemMap;

};


#define TOTEMSCRIPT cTotemScript::mpTotemScript
