/* ==========================================================================
* 파    일 : AIManager.h
* 목    적 : ai관리자
* 작 성 자 : 박경희
* 작 성 일 : 2007-11-13
* 주의사항 : 
*===========================================================================*/

#pragma once

#include "BaseObject_Common.h"
#include "Monster_Common.h"
#include "Gathering_Common.h"
#include "IndexGenerator.h"

const int AI_REGEN_PROCESS_DELAY = 100;

class cNaviField;
class cPathFinder;
class cMonster;
class cGathering;
class cNaviField;
class cPathFinder;

struct sRegenGroupScript;

enum eACTION_ID;
enum eACTION_CHANGE;
enum ePathfindresult;
enum eACTION_GAT_ID;
enum eACTION_GAT_CHANGE;

struct sRegenMonster
{
	unsigned long		mObjectIdx;
	unsigned long		mRegenIdx;
	unsigned long		mRegenTime;
	bool				mDie;
};

struct sRegenGroup
{
	/// 스크립트 참고용 포인터
	sRegenGroupScript*	mpRegenGroup;

	/// 리젠여부를 체크하는항목 - 조건에 해당하는 몬스터수가 되면 이플레그로 리젠여부를 판단
	bool				mUse;
	/// 그룹버놓
	unsigned long		mGroupIdx;
	/// 리젠 퍼센테지를 만족 할 경우 리젠 카운트를 시작하라는 플래그
	bool				mIsRegen;		
	/// 리젠 수행 시간
	unsigned long		mRegenTime;		
	/// 그룹에 해당하는 몬스터 살아 있는 수
	unsigned long		mMonsterAliveCnt;	

	tArray<void*>		mMonsterList;
};

struct sSwichGroup
{
	unsigned long		mSwitchGroup;
	/// 리젠 선택된 그룹인덱스
	unsigned long		mGroupIdx;
	/// 다음 리젠 시간
	unsigned long		mRegenTime;
	/// 리젠 하지 않음
	bool				mRegenStop;
	/// 리젠 동기화 대기중
	bool				mRegenSyncWait;
	/// 체크 시간
	unsigned long		mCheckTime;

	tArray<unsigned long>*	mpGroupIdxAry;
};

struct sRegenGather
{
	unsigned long		mRegenIdx;
	/// 리젠 수행 시간
	unsigned long		mRegenTime;		
	/// 리젠 준비 상태
	bool				mRegenReady;
};

struct sMapPathInfo
{
	cNaviField*			mNaviField;			/// 네비필드
	cPathFinder*		mPathFinder;		/// 길찾기
	float				mMapSize;
};

class cAIManager
{
public:

	static cAIManager*	mpAIManager;

public:

	/// 생성소멸
	cAIManager();
	~cAIManager();

	/// 자원관리
	bool				Init( int serverType );
	bool				LoadMapPathData( unsigned short mapNum, unsigned char modeNum );
	bool				InitMonsterRegen( unsigned char levelMode, tPointerHashMap<unsigned long, void*>* pRegenGroupMap, 
		tPointerHashMap<unsigned long, void*>* pFamilyMap,
		tPointerHashMap<unsigned long, void*>* pRegenSwitchMap );
	void				Release();
	void				Process( unsigned long elapsedTime, unsigned long accumTime );

	//////////////////////////////////////////////////////////////////////////
	/// 몬스터
	
	/// 몬스터 리젠 프로세스
	void				ProcessMonRegen( unsigned char levelMode, tPointerHashMap<unsigned long, void*>* pRegenGroupMap, tPointerHashMap<unsigned long, void*>* pRegenSwitchMap,
											tPointerHashMap<unsigned long, void*>* pFamilyMap, tHashMap<unsigned long, unsigned long>* pRegenSearch,
											unsigned long /*elapsedTime*/, unsigned long accumTime, unsigned short mapNumber, bool* pServerOnRegen );
	void				ProcessMonPatternMove( tPointerHashMap<unsigned long, void*>* pFamilyMap, tHashMap<unsigned long, unsigned long>* pRegenSearch,
											   unsigned long elapsedTime, unsigned long accumTime );

	void				BossSummonMonRegen( cMonster* pBoss, unsigned short mapNumber, float x , float y, unsigned long genRangeMin, unsigned long genRangeMax, 
											unsigned long genCnt, unsigned long monsterClassIdx, bool isRand = true );
	void				CheatSummonMonRegen( unsigned short mapNumber, float x , float y, unsigned long monsterClassIdx, unsigned long genCnt );
	unsigned long		QuestSummonMonRegen( unsigned short mapNumber, float x , float y, unsigned long monsterClassIdx, unsigned long destroyTime );

	cMonster*			StoneSummonMonRegen( unsigned short mapNumber, float x , float y, unsigned long monsterClassIdx, unsigned long teamType );

	bool				AutoDetectSummonMonRegen( unsigned short mapNumber, float x , float y );

	/// 죽어서 리젠 대기로 넘어가는 몬스터 등록
	void				InsertReadyMonster( unsigned long monsterIdx, unsigned long regenIdx, unsigned long groupIdx );

	void				SwitchRegen( unsigned short levelMode, unsigned short chennelNum, unsigned long switchGroupIdx );

	//////////////////////////////////////////////////////////////////////////
	/// 채집

	/// 채집 리젠 프로세스
	void				ProcessGatRegen( unsigned char levelMode, unsigned long elapsedTime, unsigned long accumTime );
	void				ProcessGetRegen( unsigned long accumTime, unsigned char levelMode, unsigned short mapNumber, 
										 tPointerHashMap<unsigned long, void*>* pRegenGroupMap, tHashSet<unsigned long>* pGatheringSet );

	/// 죽어서 리젠 대기로 넘어가는 채집 등록
	void				InsertReadyGathering( unsigned long gatheringIdx, unsigned long mapNumber, unsigned long regenIdx );

	//////////////////////////////////////////////////////////////////////////
	/// 길찾기

	/// 맵 사이즈
	float				GetPathMaxSideSize( unsigned int mapNum );
	/// 현재 위치 높이값
	bool				CalcHeight( unsigned short mapNumber, float* height, float x, float y ) const;
	/// 이동 가능지역 체크
	bool				IsPossible( unsigned short mapNumber, float x, float y, sObject object ) const;
	bool				IsPossible( unsigned short mapNumber, NiPoint2 start, NiPoint2 goal, sObject object ) const;

	/// 이동 경로 겟수 산출
	unsigned long		CalcPathCnt( unsigned short mapNumber, NiPoint2 start, NiPoint2 goal );

	bool				SetPathFinder( cMonster* pMonster, unsigned short mapNum, unsigned char levelMode );

	//////////////////////////////////////////////////////////////////////////
	/// 그룹 행동 (동족)
	void				FamilyAttack( unsigned long mainMonIdx, unsigned long groupIdx, sObject attacker );
	bool				FamilyNextPattern( unsigned short mapNum, unsigned long groupIdx, long* pPatternPos, char* pPatternDir );

	bool				FamilyFar( unsigned short mapNum, unsigned long familyNum, NiPoint2 pos );

private:
	void				RegenMonster( unsigned char levelMode, tPointerHashMap<unsigned long, void*>* pRegenGroupMap, tPointerHashMap<unsigned long, void*>* pFamilyMap,
		tHashMap<unsigned long, unsigned long>* pRegenSearch, unsigned long groupIdx, sRegenMonster* pRM, unsigned short mapNumber );

private:

	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned long,unsigned long> cHashMap;
	typedef tArray<unsigned long> cArray;
	typedef tArray<void*> cVoidAry;

	/// 리젠그룹을 기준으로 관리해야 하는 몬스터 목록 전체 보유
	/// typedef tPointerHashMap<unsigned long, sRegenGroup*> cPointerHashMap;
	cPointHashMap*		mpRegenGroupMap;

	/// 동족그룹
	/// <unsigned long familyNum, cHashSet* regenIdx>
	cPointHashMap*		mpFamilyMap;

	cPointHashMap*		mpRegenSwitchMap;


	/// typedef tPointerHashMap<unsigned long, sRegenGatherGroup*> cPointerHashMap;
    cPointHashMap*		mpRegenGatherGroupMap;	


	/// 리젠 인덱스로 몬스터 고유번호 찾기
	cHashMap			mRegenSearch;

	/// 인덱스 생성기
	cIndexGenerator		mMonsterIdxGen;
	/// cIndexGenerator		mGatheringIdxGen;

	/// 서버부팅후 바로 리젠
	bool				mServerOnRegen;

	/// 리젠 프로세스 부하 방지
	unsigned long		mNextCheckTime;

	/// 채집
	cArray				mReadyGatheringAry;

	/// 일반채널 / 인던 구분
	int					mServerType;

	/// 길찾기 관련 맵정보
	/// <unsigned long mapFolderNum, sMapPathInfo*>
	cPointHashMap		mMapPathMap;
	NiPoint2			mTempPathAry[MAX_PATH_COUNT];
};

#define AIMANAGER	cAIManager::mpAIManager
