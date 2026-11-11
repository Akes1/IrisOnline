#include "stdafx.h"
#include "PartyStateWindow.h"

#include "Gauge.h"
#include "Label.h"
#include "UIImage.h"
#include "PartyManager.h"
#include "PartyBuff.h"
#include "GameResourceManager.h"
#include "ObjectManager.h"
#include "Hero.h"
#include "RButtonMenuWindow.h"
#include "GameUIManager.h"
#include "PVPManager.h"
#include "PartyUnionManager.h"
#include "PartyWindow.h"

cPartyStateMiniWindow::cPartyStateMiniWindow()
: mUserIndex(0)
, mpHPGauge(0)
, mpMPGauge(0)
, mpName(0)
, mpNodeSkin(0)
, mpLeaderSkin(0)
, mpLeaderImage(0)
, mLinkArrIndex(-1)
, mNodeIndex(0)
, mIsLeader( false )
, mIsSelect( false )
, mpSelectedImage(0)
, mpPvpLeaderSkin(0)
, mpPvpLeaderImage(0)
, mShowPvpLeader( false )
, mpFlagPlayerSkin(0)
, mpFlagPlayerImage(0)
, mShowFlagPlayerLeader( false )
, mRoomNum(0)
, mMapNum(0)
{
}

cPartyStateMiniWindow::~cPartyStateMiniWindow()
{
	SAFE_DELETE( mpFlagPlayerImage );
	SAFE_DELETE( mpPvpLeaderImage );
	SAFE_DELETE( mpSelectedImage );
	SAFE_DELETE( mpLeaderImage );
}

void cPartyStateMiniWindow::Clear()
{
	mUserIndex = 0;
	mLinkArrIndex = -1;

	mIsLeader = false;
	mIsSelect = false;

	if( mpName )
		mpName->Clear();

	if( mpHPGauge )
		mpHPGauge->Clear();

	if( mpMPGauge )
		mpMPGauge->Clear();

	mShowPvpLeader = false;
	mShowFlagPlayerLeader = false;
}

void cPartyStateMiniWindow::Open()
{
}

void cPartyStateMiniWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

bool cPartyStateMiniWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpName = (cLabel*)GetChild( eUIID_GAME_PARTYSTATE_MINI_USERNAME );
	mpHPGauge = (cGauge*)GetChild( eUIID_GAME_PARTYSTATE_MINI_HP );
	mpMPGauge = (cGauge*)GetChild( eUIID_GAME_PARTYSTATE_MINI_MP );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	mpNodeSkin = pSkin->GetNodeSkin( mSkinName );
	if( !mpNodeSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "PartyStateMini_Selected" );
	if( selectSkin && selectSkin->mpTexture )
	{
		mpSelectedImage = new cUIImage( selectSkin->mpTexture );

		/// 텍스처 영역 설정
		unsigned int tx = selectSkin->mSkinInfo->mTexX;
		unsigned int ty = selectSkin->mSkinInfo->mTexY;
		unsigned int tw = selectSkin->mSkinInfo->mWidth;
		unsigned int th = selectSkin->mSkinInfo->mHeight;

		/// 이미지 영역을 갱신
		cUIRect   rect = mAbsoluteRect;
		unsigned int x = rect.mLeft - 2;
		unsigned int y = rect.mTop  - 2;
		rect.SetPos( cUIPos( x, y ) );

		mpSelectedImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpSelectedImage->SetScreenRect( rect );
	}
	
	mpLeaderSkin = pSkin->GetNodeSkin( "PartyStateMini_Leader" );
	if( mpLeaderSkin && mpLeaderSkin->mpTexture )
	{
		mpLeaderImage = new cUIImage( mpLeaderSkin->mpTexture );
		unsigned int tx = mpLeaderSkin->mSkinInfo->mTexX;
		unsigned int ty = mpLeaderSkin->mSkinInfo->mTexY;
		unsigned int x = mpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = mpLeaderSkin->mSkinInfo->mWidth;
		unsigned int h = mpLeaderSkin->mSkinInfo->mHeight;
		mpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	mpPvpLeaderSkin = pSkin->GetNodeSkin( "PartyStateMini_PvpLeader" );
	if( mpPvpLeaderSkin && mpPvpLeaderSkin->mpTexture )
	{
		mpPvpLeaderImage = new cUIImage( mpPvpLeaderSkin->mpTexture );
		unsigned int tx = mpPvpLeaderSkin->mSkinInfo->mTexX;
		unsigned int ty = mpPvpLeaderSkin->mSkinInfo->mTexY;
		unsigned int  x = mpPvpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int  y = mpPvpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int  w = mpPvpLeaderSkin->mSkinInfo->mWidth;
		unsigned int  h = mpPvpLeaderSkin->mSkinInfo->mHeight;
		mpPvpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpPvpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	mpFlagPlayerSkin = pSkin->GetNodeSkin( "PartyStateMini_FlagPlayer" );
	if( mpFlagPlayerSkin && mpFlagPlayerSkin->mpTexture )
	{
		mpFlagPlayerImage = new cUIImage( mpFlagPlayerSkin->mpTexture );
		unsigned int tx = mpFlagPlayerSkin->mSkinInfo->mTexX;
		unsigned int ty = mpFlagPlayerSkin->mSkinInfo->mTexY;
		unsigned int  x = mpFlagPlayerSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int  y = mpFlagPlayerSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int  w = mpFlagPlayerSkin->mSkinInfo->mWidth;
		unsigned int  h = mpFlagPlayerSkin->mSkinInfo->mHeight;
		mpFlagPlayerImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpFlagPlayerImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	return true;
}

void cPartyStateMiniWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpLeaderSkin && mpLeaderImage )
	{
		unsigned int x = mpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpLeaderImage->SetPos( x, y );
	}

	if( mpSelectedImage && mIsSelect )
	{
		mpSelectedImage->SetPos( mAbsoluteRect.mLeft - 1, mAbsoluteRect.mTop - 1 );
	}

	if( mShowPvpLeader && mpPvpLeaderSkin && mpPvpLeaderImage )
	{
		unsigned int x = mpPvpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpPvpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpPvpLeaderImage->SetPos( x, y );
	}

	if( mShowFlagPlayerLeader && mpFlagPlayerSkin && mpFlagPlayerImage )
	{
		unsigned int x = mpFlagPlayerSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpFlagPlayerSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpFlagPlayerImage->SetPos( x, y );
	}
}

void cPartyStateMiniWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	InitPosition( mNodeIndex );
}

void cPartyStateMiniWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mUserIndex == 0 )
		return;

	if( HERO->GetPartyIndex() > 0 )
	{
		unsigned long leaderIndex = PARTYMAN->GetLeaderIndex();
		mIsLeader = ( leaderIndex == mUserIndex );
	}
	else
		mIsLeader = false;
}

void cPartyStateMiniWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mUserIndex == 0)
		return;

	cUIWindow::OnRender( pKeeper );

	if( mpLeaderImage && mIsLeader )
		mpLeaderImage->Draw();

	if( mpSelectedImage && mIsSelect )
		mpSelectedImage->Draw();

	if( mpPvpLeaderImage && mShowPvpLeader )
		mpPvpLeaderImage->Draw();

	if( mpFlagPlayerImage && mShowFlagPlayerLeader )
		mpFlagPlayerImage->Draw();
}

void cPartyStateMiniWindow::OnMouseMove( const cUIPos& pos )
{
	cPartyWindow* partyWin = GAMEUI->GetPartyWindow();
	if( partyWin && partyWin->IsLockedMove() == false )
	{
		cUIWindow::OnMouseMove( pos );
	}
}

void cPartyStateMiniWindow::OnLButtonDoubleClick( const cUIPos& pos )
{
	OnLButtonDown( pos, false, false, false );
}

void cPartyStateMiniWindow::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shiftPress )
{
	cUIWindow::OnLButtonDown( pos, ctrl, alt, shiftPress );

	if( HERO->GetPartyIndex() > 0 && ContainPoint( pos ) )
		PARTYMAN->SetTargetByUI( mUserIndex );
	else if( HERO->GetPartyUnionIndex() > 0 && ContainPoint( pos ) )
		PARTYUNIONMAN->SetTargetByUI( mUserIndex );
}

void cPartyStateMiniWindow::OnRButtonUp( const cUIPos& pos )
{
	cRButtonMenuWindow* win = GAMEUI->GetRButtonMenuWindow();
	if( win )
		win->PopUp( pos.mX, pos.mY, mUserIndex, mpName->GetText(), ePopupPos_PartyWindow );
}

void cPartyStateMiniWindow::OnMouseLeft( const cUIPos& pos )
{
	UIMAN->HideTip();
}

void cPartyStateMiniWindow::OnMouseHovered( const cUIPos& pos )
{
	UIMAN->HideTip();

	DWORD heroChannelNum = HERO->GetChannelNum();
	if( heroChannelNum == mChannelNum )
	{
		UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( mMapNum ) );
	}
	else
	{
		// 타채널의 일반 맵인경우
		if( mMapNum >= MAP_MIN && mMapNum <= MAP_MAX )
		{
			TCHAR temp[128]={0,};
			::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 1260 ), mChannelNum );
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, temp );
		}
		else
		{
			// 타채널의 던전 맵인경우
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( mChannelNum ) );
		}
	}
}

void cPartyStateMiniWindow::SetInfo( unsigned long userIdx, LPCTSTR name,
								unsigned long maxhp, unsigned long hp, 
								unsigned long maxmp, unsigned long mp, 
								unsigned short mapnum, unsigned short roomnum, DWORD channelnum, unsigned int linkIdx )
{
	mUserIndex = userIdx;

	if( mpName )
		mpName->SetText( name );

	if( mpHPGauge )
		mpHPGauge->SetMaxValue( (float)maxhp );

	if( mpHPGauge )
		mpHPGauge->SetGaugeNoAni( (float)hp );

	if( mpMPGauge )
		mpMPGauge->SetMaxValue( (float)maxmp );

	if( mpMPGauge )
		mpMPGauge->SetGaugeNoAni( (float)mp );

	UpdateMap( mapnum, roomnum, channelnum );

	SetLinkArrIndex( linkIdx );
}

void cPartyStateMiniWindow::UpdateInfo( unsigned long maxhp, unsigned long hp, unsigned long maxmp, unsigned long mp )
{
	if( mUserIndex == 0 )
		return;

	if( mpHPGauge )
		mpHPGauge->SetMaxValue( (float)maxhp );

	if( mpHPGauge )
		mpHPGauge->SetGaugeNoAni( (float)hp );

	if( mpMPGauge )
		mpMPGauge->SetMaxValue( (float)maxmp );

	if( mpMPGauge )
		mpMPGauge->SetGaugeNoAni( (float)mp );
}

void cPartyStateMiniWindow::UpdateMap( unsigned short mapnum, unsigned short roomnum, DWORD channelnum )
{
	mMapNum = mapnum;
	mRoomNum = roomnum;
	mChannelNum = channelnum;
	unsigned short heroMapNum = HERO->GetMapNum();
	unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
	DWORD		   heroChannelNum = HERO->GetChannelNum();

	// 동일 맵 여부
	bool isSameMap = false;	

	// 일반 맵인 경우 
	if( mapnum >= MAP_MIN && mapnum <= MAP_MAX )
	{
		isSameMap = ( heroMapNum == mapnum && heroChannelNum == channelnum );
	}
	// 그외 맵 인 경우
	else
	{
		isSameMap = ( heroMapNum == mapnum && heroRoomIndex == roomnum );
	}

	//// 일반 맵
	//if( roomnum == 0 )
	//{
	//	isSameMap = ( heroMapNum == mapnum );
	//}
	//// 인던 맵 ( 맵은 동일해도 동일 룸에 있어야 정보를 보여준다 )
	//else
	//{
	//	isSameMap = ( heroMapNum == mMapNum && heroRoomIndex == roomnum );
	//}

	// 동일 위치
	if( isSameMap == true )
	{
		if( mpName )
			mpName->SetTextColor( COLOR_WHITE );

		if( mpHPGauge )
			mpHPGauge->Show();

		if( mpMPGauge )
			mpMPGauge->Show();
	}
	else
	{
		if( mpName )
			mpName->SetTextColor( COLOR_GRAY );

		if( mpHPGauge )
			mpHPGauge->Hide();

		if( mpMPGauge )
			mpMPGauge->Hide();
	}
}

void cPartyStateMiniWindow::InitPosition( int i )
{
	/// 위치 갱신
	if( mpNodeSkin )
	{
		unsigned int y = ( GetScreenHeight() * mpNodeSkin->mSkinInfo->mY ) / mDefaultHeight;
		unsigned int addy = ( mpNodeSkin->mSkinInfo->mHeight * i );

		cUIPos pos;
		pos.mX = mpNodeSkin->mSkinInfo->mX;
		pos.mY = y + addy;

		SetRelativePos( pos );
	}

	mNodeIndex = i;
	mLinkArrIndex = i;
}

/// 선택된 윈도우 색 변환
void cPartyStateMiniWindow::SetSelect( bool set )
{
	mIsSelect = set;

	if( mIsSelect == true )
	{
		if( mpName )
			mpName->SetTextColor( COLOR_YELLOW );

		if( mpSelectedImage )
			mpSelectedImage->SetPos( mAbsoluteRect.mLeft - 1, mAbsoluteRect.mTop - 1 );
	}
	else
	{
		unsigned long color = ( HERO->GetMapNum() == mMapNum ) ? COLOR_WHITE : COLOR_GRAY;

		if( mpName )
			mpName->SetTextColor( color );
	}
}

void cPartyStateMiniWindow::SetPvpLeaderVisible( bool show )
{ 
	mShowPvpLeader = show;
	if( show && mpPvpLeaderSkin && mpPvpLeaderImage )
	{
		unsigned int x = mpPvpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpPvpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpPvpLeaderImage->SetPos( x, y );
	}
}

void cPartyStateMiniWindow::SetFlagPlayerVisible( bool show )
{ 
	mShowFlagPlayerLeader = show;
	if( show && mpFlagPlayerSkin && mpFlagPlayerImage )
	{
		unsigned int x = mpFlagPlayerSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpFlagPlayerSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpFlagPlayerImage->SetPos( x, y );
	}
}

//----------------------------------------------------------------------------------

cPartyStateWindow::cPartyStateWindow()
: mJob( ePLAYER_NONEJOB )
, mpLevel(0)
, mpBuff(0)
, mpJobImage(0)
, mpJobSkin(0)
, mIsShowMapTip( false )
, mIsShowJobTip( false )
{
}

cPartyStateWindow::~cPartyStateWindow()
{
	SAFE_DELETE( mpLeaderImage );
	SAFE_DELETE( mpJobImage );
	SAFE_DELETE( mpSelectedImage );
}

void cPartyStateWindow::Clear()
{
	mUserIndex = 0;
	mLinkArrIndex = -1;
	mJob = ePLAYER_NONEJOB;

	mIsLeader = false;
	mIsSelect = false;
	mIsShowMapTip = false;
	mIsShowJobTip = false;
	mShowPvpLeader = false;
	mShowFlagPlayerLeader = false;

	/// 초기화
	if( mpLevel )
		mpLevel->Clear();

	if( mpBuff )
	{
		mpBuff->Clear();
		mpBuff->Hide();
	}

	if( mpName )
		mpName->Clear();

	if( mpHPGauge )
		mpHPGauge->Clear();

	if( mpMPGauge )
		mpMPGauge->Clear();
}

void cPartyStateWindow::ClearBuff()
{
	if( mpBuff )
		mpBuff->Clear();
}

void cPartyStateWindow::OnShow()
{
	cUIWindow::OnShow();

	if( mpBuff )
		mpBuff->Show();
}

void cPartyStateWindow::OnHide()
{
	cUIWindow::OnHide();

	if( mpBuff )
		mpBuff->Hide();
}

bool cPartyStateWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpLevel = (cLabel*)GetChild( eUIID_GAME_PARTYSTATE_USERLEVEL );
	mpName = (cLabel*)GetChild( eUIID_GAME_PARTYSTATE_USERNAME );
	mpHPGauge = (cGauge*)GetChild( eUIID_GAME_PARTYSTATE_HP );
	mpMPGauge = (cGauge*)GetChild( eUIID_GAME_PARTYSTATE_MP );
	mpBuff = (cPartyBuff*)GetChild( eUIID_GAME_PARTYBUFF );

	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	mpNodeSkin = pSkin->GetNodeSkin( mSkinName );
	if( !mpNodeSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "PartyState_Selected" );
	if( selectSkin && selectSkin->mpTexture )
	{
		mpSelectedImage = new cUIImage( selectSkin->mpTexture );

		/// 텍스처 영역 설정
		unsigned int tx = selectSkin->mSkinInfo->mTexX;
		unsigned int ty = selectSkin->mSkinInfo->mTexY;
		unsigned int tw = selectSkin->mSkinInfo->mWidth;
		unsigned int th = selectSkin->mSkinInfo->mHeight;

		/// 이미지 영역을 갱신
		cUIRect   rect = mAbsoluteRect;
		unsigned int x = rect.mLeft - 2;
		unsigned int y = rect.mTop  - 2;
		rect.SetPos( cUIPos( x, y ) );

		mpSelectedImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		mpSelectedImage->SetScreenRect( rect );
	}

	mpLeaderSkin = pSkin->GetNodeSkin( "PartyState_Leader" );
	if( mpLeaderSkin && mpLeaderSkin->mpTexture )
	{
		mpLeaderImage = new cUIImage( mpLeaderSkin->mpTexture );
		unsigned int tx = mpLeaderSkin->mSkinInfo->mTexX;
		unsigned int ty = mpLeaderSkin->mSkinInfo->mTexY;
		unsigned int x = mpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = mpLeaderSkin->mSkinInfo->mWidth;
		unsigned int h = mpLeaderSkin->mSkinInfo->mHeight;
		mpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	mpJobSkin = pSkin->GetNodeSkin( "PartyState_JobPos" );
	if( mpJobSkin )
	{
		mpJobImage = new cUIImage;
		unsigned int x = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = mpJobSkin->mSkinInfo->mWidth;
		unsigned int h = mpJobSkin->mSkinInfo->mHeight;
		mpJobImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	mpPvpLeaderSkin = pSkin->GetNodeSkin( "PartyState_PvpLeader" );
	if( mpPvpLeaderSkin && mpPvpLeaderSkin->mpTexture )
	{
		mpPvpLeaderImage = new cUIImage( mpPvpLeaderSkin->mpTexture );
		unsigned int tx = mpPvpLeaderSkin->mSkinInfo->mTexX;
		unsigned int ty = mpPvpLeaderSkin->mSkinInfo->mTexY;
		unsigned int x = mpPvpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpPvpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int w = mpPvpLeaderSkin->mSkinInfo->mWidth;
		unsigned int h = mpPvpLeaderSkin->mSkinInfo->mHeight;
		mpPvpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpPvpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	mpFlagPlayerSkin = pSkin->GetNodeSkin( "PartyState_FlagPlayer" );
	if( mpFlagPlayerSkin && mpFlagPlayerSkin->mpTexture )
	{
		mpFlagPlayerImage = new cUIImage( mpFlagPlayerSkin->mpTexture );
		unsigned int tx = mpFlagPlayerSkin->mSkinInfo->mTexX;
		unsigned int ty = mpFlagPlayerSkin->mSkinInfo->mTexY;
		unsigned int  x = mpFlagPlayerSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int  y = mpFlagPlayerSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int  w = mpFlagPlayerSkin->mSkinInfo->mWidth;
		unsigned int  h = mpFlagPlayerSkin->mSkinInfo->mHeight;
		mpFlagPlayerImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpFlagPlayerImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	return true;
}

void cPartyStateWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	/// 이미지 영역을 갱신
	if( mpImage )
	{
		if( mpNodeSkin )
		{
			cUIRect imgRect( mAbsoluteRect.mLeft, 
				mAbsoluteRect.mTop, 
				mAbsoluteRect.mLeft + mpNodeSkin->mSkinInfo->mWidth, 
				mAbsoluteRect.mTop  + mpNodeSkin->mSkinInfo->mHeight );
			mpImage->SetScreenRect( imgRect );
		}
		else
			mpImage->SetScreenRect( mAbsoluteRect );
	}

	if( mpLeaderSkin && mpLeaderImage )
	{
		unsigned int x = mpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpLeaderImage->SetPos( x, y );
	}

	if( mpJobSkin && mpJobImage )
	{
		unsigned int x = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpJobImage->SetPos( x, y );
	}

	if( mpSelectedImage && mIsSelect )
	{
		mpSelectedImage->SetPos( mAbsoluteRect.mLeft - 1, mAbsoluteRect.mTop - 1 );
	}

	if( mShowPvpLeader && mpPvpLeaderSkin && mpPvpLeaderImage )
	{
		unsigned int x = mpPvpLeaderSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpPvpLeaderSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpPvpLeaderImage->SetPos( x, y );
	}

	if( mShowFlagPlayerLeader && mpFlagPlayerSkin && mpFlagPlayerImage )
	{
		unsigned int x = mpFlagPlayerSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpFlagPlayerSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpFlagPlayerImage->SetPos( x, y );
	}
}

void cPartyStateWindow::UpdateSkin()
{
	cPartyStateMiniWindow::UpdateSkin();

	// 초기화 시에는 스크립트 설정 좌표로 셋팅
	InitPosition( mNodeIndex );

	UpdateWindowSize();
}

void cPartyStateWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mUserIndex == 0)
		return;

	cUIWindow::OnRender( pKeeper );

	if( mpJobImage )
		mpJobImage->Draw();

	if( mpLeaderImage && mIsLeader )
		mpLeaderImage->Draw();

	if( mpSelectedImage && mIsSelect )
		mpSelectedImage->Draw();

	if( mpPvpLeaderImage && mShowPvpLeader )
		mpPvpLeaderImage->Draw();

	if( mpFlagPlayerImage && mShowFlagPlayerLeader )
		mpFlagPlayerImage->Draw();
}

void cPartyStateWindow::OnMouseLeft( const cUIPos& pos )
{
	if( mIsShowMapTip || mIsShowJobTip )
	{
		UIMAN->HideTip();

		mIsShowMapTip = false;
		mIsShowJobTip = false;
	}
}

void cPartyStateWindow::OnMouseMove( const cUIPos& pos )
{
	cPartyStateMiniWindow::OnMouseMove( pos );

	//
	cUIRect jobImgRect;
	jobImgRect.mLeft   = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
	jobImgRect.mTop    = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
	jobImgRect.mRight  = jobImgRect.mLeft + mpJobSkin->mSkinInfo->mWidth;
	jobImgRect.mBottom = jobImgRect.mTop + mpJobSkin->mSkinInfo->mHeight; 

	cUIRect skinImgRect;
	skinImgRect.mLeft   = GetAbsoluteRect().mLeft;
	skinImgRect.mTop    = GetAbsoluteRect().mTop;
	skinImgRect.mRight  = skinImgRect.mLeft + mpNodeSkin->mSkinInfo->mWidth;
	skinImgRect.mBottom = skinImgRect.mTop  + mpNodeSkin->mSkinInfo->mHeight; 

	// 직업 이미지에 마우스 오버
	if ( jobImgRect.ContainPoint( pos ) == true )
	{
		if( mIsShowJobTip == false )
		{
			LPCTSTR jobName = GAMERESOURCEMAN->GetJobName( mJob );
			if( jobName )
			{
				UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, jobName );

				mIsShowJobTip = true;
				mIsShowMapTip = false;
			}
		}
	}
	// 그 외 영역에 오버
	else if( skinImgRect.ContainPoint( pos ) == true )
	{
		if( mIsShowMapTip == false )
		{
			if( mMapNum > 0 )
			{
				/// 일반맵
				UIMAN->HideTip();

				DWORD heroChannelNum = HERO->GetChannelNum();
				if( heroChannelNum == mChannelNum )
				{
					UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( mMapNum ) );
				}
				else
				{
					// 타채널의 일반 맵인경우
					if( mMapNum >= MAP_MIN && mMapNum <= MAP_MAX )
					{
						TCHAR temp[128]={0,};
						::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 1260 ), mChannelNum );
						UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, temp );
					}
					else
					{
						// 타채널의 던전 맵인경우
						UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( mChannelNum ) );
					}
				}

				mIsShowJobTip = false;
				mIsShowMapTip = true;
			}
		}
	}
}

void cPartyStateWindow::SetInfo( unsigned long userIdx, LPCTSTR name,
			 unsigned long maxhp, unsigned long hp, 
			 unsigned long maxmp, unsigned long mp, 
			 unsigned short mapnum, unsigned short roomnum, DWORD channelnum, unsigned int linkIdx )
{
	assert(0);
}

void cPartyStateWindow::SetInfo( unsigned long userIdx, LPCTSTR name,
								char level, ePLAYER_JOB job, 
								unsigned long maxhp, unsigned long hp, 
								unsigned long maxmp, unsigned long mp, 
								unsigned short mapnum, unsigned short roomnum, DWORD channelnum, unsigned int linkIdx )
{
	//
	mUserIndex = userIdx;

	if( mpName )
		mpName->SetText( name );

	if( mpHPGauge )
		mpHPGauge->SetMaxValue( (float)maxhp );

	if( mpHPGauge )
		mpHPGauge->SetGaugeNoAni( (float)hp );

	if( mpMPGauge )
		mpMPGauge->SetMaxValue( (float)maxmp );

	if( mpMPGauge )
		mpMPGauge->SetGaugeNoAni( (float)mp );

	if( mpLevel )
		mpLevel->SetValue( level );

	UpdateJob( job );

	UpdateMap( mapnum, roomnum, channelnum );

	SetLinkArrIndex( linkIdx );
}

void cPartyStateWindow::UpdateInfo( char level, unsigned long maxhp, unsigned long hp, unsigned long maxmp, unsigned long mp )
{
	if( mUserIndex == 0 )
		return;

	if( mpHPGauge )
		mpHPGauge->SetMaxValue( (float)maxhp );

	if( mpHPGauge )
		mpHPGauge->SetGaugeNoAni( (float)hp );

	if( mpMPGauge )
		mpMPGauge->SetMaxValue( (float)maxmp );

	if( mpMPGauge )
		mpMPGauge->SetGaugeNoAni( (float)mp );

	if( mpLevel )
		mpLevel->SetValue( level );
}

void cPartyStateWindow::UpdateMap( unsigned short mapnum, unsigned short roomnum, DWORD channelnum )
{
	mMapNum = mapnum;
	mRoomNum = roomnum;
	mChannelNum = channelnum;
	unsigned short heroMapNum = HERO->GetMapNum();
	unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
	DWORD		   heroChannelNum = HERO->GetChannelNum();

	// 동일 맵 여부
	bool isSameMap = false;	

	// 일반 맵인 경우 
	if( mapnum >= MAP_MIN && mapnum <= MAP_MAX )
	{
		isSameMap = ( heroMapNum == mapnum && heroChannelNum == channelnum );
	}
	// 그외 맵 인 경우
	else
	{
		isSameMap = ( heroMapNum == mapnum && heroRoomIndex == roomnum );
	}

	// 동일 위치
	if( isSameMap == true )
	{
		if( mpName )
			mpName->SetTextColor( COLOR_WHITE );

		if( mpHPGauge )
			mpHPGauge->Show();

		if( mpMPGauge )
			mpMPGauge->Show();

		if( mpBuff )
		{
			if( mVisible == true && mpBuff->IsHaveBuff() == true )
				mpBuff->Show();
			else
				mpBuff->Hide();
		}
	}
	else
	{
		if( mpName )
			mpName->SetTextColor( COLOR_GRAY );

		if( mpHPGauge )
			mpHPGauge->Hide();

		if( mpMPGauge )
			mpMPGauge->Hide();

		if( mpBuff )
			mpBuff->Hide();
	}

	UpdateWindowSize();
}

void cPartyStateWindow::UpdateJob( ePLAYER_JOB job )
{
	mJob = job;

    /// 직업 이미지 설정
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return;
	}

	sJobInfo* pInfo = GAMERESOURCEMAN->GetJobInfo( job );
	if( pInfo )
	{
		cIconParam* pParam = UIMAN->GetIconParam( pInfo->jobTargetIconIndex );
		if( pParam )
		{
			mpJobImage->SetTexture( pParam->mpTexture );
			int tx = pParam->mTexPos.mX;
			int ty = pParam->mTexPos.mY;
			int w = mpJobSkin->mSkinInfo->mWidth;
			int h = mpJobSkin->mSkinInfo->mHeight;

			mpJobImage->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ));
		}
		else
			assert( 0 && "Fail to get icon param" );
	}
	else
		assert( 0 && "Fail to job info" );
}

void cPartyStateWindow::AddBuff(unsigned long uniqueIdx, unsigned long influenceIdx )
{
	unsigned short heroMapNum = HERO->GetMapNum();
	unsigned short heroRoomIndex = HERO->GetPartyRoomIndex();
	DWORD		   heroChannelNum = HERO->GetChannelNum();

	// 동일 맵 여부
	bool isSameMap = false;	

	// 일반 맵인 경우 
	if( mMapNum >= MAP_MIN && mMapNum <= MAP_MAX )
	{
		isSameMap = ( heroMapNum == mMapNum && heroChannelNum == mChannelNum );
	}
	// 그외 맵 인 경우
	else
	{
		isSameMap = ( heroMapNum == mMapNum && heroRoomIndex == mRoomNum );
	}

	if( mpBuff )
	{
		mpBuff->AddBuff( uniqueIdx, influenceIdx );

		if( mVisible == true && isSameMap == true )
		{
			mpBuff->Show();
		}
		else
		{
			mpBuff->Hide();
		}

		UpdateWindowSize();
	}
}

void cPartyStateWindow::DelBuff( unsigned long uniqueIdx )
{
	if( mpBuff )
	{
		mpBuff->DelBuff( uniqueIdx );
		UpdateWindowSize();
	}
}

void cPartyStateWindow::DelDeBuff( unsigned long uniqueIdx )
{
	if( mpBuff )
	{
		mpBuff->DelDeBuff( uniqueIdx );
		UpdateWindowSize();
	}
}

void cPartyStateWindow::InitPosition( int i )
{
	/// 위치 갱신
	if( mpNodeSkin )
	{
		// 위치값 보정
		unsigned int y = ( GetScreenHeight() * mpNodeSkin->mSkinInfo->mY ) / mDefaultHeight;
		unsigned int addy = ( mpNodeSkin->mSkinInfo->mHeight * i );

		cUIPos pos;
		pos.mX = mpNodeSkin->mSkinInfo->mX;
		pos.mY = y + addy;

		SetRelativePos( pos );

		if( mpBuff )
			mpBuff->UpdatePos();
	}		

	mNodeIndex = i;
	mLinkArrIndex = i;
}

void cPartyStateWindow::UpdateWindowSize()
{
 	if( mpBuff && mpNodeSkin )
 	{
 		cUISize size;
 		size.mWidth = mpNodeSkin->mSkinInfo->mWidth;
 		size.mHeight = mpNodeSkin->mSkinInfo->mHeight;
 
 		if( mpBuff->IsHaveBuff() )
 			size.mWidth += mpBuff->GetAbsoluteRect().GetWidth();
 
 		SetRelativeSize( size );
 	}
}

void cPartyStateWindow::OnMouseHovered( const cUIPos& pos )
{
	UIMAN->HideTip();

	cUIRect skinImgRect;
	skinImgRect.mLeft   = GetAbsoluteRect().mLeft;
	skinImgRect.mTop    = GetAbsoluteRect().mTop;
	skinImgRect.mRight  = skinImgRect.mLeft + mpNodeSkin->mSkinInfo->mWidth;
	skinImgRect.mBottom = skinImgRect.mTop  + mpNodeSkin->mSkinInfo->mHeight; 

	DWORD		   heroChannelNum = HERO->GetChannelNum();
	if( skinImgRect.ContainPoint( pos ) == true )
	{
		if( heroChannelNum == mChannelNum )
		{
			UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( mMapNum ) );
		}
		else
		{
			// 타채널의 일반 맵인경우
			if( mMapNum >= MAP_MIN && mMapNum <= MAP_MAX )
			{
				TCHAR temp[128]={0,};
				::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText( 1260 ), mChannelNum );
				UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, temp );
			}
			else
			{
				// 타채널의 던전 맵인경우
				UIMAN->ShowCommunityTip( GAMEUI->GetSimpleTip(), pos, GAMERESOURCEMAN->GetMapName( mChannelNum ) );
			}
		}
	}
}

void cPartyStateWindow::UpdateMouseHover( cUIPos pos )
{
	if( mpBuff && mpBuff->IsHaveBuff() )
	{
		mpBuff->UpdateMouseHover( pos );
	}
}

void cPartyStateWindow::SetLevel( unsigned char level )
{
	if( mpLevel )
		mpLevel->SetValue( level );
}
