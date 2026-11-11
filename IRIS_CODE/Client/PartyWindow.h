/* ==========================================================================
* 파    일 : PartyWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-07
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "Party_Common.h"

enum ePartyStateDisplay
{
	ePARTY_STATE_DISPLAY_NORMAL = 0,
	ePARTY_STATE_DISPLAY_MINI,
	ePARTY_STATE_DISPLAY_HIDE,

	ePARTY_STATE_DISPLAY_MAX,
};

/// 파티관리 윈도우
class cLabel;
class cButton;
class cPartyStateWindow;
class cPushButton;
class cPartyWindow : public cUIWindow
{
public:
	cPartyWindow();
	virtual ~cPartyWindow();

	virtual void Open();
	virtual void Close();

	void Clear();

	void SetUnion();

	/// hero 의 타겟을 갱신할지 여부
	void ChangeTarget( cPartyStateWindow* w );
	void SetLeader( bool set );
	void SetDivideType( PARTY_DIVIDE_TYPE type );
	
	cUIRect		 GetButtonRect();

	void ChangeDisplayType( unsigned char type );

	unsigned char GetDisplayType() { return mDisplayType; }

	// 이동 기능 고정
	bool IsLockedMove();
	void SetMoveLock( bool lock );

	// 주사위 굴리기 버튼 
	void SetEnableDice( bool enable );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* caller, unsigned int id );
	void OnMouseMove( const cUIPos& pos );
	void UpdateRect();
	void OnHide();

	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	
private:
	cUINodeSkin*	mpNodeSkin;

	cLabel*		mpDivide;
	
	/// 메뉴버튼
	cButton*	mpMenuButton;

	// 이동 고정
	cPushButton*	mpMoveLock;

	// 파티창 보기 타입
	unsigned char	mDisplayType;

	// 주사위 굴리기 버튼
	cButton*	mpDiceButton;
};


/// 파티 메뉴
class cPartyMenu : public cUIWindow
{
public:
	cPartyMenu();
	virtual ~cPartyMenu();

	virtual void Open() {}

	/// 버튼활성화 설정
	void SetEnableButton( bool set );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* caller, unsigned int id );
	void OnMouseOtherClick( const cUIPos& pos );
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	/// 리더인지 여부에 따른 활성화
	bool		mSetEnable;

	cButton*	mpInvite;
	cButton*	mpChangeLeader;
	cButton*	mpChangeDivide;
	cButton*	mpThrowUser;
	cButton*	mpOut;
};


/// 파티 분배 메뉴
class cPartyDivideMenu : public cUIWindow
{
public:
	cPartyDivideMenu();
	virtual ~cPartyDivideMenu();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* caller, unsigned int id );
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	// 버튼
	cButton*	mpSequence;
	cButton*	mpRandom;
	cButton*	mpDamage;
	cButton*	mpFree;
	cButton*	mpLeader;
};
