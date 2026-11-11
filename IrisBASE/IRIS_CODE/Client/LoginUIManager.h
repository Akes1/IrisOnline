/* ==========================================================================
* 파    일 : LoginUIManager.h
* 목    적 : 로그인 스테이지 ui 관리자 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-30
* 주의사항 :
*===========================================================================*/
#pragma once

class cLoginWindow;
class cVersionWindow;

class cLoginUIManager
{
	static cLoginUIManager* mpSingleton;

public:
	cLoginUIManager();
	~cLoginUIManager();

	/// stage->InitStage에서 호출 : 초기화
	void Init();

	/// stage->Open에서 호출 : 진입 시점
	void Open();

	/// stage->Close에서 호출 : 변경 시점
	void Close();

	/// 화면을 보여주기 시작하는 시점에 호출
	void Visible();

	/// stage->Process에서 호출
	void Process( unsigned long accumtime );

	void FocusIDEdit();
	void FocusPassEdit();

	void SetSendLoginFlag( bool set );

	cLoginWindow*   GetLoginWindow(){   return mpLoginWindow; }

public:
	/// 단일체를 리턴
	static cLoginUIManager* GetSingleton();

protected:
	cLoginWindow*	mpLoginWindow;
	cVersionWindow* mpVersionWindow;
};

inline cLoginUIManager* cLoginUIManager::GetSingleton()
{
	return mpSingleton;
}

#define LOGINUI cLoginUIManager::GetSingleton()