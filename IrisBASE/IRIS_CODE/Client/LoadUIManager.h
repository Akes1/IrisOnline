/* ==========================================================================
* 파    일 : LoadUIManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-21
* 주의사항 :
*===========================================================================*/
#pragma once

class cLoadBackWindow;
class cLoadingWindow;

class cLoadUIManager
{
public:
	cLoadUIManager();
	~cLoadUIManager();

	void Init();
	void Exit();
	void Open( unsigned long mapIdx, unsigned char mapMode );
	void Close();
	void Process();

	bool UpdateLoadingGauge( float curMax );

protected:
	/// 배경 윈도우
	cLoadBackWindow*	mpBackWindow;
	cLoadingWindow*		mpLoadingWindow;
};