///* ==========================================================================
//* 파    일 : CircusObject.h
//* 목    적 : 
//* 작 성 자 : 
//* 작 성 일 : 2009-05-25
//* 주의사항 :
//*===========================================================================*/
#pragma once
#include "ThemeObject.h"

// 문 객체 삭제후 리젠 타임
const unsigned long	 DOOR_REGENTIME	= 30000;

// 튜토리얼 객체
class cTutorialObject : public cThemeObject
{
public:
	cTutorialObject();
	virtual ~cTutorialObject();

	void* operator new( size_t n ); 
	void operator delete( void* ptr, size_t n );

	virtual bool InitTheme( unsigned short themeIdx, unsigned char mode );
	virtual void ProcessTheme( unsigned long elapsedTime, unsigned long accumTime );

	// 장애물제거
	virtual void DeleteObstruction( unsigned long obstructionIdx );

protected:
	//
	void RegenDoor();

private:
	// npc 리젠 정보 (npcindex)
	typedef tArray<unsigned long> cNpcArr;
	cNpcArr	mNpcArr;

	// 장애물 마지막 갱신시간
	unsigned long mLastAccumTime;
};







//
//#include "RangeCheck.h"
//
//const int CIRCUS_MEMBER_COUNT_MAX = 6;
//
//struct sStageChangePos;
//
//class cThemeObject
//{
//public:
//	cThemeObject();
//	virtual ~cThemeObject();
//
//	/// Duelmanager 메모리풀처리
//	void* operator new( size_t n ); 
//	void operator delete( void* ptr, size_t n );
//
//	bool				InitTheme( unsigned short circusIdx, unsigned char mode );
//	void				ProcessTheme( unsigned long elapsedTime, unsigned long accumTime );
//
//	unsigned short      GetIdx() { return mThemeIdx; }
//	unsigned char		GetMode()	{ return mMode; }
//
//	bool				ThemeAddPlayer( unsigned long playerIdx );
//
//	bool				IsMovePossible( NiPoint2 startPos, NiPoint2 goalPos );
//	void				DeleteObstruction( unsigned long obstructionIdx );
//
//	void				SendObstruction( cPlayer* pPlayer );
//
//	void				AddTeleportPortal( unsigned long mapChangeIdx );
//	void				SendTeleportPortal( cPlayer* pPlayer );
//	int					UseTeleportPortal( cPlayer* pPlayer, unsigned long mapChangeIdx );
//
//	void				ThemeMemberOut( unsigned long playerIdx );
//	sStageChangePos*	ThemeOutPos();
//	void				ThemeAllOut();
//
//	void				Dependency()	{ mDependency = true; }
//
//	tPointerHashMap<unsigned long, void*>* GetRegenGroupMap()	{ return mpRegenGroupMap; }
//	tHashMap<unsigned long, unsigned long>* GetRegenSearchMap()	{ return &mRegenSearch; }
//	tPointerHashMap<unsigned long, void*>* GetFamilyMap()		{ return mpFamilyMap; }
//private:
//
//	typedef tPointerHashMap<unsigned long, void*> cPHashMap;
//	typedef tHashSet<unsigned long> cHashSet;
//	typedef tHashMap<unsigned long, unsigned long> cHashMap;
//	typedef tPointerArray<void*> cPArray;
//	typedef tArray<unsigned long> cAry;
//
//	/// 초기화
//	unsigned short		mThemeIdx;		/// 방번호
//	unsigned char		mMode;
//
//	unsigned long		mMemberCnt;
//	unsigned long		mMember[CIRCUS_MEMBER_COUNT_MAX];
//	
//	bool				mDependency;
//	//////////////////////////////////////////////////////////////////////////
//	/// 장애물
//	/// <unsigned long, sDoorInfo* pDoorInfo>
//	cPHashMap			mObstructionMap;
//
//	//////////////////////////////////////////////////////////////////////////
//	/// 텔레포트 위치이동 포탈
//	cHashSet			mTeleportSet;
//
//	//////////////////////////////////////////////////////////////////////////
//	/// 몬스터 리젠 관리
//	/// <unsigned long, sRegenGroup*>;
//	cPHashMap*			mpRegenGroupMap;		/// 리젠그룹을 기준으로 관리해야 하는 몬스터 목록 전체 보유
//	/// <unsigned long familyNum, cHashSet* regenIdx>
//	cPHashMap*			mpFamilyMap;			/// 동족그룹
//	cPHashMap*			mpRegenSwitchMap;
//	cHashMap			mRegenSearch;			/// 리젠 인덱스로 몬스터 고유번호 찾기
//
//	bool				mServerOnRegen;
//
//	cRangeCheck			mRangeCheck;
//
//};
