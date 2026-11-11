/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.12
* 내    용 : 가방 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "ItemManager.h"

class cInventoryWindow;
class cLabel;
class cUIImage;
class cItemIcon;
/// 가방 윈도우
class cBagWindow : public cUIWindow
{
public:
	cBagWindow( cInventoryWindow* invenWin, unsigned int index );
	virtual ~cBagWindow();

	virtual void Open();
	virtual void Close();

	void ChangeSelect( unsigned short slotIndex );
	void HideSelect();

	bool Reset();
	void SetUseFlag( bool flag ){ mUseFlag = flag; }

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnRender(cUIFontItemKeeper* pKeeper );
	void OnCommand( cUINode* node, unsigned int id );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconDoubleClicked( cUINode* caller, unsigned int id );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );
	void UpdateRect();
	void UpdateSkin();
	void OnHide();
	void OnShow();
	void Show( bool onsound = true );
	void HideESC( bool onsound );
	
	void UpdateSelect( cItemIcon* icon );

protected:
	/// 인벤토리 윈도우
	cInventoryWindow* mInventoryWindow;
	cLabel*			mCaption;

	/// 선택된 아이콘 표시 
	cItemIcon*  mSelectIcon;
	cUIImage*	mpSelectImage;
	bool		mShowSelect;

	/// 인덱스
	unsigned int mIndex;

	/// 아이콘
	cItemIcon* mIcon[BAG_SLOT_COUNT];

	bool mUseFlag; /// 백을 사용할수 있는지 체크하는 플레그
};

inline void cBagWindow::HideSelect()
{
	mShowSelect = false;
}