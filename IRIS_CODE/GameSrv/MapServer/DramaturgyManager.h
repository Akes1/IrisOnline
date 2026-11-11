/* =====================================================================
* 파    일 :	DramaturgyManager.h , DramaturgyManager.cpp
* 목    적 :	연출정보관리 , 연출파일 파싱
* 작 성 자 :	정원주
* 작 성 일 :    2006년 10월 16일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 :	.
* =======================================================================*/
#pragma once

#include "DramaturgyInfo.h"

enum eMONSTERATTACK_TYPE;

/// 기본공격	- 종족*성별*무기 (3 * 2 * 7 = 42)
/// 스킬		- 종족*성별 (3 * 2 = 6)
class cDramaturgyManager
{
public:
	static cDramaturgyManager*		mpDramaturgyManager;

	cDramaturgyManager();
	~cDramaturgyManager();

	bool				Init();

	/// 연출정보 획득 (연출객체는 Thread Loading)
	cDramaturgyInfo*	GetPlayerDramaturgyInfo( unsigned long skillIdx, unsigned short weaponstate, unsigned char race, unsigned char gender, unsigned long* dramakey );
	cDramaturgyInfo*	GetMonsterDramaturgyInfo( unsigned long monsterIdx, eMONSTERATTACK_TYPE type );

protected:
	/// "PlayerDramaFileList.txt" load
	bool LoadPlayerDramatugyFileList();

	/// "MonsterDramaFileList.txt" load
	bool LoadMonsterDramaturgyFileList();

protected:
	/// 연출 파일 이름 저장
	typedef tPointerHashMap<unsigned long, void*>	cSkillDramaMap;
	/// key = ((race*gender_max)+gender)*weaponstate_max + weaponstate;
	typedef tPointerHashMap<unsigned long, void*> cDramaturgyInfoMap;

	/// 연출파일 메모리
	/// key = skillIdx, value = cDramatugyInfo pointer;
	cSkillDramaMap		mPlayerDramaturgyInfoMap;
	/// key = skillIdx, value = cDramatugyInfo pointer;
	cSkillDramaMap		mMonsterDramaturgyInfoMap;
};

#define DRAMATURGYMAN cDramaturgyManager::mpDramaturgyManager