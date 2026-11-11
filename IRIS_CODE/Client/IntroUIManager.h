/* ==========================================================================
* 파    일 : IntroUIManager.h
* 목    적 : 인트로 스테이지 UI 관리자 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-21
* 주의사항 :
*===========================================================================*/
#pragma once

class cIntroWindow;
class cIntroUIManager
{
public:
	cIntroUIManager();
	~cIntroUIManager();

	void Init();
	void InitStage();
	void Exit();
	void Open();
	void Close();

	cIntroWindow* GetBackWindow() { return mpBackWindow; }
protected:
	/// 배경 윈도우
	cIntroWindow*		mpBackWindow;
};
