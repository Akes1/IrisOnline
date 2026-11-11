/* ==========================================================================
* 파    일 : DuelObject.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-04-23
* 주의사항 :
*===========================================================================*/
#pragma once

#include "RangeCheck.h"

enum eDUEL_PROCESS
{
	eDUELPROCESS_FIGHT_COUNT = 0,
	eDUELPROCESS_FIGHT,
	eDUELPROCESS_FIGHT_END,
	eDUELPROCESS_END,
};

class cDuelObject
{
public:
	cDuelObject();
	virtual ~cDuelObject();

	/// Duelmanager의 메모리풀에서 객체 공간을 할당받음
	void* operator new( size_t n ); 
	/// 메모리풀에서 할당 받은 객체를 제거해줌
	void operator delete( void* ptr, size_t n );

	bool			Init( unsigned long duelIdx, unsigned long attackerIdx, unsigned long targetIdx );
	void			Process( unsigned long elapsedTime, unsigned long accumTime );

	unsigned long	GetAttackerIdx()	{ return mAttackerIdx; }
	unsigned long	GetTargetIdx()		{ return mTargetIdx; }

	NiPoint2		GetCenterPos()	{ return mCenterPos; }
	unsigned long	GetDuelProcess()	{ return mDuelProcess; }

	void			LastAttack( unsigned long dieIdx );

	void			FightEnd( unsigned long dieIdx );


private:
	void			FightStart();
	void			FightProcess( unsigned long elapsedTime, unsigned long accumTime );

private:

	unsigned long	mDuelIdx;			/// 고유 번호
	unsigned long	mAttackerIdx;		/// 결투자
	wchar_t			mAttackerName[MAX_NAME_BUFFER_SIZE];	/// 이름
	unsigned long	mTargetIdx;			
	wchar_t			mTargetName[MAX_NAME_BUFFER_SIZE];

	eDUEL_PROCESS	mDuelProcess;		/// 프로세스 진행상태
	unsigned long	mProcessWaitTime;	/// 프로세스 대기 시간

	NiPoint2		mCenterPos;			/// 결투 중심점

	unsigned long	mDelayTime;			/// 프로세스 부하 방지
	cRangeCheck		mRangeCheck;		/// 거리 체크

	bool			mAttacerRangeIn;	/// 플레이어 결투 범위 in/out 유무
	bool			mTargetRangeIn;
	unsigned long	mAttackerOutTime;	/// 플레이어 거리 밖으로 나간 시간 기록
	unsigned long	mTargetOutTime;

	unsigned long	mDieIdx;
};
