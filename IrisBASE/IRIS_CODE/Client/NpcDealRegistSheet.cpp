#include "stdafx.h"
#include "NpcDealRegistSheet.h"
#include "NumberEditBox.h"
#include "Label.h"
#include "ItemIcon.h"
#include "ComboBox.h"
#include "ItemManager.h"
#include "TipWindow.h"
#include "GameUIManager.h"
#include "DragWindow.h"
#include "Button.h"
#include "SpinBox.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameResourceManager.h"
#include "Npc_Common.h"

cNpcDealRegistSheet::cNpcDealRegistSheet( eUINodeType type )
: cUIWindow( type )
, mpIconSlot(0)
, mpPriceEdit(0)
, mpRegistCharge(0)
, mpRegist(0)
, mpRegistPeriod(0)
, mpItemList(0)
, mpPageSpin(0)
{
	::memset( mpCancelButton, 0, sizeof(mpCancelButton) );
}

cNpcDealRegistSheet::~cNpcDealRegistSheet()
{
	Clear();
}

void cNpcDealRegistSheet::Clear()
{
	if( mpItemList )
		mpItemList->Clear();

	cNpcDealOpenMap::cIterator i = mDealOpenMap.Begin();
	cNpcDealOpenMap::cIterator end = mDealOpenMap.End();
	for( ; i != end; ++i )
	{
		NPC_ITEM_AGENT_OPEN_RESULT* p = (NPC_ITEM_AGENT_OPEN_RESULT*)(i->mSecond);
		SAFE_DELETE( p );
	}
	mDealOpenMap.Clear();

	///
	if( mpPriceEdit )
		mpPriceEdit->SetValue( 0 );

	if( mpRegistPeriod )
		mpRegistPeriod->SetSelect( 0 );

	SetTotalPage( 0 );

	//
	UpdateCharge();
}

void cNpcDealRegistSheet::OnHide()
{
	cUIWindow::OnHide();

	DeleteRegistItem();
}

bool cNpcDealRegistSheet::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpPriceEdit = (cNumberEditBox*)GetChild( eUIID_NPCDEAL_REGISTPRICE );
	mpRegistCharge = (cLabel*)GetChild( eUIID_NPCDEAL_REGISTCHARGE );
	mpRegistPeriod = (cComboBox*)GetChild( eUIID_NPCDEAL_REGISTPERIOD );
	mpItemList = (cImageAndFontDataListBox*)GetChild( eUIID_NPCDEAL_REGISTITEMLIST );
	mpPageSpin = (cSpinBox*)GetChild( eUIID_NPCDEAL_REGISTSPIN );
	mpRegist = GetChild( eUIID_NPCDEAL_REGISTBUTTON );

	// 등록슬롯
	mpIconSlot = new cItemIcon;
	if( mpIconSlot->CreateBySkinName( "NpcDeal_RegistSlot", this, 1 ) == false )
		return false;

	if( mpRegistPeriod )
	{
		cStringT str;
		str.Format( UIMAN->GetUIText( 240 ), NPC_ITEM_AGENT_TIME1 );
		mpRegistPeriod->AddItem( (LPCTSTR)str.Cstr(), (void*)NPC_ITEM_AGENT_TIME1 );

		str.Format( UIMAN->GetUIText( 240 ), NPC_ITEM_AGENT_TIME2 );
		mpRegistPeriod->AddItem( (LPCTSTR)str.Cstr(), (void*)NPC_ITEM_AGENT_TIME2 );

		str.Format( UIMAN->GetUIText( 240 ), NPC_ITEM_AGENT_TIME3 );
		mpRegistPeriod->AddItem( (LPCTSTR)str.Cstr(), (void*)NPC_ITEM_AGENT_TIME3 );

		mpRegistPeriod->SetSelect( 0 );
	}

	cUIPos pos;
	unsigned int rowHeight = mpItemList->GetRowHeight();
	for( int i = 0; i < 10; ++i )
	{
		mpCancelButton[i] = new cButton;
		if( mpCancelButton[i]->CreateBySkinName( "NpcDeal_RegistCancel", this, CANCELBUTTON_ID + i ) == false )
		{
			assert(0);
			return false;
		}

		pos.mX = mpCancelButton[i]->GetRelativeRect().mLeft;
		pos.mY = mpCancelButton[i]->GetRelativeRect().mTop + ( i * rowHeight );

		mpCancelButton[i]->SetRelativePos( pos );
	}

	if( mpRegist )
		mpRegist->SetEnabled( false );

	Clear();
	return true;
}

void cNpcDealRegistSheet::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mpItemList )
	{
		unsigned int rowHeight = mpItemList->GetRowHeight();

		for( int i = 0; i < 10; ++i )
		{
			if( mpCancelButton[i] )
			{
				cUIPos pos;
				pos.mX = mpCancelButton[i]->GetRelativeRect().mLeft;
				pos.mY = mpCancelButton[i]->GetRelativeRect().mTop + ( i * rowHeight );

				mpCancelButton[i]->SetRelativePos( pos );
			}
		}
	}
}

void cNpcDealRegistSheet::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_NPCDEAL_CLOSE:
		Hide();
		break;
	case eUIID_NPCDEAL_REGISTBUTTON:
		if( mpIconSlot )
		{
			char valid = (char)mpRegistPeriod->GetSelectData();

			/// 등록
			ITEMMAN->TryToNpcDealRegistReq( mpIconSlot->GetSlotIndex(), mpPriceEdit->GetValue(), valid );
		}
		else
			assert(0);
		break;
	case eSPINBOX_LEFTBTN:
	case eSPINBOX_RIGNTBTN:
		{
			int page = mpPageSpin->GetSelectedIdx();

			if( page < mpPageSpin->GetSize() )
			{
				ITEMMAN->TryToNpcDealOpenPage( page + 1 );
			}
		}
		break;
	}

	/// 버튼 눌림
	if( id >= CANCELBUTTON_ID && id < CANCELBUTTON_ID + 10 )
	{
		int i = id - CANCELBUTTON_ID;
		if( mpCancelButton[i] && mpCancelButton[i]->IsVisible() )
		{
			///
			//int page = mpPageSpin->GetSelectedIdx();
			mpItemList->ChangeSelectRow( i );

			NPC_ITEM_AGENT_OPEN_RESULT* data = (NPC_ITEM_AGENT_OPEN_RESULT*)mpItemList->GetExtraData( i );
			if( data )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NPCDEAL_REGISTCANCEL, eSKIN_YESNO,
					GAMERESOURCEMAN->GetGameText( 1513 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					(void*)data->Idx );
			}
		}
		else
			assert(0);
	}
}

void cNpcDealRegistSheet::OnComboSelected( cUINode* , unsigned int )
{
	UpdateCharge();
}

void cNpcDealRegistSheet::OnListBoxImageHorvered( cUINode* , unsigned int , const cUIPos& pos, void* extraData )
{
	NPC_ITEM_AGENT_OPEN_RESULT* data = (NPC_ITEM_AGENT_OPEN_RESULT*)extraData;
	if( data )
	{
		UIMAN->ShowOtherItemTip( eTIP_ITEM, pos, data->Inventory );
	}
}

void cNpcDealRegistSheet::OnListBoxImageLeft( cUINode* , unsigned int )
{
	UIMAN->HideTip();
}

void cNpcDealRegistSheet::OnIconDragged( cUINode* caller, unsigned int , const cUIPos& pos, bool shift )
{
	cIcon* icon = (cIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_NPCDEALREGIST, icon, pos, shift );
}

void cNpcDealRegistSheet::UpdateCharge()
{
	if( mpRegistCharge && mpRegistPeriod )
	{
		unsigned int time = (unsigned int)mpRegistPeriod->GetSelectData();

		/// 수수료 = time * ITEM_AGENT_NPC_FEE
		unsigned long charge = (time / NPC_ITEM_AGENT_TIME1) * ITEM_AGENT_NPC_FEE;

		LPCTSTR str = cNumberEditBox::FormatNumber( charge );
		mpRegistCharge->SetText( str );
	}
}

bool cNpcDealRegistSheet::RegistItem( unsigned short slotIndex )
{
	if( slotIndex > MAX_INVENTORY )
		return false;

 	/// 10개 제한 설정
 	if( mpItemList->GetRowCount() >= MAX_REGIST_ITEM_COUNT )
 	{
 		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
 							eMBEVENT_NONE, eSKIN_OK,
 							GAMERESOURCEMAN->GetGameText( 1527 ),
 							GAMERESOURCEMAN->GetGameText( 301 )
 			);
 		return false;
 	}

	cItem* item = ITEMMAN->GetItem( slotIndex );
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

	if( !mpIconSlot )
	{
		assert(0);
		return false;
	}

	/// 거래불가 아이템 등록 안됨
	if( define->GetExchange() == eNORMAL_ITEM_EXCHAGNE_NO || define->GetExchange() == eSEALED_ITEM_EXCHAGNE_NO )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1516 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}

	/// 미인증 거래 가능 아이템이면서 실제로 미인증된 아이템이여야만 거래가능
	if( define->GetExchange() == eSEALED_ITEM_EXCHAGNE_YES && item->IsSeal() == false )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1516 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return false;
	}


	unsigned short itemSlotIndex = mpIconSlot->GetSlotIndex();
	if( itemSlotIndex != slotIndex )
	{
		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
			{
				oldItem->SetLock( false );
			}
		}

		/// 참조 슬롯 인덱스
		mpIconSlot->SetSlotIndex( slotIndex );
		item->SetLock( true );
	}

	///
	mpIconSlot->ChangeImage( define->GetIconParam() );
	mpIconSlot->SetTipType( define->GetTipType() );
	mpIconSlot->SetItemIndex( item->GetIndex() );
	mpIconSlot->SetNumber( item->GetCount() );
	mpIconSlot->SetSeal( item->IsSeal() );

	// 제한 컬러
	int ret0 = ITEMMAN->CheckLimit( item->GetIndex() );
	int ret1 = ITEMMAN->CheckLimitEtc( item->GetIndex() );

	NiColorA color = NiColorA::WHITE;
	if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
	{
		color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
	}
	mpIconSlot->SetColor( color );

	bool show = ( define->GetCapacity() > 1 ) ? true : false;
	mpIconSlot->ShowNumber( show );

	if( mpRegist )
		mpRegist->SetEnabled( true );

	if( mpPriceEdit )
	{
		mpPriceEdit->SetFocus();
		mpPriceEdit->SetAllSelect();
	}

	///아이템 슬롯에 셋팅 시 사운드 재생
	ITEMMAN->PlayIconSettingSound( slotIndex );

	return true;
}

void cNpcDealRegistSheet::DeleteRegistItem()
{
	if( mpIconSlot )
	{
		mpIconSlot->ChangeImage( 0 );
		mpIconSlot->SetTipType( eTOOLTIP_NONE );
		mpIconSlot->SetItemIndex( 0 );
		mpIconSlot->SetColor( NiColorA::WHITE );
		mpIconSlot->SetSeal( false );

		/// 아이템 락 초기화
		unsigned short itemSlotIndex = mpIconSlot->GetSlotIndex();
		if( itemSlotIndex <= MAX_INVENTORY )
		{
			cItem* oldItem = ITEMMAN->GetItem( itemSlotIndex );
			if( oldItem )
				oldItem->SetLock( false );

			mpIconSlot->SetSlotIndex( USHRT_MAX );
		}
	}

	if( mpRegist )
		mpRegist->SetEnabled( false );
}

bool cNpcDealRegistSheet::AddItem( NPC_ITEM_AGENT_OPEN_RESULT* data )
{
	if( mpItemList && data )
	{
		cItemDefine* define = ITEMMAN->GetItemDefine( data->Inventory.ItemIndex );
		if( !define )
		{
			assert(0);
			return false;
		}

		cIconParam* iconParam = define->GetIconParam();
		if( !iconParam )
		{
			assert(0);
			return false;
		}

		if( mDealOpenMap.Insert( data->Idx, data ) == false )
		{
			assert(0);
			return false;
		}

		// 제한 컬러
		int ret0 = ITEMMAN->CheckLimit( data->Inventory.ItemIndex );
		int ret1 = ITEMMAN->CheckLimitEtc( data->Inventory.ItemIndex );

		NiColorA color = NiColorA::WHITE;
		if( ret0 != ITEMLIMIT_SUCCESS || ret1 != ITEMLIMIT_SUCCESS )
		{
			color = NiColorA( 1.0f, 0.0f, 0.0f, 1.0f );
		}

		/// 아이템 이미지 생성
		cUIImage* pImage = new cUIImage( iconParam->mpTexture );
		if( pImage )
		{
			unsigned int tx = iconParam->mTexPos.mX;
			unsigned int ty = iconParam->mTexPos.mY;
			pImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
			pImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
			pImage->SetColor( color );
		}

		cIconParam* pTipParam = UIMAN->GetIconParam( 10 );
		cUIImage* edgeImage = 0;
		if( pTipParam && define->GetTipType() != eTOOLTIP_NONE )
		{
			edgeImage = new cUIImage( pTipParam->mpTexture );

			unsigned int tx = pTipParam->mTexPos.mX + ( define->GetTipType() * ICON_SIZE );
			unsigned int ty = pTipParam->mTexPos.mY;
			edgeImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
			edgeImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
		}

		cUIImage* sealImage = 0;
		cIconParam* pSealParam = UIMAN->GetIconParam( SEAL_INDEX );
		if( pSealParam && data->Inventory.seal == 1 )
		{
			sealImage = new cUIImage( pSealParam->mpTexture );

			unsigned int tx = pSealParam->mTexPos.mX;
			unsigned int ty = pSealParam->mTexPos.mY;
			sealImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
			sealImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
		}

		/*--	데이터 세팅		*/
		/// 아이템명
		TCHAR temp[ITENNAME_LEN] = {0,};
		TCHAR name[ITENNAME_LEN] = {0,};

		// 컬러가져오기
		unsigned long itemColor = COLOR_WHITE;
		ITEMMAN->GetTooltipColor( define->GetTipType(), itemColor );

		/// 아이템 정보 받아서 출력
		if( data->Inventory.enhanced > 0 )
		{
			/// 강화 단계 출력
			::_stprintf_s( temp, _T("+%d %s"), data->Inventory.enhanced, define->GetName() );
		}
		else
		{
			::_stprintf_s( temp, _T("%s"), define->GetName() );
		}

		unsigned int colWidth = mpItemList->GetColumnMaxWidth( 0 );
		bool ret = UIMAN->ModifyText( temp, name, ITENNAME_LEN, colWidth );
		LPCTSTR itemname = ( ret == true ) ? name : temp;

		cUIImageFont* font = UIMAN->GetImageFont( eIAMGEFONT_ITEMCOUNT );
		int row = mpItemList->AddRow( itemname, itemColor, pImage, edgeImage, sealImage, font, false, 0, (void*)data );
		if( row == -1 )
		{
			assert(0);
			return false;
		}

		/// 착용제한 레벨
		cItemLimit* limit = define->GetLimit();
		char level = ( limit != NULL ) ? limit->GetLevel() : 0;

		cStringT levelStr;
		levelStr.Format( _T("%d"), level );
		mpItemList->AddColume( row, (LPCTSTR)levelStr.Cstr(), mDefaultColor );

		/// 마감시한
		cStringT timeStr;
		timeStr.Format( UIMAN->GetUIText( 118 ), (data->ValidThru < 0)? 0:data->ValidThru );
		mpItemList->AddColume( row, (LPCTSTR)timeStr.Cstr(), mDefaultColor );

		/// 판매자명
		mpItemList->AddColume( row, HERO->GetName(), mDefaultColor );

		/// 수량
		cStringT countStr;
		countStr.Format( _T("%d"), data->Inventory.count );
		mpItemList->AddColume( row, (LPCTSTR)countStr.Cstr(), mDefaultColor );

		/// 판매가
		cStringT priceStr;
		LPCTSTR price = cNumberEditBox::FormatNumber( data->Price );
		priceStr.Format( UIMAN->GetUIText( 354 ), price );

		mpItemList->AddColume( row, (LPCTSTR)priceStr.Cstr(), mDefaultColor );
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

void cNpcDealRegistSheet::SetTotalPage( unsigned int total )
{
	if( mpPageSpin )
	{
		mpPageSpin->Clear();

		if( total > 0 )
		{
			cStringT str;
			for( unsigned int i = 0; i < total; ++i )
			{
				str.Format( _T("%d / %d"), i + 1, total );
				mpPageSpin->AddData( (LPCTSTR)str.Cstr() );
			}
		}

		if( total <= 1 )
			mpPageSpin->Hide();
		else
			mpPageSpin->Show();
	}

	UpdatePage();
}

/// 페이지 버튼 등 재 세팅
void cNpcDealRegistSheet::UpdatePage()
{
	if( mpItemList && mpPageSpin )
	{
		int page = mpPageSpin->GetSelectedIdx();

		/// 리스트박스 갱신
		mpItemList->ChangeTopIndex( page * 10 );

		/// 버튼 갱신
		int count = mpItemList->GetRowCount() - mpItemList->GetCurrentTopIndex();

		for( int i = 0; i < 10; ++i )
		{
			if( i < count )
				mpCancelButton[i]->Show();
			else
				mpCancelButton[i]->Hide();
		}
	}
}

void cNpcDealRegistSheet::ChangePage( unsigned int page )
{
	if( page > 0 )
	{
		if( mpPageSpin )
		{
			mpPageSpin->SetSelectIdx( page-1 );
			UpdatePage();
		}
	}
}

void cNpcDealRegistSheet::OnNetLock( int lockTry )
{
	cUINode::NetLock( lockTry );
}

void cNpcDealRegistSheet::OnNetUnLock( int lockTry )
{
	cUINode::NetUnLock( lockTry );
}
