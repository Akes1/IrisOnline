/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.08.10
* 내    용 : 창고 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "TabWindow.h"
#include "MoneyWindow.h"
#include "ItemManager.h"

class cPushButton;
class cItemIcon;
class cUIImage;
class cLabel;

/// 창고 윈도우
class cWareHouseWindow : public cTabWindow
{
public:
	cWareHouseWindow();
	virtual ~cWareHouseWindow();

	virtual void Open();
	virtual void Close();

	/// 가방 윈도우 아이콘 눌림 상태 설정
	void SetBagPressed( unsigned int index, bool pressed );

	/// 금액
	void SetMoney( unsigned long money );

	void ChangeSelect( unsigned short slotIndex );
	bool SetWareBagEndSlot( unsigned short endBagSlot );
	void HideSelect(){ mShowSelect = false; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnCommand( cUINode* node, unsigned int id );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnPushButtonPressed( cUINode* caller, unsigned int id );
	void OnTabButtonPressed(cUINode* caller, unsigned int id );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void UpdateRect();
	void OnShow();
	void OnHide();

	void UpdateSelect( cItemIcon* icon );
	
protected:
	static const int TAB_COUNT = 1;

	/// 선택된 아이콘 표시 
	cItemIcon*  mSelectIcon;
	cUIImage*	mpSelectImage;
	bool		mShowSelect;

	cPushButton* mBagButton[3];

	BYTE mShowBit;

	/// 금액 에디트
	cNumberEditBox*	mpMoney;
};


/// 창고 가방 윈도우
class cWareHouseBagWindow : public cUIWindow
{
public:
	cWareHouseBagWindow( cWareHouseWindow* wareHouseWindow, unsigned int index );
	virtual ~cWareHouseBagWindow();

	virtual void Open();
	virtual void Close();

	bool Reset();

	void ChangeSelect( unsigned short slotIndex );
	void HideSelect(){ mShowSelect = false; }
	void SetUseFlag( bool flag ){ mUseFlag = flag; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* node, unsigned int id );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void UpdateRect();
	void Show( bool onsound = true );
	void OnHide();
	void HideESC( bool onsound );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );

	void UpdateSelect( cItemIcon* icon );

protected:
	/// 창고 윈도우
	cWareHouseWindow* mWareHouseWindow;

	/// 라벨 인덱스
	unsigned int	mIndex;
	cLabel*			mCaption;

	/// 선택된 아이콘 표시 
	cItemIcon*		mSelectIcon;
	cUIImage*		mpSelectImage;
	bool			mShowSelect;
	bool		    mUseFlag; /// 백을 사용할수 있는지 체크하는 플레그

	cItemIcon* mIcon[WAREHOUSE_SLOT_COUNT];
};
