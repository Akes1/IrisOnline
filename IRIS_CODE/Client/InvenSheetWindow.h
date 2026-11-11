/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.10
* 내    용 : 인벤토리 쉬트 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIWindow.h"
#include "ItemManager.h"

/// 인벤토리 쉬트 윈도우
class cItemIcon;
class cInvenSheetWindow : public cUIWindow
{
public:
	cInvenSheetWindow( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cInvenSheetWindow();

	bool BindSlot( unsigned int tabIndex );

protected:
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconDoubleClicked( cUINode* caller, unsigned int id );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );

protected:
	/// 탭 인덱스
	unsigned int mTabIndex;

	/// 아이콘
	cItemIcon* mIcon[INVENSHEET_SLOT_COUNT];
};
