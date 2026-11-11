/* ==========================================================================
* 파    일 : MakeUIManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-21
* 주의사항 :
*===========================================================================*/
#pragma once

class cUIWindow;
class cMakeWindow;
class cMakeInfoWindow;
class cMakeRotateWindow;

struct sPlayerInfo;
class cMakeUIManager
{
public:
	cMakeUIManager();
	~cMakeUIManager();


	/// stage->InitStage에서 호출 : 초기화
	void Init( );

	/// stage->Open에서 호출 : 진입 시점
	void Open();

	/// stage->Close에서 호출 : 변경 시점
	void Close();

	/// 화면을 보여주기 시작하는 시점에 호출
	//void Visible();

	/// stage->Process에서 호출
	void Process( unsigned long accumtime );

	/// make창 변화
	void GetDisplayInfo( sPlayerInfo* info );

	unsigned char CheckDisplayUpdate();

	void SetEnableWindow( bool enable );

	void SetMakeInfoWindow( unsigned char race, unsigned char gender, unsigned long job );

protected:
	/// 생성 윈도우
	cMakeWindow*		mpMakeWindow;
	/// 생성 정보 윈도우 
	cMakeInfoWindow*	mpInfoWindow;
	/// 생성 회전버튼 윈도우
	cMakeRotateWindow*	mpRotateWindow;
	/// 등급 표시 윈도우
	cUIWindow*			mpGradeWindow;
};

