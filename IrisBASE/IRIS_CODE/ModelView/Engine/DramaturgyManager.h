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

enum eAPPLYDRAMA_TYPE
{
	eAPPLYDRAMA_DAMAGE = 0,
	eAPPLYDRAMA_CRITICAL,
	eAPPLYDRAMA_DIE1,
	eAPPLYDRAMA_DIE2,

	eAPPLYDRAMA_MAX,
};

/// 기본공격	- 종족*성별*무기 (3 * 2 * 7 = 42)
/// 스킬		- 종족*성별 (3 * 2 = 6)
class cDramaturgyManager
{
public:
	static cDramaturgyManager*		mpDramaturgyManager;

	cDramaturgyManager();
	~cDramaturgyManager();

	bool Init();

	/// 연출에 포함된 3D정보를 미리 로딩하는데 사용한다.
	void				DirectLoadPlayerDramaObj( unsigned long skillIdx );
	void				DirectLoadMonsterDramaObj( unsigned long monsterIdx );
	void				DirectLoadPlayerApplyDramaObj( unsigned char race, unsigned char gender );
	void				DirectLoadMonsterApplyDramaObj( unsigned long monsterIdx );
	void				DirectLoadInfluenceDramaObj( unsigned long influenceIdx );

	/// 연출정보 획득 (연출객체는 Thread Loading)
	cDramaturgyInfo*	GetPlayerDramaturgyInfo( unsigned long skillIdx, unsigned short weaponstate, unsigned char race, unsigned char gender );
	cDramaturgyInfo*	GetPlayerDramaturgyInfo( unsigned long skillIdx, unsigned short dramaKey );
	cDramaturgyInfo*	GetMonsterDramaturgyInfo( unsigned long monsterIdx, eMONSTERATTACK_TYPE type );

	cDramaturgyInfo*	GetPlayerApplyDramaInfo( unsigned char race, unsigned char gender, eAPPLYDRAMA_TYPE type );
	cDramaturgyInfo*	GetMonsterApplyDramaInfo( unsigned long monsterIdx, eAPPLYDRAMA_TYPE type );

	///  
	cDramaturgyInfo*	GetInfluenceDramaturgyInfo( unsigned long influenceIdx );

	void ReLoadDramaturgyFileList();

	/// 효과 연출은 외부호출
	bool LoadInfluenceDrama( unsigned long influenceIdx, const cString& fileName, bool loadObject = false );

protected:
	/// "PlayerDramaFileList.txt" load
	bool LoadPlayerDramaturgyFileList();

	/// "MonsterDramaFileList.txt"
	bool LoadMonsterDramaturgyFileList();

	/// "ApplyDramaList.txt" load
	bool LoadApplyDramaturgyFileList();

protected:
	/// 
	typedef tPointerHashMap<unsigned long, void*>	cSkillDramaMap;
	/// key = ((race*gender_max)+gender)*weaponstate_max + weaponstate;
	typedef tPointerHashMap<unsigned long, void*> cDramaturgyInfoMap;

	/// 연출파일 메모리
	/// key = skillIdx, value = cDramatugyInfoMap pointer;
	cSkillDramaMap	mPlayerDramaturgyInfoMap;

	/// key = monsterIdx, value = cDramatugyInfo pointer;
	cSkillDramaMap	mMonsterDramaturgyInfoMap;

	/// 효과 연출 파일
	cSkillDramaMap	mInfluenceDramaturgyInfoMap;

	/// key = monster Index (예외 - 1001이상은 캐릭터 표현)
	cSkillDramaMap	mApplyDramaMap;
};

#define DRAMATURGYMAN cDramaturgyManager::mpDramaturgyManager