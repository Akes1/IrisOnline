/* ==========================================================================
* 파    일 : SyncTimer.h
* 목    적 : 서버와 동기화 목적의 타이머
* 작 성 자 : 박경희
* 작 성 일 : 2007-01-10
* 주의사항 :
*===========================================================================*/

#pragma once

class cSyncTimer 
{
private:
	/// 서버에서 보내온 시간
	unsigned long		mServerTime;
	/// 서버와 클라이언트 시간차 최소값
	long				mLoseTimer;
    
public:

	static cSyncTimer*	mpSyncTimer;

	cSyncTimer(void);
	~cSyncTimer(void);

	/// 서버시간과 가장 가까운 시간을 셋팅한다.
	long				SetServerTime( unsigned long time );
	/// 서버와 동기화된 시간을 알려준다.
	long				GetServerTime() { return mServerTime; }
	/// 동기화된 시간과 입력받은 값의 시간차를 알려준다.
	long				GetLoseTime( long time ) { return  mServerTime==0?0:mServerTime - time;	}

	void				process( float time );

};

#define SYNCTIMER	cSyncTimer::mpSyncTimer