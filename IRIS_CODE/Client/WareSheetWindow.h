/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.10
* 내    용 : 창고 쉬트 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIWindow.h"
#include "ItemManager.h"

/// 창고 쉬트 윈도우
class cItemIcon;
class cWareSheetWindow : public cUIWindow
{
public:
	cWareSheetWindow( eUINodeType type = eUINODE_TABSHEET );
	virtual ~cWareSheetWindow();

	void InitWindow( unsigned int tabIndex );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnIconDragged( cUINode* pcaller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void OnIconRightClicked( cUINode* caller, unsigned int id, bool shift );

protected:
	/// 인덱스
	unsigned int mTabIndex;

	/// 아이콘
	cItemIcon* mIcon[WAREHOUSE_SLOT_COUNT];
};
