#include "StdAfx.h"
#include "tarotwindow.h"

#include "TarotOpenWindow.h"
#include "TarotCheckWindow.h"
#include "TarotSelectWindow.h"
#include "TarotResultWindow.h"
//
#include "ItemManager.h"
#include "TarotManager.h"
//
#include "CardImage.h"
#include "ItemIcon.h"
#include "TipWindow.h"
#include "Application.h"
#include "FontAgent.h"
#include "PlaneObject.h"
#include "UIImage.h"
//
#include "SoundSystem.h"
#include "GameResourceManager.h"
//
#include "ObjectManager.h"
#include "Hero.h"

cTarotWindow::cTarotWindow()
: mpTarotSkin(0)
, mpOpenWindow(0)
, mpCheckWindow(0)
, mpSelectWindow(0)
, mpResultWindow(0)
, mpNpcCheckWindow(0)
, mpNpcSelectWindow(0)
, mpNpcResultWindow(0)
, mpSpreadInfo(0)
, mPrice(0)
, mpSelectImage(0)
, mSelectArrIndex(-1)
, mpSpreadBackImg(0)
, mpSpreadBackSkin(0)
, mMaxSelect(0)
, mSelectCardCount(0)
, mShowReviewIndex(0)
, mShowSpreadBack( true )
, mEnableOpenDrama( false )
, mTarotFront( true )
, mCurrentPattern(0)
, mTarotFadeState(0)
, mFortuneData(0)
, mCardHalfWidth(0)
, mCardHalfHeight(0)
{
	memset(mpIcon, 0, sizeof(mpIcon));
	memset(mNpcTarotResult, 0, sizeof(mNpcTarotResult));
	memset(mNpcSpreadIndex, 0, sizeof(mNpcSpreadIndex));
	memset(mNpcTarotIndex, 0, sizeof(mNpcTarotIndex));

	mCardTotalPoint = 0;
	mCurrentResult = 0;
	mSelectSpreadIndex = 0;
	mStartTime = 0;
	mNpcSpreadCount = 0;
}

cTarotWindow::~cTarotWindow()
{
	SAFE_DELETE( mpSpreadBackImg );
	SAFE_DELETE( mpSelectImage );
	mCardArr.Clear();
}

void cTarotWindow::Open()
{
}

void cTarotWindow::Close()
{
	cUIWindow::Close();

	if( mpOpenWindow )
		mpOpenWindow->Hide();
	if( mpCheckWindow )
		mpCheckWindow->Hide();
	if( mpSelectWindow )
		mpSelectWindow->Hide();
	if( mpResultWindow )
		mpResultWindow->Hide();
	if( mpNpcCheckWindow )
		mpNpcCheckWindow->Hide();
	if( mpNpcSelectWindow )
		mpNpcSelectWindow->Hide();
	if( mpNpcResultWindow )
		mpNpcResultWindow->Hide();
	
	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		if( mpIcon[i] )
		{
			mpIcon[i]->SetItemIndex(0);
			mpIcon[i]->ChangeImage(0);
			mpIcon[i]->SetColor( NiColorA::WHITE );

			mStringData[i].spreadName.Clear();
			mStringData[i].color = mDefaultColor;
		}
	}

	mPrice = 0;
	mTitle.Clear();
	mSelectArrIndex = -1;
	mSelectCardCount = 0;
	mShowReviewIndex = 0;
	mFortuneData = 0;

	memset(mNpcTarotIndex, 0, sizeof(mNpcTarotIndex));
	memset(mNpcTarotResult, 0, sizeof(mNpcTarotResult));
}

void cTarotWindow::OnShow()
{
	cUIWindow::OnShow();

	/// 화면 중앙에 출력
	int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
	int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
	SetRelativePos( cUIPos( x, y) );
}

void cTarotWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpOpenWindow )
		mpOpenWindow->Hide();
	if( mpCheckWindow )
		mpCheckWindow->Hide();
	if( mpSelectWindow )
		mpSelectWindow->Hide();
	if( mpResultWindow )
		mpResultWindow->Hide();
	if( mpNpcCheckWindow )
		mpNpcCheckWindow->Hide();
	if( mpNpcSelectWindow )
		mpNpcSelectWindow->Hide();
	if( mpNpcResultWindow )
		mpNpcResultWindow->Hide();
	
	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		if( mpIcon[i] )
		{
			mpIcon[i]->SetItemIndex(0);
			mpIcon[i]->ChangeImage(0);
			mpIcon[i]->SetColor( NiColorA::WHITE );

			mStringData[i].spreadName.Clear();
			mStringData[i].color = mDefaultColor;
		}
	}

	mPrice = 0;
	mTitle.Clear();
	mSelectArrIndex = -1;
	mSelectCardCount = 0;
	mShowReviewIndex = 0;
	mFortuneData = 0;

	memset(mNpcTarotIndex, 0, sizeof(mNpcTarotIndex));
	memset(mNpcTarotResult, 0, sizeof(mNpcTarotResult));
}

bool cTarotWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// managed child window
	mpOpenWindow = (cTarotOpenWindow*)GetChild( eUIID_GAME_TAROTOPEN_WINDOW );
	mpCheckWindow = (cTarotCheckWindow*)GetChild( eUIID_GAME_TAROTCHECK_WINDOW );
	mpSelectWindow = (cTarotSelectWindow*)GetChild( eUIID_GAME_TAROTSELECT_WINDOW );
	mpResultWindow = (cTarotResultWindow*)GetChild( eUIID_GAME_TAROTRESULT_WINDOW );
	mpNpcCheckWindow = (cNpcTarotCheckWindow*)GetChild( eUIID_GAME_NPCTAROTCHECK_WINDOW );
	mpNpcSelectWindow = (cNpcTarotSelectWindow*)GetChild( eUIID_GAME_NPCTAROTSELECT_WINDOW );
	mpNpcResultWindow = (cNpcTarotResultWindow*)GetChild( eUIID_GAME_NPCTAROTRESULT_WINDOW );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 카드이미지 스킨
	mpTarotSkin = pSkin->GetNodeSkin( "TarotCard" );
	if( mpTarotSkin == 0 )
	{
		assert( 0 && "failed to find card Image skin" );
		return false;
	}

	// 스프레드 이미지 생성
	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		mpIcon[i] = new cItemIcon;
		if( mpIcon[i]->CreateBySkinName( "SpreadIcon", this, i + 30 ) == false )
		{
			assert( 0 && "failed to create spread icon" );
			return false;
		}

		cUIRect iconRc = mpIcon[i]->GetRelativeRect();
		unsigned int y = iconRc.mTop + ( i % MAX_SPREAD ) * ( iconRc.GetHeight() + SPREAD_GAPY );

		/// 좌표 세팅 
		cUIPos iconPos( iconRc.mLeft, y );
		mpIcon[i]->SetRelativePos( iconPos );
		mpIcon[i]->Hide();
	}

	/// 관리용 카드 이미지 생성..
	for( int i = 0; i < MAX_TAROT_CARDS; ++i )
	{	
		cCardImage* cardImg = new cCardImage;
		if( cardImg->CreateBySkinName( "TarotCard", this, i + 1) == false  )
		{
			assert( 0 && "failed to create card image" );
			return false;
		}

		mDefaultCardRect = cardImg->GetRelativeRect();
		unsigned int x = mDefaultCardRect.mLeft + ( i % 5 ) * ( mDefaultCardRect.GetWidth() + TAROT_GAPX );
		unsigned int y = mDefaultCardRect.mTop + ( i / 5 ) * ( mDefaultCardRect.GetHeight() + TAROT_GAPY );	

		/// 좌표 세팅
		cUIPos imgPos( x, y );
		cardImg->SetRelativePos( imgPos );
		cardImg->Show();

		mCardArr.PushBack( cardImg );
	}

	/// 선택 이미지 생성
	cUINodeSkin* imageSkin = pSkin->GetNodeSkin( "Selection" );
	if( imageSkin )
	{
		unsigned int tx = imageSkin->mSkinInfo->mTexX;
		unsigned int ty = imageSkin->mSkinInfo->mTexY;
		unsigned int tw = imageSkin->mSkinInfo->mWidth;
		unsigned int th = imageSkin->mSkinInfo->mHeight;

		mpSelectImage = new cPlaneObject;

		if( mpSelectImage->Create( imageSkin->mpTexture, 0, 0, tw, th,
								   tx, ty, tx + tw, ty + th ) == false )
		{
			assert( 0 && "failed to create select image");
			return false;
		}
	}

	/// 스프레드 배경 이미지 생성
	mpSpreadBackSkin = pSkin->GetNodeSkin( "SpreadBackImage" );
	if( mpSpreadBackSkin && mpSpreadBackSkin->mpTexture )
	{
		mpSpreadBackImg = new cUIImage( mpSpreadBackSkin->mpTexture );

		/// 텍스처 영역 설정
		unsigned int tx = mpSpreadBackSkin->mSkinInfo->mTexX;
		unsigned int ty = mpSpreadBackSkin->mSkinInfo->mTexY;
		unsigned int w = mpSpreadBackSkin->mSkinInfo->mWidth;
		unsigned int h = mpSpreadBackSkin->mSkinInfo->mHeight;
		unsigned int x = mpSpreadBackSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpSpreadBackSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpSpreadBackImg->SetTextureRect( tx, ty, tx + w, ty + h );
		mpSpreadBackImg->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	cString str;
	cUINodeSkin* nodeSkin = 0;
	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		str.Format( "SelectRect%02d", i );
		nodeSkin = pSkin->GetNodeSkin( str.Cstr() );

		int left = nodeSkin->mSkinInfo->mX;
		int right = nodeSkin->mSkinInfo->mY;
		unsigned int width = nodeSkin->mSkinInfo->mWidth;
		unsigned int height = nodeSkin->mSkinInfo->mHeight;

		mSelectRect[i].Set( left, right, left + width, right + height );
	}

	cCardImage* pCardImg = (cCardImage*)mCardArr[0];
	mCardHalfWidth  = unsigned int(pCardImg->GetRelativeRect().GetWidth() * 0.5f);
	mCardHalfHeight = unsigned int(pCardImg->GetRelativeRect().GetHeight() * 0.5f);
	return true;
}

void cTarotWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		cUIRect iconRc = mpIcon[i]->GetRelativeRect();
		unsigned int y = iconRc.mTop + ( i % MAX_SPREAD ) * ( iconRc.GetHeight() + SPREAD_GAPY );

		/// 좌표 세팅 
		cUIPos iconPos( iconRc.mLeft, y );
		mpIcon[i]->SetRelativePos( iconPos );
	}

	/// 결과창에서는 스프레드 형태에 맞춘다
	if( TAROTMAN->GetCurrentPage() == ePAGE_RESULT )
	{
		for( unsigned int i=0; i<MAX_TAROT_CARDS; i++ )
		{
			cCardImage* pCardImg = (cCardImage*)mCardArr[i];
			if( pCardImg )
			{
				unsigned int cx = mpSpreadInfo->mParam[i].mX;
				unsigned int cy = mpSpreadInfo->mParam[i].mY;
				cUIPos cardPos( cx, cy );
				pCardImg->SetRelativePos( cardPos );
			}
		}
	}
	else if( TAROTMAN->GetCurrentPage() == ePAGE_NPCRESULT )
	{
		sNpcSpread* spread = TAROTMAN->GetNpcSpread( mSelectSpreadIndex );
		if( spread )
		{
			for( unsigned int i = 0; i < MAX_TAROT_CARDS; i++ )
			{
				cCardImage* pCardImg = (cCardImage*)mCardArr[i];
				assert(pCardImg);
				if( pCardImg )
				{
					if( i >= (int)mMaxSelect )
					{
						pCardImg->Hide();
						continue;
					}

					unsigned long cardIndex = mNpcTarotResult[i].TarotCardIndex;
					sNpcTarotCard* data = TAROTMAN->GetNpcTarot( cardIndex );
					assert(data);
					if( data )
					{
						cCardParam* pParam = TAROTMAN->GetImageParam( data->mImageIndex );
						assert(pParam);
						if( pParam )
						{
							unsigned int cx = spread->mParam[i].mX;
							unsigned int cy = spread->mParam[i].mY;
							cUIPos cardPos( cx, cy );
							pCardImg->SetRelativePos( cardPos );
						}
					}
				}
			}
		}
	}
	else
	{
		/// 그외에는 그냥 출력
		for( unsigned int i=0; i<MAX_TAROT_CARDS; i++ )
		{
			cCardImage* pCardImg = (cCardImage*)mCardArr[i];
			if( pCardImg )
			{
				/// 카드 위치 초기화
				unsigned int x = mDefaultCardRect.mLeft + ( i % 5 ) * ( mDefaultCardRect.GetWidth() + TAROT_GAPX );
				unsigned int y = mDefaultCardRect.mTop + ( i / 5 ) * ( mDefaultCardRect.GetHeight() + TAROT_GAPY );	
				cUIPos imgPos( x, y );

				/// 이미지 데이터 획득 
				pCardImg->SetRelativePos( imgPos );
			}
		}
	}

	/// 화면 중앙에 출력
	{
		int x = (int)(( GetScreenWidth() - GetAbsoluteRect().GetWidth() ) * 0.5f);
		int y = (int)(( GetScreenHeight() - GetAbsoluteRect().GetHeight() ) * 0.5f);
		SetRelativePos( cUIPos( x, y) );
	}
}

void cTarotWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mSelectArrIndex > -1 && mpSelectImage )
	{
		mpSelectImage->SetScreenRect( mSelectRect[mSelectArrIndex].mLeft + GetAbsoluteRect().mLeft, 
			mSelectRect[mSelectArrIndex].mTop + GetAbsoluteRect().mTop, 
			mSelectRect[mSelectArrIndex].GetWidth(), mSelectRect[mSelectArrIndex].GetHeight() );
	}

	if( mpSpreadBackSkin && mpSpreadBackImg )
	{
		unsigned int x = mpSpreadBackSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpSpreadBackSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpSpreadBackImg->SetPos( x, y );
	}
}

void cTarotWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	if( mEnableOpenDrama )
	{
		if( mCurrentPattern == 0 )
		{
			assert(0);
			for( unsigned int i=0;i<MAX_TAROT_CARDS; i++ )
			{
				cCardImage* pCardImg = (cCardImage*)mCardArr[i];
				if( pCardImg )
				{
					pCardImg->Show();
					pCardImg->ShowImage( mTarotFront, true, mTarotFadeState );
				}
			}
			mEnableOpenDrama = false;

			mpCheckWindow->SetEnabled( true );
			mpSelectWindow->SetEnabled( true );
		}
		else
		{
			unsigned int count = 0;
			for( unsigned int i=0;i<MAX_TAROT_CARDS;i++ )
			{
				cCardImage* pCardImg = (cCardImage*)mCardArr[i];

				if( mCurrentPattern[i] <= (accumTime - mStartTime) )
				{
					if( pCardImg && pCardImg->IsShow() == !mTarotFront )
					{
						pCardImg->ShowImage( mTarotFront, true, mTarotFadeState );
						pCardImg->Show();
					}
				}

				if( pCardImg )
				{
					if( pCardImg->IsShow() == mTarotFront )
						count++;
				}
				else
					count++;
			}

			if( count >= MAX_TAROT_CARDS )
			{
				mEnableOpenDrama = false;

				mpCheckWindow->SetEnabled( true );
				mpSelectWindow->SetEnabled( true );
			}
		}
	}
}

void cTarotWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	if( mpSpreadBackImg && mShowSpreadBack )
		mpSpreadBackImg->Draw();

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
			((cUINode*)(*i))->OnRender(pKeeper);
	}

	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		if( mpIcon[i] && mpIcon[i]->IsVisible() && mStringData[i].spreadName.IsEmpty() == false )
		{
			cUIRect rect = mpIcon[i]->GetAbsoluteRect();
			int x = rect.mRight + 3;
			int y = rect.mTop + (int)((rect.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_UI)) * 0.5f);
			pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mStringData[i].spreadName.Cstr()), x, y, mStringData[i].color );
		}
	}
	
	if( mpSelectImage && mSelectArrIndex > -1 )
		mpSelectImage->Draw();

	pKeeper->DrawAll();
}


void cTarotWindow::OnCommand( cUINode* pnode, unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_TAROT_CLOSE:
		{
			switch( TAROTMAN->GetCurrentPage() )
			{
			case ePAGE_OPEN:
				{
					// 개설자
					MSG_REQ_TAROT_READER_CLOSE msg;
					msg.Category = NM_TAROT;
					msg.Protocol = NM_TAROT_READER_CLOSE_REQ;
					NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

					NetLock( ePAGE_OPEN );
				}
				break;
			case ePAGE_VIEW:
			case ePAGE_SELFVIEW:
				{
					// 이용자
					eTarotType type = TAROTMAN->GetTarotType();
					if( type == eTAROT_USER )
					{
						MSG_REQ_TAROT_SEEKER_CLOSE msg;
						msg.Category = NM_TAROT;
						msg.Protocol = NM_TAROT_SEEKER_CLOSE_REQ;
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

						NetLock( ePAGE_VIEW );
					}
					else if( type == eTAROT_SELF )
					{
						MSGROOT msg;
						msg.Category = NM_TAROT;
						msg.Protocol = NM_TAROT_SELF_CLOSE_REQ;
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

						NetLock( ePAGE_SELFVIEW );
					}
					else if( type == eTAROT_TUTORIAL )
					{
						MSG_REQ_TAROT_TUTORIAL_SEEKER_CLOSE msg;
						msg.Category = NM_TAROT;
						msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_CLOSE_REQ;
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

						NetLock( ePAGE_VIEW );
					}
					else
						assert(0);
				}
				break;
			case ePAGE_SELECT:
			case ePAGE_RESULT:
				{
					eTarotType type = TAROTMAN->GetTarotType();
					if( type == eTAROT_USER )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_TAROTSEEKER_EXIT, eSKIN_YESNO,
							GAMERESOURCEMAN->GetGameText( 312 ), // 종료 경고 메시지
							GAMERESOURCEMAN->GetGameText( 301 )
							);
					}
					else if( type == eTAROT_SELF )
					{
						UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
							eMBEVENT_TAROTSEEKER_EXIT, eSKIN_YESNO,
							GAMERESOURCEMAN->GetGameText( 328 ), // 종료 경고 메시지
							GAMERESOURCEMAN->GetGameText( 301 )
							);
					}
					else if( type == eTAROT_TUTORIAL )
					{
						MSG_REQ_TAROT_TUTORIAL_SEEKER_CLOSE msg;
						msg.Category = NM_TAROT;
						msg.Protocol = NM_TAROT_TUTORIAL_SEEKER_CLOSE_REQ;
						NETWORK->SendNetworkMsg( (char*)&msg, sizeof(msg) );

						NetLock( ePAGE_SELECT );
						NetLock( ePAGE_SELFVIEW );
						NetLock( ePAGE_RESULT );
					}
				}
				break;
			case ePAGE_NPCVIEW:
				{
					HERO->CloseNpcTalk();
					TAROTMAN->ChangePage( ePAGE_NONE );
				}
				break;
			case ePAGE_NPCSELECT:
			case ePAGE_NPCRESULT:
				{
					UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
						eMBEVENT_NPCTAROT_EXIT, eSKIN_YESNO,
						GAMERESOURCEMAN->GetGameText( 312 ),
						GAMERESOURCEMAN->GetGameText( 301 )
						);
				}
				break;
			}
		}
		break;
	}
}

void cTarotWindow::OnImageHovered( cUINode* caller, unsigned int id, const cUIPos& pos )
{
	cCardImage* image = (cCardImage*)caller;
	
	if( image && image->IsShow() && image->GetTarotItemIdx() > 0 )
	{
		UIMAN->ShowItemTip( eTIP_ITEM, pos, image->GetTarotItemIdx(), USHRT_MAX, false );
	}
}

void cTarotWindow::OnIconHovered( cUINode* caller, unsigned int, const cUIPos& pos )
{
	cItemIcon* icon = (cItemIcon*)caller;

	if( icon && icon->GetItemIndex() > 0 )
		UIMAN->ShowItemTip( eTIP_ITEM, pos, icon->GetItemIndex(), USHRT_MAX, false );
}

void cTarotWindow::OnImageClicked( cUINode* pcaller, unsigned int id )
{
	if( pcaller == 0 )
	{
		assert(0);
		return;
	}

	if( IsEnalbeOpenDrama() == true )
		return;

	bool onSound = false;
	eTarotPage page = TAROTMAN->GetCurrentPage();
	if( page ==  ePAGE_SELECT )
	{
		onSound = OpenSelectCard( (cCardImage*)pcaller );
	}
	else if( page == ePAGE_NPCSELECT )
	{
		onSound = OpenSelectNpcCard( (cCardImage*)pcaller );
	}
	// 
	//ePAGE_SELFSELECT

	/// 카드 이미지 보여줄 때 나는 사운드 재생
	if( onSound )
		SOUNDSYS->Play2DSound( 3020 );
}

void cTarotWindow::OnIconClicked( cUINode* pcaller, unsigned int id )
{
	eTarotPage page = TAROTMAN->GetCurrentPage();
	if( !(page == ePAGE_SELECT || page == ePAGE_NPCSELECT) )
		return;

	for(unsigned int i = 0; i < MAX_SPREAD; i++)
	{
		cItemIcon* p = mpIcon[i];
		if( p )
		{
			cItemIcon* pIcon = (cItemIcon*)pcaller;	
			if( p == pIcon )
			{
				/// 선택이미지 설정
				if( mpSelectImage )
				{
					mSelectArrIndex = i;
					mpSelectImage->SetScreenRect( mSelectRect[i].mLeft + GetAbsoluteRect().mLeft, 
													mSelectRect[i].mTop + GetAbsoluteRect().mTop, 
													mSelectRect[i].GetWidth(), mSelectRect[i].GetHeight() );
				}

				if( page == ePAGE_SELECT )
				{
					SetSelectedSpread( p->GetItemIndex() );
				}
				else
				{
					SetSelectedNpcSpread( p->GetItemIndex() );
				}
			}
		}
	}
}

void cTarotWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, false, false, false );
}

void cTarotWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool )
{
	eTarotPage page = TAROTMAN->GetCurrentPage();
	if( !(page == ePAGE_SELECT || page == ePAGE_NPCSELECT) )
		return;

	for( int i = 0; i < MAX_SPREAD; ++i )
	{
		cUIRect rc = mSelectRect[i];

		rc.mLeft += GetAbsoluteRect().mLeft;
		rc.mTop += GetAbsoluteRect().mTop;
		rc.mRight = rc.mLeft + mSelectRect[i].GetWidth();
		rc.mBottom = rc.mTop + mSelectRect[i].GetHeight();

		if( rc.ContainPoint( pos ) == true )
		{
			unsigned long itemIndex = mpIcon[i]->GetItemIndex();
			if( itemIndex == 0 )
				break;

			/// 선택이미지 설정
			if( mpSelectImage )
			{
				mSelectArrIndex = i;
				mpSelectImage->SetScreenRect( mSelectRect[i].mLeft + GetAbsoluteRect().mLeft, 
											  mSelectRect[i].mTop + GetAbsoluteRect().mTop, 
											  mSelectRect[i].GetWidth(), mSelectRect[i].GetHeight() );
			}

			if( page == ePAGE_SELECT )
			{
				SetSelectedSpread( itemIndex );
			}
			else
			{
				SetSelectedNpcSpread( itemIndex );
			}
			break;
		}
	}
}


void cTarotWindow::ChangeTarotPage( unsigned char page )
{
	if( mpOpenWindow )
		mpOpenWindow->Hide();
	if( mpCheckWindow )
		mpCheckWindow->Hide();
	if( mpSelectWindow )
		mpSelectWindow->Hide();
	if( mpResultWindow )
		mpResultWindow->Hide();
	if( mpNpcCheckWindow )
		mpNpcCheckWindow->Hide();
	if( mpNpcSelectWindow )
		mpNpcSelectWindow->Hide();
	if( mpNpcResultWindow )
		mpNpcResultWindow->Hide();

	UIMAN->HideTip();

	/// 스프레드 배경이미지 보기
	mShowSpreadBack = true;

	switch( page )
	{
	case ePAGE_NONE:
		{
			Hide();
		}
		break;
	case ePAGE_OPEN:
		{
			ShowHeroTarotCard();

			Show();
			mpOpenWindow->Show( false );
			mpOpenWindow->SetEnabled( true );

			mpCheckWindow->Hide();
			mpCheckWindow->SetEnabled( false );
			mpSelectWindow->Hide();
			mpSelectWindow->SetEnabled( false );
			mpResultWindow->Hide();
			mpResultWindow->SetEnabled( false );
		}
		break;
	case ePAGE_VIEW:
		{
			Show();
			mpCheckWindow->Show( false );

			mpOpenWindow->Hide();
			mpSelectWindow->Hide();
			mpResultWindow->Hide();

			mpOpenWindow->SetEnabled( false );
			mpCheckWindow->SetEnabled( false );
			mpSelectWindow->SetEnabled( false );
			mpResultWindow->SetEnabled( false );

			for( int i = 0; i < MAX_TAROT_CARDS; ++i )
			{	
				unsigned int x = mDefaultCardRect.mLeft + ( i % 5 ) * ( mDefaultCardRect.GetWidth() + TAROT_GAPX );
				unsigned int y = mDefaultCardRect.mTop + ( i / 5 ) * ( mDefaultCardRect.GetHeight() + TAROT_GAPY );	
				cUIPos imgPos( x, y );

				cCardImage* pCardImg = (cCardImage*)mCardArr[i];
				assert(pCardImg);
				if( pCardImg )
				{
					pCardImg->SetLock( false );
					pCardImg->SetColor( NiColorA::WHITE );
					pCardImg->SetEnabled( true );
					pCardImg->SetRelativePos( imgPos );
					pCardImg->ShowImage( false );
					pCardImg->SetDirection(0);
					pCardImg->Show();
				}
			}

			mStartTime = THEAPP->GetWorldAccumTime();
			mCurrentPattern = GAMERESOURCEMAN->GetTarotPattern();
			mTarotFadeState = cCardImage::eFade_In;
			mTarotFront = true;
			mEnableOpenDrama = true;
		}
		break;
	case ePAGE_SELFVIEW:
		{
			ShowHeroTarotCard();

			Show();
			mpCheckWindow->Show( false );
					
			mpOpenWindow->Hide();
			mpSelectWindow->Hide();
			mpResultWindow->Hide();
			
			mpCheckWindow->SetEnabled( false );
			mpOpenWindow->SetEnabled( false );
			mpSelectWindow->SetEnabled( false );
			mpResultWindow->SetEnabled( false );

			for( int i = 0; i < MAX_TAROT_CARDS; ++i )
			{	
				unsigned int x = mDefaultCardRect.mLeft + ( i % 5 ) * ( mDefaultCardRect.GetWidth() + TAROT_GAPX );
				unsigned int y = mDefaultCardRect.mTop + ( i / 5 ) * ( mDefaultCardRect.GetHeight() + TAROT_GAPY );	
				cUIPos imgPos( x, y );

				cCardImage* pCardImg = (cCardImage*)mCardArr[i];
				assert(pCardImg);
				if( pCardImg )
				{
					pCardImg->SetLock( false );
					pCardImg->SetColor( NiColorA::WHITE );
					pCardImg->SetEnabled( true );
					pCardImg->SetRelativePos( imgPos );
					pCardImg->ShowImage( false );
					pCardImg->SetDirection(0);
					pCardImg->Show();
				}
			}

			mStartTime = THEAPP->GetWorldAccumTime();
			mCurrentPattern = GAMERESOURCEMAN->GetTarotPattern();
			mTarotFadeState = cCardImage::eFade_In;
			mTarotFront = true;
			mEnableOpenDrama = true;
		}
		break;
	case ePAGE_SELECT:
		{
			mSelectCardCount = 0;
			
			/// 기본 선택 설정
			SetSelectedSpread( mpIcon[0]->GetItemIndex() );
			if( mpSelectImage )
			{
				mSelectArrIndex = 0;
				mpSelectImage->SetScreenRect( mSelectRect[0].mLeft + GetAbsoluteRect().mLeft, 
												mSelectRect[0].mTop + GetAbsoluteRect().mTop, 
												mSelectRect[0].GetWidth(), mSelectRect[0].GetHeight() );
			}


			Show();
			mpSelectWindow->Show( false );

			mpOpenWindow->Hide();
			mpOpenWindow->SetEnabled( false );
			mpCheckWindow->Hide();
			mpCheckWindow->SetEnabled( false );
			mpSelectWindow->SetEnabled( false );
			mpResultWindow->Hide();
			mpResultWindow->SetEnabled( false );

			for( unsigned int i=0; i<MAX_TAROT_CARDS; i++ )
			{
				cCardImage* pCardImg = (cCardImage*)mCardArr[i];
				if( pCardImg )
				{
					pCardImg->ShowImage( true, false );
				}
				else
				{
					assert(0);
				}
			}

			mStartTime = THEAPP->GetWorldAccumTime();
			mCurrentPattern = GAMERESOURCEMAN->GetTarotPattern();
			mTarotFadeState = cCardImage::eFade_In;
			mTarotFront = false;
			mEnableOpenDrama = true;
		}
		break;
	case ePAGE_RESULT:
		{
			mShowSpreadBack = false;

			mSelectArrIndex = -1;
			mSelectCardCount = 0;
			mCardTotalPoint = 0;
			mCurrentResult = 0;
			for( unsigned int i=0;i<MAX_SPREAD;i++ )
				mpIcon[i]->Hide();

			Show();
			mpResultWindow->Show( false );

			mpOpenWindow->Hide();
			mpOpenWindow->SetEnabled( false );
			mpCheckWindow->Hide();
			mpCheckWindow->SetEnabled( false );
			mpSelectWindow->Hide();
			mpSelectWindow->SetEnabled( false );
			mpResultWindow->SetEnabled( true );

			for( unsigned int i=0; i<MAX_TAROT_CARDS; i++ )
			{
				cCardImage* pCardImg = (cCardImage*)mCardArr[i];
				assert(pCardImg);
				if( pCardImg )
				{
					if( i >= (int)mMaxSelect )
					{
						pCardImg->Hide();
						continue;
					}

					/// 개설자 소유타로카드 인덱스를 받아온다
					sResultInfo* info = TAROTMAN->GetSelectCardInfo( i );
					if( !info )
					{
						assert(0);
						return;
					}

					unsigned int itemIdx = info->TarotCardIndex;
					cCardParam* pParam = TAROTMAN->GetTarotParam( itemIdx );
					if( pParam == 0 )
					{
						assert(0);
						return;
					}

					unsigned int cx = mpSpreadInfo->mParam[i].mX;
					unsigned int cy = mpSpreadInfo->mParam[i].mY;

					cUIPos cardPos( cx, cy );
					pCardImg->SetRelativePos( cardPos );

					/// 카드 세팅
					pCardImg->Show();
					pCardImg->SetEnabled( false );
					pCardImg->SetImage( itemIdx, pParam );
					pCardImg->SetColor( NiColorA::WHITE );
					pCardImg->ShowImage( false, true, cCardImage::eFade_In );
					pCardImg->SetDirection( info->Dir );
				}
			}
		}
		break;
	case ePAGE_NPCVIEW:
		{
			/// 1. 카드 세팅
			for( int i = 0; i < MAX_TAROT_CARDS; ++i )
			{	
				sNpcTarotCard* data = TAROTMAN->GetNpcTarot( mNpcTarotIndex[i] );
				if( data )
				{
					cCardParam* pParam = TAROTMAN->GetImageParam( data->mImageIndex );
					if( pParam )
					{
						cCardImage* card = (cCardImage*)mCardArr[i];
						assert(card);
						if( card )
						{
							unsigned int x = mDefaultCardRect.mLeft + ( i % 5 ) * ( mDefaultCardRect.GetWidth() + TAROT_GAPX );
							unsigned int y = mDefaultCardRect.mTop + ( i / 5 ) * ( mDefaultCardRect.GetHeight() + TAROT_GAPY );	
							cUIPos imgPos( x, y );

							/// 카드 세팅
							card->SetLock( false );
							card->SetRelativePos( imgPos );
							card->SetColor( NiColorA::WHITE );
							card->SetEnabled( true );
							card->SetImage( mNpcTarotIndex[i], pParam );
							card->SetDirection(0);
							card->ShowImage( false );
							card->Show();
						}
					}
					else
					{
						assert( 0 && "error tarot image param" );
					}
				}
			}

			/// 2. 스프레드 세팅
			for( unsigned int i = 0; i < mNpcSpreadCount; ++i )
			{
				sNpcSpread* spread = TAROTMAN->GetNpcSpread( mNpcSpreadIndex[i] );
				cItemDefine* spreadDefine = ITEMMAN->GetItemDefine( mNpcSpreadIndex[i] );
				if( spread && spreadDefine )
				{
					mpIcon[i]->ChangeImage( spreadDefine->GetIconParam() );
					mpIcon[i]->SetItemIndex( mNpcSpreadIndex[i] );
					mpIcon[i]->Show();

					unsigned long compare = 0x00010000;
					compare = compare >> (4 * spread->mFortuneProp);

					unsigned long color = ((mFortuneData & compare) == compare ) ? COLOR_DARKGRAY : mDefaultColor;
					mStringData[i].spreadName	= spreadDefine->GetName();
					mStringData[i].color		= color;
				}
				else
				{
					assert(0);
				}
			}

			/// show
			mpNpcCheckWindow->Show();
			Show();

			mStartTime = THEAPP->GetWorldAccumTime();
			mCurrentPattern = GAMERESOURCEMAN->GetTarotPattern();
			mTarotFadeState = cCardImage::eFade_In;
			mTarotFront = true;
			mEnableOpenDrama = true;
		}
		break;
	case ePAGE_NPCSELECT:
		{
			mSelectCardCount = 0;
			mMaxSelect = 0;

			/// 기본 선택 설정
			if( mpSelectImage )
			{
				mSelectArrIndex = 0;
				mpSelectImage->SetScreenRect( mSelectRect[0].mLeft + GetAbsoluteRect().mLeft, 
												mSelectRect[0].mTop + GetAbsoluteRect().mTop, 
												mSelectRect[0].GetWidth(), mSelectRect[0].GetHeight() );
			}

			SetSelectedNpcSpread( mNpcSpreadIndex[0] );

			///
			mpNpcSelectWindow->Show();
			Show();

			mStartTime = THEAPP->GetWorldAccumTime();
			mCurrentPattern = GAMERESOURCEMAN->GetTarotPattern();
			mTarotFadeState = cCardImage::eFade_In;
			mTarotFront = false;
			mEnableOpenDrama = true;
		}
		break;
	case ePAGE_NPCRESULT:
		{
			mShowSpreadBack = false;
			mSelectCardCount = 0;
			mShowReviewIndex = 0;
			mSelectArrIndex = -1;
			for( unsigned int i = 0; i < MAX_SPREAD; i++ )
				mpIcon[i]->Hide();

			/// 결과 생성
			MakeNpcTarotResult();

			/// 스프레드 가져오기
			sNpcSpread* spread = TAROTMAN->GetNpcSpread( mSelectSpreadIndex );
			if( spread )
			{
				for( unsigned int i = 0; i < MAX_TAROT_CARDS; i++ )
				{
					cCardImage* pCardImg = (cCardImage*)mCardArr[i];
					assert(pCardImg);
					if( pCardImg )
					{
						if( i >= (int)mMaxSelect )
						{
							pCardImg->Hide();
							continue;
						}

						unsigned long cardIndex = mNpcTarotResult[i].TarotCardIndex;
						sNpcTarotCard* data = TAROTMAN->GetNpcTarot( cardIndex );
						assert(data);
						if( data )
						{
							cCardParam* pParam = TAROTMAN->GetImageParam( data->mImageIndex );
							assert(pParam);
							if( pParam )
							{
								unsigned int cx = spread->mParam[i].mX;
								unsigned int cy = spread->mParam[i].mY;
								cUIPos cardPos( cx, cy );
								pCardImg->SetRelativePos( cardPos );

								/// 카드 세팅
								pCardImg->SetEnabled( false );
								pCardImg->SetColor( NiColorA::WHITE );
								pCardImg->SetImage( cardIndex, pParam );
								pCardImg->SetDirection( mNpcTarotResult[i].Dir );
								pCardImg->ShowImage( false, true, cCardImage::eFade_In );
								pCardImg->Show();
							}
						}
					}
				}
			}

			

			mpNpcResultWindow->Show();
			Show();
		}
		break;
	default:
		return;
	}

}

void cTarotWindow::ShowHeroTarotCard()
{
	unsigned int spreadCount = 0;
	for( unsigned int i = 0; i<MAX_SPREAD; i++ )
	{
		if( mpIcon[i] )
			mpIcon[i]->Hide();
		else
			assert(0);

		mStringData[i].spreadName.Clear();
		mStringData[i].color = mDefaultColor;
		
		cItem* item = ITEMMAN->GetItem( (unsigned short)(INVENTORY_SPREAD_NORMAL_BEGIN + i) );
		if( !item )
		{
			assert(0);
			continue;
		}

		unsigned int spreadIdx = item->GetIndex();
		if( spreadIdx == 0 )
			continue;	

		cItemDefine* spreadDefine = ITEMMAN->GetItemDefine( spreadIdx );
		if( !spreadDefine )
		{
			assert(0);
			continue;
		}

		unsigned char type = spreadDefine->GetType();
		if( type != ITEM_SPREAD )
			continue;

		cIconParam* pIconParam = spreadDefine->GetIconParam();
		assert(pIconParam);
		if( pIconParam )
		{
			mpIcon[spreadCount]->SetItemIndex( spreadIdx );
			mpIcon[spreadCount]->ChangeImage( pIconParam );
			mpIcon[spreadCount]->SetColor( NiColorA::WHITE );
			mpIcon[spreadCount]->Show();

			mStringData[spreadCount].spreadName = spreadDefine->GetName();
			mStringData[spreadCount].color = mDefaultColor;

			spreadCount++;
		}
	}

	mCardTotalPoint = 0;
	for( unsigned int i=0; i<MAX_TAROT_CARDS; i++ )
	{
		cCardImage* pCardImg = (cCardImage*)mCardArr[i];
		if( pCardImg )
		{
			pCardImg->Hide();
		}
		else
		{
			assert(0);
		}

		/// 타롯슬롯에 있는 아이템 인덱스를 획득
		cItem* item = ITEMMAN->GetItem( (unsigned short)(INVENTORY_TAROT_NORMAL_BEGIN + i) );
		if( !item )
		{
			assert(0);
			continue;
		}

		unsigned int itemIdx = item->GetIndex();
		if( itemIdx == 0 )
			continue;

		unsigned char subType = item->GetDefine()->GetSubType();
		if( subType != ITEM_CARD_TAROT )
			continue;

		/// 총합 구하기 
		cItemTarot* pTarot = item->GetDefine()->GetTarotDefine();
		if( pTarot )
			mCardTotalPoint += (int)pTarot->GetStraightValue();

		/// 카드 위치 초기화
		unsigned int x = mDefaultCardRect.mLeft + ( i % 5 ) * ( mDefaultCardRect.GetWidth() + TAROT_GAPX );
		unsigned int y = mDefaultCardRect.mTop + ( i / 5 ) * ( mDefaultCardRect.GetHeight() + TAROT_GAPY );	
		cUIPos imgPos( x, y );

		/// 이미지 데이터 획득 
		cCardParam* pParam = TAROTMAN->GetTarotParam( itemIdx );
		if( pParam )
		{
			pCardImg->SetRelativePos( imgPos );
			pCardImg->SetDirection(0);

			pCardImg->SetImage( itemIdx, pParam );
			pCardImg->ShowImage( false );
			pCardImg->Show();
		}
		else
			assert( 0 && "error tarot image param" );
	}

	mStartTime = THEAPP->GetWorldAccumTime();
	mCurrentPattern = GAMERESOURCEMAN->GetTarotPattern();
	mTarotFront = true;
	mEnableOpenDrama = true;
}

void cTarotWindow::SetCardInfo( unsigned int index, unsigned int itemIndx, bool InitPoint )
{
	if( InitPoint == true )
		mCardTotalPoint = 0;

	cItemDefine* pDefine = ITEMMAN->GetItemDefine( itemIndx );
	if( pDefine == 0 )
	{
		assert(0);
		return;
	}

	cItemTarot* pTarot = pDefine->GetTarotDefine();
	if( pTarot == 0 )
	{
		assert(0);
		return;
	}

//	/// 총합 구하기 
	mCardTotalPoint += (int)pTarot->GetStraightValue();

	/// 이미지 데이터 획득 
	cCardParam* pParam = TAROTMAN->GetTarotParam( itemIndx );
	if( pParam )
	{
		cCardImage* pCardImg = (cCardImage*)mCardArr[index];
		if( pCardImg )
		{
			pCardImg->SetImage( itemIndx, pParam );
			pCardImg->ShowImage( true );
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		assert( 0 && "error tarot image param" );
	}
}

void cTarotWindow::SetSpreadInfo( unsigned int index, unsigned int itemIndex )
{
	cItemDefine* spreadDefine = ITEMMAN->GetItemDefine( itemIndex );
	if( spreadDefine )
	{
		cIconParam* pIconParam = spreadDefine->GetIconParam();
		if( pIconParam )
		{
			mpIcon[index]->SetItemIndex( itemIndex );
			mpIcon[index]->ChangeImage( pIconParam );
			mpIcon[index]->SetColor( NiColorA::WHITE );
			mpIcon[index]->Show();
			
			mStringData[index].spreadName = spreadDefine->GetName();
			mStringData[index].color = mDefaultColor;
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		assert(0);
	}
}

bool cTarotWindow::OpenSelectCard( cCardImage* image )
{
	bool ret = false;

	if( mSelectCardCount >= mMaxSelect )
		return ret;

	cCardImage* pCardImg = image;
	if( pCardImg == 0 )
	{
		for(;;)
		{
			// 계속 고른다
			int selectNum = ::rand()%MAX_TAROT_CARDS;

			pCardImg = (cCardImage*)mCardArr[selectNum];
			if( !pCardImg )
			{
				assert(0);
				break;
			}
			if( pCardImg->IsShow() == false )
				break;
		}
	}

	// 반복해서 골랐는데 없으면 에러!!!
	if( !pCardImg )
	{
		assert(0);
		return false;
	}

	if( pCardImg->IsShow() == false )
	{
		if( image )
			UIMAN->ReleaseHoverNode( image );

		/// 서버에서 이미 보낸 정보로 세팅해서 오픈
		sResultInfo* info = TAROTMAN->GetSelectCardInfo( mSelectCardCount );
		if( info )
		{
			cCardParam* pParam = TAROTMAN->GetTarotParam( info->TarotCardIndex );
			if( pParam )
			{
				pCardImg->SetImage( info->TarotCardIndex, pParam );
				pCardImg->ShowImage( true, cCardImage::eFade_In );

				mSelectCardCount++;
				mpSelectWindow->SetSelectCardCount( mSelectCardCount );

				ret = true;
			}
			else
			{
				assert(0);
			}
		}
		else
		{
			assert(0);
		}
	}

	return ret;
}

bool cTarotWindow::OpenSelectNpcCard( cCardImage* image )
{
	bool ret1 = false;
	bool ret2 = false;

	if( mSelectCardCount < mMaxSelect )
	{
		cCardImage* pCardImg = image;
		if( image == 0 )
		{
			for(;;)
			{
				int selectNum = rand() % MAX_TAROT_CARDS;
				pCardImg = (cCardImage*)mCardArr[selectNum];
				if( !pCardImg )
				{
					assert(0);
					break;
				}
				if( !pCardImg->IsLock() )
					break;
			}
		}

		// 반복해서 골랐는데 없으면 에러!!!
		if( !pCardImg )
		{
			assert(0);
			return false;
		}

		if( pCardImg->IsShow() == false )
		{
			if( image )
				UIMAN->ReleaseHoverNode( image );

			/// 선택 처리
			pCardImg->ShowImage( false, cCardImage::eFade_In );
			pCardImg->SetLock( true );
			pCardImg->SetColor( NiColorA( 0.5f, 0.5f, 0.5f, 1.0f ) );
			mSelectCardCount++;

			ret1 = true;
		}
	}

	if( mpNpcSelectWindow )
	{
		if( mSelectCardCount >= mMaxSelect )
			mpNpcSelectWindow->SetEnableComplete( true );

		mpNpcSelectWindow->SelectCount( mSelectCardCount, mMaxSelect );

		ret2 = true;
	}

	return ( ret1 && ret2 );
}
bool cTarotWindow::OpenResultCard()
{
	if( mSelectCardCount >= (int)mMaxSelect )
		return false;

	if( mSelectCardCount >= MIN_TAROT_CARDS )
		return false;

	/// 정보 가져오기
	cCardImage* cardImg = (cCardImage*)mCardArr[mSelectCardCount];
	sResultInfo* result = TAROTMAN->GetSelectCardInfo( mSelectCardCount );
	if( result && cardImg && cardImg->IsShow() == false )
	{
		cItemDefine* cardDefine = ITEMMAN->GetItemDefine( cardImg->GetTarotItemIdx() );	
		if( cardDefine )
		{
			cItemTarot* pTarot = cardDefine->GetTarotDefine();
			if( pTarot )
			{
				/// 카드 오픈
				cardImg->ShowImage( true, cCardImage::eFade_In );
				cardImg->SetDirection( result->Dir );

				/// 카드 해석
				unsigned long descIdx = ( result->Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ? pTarot->GetStraightDesc() : pTarot->GetInverseDesc();
				mpResultWindow->SetDescription( GAMERESOURCEMAN->GetTarotText(descIdx) );
				
				/// 결과 더하기
				mCurrentResult = result->Value;
				mCardTotalPoint += result->Value;

				mSelectCardCount++;
			}
			else
			{
				assert(0);
				return false;
			}
		}
		else
		{
			assert(0);
			return false;
		}
	}
	else
	{
		assert(0);
		return false;
	}

	return true;
}

/// npc 점집 결과 카드 오픈
bool cTarotWindow::OpenNpcResultCard()
{
	if( mSelectCardCount >= (int)mMaxSelect )
		return false;

	sTarotResult& data = mNpcTarotResult[mSelectCardCount];
	cCardImage* cardImg = (cCardImage*)mCardArr[mSelectCardCount];
	sNpcTarotCard* card = TAROTMAN->GetNpcTarot( data.TarotCardIndex );
	sNpcSpread* spread = TAROTMAN->GetNpcSpread( mSelectSpreadIndex );
	if( card && spread && cardImg && cardImg->IsShow() == false )
	{
		/// 카드 오픈
		cardImg->ShowImage( true, cCardImage::eFade_In );
		cardImg->SetDirection( data.Dir );

		/// 카드 자체 해석
		long descIdx = ( data.Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ? card->mStraightIdx : card->mInverseIdx;
		LPCTSTR desc = GAMERESOURCEMAN->GetTarotText( descIdx );

		/// 카드 질문
		sNpcSpreadParam& spreadParam = spread->mParam[mSelectCardCount];
		LPCTSTR question = GAMERESOURCEMAN->GetNpcTarotText( spreadParam.mQuestionIdx );

		/// 카드 정보
		long dirIndex = ( data.Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ? 330 : 331;
		LPCTSTR dirStr = GAMERESOURCEMAN->GetGameText( dirIndex );
		LPCTSTR cardStr = GAMERESOURCEMAN->GetNpcTarotText( card->mNumber + TAROTNUMBER_TEXTSTART );

		TCHAR infoStr[128] = {0,};
		::_stprintf_s( infoStr, _T("%s / %s"), cardStr, dirStr );
		
		/// 질문에 대한 해석
		sNpcTarotComment* comment = TAROTMAN->GetNpcTarotComment( card->mCommnetIndex );
		if( comment )
		{
			// [해석랜덤]
			unsigned int i = rand() % 3;

			// [운세속성][위치속성][방향][해석랜덤]
			unsigned long textIndex = comment->mComment[ spread->mFortuneProp ][ spreadParam.mPositionProp ][ data.Dir ][ i ];
			LPCTSTR commentStr = GAMERESOURCEMAN->GetNpcTarotText( textIndex );

			/// 해석 정보 저장
			data.Value = textIndex;

			/// UI 세팅
			mpNpcResultWindow->SetComment( commentStr );
		}
		else
		{
			assert(0);
		}

		/// UI 세팅
		mpNpcResultWindow->SetDesc( desc );
		mpNpcResultWindow->SetQuestion( question );
		mpNpcResultWindow->SetCardInfo( infoStr );

		/// 
		mSelectCardCount++;
		mShowReviewIndex = mSelectCardCount-1;
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

/// npc 점집 결과 해석 리뷰
bool cTarotWindow::OpenNpcResultReview( unsigned int selectIndex )
{
	if( selectIndex < 0 || selectIndex >= (int)mMaxSelect )
		return false;

	mShowReviewIndex = selectIndex;

	sTarotResult& data = mNpcTarotResult[selectIndex];
	sNpcTarotCard* card = TAROTMAN->GetNpcTarot( data.TarotCardIndex );
	sNpcSpread* spread = TAROTMAN->GetNpcSpread( mSelectSpreadIndex );
	if( card && spread )
	{
		/// 카드 자체 해석
		long descIdx = ( data.Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ? card->mStraightIdx : card->mInverseIdx;
		LPCTSTR desc = GAMERESOURCEMAN->GetTarotText( descIdx );

		/// 카드 질문
		sNpcSpreadParam& spreadParam = spread->mParam[selectIndex];
		LPCTSTR question = GAMERESOURCEMAN->GetNpcTarotText( spreadParam.mQuestionIdx );

		/// 카드 정보
		long dirIndex = ( data.Dir == TAROT_SEEKER_RESULT_STRAIGHT ) ? 330 : 331;
		LPCTSTR dirStr = GAMERESOURCEMAN->GetGameText( dirIndex );
		LPCTSTR cardStr = GAMERESOURCEMAN->GetNpcTarotText( card->mNumber + TAROTNUMBER_TEXTSTART );

		TCHAR infoStr[128] = {0,};
		::_stprintf_s( infoStr, _T("%s / %s"), cardStr, dirStr );

		/// 질문에 대한 해석
		LPCTSTR commentStr = GAMERESOURCEMAN->GetNpcTarotText( data.Value );

		/// UI 세팅
		mpNpcResultWindow->SetDesc( desc );
		mpNpcResultWindow->SetQuestion( question );
		mpNpcResultWindow->SetCardInfo( infoStr );
		mpNpcResultWindow->SetComment( commentStr );
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}

void cTarotWindow::SetSelectedSpread( unsigned int index )
{
	mSelectSpreadIndex = index;

	mpSpreadInfo = 0;
	cItemDefine* pDefine = ITEMMAN->GetItemDefine( mSelectSpreadIndex );
	if( pDefine )
	{
		mpSpreadInfo = pDefine->GetSpreadDefine();
		mMaxSelect = MIN_TAROT_CARDS;
	}
	else
	{
		assert(0);
	}
}

void cTarotWindow::SetSelectedNpcSpread( unsigned int index )
{
	mSelectSpreadIndex = index;

	sNpcSpread* spread = TAROTMAN->GetNpcSpread( index );
	if( spread )
	{
		mMaxSelect = spread->mCount;
		if( mMaxSelect > MAX_NPCSPREAD_COUNT )
		{
			assert(0);
			return;
		}

		/// 초기화
		mSelectCardCount = 0;
		
		/// 카드 뒤집어 놓기
		for( unsigned int i = 0; i < MAX_TAROT_CARDS; i++ )
		{
			cCardImage* pCardImg = (cCardImage*)mCardArr[i];
			if( pCardImg )
			{
				pCardImg->SetLock( false );
				pCardImg->SetColor( NiColorA::WHITE );
				pCardImg->ShowImage( false, false );
			}
			else
			{
				assert(0);
			}
		}

		if( mpNpcSelectWindow )
		{
			mpNpcSelectWindow->SetEnableComplete( false );
			mpNpcSelectWindow->SelectCount( mSelectCardCount, mMaxSelect );
		}
	}
	else
	{
		assert(0);
	}
}

void cTarotWindow::MakeNpcTarotResult()
{
	memset(mNpcTarotResult, 0, sizeof(mNpcTarotResult));

	/// 운세
	unsigned long temp[MAX_TAROT_CARDS];
	memset(temp, 0, sizeof(temp));

	for( int i = 0; i < MAX_TAROT_CARDS; ++i )
		temp[i] = mNpcTarotIndex[i];

	/// suffle
	unsigned long value = 0;
	for( int i = 0; i < MAX_TAROT_CARDS; ++i )
	{
		int s = (int)(rand() % MAX_TAROT_CARDS);
		value   = temp[i];
		temp[i] = temp[s];
		temp[s] = value;
	}

	for( unsigned int i = 0; i < mMaxSelect; ++i )
	{
		mNpcTarotResult[i].TarotCardIndex	= temp[i];
		mNpcTarotResult[i].Dir				= (BYTE)(rand() % 2);
	}
}

void cTarotWindow::OnNetLock( int lockTry )
{
	switch( lockTry )
	{
	case ePAGE_OPEN:
		{
			if( mpOpenWindow )
				mpOpenWindow->NetLock( lockTry );
		}
		break;
	case ePAGE_VIEW:
	case ePAGE_SELFVIEW:
		{
			if( mpCheckWindow )
				mpCheckWindow->NetLock( lockTry );
		}
		break;
	case ePAGE_SELECT:
		{
			if( mpSelectWindow )
				mpSelectWindow->NetLock( lockTry );
		}
		break;
	case ePAGE_RESULT:
		{
			if( mpResultWindow )
				mpResultWindow->NetLock( lockTry );
		}
		break;
	case ePAGE_NPCVIEW:
		{
			if( mpNpcCheckWindow )
				mpNpcCheckWindow->NetLock( lockTry );
		}
		break;
	case ePAGE_NPCSELECT:
		{
			if( mpNpcSelectWindow )
				mpNpcSelectWindow->NetLock( lockTry );
		}
		break;
	default:
		return;
	}
}

void cTarotWindow::OnNetUnLock( int lockTry )
{
	switch( lockTry )
	{
	case ePAGE_OPEN:
		{
			if( mpOpenWindow )
				mpOpenWindow->NetUnLock( lockTry );
		}
		break;
	case ePAGE_VIEW:
	case ePAGE_SELFVIEW:
		{
			if( mpCheckWindow )
				mpCheckWindow->NetUnLock( lockTry );
		}
		break;
	case ePAGE_SELECT:
		{
			if( mpSelectWindow )
				mpSelectWindow->NetUnLock( lockTry );
		}
		break;
	case ePAGE_RESULT:
		{
			if( mpResultWindow )
				mpResultWindow->NetUnLock( lockTry );
		}
		break;
	case ePAGE_NPCVIEW:
		{
			if( mpNpcCheckWindow )
				mpNpcCheckWindow->NetUnLock( lockTry );
		}
		break;
	case ePAGE_NPCSELECT:
		{
			if( mpNpcSelectWindow )
				mpNpcSelectWindow->NetUnLock( lockTry );
		}
		break;
	default:
		return;
	}
}
