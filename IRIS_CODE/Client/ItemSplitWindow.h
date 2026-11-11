/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.08.03
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIWindow.h"

#include "Label.h"
#include "Button.h"
#include "NumberEditBox.h"

//class cLabel;
//class cButton;
//class cTextBox;
//class cIcon;
//class cInventoryWindow;

/// 아이템 분리 윈도우
class cItemSplitWindow : public cUIWindow
{
public:
	cItemSplitWindow();
	virtual ~cItemSplitWindow();

	void ShowModal( unsigned int slotIndex0, unsigned int slotIndex1 );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( float accumTime );
	//void OnRender( cUIFontItemKeeper* keeper );
	void OnOk();
	void OnCancel();
	void OnCommand( cUINode* node, unsigned int id );

protected:
	/// 캡션
	cLabel mCaptionLabel;

	/// 버튼
	cButton mCloseButton;
	cButton mUpButton;
	cButton mDownButton;
	cButton mMaxButton;
	cButton mOkButton;
	cButton mCancelButton;

	/// 숫자 에디트
	cNumberEditBox mNumEdit;

	/// 슬롯 인덱스
	unsigned int mSlotIndex0;
	unsigned int mSlotIndex1;

	int mMaxValue;
	int mValue;
	/// 인벤토리 윈도우
	//cInventoryWindow* mInventoryWindow;
};
