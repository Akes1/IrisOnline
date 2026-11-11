#include "stdafx.h"
#include "InventoryWindow.h"
#include "UIImage.h"

//#include "../Common/Item_Common.h"
#include "GameUIManager.h"
#include "PushButton.h"
#include "RadioButton.h"
#include "ItemIcon.h"
#include "ItemManager.h"
#include "InvenSheetWindow.h"
#include "InvenEquipWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Gauge.h"
#include "NumberEditBox.h"
#include "BagWindow.h"
#include "UserSellWindow.h"
#include "Npc_Common.h"
#include "NpcDealRegistSheet.h"
#include "NpcDealWindow.h"
#include "TradeWindow.h"
#include "ChatManager.h"
#include "EnhancedWindow.h"
#include "ChatWindow.h"
#include "NpcDisjointWindow.h"
#include "MoneyWindow.h"
#include "PutCardWindow.h"
#include "MailWindow.h"
#include "MailSheet.h"
#include "GameResourceManager.h"
#include "MegaphoneWindow.h"

cInventoryWindow::cInventoryWindow()
: mWeightGauge(0)
, mSelectIcon(0)
, mpMoney(0)
, mShowBit(0)
, mpSelectImage(0)
, mShowSelect( false )
, mpEquipWindow(0)
, mpForcePointEdit(0)
{
	mBagButton[0] = 0;
	mBagButton[1] = 0;
	mBagButton[2] = 0;

	mIsBackup = false;

	mBackupBagBtn[0] = false;
	mBackupBagBtn[1] = false;
	mBackupBagBtn[2] = false;

	::ZeroMemory( mpForceRadio, sizeof(mpForceRadio) );
}

cInventoryWindow::~cInventoryWindow()
{
	SAFE_DELETE( mpSelectImage );
}

void cInventoryWindow::Open()
{
	mShowBit = 0;

	if( mpEquipWindow )
	{
		mpEquipWindow->Open();
	}
	else
		assert(0);

	/// 돈 초기 설정
	SetMoney( HERO->GetMoney() );

	SetBagEndSlot( HERO->GetBagEndSlot() );
}

void cInventoryWindow::Close()
{
	cUIWindow::Close();

	for( int i = 0; i < 3; ++i )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetBagWindow(i);
		if( b )
			b->Hide( false );
	}

	UIMAN->HideTip();
	UIMAN->HideDrag( this );

	mSelectIcon = 0;
	mShowSelect = false;

	/// npc 상점창이 열려있으면 닫기
	if( HERO->IsOpenNpcTalk() == true )
		HERO->CloseNpcTalk( false );
	
	ApplyBackupPos();
}

void cInventoryWindow::Show( bool onsound )
{
	cUIWindow::Show( onsound );

	UpdateUIByInven( true );

	BYTE byte = mShowBit;
	for( int i = 0; i < 3; ++i )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetBagWindow(i);
		if( b && (byte & 1) )
			b->Show();

		byte = byte >> 1;
	}
}

void cInventoryWindow::OnHide()
{
	cUIWindow::OnHide();

	for( int i = 0; i < 3; ++i )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetBagWindow(i);
		if( b )
			b->Hide( false );
	}

	UIMAN->HideTip();
	UIMAN->HideDrag( this );

	mSelectIcon = 0;
	mShowSelect = false;

	ApplyBackupPos();
	
	/// npc 상점창이 열려있으면 닫기
	if( HERO->IsOpenNpcTalk() == true )
	{
		HERO->CloseNpcTalk();
	}	
}

/// ESC 로 인해 닫힐때 가방도 같이닫는다
void cInventoryWindow::HideESC( bool onsound )
{
	cUIWindow::HideESC( onsound );

	for( unsigned int i = 0; i < 3; ++i )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetBagWindow(i);
		if( b )
			b->Hide( false );
	}
}

void cInventoryWindow::SetBagButtonPressed( unsigned int i, bool pressed )
{
	assert( i >= 0 && i <= 2 && "index out of range" );

	/// 허용된 가방만 보이기
	unsigned int   endBagIndex = UINT_MAX;
	unsigned short endBagSlot = HERO->GetBagEndSlot();
	switch( endBagSlot )
	{
	case INVENTORY_BAG0_END:	return;
	case INVENTORY_BAG1_END:	endBagIndex = 0;		break;
	case INVENTORY_BAG2_END:	endBagIndex = 1;		break;
	case INVENTORY_BAG3_END:	endBagIndex = 2;	    break;
	default:
		{
			assert( 0 && "Bad slot index" );
			return;
		}
		break;
	}
	
	if( i > endBagIndex )
		return;


	if( mBagButton[i] )
		mBagButton[i]->SetPress( pressed );

	//특정비트를 1로 지정하는 공식은 a | (a<<(n-1)) 가 되고
	//특정비트를 0으로 지정하는 공식은 (a&~(1<<n-1)) 가 되고
	//원하는 특정 비트를 반대로 하려면 (a^(1<<n-1)) 가 된다.

	BYTE byte = 0;
	BYTE end = 1;
	if( pressed )
	{
		byte = (BYTE)(1 << i);
	}

	end = (BYTE)(1 << i);
	BYTE mask = 0xFF ^ end;

	/// 마스크 씌워서 해당 위치의 비트만 세팅하기
	mShowBit = ( mShowBit & mask ) | byte;
}

bool cInventoryWindow::IsShowBag( unsigned int bagIdx )
{
	BYTE byte = mShowBit;
	byte = (BYTE)(byte >> bagIdx);

	return (byte & 1);
}

bool cInventoryWindow::OnCreate( cUINodeProperty* property )
{
	if( cTabWindow::OnCreate( property ) == false )
		return false;

	mpEquipWindow = (cUIWindow*)GetChild( eUIID_INVENTORY_EQUIPWINDOW );

	/// 탭윈도우 세팅 
	cInvenSheetWindow* p = 0;
	for( unsigned int i = 0; i < MAX_INVENTAB; ++i )
	{
		p = (cInvenSheetWindow*)GetChild( eUIID_INVEN_SHEET1 + i );
		if( p )
		{
			p->BindSlot( i );
			p->Hide();
		}
	}

	SetCurrent( 0 );

	/// 장착 윈도우를 생성
	cUIWindow* pEquipWindow = (cUIWindow*)GetChild( eUIID_INVENTORY_EQUIPWINDOW );
	if( pEquipWindow )
	{
		pEquipWindow->Show( false );
	}
	else
	{
		assert(0);
	}

	/// 무게 게이지
	mWeightGauge = (cGauge*)GetChild( eUIID_INVENTORY_WEIGHT );
	mpMoney = (cNumberEditBox*)GetChild( eUIID_INVENTORY_MONEY );

	if( mWeightGauge )
	{
		ITEMMAN->SetWeightGauge( mWeightGauge );
	}
	else
	{
		assert(0);
	}
	
	/// 가방 버튼
	mBagButton[0] = (cPushButton*)GetChild( eUIID_INVENTORY_BAG0 );
	mBagButton[1] = (cPushButton*)GetChild( eUIID_INVENTORY_BAG1 );
	mBagButton[2] = (cPushButton*)GetChild( eUIID_INVENTORY_BAG2 );

	/// 전장 관련 UI
	mpForceRadio[0] = (cRadioButton*)GetChild( eUIID_INVENTORY_FIRE_RADIO );
	mpForceRadio[1] = (cRadioButton*)GetChild( eUIID_INVENTORY_WATER_RADIO );
	mpForceRadio[2] = (cRadioButton*)GetChild( eUIID_INVENTORY_WIND_RADIO );
	mpForceRadio[3] = (cRadioButton*)GetChild( eUIID_INVENTORY_EARTH_RADIO );

	mpForcePointEdit = (cNumberEditBox*)GetChild( eUIID_INVENTORY_FORCE_POINT );

	InitForcePoint();

	/// 무게게이지 뚜껑이미지
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 선택 이미지 생성
	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "Inventory_SelectImage" );
	if( selectSkin )
	{
		mpSelectImage = new cUIImage( selectSkin->mpTexture );
		unsigned int tx = selectSkin->mSkinInfo->mTexX;
		unsigned int ty = selectSkin->mSkinInfo->mTexY;
		unsigned int w = selectSkin->mSkinInfo->mWidth;
		unsigned int h = selectSkin->mSkinInfo->mHeight;
		unsigned int x = selectSkin->mSkinInfo->mX;
		unsigned int y = selectSkin->mSkinInfo->mY;
		mpSelectImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSelectImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}
	return true;
}

void cInventoryWindow::UpdateRect()
{
	cTabWindow::UpdateRect();

	if( mSelectIcon && mpSelectImage )
	{
		cUIRect rc = mSelectIcon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
	}
}	

void cInventoryWindow::UpdateSkin()
{
	UpdateRect();

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();

	/// 자식 스킨 적용
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n )
			n->UpdateSkin();
	}

	UpdateUIByInven( false );
}

void cInventoryWindow::UpdateUIByInven( bool backup )
{
	cUIWindow* trade = (cUIWindow*)GAMEUI->GetTradeWindow();
	if( trade && trade->IsVisible() )
	{
		GAMEUI->UpdateInvenWithUI( trade, backup );
		return;
	}

	cUIWindow* sell = (cUIWindow*)GAMEUI->GetUserSellWindow();
	if( sell && sell->IsVisible() )
	{
		GAMEUI->UpdateInvenWithUI( sell, backup );
		return;
	}
}

void cInventoryWindow::OnRender(cUIFontItemKeeper* pKeeper )
{
	cTabWindow::OnRender( pKeeper );

	if( mShowSelect && mpSelectImage )
		mpSelectImage->Draw();
}

void cInventoryWindow::OnCommand( cUINode* /*node*/, unsigned int id )
{
	if( id == eUIID_INVENTORY_CLOSE )
		Hide();
}

void cInventoryWindow::OnTabButtonPressed( cUINode* caller, unsigned int id )
{
	cTabWindow::OnTabButtonPressed( caller, id );
	mShowSelect = false;
}

void cInventoryWindow::OnPushButtonPressed( cUINode* /*caller*/, unsigned int id )
{
	switch( id )
	{
	case eUIID_INVENTORY_BAG0:
		{
			cBagWindow* bag = GAMEUI->GetBagWindow( 0 );
			if( bag )
			{
				bag->Toggle();
				SetBagButtonPressed( 0, bag->IsVisible() );
			}
		}
		break;

	case eUIID_INVENTORY_BAG1:
		{
			cBagWindow* bag = GAMEUI->GetBagWindow( 1 );
			if( bag )
			{
				bag->Toggle();
				SetBagButtonPressed( 1, bag->IsVisible() );
			}
		}
		break;

	case eUIID_INVENTORY_BAG2:
		{
			cBagWindow* bag = GAMEUI->GetBagWindow( 2 );
			if( bag )
			{
				bag->Toggle();
				SetBagButtonPressed( 2, bag->IsVisible() );
			}
		}
		break;
	}
}

void cInventoryWindow::OnRadioPressed( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_INVENTORY_FIRE_RADIO:		
		SetForcePoint( eFORCETYPE_FIRE );
		break;
	case eUIID_INVENTORY_WATER_RADIO:
		SetForcePoint( eFORCETYPE_WATER );
		break;
	case eUIID_INVENTORY_WIND_RADIO:	
		SetForcePoint( eFORCETYPE_WIND );
		break;
	case eUIID_INVENTORY_EARTH_RADIO:
		SetForcePoint( eFORCETYPE_EARTH );
		break;
	}
}

///
void cInventoryWindow::OnIconClicked( cUINode* caller, unsigned int )
{
	UpdateSelect( (cItemIcon*)caller );
}

void cInventoryWindow::OnIconDoubleClicked(cUINode* caller, unsigned int )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
	{
		if( HERO->GetNpcFunc() == NPC_FUNC_MAX )
		{
			/// 강화 카드면 강화 시도
			cItem* pItem = ITEMMAN->GetItem( icon->GetSlotIndex() );
			if( pItem )
			{
				if( pItem->GetDefine()->GetType() == ITEM_CARD && 
					pItem->GetDefine()->GetSubType() >= ITEM_CARD_ENHANCE_WEAPON_LV1 && 
					pItem->GetDefine()->GetSubType() <= ITEM_CARD_ENHANCE_WEAR_LV2_75 )
				{
					
					unsigned long textIndex = 0;
					int ret = ITEMMAN->CheckLimit( pItem->GetIndex() );
					switch( ret )
					{
					case ITEMLIMIT_RACE:		textIndex = 280; break;
					case ITEMLIMIT_GENDER:		textIndex = 281; break;
					case ITEMLIMIT_JOB:			textIndex = 282; break;
					case ITEMLIMIT_LEVEL:		textIndex = 283; break;
					case ITEMLIMIT_PVPONLY:		textIndex = 284; break;
					case ITEMLIMIT_TUTORIALONLY:textIndex = 291; break;
					case ITEMLIMIT_FIELDONLY:    textIndex = 12400; break;
					case ITEMLIMIT_FORCETYPE:	 textIndex = 285;   break;
					case ITEMLIMIT_MAKESKILL:	 textIndex = 286;   break;
					case ITEMLIMIT_MAKESKILLEXP: textIndex = 292;   break;
					case ITEMLIMIT_MIXSKILLSTEP: textIndex = 288;   break;
					case ITEMLIMIT_MIXRECIPESTEP:textIndex = 289;   break;
					case ITEMLIMIT_MIXRECIPEEXP: textIndex = 290;   break;
					case ITEMLIMIT_ERROR:		assert(0);		textIndex = 239; break;
					}

					if( ret != ITEMLIMIT_SUCCESS   )
					{
						CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( textIndex ) );
						return;
					}

					/// 아이템 강화창 열기
					cEnhancedWindow* pEnhan = GAMEUI->GetEnhancedWindow();
					if( pEnhan )
					{
						if( pEnhan->IsVisible() == false )
						{
							pEnhan->Show();
							pEnhan->AddCard( icon->GetSlotIndex() );
						}
					}
					/// 인벤창 열기
					cUIWindow* inven = (cUIWindow*)GAMEUI->GetInventoryWindow();
					if( pEnhan->IsVisible() == true )
					{
						if( inven && inven->IsVisible() == false )
							inven->Show(false);
					}
				}
			}

			ITEMMAN->TryToEquipItem( icon->GetSlotIndex() );
		}
	}
}

void cInventoryWindow::OnIconRightClicked(cUINode* caller, unsigned int , bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( !icon )
	{
		assert(0);
		return;
	}
	unsigned short curSlotIdx = icon->GetSlotIndex();

	/// 상점인경우
	switch( HERO->GetNpcFunc() )
	{
	case NPC_FUNC_STORE:
		{
			cItem* item = ITEMMAN->GetItem( curSlotIdx );
			if( item && item->GetCount() > 0 )
			{
				if( shift == true )
				{
					ITEMMAN->TryToSellItem( curSlotIdx, item->GetCount() );
				}
				else
				{
					GAMEUI->ShowItemSellAskWindow( curSlotIdx, item->GetCount() );
				}
			}
		}
		break;
	case NPC_FUNC_DEAL:
		{
			/// 판매대행
			cNpcDealWindow* dealWin = GAMEUI->GetNpcDealWindow();
			if( dealWin )
			{
				cNpcDealRegistSheet* registSheet = dealWin->GetRegistSheet();
				if( registSheet && registSheet == dealWin->GetCurrentSheet() )
				{
					registSheet->RegistItem( curSlotIdx );
				}
			}
		}
		break;
	case NPC_FUNC_WAREHOUSE:
		{
			/// 창고
			unsigned short slotIdx = 0;
			unsigned short wareHouseEnd = HERO->GetWareHouseEndSlot();
			
			if( ITEMMAN->FindEmptySlot( &slotIdx, INVENTORY_WAREHOUSE0_BEGIN, wareHouseEnd ) == true )
			{
				ITEMMAN->TryToMoveItem( curSlotIdx, slotIdx, icon );
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 202 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
			}
		}
		break;
	case NPC_FUNC_ITEMDISJOINT:
		{
			unsigned short curSlotIdx = icon->GetSlotIndex();

			/// 분해
			cNpcDisjointWindow* disWin = GAMEUI->GetNpcDisjointWindow();
			if( disWin )
			{
				disWin->AddItem( curSlotIdx );
			}
		}
		break;
	case NPC_FUNC_MAIL:
		{
			cMailWindow* win = GAMEUI->GetMailWindow();
			cMailSendSheet* sendSheet = win->GetSendSheet();
			if( sendSheet && sendSheet == win->GetCurrentSheet() )
			{
				if( win->IsNetLock() == false )
				{
					sendSheet->AddItem( curSlotIdx );
				}
			}
		}
		break;
	default:
		{
			/// shift + 우클릭 = 아이템 링크
			if( shift == true )
			{
				cMegaphoneWindow* megaWin = GAMEUI->GetMegaphoneWindow();
				if( megaWin && megaWin->IsVisible() )
				{
					megaWin->AddLinkItem( icon->GetSlotIndex(), 0 );
					return;
				}

				cChatWindow* chatWin = GAMEUI->GetChatWindow();
				if( chatWin && chatWin->IsVisible() )
				{
					chatWin->AddLinkItem( icon->GetSlotIndex(), 0 );
					return;
				}
			}
			else
			{
				if( HERO->GetState() == eOBJECT_STATE_STOP )
				{
					if( HERO->GetStopFlag() == eSTOP_OPENSTALL )
					{
						/// 판매 노점
						cUserSellWindow* userSell = GAMEUI->GetUserSellWindow();
						if( userSell && userSell->IsVisible() )
						{
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
									GAMEUI->ShowMoneyWindow( eMONEY_NONE, eMBEVENT_USERSELL_ITEMMONEYADD, curSlotIdx );
								}
							}
							return;
						}
					}
					else if( HERO->GetStopFlag() == eSTOP_SUMMON_PAPER )
					{
						tPointerList<void*>* pManagedList = UIMAN->GetManagedSortUIList();
						if( pManagedList == 0 )
							return;
						if( pManagedList->IsEmpty() == false )
						{
							tPointerList<void*>::cIterator i = pManagedList->Begin();
							tPointerList<void*>::cIterator end = pManagedList->End();
							for( ; i != end; ++i )
							{
								cUINode* node = (cUINode*)(*i);
								if( node == 0 )
									continue;

								int id = node->GetID();
								switch( id )
								{
								case eUIID_MAIL_WINDOW:
									{
										cMailWindow* win = (cMailWindow*)node;
										cMailSendSheet* sendSheet = win->GetSendSheet();
										if( sendSheet && sendSheet == win->GetCurrentSheet() )
										{
											if( win->IsNetLock() == false )
											{
												sendSheet->AddItem( curSlotIdx );
											}
										}
									}
									return;
								case eUIID_WAREHOUSE_WINDOW:
									{
										/// 창고
										unsigned short slotIdx = 0;
										unsigned short wareHouseEnd = HERO->GetWareHouseEndSlot();

										if( ITEMMAN->FindEmptySlot( &slotIdx, INVENTORY_WAREHOUSE0_BEGIN, wareHouseEnd ) == true )
										{
											ITEMMAN->TryToMoveItem( curSlotIdx, slotIdx, icon );
										}
										else
										{
											UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
												eMBEVENT_NONE, eSKIN_OK,
												GAMERESOURCEMAN->GetGameText( 202 ),
												GAMERESOURCEMAN->GetGameText( 301 ) );
										}
									}
									return;
								case eUIID_NPCDEAL_WINDOW:
									{
										/// 판매대행
										cNpcDealWindow* win = (cNpcDealWindow*)node;
										if( win )
										{
											cNpcDealRegistSheet* registSheet = win->GetRegistSheet();
											if( registSheet && registSheet == win->GetCurrentSheet() )
											{
												registSheet->RegistItem( curSlotIdx );
											}
										}
									}
									return;
								}
							}
						}
					}
				}
				else
				{
					tPointerList<void*>* pManagedList = UIMAN->GetManagedSortUIList();
					if( pManagedList == 0 )
						return;
					if( pManagedList->IsEmpty() == false )
					{
						tPointerList<void*>::cIterator i = pManagedList->Begin();
						tPointerList<void*>::cIterator end = pManagedList->End();
						for( ; i != end; ++i )
						{
							cUINode* node = (cUINode*)(*i);
							if( node == 0 )
								continue;

							int id = node->GetID();
							switch( id )
							{
							case eUIID_GAME_ENHANCED_WINDOW:
								{
									cEnhancedWindow* win = (cEnhancedWindow*)node;
									if( win && win->IsVisible() )
									{
										cItem* item = ITEMMAN->GetItem( curSlotIdx );
										if( item )
										{
											cItemDefine* pItemDefine = item->GetDefine();
											if( pItemDefine )
											{
												if( pItemDefine->GetType() == ITEM_CARD && (pItemDefine->GetSubType() >= ITEM_CARD_ENHANCE_WEAPON_LV1 && 
																							pItemDefine->GetSubType() <= ITEM_CARD_ENHANCE_WEAR_LV2_75	)  )
												{
													win->AddCard( curSlotIdx, true );	
												}
												else if( pItemDefine->GetType() == ITEM_MATERIAL && (	pItemDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_30		|| 
																										pItemDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DOWN_60		|| 
																										pItemDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DESTROY_30	||
																										pItemDefine->GetSubType() == ITEM_MATERIAL_ENHANCED_DESTROY_60	) )
												{
													win->AddCashProtectCard( curSlotIdx, true );	
												}
												else
													win->AddItem( curSlotIdx, true );
											}
										}									

									}
								}
								return;
							case eUIID_GAME_PUTCARD_WINDOW:
								{
									cPutCardWindow* win = (cPutCardWindow*)node;
									if( win && win->IsVisible() )
										win->AddItem( curSlotIdx );
								}
								return;
							case eUIID_TRADE_WINDOW:
								{
									/// 개인 거래
									cTradeWindow* trade = GAMEUI->GetTradeWindow();
									if( trade && trade->IsVisible() )
									{
										if( trade->IsNetLock() == false )
										{
											ITEMMAN->TryToTradeItemAdd( curSlotIdx );
										}
									}
								}
								return;
							}
						}
					}
					else
					{
						// 상위에 뜬 관련 윈도우가 없으면 아이템 사용으로 보내기
						ITEMMAN->TryToEquipItem( icon->GetSlotIndex() );
					}
				}
			}
		}
		break;
	}
}

/// 새로운 슬롯으로 선택 갱신
void cInventoryWindow::ChangeSelect( unsigned short slotIndex )
{
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
	{
		cItemIcon* newIcon = item->GetIcon();
		UpdateSelect( newIcon );

		/// 일반 쉬트 아니면 보이지 않게 하기
		cUIWindow* sheet = GetCurrentSheet();
		if( sheet && sheet->GetID() != eUIID_INVEN_SHEET1 )
			mShowSelect = false;
	}
}

/// 눌린아이콘을 선택된 아이콘으로 저장.
void cInventoryWindow::UpdateSelect( cItemIcon* icon )
{
	if( icon == 0 )
	{
		assert(0);
		return;
	}

	unsigned int itemIdex = icon->GetItemIndex();
	if( mpSelectImage && itemIdex > 0 )
	{
		/// 새 선택 아이콘 눌림효과
		cUIRect rc = icon->GetAbsoluteRect();
		mpSelectImage->SetPos( rc.mLeft-2, rc.mTop-2 );
		mSelectIcon = icon;
		mShowSelect = true;

		ITEMMAN->SetInvenSelect( this );
	}
	else
		mShowSelect = false;
}

void cInventoryWindow::SetMoney( unsigned long money )
{
	if( mpMoney )
	{
		mpMoney->SetValue( money );
	}
	else
	{
		assert(0);
	}
}

unsigned long cInventoryWindow::GetMoney()
{
	if( mpMoney )
	{
		return mpMoney->GetValue();
	}
	else
	{
		assert(0);
		return 0;
	}
}


void cInventoryWindow::BackupWindowPos()
{
	cUIRect rt;
	mIsBackup = true;

	/// inven
	rt = GetAbsoluteRect();
	mBackupPos[0].mX = rt.mLeft;
	mBackupPos[0].mY = rt.mTop;

	/// bag
	for( unsigned int i=0; i<3; i++ )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetBagWindow(i);
		if( b )
		{
			rt = b->GetAbsoluteRect();
			mBackupPos[i+1].mX = rt.mLeft;
			mBackupPos[i+1].mY = rt.mTop;
		}
		else
		{
			mBackupPos[i+1].mX = 0;
			mBackupPos[i+1].mY = 0;
		}

		mBackupBagBtn[i] = IsShowBag(i);
	}
}

void cInventoryWindow::ApplyBackupPos()
{
	if( mIsBackup == false )
		return;

	///
	SetRelativePos( mBackupPos[0] );

	///
	for( unsigned int i=0; i<3; i++ )
	{
		cUIWindow* b = (cUIWindow*)GAMEUI->GetBagWindow(i);
		if( b )
			b->SetRelativePos( mBackupPos[i+1] );

		SetBagButtonPressed(i, mBackupBagBtn[i]);
	}

	mIsBackup = false;
}

bool cInventoryWindow::SetBagEndSlot( unsigned short slotIndex )
{
	/// 허용된 가방만 보이기
	int	bagButtonIndex = -1;
	unsigned short bagEnd = HERO->GetBagEndSlot();
	switch( bagEnd )
	{
	case INVENTORY_BAG0_END:    break;
	case INVENTORY_BAG1_END:	bagButtonIndex = 0;		break;
	case INVENTORY_BAG2_END:	bagButtonIndex = 1;		break;
	case INVENTORY_BAG3_END:	bagButtonIndex = 2;	    break;
	default:
		{
			assert( 0 && "Bad slot index" );
			return false;
		}
		break;
	}

	cBagWindow* bagWin[3] = {0,};
	bagWin[0] = GAMEUI->GetBagWindow(0);
	bagWin[1] = GAMEUI->GetBagWindow(1);
	bagWin[2] = GAMEUI->GetBagWindow(2);
	if( !bagWin[0] || !bagWin[1] || !bagWin[2] )
	{
		assert(0);
		return false;
	}

	/// 해당 슬롯 인덱스까지 아이콘 enable 시키기
	for( int idx = 0; idx < 3; ++idx )
	{
		if( idx <= bagButtonIndex )
		{
			mBagButton[idx]->SetEnabled( true );
			mBagButton[idx]->Show();
			bagWin[idx]->SetUseFlag( true );
		}
		else
		{
			mBagButton[idx]->SetEnabled( false );
			mBagButton[idx]->Hide();
			bagWin[idx]->SetUseFlag( false );
		}
	}

	return true;
}

bool cInventoryWindow::SetWearCardSlot( unsigned short slotEnd )
{
	if( mpEquipWindow )
	{
		return ((cInvenEquipWindow*)mpEquipWindow)->SetWearCardSlot( slotEnd );
	}
	else
	{
		assert( 0 );
		return false;
	}
}


/// 전장 
void cInventoryWindow::SetForcePoint( unsigned char force )
{
	if( mpForcePointEdit && HERO )
	{
		unsigned long nPvpForcePoint = HERO->GetPVPPoint( force );

		mpForcePointEdit->SetValue( nPvpForcePoint );
	}
	else
	{
		assert(0);
	}	
}

void cInventoryWindow::UpdateForcePoint()
{
	for( int i = 0; i < 4; i++ )
	{
		if( mpForceRadio[i]->IsPress() == true )
		{
			SetForcePoint( (unsigned char)( eFORCETYPE_FIRE + i) );
			break;
		}		
	}
}

void cInventoryWindow::InitForcePoint()
{
	if( HERO )
	{
		if( mpForcePointEdit )
			mpForcePointEdit->SetValue( 0 );

		sPlayerExrInfo* exrInfo = HERO->GetPlayerExrInfo();
		if( exrInfo )
		{
			if( exrInfo->mForceType > eFORCETYPE_NONE )
			{
				int index = exrInfo->mForceType - eFORCETYPE_FIRE;

				if( index < 0 )
					return; 

				if( mpForceRadio[ index ] )
				{
					mpForceRadio[ index ]->SetPress( true );
					SetForcePoint( exrInfo->mForceType );
				}
				else
				{
					assert(0);
					return;
				}				
			}
			else
			{
				for( int i = 0; i < 4; i++ )
				{
					if( mpForceRadio[ i ] )
						mpForceRadio[ i ]->SetPress(false);
				}

				if( mpForceRadio[ 0 ] )
					mpForceRadio[ 0 ]->SetPress(true);
				
			}
		}
		else
		{
			assert(0);
		}
	}
}
