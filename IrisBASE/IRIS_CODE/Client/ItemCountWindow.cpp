#include "stdafx.h"
#include "ItemCountWindow.h"

#include "GameUIManager.h"
#include "ItemManager.h"
#include "NumberEditBox.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "StageManager.h"
#include "NpcStoreWindow.h"
#include "GameResourceManager.h"

cItemCountWindow::cItemCountWindow()
: mMaxValue( 1 )
, mSlotIndex0( USHRT_MAX )
, mSlotIndex1( USHRT_MAX )
, mItemIndex( 0 )
, mNumEdit(0)
, mPrice(0)
{
}

cItemCountWindow::~cItemCountWindow()
{
}

void cItemCountWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mNumEdit )
	{
		mNumEdit->Clear();
		mNumEdit->SetMaxValue( mMaxValue );
		mNumEdit->SetFocus();
	}
}


bool cItemCountWindow::HandleEvent(const cUIEvent& event )
{	
	if( mEnabled == false || mVisible == false )
		return mpParent->HandleEvent( event );

	switch( event.mType )
	{
	case eUIEVENT_LBUTTON_DOWN:
	case eUIEVENT_LBUTTON_UP:
	case eUIEVENT_MOUSE_MOVE:
		{
			cUINode* pNode = this->GetNode( event.mPos );
			if( pNode && (pNode->GetType() == eUINODE_BUTTON || pNode->GetType() == eUINODE_EDITBOX ))
				pNode->HandleEvent( event );
			else
				cUIWindow::HandleEvent( event );
		}
		break;
	case eUIEVENT_COMMAND:
		if( event.mID == eMESSAGE_BUTTON_NO )
		{
			Hide();
		}
		else
			cUIWindow::HandleEvent( event );
		break;
	case eUIEVENT_EDITBOX_ENTER:
	case eUIEVENT_KEY_UP:
	case eUIEVENT_KEY_DOWN:
		{
			if( event.mCode == eKEY_RETURN )
			{
				/// 확인 버튼 입력용 
				OnCommand( 0, eUIID_ITEMCOUNT_OK );
			}
			else if( event.mCode == eKEY_ESCAPE )
			{
				OnCommand( 0, eUIID_ITEMCOUNT_CANCEL );
			}
		}
		break;
	default:
		cUIWindow::HandleEvent( event );
		break;
	}
	return true;
}

/// 슬롯 끼리의 이벤트 
void cItemCountWindow::ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex0, unsigned long slotIndex1 )
{
	cItem* item0 = ITEMMAN->GetItem( (unsigned short)slotIndex0 );
	if( !item0 )
	{
		assert(0);
		return;
	}

	if( item0->GetCount() <= 1 )
		return;
	
	cItem* item1 = ITEMMAN->GetItem( (unsigned short)slotIndex1 );
	if( !item1 )
	{
		assert(0);
		return;
	}

	if( item1->GetCount() && item0->GetIndex() != item1->GetIndex() )
		return;

	mSlotIndex0 = slotIndex0;
	mSlotIndex1 = slotIndex1;

	mMaxValue = item0->GetCount();

	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;

	cUIWindow::ShowModal( eventtype );
}

void cItemCountWindow::ShowModal( eMSGBox_Event eventtype, unsigned long slotIndex, unsigned long itemIndex, unsigned long maxValue, unsigned long price )
{
	mSlotIndex0 = slotIndex;
	mItemIndex = itemIndex;
//	mMaxValue = maxValue;
	mPrice = price;

	unsigned long heroMoney = HERO->GetMoney();
	unsigned long count = heroMoney / price;
	count = ( count > maxValue ) ? maxValue : count;
	mMaxValue = count;

	///유저에게서 사는 경우 무게, 최대개수 제한, 빈공간 체크등을 해야함.
	if( eventtype == eMBEVENT_ITEMCOUNT_USERSELLBUY )
	{
		cItemDefine* itemDefine = ITEMMAN->GetItemDefine( mItemIndex );
		if( itemDefine == NULL )
		{
			assert(0);
			return;
		}

		/// 무게
		unsigned long remainWei = HERO->GetMaxWeight() - (int)ITEMMAN->GetWeight();
		unsigned long weiCount = ( itemDefine->GetWeight() > 0 ) ? remainWei / itemDefine->GetWeight() : remainWei;

		unsigned long mincount = ( count > weiCount ) ? weiCount : count;

		/// max inven 체크 ( 갯수 제한 있을때만 )
		if( itemDefine->GetMaxHaveInven() > 0 )
		{
			sItemCount* p = ITEMMAN->GetItemCount( mItemIndex );
			int haveCnt = ( p ) ? p->total : 0;

			unsigned long maxInvenCnt = itemDefine->GetMaxHaveInven() - haveCnt;
			if( maxInvenCnt >= 0 )
			{
				mincount = ( mincount > maxInvenCnt ) ? maxInvenCnt : mincount;
			}
			else
			{
				assert(0);
				mincount = 0;
			}
		}

		/// 슬롯에 들어갈수 있는 갯수 
		//unsigned long invenCount = ITEMMAN->GetRemainCount( mItemIndex, mincount );
		//mMaxValue = ( invenCount > mincount ) ? mincount : invenCount;

		/// 슬롯 full 검사 
		unsigned short slotIndex = 0;
		ITEMMAN->FindEmptySlot( &slotIndex, INVENTORY_BAG_BEGIN, HERO->GetBagEndSlot() );
		if( slotIndex == 0 )
		{
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_NONE, eSKIN_OK,
				GAMERESOURCEMAN->GetGameText( 202 ),
				GAMERESOURCEMAN->GetGameText( 301 ) );
			return;
		}
	}
	
	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;

	cUIWindow::ShowModal( eventtype );
}


/// 아이템 인덱스와 관련된 이벤트
void cItemCountWindow::ShowModal( eMSGBox_Event eventtype, unsigned long index )
{
	switch( eventtype )
	{
	case eMBEVENT_ITEMCOUNT_BUY:
		{
			mItemIndex = index;
			
			cItemDefine* itemDefine = ITEMMAN->GetItemDefine( mItemIndex );
			if( itemDefine && itemDefine->GetBuyType() != ItemBuyNone )
			{
				mMaxValue = itemDefine->GetCapacity();
				unsigned long money = 0;

				/// 한번에 구입할수 있는 최대 갯수.
				switch( itemDefine->GetBuyType() )
				{
				case ItemBuyMoney:
					money = HERO->GetMoney();						
					break;
				case ItemBuyTarot:
					money = HERO->GetTarotPoint();					
					break;
				case ItemBuyPvp:
					{
						/// 어떤 전장 포인트가 소모되야 하는지는 대화중인 npc의 탭의 selltype() 
						cNpcStoreWindow* win = GAMEUI->GetNpcStore();
						if( win )
						{
							unsigned char npcSellType = win->GetCurrentItemSellType();
							switch( npcSellType )
							{
							case eNPC_ITEM_SELL_TYPE_FIRE:  money = HERO->GetFirePoint(); break;
							case eNPC_ITEM_SELL_TYPE_WATER: money = HERO->GetWaterPoint(); break;
							case eNPC_ITEM_SELL_TYPE_WIND:  money = HERO->GetWindPoint(); break;
							case eNPC_ITEM_SELL_TYPE_EARTH: money = HERO->GetEarthPoint(); break;
							default: assert(0); break;
							}
						}
						else
							assert(0);
					}
					break;
				default:
					{
						/// 코인류 - 코인일때는 GetBuyType() 이 아이템 인덱스임.
						sItemCount* p = ITEMMAN->GetItemCount( itemDefine->GetBuyType() );
						money = ( p ) ? p->bag : 0;
					}
					break; 
				}

				/// 소지금
				unsigned long count = money / itemDefine->GetBuyPrice();

				/// 현재 NPC가 판매 비율이 적용되어 있는지 체크.
				cNpcStoreWindow* npcStoreWin = GAMEUI->GetNpcStore();
				if( npcStoreWin )
				{
					unsigned long sellRate = npcStoreWin->GetCurrentItemPriceRate();
					if( sellRate == ULONG_MAX )
					{	
						//assert(0);
						sellRate = 100;
					}
					count = money / ( itemDefine->GetBuyPrice() * sellRate / 100 ) ;
				}
				
				/// 무게
				unsigned long remainWei = HERO->GetMaxWeight() - (int)ITEMMAN->GetWeight();
				unsigned long weiCount = ( itemDefine->GetWeight() > 0 ) ? remainWei / itemDefine->GetWeight() : remainWei;

				unsigned long mincount = ( count > weiCount ) ? weiCount : count;

				/// max inven 체크 ( 갯수 제한 있을때만 )
				if( itemDefine->GetMaxHaveInven() > 0 )
				{
					sItemCount* p = ITEMMAN->GetItemCount( mItemIndex );
					int haveCnt = ( p ) ? p->total : 0;

					unsigned long maxInvenCnt = itemDefine->GetMaxHaveInven() - haveCnt;
					if( maxInvenCnt >= 0 )
					{
						mincount = ( mincount > maxInvenCnt ) ? maxInvenCnt : mincount;
					}
					else
					{
						assert(0);
						mincount = 0;
					}
				}

				/// 슬롯에 들어갈수 있는 갯수 
				unsigned long invenCount = ITEMMAN->GetRemainCount( mItemIndex, mincount );
				mMaxValue = ( invenCount > mincount ) ? mincount : invenCount;
			}
			else
			{
				assert(0);
				return;
			}
		}
		break;
	case eMBEVENT_ITEMCOUNT_SELL:
		{
			mSlotIndex0 = (unsigned short)index;

			cItem* item = ITEMMAN->GetItem( (unsigned short)mSlotIndex0 );
			if( item )
			{
				mMaxValue = item->GetCount();
			}
		}
		break;
	default:
		assert( 0 && "invalid item try" );
		return;
	}

	mMsgHandler = STAGEMAN->GetStage( STAGEMAN->GetCurrentStage() );
	mEventType = eventtype;

	cUIWindow::ShowModal( eventtype );
}

bool cItemCountWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mNumEdit = (cNumberEditBox*)GetChild( eUIID_ITEMCOUNT_NUMBER );
	if( mNumEdit )
		mNumEdit->SetUseZero( false );
	return true;
}

void cItemCountWindow::OnCommand( cUINode*, unsigned int id )
{
	if( !mNumEdit )
	{
		assert(0);
		return;
	}

	switch( id )
	{
	case eUIID_ITEMCOUNT_UP:
		{
			unsigned long value = mNumEdit->GetValue();

			value++;
			if( value > mMaxValue )
				value = mMaxValue;

			mNumEdit->SetValue( value );
		}
		break;
	case eUIID_ITEMCOUNT_DOWN:
		{
			unsigned long value = mNumEdit->GetValue();

			value--;
			if( value < 1 )
				value = 1;

			mNumEdit->SetValue( value );
		}
		break;
	case eUIID_ITEMCOUNT_MAX:
		{
			mNumEdit->SetValue( mMaxValue );
		}
		break;
	case eUIID_ITEMCOUNT_OK:
		{
			unsigned long value = mNumEdit->GetValue();
			if( value > 0 )
			{
				if( value >= mMaxValue )
					value = mMaxValue;

				mNumEdit->SetValue( value );

				if( mMsgHandler )
					mMsgHandler->MsgBoxEventParser( mEventType, id, 0 );

				Hide();
			}
		}
		break;
	case eUIID_ITEMCOUNT_CLOSE:
	case eUIID_ITEMCOUNT_CANCEL:
		Hide();
		break;
	default:
		assert(0);
		break;
	}
}

void cItemCountWindow::OnButtonDowned( cUINode* , unsigned int id )
{
	if( !mNumEdit )
	{
		assert(0);
		return;
	}

	switch( id )
	{
	case eUIID_ITEMCOUNT_UP:
		{
			unsigned long value = mNumEdit->GetValue();

			value++;
			if( value > mMaxValue )
				value = mMaxValue;

			mNumEdit->SetValue( value );
		}
		break;
	case eUIID_ITEMCOUNT_DOWN:
		{
			int value = mNumEdit->GetValue();

			value--;
			if( value < 1 )
				value = 1;

			mNumEdit->SetValue( value );
		}
		break;
	}
}

unsigned short cItemCountWindow::GetCount()
{
	return (mNumEdit)? (unsigned short)mNumEdit->GetValue() : 0;
}
