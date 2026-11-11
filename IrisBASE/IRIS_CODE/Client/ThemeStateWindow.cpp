#include "stdafx.h"
#include "ThemeStateWindow.h"
#include "ListFolderBox.h"
#include "TextBox.h"
#include "Theme_Common.h"
#include "GameResourceManager.h"
#include "Application.h"
#include "UIImage.h"
#include "ItemManager.h"

cThemeStateWindow::cThemeStateWindow()
: mpFolderBox(0)
, mpInfoBox(0)
, mpHighImage(0)
, mTimeLineNum(-1)
{
}

cThemeStateWindow::~cThemeStateWindow()
{
	Clear();
	SAFE_DELETE( mpHighImage );
}

void cThemeStateWindow::Clear()
{
	if( mpInfoBox)
		mpInfoBox->Clear();

	if( mpFolderBox )
		mpFolderBox->Clear();

	mTimeLineNum = -1;
}

void cThemeStateWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

bool cThemeStateWindow::OnCreate( cUINodeProperty* property )
{
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	mpFolderBox = (cListFolderBox*)GetChild( eUIID_GAME_THEME_STATE_LIST_FOLDER_BOX );
	mpInfoBox = (cTextBox*)GetChild( eUIID_GAME_THEME_STATE_DESCBOX );

	/// 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* pHighSkin = pSkin->GetNodeSkin( "ThemeState_FolderCheck" );
	cUINodeSkin* pHighNoSkin = pSkin->GetNodeSkin( "ThemeState_FolderNoCheck" );
	if( pHighSkin && pHighNoSkin )
	{
		mpHighImage = new cUIImage( pHighSkin->mpTexture );

		unsigned int tx = pHighSkin->mSkinInfo->mTexX;
		unsigned int ty = pHighSkin->mSkinInfo->mTexY;
		unsigned int w = pHighSkin->mSkinInfo->mWidth;
		unsigned int h = pHighSkin->mSkinInfo->mHeight;
		unsigned int x = pHighSkin->mSkinInfo->mX;
		unsigned int y = pHighSkin->mSkinInfo->mY;

		mpHighImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpHighImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		unsigned int tx2 = pHighNoSkin->mSkinInfo->mTexX;
		unsigned int ty2 = pHighNoSkin->mSkinInfo->mTexY;

		mpFolderBox->SetHighNoCheckPos( tx, ty );
		mpFolderBox->SetHighCheckPos( tx2, ty2 );

		/// 하위열 출력위치 조정
		mpFolderBox->SetLowGap( w );
	}

	return true;
}

void cThemeStateWindow::OnCommand( cUINode*, unsigned int id )
{
	if( id == eUIID_GAME_THEME_STATE_CLOSE )
		Hide();
}

void cThemeStateWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible == false )
		return;

	if( !mpFolderBox )
		return;

	if( mTimeLineNum != -1 )
	{
		unsigned int row  = mpFolderBox->GetSelectRow();
		unsigned int column = mpFolderBox->GetSelectRowColumn();
		sPlayerThemeList* pThemeInfo = (sPlayerThemeList*)mpFolderBox->GetColumnData( row, column );
		if( pThemeInfo )
		{
			cStringT time;

			/// 분의 올림 처리를 위해 1분을 더한다. ex) 1분 20 초 -> 2분으로 표시하기 위해서.
			unsigned long reftTime = pThemeInfo->mResetLeftTime + (59 * SECOND);

			unsigned long d  = reftTime / DAY;
			unsigned long h = ( reftTime % DAY ) / HOUR;
			unsigned long m  = ( reftTime % HOUR ) / MINUTE;

			if( d > 0 )
				time.Format( GAMERESOURCEMAN->GetGameText( 9009 ), d, h, m );
			else if( h > 0 )
				time.Format( GAMERESOURCEMAN->GetGameText( 9010 ), h, m );
			else if( m > 1 )
				time.Format( GAMERESOURCEMAN->GetGameText( 9011 ), m );
			else
			{
				reftTime -= (59 * SECOND);
				unsigned long s  = reftTime / SECOND;

				time.Format( GAMERESOURCEMAN->GetGameText( 9019 ), s );
			}

			mpInfoBox->UpdateRowText( mTimeLineNum, time.Cstr() );
		}
	}
}

void cThemeStateWindow::OnShow()
{
	UpdateInfo();
}

void cThemeStateWindow::OnListBoxChanged( cUINode*, unsigned int id )
{
	if( id == eUIID_GAME_THEME_STATE_LIST_FOLDER_BOX )
	{
		UpdateInfo();
	}
}

void cThemeStateWindow::AddInfo( sPlayerThemeList* info )
{
	// 리스트박스 갱신
	if( info && mpFolderBox )
	{
		LPCTSTR modeName = UIMAN->GetUIText( 26500 + info->mThemeMode );
		LPCTSTR mapName = GAMERESOURCEMAN->GetMapName( info->mMapNumber );

		mpFolderBox->AddHighData( info->mThemeMode, modeName, COLOR_YELLOW, 0, mpHighImage );
		mpFolderBox->AddLowData(  info->mThemeMode, mapName, COLOR_WHITE, info );
	}
	else
		assert(0);
}

void cThemeStateWindow::UpdateInfo()
{
	if( mpFolderBox == 0 || mpInfoBox == 0 )
	{
		assert(0);
		return;
	}

	mpInfoBox->Clear();

	unsigned int row  = mpFolderBox->GetSelectRow();
	unsigned int column = mpFolderBox->GetSelectRowColumn();
	sPlayerThemeList* info = (sPlayerThemeList*)mpFolderBox->GetColumnData( row, column );

	if( info )
	{
		cStringT temp;
		cStringT time;

		/// 분의 올림 처리를 위해 1분을 더한다. ex) 1분 20 초 -> 2분으로 표시하기 위해서.	
		unsigned long reftTime = info->mResetLeftTime + (59 * SECOND);
		
		unsigned long d  = reftTime / DAY;
		unsigned long h = ( reftTime % DAY ) / HOUR;
		unsigned long m  = ( reftTime % HOUR ) / MINUTE;

		if( d > 0 )
			time.Format( GAMERESOURCEMAN->GetGameText( 9009 ), d, h, m );
		else if( h > 0 )
			time.Format( GAMERESOURCEMAN->GetGameText( 9010 ), h, m );
		else if( m > 1 )
			time.Format( GAMERESOURCEMAN->GetGameText( 9011 ), m );
		else
		{
			reftTime -= (59 * SECOND);
			unsigned long s  = reftTime / SECOND ;
			time.Format( GAMERESOURCEMAN->GetGameText( 9019 ), s );
		}

		LPCTSTR mapName = GAMERESOURCEMAN->GetMapName( info->mMapNumber );
		mpInfoBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 9016 ), COLOR_YELLOW );
		mpInfoBox->AddNewRow( mapName, COLOR_WHITE );
		mpInfoBox->AddEmptyRow( 1 );

		temp.Format(_T("%d"), info->mUniqueIdx );
		mpInfoBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 9017 ), COLOR_YELLOW );
		mpInfoBox->AddNewRow( temp.Cstr(), COLOR_WHITE );
		mpInfoBox->AddEmptyRow( 1 );

		mpInfoBox->AddNewRow( GAMERESOURCEMAN->GetGameText( 9018 ), COLOR_YELLOW );
		mTimeLineNum = mpInfoBox->AddNewRow( time.Cstr(), COLOR_WHITE );
	}
}

//---------------------------------------------------------------------------------
//
cThemeResetWindow::cThemeResetWindow()
: mpReset(0)
, mInventorySlotIndex( USHRT_MAX )
{

}

cThemeResetWindow::~cThemeResetWindow()
{
}

void cThemeResetWindow::Close()
{
	cUIWindow::Close();

	Clear();
}

void cThemeResetWindow::Show( unsigned short slotIndex )
{
	// 기존 호출 아이템 락 풀기
	if( mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			cItem* Item = ITEMMAN->GetItem( mInventorySlotIndex );
			if( Item )
				Item->SetLock( false );
		}

		mInventorySlotIndex = USHRT_MAX; 
	}

	// 셋팅
	mInventorySlotIndex = slotIndex;

	// 메가폰 호출 아이템 락 걸기
	cItem* item = ITEMMAN->GetItem( slotIndex );
	if( item )
		item->SetLock( true );

	cUIWindow::Show();
}

void cThemeResetWindow::OnHide()
{
	cUIWindow::OnHide();

	// 락 풀기
	if( mInventorySlotIndex != USHRT_MAX )
	{
		if( mInventorySlotIndex <= MAX_INVENTORY )
		{
			cItem* Item = ITEMMAN->GetItem( mInventorySlotIndex );
			if( Item )
				Item->SetLock( false );
		}

		mInventorySlotIndex = USHRT_MAX; 
	}
}

bool cThemeResetWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpReset = GetChild( eUIID_GAME_THEME_RESET_RESET );
	mpFolderBox = (cListFolderBox*)GetChild( eUIID_GAME_THEME_RESET_LISTFOLDER );
	mpInfoBox = (cTextBox*)GetChild( eUIID_GAME_THEME_RESET_DESCBOX );

	/// 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	cUINodeSkin* pHighSkin = pSkin->GetNodeSkin( "ThemeState_FolderCheck" );
	cUINodeSkin* pHighNoSkin = pSkin->GetNodeSkin( "ThemeState_FolderNoCheck" );
	if( pHighSkin && pHighNoSkin )
	{
		mpHighImage = new cUIImage( pHighSkin->mpTexture );

		unsigned int tx = pHighSkin->mSkinInfo->mTexX;
		unsigned int ty = pHighSkin->mSkinInfo->mTexY;
		unsigned int w = pHighSkin->mSkinInfo->mWidth;
		unsigned int h = pHighSkin->mSkinInfo->mHeight;
		unsigned int x = pHighSkin->mSkinInfo->mX;
		unsigned int y = pHighSkin->mSkinInfo->mY;

		mpHighImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpHighImage->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

		unsigned int tx2 = pHighNoSkin->mSkinInfo->mTexX;
		unsigned int ty2 = pHighNoSkin->mSkinInfo->mTexY;

		mpFolderBox->SetHighNoCheckPos( tx, ty );
		mpFolderBox->SetHighCheckPos( tx2, ty2 );

		/// 하위열 출력위치 조정
		mpFolderBox->SetLowGap( w );
	}
	return true;
}

void cThemeResetWindow::OnListBoxChanged( cUINode*, unsigned int id )
{
	if( id == eUIID_GAME_THEME_RESET_LISTFOLDER )
	{
		UpdateInfo();
	}
}

void cThemeResetWindow::OnCommand( cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_GAME_THEME_RESET_CLOSE:
		Hide();
		break;
	case eUIID_GAME_THEME_RESET_RESET:
		{
			// 선택한 항목 잔여시간 체크
			unsigned int row  = mpFolderBox->GetSelectRow();
			unsigned int column = mpFolderBox->GetSelectRowColumn();
			sPlayerThemeList* info = (sPlayerThemeList*)mpFolderBox->GetColumnData( row, column );
			if( !info )
				return;

			// 5분 미만 초기화 불가
			unsigned long h = ( info->mResetLeftTime / HOUR );
			unsigned long m = ( info->mResetLeftTime % HOUR ) / MINUTE;
			if( h == 0 && m <= 5 )
			{
				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODALLESS,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 12007 ),
					GAMERESOURCEMAN->GetGameText( 301 ),
					0, this );
				return;
			}

			// 
			UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
				eMBEVENT_THEME_RESET, eSKIN_YESNO,
				GAMERESOURCEMAN->GetGameText( 12009 ),
				GAMERESOURCEMAN->GetGameText( 301 ),
				(void*)mInventorySlotIndex );
		}
		break;
	}
}

void cThemeResetWindow::OnNetLock( int lockTry )
{
	if( mpReset )
		mpReset->NetLock( lockTry );
}

void cThemeResetWindow::OnNetUnLock( int lockTry )
{
	if( mpReset )
		mpReset->NetUnLock( lockTry );
}

sPlayerThemeList* cThemeResetWindow::GetSelectData()
{
	if( mpFolderBox )
	{
		unsigned int row  = mpFolderBox->GetSelectRow();
		unsigned int column = mpFolderBox->GetSelectRowColumn();
		return (sPlayerThemeList*)mpFolderBox->GetColumnData( row, column );
	}
	return 0;
}

void cThemeResetWindow::UpdateInfo()
{
	cThemeStateWindow::UpdateInfo();

	unsigned int row  = mpFolderBox->GetSelectRow();
	unsigned int column = mpFolderBox->GetSelectRowColumn();
	sPlayerThemeList* info = (sPlayerThemeList*)mpFolderBox->GetColumnData( row, column );

	bool enable = ( info ) ? true : false;

	if( mpReset )
		mpReset->SetEnabled( enable );
}
