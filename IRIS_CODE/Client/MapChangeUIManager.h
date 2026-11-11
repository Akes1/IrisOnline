#pragma once

class cLoadBackWindow;
class cLoadingWindow;


class cMapChangeUIManager
{
public:
	cMapChangeUIManager();
	~cMapChangeUIManager();

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
