/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.08.03
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIWindow.h"

/// 아이템 메시지 윈도우
class cButton;
class cLabel;
class cItemMsgWindow : public cUIWindow
{
public:
	cItemMsgWindow();
	~cItemMsgWindow();

	virtual void Open();
	virtual void Close();

	void ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex, unsigned long itemIndex, unsigned int count, unsigned long price );
	void ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex0, unsigned long slotIndex1, unsigned long itemIndex );

	unsigned long GetSlotIndex0() { return mSlotIndex0; }
	unsigned long GetSlotIndex1() { return mSlotIndex1; }

	unsigned long GetItemIndex() { return mItemIndex; }
	unsigned int GetCount() { return mCount; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	bool HandleEvent( const cUIEvent& event );
	void OnHide();

private:
	/// 확인, 예, 취소
	cButton*	mpOk;
	cButton*	mpYes;
	cButton*	mpCancel;

	cLabel*		mpCaption;
	cLabel*		mpText;

	/// 관련 정보
	unsigned long mSlotIndex0;
	unsigned long mSlotIndex1;
	unsigned long mItemIndex;
	unsigned int mCount;
};
