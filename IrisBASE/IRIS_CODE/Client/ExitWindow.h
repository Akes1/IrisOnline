/* ==========================================================================
* 파    일 : ExitWindow.h
* 목    적 : 게임 종료 윈도우
* 작 성 자 : 김가람
* 작 성 일 : 2007-10-24
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cButton;
class cLabel;
class cExitWindow : public cUIWindow
{
public:
	cExitWindow();
	virtual ~cExitWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void OnShow();

private:
	/// 버튼
	cButton* mpDisConnect;
	cButton* mpGameOut;

	cLabel* mpTitle;
	cLabel* mpText;
};

// 튜토리얼 종료 윈도우
class cTutorialExitWindow : public cUIWindow
{
public:
	cTutorialExitWindow();
	virtual ~cTutorialExitWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void UpdateSkin();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void OnShow();

private:
	/// 버튼
	cButton* mpTutorialOut;
	cButton* mpGameOut;

	cLabel* mpTitle;
	cLabel* mpText;
};

enum eExitEventType
{
	eEXIT_DISCONNECT,		// 게임종료
	eEXIT_GAMEOUT,			// 캐릭터선택

	eEXIT_NONE,
};

//
class cExitMsgWindow : public cUIWindow
{
public:
	cExitMsgWindow();
	virtual ~cExitMsgWindow();

	virtual void Open() {}

	void ShowExitMsg( int type );
	void StartCount() { mStartCount = true; }

	void HideESC( bool onsound = true );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* node, unsigned int id );
	void UpdateSkin();
	bool HandleEvent( const cUIEvent& event );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	cLabel* mpCaption;
	cLabel* mpMsgText;
	cButton* mpCancelBtn;

	unsigned long mDelayTime;
	bool mStartCount;

	// 타입
	int mExitEventType;
};
