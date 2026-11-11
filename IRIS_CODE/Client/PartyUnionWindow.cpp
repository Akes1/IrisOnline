#include "stdafx.h"
#include "PartyUnionWindow.h"
#include "GameUIManager.h"
#include "UnionIcon.h"
#include "UIImage.h"
#include "RButtonMenuWindow.h"
#include "PartyUnionManager.h"

cPartyUnionWindow::cPartyUnionWindow()
: mpLeaderImage(0)
, mpSelectImage(0)
, mpPvpLeaderImage(0)
, mpFlagPlayerImage(0)
, mSelectArrIndex(UINT_MAX)
, mLeaderArrIndex(UINT_MAX)
, mPvpLeaderArrIndex(UINT_MAX)
, mFlagPlayerArrIndex(UINT_MAX)
, mEnableIcon( false )
{
	::memset( mIcon, 0, sizeof(mIcon) );
}

cPartyUnionWindow::~cPartyUnionWindow()
{
	SAFE_DELETE( mpFlagPlayerImage );
	SAFE_DELETE( mpPvpLeaderImage );
	SAFE_DELETE( mpSelectImage );
	SAFE_DELETE( mpLeaderImage );
}

void cPartyUnionWindow::Clear()
{
	for( int i = 0; i < MAX_PARTYUNION; ++i )
	{
		if( mIcon[i] )
			mIcon[i]->ClearInfo();
	}

	mLeaderArrIndex = UINT_MAX;
	mSelectArrIndex = UINT_MAX;
	mPvpLeaderArrIndex = UINT_MAX;
	mFlagPlayerArrIndex = UINT_MAX;
}

void cPartyUnionWindow::Open()
{
}

void cPartyUnionWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

bool cPartyUnionWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cString str;
	for( unsigned int i = 0; i < MAX_PARTYUNION; ++i )
	{
		str.Format( "PartyUnion_Slot%02d", i );
		cUnionIcon* icon = new cUnionIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return false;

		mIcon[i] = icon;
	}

	// 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 리더이미지
	cUINodeSkin* leaderSkin = pSkin->GetNodeSkin( "PartyUnion_Leader" );
	if( leaderSkin && leaderSkin->mpTexture )
	{
		mpLeaderImage = new cUIImage( leaderSkin->mpTexture );
		unsigned int tx = leaderSkin->mSkinInfo->mTexX;
		unsigned int ty = leaderSkin->mSkinInfo->mTexY;
		unsigned int w = leaderSkin->mSkinInfo->mWidth;
		unsigned int h = leaderSkin->mSkinInfo->mHeight;
		unsigned int x = leaderSkin->mSkinInfo->mX;
		unsigned int y = leaderSkin->mSkinInfo->mY;
		mpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		SetLeader( 0 );
	}

	/// 선택 이미지
	cUINodeSkin* selectSkin = pSkin->GetNodeSkin( "PartyUnion_Select" );
	if( selectSkin && selectSkin->mpTexture )
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

	/// 수장 이미지
	cUINodeSkin* pvpLeaderSkin = pSkin->GetNodeSkin( "PartyUnion_PvpLeader" );
	if( pvpLeaderSkin && pvpLeaderSkin->mpTexture )
	{
		mpPvpLeaderImage = new cUIImage( pvpLeaderSkin->mpTexture );
		unsigned int tx = pvpLeaderSkin->mSkinInfo->mTexX;
		unsigned int ty = pvpLeaderSkin->mSkinInfo->mTexY;
		unsigned int w  = pvpLeaderSkin->mSkinInfo->mWidth;
		unsigned int h  = pvpLeaderSkin->mSkinInfo->mHeight;
		unsigned int x  = pvpLeaderSkin->mSkinInfo->mX;
		unsigned int y  = pvpLeaderSkin->mSkinInfo->mY;
		mpPvpLeaderImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpPvpLeaderImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	/// 플래그 매치에서 깃발든 유저 이미지
	cUINodeSkin* flagPlayerSkin = pSkin->GetNodeSkin( "PartyUnion_FlagPlayer" );
	if( flagPlayerSkin && flagPlayerSkin->mpTexture )
	{
		mpFlagPlayerImage = new cUIImage( flagPlayerSkin->mpTexture );
		unsigned int tx = flagPlayerSkin->mSkinInfo->mTexX;
		unsigned int ty = flagPlayerSkin->mSkinInfo->mTexY;
		unsigned int w  = flagPlayerSkin->mSkinInfo->mWidth;
		unsigned int h  = flagPlayerSkin->mSkinInfo->mHeight;
		unsigned int x  = flagPlayerSkin->mSkinInfo->mX;
		unsigned int y  = flagPlayerSkin->mSkinInfo->mY;
		mpFlagPlayerImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpFlagPlayerImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}

	return true;
}

void cPartyUnionWindow::UpdateRect()
{
	cUIWindow::UpdateRect();

	SetSelect( mSelectArrIndex );
	SetLeader( mLeaderArrIndex );
	SetPvpLeader( mPvpLeaderArrIndex );
	SetFlagPlayer( mFlagPlayerArrIndex );
}

void cPartyUnionWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpSelectImage && mSelectArrIndex != -1 )
		mpSelectImage->Draw();

	if( mpLeaderImage && mLeaderArrIndex != -1 )
		mpLeaderImage->Draw();

	if( mpPvpLeaderImage && mPvpLeaderArrIndex != -1 )
		mpPvpLeaderImage->Draw();

	if( mpFlagPlayerImage && mFlagPlayerArrIndex != -1 )
		mpFlagPlayerImage->Draw();
}

void cPartyUnionWindow::OnCommand( cUINode* , unsigned int id )
{
	if( id == eUIID_PARTYUNION_SMALL )
	{
		PARTYUNIONMAN->ShowLargeUnionUI( false );
	}
}

void cPartyUnionWindow::OnIconDragged( cUINode* caller, unsigned int , const cUIPos& pos, bool shift )
{
	if( mEnableIcon == false )
		return;

	cUnionIcon* icon = (cUnionIcon*)caller;
	if( icon )
		UIMAN->ShowUnionDrag( this, icon, pos );
}

void cPartyUnionWindow::OnIconClicked( cUINode* , unsigned int id )
{
	sUnionUserData* data = PARTYUNIONMAN->GetData( id );
	if( data )
	{
		PARTYUNIONMAN->SetTargetByUI( data->userIndex );
	}
}

void cPartyUnionWindow::OnIconRightClicked( cUINode* caller, unsigned int , bool )
{
	cUnionIcon* icon = (cUnionIcon*)caller;
	if( icon )
	{
		cUIPos pos = icon->GetAbsoluteRect().GetCenter();

		sUnionUserData* data = PARTYUNIONMAN->GetData( icon->GetID() );
		cRButtonMenuWindow* win = GAMEUI->GetRButtonMenuWindow();
		if( win && data && data->userIndex != 0 )
		{
			win->PopUp( pos.mX, pos.mY, data->userIndex, icon->GetName(), ePopupPos_PartyWindow );
		}
	}
}

void cPartyUnionWindow::SetInfo( unsigned int i, LPCTSTR name, unsigned int maxhp, unsigned int hp, unsigned short job, unsigned char level )
{
	if( i < 0 || i >= MAX_PARTYUNION )
	{
		assert(0);
		return;
	}

	if( mIcon[i] )
	{
		mIcon[i]->SetInfo( name, maxhp, hp, job, level );
	}
	else
		assert(0);
}

void cPartyUnionWindow::UpdateInfo( unsigned int i, unsigned int maxhp, unsigned int hp )
{
	if( i < 0 || i >= MAX_PARTYUNION )
	{
		assert(0);
		return;
	}

	if( mIcon[i] )
	{
		mIcon[i]->UpdateGaugeInfo( maxhp, hp );
	}
	else
		assert(0);
}

void cPartyUnionWindow::ClearInfo( unsigned int i )
{
	if( i < 0 || i >= MAX_PARTYUNION )
	{
		assert(0);
		return;
	}

	if( mIcon[i] )
	{
		mIcon[i]->ClearInfo();

		// 선택정보 초기화
		if( mSelectArrIndex == i )
		{
			SetSelect( UINT_MAX );
		}
		/// 전장 수장 표시 정보 초기화
		if( mPvpLeaderArrIndex == i )
		{
			SetPvpLeader( UINT_MAX );
		}

		/// 플래그 매치 깃발 소유 정보 초기화
		if( mFlagPlayerArrIndex == i )
		{
			SetFlagPlayer( UINT_MAX );
		}
		
	}
	else
		assert(0);
}

void cPartyUnionWindow::SetLeader( unsigned int i )
{
	mLeaderArrIndex = i;

	if( i >= 0 && i < MAX_PARTYUNION )
	{
		if( mIcon[i] )
		{
			cUIRect rc = mIcon[i]->GetAbsoluteRect();

			if( mpLeaderImage )
				mpLeaderImage->SetPos( rc.mLeft, rc.mTop );
		}
		else
			assert(0);
	}
}

void cPartyUnionWindow::SetSelect( unsigned int i )
{
	mSelectArrIndex = i;

	if( i >= 0 && i < MAX_PARTYUNION )
	{
		if( mIcon[i] )
		{
			cUIRect rc = mIcon[i]->GetAbsoluteRect();

			if( mpSelectImage )
				mpSelectImage->SetPos( rc.mLeft, rc.mTop );
		}
		else
			assert(0);
	}
}

void cPartyUnionWindow::SetPvpLeader( unsigned int i )
{
	mPvpLeaderArrIndex = i;

	if( i >= 0 && i < MAX_PARTYUNION )
	{
		if( mIcon[i] )
		{
			cUIRect rc = mIcon[i]->GetAbsoluteRect();

			if( mpPvpLeaderImage )
				mpPvpLeaderImage->SetPos( rc.mLeft, rc.mTop );
		}
		else
			assert(0);
	}
}

void cPartyUnionWindow::SetFlagPlayer( unsigned int i )
{
	mFlagPlayerArrIndex = i;

	if( i >= 0 && i < MAX_PARTYUNION )
	{
		if( mIcon[i] )
		{
			cUIRect rc = mIcon[i]->GetAbsoluteRect();

			if( mpFlagPlayerImage )
				mpFlagPlayerImage->SetPos( rc.mLeft, rc.mTop );
		}
		else
			assert(0);
	}
}


void cPartyUnionWindow::EnableLeader( bool enable )
{
	// 팝업메뉴와 아이콘 무브 사용가능
	mEnableIcon = enable;
}

void cPartyUnionWindow::OnNetLock( int lockTry )
{

}

void cPartyUnionWindow::OnNetUnLock( int lockTry )
{

}

//----------------------------------------------------
// 연합윈도우 접기
cPartyUnionSmallWindow::cPartyUnionSmallWindow()
{

}

cPartyUnionSmallWindow::~cPartyUnionSmallWindow()
{

}

bool cPartyUnionSmallWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	return true;
}

void cPartyUnionSmallWindow::OnCommand( cUINode* , unsigned int id )
{
	if( id == eUIID_PARTYUNIONSMALL_LARGE )
	{
		PARTYUNIONMAN->ShowLargeUnionUI( true );
	}
}
