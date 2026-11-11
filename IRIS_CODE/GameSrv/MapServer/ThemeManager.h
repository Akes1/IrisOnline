/* ==========================================================================
* 파    일 : ThemeManager.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-09-04
* 주의사항 :
*===========================================================================*/
#pragma once
#include "ThemeObject.h"
#include "rangecheck.h"
#include "indexgenerator.h"
#include "themeuserpool.h"
#include "theme_common.h"
#include "ThemeTimeScript.h"

enum eTHEME_PROCESS
{
	eTHEMEPROCESS_READY,
	eTHEMEPROCESS_WAITDB,
	eTHEMEPROCESS_RUN,

	eTHEMEPROCESS_5MIN, /// 5분 추가 대기
};

const unsigned long THEME_OPEN_SEND_TIME = 1 * MINUTE;
const unsigned long THEME_READY_TIME = 5 * MINUTE;

class cThemeManager
{
public:

	static cThemeManager* mpThemeManager;

public:

	cThemeManager();
	~cThemeManager();

	/// pool 처리
	cThemeObject*		AllocThemeObject() { return mThemeObjectPool.Alloc(); }
	void				FreeThemeObject(cThemeObject* ptr) { mThemeObjectPool.Free(ptr); }

	bool				Init( SERVER_TYPE type );
	void				Release();
	void				Process( unsigned long elapsedTime, unsigned long accumTime );

	/// 인던 입장 관리자(npc)를 통한 입장요청
	void				ThemeRequest( unsigned long playerIdx, unsigned short themeMapNum );
	void				ThemeReadyAccept( unsigned long playerIdx, unsigned short themeMapNum, unsigned char themeMode );
	void				ThemeDBErr( unsigned long playerIdx, unsigned char errorCode );

	/// 인던 입장
	cThemeObject*		ThemeJoin( cPlayer* pPlayer, unsigned short themeIdx, unsigned char levelMode, float *pX, float *pY );
	cThemeObject*		TutorialJoin( cPlayer* pPlayer, unsigned char levelMode, unsigned long tutorialMode, float *pX, float *pY, bool gmJoin );

	cThemeObject*		GetThemeObject( unsigned long themeIdx );

	void				ThemeRestart();
	void				SetThemeRestartTime( unsigned long resetLeftTime );

	void				InsertDelTheme( unsigned long themeIdx )	{ mDelAry.PushBack( themeIdx ); }

	tPointerHashMap<unsigned long, void*>* GetThemeObjectMap()	{ return &mThemeMap; }

	unsigned long		NextResetTime()	{ return mNextResetTime; }
	unsigned long		GetProcess()	{ return mProcess; }

	unsigned long		GetNpcGenIdx()	{ return mNpcIdxGen.GeneratIdx(); }
	void				DelNpcGenIdx( unsigned long objectIdx )	{ mNpcIdxGen.DelIdx( objectIdx ); }

	unsigned long		GetStartPosIdx()	{ return mStartPosIdx; }

	bool				MonsterCntTest( unsigned short mapNum, unsigned long monsterIdx, unsigned long classIdx, unsigned long regenIdx );
private:

	/// 파니예 서커스 인던 메모리풀
	typedef tPool<cThemeObject> cThemeObjectPool;
	cThemeObjectPool	mThemeObjectPool;

	typedef tHashMap<unsigned long, unsigned long> cHashMap;
	typedef tPointerHashMap<unsigned long, void*> cPHashMap;
	typedef tArray<unsigned long> cArray;

	SERVER_TYPE			mType;

	/// 생성된 서커스 인던맵
	///<unsigned long themeIdx, cThemeObject*>
	cPHashMap			mThemeMap;

	/// 플레이어별 서커스인던 입장 방번호
	///<unsigned long playerIdx, unsigned long themeIdx>
	cHashMap			mThemePlayerMap;

	unsigned long		mThemeOpenSendTime;

	cIndexGenerator		mIdxGen;

	unsigned long		mNextEventTime;
	unsigned long		mNextResetTime;
	eTHEME_PROCESS		mProcess;

	unsigned long		mReadyPlusTime;

	cThemeTimeScript	mThemeTimeScript;

	unsigned long		mStartPosIdx;

	cArray				mDelAry;

	cIndexGenerator		mNpcIdxGen;
};

#define THEMEMANAGER	cThemeManager::mpThemeManager
