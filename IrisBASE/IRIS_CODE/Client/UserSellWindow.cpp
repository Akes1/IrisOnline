#include "stdafx.h"
#include "UserSellWindow.h"
#include "GameUIManager.h"
#include "UIManager.h"

#include "EditBox.h"
#include "Button.h"
#include "NumberEditBox.h"
#include "FontAgent.h"
#include "DragWindow.h"
#include "PlaneObject.h"
#include "TipWindow.h"
#include "SoundSystem.h"
#include "GameResourceManager.h"
#include "FilterManager.h"
#include "ChatManager.h"


cUserSellWindow::cUserSellWindow()
: mpName(0)
, mNameSkin(0)
, mPriceSkin(0)
, mImageSkin(0)
, mpNameRegist(0)
, mpItemUpdate(0)
, mpEnd(0)
, mpJoinEnd(0)
, mLastSlot(0)
, mArrayIndex(0)
, mUserSellOpen( false )
, mpSelectImg(0)
, mTargetIndex(0)
, mStatus(eUSERSELL_NONE)
, mShowSelect( false )
{
}

cUserSellWindow::~cUserSellWindow()
{
	SAFE_DELETE( mpSelectImg );
}

/// 클리어
void cUserSellWindow::ClearSellData( sUserSellData* data, unsigned int index )
{
	sUserSellData* p = data + index;
	p->mSlotIndex = USHRT_MAX;
	p->mPrice = 0;
	p->mStatus = StallSellItemNone;
	p->mColor = mDefaultColor;
	p->mItemName.Clear();
	p->mPriceStr.Clear();

	cItemIcon* icon = p->mIcon;
	if( icon )
	{
		icon->ChangeImage( 0 );
		icon->SetTipType( eTOOLTIP_NONE );
		icon->SetItemIndex( 0 );
		icon->SetNumber( 0 );
		icon->ShowNumber( false );
		icon->SetSeal( false );
	}
}

///
void cUserSellWindow::Clear()
{
	if( mpName )
		mpName->Clear();

	/// 아이콘 클리어
	if( mStatus == eUSERSELL_SELL )
	{
		if( mpName )
			mpName->SetEnabled( true );
	}
	else
	{
		if( mpName )
			mpName->SetEnabled( false );
	}

	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i )
	{
		/// 판매 상점
		sUserSellData& data = mSellData[i];

		/// 락 풀기
		if( data.mSlotIndex <= MAX_INVENTORY )
		{
			cItem* invItem = ITEMMAN->GetItem( data.mSlotIndex );
			if( invItem )
				invItem->SetLock( false );
		}

		///
		ClearSellData( mSellData, i );
	}

	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i )
	{
		sUserSellData& data = mJoinData[i];

		/// 인벤정보 초기화
		sInventory& inv = data.mThatInven;
		inv.idx			= 0;
		inv.ItemIndex	= 0;
		inv.number		= 0;
		inv.count		= 0;
		inv.enhanced	= 0;
		inv.seal		= 0;

		ClearSellData( mJoinData, i );
	}

	mLastSlot = 0;
	mArrayIndex = 0;
	mTargetIndex = 0;
	mUserSellOpen = false;
	mShowSelect = false;
	mLockTry = eLOCK_NONE;

	SetStatus( eUSERSELL_NONE );
}

void cUserSellWindow::Open()
{
}

void cUserSellWindow::Close()
{
	cUIWindow::Close();

	Clear();
	UIMAN->HideTip();
}

void cUserSellWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpName && mStatus == eUSERSELL_SELL )
		mpName->SetFocus();
}

///
void cUserSellWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mStatus != eUSERSELL_NONE )
	{
		// 닫을때 인벤도 닫기
		cUIWindow* win = (cUIWindow*)GAMEUI->GetInventoryWindow();
		if( win )
			win->Hide();

		UIMAN->RestoreShow();
	}

	/// 
	Clear();
	UIMAN->HideTip();
}

void cUserSellWindow::HideESC( bool onsound )
{
	if( mUserSellOpen && mStatus == eUSERSELL_SELL )
	{
		// 판매상태일때는 적용하지 않는다
	}
	else if( mStatus == eUSERSELL_JOIN )
	{
		ITEMMAN->TryToUserSellJoinClose();
	}
	else	/// 아직 오픈하지 않았으면
	{
		cUIWindow::HideESC( onsound );
	}
}

///
bool cUserSellWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpName = (cEditBox*)GetChild( eUIID_GAME_USERSELL_NAME );
	mpNameRegist = (cButton*)GetChild( eUIID_GAME_USERSELL_NAMEREGIST );
	mpItemUpdate = (cButton*)GetChild( eUIID_GAME_USERSELL_ITEMUPDATE );
	mpEnd		 = (cButton*)GetChild( eUIID_GAME_USERSELL_END );
	mpJoinEnd	 = (cButton*)GetChild( eUIID_GAME_USERSELL_JOINEND );

	cString str;
	/// 판매상태 아이콘 생성
	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i )
	{
		str.Format( "UserSell_SellSlot%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return false;

		icon->SetLinked( true );
		mSellData[i].mIcon = icon;
	}

	/// 구매상태 아이콘 생성
	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i )
	{
		str.Format( "UserSell_JoinSlot%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, i + 100 ) == false )
			return false;

		/// hover 이벤트는 처리하지만 이동 불가 
		icon->SetLinked( true );
		icon->Hide();
		mJoinData[i].mIcon = icon;
	}

	/// 출력위치 저장
	cUISkin* pSkin = UIMAN->GetSkin();
	if( pSkin )
	{
		mNameSkin = pSkin->GetNodeSkin( "UserSell_ItemName" );
		if( !mNameSkin )
			return false;

		mPriceSkin = pSkin->GetNodeSkin( "UserSell_ItemPrice" );
		if( !mPriceSkin )
			return false;

		mImageSkin = pSkin->GetNodeSkin( "UserSell_Select" );
		if( !mImageSkin )
			return false;
	}

	/// 선택용 이미지 생성
	if( mImageSkin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)mImageSkin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)mImageSkin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)mImageSkin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)mImageSkin->mSkinInfo->mTexHeight;

		mpSelectImg = new cPlaneObject;

		if( mpSelectImg->Create( mImageSkin->mpTexture, 0, 0, tw, th,
			tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create select image");
			return false;
		}
	}

	/// 판매 영역
	if( pSkin )
	{
		cString str;
		cUINodeSkin* nodeSkin = 0;
		for( int i = 0; i < MAX_STALL_ITEM; ++i )
		{
			str.Format( "UserSellRect%02d", i );
			nodeSkin = pSkin->GetNodeSkin( str.Cstr() );

			int left = nodeSkin->mSkinInfo->mX;
			int right = nodeSkin->mSkinInfo->mY;
			unsigned int width = nodeSkin->mSkinInfo->mWidth;
			unsigned int height = nodeSkin->mSkinInfo->mHeight;

			mRect[i].Set( left, right, left + width, right + height );
		}
	}

	/// 기본 세팅
	Clear();
	SetStatus( eUSERSELL_NONE );
	return true;
}

void cUserSellWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	UIMAN->AddManagedSortUI( this );

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}

	/// 출력 
	int posX = 0;
	int posY = 0;

	sUserSellData* data = 0;		
	if( mStatus == eUSERSELL_SELL )
		data = mSellData;
	else
		data = mJoinData;
	
	cStringT priceStr;
	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i, ++data )
	{
		cItemIcon* icon = data->mIcon;
		if( !icon )
			continue;

		cUIRect rc = icon->GetAbsoluteRect();
		
		/// 아이템 이름 출력
		if( data->mItemName.IsEmpty() == false )
		{
			posX = rc.mRight + mNameSkin->mSkinInfo->mX;
			posY = rc.mTop + mNameSkin->mSkinInfo->mY;
			pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(data->mItemName.Cstr()), posX, posY, data->mColor );
		}
	
		/// 아이템 가격 출력
		if( data->mPriceStr.IsEmpty() == false )
		{
			if( data->mStatus == StallSellItemPause )
			{
				/// 수정중임 표시
				cStringT str = UIMAN->GetUIText( 353 );
				unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)str.Cstr(), str.GetSize() );
				posX = rc.mRight + mPriceSkin->mSkinInfo->mX + mPriceSkin->mSkinInfo->mWidth - width;
				posY = rc.mTop + mPriceSkin->mSkinInfo->mY;
				pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(str.Cstr()), posX, posY, mDefaultColor );
			}
			else
			{

				int textIdx  = ( icon->IsShowNumber() == true ) ? 352 : 354;
				priceStr.Format( UIMAN->GetUIText( textIdx ), (LPCTSTR)data->mPriceStr.Cstr() );

				unsigned int width = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPCTSTR)priceStr.Cstr(), priceStr.GetSize() );
				posX = rc.mRight + mPriceSkin->mSkinInfo->mX + mPriceSkin->mSkinInfo->mWidth - width;
				posY = rc.mTop + mPriceSkin->mSkinInfo->mY;
				pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(priceStr.Cstr()), posX, posY, mDefaultColor );
			}
		}
	}

	if( mShowSelect && mpSelectImg )
		mpSelectImg->Draw();

	pKeeper->DrawAll();
}

///
void cUserSellWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_USERSELL_CLOSE:
		if( mUserSellOpen && mStatus == eUSERSELL_SELL )
			ITEMMAN->TryToUserSellClose();
		else if( mStatus == eUSERSELL_JOIN )
			ITEMMAN->TryToUserSellJoinClose();
		else	/// 아직 오픈하지 않았으면
			Hide();
		break;
	case eUIID_GAME_USERSELL_NAMEREGIST:	/// 노점명 등록
		if( mStatus == eUSERSELL_SELL )
		{
			if( mpName )
			{
				LPCTSTR pName = mpName->GetText();
				if( pName == NULL )
					return;

				/// filtering
				if( FILTERMAN->CheckWholeMatch( pName ) == true )
				{
					/// 에러메시지 출력
					cStringT errorMsg;
					errorMsg.Format( GAMERESOURCEMAN->GetGameText( 2517 ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
					return;
				}

				/// filtering
				if( FILTERMAN->CheckPartMatch( pName ) == true )
				{
					/// 에러메시지 출력
					cStringT errorMsg;
					errorMsg.Format( GAMERESOURCEMAN->GetGameText( 2517 ) );
					CHATMANAGER->AddSystemMsg( eSYSTEM_NORMAL, (LPCTSTR)errorMsg.Cstr() );
					return;
				}

				ITEMMAN->TryToUserSellReName( mpName->GetText() );
			}
		}
		break;
	case eUIID_GAME_USERSELL_ITEMUPDATE:	/// 아이템 수정
		if( mStatus == eUSERSELL_SELL )
		{
			unsigned short invenSlot = GetSelectInvenSlot();
			if( invenSlot != USHRT_MAX )
				ITEMMAN->TryToUserSellModify( invenSlot );
		}
		break;
	case eUIID_GAME_USERSELL_END:			/// 노점 종료
		if( mUserSellOpen && mStatus == eUSERSELL_SELL )
			ITEMMAN->TryToUserSellClose();
		else if( mStatus == eUSERSELL_SELL )
			Hide();
		break;
	case eUIID_GAME_USERSELL_JOINEND:		/// 이용 종료
		if( mStatus == eUSERSELL_JOIN )
			ITEMMAN->TryToUserSellJoinClose();
		break;
	}
}

void cUserSellWindow::OnIconClicked( cUINode* caller, unsigned int /*id*/ )
{
	cItemIcon* hoverIcon = (cItemIcon*)caller; 
	sUserSellData* data = ( mStatus == eUSERSELL_SELL ) ? mSellData : mJoinData;
	for( unsigned short i = 0; i < MAX_STALL_ITEM; ++i, ++data )
	{
		if( hoverIcon == data->mIcon )
		{
			UpdateSelect( i );
			break;	
		}
	}
}

void cUserSellWindow::OnIconDoubleClicked( cUINode* caller , unsigned int )
{
	cItemIcon* hoverIcon = (cItemIcon*)caller; 
	sUserSellData* data = ( mStatus == eUSERSELL_SELL ) ? mSellData : mJoinData;

	int select = -1;
	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i, ++data )
	{
		if( hoverIcon == data->mIcon )
		{
			select = i;
			break;	
		}
	}

	if( select > -1 )
		EventSelectItem( select );
}

void cUserSellWindow::OnIconRightClicked( cUINode* caller, unsigned int , bool )
{
	cItemIcon* hoverIcon = (cItemIcon*)caller; 
	sUserSellData* data = ( mStatus == eUSERSELL_SELL ) ? mSellData : mJoinData;

	int select = -1;
	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i, ++data )
	{
		if( hoverIcon == data->mIcon )
		{
			select = i;
			break;	
		}
	}

	if( select > -1 )
		EventSelectItem( select );
}

void cUserSellWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_USERSELL, icon, pos, shift );
}

void cUserSellWindow::OnIconHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
{
	cItemIcon* hoverIcon = (cItemIcon*)caller;
	sUserSellData* data = ( mStatus == eUSERSELL_SELL ) ? mSellData : mJoinData;
	for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i, ++data )
	{
		if( hoverIcon == data->mIcon )
		{
			if( mStatus == eUSERSELL_SELL )
			{
				if( data->mSlotIndex > MAX_INVENTORY )
					continue;

				cItem* item = ITEMMAN->GetItem( data->mSlotIndex );
				if( item )
				{
					UIMAN->ShowItemTip( eTIP_ITEM, pos, item->GetIndex(), data->mSlotIndex, false );
				}
			}
			else
			{
				UIMAN->ShowOtherItemTip( eTIP_ITEM, pos, data->mThatInven );
			}
			break;
		}
	}
}

void cUserSellWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mArrayIndex >= 0 && mpSelectImg )
	{
		mpSelectImg->SetScreenRect( mRect[mArrayIndex].mLeft + GetAbsoluteRect().mLeft, mRect[mArrayIndex].mTop + GetAbsoluteRect().mTop, 
			mRect[mArrayIndex].GetWidth(), mRect[mArrayIndex].GetHeight() );
	}
}

void cUserSellWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mVisible )
	{
		GAMEUI->UpdateInvenWithUI( (cUIWindow*)this, false );
	}
}

void cUserSellWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );

	/// 찾기 
	int select = FindSlot( pos );

	if( select >= 0 )
		UpdateSelect( select );
}

void cUserSellWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUIWindow::OnLButtonDoubleClick( pos );

	/// 찾기 
	int select = FindSlot( pos );
	if( select > -1 )
		EventSelectItem( select );
}

void cUserSellWindow::OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnRButtonDown( pos, ctrl, alt, shift );

	/// 찾기 
	int select = FindSlot( pos );
	if( select > -1 )
		EventSelectItem( select );
}

/// 구입, 수정 처리
void cUserSellWindow::EventSelectItem( int select )
{
	if( mStatus == eUSERSELL_SELL )
	{
		/// 등록된 아이템 수정
		if( mSellData[select].mSlotIndex <= MAX_INVENTORY )
		{	
			UpdateSelect( select );

			unsigned short invenSlot = GetSelectInvenSlot();
			if( invenSlot != USHRT_MAX )
				ITEMMAN->TryToUserSellModify( invenSlot );
		}
	}
	else 
	{
		/// 구입자 상태이면
		if( mJoinData[select].mThatInven.number > 0 )
		{
			UpdateSelect( select );

			/// 수정중이면 구입불가
			if( mJoinData[select].mStatus == StallSellItemPause )
				return;

			/// 구입요청
			unsigned int itemIndex = mJoinData[select].mThatInven.ItemIndex;
			unsigned short slotIndex = mJoinData[select].mThatInven.number;
			unsigned short maxCount = mJoinData[select].mThatInven.count;

			cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
			if( define )
			{
				if( define->GetCapacity() > 1 )
					GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_USERSELLBUY, slotIndex, itemIndex, 
												 maxCount, mJoinData[select].mPrice );
				else
					GAMEUI->ShowItemUserSellBuyAskWindow( mJoinData[select].mThatInven.number, 
														  itemIndex, 1, mJoinData[select].mPrice );
			}
			else
			{
				assert(0);
			}
		}
	}
}

/// 선택된 아이템 리턴
int cUserSellWindow::FindSlot( const cUIPos& pos )
{
	int select = -1;
	for( int i = 0; i < MAX_STALL_ITEM; ++i )
	{
		cUIRect rc = mRect[i];

		rc.mLeft += GetAbsoluteRect().mLeft;
		rc.mTop += GetAbsoluteRect().mTop;
		rc.mRight = rc.mLeft + mRect[i].GetWidth();
		rc.mBottom = rc.mTop + mRect[i].GetHeight();

		if( rc.ContainPoint( pos ) == true )
		{
			select = i;
			break;
		}
	}

	return select;
}

void cUserSellWindow::UpdateSelect( int i )
{
	if( mStatus == eUSERSELL_SELL )
	{
		/// 데이터가 없으면 리턴
		if( mSellData[i].mSlotIndex == USHRT_MAX )
		{
			mShowSelect = false;

			if( mpItemUpdate )
				mpItemUpdate->SetEnabled( false );
			return;
		}
	}
	else if( mStatus == eUSERSELL_JOIN )
	{
		/// 
		if( mJoinData[i].mThatInven.ItemIndex == 0 )
		{
			mShowSelect = false;

			if( mpItemUpdate )
				mpItemUpdate->SetEnabled( false );
			return;
		}
	}
	else
	{
		assert(0);
		return;
	}

	if( mpSelectImg )
	{
		mShowSelect = true;
		mpSelectImg->SetScreenRect( mRect[i].mLeft + GetAbsoluteRect().mLeft, mRect[i].mTop + GetAbsoluteRect().mTop, 
									mRect[i].GetWidth(), mRect[i].GetHeight() );

		if( mpItemUpdate )
			mpItemUpdate->SetEnabled( true );
	}

	mArrayIndex = i;
}

/// 참조한 인벤 슬롯 정보 획득
unsigned short cUserSellWindow::GetSelectInvenSlot()
{
	if( mStatus == eUSERSELL_JOIN )
		return USHRT_MAX;

	if( mArrayIndex >= mLastSlot )
		return USHRT_MAX;

	return mSellData[mArrayIndex].mSlotIndex;
}

/// 해당 아이템이 이미 등록되어있는지 여부
int cUserSellWindow::IsRegistSellSlot( unsigned int slot )
{
	sUserSellData* data = mSellData;
	for( unsigned int i = 0; i < mLastSlot; ++i, ++data )
	{
		if( data->mSlotIndex == slot )
			return i;
	}
	return -1;
}

/// 해당 아이템이 이미 등록되어있는지 여부
int cUserSellWindow::IsRegistJoinSlot( unsigned int slot )
{
	sUserSellData* data = mJoinData;
	for( unsigned int i = 0; i < mLastSlot; ++i, ++data )
	{
		sInventory* inv = &data->mThatInven;
        if( inv->number == slot )
			return i;
	}
	return -1;
}

sUserSellData* cUserSellWindow::GetSelectSellData()
{
	return &mSellData[mArrayIndex];
}

sUserSellData* cUserSellWindow::GetSelectJoinData()
{
	return &mJoinData[mArrayIndex];
}

void cUserSellWindow::SetOpen()
{
	mUserSellOpen = true;
}

/// 상태 세팅
void cUserSellWindow::SetStatus( unsigned char status )
{
	mStatus = status;

	mpName->SetEnabled( false ); 
	mpNameRegist->Hide();
	mpItemUpdate->Hide();
	mpEnd->Hide();
	mpJoinEnd->Hide();

	/// 구매자 상태이면
	if( status == eUSERSELL_JOIN )
	{
		mpName->SetEnabled( false );
		mpJoinEnd->Show();

		///
		for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i )
		{
			sUserSellData& sellData = mSellData[i];
			if( sellData.mIcon )
				sellData.mIcon->Hide();
			
			sUserSellData& joinData = mJoinData[i];
			if( joinData.mIcon )
				joinData.mIcon->Show();
		}
	}
	/// 판매자 상태
	else if( status == eUSERSELL_SELL )
	{
		mpName->SetEnabled( true );
		mpNameRegist->Show();
		mpItemUpdate->Show();
		mpEnd->Show();

		mpItemUpdate->SetEnabled( false );

		///
		for( unsigned int i = 0; i < MAX_STALL_ITEM; ++i )
		{
			sUserSellData& joinData = mJoinData[i];
			if( joinData.mIcon )
				joinData.mIcon->Hide();

			sUserSellData& sellData = mSellData[i];
			if( sellData.mIcon )
				sellData.mIcon->Show();
		}
	}
}


bool cUserSellWindow::AddSellItem( unsigned short slot, unsigned long money )
{
	if( slot > MAX_INVENTORY )
		return false;

	/// 아이템 등록
	cItem* item = ITEMMAN->GetItem( slot );
	if( !item )
	{
		assert(0);
		return false;
	}

	cItemDefine* define = item->GetDefine();
	if( !define )
	{
		assert(0);
		return false;
	}

	/// 해당 슬롯을 가진 아이템이 이미 등록되어있으면 가격만 변경한다
	int sellSlot = IsRegistSellSlot( slot );
	if( sellSlot > -1 )
	{
		mSellData[sellSlot].mPrice = money;
		mSellData[sellSlot].mPriceStr = cNumberEditBox::FormatNumber( money );
		return true;
	}

	if( mLastSlot >= MAX_STALL_ITEM )
	{
		assert(0);
		return false;
	}

	/// 락걸기 - 인벤토리 내, 참조된 아이템 
	item->SetLock( true );

	/// 컬러 설정
	unsigned long itemColor = COLOR_WHITE;
	ITEMMAN->GetTooltipColor( define->GetTipType(), itemColor );

	/// 이름 설정
	bool ret = true;

	TCHAR name[ITENNAME_LEN] = {0,};
	if( item->GetEnhanceStep() > 0 )
	{
		TCHAR temp[ITENNAME_LEN] = {0,};
		::_stprintf_s( temp, _T("+%d %s"), item->GetEnhanceStep(), define->GetName() );

		if ( mNameSkin )
			ret = UIMAN->ModifyText( temp, name, ITENNAME_LEN, mNameSkin->mSkinInfo->mWidth );

		mSellData[mLastSlot].mItemName = ( ret == true ) ? name : temp;
	}
	else
	{
		if ( mNameSkin )
			ret = UIMAN->ModifyText( define->GetName(), name, ITENNAME_LEN, mNameSkin->mSkinInfo->mWidth );

		mSellData[mLastSlot].mItemName = ( ret == true ) ? name : define->GetName();
	}
	 
	/// 참조한 슬롯인덱스 저장
	mSellData[mLastSlot].mSlotIndex = slot;
	mSellData[mLastSlot].mPriceStr = cNumberEditBox::FormatNumber( money );
	mSellData[mLastSlot].mPrice = money;
	mSellData[mLastSlot].mColor = itemColor;
	
	/// 판매할 아이템 이미지 세팅
	cItemIcon* icon = mSellData[mLastSlot].mIcon;
	if( icon )
	{
		int ret0 = ITEMMAN->CheckLimit( item->GetIndex() );
		int ret1 = ITEMMAN->CheckLimitEtc( item->GetIndex() );

		NiColorA color = NiColorA::WHITE;
		if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
		{
			color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
		}

		icon->SetItemIndex( item->GetIndex() );
		icon->ChangeImage( define->GetIconParam() );
		icon->SetTipType( define->GetTipType() );
		icon->SetNumber( item->GetCount() );
		icon->SetSeal( item->IsSeal() );
		icon->SetColor( color );
		

		bool show = ( define->GetCapacity() > 1 ) ? true : false;
		icon->ShowNumber( show );
	}
	
	mLastSlot++;

	UpdateSelect( mArrayIndex );

	/// 아이템 등록 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( slot );

	return true;
}

bool cUserSellWindow::DelSellItem( unsigned int invenSlot, bool onSound )
{
	UIMAN->HideTip();

	int sellSlot = IsRegistSellSlot( invenSlot );
	
	/// 락걸린 아이템 락풀기
	if( mSellData[sellSlot].mSlotIndex <= MAX_INVENTORY )
	{
		cItem* invItem = ITEMMAN->GetItem( mSellData[sellSlot].mSlotIndex );
		if( invItem )
			invItem->SetLock( false );
	}

	/// 슬롯에서 삭제
	ClearSellData( mSellData, sellSlot );

	UpdateSelect( sellSlot );

	/// 업데이트
	UpdateData( mSellData, sellSlot, true );

	if( mLastSlot > 0 )
		mLastSlot--;

	/// 현재 수정중인 아이템은 못 판다는 사실이 있음.
	/// 현재 선택중인 아이템 이전의 것이 삭제되면 땡기기
	if( sellSlot <= (int)mArrayIndex && mArrayIndex > 0 )
	{
		UpdateSelect( mArrayIndex - 1 );
	}

	if( onSound )
	{	
		/// 아이템 판매 시 사운드 재생
		SOUNDSYS->Play2DSound( 2048 );
	}

	return true;
}

/// 구입이 됨으로써 아이템 정보 갱신
void cUserSellWindow::ModSellItem( unsigned short slot, unsigned int count, bool onSound )
{
	if( slot > MAX_INVENTORY )
		return;

	/// 아이템 등록
	cItem* item = ITEMMAN->GetItem( slot );
	if( !item )
	{
		assert(0);
		return;
	}

	const cItemDefine* define = item->GetDefine();
	if( !define )
	{
		assert(0);
		return;
	}

	int sellSlot = IsRegistSellSlot( slot );
	if( sellSlot == -1 )
	{
		assert(0);
		return;
	}
	
	/// 아이템 갯수 갱신
	cItemIcon* icon = mSellData[sellSlot].mIcon;
	if( icon )
		icon->SetNumber( count );

	if( onSound )
	{	
		/// 아이템 판매 시 사운드 재생
		SOUNDSYS->Play2DSound( 2048 );
	}
}

bool cUserSellWindow::AddJoinItem( sStallSellData* sell )
{
	if( !sell )
	{
		assert(0);
		return false;
	}

	sInventory* inv = &sell->inventory;
	cItemDefine* define = ITEMMAN->GetItemDefine( inv->ItemIndex );
	if( !define )
	{
		assert(0);
		return false;
	}

	/// 해당 슬롯을 가진 아이템이 이미 등록되어있으면 가격만 변경한다
	int joinSlot = IsRegistJoinSlot( inv->number );
	if( joinSlot > -1 )
	{
		mJoinData[joinSlot].mStatus = StallSellItemStart;
		mJoinData[joinSlot].mPrice = sell->price;
		mJoinData[joinSlot].mPriceStr = cNumberEditBox::FormatNumber( sell->price );
		mJoinData[joinSlot].mThatInven.count = inv->count;

		cItemIcon* icon = mJoinData[joinSlot].mIcon;
		if( icon )
			icon->SetNumber( inv->count );

		/// 만약 아이템 카운터 윈도우가 열려있었다면 닫는다.
		cUIWindow* itemCntWin = (cUIWindow*)GAMEUI->GetItemCountWindow();
		if( itemCntWin && itemCntWin->GetEvent() == eMBEVENT_ITEMCOUNT_USERSELLBUY )
			itemCntWin->Hide();

		return true;
	}

	if( mLastSlot >= MAX_STALL_ITEM )
	{
		assert(0);
		return false;
	}

	unsigned long itemColor = COLOR_WHITE;
	ITEMMAN->GetTooltipColor( define->GetTipType(), itemColor );

	/// 이름 설정
	TCHAR name[ITENNAME_LEN] = {0,};
	if( inv->enhanced > 0 )
	{
		TCHAR temp[ITENNAME_LEN] = {0,};
		::_stprintf_s( temp, _T("+%d %s"), inv->enhanced, define->GetName() );
		unsigned int colWidth = 0;
		bool ret = true;
		if ( mNameSkin )
		{
			colWidth = mNameSkin->mSkinInfo->mWidth;
			ret = UIMAN->ModifyText( temp, name, ITENNAME_LEN, colWidth );
		}

		mJoinData[mLastSlot].mItemName = ( ret == true ) ? name : temp;
	}
	else
	{
		unsigned int colWidth = 0;
		bool ret = true;
		if ( mNameSkin )
		{
			colWidth = mNameSkin->mSkinInfo->mWidth;
			ret = UIMAN->ModifyText( define->GetName(), name, ITENNAME_LEN, colWidth );
		}
		mJoinData[mLastSlot].mItemName = ( ret == true ) ? name : define->GetName();
	}

	/// 아이템 삽입
	sUserSellData* data = &mJoinData[mLastSlot];
	data->mThatInven = *inv;
	data->mStatus = (unsigned char)sell->status;
	data->mPrice = sell->price;
	data->mPriceStr = cNumberEditBox::FormatNumber( sell->price );
	data->mColor = itemColor;

	/// 아이콘 세팅
	cItemIcon* icon = data->mIcon;
	if( icon )
	{
		int ret0 = ITEMMAN->CheckLimit( inv->ItemIndex );
		int ret1 = ITEMMAN->CheckLimitEtc( inv->ItemIndex );

		NiColorA color = NiColorA::WHITE;
		if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
		{
			color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
		}

		icon->SetItemIndex( inv->ItemIndex );
		icon->ChangeImage( define->GetIconParam() );
		icon->SetTipType( define->GetTipType() );
		icon->SetNumber( inv->count );
		icon->SetSeal( (inv->seal==1) );
		icon->SetColor( color );

		bool show = ( define->GetCapacity() > 1 ) ? true : false;
		icon->ShowNumber( show );
	}
	
	mLastSlot++;
	return true;
}

bool cUserSellWindow::DelJoinItem( sInventory* inv )
{
	if( inv->ItemIndex == 0 )
		return false;
	
	/// 툴팁 닫기
	UIMAN->HideTip();

	sUserSellData* thatItem = mJoinData;
	for( unsigned int i = 0; i < mLastSlot; ++i, ++thatItem )
	{
		sInventory& thatInv = thatItem->mThatInven;	
		if( thatInv.number == inv->number )
		{
			/// 동일한 슬롯 이면, 삭제
			thatInv.idx			= 0;
			thatInv.ItemIndex	= 0;
			thatInv.number		= 0;
			thatInv.count		= 0;
			thatInv.enhanced	= 0;
			thatInv.seal		= 0;

            /// 아이콘 초기화
			ClearSellData( mJoinData, i );
			
			/// 업데이트
			UpdateData( mJoinData, i, false );

			if( mLastSlot > 0 )
				mLastSlot--;

			/// 현재 선택중인 아이템 이전의 것이 삭제되면 땡기기
			if( i <= (int)mArrayIndex && mArrayIndex > 0 )
			{
				UpdateSelect( mArrayIndex - 1 );
			}

			if( mLastSlot <= 0 )
				mShowSelect = false;

			return true;
		}
	}
	return true;
}

/// 수정중 표시
void cUserSellWindow::PauseJoinItem( sInventory* inv )
{
	int joinSlot = IsRegistJoinSlot( inv->number );
	if( joinSlot > -1 )
	{
		mJoinData[joinSlot].mStatus = StallSellItemPause;
	}
}

/// 데이터 땡기기
void cUserSellWindow::UpdateData( sUserSellData* data, unsigned int index, bool enable )
{
	sUserSellData* data1 = data + index;
	sUserSellData* data2 = 0;
	
	for( unsigned int i = index; i < mLastSlot-1; ++i, ++data1 )
	{
		data2 = data1 + 1;
		data1->mSlotIndex = data2->mSlotIndex;
		data1->mColor	  = data2->mColor;
		data1->mItemName  = data2->mItemName;
		data1->mPrice	  = data2->mPrice;
		data1->mPriceStr  = data2->mPriceStr;
		data1->mStatus	  = data2->mStatus;

		data1->mThatInven = data2->mThatInven;

		/// 이미지 업데이트
		cItemIcon* icon = data1->mIcon;
		cItemIcon* nextIcon = data2->mIcon;
		if(	icon && nextIcon )
		{
			icon->ChangeImage( nextIcon->GetIconParam() );
			icon->SetItemIndex( nextIcon->GetItemIndex() );
			icon->SetTipType( nextIcon->GetTipType() );
			icon->SetNumber( nextIcon->GetNumber() );
			icon->ShowNumber( nextIcon->IsShowNumber() );
			icon->SetSeal( nextIcon->IsSeal() );
		}
	}

	/// 마지막 아이템 초기화
	if( mStatus == eUSERSELL_SELL )
		ClearSellData( mSellData, mLastSlot-1 );
	else
	{
		sUserSellData* data = &mJoinData[mLastSlot-1];
		sInventory& inv = data->mThatInven;
		inv.idx			= 0;
		inv.ItemIndex	= 0;
		inv.number		= 0;
		inv.count		= 0;
		inv.enhanced	= 0;
		inv.seal		= 0;

		///
		ClearSellData( mJoinData, mLastSlot-1 );
	}
}

///
void cUserSellWindow::SetName( LPCTSTR title )
{
	if( mpName )
		mpName->SetText( title );
}

bool cUserSellWindow::IsNetLock()
{
	return ( mLockTry != eLOCK_NONE );
}

void cUserSellWindow::OnNetLock( int lockTry )
{	
	switch( lockTry )
	{
	case eLOCK_REGIST:
		if( mpNameRegist )
			mpNameRegist->NetLock( lockTry );
		break;
	case eLOCK_ITEMUPDATE:
		if( mpItemUpdate )
			mpItemUpdate->NetLock( lockTry );
		break;
	case eLOCK_END:
		if( mpEnd )
			mpEnd->NetLock( lockTry );
		break;
	case eLOCK_JOINEND:
		if( mpJoinEnd )
			mpJoinEnd->NetLock( lockTry );
		break;
	}

	mLockTry = lockTry;
}

void cUserSellWindow::OnNetUnLock( int lockTry )
{
	switch( lockTry )
	{
	case eLOCK_REGIST:
		if( mpNameRegist )
			mpNameRegist->NetUnLock( lockTry );
		break;
	case eLOCK_ITEMUPDATE:
		if( mpItemUpdate )
			mpItemUpdate->NetUnLock( lockTry );
		break;
	case eLOCK_END:
		if( mpEnd )
			mpEnd->NetUnLock( lockTry );
		break;
	case eLOCK_JOINEND:
		if( mpJoinEnd )
			mpJoinEnd->NetUnLock( lockTry );
		break;
	}

	mLockTry = eLOCK_NONE;
}

