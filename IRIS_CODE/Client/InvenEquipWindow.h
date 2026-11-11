/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.10
* 내    용 : 인벤토리 장착 윈도우
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIWindow.h"
#include "ItemManager.h"

/// 인벤토리 장착 윈도우
class cItemIcon;
class cInvenEquipWindow : public cUIWindow
{
public:
	cInvenEquipWindow();
	virtual ~cInvenEquipWindow();
	
	virtual void Open();
	bool SetWearCardSlot( unsigned short slotEnd );

protected:
	bool OnCreate( cUINodeProperty* property );
	void OnIconDragged( cUINode* pcaller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconDoubleClicked( cUINode* pcaller, unsigned int id );
	void OnIconClicked( cUINode* caller, unsigned int id );
	void UpdateSkin() {}

protected:
	cItemIcon* mIcon[EQUIP_SLOT_COUNT];
};
