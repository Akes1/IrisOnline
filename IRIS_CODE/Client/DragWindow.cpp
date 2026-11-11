#include "stdafx.h"
#include "DragWindow.h"

#include "ItemIcon.h"
#include "SkillIcon.h"
#include "UnionIcon.h"

#include "UIRootNode.h"
#include "ItemManager.h"
#include "GameUIManager.h"
#include "QuickManager.h"
#include "ObjectManager.h"
#include "hero.h"
#include "ChatManager.h"
#include "GameResourceManager.h"

#include "NpcDisjointWindow.h"
#include "EnhancedWindow.h"
#include "TradeWindow.h"
#include "UserSellWindow.h"
#include "WareHouseWindow.h"
#include "PutCardWindow.h"
#include "CardBookWindow.h"
#include "TarotSheetWindow.h"
#include "NpcDealRegistSheet.h"
#include "NpcDealWindow.h"
#include "ChatWindow.h"
#include "MailWindow.h"
#include "MailSheet.h"
#include "InventoryWindow.h"
#include "BagWindow.h"
#include "PartyUnionManager.h"
#include "SoundSystem.h"
#include "ShortcutWindow.h"
#include "MegaphoneWindow.h"

cDragWindow::cDragWindow( eUINodeType type )
: cUIWindow( type )
, mpCaller(0)
, mShowIcon(0)
, mIconType(eICON_MAX)
, mShiftDown( false )
, mDragFrom( DRAG_FROM_MAX )
{
	SetExceptSnap( true );
}

cDragWindow::~cDragWindow()
{
}

void cDragWindow::Open()
{
}

void cDragWindow::Close()
{
	cUIWindow::Close();

	mpCaller = 0;

	ITEMMAN->UnLinkIcon( true );
	QUICKMAN->UnDragIcon();
	PARTYUNIONMAN->UnLinkIcon();

	mIconFrom = 0;
	mIconType = eICON_MAX;
}

void cDragWindow::Show( cUIWindow* pCaller, eDragFrom dragFrom, cBaseIcon* icon, unsigned int iconType, const cUIPos& pos, bool shift )
{
	if( icon == 0 || iconType >= eICON_MAX ) 
	{
		assert( 0 && "Bad Icon or IconType" );
		return;
	}
	
	int iconWidth = icon->GetRelativeRect().GetWidth();
	int iconHeight = icon->GetRelativeRect().GetHeight();

	mIconFrom = (cIcon*)icon;
	mIconType = iconType;

	mShowIcon->SetRelativeSize( cUISize( iconWidth, iconHeight ) );
	mShowIcon->ChangeImage( icon->GetIconParam() );
	mShowIcon->ShowNumber( false );
	mShowIcon->Show();

	switch( mIconType )
	{
	case eICON_NORMAL:
		QUICKMAN->DragIcon( mIconFrom );
		break;
	case eICON_ITEM:
		ITEMMAN->LinkIcon( (cItemIcon*)mIconFrom );
		break;
	case eICON_UNION:
		PARTYUNIONMAN->LinkIcon( (cUnionIcon*)mIconFrom );
		break;
	}

	mpCaller = pCaller;
	mShiftDown = shift;
	mDragFrom = dragFrom;

	/// 드래그윈도우 위치 선정, 크기 재 설정, 이미지 크기 재설정
	cUIPos newPos( (int)(pos.mX - GetAbsoluteRect().GetWidth() * 0.5), (int)(pos.mY - GetAbsoluteRect().GetHeight() * 0.5) );
	SetRelativePos( newPos );
	SetRelativeRect( cUIRect( newPos.mX, newPos.mY, newPos.mX + iconWidth + 4, newPos.mY + iconHeight + 4 ) );

	//
	cUIWindow::Show();
	SetCapture();
}

/// 외부에서 닫힘.
void cDragWindow::Hide( bool onsound )
{
	cUIWindow::Hide( onsound );
	mpCaller = 0;

	ITEMMAN->UnLinkIcon( true );
	QUICKMAN->UnDragIcon();
	PARTYUNIONMAN->UnLinkIcon();

	mIconFrom = 0;
	mIconType = eICON_MAX;
}

void cDragWindow::Hide( cUIWindow* pCaller )
{
	/// 자기를 호출한 콜러가 아니면 무반응
	if( pCaller == mpCaller )
		Hide();
}	

bool cDragWindow::OnCreate( cUINodeProperty* )
{
	mShowIcon = new cIcon;
	if( mShowIcon->CreateBySkinName( "Drag_Icon", this, 1 ) == false )
		return false;

	Hide( false );
	return true;
}

void cDragWindow::OnMouseMove( const cUIPos& pos )
{
	cUIPos newPos( (int)(pos.mX - GetAbsoluteRect().GetWidth() * 0.5), (int)(pos.mY - GetAbsoluteRect().GetHeight() * 0.5) );
	SetRelativePos( newPos );
}

void cDragWindow::OnLButtonUp( const cUIPos& pos )
{
	/// 윈도우 숨기기
	cUIWindow::Hide();
	mpCaller = 0;
	ITEMMAN->UnLinkIcon( true );

	/// 죽어있는데 아이템을 드래그하면 실패처리
	if( HERO->GetState() == eOBJECT_STATE_DIE )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 235 ) );
	}

	switch( mDragFrom )
	{
	case DRAG_FROM_INVENTORY:
		OnDragFromInventory( pos );
		break;
	case DRAG_FROM_NPC_STORE:
		OnDragFromNpcStore( pos );
		break;
	case DRAG_FROM_QUICKSLOT:
		OnDragFromQuickSlot( pos );
		break;
	case DRAG_FROM_TRADE:
		OnDragFromTradeWindow( pos );
		break;
	case DRAG_FROM_USERSELL:
		OnDragFromUserSellWindow( pos );
		break;
	case DRAG_FROM_CHATLINK:
		OnDragFromChatWindow( pos );
		break;
	case DRAG_FROM_MEGAPHONELINK:
		OnDragFromMegaphoneWindow( pos );
		break;
	case DRAG_FROM_SKILLWINDOW:
		OnDragFromSkillWindow( pos );
		break;
	case DRAG_FROM_COMMUNITYWINDOW:
		OnDragFromCommunityWindow( pos );
		break;
	case DRAG_FROM_NPCDEALREGIST:
		OnDragFromNpcDealRegistWindow( pos );
		break;
	case DRAG_FROM_MAILREAD:
		OnDragFromMailReadWindow( pos );
		break;
	case DRAG_FROM_MAILSEND:
		OnDragFromMailSendWindow( pos );
		break;
	case DRAG_FROM_SHORTCUTWINDOW:
		OnDragFromShortcutWindow( pos );
		break;
	}

	QUICKMAN->UnDragIcon();
}

/// 인벤토리 -> 외부 
void cDragWindow::OnDragFromInventory( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	unsigned short slotIndex0 = itemIcon->GetSlotIndex();
	eItemIn in = ITEMMAN->GetItemIn( slotIndex0 );

	cItem* item0 = ITEMMAN->GetItem( slotIndex0 );
	if( !item0 )
	{
		assert(0);
		return;
	}

	/// 쓰레기통에 대한 처리 
	if( node->GetID() == eUIID_INVENTORY_GARBAGE || 
		node->GetID() == eUIID_WAREHOUSE_TRASH ||
		node->GetID() == eUIID_WAREHOUSE_BAG1_TRASH ||
		node->GetID() == eUIID_WAREHOUSE_BAG2_TRASH ||
		node->GetID() == eUIID_WAREHOUSE_BAG3_TRASH ||
		node->GetID() == eUIID_BAG0_TRASH ||
		node->GetID() == eUIID_BAG1_TRASH ||
		node->GetID() == eUIID_BAG2_TRASH )
	{
		/// 장비창에 있는것은 버릴수 없다.
		if( in != ITEM_IN_EQUIP )
			GAMEUI->ShowItemDumpAskWindow( slotIndex0 );
		return;
	}

	/// NPC 상점 체크
	cUIWindow* store = (cUIWindow*)GAMEUI->GetNpcStore();
	if( store && UIMAN->IsIncludeParent( node, store ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		if( mShiftDown )
			return;

		if( item0->GetCount() > 0 )
		{
			if( item0->GetCount() == 1 )
				GAMEUI->ShowItemSellAskWindow( slotIndex0, 1 );
			else
				GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_SELL, slotIndex0 );
		}
		return;
	}

	/// 판매노점 체크
	cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
	if( userSell && UIMAN->IsIncludeParent( node, userSell ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		if( userSell->GetStatus() == eUSERSELL_JOIN )
			return;

		if( userSell->IsNetLock() == false )
		{
			if( userSell->IsOpen() == false )
			{
				CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 2509 ) );
			}
			else
			{
				GAMEUI->ShowMoneyWindow( eMONEY_NONE, eMBEVENT_USERSELL_ITEMMONEYADD, slotIndex0 );
			}
		}
		return;
	}

	/// 채팅윈도우 체크
	cChatWindow* chatWin = GAMEUI->GetChatWindow();
	if( chatWin && UIMAN->IsIncludeParent( node, chatWin ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		chatWin->AddLinkItem( slotIndex0, 0 );
		return;
	}

	/// 메가폰 윈도우
	cMegaphoneWindow* megaWin = GAMEUI->GetMegaphoneWindow();
	if( megaWin && UIMAN->IsIncludeParent( node, megaWin ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		megaWin->AddLinkItem( slotIndex0, 0 );
		return;
	}

	/// 거래 체크
	cTradeWindow* trade = GAMEUI->GetTradeWindow();
	if( trade && UIMAN->IsIncludeParent( node, trade ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		if( trade->IsNetLock() == false )
		{
			ITEMMAN->TryToTradeItemAdd( slotIndex0 );
		}
		return;
	}
	
	/// 카드컬렉션북 체크
	cCardBookWindow* cardBook = GAMEUI->GetCardBookWindow();
	if( cardBook && UIMAN->IsIncludeParent( node, cardBook ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		cItemDefine* define = ITEMMAN->GetItemDefine( item0->GetIndex() );
		if( !define )
			return;

		unsigned char type = define->GetType();
		unsigned char subtype = define->GetSubType();

		/// 현재 컬렉션북 페이지 검사
		if( cardBook->GetCurrentPage() == ePAGE_TAROT )
		{
			if( type == ITEM_CARD && subtype == ITEM_CARD_TAROT )
			{
				/// 타롯 카드인경우,
				cItemTarot* tarot = define->GetTarotDefine();
				cTarotSheetWindow* sheet = cardBook->GetTarotSheet();
				if( tarot && sheet )
				{
					unsigned short bookslotIndex = sheet->GetSlotIndex( tarot->GetUniqueNumber() );
					if( bookslotIndex != USHRT_MAX )
						GAMEUI->ShowItemCardBook( eMBEVENT_ITEM_CARDBOOK_TAROTNEW, slotIndex0, bookslotIndex, 0 );
				}
			}
			else if( type == ITEM_SPREAD )
			{
				/// 스프레드인경우,
				cTarotSheetWindow* sheet = cardBook->GetTarotSheet();
				if( sheet )
				{
					/// 스프레드의 슬롯에 끌어놓은 경우.
					if( node->IsKindof(eUINODE_ICON) )
					{
						cItemIcon* icon = (cItemIcon*)node;
						if( icon->GetSlotIndex() >= INVENTORY_SPREAD_NORMAL_BEGIN && icon->GetSlotIndex() <= INVENTORY_SPREAD_NORMAL_END )
						{
							unsigned short emptyItemSlot = icon->GetSlotIndex();	

							unsigned long registItemIdx = sheet->IsRegistItem( emptyItemSlot - INVENTORY_TAROT_NORMAL_BEGIN );
							if( registItemIdx != ULONG_MAX )
							{
								eMSGBox_Event event = (registItemIdx == 0)? eMBEVENT_ITEM_CARDBOOK_SPREADNEW : eMBEVENT_ITEM_CARDBOOK_SPREADUPDATE;
								GAMEUI->ShowItemCardBook( event, slotIndex0, emptyItemSlot, registItemIdx );
							}
						}
					}
				}
			}
		}
	
		return;
	}

	// 우편함 체크
	cMailWindow* mailWin = GAMEUI->GetMailWindow();
	if( mailWin && UIMAN->IsIncludeParent( node, mailWin ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		cMailSendSheet* sendSheet = mailWin->GetSendSheet();
		if( sendSheet && mailWin->GetCurrentSheet() == sendSheet )
		{
			if( mailWin->IsNetLock() == false )
			{
				sendSheet->AddItem( slotIndex0 );
			}
			return;
		}
	}

	// 분해 체크 
	cNpcDisjointWindow* disjoint = GAMEUI->GetNpcDisjointWindow();
	if( disjoint && UIMAN->IsIncludeParent( node, disjoint ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		disjoint->AddItem( slotIndex0 );
		return;
	}

	// 강화 체크
	cEnhancedWindow* enhance = GAMEUI->GetEnhancedWindow();
	if( enhance && UIMAN->IsIncludeParent( node, enhance ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		cItemDefine* define = item0->GetDefine();
		if( !define )
		{
			assert(0);
			return;
		}

		// item or card
		if( define->GetType() == ITEM_CARD && (define->GetSubType() >= ITEM_CARD_ENHANCE_WEAPON_LV1 && 
											   define->GetSubType() <= ITEM_CARD_ENHANCE_WEAR_LV2_75)  )
		{
			enhance->AddCard( slotIndex0, true );	
		}
		else if( define->GetType() == ITEM_MATERIAL && (define->GetSubType() >= ITEM_MATERIAL_ENHANCED_DOWN_30 && 
														define->GetSubType() <= ITEM_MATERIAL_ENHANCED_DESTROY_60 )	)
		{
			enhance->AddCashProtectCard( slotIndex0, true );	
		}
		else
			enhance->AddItem( slotIndex0, true );
		
		return;
	}

	// 판매대행
	cNpcDealWindow* dealWin = GAMEUI->GetNpcDealWindow();
	if( dealWin && UIMAN->IsIncludeParent( node, dealWin ) )
	{
		if( in != ITEM_IN_INVEN_PRIVATE )
			return;

		cNpcDealRegistSheet* regist = dealWin->GetRegistSheet();
		if( regist && dealWin->GetCurrentSheet() == regist )
		{
			regist->RegistItem( slotIndex0 );
		}
		return;
	}

	/// 아이콘 체크
	if( node->IsKindof(eUINODE_ICON) )
	{
		cIcon* icon = (cIcon*)node;
		
		/// 퀵슬롯 처리
		cUIContainer* parent = node->GetParent();
		if ( parent == NULL )
			return;
		int parentID = parent->GetID();

		/// 퀵슬롯 확인검사
		if( parentID == eUIID_COLUMN_QUICKSLOT    || parentID == eUIID_ROW_QUICKSLOT  ||
			parentID == eUIID_COLUMN_EXTRASLOT1   || parentID == eUIID_ROW_EXTRASLOT1 || 
			parentID == eUIID_COLUMN_EXTRASLOT2   || parentID == eUIID_ROW_EXTRASLOT2 ||
			parentID == eUIID_COLUMN_QUICKSLOT_FS || parentID == eUIID_ROW_QUICKSLOT_FS )
		{
			if( in != ITEM_IN_INVEN_PRIVATE )
				return;

			DragToQuickSlot( DRAG_FROM_INVENTORY, icon );
			return;
		}

		/// 아이템 처리
		if( icon->GetIconType() == eICON_ITEM )
		{
			DragToItemSlot( icon );
			return;
		}
	}
}

/// 스킬창 -> 외부
void cDragWindow::OnDragFromSkillWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );

	if( node && node->IsKindof(eUINODE_ICON) )
	{
		cIcon* icon = (cIcon*)node;
		if( icon->GetIconType() == eICON_NORMAL )
		{
			/// 퀵슬롯 처리
			DragToQuickSlot( DRAG_FROM_SKILLWINDOW, icon );
		}
	}
}

void cDragWindow::OnDragFromCommunityWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );

	if( node && node->IsKindof(eUINODE_ICON) )
	{
		cIcon* icon = (cIcon*)node;
		if( icon->GetIconType() == eICON_NORMAL )
		{
			/// 퀵슬롯 처리
			DragToQuickSlot( DRAG_FROM_COMMUNITYWINDOW, icon );
		}
	}
}

///  npc 상점 -> inven 
void cDragWindow::OnDragFromNpcStore( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	if( node && node->IsKindof(eUINODE_ICON) )
	{
		cItemIcon* icon = (cItemIcon*)node;

		unsigned short slotIndex = icon->GetSlotIndex();
		eItemIn in = ITEMMAN->GetItemIn( slotIndex );

		/// Npc 상점 -> 인벤 개인
		if( in == ITEM_IN_INVEN_PRIVATE )
		{
			unsigned int itemIndex = itemIcon->GetItemIndex();
			assert( itemIndex );

			cItemDefine* itemDefine = ITEMMAN->GetItemDefine( itemIndex );
			assert( itemDefine );

			if( itemDefine )
			{
				/// 1개라도 살수 있어야 구입가능
				if( ITEMMAN->CheckBuyItem( itemIndex, 1 ) == false )
					return;

				if( itemDefine->GetCapacity() == 1 )
				{
					GAMEUI->ShowItemBuyAskWindow( itemIndex, 1 );
				}
				else
				{
					GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_BUY, itemIndex );
				}
			}
		}
	}
}

///
void cDragWindow::OnDragFromQuickSlot( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );

	if( !node )
		return;

	if( node->IsKindof(eUINODE_ICON) )
	{
		cIcon* icon = (cIcon*)node;
		if( icon->GetIconType() == eICON_NORMAL )
		{
			/// 퀵슬롯 처리
			DragToQuickSlot( DRAG_FROM_QUICKSLOT, icon );
		}
	}
	else
	{
		QUICKMAN->DeleteData();
	}
}

void cDragWindow::OnDragFromTradeWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	cTradeWindow* trade = GAMEUI->GetTradeWindow();
	if( trade )
	{
		if( trade->IsNetLock() == true )
			return;

		if( trade->GetStatus() != ItemExchangeBegin )
			return;

		/// 트레이드 윈도우 영역인지 체크
		if( trade->ContainPoint( pos ) )
			return;

		unsigned short slot = itemIcon->GetSlotIndex();
		if( slot != USHRT_MAX && ITEMMAN->TryToTradeItemDelete( slot, itemIcon->GetID() ) == false )
			return;
	}
}

void cDragWindow::OnDragFromChatWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	/// 등록 삭제
	cChatWindow* chatWin = GAMEUI->GetChatWindow();
	if( chatWin && chatWin->ContainPoint( pos ) == false )
		chatWin->DeleteLinkItem();
}

void cDragWindow::OnDragFromMegaphoneWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	/// 등록 삭제
	cMegaphoneWindow* megaWin = GAMEUI->GetMegaphoneWindow();
	if( megaWin && megaWin->ContainPoint( pos ) == false )
		megaWin->DelLinkItem();
}

void cDragWindow::OnDragFromUserSellWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
	if( !userSell )
		return;

	if( userSell->IsNetLock() == true )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	/// 판매 상태
	if( userSell->GetStatus() == eUSERSELL_SELL )
	{
		/// 판매노점상 내부이면 리턴
		if( userSell->ContainPoint( pos ) == true )
			return;

		/// 영역에 없으면 삭제
		unsigned short slot = userSell->GetSelectInvenSlot();
		if( slot != USHRT_MAX && ITEMMAN->TryToUserSellDelete( slot ) == false )
			return;
	}
	/// 구매상태
	else if( userSell->GetStatus() == eUSERSELL_JOIN )
	{
		if( node->IsKindof( eUINODE_ICON ) )
		{
			cItemIcon* icon = (cItemIcon*)node;
			eItemIn in = ITEMMAN->GetItemIn( icon->GetSlotIndex() );

			/// 구매창 -> 인벤 개인
			if( in == ITEM_IN_INVEN_PRIVATE )
			{
				sUserSellData* data = userSell->GetSelectJoinData();
				sInventory* inv = &data->mThatInven;

				cItemDefine* itemDefine = ITEMMAN->GetItemDefine( inv->ItemIndex );

				if( itemDefine->GetCapacity() > 1 )
					GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_USERSELLBUY, inv->number, inv->ItemIndex, inv->count, data->mPrice );
				else
					GAMEUI->ShowItemUserSellBuyAskWindow( inv->number, inv->ItemIndex, 1, data->mPrice );
			}	
		}
	}
}

void cDragWindow::OnDragFromNpcDealRegistWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	cNpcDealWindow* deal = GAMEUI->GetNpcDealWindow();
	if( deal )
	{
		cNpcDealRegistSheet* regist = deal->GetRegistSheet();
		if( !regist )
			return;

		if( deal->IsNetLock() == true )
			return;

		///
		cUIRect dealRect = deal->GetAbsoluteRect();
		if( dealRect.ContainPoint( pos ) == false )
		{
			regist->DeleteRegistItem();			
		}
	}
}

void cDragWindow::OnDragFromMailReadWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	// 영역 체크
	bool check = false;
	cInventoryWindow* inven = GAMEUI->GetInventoryWindow();
	if( inven && inven->ContainPoint( pos ) )
	{
		check = true;
	}

	if( check == false )
	{
		for( unsigned int i = 0; i < 3; ++i )
		{
			cBagWindow* bag = GAMEUI->GetBagWindow( i );
			if( bag && bag->ContainPoint( pos ) )
			{
				check = true;
				break;
			}
		}
	}

	// 인벤 혹은 가방으로 드래그 하였다면
	if( check == true )
	{
		cMailWindow* mailWin = GAMEUI->GetMailWindow();
		if( mailWin )
		{
			cMailReadSheet* read = mailWin->GetReadSheet();
			if( read )
			{
				read->SendItemDownload( itemIcon->GetID() );
			}
		}		
	}
}

void cDragWindow::OnDragFromMailSendWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIcon = (cItemIcon*)mIconFrom;
	if( itemIcon == 0 )
		return;

	cMailWindow* mailWin = GAMEUI->GetMailWindow();
	if( mailWin )
	{
		cMailSendSheet* sheet = mailWin->GetSendSheet();
		if( sheet )
		{
			if( sheet->ContainPoint( pos ) )
				return;

			sheet->DeleteItem( itemIcon->GetID() );
		}
	}
}

/// 퀵슬롯으로 드래그
void cDragWindow::DragToQuickSlot( eDragFrom dragFrom, cIcon* icon )
{
	if( icon == NULL )
		return;

	cUIContainer* parent = icon->GetParent();
	if( parent == NULL )
		return;

	unsigned int parentID = parent->GetID();

	/// 퀵슬롯 확인검사
	if( parentID != eUIID_COLUMN_QUICKSLOT    && parentID != eUIID_ROW_QUICKSLOT  &&
		parentID != eUIID_COLUMN_EXTRASLOT1   && parentID != eUIID_ROW_EXTRASLOT1 && 
		parentID != eUIID_COLUMN_EXTRASLOT2   && parentID != eUIID_ROW_EXTRASLOT2 &&
		parentID != eUIID_COLUMN_QUICKSLOT_FS && parentID != eUIID_ROW_QUICKSLOT_FS )
		return;

	if( mIconFrom == 0 )
		return;

	/// 놓는 위치에 따른 fs 슬롯 판별
	bool toIsFS = false;
	if( parentID == eUIID_COLUMN_QUICKSLOT_FS || parentID == eUIID_ROW_QUICKSLOT_FS ) 
		toIsFS = true;
	
	/// 몬스터 변신상태일때, 몬스터 스킬 슬롯에 등록 불가!! 
	if( toIsFS && HERO->IsTransformMonster() && icon->GetID() >= QUICKSLOT_ICON_FS_BEGIN && icon->GetID() <= QUICKSLOT_ICON_FS_END )
		return;

	/// 퀵슬롯에 등록
	if( dragFrom == DRAG_FROM_QUICKSLOT )
	{
		QUICKMAN->MoveData( icon->GetID(), mShowIcon, toIsFS );
	}
	else
	{
		QUICKMAN->AddData( icon->GetID(), mIconFrom, toIsFS );
	}
}

/// 인벤토리 -> NPC상점, 인벤토리, 분해창,...
/// 슬롯 대 슬롯으로 비교해야하는경우 사용 ( 반드시 해당 위치 슬롯을 체크하는경우 )
void cDragWindow::DragToItemSlot( cIcon* icon )
{
	if( mIconType != eICON_ITEM )
		return;

	cItemIcon* itemIconFrom = (cItemIcon*)mIconFrom;
	if( itemIconFrom == 0 )
		return;

	cItemIcon* itemIcon = (cItemIcon*)icon;
	unsigned short slotIndex0 = itemIconFrom->GetSlotIndex();
	unsigned short slotIndex1 = itemIcon->GetSlotIndex();

	if( slotIndex0 == slotIndex1 )
		return;

	eItemIn in = ITEMMAN->GetItemIn( slotIndex0 );
	eItemIn out = ITEMMAN->GetItemIn( slotIndex1 );
	if( in == ITEM_IN_NULL || out == ITEM_IN_NULL )
		return;

	cItem* item = ITEMMAN->GetItem( slotIndex0 );
	if( !item )
	{
		assert(0);
		return;
	}

	if( mShiftDown == true && item->IsCash() == false )
	{
		/// 인벤 개인 -> 인벤 개인
		/// 인벤 개인 <-> 창고 개인
		/// 창고 개인 -> 창고 개인
		if( (in == ITEM_IN_INVEN_PRIVATE && in == out) ||
			(in == ITEM_IN_INVEN_PRIVATE && out == ITEM_IN_WAREHOUSE_PRIVATE) ||
			(in == ITEM_IN_WAREHOUSE_PRIVATE && out == ITEM_IN_INVEN_PRIVATE) ||
			(in == ITEM_IN_WAREHOUSE_PRIVATE && in == out ) )
		{
			// 놓아질 위치에 아이템 존재 판단
			/// 분할 + 머지 기능은 없음
			if( itemIcon->GetNumber() > 0 )
				return;

			GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_SPLIT, slotIndex0, slotIndex1 );
			return;
		}
	}
	
	/// 인벤 개인 -> 카드 장착 [아이템] 슬롯
	if( in == ITEM_IN_INVEN_PRIVATE && out == ITEM_IN_PUTCARD_ITEMSLOT )
	{
		/// 아이템 등록
		cPutCardWindow* putcard = GAMEUI->GetPutCardWindow();
		if( putcard )
			putcard->AddItem( slotIndex0 );
	}
	/// 인벤 개인 -> 카드 장착 [카드] 슬롯
	else if( in == ITEM_IN_INVEN_PRIVATE && out == ITEM_IN_PUTCARD_CARDSLOT )
	{
		cPutCardWindow* putcard = GAMEUI->GetPutCardWindow();
		if( putcard )
			putcard->TestAddCard( slotIndex0, slotIndex1 );
	}
	/// 카드북 -> 인벤 개인
	else if( in == ITEM_IN_CARDBOOK_NORMAL && out == ITEM_IN_INVEN_PRIVATE )
	{
		cItem* item1 = ITEMMAN->GetItem( slotIndex1 );
		if( item1 )
		{
			if( item1->GetIndex() > 0 && item1->IsLock() == true )
				return;

			// 빈자리거나, 자리에 존재하는 아이템이 동일 타로카드일때만 가능
			if( item1->GetCount() > 0 )
			{
				if( item->GetIndex() != item1->GetIndex() )
					return;
			}

			GAMEUI->ShowItemCardBook( eMBEVENT_ITEM_CARDBOOK_TAROTRETURN, slotIndex0, slotIndex1, 0 );
		}
		else
		{
			assert(0);
			return;
		}
	}
	else if( out == ITEM_IN_NPCSTORE ||	out == ITEM_IN_PUTCARD_ITEMSLOT )
	{
		/// 도착지점이 잘못되었으면 리턴
		assert(0);
		return;
	}
	else
	{
		///  락 걸린 슬롯이면 리턴
		cItem* item1 = ITEMMAN->GetItem( slotIndex1 );
		if( !item1 )
		{
			assert(0);
			return;
		}

		if( item1->GetIndex() > 0 && item1->IsLock() == true )
			return;

		ITEMMAN->TryToMoveItem( slotIndex0, slotIndex1, itemIconFrom );
	}
	return;
}

void cDragWindow::OnDragFromShortcutWindow( const cUIPos& pos )
{
	cUINode* node = NULL;

	/// 해당 아이콘이 tab키, ctrl키, alt키, space bar 에 해당하는지 검사(이들은 아이콘 영역이 다른것과 다르다.
	cShortcutWindow* pShortcutWinow = GAMEUI->GetShortcutWindow();
	if( pShortcutWinow )
	{
		node = pShortcutWinow->GetSpecialKeyNode( pos );
	}


	/// 크기가 다른 아이콘이 아니면 해당 아이콘 얻어오기
	if( node == NULL )
		node = GetRoot()->GetNode( pos );
	
	if( !node )
	{
		assert( 0 );
		return;
	}

 	if( mIconType != eICON_SHORTCUT || mIconFrom == NULL )
 		return;

	/// 아이콘을 들었던 위치와 동일하다면
	if( mIconFrom->GetID() == node->GetID() )
		return;
	
	/// 출발 지점이 키보드 아이콘이며 목표 지점이 키보드 아이콘이 아닌경우 아이콘 삭제.
	typedef tPointerHashMap< int, void* > sKeyMap;
	sKeyMap *shortcutMap;
	sKeyMap *keyboardMap;

	shortcutMap = SHORTCUTMAN->GetShortcutScriptMap();
	keyboardMap = SHORTCUTMAN->GetKeyboardScriptMap();


	if( keyboardMap->GetAt( mIconFrom->GetID() ) != 0 )
	{
		if( node->GetType() != eUINODE_ICON )
		{
			SHORTCUTMAN->EraseShortcut( mIconFrom->GetID() );
			return;
		}

		cBaseIcon* pIcon = (cBaseIcon*)node;
		if( pIcon->GetIconType() != eICON_SHORTCUT )
		{
			SHORTCUTMAN->EraseShortcut( mIconFrom->GetID());
			return;
		}

		if( keyboardMap->GetAt( pIcon->GetID() ) == 0 )
		{
			SHORTCUTMAN->EraseShortcut( mIconFrom->GetID() );
			return;
		}

	}

	/// 단축키 아이콘 박스에서 키보드 박스로 옴긴거라면
	if( shortcutMap->GetAt( mIconFrom->GetID() ) != 0 && keyboardMap->GetAt( node->GetID() ) != 0 )
	{
		SHORTCUTMAN->SetShortcut( mIconFrom->GetID(), node->GetID() );
	}

	/// 키보드 탭에서 키보드 탭으로 옴긴거라면
	if( keyboardMap->GetAt( mIconFrom->GetID() ) != 0 && keyboardMap->GetAt( node->GetID() ) != 0 )
	{
		SHORTCUTMAN->MoveShortcut( mIconFrom->GetID() , node->GetID()  );
	}
}

//--------------------------------------------------------------
cUnionDragWindow::cUnionDragWindow( eUINodeType type )
: cDragWindow( type )
, mpCaller(0)
, mUnionShowIcon(0)
, mUnionIconFrom(0)
{
	mIconType = eICON_MAX;
}

cUnionDragWindow::~cUnionDragWindow()
{

}

void cUnionDragWindow::Open()
{
}

void cUnionDragWindow::Close()
{
	cUIWindow::Close();
	mpCaller = 0;

	PARTYUNIONMAN->UnLinkIcon();

	mUnionIconFrom = 0;
	mIconType = eICON_MAX;
}

bool cUnionDragWindow::OnCreate( cUINodeProperty* )
{
	mUnionShowIcon = new cUnionIcon;
	if( mUnionShowIcon->CreateBySkinName( "UnionDrag_Icon", this, 1 ) == false )
		return false;

	Hide( false );
	return true;
}


void cUnionDragWindow::Show( cUIWindow* pCaller, cUnionIcon* icon, const cUIPos& pos )
{
	assert( icon );

	if( icon->GetIconType() != eICON_UNION )
	{
		assert(0);
		return;
	}

	mUnionIconFrom = icon;
	mIconType = icon->GetIconType();

	// 정보 복사
	mUnionShowIcon->ClearInfo();
	mUnionShowIcon->SetInfo( icon->GetName(), icon->GetMaxHP(), icon->GetHP(), icon->GetJob(), icon->GetLevel() );
	mUnionShowIcon->Show();

	PARTYUNIONMAN->LinkIcon( mUnionIconFrom );

	mpCaller = pCaller;

	/// 드래그윈도우 위치 선정
	cUIPos newPos( (int)(pos.mX - GetAbsoluteRect().GetWidth() * 0.5), (int)(pos.mY - GetAbsoluteRect().GetHeight() * 0.5) );
	SetRelativePos( newPos );
	//ShowModal( eMBEVENT_NONE );
	cUIWindow::Show();
	SetCapture();
}

/// 외부에서 닫힘.
void cUnionDragWindow::Hide( bool onsound )
{
	cUIWindow::Hide( onsound );
	mpCaller = 0;

	PARTYUNIONMAN->UnLinkIcon();

	mUnionIconFrom = 0;
	mIconType = eICON_MAX;
}

void cUnionDragWindow::Hide( cUIWindow* pCaller )
{
	cDragWindow::Hide( pCaller );
}	

void cUnionDragWindow::OnLButtonUp( const cUIPos& pos )
{
	/// 윈도우 숨기기
	cUIWindow::Hide();
	mpCaller = 0;

	//
	OnDragFromUnionWindow( pos );

	//
	PARTYUNIONMAN->UnLinkIcon();
}

void cUnionDragWindow::OnDragFromUnionWindow( const cUIPos& pos )
{
	cUINode* node = GetRoot()->GetNode( pos );
	if( !node )
		return;

	if( mIconType != eICON_UNION )
		return;

	if( node->IsKindof(eUINODE_ICON) == false )
		return;

	cBaseIcon* baseIcon = (cBaseIcon*)node;
	if( baseIcon->GetIconType() != eICON_UNION )
		return;

	unsigned char i0 = (unsigned char)mUnionIconFrom->GetID();
	unsigned char i1 = (unsigned char)baseIcon->GetID();

	if( PARTYUNIONMAN->TryToMovePlayer( i0, i1 ) == false )
		assert(0);
}

