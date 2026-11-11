/* ==========================================================================
* 파    일 : ServerUIManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-21
* 주의사항 :
*===========================================================================*/
#pragma once

class cServerWindow;

class cServerUIManager
{
public:
	cServerUIManager();
	~cServerUIManager();

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

	void SetEnableWindow( bool enable );
	void SetEnableConnectBtn( bool enable );

protected:
	cServerWindow*		mpServerWindow;
};