/* ==========================================================================
* 파    일 : UIWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006.09.29
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIContainer.h"
#include "UIMsgboxEvent.h"

/// UI 윈도우
class cListBox;
class cUIWindow : public cUIContainer
{
public:
	cUIWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cUIWindow();

protected:
	/// 드래그
	bool mDragging;
	cUIPos mDragStart;
	
	/// 캡션영역 
	cUIRect mCaptionRect;

	/// 타이머
	bool mTimerEnabled;
	unsigned long mTimeOut;
	unsigned long mAccumTime;
	unsigned long mLastAccumTime;

//	cBaseStage* mMsgHandler;	/// 메시지 핸들러
	eMSGBox_Event mEventType;	/// 메시지박스 이벤트 
	eMSGBox_Type mModalType;	/// 메시지박스 타입
	cUIWindow* mpCaller;

	/// show 시 플레이 사운드 인덱스
	unsigned long mOpenSoundIdx;

	/// close 시 플레이 사운드 인덱스
	unsigned long mCloseSoundIdx;

	/// 알파 비트 배열
	cBitArray* mAlphaBitArray;

	// 등록 가능한 메세지박스
	cUIWindow*	mpRegistMsgBox;
};

////////////////////////////////////////////////////////////////////////////////////
class cBitArray;

/// UI 윈도우 스킨
class cUIWindowSkin : public cUIContainerSkin
{
	friend class cUISkin;

public:
	cUIWindowSkin( eUINodeType type = eUINODE_WINDOW );
	virtual ~cUIWindowSkin();

	static cUINodeSkin*	Create( const cString& name );

	/// 로딩
	bool Load( cParser& parser );

public:
	/// 알파 비트 배열
	cBitArray* mAlphaBitArray;

	///
	float mAlpha;

	/// 반드시 보여야하는 윈도우 설정 
	bool mHoldView;

	/// 캡션영역 
	cUIRect mCaptionRect;

	/// 
	unsigned long mOpenSoundIdx;
	unsigned long mCloseSoundIdx;
};
