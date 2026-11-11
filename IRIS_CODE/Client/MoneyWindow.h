/* ==========================================================================
* 파    일 : MoneyWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-04-25
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

enum eMoneyType
{
	eMONEY_NONE,	/// 금액 입력이지만, max 버튼 없음
	eMONEY_IN,		/// 입금	
	eMONEY_OUT,		/// 출금
};

enum eWindowType
{
	eWINDOW_MONEY,	/// 일반 금액입력용
	eWINDOW_MODIFY,	/// 금액 수정용
};

/// 금액 윈도우
class cButton;
class cNumberEditBox;
class cMoneyWindow : public cUIWindow
{
public:
	cMoneyWindow();
	virtual ~cMoneyWindow();
	
	virtual void Open() {}

	/// 금액 입력용 윈도우
	void ShowMoney( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex = USHRT_MAX );

	/// 금액 변경용 윈도우
	void ShowMoneyModify( eMoneyType type, eMSGBox_Event eventtype, unsigned short slotIndex = USHRT_MAX );

	void SetMoney( unsigned int value );
	unsigned long GetMoney();
	unsigned short GetSlotIndex() { return mItemSlotIndex; }

protected:
	bool OnCreate( cUINodeProperty* property );
	bool HandleEvent( const cUIEvent& event );
	void OnCommand( cUINode* node, unsigned int id );
	void OnButtonDowned( cUINode* , unsigned int id );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void OnHide();

protected:
	/// 숫자 에디트
	cNumberEditBox* mNumEdit;
	cButton*	mpMax;
	cButton*	mpOk;
	cButton*	mpCancel;
	cButton*	mpModify;
	cButton*	mpDelete;

	/// 값
	unsigned long mMaxValue;

	eWindowType mWindowType;
	bool mSendEvent;

	///
	cBaseStage* mMsgHandler;
	eMSGBox_Event mEventType;
	unsigned short mItemSlotIndex;
};