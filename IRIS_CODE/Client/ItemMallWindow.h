/* ==========================================================================
* 파    일 : ItemMallWindow.h
* 목    적 : 아이템 몰 윈도우
* 작 성 자 :
* 작 성 일 : 2009-12-14
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cItemMallWindow : public cUIWindow
{
public:
	cItemMallWindow();
	virtual ~cItemMallWindow();

	void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void UpdateRect();

	void OnShow();
	void OnHide();

};