#include "StdAfx.h"
#include "PutCardWindow.h"
#include "ItemManager.h"
#include "Label.h"
#include "GameUIManager.h"
#include "TipWindow.h"
#include "ItemIcon.h"
#include "UIImage.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "SoundSystem.h"
#include "ChatManager.h"
#include "GameResourceManager.h"


cPutCardWindow::cPutCardWindow()
: mItemSlotIndex( USHRT_MAX )
, mpCount( 0 )
, mItemSlot( 0 )
, mSlotCount( 0 )
{
	::memset( mCardSlot, 0, sizeof(mCardSlot) );
	::memset( mSlotImage, 0, sizeof(mSlotImage) );
	::memset( mSlotImageSkin, 0, sizeof(mSlotImageSkin) );
}

cPutCardWindow::~cPutCardWindow()
{
	for( int i = 0; i < MAX_CARD_SLOT; ++i )
		SAFE_DELETE( mSlotImage[i] );
}

///
void cPutCardWindow::Clear()
{
	mItemSlot->ChangeImage( 0 );
	mItemSlot->SetItemIndex( 0 );
	mItemSlot->SetTipType( eTOOLTIP_NONE );
	
	/// 아이템 락 초기화
	if( mItemSlotIndex <= MAX_INVENTORY )
	{
		cItem* oldItem = ITEMMAN->GetItem( mItemSlotIndex );
		if( oldItem )
		{
			oldItem->SetLock( false );
		}

		mItemSlotIndex = USHRT_MAX;
	}

	for( unsigned int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		mCardSlot[i]->ChangeImage( 0 );
		mCardSlot[i]->SetItemIndex( 0 );
		mCardSlot[i]->SetTipType( eTOOLTIP_NONE );
	}

	/// 카운트 초기화
	SetCount( 0 );
}

void cPutCardWindow::Open()
{
}

void cPutCardWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cPutCardWindow::Show( bool onsound )
{
	if( HERO->GetStopFlag() != eSTOP_ITEMMIX )
		cUIWindow::Show( onsound );
}

void cPutCardWindow::OnShow()
{
	cUIWindow::OnShow();
	GAMEUI->HideItemSetWindow( this );
}

///
void cPutCardWindow::OnHide()
{
	cUIWindow::OnHide();

	Clear();
}

///
bool cPutCardWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpCount = (cLabel*)GetChild( eUIID_GAME_PUTCARD_COUNT );

	/// 아이템 슬롯 생성
	mItemSlot = new cItemIcon;
	if( mItemSlot->CreateBySkinName( "PutCard_ItemSlot", this, 1 ) == false )
		return false;

	mItemSlot->SetUsedFlag( false, false );
	mItemSlot->SetSlotIndex( PUTCARD_ITEM_SLOT );
	mItemSlot->SetLinked( true );		/// 실제 아이템이아니라 인벤으로부터 참조했음을 세팅

	/// 카드장착 슬롯 생성
	cString str;
	unsigned short slotIndex = PUTCARD_CARD_SLOT_BEGIN;
	for( unsigned short i = 0; i < MAX_CARD_SLOT; ++i, ++slotIndex )
	{
		str.Format( "PutCard_Slot%d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		icon->SetLinked( true );
		icon->SetSlotIndex( slotIndex );
		mCardSlot[i] = icon;
	}

	/// 슬롯 타입 이미지 읽기
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	NiTexture* texture = 0;
	for( unsigned int i = 0; i < MAX_CARD_SLOT-1; ++i )
	{
		str.Format( "PutCard_SlotType%d", i );
		cUINodeSkin* pSlotSkin = pSkin->GetNodeSkin( str );
		if( pSlotSkin )
		{
			mTexPos[i].mX = pSlotSkin->mSkinInfo->mTexX;
			mTexPos[i].mY = pSlotSkin->mSkinInfo->mTexY;

			texture = pSlotSkin->mpTexture;
		}
	}

	if( !texture )
	{
		assert(0);
		return false;
	}

	/// 슬롯 이미지 생성
	for( int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		str.Format( "PutCard_SlotPos%d", i );
		mSlotImageSkin[i] = pSkin->GetNodeSkin( str );
		if( mSlotImageSkin[i] && texture )
		{
			mSlotImage[i] = new cUIImage( texture );
			unsigned int x = mSlotImageSkin[i]->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mSlotImageSkin[i]->mSkinInfo->mY + GetAbsoluteRect().mTop;
			unsigned int w = mSlotImageSkin[i]->mSkinInfo->mWidth;
			unsigned int h = mSlotImageSkin[i]->mSkinInfo->mHeight;
			mSlotImage[i]->SetScreenRect( cUIRect(x, y, x+w, y+h) );
		}
	}

	///
	SetCount( 0 );

	if( !mItemSlot )
		return false;

	for( int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		if( !mCardSlot[i] )
			return false;
	}
	return true;
}

void cPutCardWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	for( unsigned int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		if( mSlotImage[i] && mSlotImageSkin[i] )
		{
			unsigned int x = mSlotImageSkin[i]->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mSlotImageSkin[i]->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mSlotImage[i]->SetPos( x, y );
		}
	}
}

void cPutCardWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	for( unsigned int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		if( mSlotImage[i] && mSlotImageSkin[i] )
		{
			unsigned int x = mSlotImageSkin[i]->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mSlotImageSkin[i]->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mSlotImage[i]->SetPos( x, y );
		}
	}
}

void cPutCardWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	for( unsigned int i = 0; i < mSlotCount; ++i )
	{
		if( mSlotImage[i] )
			mSlotImage[i]->Draw();
	}

	/// 자식 렌더링
	cChildList::cIterator mi = mChildList.Begin();
	cChildList::cIterator mend = mChildList.End();

	for( ; mi != mend; ++mi )
	{
		if( ((cUINode*)(*mi))->IsVisible() == true )
		{
			((cUINode*)(*mi))->OnRender(pKeeper);
		}
	}
	pKeeper->DrawAll();
}

///
void cPutCardWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_PUTCARD_CLOSE:
		Hide();
		break;
	}
}

void cPutCardWindow::OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
{
	cItemIcon* hoverIcon = (cItemIcon*)caller;
	
	if( mItemSlotIndex <= MAX_INVENTORY && hoverIcon == mItemSlot )
	{
		cItem* item = ITEMMAN->GetItem( mItemSlotIndex );
		if( item )
		{
			UIMAN->ShowItemTip( eTIP_ITEM, pos, item->GetIndex(), mItemSlotIndex, false );
		}
	}
	else
	{
		for( int i = 0; i < MAX_CARD_SLOT; ++i )
		{
			if( hoverIcon == mCardSlot[i] )
			{
				UIMAN->ShowItemTip( eTIP_ITEM, pos, mCardSlot[i]->GetItemIndex(), USHRT_MAX, false );
				return;
			}
		}
	}
}


/// 카드 삽입 (slot0 : 아이템, slot1 : 도착지점 )
bool cPutCardWindow::TestAddCard( unsigned short slot0, unsigned short slot1 )
{
	if( IsNetLock() == true )
		return false;

	if( slot0 > MAX_INVENTORY )
		return false;
		
	cItem* card = ITEMMAN->GetItem( slot0 );
	if( !card )
	{
		assert(0);
		return false;
	}

	/// 미인증된 아이템이면 장착 불가능
	if( card->IsSeal() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
		return false;
	}

	cItemDefine* cardDefine = card->GetDefine();
	if( !cardDefine )
	{
		assert(0);
		return false;
	}

	if( mItemSlotIndex > MAX_INVENTORY )
		return false;

	cItem* item = ITEMMAN->GetItem( mItemSlotIndex );
	if( !item )
	{
		assert(0);
		return false;
	}

	cItemDefine* itemDefine = item->GetDefine();
	if( !itemDefine )
	{
		assert(0);
		return false;
	}

	/// 장착용 카드 아니면 리턴
	if( cardDefine->GetType() != ITEM_CARD )
		return false;

	/// 혈액형 타입 카드 아니면 리턴
	if( !(cardDefine->GetSubType() == ITEM_CARD_EQUIP_A || 
		cardDefine->GetSubType() == ITEM_CARD_EQUIP_B || 
		cardDefine->GetSubType() == ITEM_CARD_EQUIP_AB ||
		cardDefine->GetSubType() == ITEM_CARD_EQUIP_O ) )
		return false;

	/// 슬롯 형태 세팅
	sItemPutCardSlot* data = itemDefine->GetPutCard();
	if( !data )
		return false;

	if( slot1 < PUTCARD_CARD_SLOT_BEGIN )
	{
		assert(0);
		return false;
	}

	/// 도착지점 정보 보기
	unsigned short i = (unsigned short)(slot1 - PUTCARD_CARD_SLOT_BEGIN);
	short cardType = cardDefine->GetSubType();

	/// 타입비교
	short slotType = data->mSlotType[i];
	if( slotType == ITEM_CARD_SLOT_NONE )
		return false;

	switch( slotType )
	{
	case ITEM_CARD_SLOT_A:
		{
			if( !(cardType == ITEM_CARD_EQUIP_A || cardType == ITEM_CARD_EQUIP_O) )
				return false;
		}
		break;
	case ITEM_CARD_SLOT_B:
		{
			if( !(cardType == ITEM_CARD_EQUIP_B || cardType == ITEM_CARD_EQUIP_O) )
				return false;
		}
		break;
	case ITEM_CARD_SLOT_O:
		{
			if( cardType != ITEM_CARD_EQUIP_O )
				return false;
		}
		break;
	case ITEM_CARD_SLOT_AB:
		break;
	case ITEM_CARD_SLOT_NONE:
		return false;
	default:
		assert(0);
		return false;
	}


	unsigned short putSlot = (unsigned short)(i + MIN_CARD_SLOT);

	/// 슬롯에 이미 카드가 박혀있을때,
	if( mCardSlot[i]->GetItemIndex() > 0 )
		GAMEUI->ShowItemPutCard( eMBEVENT_ITEM_PUTCARD_UPDATE, slot0, putSlot, mCardSlot[i]->GetItemIndex() );
	/// 슬롯에 새로운 카드 장착시,
	else
		GAMEUI->ShowItemPutCard( eMBEVENT_ITEM_PUTCARD_NEW, slot0, putSlot, 0 );
	
	return true;
}


/// 카드장착할 아이템 삽입
bool cPutCardWindow::AddItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
		return false;

	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( !item )
	{
		assert(0);
		return false;
	}

	/// 미인증된 아이템이면 강화 불가능
	if( item->IsSeal() == true )
	{
		CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, GAMERESOURCEMAN->GetGameText( 293 ) );
		return false;
	}

	cItemDefine* define = item->GetDefine();
	if( !define )
	{
		assert(0);
		return false;
	}

	/// 무기, 방어구 타입 아니면 리턴
	if( !(define->GetType() == ITEM_WEAPON || define->GetType() == ITEM_WEAR || define->GetType() == ITEM_ACCESSORY) )
		return false;

	/// 데이터 삽입전 초기화
	for( unsigned int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		mCardSlot[i]->ChangeImage( 0 );
		mCardSlot[i]->SetItemIndex( 0 );
		mCardSlot[i]->SetTipType( eTOOLTIP_NONE );
	}

	/// 슬롯갯수 세팅
	short slotCount = define->GetPutSlotCount();
	if( slotCount == 0 )
		return false;

	SetCount( slotCount );

	/// 슬롯 이미지 세팅
	sItemPutCardSlot* data = define->GetPutCard();
	if( data && slotCount > 0 )
	{
		cUIPos texpos;
		for( int i = 0; i < slotCount; ++i )
		{
			if( !(mSlotImage[i] && mSlotImageSkin[i]) )
			{
				assert(0);
				continue;
			}

			switch( data->mSlotType[i] )
			{
			case ITEM_CARD_SLOT_NONE:	continue;
			case ITEM_CARD_SLOT_A:	texpos = mTexPos[0]; break;
			case ITEM_CARD_SLOT_B:	texpos = mTexPos[1]; break;
			case ITEM_CARD_SLOT_AB:	texpos = mTexPos[2]; break;
			case ITEM_CARD_SLOT_O:	texpos = mTexPos[3]; break;
			}

			unsigned int w = mSlotImageSkin[i]->mSkinInfo->mWidth;
			unsigned int h = mSlotImageSkin[i]->mSkinInfo->mHeight;
			mSlotImage[i]->SetTextureRect( cUIRect(texpos.mX, texpos.mY, texpos.mX + w, texpos.mY + h ) );
		}
	}

	/// 현재 적용된 데이터 세팅 ( 카드장착된 내용 )
	if( item->GetCardIndex1() > 0 )
	{
		mCardSlot[0]->SetItemIndex( item->GetCardIndex1() );
	}
	if( item->GetCardIndex2() > 0 )
	{
		mCardSlot[1]->SetItemIndex( item->GetCardIndex2() );
	}
	if( item->GetCardIndex3() > 0 )
	{
		mCardSlot[2]->SetItemIndex( item->GetCardIndex3() );
	}
	if( item->GetCardIndex4() > 0 )
	{
		mCardSlot[3]->SetItemIndex( item->GetCardIndex4() );
	}
	if( item->GetCardIndex5() > 0 )
	{
		mCardSlot[4]->SetItemIndex( item->GetCardIndex5() );
	}

	/// 이미지 세팅
	for( short i = 0; i < slotCount; ++i )
	{
		cItemDefine* cardDefine = ITEMMAN->GetItemDefine( mCardSlot[i]->GetItemIndex() );
		if( cardDefine )
		{
			mCardSlot[i]->ChangeImage( cardDefine->GetIconParam() );
			mCardSlot[i]->SetTipType( cardDefine->GetTipType() );
		}
	}

	/// 
	if( mItemSlotIndex != slotIndex )
	{
		if( mItemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( mItemSlotIndex );
			if( oldItem )
			{
				oldItem->SetLock( false );
			}
		}

		/// 참조 슬롯 인덱스
		mItemSlotIndex = slotIndex;
		item->SetLock( true );
	}

	/// 아이템 이미지 체인지
	mItemSlot->SetItemIndex( item->GetIndex() );
	mItemSlot->ChangeImage( define->GetIconParam() );
	mItemSlot->SetTipType( define->GetTipType() );

	/// 아이템 추가 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( slotIndex );

	return true;
}

/// 실제 카드 장착
bool cPutCardWindow::AddCard( sInventory& inv )
{
	if( inv.cardSlot1 > 0 )
		mCardSlot[0]->SetItemIndex( inv.cardSlot1 );
	if( inv.cardSlot2 > 0 )
		mCardSlot[1]->SetItemIndex( inv.cardSlot2 );
	if( inv.cardSlot3 > 0 )
		mCardSlot[2]->SetItemIndex( inv.cardSlot3 );
	if( inv.cardSlot4 > 0 )
		mCardSlot[3]->SetItemIndex( inv.cardSlot4 );
	if( inv.cardSlot5 > 0 )
		mCardSlot[4]->SetItemIndex( inv.cardSlot5 );

	/// 이미지 세팅
	for( unsigned int i = 0; i < MAX_CARD_SLOT; ++i )
	{
		cItemDefine* cardDefine = ITEMMAN->GetItemDefine( mCardSlot[i]->GetItemIndex() );
		if( cardDefine )
		{
			mCardSlot[i]->ChangeImage( cardDefine->GetIconParam() );
			mCardSlot[i]->SetTipType( cardDefine->GetTipType() );
		}
	}

	/// 카드 추가 시 사운드 재생
	SOUNDSYS->Play2DSound( 3035 );

	return true;
}

/// 슬롯갯수 세팅
void cPutCardWindow::SetCount( short count )
{
	mSlotCount = count;

	if( mpCount )
	{
		cStringT str;
		str.Format( _T("%d"), count );

		mpCount->SetText( (LPCTSTR)str.Cstr() );
	}
}
