/* ==========================================================================
* 파    일 : ThemeObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2009-05-25
* 주의사항 :
*===========================================================================*/
#pragma once

#include "RangeCheck.h"

struct sStageChangePos;

class cThemeObject
{
public:
	cThemeObject();
	virtual ~cThemeObject();

	/// Duelmanager 메모리풀처리
	void* operator new( size_t n ); 
	void operator delete( void* ptr, size_t n );

	virtual bool		InitTheme( unsigned short themeIdx, unsigned char mode );
	void				RelaseObject();
	virtual void		ProcessTheme( unsigned long elapsedTime, unsigned long accumTime );
	
	bool				ThemeAddPlayer( unsigned long playerIdx );
	void				ThemeOutPlayer( unsigned long playerIdx );
	void				ThemeAllOut();
		
	bool				IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos );
	virtual void		DeleteObstruction( unsigned long obstructionIdx );
	void				SendObstruction( cPlayer* pPlayer );

	void				AddTeleportPortal( unsigned long mapChangeIdx );
	void				SendTeleportPortal( cPlayer* pPlayer );
	int					UseTeleportPortal( cPlayer* pPlayer, unsigned long mapChangeIdx );

	void				Dependency()	{ mDependency = true; }

	bool				IsMemberMax()	{ return mMemberSet.GetSize() >= mMaxCount; }

	sStageChangePos*	GetThemeOutPos();
	unsigned short      GetThemeIndex() { return mThemeRoomIndex; }
	unsigned char		GetMode()	{ return mMode; }

	void				DeleteGathering( unsigned long objectIdx )	{ mGatheringSet.Erase( objectIdx ); }

	tPointerHashMap<unsigned long, void*>* GetRegenGroupMap()	{ return &mRegenGroupMap; }
	tHashMap<unsigned long, unsigned long>* GetRegenSearchMap()	{ return &mRegenSearch; }
	tPointerHashMap<unsigned long, void*>* GetFamilyMap()		{ return &mFamilyMap; }
	tPointerHashMap<unsigned long, void*>* GetGatheringGroupMap() { return &mGatherRegenGroupMap; }

	bool				MonsterCntTestObject( unsigned short mapNum, unsigned long monsterIdx, unsigned long classIdx, unsigned long regenIdx );

protected:
	typedef tPointerHashMap<unsigned long, void*> cPHashMap;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned long, unsigned long> cHashMap;
	typedef tPointerArray<void*> cPArray;
	typedef tArray<unsigned long> cAry;


	/// 초기화
	unsigned short		mThemeRoomIndex;		/// 방번호
	unsigned char		mMode;

	// 유저 인덱스 관리
	unsigned long		mMaxCount;			// 유저 관리 정보 최대 값
	cHashSet			mMemberSet;

	bool				mServerOnRegen;
	cRangeCheck			mRangeCheck;
	bool				mDependency;
	//////////////////////////////////////////////////////////////////////////
	/// 장애물
	/// <unsigned long, sDoorInfo* pDoorInfo>
	cPHashMap			mObstructionMap;

	//////////////////////////////////////////////////////////////////////////
	/// 텔레포트 위치이동 포탈
	cHashSet			mTeleportSet;

	//////////////////////////////////////////////////////////////////////////
	/// 몬스터 리젠 관리
	/// <unsigned long, sRegenGroup*>;
	cPHashMap			mRegenGroupMap;		/// 리젠그룹을 기준으로 관리해야 하는 몬스터 목록 전체 보유
	/// <unsigned long familyNum, cHashSet* regenIdx>
	cPHashMap			mFamilyMap;			/// 동족그룹
	cPHashMap			mRegenSwitchMap;
	cHashMap			mRegenSearch;			/// 리젠 인덱스로 몬스터 고유번호 찾기

	/////////////////////////////////////////////////////////////////////////
	/// 채집물 관리
	cPHashMap			mGatherRegenGroupMap;

	cHashSet			mGatheringSet;
	//////////////////////////////////////////////////////////////////////////
	// npc 리젠 정보 (npcindex)
	typedef tArray<unsigned long> cNpcArr;
	cNpcArr	mNpcArr;

	unsigned long		mMonsterCntTest;
};
