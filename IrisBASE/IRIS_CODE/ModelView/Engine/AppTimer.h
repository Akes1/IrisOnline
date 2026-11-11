/* ====================================================================
* 파    일 :	AppTimer.h
* 목    적 :	어플리케이션 기본 타이머 클래스
* 작 성 자 :	이법석
* 작 성 일 :	2005.12
* 주의사항 :	
* =================================================================== */
#pragma once

#include "sqltypes.h"

/// 어플리케이션 타이머
class cAppTimer
{
	static cAppTimer* mpSingleton;

public:
	cAppTimer();
	~cAppTimer();

	/// 타이머 시작 및 종료
	void Start();
	void Stop();

	/// 재설정
	void Reset();

	///
	void Advance();

	// get the absolute system time
	unsigned long GetAbsoluteTime();

	// get the current time
	unsigned long GetCurrentTime();

	// get the time that elapsed between GetElapsedTime() calls
	unsigned long GetDeltaTime();

	inline bool IsStopped() { return mTimerStopped; }

	/// 현재 컴퓨터의 실제 시계를 가져옴
	TIMESTAMP_STRUCT GetRealTime();
	/// 현재 컴퓨터의 실제 시계에 sec(단위:초)를 더한 시계를 가져옴
	TIMESTAMP_STRUCT GetRealPlusTime( unsigned long sec );

	/// TIMESTAMP_STRUCT 형태의 시계 값을 time_t로 전환
	tm ConvertTStoTM( TIMESTAMP_STRUCT timestamp );
	/// time_t 형태의 시계 값을 TIMESTAMP_STRUCT로 전환
	TIMESTAMP_STRUCT ConvertTMtoTS( time_t datetime );

	/// 실제 시간과 입력된 시간의 차(단위:초/1000)를 반환한다.
	unsigned long DiffRealTime( TIMESTAMP_STRUCT timestamp );

public:
	/// 단일체를 리턴
	static cAppTimer* GetSingleton();

private:
	/// QueryPerformanceFrequency 사용 여부
	bool	mUsingQPF;

	///
	bool	mTimerStopped;

	/// QueryPerformanceCounter..
	LONGLONG		mllQPFTicksPerSec;

	LONGLONG		mllStopTime;
	LONGLONG		mllLastElapsedTime;
	LONGLONG		mllBaseTime;

	/// TimeGetTime..
	unsigned long	mLastElapsedTime;
	unsigned long	mBaseTime;
	unsigned long	mStopTime;

	//	double			mLastElapsedTime;
	//	double			mBaseTime;
	//	double			mStopTime;

};

inline cAppTimer* cAppTimer::GetSingleton()
{
	return mpSingleton;
}

#define TIMER	cAppTimer::GetSingleton()
