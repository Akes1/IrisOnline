/* ==========================================================================
* 파    일 : DuelMsgWindow.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2008-05-16
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UiWindow.h"
#include "RangeCheck.h"

const unsigned long	WINDOW_CLOSE_TIME = 10000;	/// 요청 응답 시간
const float			DUEL_REQ_RANGE = 1000;		/// 요청 가능 거리

/// 결투 윈도우
class cLabel;
class cDuelMsgWindow : public cUIWindow
{
public:
	cDuelMsgWindow();
	virtual ~cDuelMsgWindow();

	virtual void Open() {}

	void ShowModal();

public:
	void SetTargetIdx( unsigned long duelTargetIdx ) { mDuelTargetIdx = duelTargetIdx; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnShow();
	void OnCommand( cUINode* node, unsigned int id );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	bool HandleEvent( const cUIEvent& event );
	void UpdateSkin();

	void ButtonSelect( unsigned int id );

private:
	unsigned long	mWindowCloseTime;
	unsigned long	mDuelTargetIdx;

	cRangeCheck		mRangeCheck;

	unsigned long	mLeftTime;		///  남은 초

	cLabel*		mpMsgText;
};
