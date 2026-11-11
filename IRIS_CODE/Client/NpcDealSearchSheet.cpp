#include "stdafx.h"
#include "NpcDealSearchSheet.h"

#include "UIImage.h"
#include "NumberEditBox.h"
#include "ComboBox.h"
#include "ListFolderBox.h"
#include "Button.h"
#include "GameResourceManager.h"
#include "ItemManager.h"
#include "TipWindow.h"
#include "GameUIManager.h"
#include "Npc_Common.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "Application.h"
#include "FontAgent.h"
#include "ItemIcon.h"

cNpcDealSearchSheet::cNpcDealSearchSheet( eUINodeType type )
: cUIWindow( type )
, mpNameEdit(0)
, mpHighLv(0)
, mpLowLv(0)
, mpSearch(0)
, mpRare(0)
, mpEnhance(0)
, mpTypeList(0)
, mpHighImage(0)
, mpItemList(0)
, mTempType(-1)
, mTempSubType(-1)
, mTempLowLv(0)
, mTempHighLv(0)
, mTempEnhance(0)
, mTempRare(0)
, mTempPage(1)
, mDealDelayTime(0)
, mSaveStartPage(0)
, mSaveEndPage(0)
, mSaveCurrentPage(0)
, mSaveResult(-1)
, mpLeftPageButton(0)
, mpRightPageButton(0)
, mpPageNumPos(0)
{
	memset( mpBuyButton, 0, sizeof(mpBuyButton) );
	for( unsigned int cnt = 0; cnt < 10; ++cnt )
	{
		mPageNum[cnt].mColor = COLOR_WHITE;
		mPageNum[cnt].mPos.mX = 0;
		mPageNum[cnt].mPos.mY = 0;
		mPageNum[cnt].mIsShow = false;
	}

	mPageTextRect = cUIRect( 0,0,0,0 );

}

cNpcDealSearchSheet::~cNpcDealSearchSheet()
{
	if( mpItemList )
		mpItemList->Clear();

	for( unsigned int i=0;i<mDealSearchArr.GetSize(); ++i )
	{
		NPC_ITEM_AGENT_SEARCH_RESULT* p = (NPC_ITEM_AGENT_SEARCH_RESULT*)mDealSearchArr[i];
		SAFE_DELETE(p);
	}
	mDealSearchArr.Clear();

	///
	SAFE_DELETE( mpHighImage );
}

void cNpcDealSearchSheet::Clear()
{
	if( mpItemList )
		mpItemList->Clear();

	for( unsigned int i=0;i<mDealSearchArr.GetSize(); ++i )
	{
		NPC_ITEM_AGENT_SEARCH_RESULT* p = (NPC_ITEM_AGENT_SEARCH_RESULT*)mDealSearchArr[i];
		SAFE_DELETE(p);
	}
	mDealSearchArr.Clear();

	if( mpTypeList )
		mpTypeList->InitSelectRow();

	if( mpNameEdit )
		mpNameEdit->Clear();

	if( mpLowLv )
		mpLowLv->SetValue( LOWLEVEL );

	if( mpHighLv )
		mpHighLv->SetValue( HIGHLEVEL );

	if( mpRare )
		mpRare->SetSelect( 0 );

	if( mpEnhance )
		mpEnhance->SetSelect( 0 );

	mTempType = -1;
	mTempSubType = -1;
	mTempLowLv = LOWLEVEL;
	mTempHighLv = HIGHLEVEL;
	mTempEnhance = 0;
	mTempRare = 0;
	mTempName.Clear();

	mSortData = 255;

	mTempPage = 1;
	mTempSortData = 255;

	mSaveStartPage = 0;
	mSaveEndPage = 0;
	mSaveResult = -1;

	ClearPage();
}

void cNpcDealSearchSheet::ClearData()
{
	for( unsigned int i=0;i<mDealSearchArr.GetSize(); ++i )
	{
		NPC_ITEM_AGENT_SEARCH_RESULT* p = (NPC_ITEM_AGENT_SEARCH_RESULT*)mDealSearchArr[i];
		SAFE_DELETE(p);
	}
	mDealSearchArr.Clear();
}

bool cNpcDealSearchSheet::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpNameEdit = (cEditBox*)GetChild( eUIID_NPCDEAL_SEARCHNAME );
	mpHighLv = (cNumberEditBox*)GetChild( eUIID_NPCDEAL_SEARCHHIGHLV );
	mpLowLv = (cNumberEditBox*)GetChild( eUIID_NPCDEAL_SEARCHLOWLV );
	mpRare = (cComboBox*)GetChild( eUIID_NPCDEAL_SEARCHGRADE );
	mpEnhance = (cComboBox*)GetChild( eUIID_NPCDEAL_SEARCHENHANCE );
	mpTypeList = (cListFolderBox*)GetChild( eUIID_NPCDEAL_SEARCHFOLDERLIST );
	mpItemList = (cImageAndFontDataListBox*)GetChild( eUIID_NPCDEAL_SEARCHITEMLIST );
	mpSearch = GetChild( eUIID_NPCDEAL_SEARCHBUTTON );
	mpLeftPageButton = (cButton*)GetChild(eUIID_NPCDEAL_SEARCHPAGELEFT);
	mpRightPageButton = (cButton*)GetChild(eUIID_NPCDEAL_SEARCHPAGERIGHT);

	if( mpLeftPageButton )
		mpLeftPageButton->Hide();
	if( mpRightPageButton )
		mpRightPageButton->Hide();

	cUIPos pos;
	unsigned int rowHeight = mpItemList->GetRowHeight();
	for( int i = 0; i < 10; ++i )
	{
		mpBuyButton[i] = new cButton;
		if( mpBuyButton[i]->CreateBySkinName( "NpcDeal_SearchBuy", this, BUYBUTTON_ID + i ) == false )
		{
			assert(0);
			return false;
		}

		pos.mX = mpBuyButton[i]->GetRelativeRect().mLeft;
		pos.mY = mpBuyButton[i]->GetRelativeRect().mTop + ( i * rowHeight );

		mpBuyButton[i]->SetRelativePos( pos );

		mpBuyButton[i]->Hide();
	}

	if( mpRare )
	{
		// 전체
		mpRare->AddItem( GAMERESOURCEMAN->GetItemToolTipText( 2000909 ), (void*)ALL_INVENTORY_RARE );

		// 등급
		for( int i = 0; i < 8; ++i )
		{
			mpRare->AddItem( GAMERESOURCEMAN->GetItemToolTipText( 2000901 + i ), (void*)(eTOOLTIP_MALL+i) );
		}

		mpRare->SetSelect( 0 );
	}

	if( mpEnhance )
	{
		mpEnhance->AddItem( GAMERESOURCEMAN->GetItemToolTipText( 2000909 ), (void*)ALL_INVENTORY_ENHANCED );

		cStringT str;
		for( int i = 0; i <= MAX_ITEM_ENHANCED; ++i )
		{
			str.Format( GAMERESOURCEMAN->GetItemToolTipText( 2000087 ), i );
			mpEnhance->AddItem( (LPCTSTR)str.Cstr(), (void*)i );	
		}

		mpEnhance->SetSelect( 0 );
	}

	///
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* pHighSkin = pSkin->GetNodeSkin( "NpcDeal_FolderCheck" );
	cUINodeSkin* pHighNoSkin = pSkin->GetNodeSkin( "NpcDeal_FolderNoCheck" );
	if( pHighSkin && pHighNoSkin )
	{
		mpHighImage = new cUIImage( pHighSkin->mpTexture );

		unsigned int tx = pHighSkin->mSkinInfo->mTexX;
		unsigned int ty = pHighSkin->mSkinInfo->mTexY;
		unsigned int w = pHighSkin->mSkinInfo->mWidth;
		unsigned int h = pHighSkin->mSkinInfo->mHeight;
		int x = pHighSkin->mSkinInfo->mX;
		int y = pHighSkin->mSkinInfo->mY;

		mpHighImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpHighImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		unsigned int tx2 = pHighNoSkin->mSkinInfo->mTexX;
		unsigned int ty2 = pHighNoSkin->mSkinInfo->mTexY;

		mpTypeList->SetHighNoCheckPos( tx, ty );
		mpTypeList->SetHighCheckPos( tx2, ty2 );
		mpTypeList->SetLowGap( w );
	}

	mpPageNumPos = pSkin->GetNodeSkin( "NpcDeal_Page_Text_Pos" );
	if( !mpPageNumPos )
	{
		assert(0);
		return false;
	}


	/// 
	mpTypeList->ShowNumber( false );

	/// 타입 정보
	SetTypeList();

	Clear();
	return true;
}

void cNpcDealSearchSheet::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mpItemList )
	{
		unsigned int rowHeight = mpItemList->GetRowHeight();

		for( int i = 0; i < 10; ++i )
		{
			if( mpBuyButton[i] )
			{
				cUIPos pos;
				pos.mX = mpBuyButton[i]->GetRelativeRect().mLeft;
				pos.mY = mpBuyButton[i]->GetRelativeRect().mTop + ( i * rowHeight );

				mpBuyButton[i]->SetRelativePos( pos );
			}
		}
	}
}

void cNpcDealSearchSheet::UpdateRect()
{
	cUIWindow::UpdateRect();

	unsigned int totalTextWidth = 0;
	/// 최대 표현 페이지
	for( unsigned int idx = 0; idx < 10; ++idx )
	{
		if( mPageNum[idx].mIsShow == true )
			totalTextWidth += mPageNum[idx].mTextWidth;
	}

	if( mpPageNumPos )
	{
		unsigned int pageStartPos = mpPageNumPos->mSkinInfo->mX - (totalTextWidth/2) + mAbsoluteRect.mLeft;
		for( unsigned int idx = 0; idx < 10; ++idx )
		{
			/// 문자 위치;
			if( mPageNum[idx].mIsShow == true )
			{
				mPageNum[idx].mPos.mX  = pageStartPos;
				mPageNum[idx].mPos.mY  = mpPageNumPos->mSkinInfo->mY + mAbsoluteRect.mTop ;
				pageStartPos += mPageNum[idx].mTextWidth;
			}
		}

		/// 페이지 화살표 위치
		if( mpLeftPageButton )
			mpLeftPageButton->SetRelativePos( cUIPos( mpPageNumPos->mSkinInfo->mX - (totalTextWidth/2) - mpLeftPageButton->GetAbsoluteRect().GetWidth() , mpLeftPageButton->GetRelativeRect().mTop ) );

		if( mpRightPageButton )
			mpRightPageButton->SetRelativePos( cUIPos( mpPageNumPos->mSkinInfo->mX + (totalTextWidth/2), mpLeftPageButton->GetRelativeRect().mTop ) );
	}

	/// 텍스트 출력 영역 설정
	if( mPageNum[0].mIsShow == true )
	{
		mPageTextRect = cUIRect( mPageNum[0].mPos.mX, 
			mPageNum[0].mPos.mY,
			mPageNum[0].mPos.mX + totalTextWidth,
			mPageNum[0].mPos.mY +  FONTAGENT->GetTextHeight( cFontAgent::eFont_UI ));
	}
	else
		mPageTextRect = cUIRect( 0, 0, 0, 0 );
}

void cNpcDealSearchSheet::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mDealDelayTime != 0 )
	{
		if( IsNetLock() == false )
		{
			if( mDealDelayTime <= accumTime )
			{
				// 메세지박스 지우기
				UIMAN->DestroyMsgBox( eMBEVENT_NPCDEAL_ITEM_FIND );

				// 결과 후처리
				switch( mSaveResult )
				{
				case ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS:
					{
						// list set
						UpdateItemList();
					}
					break;
				case ERROR_NPC_ITEM_AGENT_SEARCH_FAIL:
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1508 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

						RollBackPage();
					}
					break;
				case ERROR_NPC_ITEM_AGENT_SEARCH_NOTYET:
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1509 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

						RollBackPage();
					}
					break;
				default:
					assert(0);
					break;
				}
			
				mDealDelayTime = 0;
			}
		}
	}
}

void cNpcDealSearchSheet::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	for( unsigned int cnt = 0; cnt < 10; ++cnt )
	{
		if( mPageNum[cnt].mIsShow )
			pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mPageNum[cnt].mPageNum.Cstr()), mPageNum[cnt].mPos.mX, mPageNum[cnt].mPos.mY, mPageNum[cnt].mColor );
	}
}

void cNpcDealSearchSheet::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
		/// 검색
	case eUIID_NPCDEAL_SEARCHBUTTON:
		{
			unsigned long accumtime = THEAPP->GetWorldAccumTime();
			if( accumtime < mDealDelayTime )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 1509 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
				return;
			}

			char type = -1;
			short subType = -1;

			unsigned int row  = mpTypeList->GetSelectRow();
			unsigned int column = mpTypeList->GetSelectRowColumn();

			sDealCategory* data = (sDealCategory*)mpTypeList->GetColumnData( row, column );
			if( data )
			{
				type = data->mHighKey;

				/// column[0]은 대분류만 선택된상태
				if( column > 0 )
					subType = data->mMiddleKey;
			}

			if( type == -1 )
			{
				assert(0);
				RollBackPage();
				return;
			}

			LPCTSTR itemName = mpNameEdit->GetText();
			int len = ::_tcslen(itemName);

			/// 옵션
			unsigned char lowLv = (unsigned char)mpLowLv->GetValue();		
			unsigned char highLv = (unsigned char)mpHighLv->GetValue();
			unsigned char enhance = (unsigned char)mpEnhance->GetSelectData();
			unsigned char rare = (unsigned char)mpRare->GetSelectData();

			/// 레벨조건 검사
			if( lowLv > highLv )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 1510 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
				RollBackPage();
				return;
			}

			// 세부타입 항목 설정 요청
			if( subType == -1 )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 1521 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
				RollBackPage();
				return;
			}

			/// 문자열 검사
			if( len > 0 && CheckItemName( itemName, len ) == false )
			{
				mpNameEdit->Clear();

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
									eMBEVENT_NONE, eSKIN_OK,
									GAMERESOURCEMAN->GetGameText( 1511 ),
									GAMERESOURCEMAN->GetGameText( 301 )
									);
				RollBackPage();
				return;
			}

			mSortData = 255;
			
			/*----------------
		   	//  검색 조건 */

			bool result = false;
			// 1. 세부 검색 ( type, subtype 기준 )
			if( len == 0 )
			{
				result = ITEMMAN->TryToNpcDealSearchType( 1, type, subType, lowLv, highLv, enhance, rare, mSortData );
			}
			// 2. 전체 검색 ( type, subtype, item name 기준 )
			else if( len > 0 )
			{
				result = ITEMMAN->TryToNpcDealSearchAll( 1, mpNameEdit->GetText(), type, subType, lowLv, highLv, enhance, rare, mSortData );
			}
			else
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 1526 ),
					GAMERESOURCEMAN->GetGameText( 301 )
					);
			}
	
			if( result == false )
			{
				RollBackPage();
				return;
			}

			//
			mTempName = mpNameEdit->GetText();
			mTempType = type;
			mTempSubType = subType;
			mTempLowLv = lowLv;
			mTempHighLv = highLv;
			mTempEnhance = enhance;
			mTempRare = rare;

			// 1초 지연 걸기
			mDealDelayTime = THEAPP->GetWorldAccumTime() + DEAL_DELAYTIME;

			// 검색중
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
								eMBEVENT_NPCDEAL_ITEM_FIND, eSKIN_NONE,
								GAMERESOURCEMAN->GetGameText( 1522 ), 
								GAMERESOURCEMAN->GetGameText( 301 )
								);
		}
		break;
	case eUIID_NPCDEAL_SEARCHITEMLEVEL:
	case eUIID_NPCDEAL_SEARCHPRICE:
		if( mDealSearchArr.GetSize() > 0 )
		{
			BYTE filed = ( id == eUIID_NPCDEAL_SEARCHITEMLEVEL ) ? 1 : 0;
			BYTE flag = (mSort.flag == 0)? 1:0;

			mSortData = 0;

			/// true : 아이템명, false : 가격
			mSort.filed = filed;
			mSort.flag = flag;
			
			SendUpdateList( mSaveCurrentPage );
		}
		break;
	case eUIID_NPCDEAL_SEARCHPAGELEFT:
		{
			/// 현재 페이지 -> 앞 페이지의 첫번째 페이지
			int prevPageNum = ( ( mSaveCurrentPage -1)  / 10 ) * 10 + 1 - 10;
			SendUpdateList( prevPageNum );
		}
		break;
	case eUIID_NPCDEAL_SEARCHPAGERIGHT:
		{
			/// 현재 페이지 -> 뒤 페이지의 첫번째 페이지
			int nextPageNum = ( ( mSaveCurrentPage -1)  / 10 ) * 10 + 1 + 10;
			SendUpdateList( nextPageNum );
		}
		break;
	}

	/// 버튼 눌림
	if( id >= BUYBUTTON_ID && id < BUYBUTTON_ID + 10 )
	{
		int i = id - BUYBUTTON_ID;
		if( mpBuyButton[i] && mpBuyButton[i]->IsVisible() )
		{
			mpItemList->ChangeSelectRow( i );

			//
			NPC_ITEM_AGENT_SEARCH_RESULT* data = (NPC_ITEM_AGENT_SEARCH_RESULT*)mpItemList->GetExtraData( i );
			if( data )
			{
				/// 여러갯수가 있으면
				if( data->Inventory.count > 1 )
				{
					GAMEUI->ShowItemCountWindow( eMBEVENT_ITEMCOUNT_NPCDEALBUY, data->Idx, data->Inventory.ItemIndex, 
												 data->Inventory.count, data->Price );
				}
				/// 한개이면
				else
				{
					GAMEUI->ShowItemNpcDealBuyAskWindow( data->Idx, data->Inventory.ItemIndex, data->Inventory.count, data->Price );
				}
			}
		}
		else
			assert(0);
	}
}

void cNpcDealSearchSheet::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shift );

	if( mPageTextRect.ContainPoint( pos ) == true )
	{
		for( unsigned int idx = 0; idx < 10; ++idx )
		{
			if( mPageNum[idx].mIsShow == false )
				continue;

			if( pos.mX  >= mPageNum[idx].mPos.mX && pos.mX <= mPageNum[idx].mPos.mX + (int)mPageNum[idx].mTextWidth )
			{
				SendUpdateList( mPageNum[idx].mPageNum.ToInt() );
				return;
			}
		}
	}

}

void cNpcDealSearchSheet::OnListBoxImageHorvered( cUINode* , unsigned int , const cUIPos& pos, void* extraData )
{
	NPC_ITEM_AGENT_SEARCH_RESULT* data = (NPC_ITEM_AGENT_SEARCH_RESULT*)extraData;
	if( data )
	{
		UIMAN->ShowOtherItemTip( eTIP_ITEM, pos, data->Inventory );
	}
}

void cNpcDealSearchSheet::OnListBoxImageLeft( cUINode* , unsigned int )
{
	UIMAN->HideTip();
}

void cNpcDealSearchSheet::OnListBoxChanged( cUINode* , unsigned int id )
{
	if( !mpTypeList )
	{
		assert(0);
		return;
	}

	if( id == eUIID_NPCDEAL_SEARCHFOLDERLIST )
	{
		char type = -1;

		unsigned int row  = mpTypeList->GetSelectRow();
		unsigned int column = mpTypeList->GetSelectRowColumn();

		sDealCategory* data = (sDealCategory*)mpTypeList->GetColumnData( row, column );
		if( data )
		{
			type = data->mHighKey;
		}

		// 타입에 따른 레벨 구간 갱신
		if( type > -1 )
		{
			// 장비구류 (무기, 방어구, 악세서리) - 판매대행용 타입임
			if( type == 1 || type == 2 || type == 3 )
			{
				unsigned char level = HERO->GetLevel();

				unsigned char lowLv = 1;
				unsigned char highLv = 99;

				if( level < 10 )
				{
					lowLv = 1;
					highLv = level + 10;
				}
				else if( level > 245 )
				{
					lowLv = level - 10;
					highLv = 255;
				}
				else
				{
					lowLv = level - 10;
					highLv = level + 10;
				}

				if( mpHighLv )
					mpHighLv->SetValue( highLv );

				if( mpLowLv )
					mpLowLv->SetValue( lowLv );
			}
			// 그외
			else
			{
				if( mpHighLv )
					mpHighLv->SetValue( HIGHLEVEL );

				if( mpLowLv )
					mpLowLv->SetValue( LOWLEVEL );
			}
		}
	}
}

void cNpcDealSearchSheet::SetTypeList()
{
	if( !mpTypeList )
	{
		assert(0);
		return;
	}

	tArray<unsigned char>* highArr = ITEMMAN->GetDealHighArr();
	if( highArr )
	{
		// 실제 데이터 삽입
		for( unsigned int i = 0; i < highArr->GetSize(); ++i )
		{
			char idx = (*highArr)[i];
			tArray<void*>* dealArr = ITEMMAN->GetDealCategoryMap( idx );
			if( dealArr )
			{
				sDealCategory* highData = (sDealCategory*)(*dealArr)[0];
				if( !highData )
				{
					assert(0);
					continue;
				}

				/// 대분류 삽입
				LPCTSTR highName = GAMERESOURCEMAN->GetCategoryName( idx );
				mpTypeList->AddHighData( highData->mHighKey, highName, mDefaultColor, highData, mpHighImage, false );

				for( unsigned int j = 0; j < dealArr->GetSize(); ++j )
				{
					/// 중분류 삽입
					sDealCategory* data = (sDealCategory*)(*dealArr)[j];
					LPCTSTR middleName = GAMERESOURCEMAN->GetCategoryName( data->mMiddleKey );
					mpTypeList->AddLowData( highData->mHighKey, middleName, mDefaultColor, data );
				}
			}
		}
	}
	else
		assert(0);
}

void cNpcDealSearchSheet::SavePage( long startPage, long endPage )
{
	mSaveStartPage = startPage;
	mSaveEndPage = endPage;
}

// 아이템 정보 저장
bool cNpcDealSearchSheet::AddItem( NPC_ITEM_AGENT_SEARCH_RESULT* data )
{
	mDealSearchArr.PushBack( data );
	return true;
}

// UI 에 세팅
void cNpcDealSearchSheet::UpdateItemList()
{
	if( !mpItemList )
	{
		assert(0);
		return;
	}

	if( mpItemList )
		mpItemList->Clear();

	ClearPage();

	for( unsigned int i=0;i<mDealSearchArr.GetSize(); ++i )
	{
		NPC_ITEM_AGENT_SEARCH_RESULT* data = (NPC_ITEM_AGENT_SEARCH_RESULT*)mDealSearchArr[i];
		if( data )
		{
			cItemDefine* define = ITEMMAN->GetItemDefine( data->Inventory.ItemIndex );
			if( !define )
			{
				assert(0);
				return;
			}

			cIconParam* iconParam = define->GetIconParam();
			if( !iconParam )
			{
				assert(0);
				return;
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
				int tx = iconParam->mTexPos.mX;
				int ty = iconParam->mTexPos.mY;
				pImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
				pImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
				pImage->SetColor( color );
			}

			/// 테두리이미지
			cUIImage* edgeImage = 0;
			cIconParam* pTipParam = UIMAN->GetIconParam( 10 );
			if( pTipParam && define->GetTipType() != eTOOLTIP_NONE )
			{
				edgeImage = new cUIImage( pTipParam->mpTexture );

				unsigned int tx = pTipParam->mTexPos.mX + ( define->GetTipType() * ICON_SIZE );
				unsigned int ty = pTipParam->mTexPos.mY;
				edgeImage->SetTextureRect( tx, ty, tx + ICON_SIZE, ty + ICON_SIZE );
				edgeImage->SetScreenRect( cUIRect( 0, 0, ICON_SIZE, ICON_SIZE ) );
			}

			/// 
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

			/// 아이템명
			TCHAR name[ITENNAME_LEN] = {0,};
			TCHAR temp[ITENNAME_LEN] = {0,};

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
				return;
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
			mpItemList->AddColume( row, data->CharacterName, mDefaultColor );

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
			assert(0);
	}

	// 페이지 갱신
	ChangePage( mSaveStartPage );
}

/// 페이지 버튼 등 재 세팅
void cNpcDealSearchSheet::UpdatePage()
{
	if( mpItemList )
	{
		/// 리스트 박스 갱신
		mpItemList->ChangeTopIndex( mSaveCurrentPage * 10 );

		/// 버튼 갱신
		int count = mpItemList->GetRowCount() - mpItemList->GetCurrentTopIndex();

		for( int i = 0; i < 10; ++i )
		{
			if( i < count )
				mpBuyButton[i]->Show();
			else
				mpBuyButton[i]->Hide();
		}
	}
}

void cNpcDealSearchSheet::ChangePage( unsigned int page )
{
	if( page > 0 )
	{
		mSaveCurrentPage = page;
		SetSelectPage( page );
		UpdatePage();

		// 백업
		mTempPage = page;
		mTempSortData = mSortData;
	}
}

// 데이터 롤백
void cNpcDealSearchSheet::RollBackPage()
{
	if( mTempPage > 0 )
	{
		mSaveCurrentPage = mTempPage;
		SetSelectPage( mSaveCurrentPage );
		UpdatePage();


		mSortData = mTempSortData;
	}
}

bool cNpcDealSearchSheet::CheckItemName( LPCTSTR word, unsigned int len )
{
	if( len <= 0 )
		return false;

	/// 내용에 " "를 제외한 문자가 입력되었는지 검사후 send
	bool search = false;
	LPCTSTR str = word;
	for( unsigned int i = 0;i < len; ++i )
	{
		if( *str != 32 )
		{
			search = true;
			break;
		}
		else
			str++;
	}

	return search;
}

// 구입 요청
void cNpcDealSearchSheet::SendDealBuy( long idx, unsigned int count )
{
	if( !(mSaveCurrentPage >= 0 && mSaveCurrentPage <= mSaveEndPage) )
	{
		assert(0);
		return;
	}

	ITEMMAN->TryToNpcDealBuy( idx, mSaveCurrentPage, (short)count, (LPCTSTR)mTempName.Cstr(), 
							  mTempType, mTempSubType, mTempLowLv, mTempHighLv, 
							  mTempEnhance, mTempRare, mSortData );
}

// 목록 재 요청 보내기
void cNpcDealSearchSheet::SendUpdateList( unsigned int pageNum )
{
	if( pageNum == 0 )
		pageNum = mSaveCurrentPage;

	unsigned long accumtime = THEAPP->GetWorldAccumTime();
	if( accumtime < mDealDelayTime )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1509 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
		return;
	}

	if( !(pageNum >= 0 && pageNum <= mSaveEndPage) )
	{
	 	assert(0);
		return;
	}

	/// 레벨조건 검사
	if( mTempLowLv > mTempHighLv )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1510 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

		RollBackPage();
		return;
	}

	if( mTempType == -1 )
		return;

	// 세부타입 항목 설정 요청
	if( mTempSubType == -1 )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1521 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

		RollBackPage();
		return;
	}

	/// 문자열 검사
	if( mTempName.GetLength() > 0 && CheckItemName( (LPCTSTR)mTempName.Cstr(), mTempName.GetLength() ) == false )
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NONE, eSKIN_OK,
							GAMERESOURCEMAN->GetGameText( 1511 ),
							GAMERESOURCEMAN->GetGameText( 301 )
							);

		RollBackPage();
		return;
	}

	/*----------------
	//  검색 조건 */

	bool result = false;
	// 1. 세부 검색 ( type, subtype 기준 )
	if( mTempName.GetLength() == 0 )
	{
		result = ITEMMAN->TryToNpcDealSearchType( pageNum, mTempType, mTempSubType, 
													mTempLowLv, mTempHighLv, mTempEnhance, mTempRare, mSortData );
	}
	// 2. 전체 검색 ( type, subtype, item name 기준 )
	else if( mTempName.GetLength() > 0 )
	{
		result = ITEMMAN->TryToNpcDealSearchAll( pageNum, (LPCTSTR)mTempName.Cstr(), 
													mTempType, mTempSubType, 
													mTempLowLv, mTempHighLv, mTempEnhance, mTempRare, mSortData );
	}
	else
	{
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
			eMBEVENT_NONE, eSKIN_OK,
			GAMERESOURCEMAN->GetGameText( 1526 ),
			GAMERESOURCEMAN->GetGameText( 301 )
			);
	}

	if( result == false )
	{
		RollBackPage();
	}
	else
	{
		// 1초 지연 걸기
		mDealDelayTime = THEAPP->GetWorldAccumTime() + DEAL_DELAYTIME;

		// 검색중
		UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_NPCDEAL_ITEM_FIND, eSKIN_NONE,
							GAMERESOURCEMAN->GetGameText( 1522 ), 
							GAMERESOURCEMAN->GetGameText( 301 )
							);
	}
}

void cNpcDealSearchSheet::OnNetLock( int lockTry )
{
	cUINode::NetLock( lockTry );
}

void cNpcDealSearchSheet::OnNetUnLock( int lockTry )
{
	cUINode::NetUnLock( lockTry );
}

void cNpcDealSearchSheet::SetSelectPage( unsigned int page )
{
	if( page == 0 )
	{
		ClearPage();
		return;
	}

	unsigned int index = (page-1)%10;
	/// 선택된 페이지 색 바꾸기
	mPageNum[index].mColor = COLOR_GREEN;

	unsigned int pageStartNum = ( (page -1)  / 10 ) * 10 + 1; /// 일의자리 지우기
	unsigned int pageEndNum   = pageStartNum + 9;
	if( pageEndNum > (unsigned int)mSaveEndPage )
	{
		pageEndNum = (unsigned int)mSaveEndPage;
		mpRightPageButton->Hide();
	}
	else
		mpRightPageButton->Show();

 	if( pageEndNum <= 10 )
 		mpLeftPageButton->Hide();
 	else
 		mpLeftPageButton->Show();

	int pageCountMax = pageEndNum - pageStartNum;
	unsigned int totalTextWidth = 0;

	if( pageCountMax >= 10 || pageCountMax < 0 )
		pageCountMax = -1;

	/// 문자 시작 위치 = 윈도우 문자 출력 중심 좌표 - (총문자 길이/2) 
	/// 문자   끝 위치 = 윈도우 문자 출력 중심 좌표 + (총문자 길이/2)
	/// 왼쪽   화살표 시작 위치 = 문자 시작 위치 - 이미지 너비;
	/// 오른쪽 화살표 시작 위치 = 문자 끝   위치;
	/// 각 문자 시작 위치 = 출력된 문자들의 길이의 합

	/// 최대 표현 페이지
	for( int idx = 0; idx <= pageCountMax; ++idx )
	{
		mPageNum[idx].mPageNum.Format(_T(" %d "), pageStartNum + idx );
		mPageNum[idx].mTextWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mPageNum[idx].mPageNum.Cstr(), mPageNum[idx].mPageNum.GetLength() );
		/// 문자 길이 +;
		totalTextWidth += mPageNum[idx].mTextWidth;
	}

	unsigned int pageStartPos = mpPageNumPos->mSkinInfo->mX - (totalTextWidth/2) + mAbsoluteRect.mLeft;
	for( int idx = 0; idx <= pageCountMax; ++idx )
	{
		/// 문자 위치;
		mPageNum[idx].mPos.mX  = pageStartPos;
		mPageNum[idx].mPos.mY  = mpPageNumPos->mSkinInfo->mY + mAbsoluteRect.mTop ;
		pageStartPos += mPageNum[idx].mTextWidth;
		mPageNum[idx].mIsShow = true;
	}

	for( int idx = pageCountMax+1; idx < 10; ++idx )
		mPageNum[idx].mIsShow = false;

	/// 페이지 화살표 위치
	if( mpLeftPageButton )
		mpLeftPageButton->SetRelativePos( cUIPos( mpPageNumPos->mSkinInfo->mX - (totalTextWidth/2) - mpLeftPageButton->GetAbsoluteRect().GetWidth() , mpLeftPageButton->GetRelativeRect().mTop ) );

	if( mpRightPageButton )
		mpRightPageButton->SetRelativePos( cUIPos( mpPageNumPos->mSkinInfo->mX + (totalTextWidth/2), mpLeftPageButton->GetRelativeRect().mTop ) );

	/// 텍스트 출력 영역 설정
	if( mPageNum[0].mIsShow == true )
	{
		mPageTextRect = cUIRect( mPageNum[0].mPos.mX, 
								 mPageNum[0].mPos.mY,
								 mPageNum[0].mPos.mX + totalTextWidth,
								 mPageNum[0].mPos.mY +  FONTAGENT->GetTextHeight( cFontAgent::eFont_UI ));
	}
	else
		mPageTextRect = cUIRect( 0, 0, 0, 0 );
}

void cNpcDealSearchSheet::ClearPage()
{
	for( unsigned int cnt = 0; cnt < 10; ++cnt )
	{
		mPageNum[cnt].mColor = COLOR_WHITE;
		mPageNum[cnt].mIsShow = false;
	}

	if( mpLeftPageButton )
		mpLeftPageButton->Hide();

	if( mpRightPageButton )
		mpRightPageButton->Hide();

	mPageTextRect = cUIRect( 0, 0, 0, 0 );

	for( int i = 0; i < 10; ++i )
	{
		if( mpBuyButton[i] )
			mpBuyButton[i]->Hide();
	}
}
