#include "StdAfx.h"
#include ".\synctimer.h"



cSyncTimer* cSyncTimer::mpSyncTimer = NULL;



cSyncTimer::cSyncTimer(void)
{
	/// 싱글톤 - 2번이상 객체를 생성 하려고 할경우 콘솔에 에러 찍어줌
	if( mpSyncTimer )
	{
		MessageBox(NULL,_T("cSyncTimer : 이 클래스는 객체가 유일한객체(singleton)이어야 합니다.\
						이전 객체를 사용하던지 이전 객체를 지우고 생성하십시오."),_T("경고"),MB_OK);
	}
	else
	{
		mpSyncTimer = this;
	}

	mServerTime = 0;
	mLoseTimer = 0;
}



cSyncTimer::~cSyncTimer(void)
{
}



long cSyncTimer::SetServerTime( unsigned long time )
{

	/// 초기값인경우 처음 넘어온 값으로 셋팅
	if( mServerTime == 0 )
	{
		mServerTime = time; 
	}
	/// 시간차를 계산해 기존 시간보다 넘어온 시간이더 동기화가 맞는경우 셋팅
	else if( time - mServerTime < 0  ) 
	{
		mServerTime = time; 
	}	

	return mServerTime;
}



void cSyncTimer::process( float time )
{
	if( mServerTime != 0 )
	{
		mServerTime += (unsigned long)(time*1000);	
	}
	
}