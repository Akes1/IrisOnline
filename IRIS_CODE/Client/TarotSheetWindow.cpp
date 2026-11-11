#include "StdAfx.h"
#include "TarotSheetWindow.h"
#include "PlaneObject.h"
#include "FontAgent.h"
#include "TextBox.h"
#include "GameResourceManager.h"
#include "TarotManager.h"
#include "CardImage.h"
#include "Label.h"
#include "ItemManager.h"
#include "DragWindow.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "GameUIManager.h"

cTarotSheetWindow::cTarotSheetWindow( eUINodeType type )
: cUIWindow( type )
, mpCardImage(0)
, mpLineImage(0)
, mpCardSkin(0)
, mpLineSkin(0)
, mpSelectImg(0)
, mSelectIndex(-1)
, mpDescBox(0)
, mpName(0)
, mpValue(0)
, mpBackImage(0)
, mpBackSkin(0)
{
	
}

cTarotSheetWindow::~cTarotSheetWindow()
{
	SAFE_DELETE( mpBackImage );
	SAFE_DELETE( mpSelectImg );
	SAFE_DELETE( mpCardImage );
	SAFE_DELETE( mpLineImage );
}

void cTarotSheetWindow::Clear()
{
	/// 카드 아이콘 이미지 초기화
	unsigned short slotIndex = INVENTORY_TAROT_NORMAL_BEGIN;
	for( int i = 0; i < ICON_MAX; ++i, ++slotIndex )
	{
		sCardBookData& data = mData[i];
		cItemIcon* icon = data.mIcon;
		if( icon )
		{	
			icon->ChangeImage( &data.mIconParam );
			icon->SetTipType( eTOOLTIP_NONE );
			icon->SetItemIndex( 0 );
		}

		data.mName.Clear();
	}

	mSelectIndex = -1;

	/// 큰 카드 이미지 초기화
	if( mpCardSkin && mpCardImage )
	{
		unsigned int tx = mpCardSkin->mSkinInfo->mTexX;
		unsigned int ty = mpCardSkin->mSkinInfo->mTexY;
		unsigned int w = mpCardSkin->mSkinInfo->mWidth;
		unsigned int h = mpCardSkin->mSkinInfo->mHeight;
		mpCardImage->SetTexture( mpCardSkin->mpTexture );
		mpCardImage->SetTextureRect( cUIRect( tx, ty, tx+w, ty+h ) );
	}

	if( mpName )
		mpName->Clear();

	if( mpDescBox )
		mpDescBox->Clear();

	if( mpValue )
		mpValue->Hide();
}

void cTarotSheetWindow::ClearIcon( int i )
{
	if( i < 0 || i >= ICON_MAX )
	{
		assert(0);
		return;
	}

	sCardBookData& data = mData[i];
	cItemIcon* icon = data.mIcon;
	if( icon )
	{	
		icon->ChangeImage( &data.mIconParam );
		icon->SetTipType( eTOOLTIP_NONE );
		icon->SetItemIndex( 0 );
	}

	data.mName.Clear();

	// 
	if( mSelectIndex == i )
	{
		if( mpCardSkin && mpCardImage )
		{
			unsigned int tx = mpCardSkin->mSkinInfo->mTexX;
			unsigned int ty = mpCardSkin->mSkinInfo->mTexY;
			unsigned int w = mpCardSkin->mSkinInfo->mWidth;
			unsigned int h = mpCardSkin->mSkinInfo->mHeight;
			mpCardImage->SetTexture( mpCardSkin->mpTexture );
			mpCardImage->SetTextureRect( cUIRect( tx, ty, tx+w, ty+h ) );
		}

		if( mpName )
			mpName->Clear();

		if( mpDescBox )
			mpDescBox->Clear();

		if( mpValue )
			mpValue->Hide();

		mSelectIndex = -1;
	}
}

void cTarotSheetWindow::Open()
{
	Clear();

	/// 정보 갱신
	for( int i = 0; i < ICON_MAX; ++i )
		UpdateData(i);
}

bool cTarotSheetWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpDescBox = (cTextBox*)GetChild( eUIID_GAME_CARDBOOK_TAROTDESCBOX );
	mpName = (cLabel*)GetChild( eUIID_GAME_CARDBOOK_TAROTNAME );
	mpValue = (cLabel*)GetChild( eUIID_GAME_CARDBOOK_TAROTVALUE );

	if( mpDescBox )
		mpDescBox->SetDrawTextBottom( false );

	if( mpName )
		mpName->SetTextColor( COLOR_WHITE );

	if( mpValue )
		mpValue->SetTextColor( COLOR_WHITE );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	/// 큰 카드 이미지 생성
	mpCardSkin = pSkin->GetNodeSkin( "TarotSheet_Card" );
	if( mpCardSkin )
	{
		mpCardImage = new cUIImage( mpCardSkin->mpTexture );
		unsigned int tx = mpCardSkin->mSkinInfo->mTexX;
		unsigned int ty = mpCardSkin->mSkinInfo->mTexY;
		unsigned int tw = mpCardSkin->mSkinInfo->mWidth;
		unsigned int th = mpCardSkin->mSkinInfo->mHeight;
		unsigned int x = mpCardSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpCardSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;

		mpCardImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpCardImage->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );
	}

	/// 큰 카드 배경 이미지 생성
	mpBackSkin = pSkin->GetNodeSkin( "TarotSheet_CardBack" );
	if( mpBackSkin )
	{
		mpBackImage = new cUIImage( mpBackSkin->mpTexture );
		unsigned int tx = mpBackSkin->mSkinInfo->mTexX;
		unsigned int ty = mpBackSkin->mSkinInfo->mTexY;
		unsigned int tw = mpBackSkin->mSkinInfo->mWidth;
		unsigned int th = mpBackSkin->mSkinInfo->mHeight;
		unsigned int x = mpBackSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpBackSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;

		mpBackImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpBackImage->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );
	}

	/// 라인 이미지 생성
	mpLineSkin = pSkin->GetNodeSkin( "TarotSheet_Line" );
	if( mpLineSkin )
	{
		mpLineImage = new cUIImage( mpLineSkin->mpTexture );
		unsigned int tx = mpLineSkin->mSkinInfo->mTexX;
		unsigned int ty = mpLineSkin->mSkinInfo->mTexY;
		unsigned int tw = mpLineSkin->mSkinInfo->mWidth;
		unsigned int th = mpLineSkin->mSkinInfo->mHeight;
		unsigned int x = mpLineSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLineSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;

		mpLineImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpLineImage->SetScreenRect( cUIRect( x, y, x + tw, y + th ) );
	}

	/// 카드 아이콘 & 스프레드 아이콘 생성
	cString str;
	unsigned short slotIndex = INVENTORY_TAROT_NORMAL_BEGIN;
	for( unsigned int i = 0; i < TAROT_COUNT; ++i, ++slotIndex )
	{
		str.Format( "TarotSheet_Icon%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		/// 아이템 매니저와 바인딩
		ITEMMAN->BindIcon( slotIndex, icon );
		
		///
		sCardBookData& data = mData[i];
		cUINodeSkin* iconSkin = pSkin->GetNodeSkin( str );
		if( iconSkin )
		{
			cIconParam& iconParam = data.mIconParam;
			iconParam.mpTexture = iconSkin->mpTexture;
			iconParam.mTexPos.mX = iconSkin->mSkinInfo->mTexX;
			iconParam.mTexPos.mY = iconSkin->mSkinInfo->mTexY;
		}

		/// 우선 기본이미지로 세팅
		icon->ChangeImage( &data.mIconParam );
		mData[i].mIcon = icon;
	}

	/// 스프레드 아이콘 생성
	slotIndex = INVENTORY_SPREAD_NORMAL_BEGIN;
	for( unsigned int i = 0; i < SPREAD_COUNT; ++i, ++slotIndex )
	{
		str.Format( "TarotSheet_Spread%02d", i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, slotIndex ) == false )
			return false;

		/// 아이템 매니저와 바인딩
		ITEMMAN->BindIcon( slotIndex, icon );
		icon->SetUsedFlag( false, false );

		///
		sCardBookData& data = mData[ i+TAROT_COUNT ];
		cUINodeSkin* iconSkin = pSkin->GetNodeSkin( str );
		if( iconSkin )
		{
			cIconParam& iconParam = data.mIconParam;
			iconParam.mpTexture = iconSkin->mpTexture;
			iconParam.mTexPos.mX = iconSkin->mSkinInfo->mTexX;
			iconParam.mTexPos.mY = iconSkin->mSkinInfo->mTexY;
		}

		/// 우선 기본이미지로 세팅
		icon->ChangeImage( &data.mIconParam );
		mData[ i+TAROT_COUNT ].mIcon = icon;
	}

	/// 선택 이미지 생성
	cUINodeSkin* pSelectSkin = pSkin->GetNodeSkin( "TarotSheet_Select" );
	if( pSelectSkin && pSelectSkin->mpTexture )
	{
		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)pSelectSkin->mSkinInfo->mTexX;
		unsigned short ty = (unsigned short)pSelectSkin->mSkinInfo->mTexY;
		unsigned short tw = (unsigned short)pSelectSkin->mSkinInfo->mTexWidth;
		unsigned short th = (unsigned short)pSelectSkin->mSkinInfo->mTexHeight;

		mpSelectImg = new cPlaneObject;
		if( mpSelectImg->Create( pSelectSkin->mpTexture, 0, 0, tw, th,
			tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create select image");
			return false;
		}
	}
	else
		assert(0);
	
	return true;
}

void cTarotSheetWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpCardSkin && mpCardImage )
	{
		unsigned int x = mpCardSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpCardSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpCardImage->SetPos( x, y );
	}

	if( mpBackSkin && mpBackImage )
	{
		unsigned int x = mpBackSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpBackSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpBackImage->SetPos( x, y );
	}

	if( mpLineSkin && mpLineImage )
	{
		unsigned int x = mpLineSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLineSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpLineImage->SetPos( x, y );
	}

	UpdateSelectImage();
}

void cTarotSheetWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 아이템 이름 출력
	for( unsigned int i = 0; i < ICON_MAX; ++i )
	{
		sCardBookData& data = mData[i];
		cItemIcon* icon = data.mIcon;
		if( icon )
		{
			cUIRect rc = icon->GetAbsoluteRect();
 			int x = rc.mRight + 3;
 			int y = rc.mTop + int(( rc.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_UI) ) * 0.5);
			
			pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPCTSTR)data.mName.Cstr(), x, y, COLOR_WHITE );
		}
	}

	if( mpLineImage )
		mpLineImage->Draw();

	if( mpBackImage )
		mpBackImage->Draw();

	if( mpCardImage )
		mpCardImage->Draw();

	if( mpSelectImg && mSelectIndex > -1 )
		mpSelectImg->Draw();

	cUIWindow::OnRender( pKeeper );
}

void cTarotSheetWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, false, false, false );
}

///
void cTarotSheetWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	sCardBookData& data = mData[0];
	cItemIcon* dataIcon = data.mIcon;
	if( !dataIcon )
		return;

	cUIRect iconRect;
	iconRect.mLeft = dataIcon->GetAbsoluteRect().mLeft;
	iconRect.mTop = dataIcon->GetAbsoluteRect().mTop;
	iconRect.mRight = iconRect.mLeft + (ROW_WIDTH*2);
	iconRect.mBottom = iconRect.mTop + (ROW_HEIGHT*ICON_MAX);

	/// 영역 체크
	if( iconRect.ContainPoint( pos ) == false )
		return;

	/// 속해있는 영역 찾기
	for( int i = 0; i < ICON_MAX; ++i )
	{
		sCardBookData& data = mData[i];
		if( !data.mIcon )
		{
			assert(0);
			continue;
		}

		/// 아이템이 있으면 검사
		if( data.mIcon->GetItemIndex() == 0 )
			continue;

		cUIRect rc = data.mIcon->GetAbsoluteRect();
		rc.mRight = rc.mLeft + ROW_WIDTH;
		rc.mBottom = rc.mTop + ROW_HEIGHT;

		/// 영역 체크
		if( rc.ContainPoint( pos ) == true )
		{
			if( mSelectIndex != i )
			{
				mSelectIndex = i;
				UpdateText();
				UpdateSelectImage();
			}
			break;
		}
	}
}

void cTarotSheetWindow::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( icon )
		UIMAN->ShowDrag( this, DRAG_FROM_INVENTORY, icon, pos, shift );
}

void cTarotSheetWindow::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	cItemIcon* icon = (cItemIcon*)caller;
	if( !icon )
	{
		assert(0);
		return;
	}

	unsigned short curSlotIdx = icon->GetSlotIndex();
	unsigned short slotIdx = 0;
	if( ITEMMAN->FindEmptySlot( &slotIdx, INVENTORY_BAG0_BEGIN, HERO->GetBagEndSlot() ) == true )
	{
		GAMEUI->ShowItemCardBook( eMBEVENT_ITEM_CARDBOOK_TAROTRETURN, curSlotIdx, slotIdx, 0 );
	}
	else
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 202 ),
			GAMERESOURCEMAN->GetGameText( 301 ) );
	}
}


/// 타롯 선택
void cTarotSheetWindow::UpdateText()
{
	if( !mpDescBox || !mpName || !mpValue )
	{
		assert(0);
		return;
	}
		
	/// 설명 정보 갱신
	mpDescBox->Clear();
	mpValue->Hide();

	/// 카드 정보삽입
	cItemIcon* icon = mData[mSelectIndex].mIcon;
	if( !icon )
		return;

	unsigned long itemIndex = icon->GetItemIndex();
	cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
	if( define )
	{
		/// 이름삽입
		mpName->SetText( define->GetName() );

		/// 타로카드
		if( define->GetType() == ITEM_CARD && define->GetSubType() == ITEM_CARD_TAROT )
		{
			cItemTarot* tarot = define->GetTarotDefine();
			if( tarot )
			{
				/// 카드 해석값 
				TCHAR temp[256] = {0,};
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 343 ), tarot->GetStraightValue(), tarot->GetInverseValue() );
				mpValue->SetText( temp );

				/// 카드 설명삽입
				mpDescBox->AddNewRow( GAMERESOURCEMAN->GetCardBookText( itemIndex ), COLOR_WHITE );

				// 카드 이미지 삽입
				cCardParam* param = TAROTMAN->GetImageParam( tarot->GetImageIndex() );
				if( param )
				{
					int x = param->mTexPos.mX;
					int y = param->mTexPos.mY;
					int w = mpCardSkin->mSkinInfo->mWidth;
					int h = mpCardSkin->mSkinInfo->mHeight;
					mpCardImage->SetTexture( param->mpTexture );
					mpCardImage->SetTextureRect( cUIRect( x, y, x+w, y+h ) );
				}

				mpValue->Show();
			}
		}
		/// 스프레드
		else if( define->GetType() == ITEM_SPREAD )
		{
			/// 스프레드 설명 삽입
			mpDescBox->AddNewRow( GAMERESOURCEMAN->GetCardBookText( itemIndex ), COLOR_WHITE );

			/// 스프레드 이미지 삽입
			cItemSpread* spread = define->GetSpreadDefine();
			if( spread )
			{
				cCardParam* param = TAROTMAN->GetImageParam( spread->mParam[0].mImageIndex );
				if( param )
				{
					int x = param->mTexPos.mX;
					int y = param->mTexPos.mY;
					int w = mpCardSkin->mSkinInfo->mWidth;
					int h = mpCardSkin->mSkinInfo->mHeight;
					mpCardImage->SetTexture( param->mpTexture );
					mpCardImage->SetTextureRect( cUIRect( x, y, x+w, y+h ) );
				}
			}
		}
	}
}

///
void cTarotSheetWindow::UpdateSelectImage()
{
	if( mSelectIndex == -1 )
		return;

	if( mSelectIndex >= ICON_MAX )
	{
		assert(0);
		return;
	}

	if( mpSelectImg )
	{
		sCardBookData& data = mData[mSelectIndex];
		if( data.mIcon )
		{
			cUIRect iconRc = data.mIcon->GetAbsoluteRect();
			mpSelectImg->SetScreenRect( iconRc.mLeft-1, iconRc.mTop-1, ROW_WIDTH, ROW_HEIGHT );
		}
	}
}

void cTarotSheetWindow::UpdateData( int i )
{
	if( i < 0 || i >= ICON_MAX )
	{
		assert(0);
		return;
	}

	cItemIcon* icon = mData[i].mIcon;
	if( icon )
	{
		unsigned short slotIndex = icon->GetSlotIndex();
		cItem* item = ITEMMAN->GetItem( slotIndex );
		if( item )
		{
			cItemDefine* define = item->GetDefine();
			if( define )
			{
				icon->SetItemIndex( item->GetIndex() );
				icon->ChangeImage( define->GetIconParam() );
				icon->SetTipType( define->GetTipType() );

				/// 정보세팅
				/// 이름 길이 조절
				TCHAR name[ITENNAME_LEN] = {0,};
				int colWidth = ROW_WIDTH - icon->GetAbsoluteRect().GetWidth();
				/// 텍스트 출력 위치와 아이콘 출력 위치를 감안하여 5정도의 차이를 둔다.
				colWidth -= 5;
				bool ret = UIMAN->ModifyText( define->GetName(), name, ITENNAME_LEN, colWidth );
				
				mData[i].mName = ( ret == true ) ? name : define->GetName();
			}
		}
	}
}

/// 카드나 스프레드가 이미 등록되어있는지 여부
unsigned long cTarotSheetWindow::IsRegistItem( int i )
{
	if( i < 0 || i >= ICON_MAX )
	{
		assert(0);
		return ULONG_MAX;
	}

	sCardBookData& data = mData[i];
	cItemIcon* icon = data.mIcon;
	if( !icon )
	{
		assert(0);
		return ULONG_MAX;
	}

	return ( icon->GetItemIndex() == 0 ) ? 0 : icon->GetItemIndex();
}

unsigned short cTarotSheetWindow::GetSlotIndex( int i )
{
	if( i < 0 || i >= ICON_MAX )
	{
		assert(0);
		return USHRT_MAX;
	}

	sCardBookData& data = mData[i];
	cItemIcon* icon = data.mIcon;
	if( !icon )
	{
		assert(0);
		return USHRT_MAX;
	}
	return icon->GetSlotIndex();
}
