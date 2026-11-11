/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.07.11
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cIcon;

/// 드래그 출처
enum eDragFrom
{
	DRAG_FROM_INVENTORY,
	DRAG_FROM_NPC_STORE,
	DRAG_FROM_QUICKSLOT,
	DRAG_FROM_TRADE,
	DRAG_FROM_USERSELL,
	DRAG_FROM_CHATLINK,
	DRAG_FROM_MEGAPHONELINK,
	DRAG_FROM_NPCDEALREGIST,
	DRAG_FROM_MAILREAD,
	DRAG_FROM_MAILSEND,

	DRAG_FROM_SKILLWINDOW,
	DRAG_FROM_COMMUNITYWINDOW,

	DRAG_FROM_SHORTCUTWINDOW,

	DRAG_FROM_MAX,
};

/// 드래그 윈도우
class cDragWindow : public cUIWindow
{
public:
	cDragWindow( eUINodeType type = eUINODE_DRAGWINDOW );
	~cDragWindow();

	virtual void Open();
	virtual void Close();

	void Show( cUIWindow* pCaller, eDragFrom dragFrom, cBaseIcon* icon, unsigned int iconType, const cUIPos& pos, bool shift );
	void Hide( bool onsound = true );
	void Hide( cUIWindow* pCaller );

protected:
	bool OnCreate( cUINodeProperty* prop );
	void OnMouseMove( const cUIPos& pos );
	void OnLButtonUp( const cUIPos& pos );
	void OnDragFromInventory( const cUIPos& pos );
	void OnDragFromNpcStore( const cUIPos& pos );
	void OnDragFromQuickSlot( const cUIPos& pos );
	void OnDragFromSkillWindow( const cUIPos& pos );
	void OnDragFromCommunityWindow( const cUIPos& pos );
	void OnDragFromTradeWindow( const cUIPos& pos ); 
	void OnDragFromUserSellWindow( const cUIPos& pos ); 
	void OnDragFromChatWindow( const cUIPos& pos ); 
	void OnDragFromMegaphoneWindow( const cUIPos& pos ); 
	void OnDragFromNpcDealRegistWindow( const cUIPos& pos ); 
	void OnDragFromMailReadWindow( const cUIPos& pos ); 
	void OnDragFromMailSendWindow( const cUIPos& pos ); 
	void OnDragFromShortcutWindow( const cUIPos& pos );

	/// 드래그 상황
	void DragToQuickSlot( eDragFrom dragFrom, cIcon* icon );
	void DragToItemSlot( cIcon* icon );

protected:
	/// 쉬프트 눌림
	bool mShiftDown;

	/// 드래그 출처
	eDragFrom mDragFrom;

	/// 드래그 윈도우 콜러 
	cUIWindow* mpCaller;

	unsigned int mIconType;
	cIcon* mIconFrom;

	cIcon* mShowIcon;
};

//---------------------------------------------
class cUnionIcon;
class cUnionDragWindow : public cDragWindow
{
public:
	cUnionDragWindow( eUINodeType type = eUINODE_DRAGWINDOW );
	~cUnionDragWindow();

	virtual void Open();
	virtual void Close();

	void Show( cUIWindow* pCaller, cUnionIcon* icon, const cUIPos& pos );
	void Hide( bool onsound = true );
	void Hide( cUIWindow* pCaller );

protected:
	bool OnCreate( cUINodeProperty* prop );
	void OnLButtonUp( const cUIPos& pos );

	void OnDragFromUnionWindow( const cUIPos& pos );

protected:
	/// 드래그 윈도우 콜러 
	cUIWindow* mpCaller;
	cUnionIcon* mUnionIconFrom;
	cUnionIcon* mUnionShowIcon;
};