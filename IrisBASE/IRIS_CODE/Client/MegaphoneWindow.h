/* ==========================================================================
* 파    일 : MegaphoneWindow.h
* 목    적 : 
* 작 성 자 : 
* 작 성 일 : 2010-01-12
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cEditBox;
class cItemIcon;
class cMegaphoneWindow : public cUIWindow
{
public:
	cMegaphoneWindow();
	virtual ~cMegaphoneWindow();

	void Open() {}
	void Close();
	void Clear();

	void Show( bool onsound = true ) { assert(0); }
	void Show( unsigned short slotIndex, unsigned long dbidx );

	bool IsLinked();
	bool AddLinkItem( unsigned short slotIndex, unsigned long itemIndex );
	bool DelLinkItem();
	unsigned short GetLinkSlotIndex();
	unsigned long GetLinkItemIndex();

	//
	void StartCoolTime();

	// 메가폰을 호출한 아이템 DB 인덱스
	unsigned long GetInventoryIndex() { return mInventoryIndex; }

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );
	void OnIconDragged( cUINode* , unsigned int , const cUIPos& , bool );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );
	void OnShow();
	void OnHide();

private:
	cEditBox*	mpChatEdit;
	cItemIcon*	mpLinkIcon;
	cUINode*	mpOk;

	// caller item DB index
	unsigned long mInventoryIndex;
	unsigned short mInventorySlotIndex;

	// 아이템 링크 ( 아이템인덱스 연결 or 아이템 슬롯 연결 )
	unsigned long mLinkItemIndex;
};	
