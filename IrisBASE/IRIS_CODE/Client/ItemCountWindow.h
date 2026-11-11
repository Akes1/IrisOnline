/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.08.03
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIWindow.h"

enum eItemTry;
class cNumberEditBox;
/// 아이템 개수 윈도우
class cItemCountWindow : public cUIWindow
{
public:
	cItemCountWindow();
	virtual ~cItemCountWindow();

	virtual void Open() {}

	void ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex0, unsigned long slotIndex1 );
	void ShowModal( eMSGBox_Event eventtype, unsigned long index );
	void ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex, unsigned long itemIndex, unsigned long maxValue, unsigned long price );

	unsigned long GetItemIndex() { return mItemIndex; }
	unsigned long GetSlotIndex0() { return mSlotIndex0; }
	unsigned long GetSlotIndex1() { return mSlotIndex1; }
	int GetMaxCount() { return mMaxValue; }
	unsigned long GetPrice() { return mPrice; }
	unsigned short GetCount();

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnButtonDowned( cUINode* , unsigned int id );
	bool HandleEvent( const cUIEvent& event );
	void OnShow();

protected:
	/// 숫자 에디트
	cNumberEditBox* mNumEdit;

	/// 슬롯 인덱스
	unsigned long mSlotIndex0;
	unsigned long mSlotIndex1;

	/// 아이템 인덱스
	unsigned long mItemIndex;

	/// 값
	unsigned long mMaxValue;

	//
	unsigned long mPrice;
};

