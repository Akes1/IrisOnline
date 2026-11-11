/* ==========================================================================
* 파    일 : SelectUIManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-12-21
* 주의사항 :
*===========================================================================*/
#pragma once

class cUIWindow;
class cSelectWindow;
class cChannelWindow;
class cSelectInfoWindow;
class cSelectButtonWindow;

struct MSG_CHARACTER_BASEINFO;

class cSelectUIManager
{
public:
	cSelectUIManager();
	~cSelectUIManager();

	/// stage->InitStage에서 호출 : 초기화
	void Init();

	/// stage->Open에서 호출 : 진입 시점
	void Open();

	/// stage->Close에서 호출 : 변경 시점
	void Close();

	/// stage->Process에서 호출
	void Process( unsigned long accumtime );

	void VisibleListState();
	void VisibleChannelState();

	void SetSelectInfo( int level, LPCTSTR name, unsigned long jobIdx, unsigned int mapIdx );
	void ClearSelectInfo();

	void SetSelectWindowFrame( unsigned char state, bool enable, bool deleteWait );

	long GetCurrentChannelNum();

	void SetEnableWindow( bool enable );

protected:
	/// 캐릭터 정보 윈도우
	cSelectInfoWindow*	mpSelectInfoWindow;

	/// list
	cSelectWindow*	mpSelectWindow;

	/// 채널윈도우 (only channel)
	cChannelWindow* mpChannelWindow;

	cSelectButtonWindow* mpSelectButtonWindow;

	cUIWindow* mpGradeWindow;
};
