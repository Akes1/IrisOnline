/* ==========================================================================
* 파    일 : UIWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006.09.29
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIID.h"
#include "UIContainer.h"
#include "UIMsgboxEvent.h"

/// UI 윈도우
class cListBox;
class cBitArray;
class cBaseStage;
class cUINodeProperty;

class cUIWindow : public cUIContainer
{
public:
	cUIWindow( eUINodeType type = eUINODE_WINDOW );
	virtual ~cUIWindow();

	/// 이벤트
	virtual bool HandleEvent( const cUIEvent& event );

	/// 위치 초기화
	virtual bool Reset();

	/// 스킨 세팅
	virtual bool SetSkin( const cUINodeSkin* pskin );

	/// 드래그
	bool ContainPoint( const cUIPos& pos ) const;

	/// stage 변경 시점에만 온다
	virtual void Open();
	virtual void Close();

	///
	virtual void ShowModal( eMSGBox_Event eventType, eMSGBox_Type modalType = eMSGBOX_MODAL );
	virtual void Show( bool onsound = true );
	virtual void Hide( bool onsound = true );

	/// ESC 로 인해 꺼졌을때 따로 처리할 내용.
	virtual void HideESC( bool onsound = true );

	/// 쇼 토글 
	virtual void Toggle();
	
	/// 캡션 높이 위치를 얻어옴
	cUIRect GetCaptionRect() const;

	void SetCaptionRect( cUIRect& rect );

	///
	virtual void NetLock( int lockTry = 0 );
	virtual void NetUnLock( int lockTry = 0 );

	inline eMSGBox_Event GetEvent() { return mEventType; }

	eMSGBox_Type GetModalType(){ return mModalType; }

	void SetCaller( cUIWindow* caller ) { mpCaller = caller; }

	void RegistMsgBox( cUIWindow* n );
	cUIWindow* GetRegistMsgBox() { return mpRegistMsgBox; }

	/// 자신의 호출자의 메세지 박스를 클리어
	void DeleteCallerMsgBox();

protected:
	/// 자식생성 ( Create 다음에 바로 호출되므로 생성할때 필요한 일들을 하면된다)
	virtual bool OnCreate( cUINodeProperty* pproperty );

	/// 처리
	virtual void OnProcess( unsigned long deltaTime, unsigned long accumTime );

	/// 렌더링
	virtual void OnPreRender();
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	
	/// 이벤트
	virtual void OnShow();
	virtual void OnHide();
	virtual void OnOk();
	virtual void OnCancel();
	virtual void OnTimer();
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool /*shiftPress*/ );
	virtual void OnLButtonUp( const cUIPos& pos );
	virtual void OnCaptureLost();
	virtual void OnNetLock( int lockTry ) {}
	virtual void OnNetUnLock( int lockTry ) {}

	/// 알파
	virtual void SetAlpha( float a );

	/// 타이머
	void SetTimer( bool enabled, unsigned long timeout = 0 );
	
private:
	cUIWindow* CreateChildWindow( eUINodeType type, const cString& name );
	cListBox* CreateChildListBox( eUINodeType type, const cString& name );

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

	cBaseStage* mMsgHandler;	/// 메시지 핸들러
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
