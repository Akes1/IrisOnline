#include "StdAfx.h"
#include "AppTimer.h"
#include "time.h"

cAppTimer* cAppTimer::mpSingleton = 0;

cAppTimer::cAppTimer()
{
	assert( mpSingleton == 0 && "bad singleton!" );
	mpSingleton = this;

	mTimerStopped = true;

	mllBaseTime        = 0;
	mllStopTime        = 0;
	mllLastElapsedTime = 0;

	mBaseTime			= 0;
	mStopTime			= 0;
	mLastElapsedTime	= 0;

	mUsingQPF = false;

	//	LARGE_INTEGER qwTicksPerSec;
	//	mUsingQPF = QueryPerformanceFrequency( &qwTicksPerSec )? true:false;

	//	if( mUsingQPF )
	//	{
	//		mllQPFTicksPerSec = qwTicksPerSec.QuadPart;
	//	}
}

cAppTimer::~cAppTimer()
{
	mpSingleton = 0;
}

void cAppTimer::Start()
{
	if( mUsingQPF )
	{
		LARGE_INTEGER qwTime;
		QueryPerformanceCounter( &qwTime );

		if( mTimerStopped )
		{
			mllBaseTime += qwTime.QuadPart - mllStopTime;
		}

		mllStopTime = 0;
		mllLastElapsedTime = qwTime.QuadPart;
		mTimerStopped = false;
	}
	else
	{
		unsigned long time;
		time = timeGetTime();

		if( mTimerStopped )
		{
			mBaseTime += time - mStopTime;
		}

		mStopTime = 0;
		mLastElapsedTime  = time;
		mTimerStopped = false;
	}
}

void cAppTimer::Stop()
{
	if( mUsingQPF )
	{
		LARGE_INTEGER qwTime;
		if( mllStopTime != 0 )
		{
			qwTime.QuadPart = mllStopTime;
		}
		else
		{
			QueryPerformanceCounter( &qwTime );
		}

		if( !mTimerStopped )
		{
			mllStopTime = qwTime.QuadPart;
			mllLastElapsedTime = qwTime.QuadPart;
			mTimerStopped = true;
		}
	}
	else
	{
		unsigned long time;
		if( mStopTime != 0 )
		{
			time = mStopTime;
		}
		else
		{
			time = timeGetTime();
		}

		if( !mTimerStopped )
		{
			mStopTime = time;
			mLastElapsedTime  = time;
			mTimerStopped = true;
		}
	}
}

void cAppTimer::Reset()
{
	if( mUsingQPF )
	{
		LARGE_INTEGER qwTime;
		if( mllStopTime != 0 )
		{
			qwTime.QuadPart = mllStopTime;
		}
		else
		{
			QueryPerformanceCounter( &qwTime );
		}

		mllBaseTime			= qwTime.QuadPart;
		mllLastElapsedTime	= qwTime.QuadPart;
		mllStopTime			= 0;
		mTimerStopped		= false;
	}
	else
	{
		unsigned long time;
		if( mStopTime != 0.0 )
		{
			time = mStopTime;
		}
		else
		{
			time = timeGetTime();
		}

		mBaseTime         = time;
		mLastElapsedTime  = time;
		mStopTime         = 0;
		mTimerStopped     = false;
	}
}

void cAppTimer::Advance()
{
	if( mUsingQPF )
	{
		///
		LARGE_INTEGER qwTime;
		if( mllStopTime != 0 )
		{
			qwTime.QuadPart = mllStopTime;
		}
		else
		{
			QueryPerformanceCounter( &qwTime );
		}

		mllStopTime += mllQPFTicksPerSec/(LONGLONG)10;
	}
	else
	{
		///
		unsigned long time;
		if( mStopTime != 0 )
			time = mStopTime;
		else
			time = timeGetTime();

		mStopTime += 100; /// += 0.1f
	}
}

unsigned long cAppTimer::GetAbsoluteTime()
{
	unsigned long absoluteTime;
	if( mUsingQPF )
	{
		LARGE_INTEGER qwTime;
		QueryPerformanceCounter( &qwTime );

		absoluteTime = (unsigned long)( (long double)qwTime.QuadPart / (long double)mllQPFTicksPerSec * 1000.0 );
	}
	else
	{
		absoluteTime = timeGetTime();
	}
	return absoluteTime;
}

unsigned long cAppTimer::GetCurrentTime()
{
	unsigned long currentTime;
	if( mUsingQPF )
	{
		LARGE_INTEGER qwTime;
		if( mllStopTime != 0 )
		{
			qwTime.QuadPart = mllStopTime;
		}
		else
		{
			QueryPerformanceCounter( &qwTime );
		}

		currentTime = (unsigned long)( (long double)( qwTime.QuadPart - mllBaseTime ) / (long double) mllQPFTicksPerSec * 1000.0 );
	}
	else
	{
		if( mStopTime != 0 )
		{
			if( mStopTime < mBaseTime )		// DWORD 형의 한계를 넘어갔다
				currentTime = mStopTime - mBaseTime + 4294967295; //( 2^32 - 1 )
			else
				currentTime = mStopTime - mBaseTime;
		}
		else
		{
			unsigned long time = timeGetTime();
			if( time < mBaseTime )		// DWORD 형의 한계를 넘어갔다
				currentTime = time - mBaseTime + 4294967295; //( 2^32 - 1 )
			else
				currentTime = time - mBaseTime;
		}
	}
	return currentTime;
}

unsigned long cAppTimer::GetDeltaTime()
{
	unsigned long deltaTime;
	if( mUsingQPF )
	{
		LARGE_INTEGER qwTime;
		if( mllStopTime != 0 )
		{
			qwTime.QuadPart = mllStopTime;
		}
		else
		{
			QueryPerformanceCounter( &qwTime );
		}

		deltaTime = (unsigned long)(((long double)qwTime.QuadPart-(long double)mllLastElapsedTime) * 1000.0f / (long double)mllQPFTicksPerSec);
		mllLastElapsedTime = qwTime.QuadPart;
	}
	else
	{
		if( mStopTime != 0 )
		{
			if( mStopTime < mLastElapsedTime )		// DWORD 형의 한계를 넘어갔다
				deltaTime = mStopTime - mLastElapsedTime + 4294967295; //( 2^32 - 1 )
			else
				deltaTime = mStopTime - mLastElapsedTime;

			mLastElapsedTime = mStopTime;
		}
		else
		{
			unsigned long time = timeGetTime();
			if( time < mLastElapsedTime )		// DWORD 형의 한계를 넘어갔다
				deltaTime = time - mLastElapsedTime + 4294967295; //( 2^32 - 1 )
			else
				deltaTime = time - mLastElapsedTime;

			mLastElapsedTime = time;
		}
	}
	return deltaTime;
}


TIMESTAMP_STRUCT cAppTimer::GetRealTime()
{
	TIMESTAMP_STRUCT timestamp;

	struct tm datetime;

	time_t now;

	time( &now );
	localtime_s( &datetime, &now );

	timestamp.year 	 = (SQLSMALLINT)datetime.tm_year+1900;
	timestamp.month  = (SQLSMALLINT)datetime.tm_mon+1;
	timestamp.day    = (SQLSMALLINT)datetime.tm_mday;
	timestamp.hour   = (SQLSMALLINT)datetime.tm_hour;
	timestamp.minute = (SQLSMALLINT)datetime.tm_min;
	timestamp.second = (SQLSMALLINT)datetime.tm_sec;

	return timestamp;
}


TIMESTAMP_STRUCT cAppTimer::GetRealPlusTime( unsigned long sec )
{

	TIMESTAMP_STRUCT timestamp;

	struct tm datetime;

	time_t now, result;

	time( &now );
	localtime_s( &datetime, &now );

	timestamp.year 	 = (SQLSMALLINT)datetime.tm_year+1900;
	timestamp.month  = (SQLSMALLINT)datetime.tm_mon+1;
	timestamp.day    = (SQLSMALLINT)datetime.tm_mday;
	timestamp.hour   = (SQLSMALLINT)datetime.tm_hour;
	timestamp.minute = (SQLSMALLINT)datetime.tm_min;
	timestamp.second = (SQLSMALLINT)datetime.tm_sec;

	datetime.tm_sec = datetime.tm_sec + ( sec / 1000 );

	if( (result = mktime( &datetime )) != (time_t)-1 )
	{
		localtime_s( &datetime, &result );
	}

	timestamp.year 	 = (SQLSMALLINT)datetime.tm_year+1900;
	timestamp.month  = (SQLSMALLINT)datetime.tm_mon+1;
	timestamp.day    = (SQLSMALLINT)datetime.tm_mday;
	timestamp.hour   = (SQLSMALLINT)datetime.tm_hour;
	timestamp.minute = (SQLSMALLINT)datetime.tm_min;
	timestamp.second = (SQLSMALLINT)datetime.tm_sec;

	return timestamp;
}


tm cAppTimer::ConvertTStoTM( TIMESTAMP_STRUCT timestamp )
{
	struct tm datetime;

	datetime.tm_year = timestamp.year-1900;
	datetime.tm_mon = timestamp.month-1;
	datetime.tm_mday = timestamp.day;
	datetime.tm_hour = timestamp.hour;
	datetime.tm_min = timestamp.minute;
	datetime.tm_sec = timestamp.second;

	return datetime;
}


unsigned long cAppTimer::DiffRealTime( TIMESTAMP_STRUCT timestamp )
{
	/// 현재 시간을 time_t값으로 전환
	struct tm tempTime = ConvertTStoTM( GetRealTime() );
	time_t realTime = mktime( &tempTime );

	/// 입력된 시간을 time_t값으로 전환
	tempTime = TIMER->ConvertTStoTM( timestamp );
	time_t endTime = mktime( &tempTime );

	/// 비교후 리턴
	return (unsigned long)(difftime( endTime, realTime ) * 1000.0f);	
}