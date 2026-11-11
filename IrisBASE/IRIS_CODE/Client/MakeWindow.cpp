#include "stdafx.h"
#include "MakeWindow.h"
#include "StageManager.h"
#include "MakeStage.h"

#include "UIImage.h"
#include "EditBox.h"
#include "SpinBox.h"
#include "Button.h"
#include "PushButton.h"
#include "GameResourceManager.h"
#include "UIManager.h"

#include "FilterManager.h"

cMakeRotateWindow::cMakeRotateWindow()
: mpButtonBackImg(0)
, mpButtonSkin(0)
, mpRotateRight(0)
, mpRotateLeft(0)
, mpZoomInOut(0)
{

}

cMakeRotateWindow::~cMakeRotateWindow()
{
	SAFE_DELETE( mpButtonBackImg );
}

bool cMakeRotateWindow::OnCreate(cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpRotateRight = (cButton*)GetChild( eUIID_MAKE_RIGHTROTATE );
	mpRotateLeft = (cButton*)GetChild( eUIID_MAKE_LEFTROTATE );
	mpZoomInOut = (cPushButton*)GetChild( eUIID_MAKE_ZOOMINOUT);

	/// 회전버튼의 이벤트 세팅
	if( mpRotateLeft && mpRotateRight )
	{
		mpRotateRight->SetContinueEvent( true );
		mpRotateLeft->SetContinueEvent( true );
	}

	if( mpZoomInOut )
		mpZoomInOut->SetPress( false );

	/// 해상도에 따른 위치 갱신
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/// 버튼 스킨 이미지
	mpButtonSkin = pSkin->GetNodeSkin( "Make_Rotate_BackImage" );
	if( mpButtonSkin && mpButtonSkin->mpTexture )
	{
		mpButtonBackImg = new cUIImage( mpButtonSkin->mpTexture );
		unsigned int x = mpButtonSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpButtonSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		unsigned int tx = mpButtonSkin->mSkinInfo->mTexX;
		unsigned int ty = mpButtonSkin->mSkinInfo->mTexY;
		unsigned int w = mpButtonSkin->mSkinInfo->mWidth;
		unsigned int h = mpButtonSkin->mSkinInfo->mHeight;
		mpButtonBackImg->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ) );
		mpButtonBackImg->SetScreenRect( cUIRect( x, y, x + w, y + h ) );
	}
	return true;
}

void cMakeRotateWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	if( mpButtonSkin && mpButtonBackImg )
	{
		unsigned int x = mpButtonSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
		unsigned int y = mpButtonSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
		mpButtonBackImg->SetPos( x, y );
	}
}

void cMakeRotateWindow::OnRender(cUIFontItemKeeper* pKeeper )
{
	if( mpButtonBackImg )
		mpButtonBackImg->Draw();

	cUIWindow::OnRender( pKeeper );
}

void cMakeRotateWindow::OnCommand(cUINode* , unsigned int id )
{
	switch( id )
	{
	case eUIID_MAKE_LEFTROTATE:
		{
			/// 왼쪽 회전
			/// 이전화면
			cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
			if( p )
				p->RotateMakeCharacter( -20 );
		}
		break;
	case eUIID_MAKE_RIGHTROTATE:
		{
			/// 오른쪽 회전
			/// 이전화면
			cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
			if( p )
				p->RotateMakeCharacter( 20 );
		}
		break;
	}
}

void cMakeRotateWindow::OnPushButtonPressed( cUINode* caller, unsigned int id )
{
	if( id ==  eUIID_MAKE_ZOOMINOUT )
	{
		cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
		if( p )
			p->ToggleZoom();
	}
}

/// 버튼을 누르고 있을시 계속 발생
void cMakeRotateWindow::OnButtonContinueDowned( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_MAKE_LEFTROTATE:
		{
			/// 왼쪽 회전
			/// 이전화면
			cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
			if( p )
				p->RotateMakeCharacter( -5 );
		}
		break;
	case eUIID_MAKE_RIGHTROTATE:
		{
			/// 오른쪽 회전
			/// 이전화면
			cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
			if( p )
				p->RotateMakeCharacter( 5 );
		}
		break;
	default:
		assert(0);
		break;
	}
}


//------------------------------------------------------------------
cMakeInfoWindow::cMakeInfoWindow()
: mpMakeSelect(0)
, mRacePos(0, 0)
, mJobPos(0, 0)
, mCurrentRace(0)
, mCurrentJob(0)
, mpRaceSkin(0)
, mpJobSkin(0)
{
	::memset( mpRaceImage, 0, sizeof(mpRaceImage) );
	::memset( mpJobImage, 0, sizeof(mpJobImage) );
}

cMakeInfoWindow::~cMakeInfoWindow()
{
	for( int i = 0; i < MAX_RACE; ++i )
		SAFE_DELETE( mpRaceImage[i] );

	for( int i = 0; i < MAX_JOB; ++i )
		SAFE_DELETE( mpJobImage[i] );
}

void cMakeInfoWindow::Open()
{
	cUIWindow::Open();

	SetExceptEvent( true );
}

///
bool cMakeInfoWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpMakeSelect = (cButton*)GetChild( eUIID_MAKE_SELECT );

	/// 해상도에 따른 사이즈 갱신
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	/*cUINodeSkin* winSkin = pSkin->GetNodeSkin( mSkinName );
	if( winSkin )
	{
		int x = ( GetScreenWidth() * winSkin->mSkinInfo->mX ) / mDefaultWidth;
		int y = ( GetScreenHeight() * winSkin->mSkinInfo->mY ) / mDefaultHeight;
		int w = ( GetScreenWidth() * winSkin->mSkinInfo->mWidth ) / mDefaultWidth;
		int h = ( GetScreenHeight() * winSkin->mSkinInfo->mHeight ) / mDefaultHeight;

		SetRelativeRect( cUIRect( x, y, x+w, y+h) );
	}

	if( mpMakeSelect )
	{
		cUINodeSkin* pskin = pSkin->GetNodeSkin( mpMakeSelect->GetSkinName() );
		if( pskin )
		{
			int x = ( GetScreenWidth() * pskin->mSkinInfo->mX ) / mDefaultWidth;
			int y = ( GetScreenHeight() * pskin->mSkinInfo->mY ) / mDefaultHeight;
			mpMakeSelect->SetRelativePos( cUIPos( x, y ) );
		}
	}*/

	/// 종족 이미지 생성
	cString str;
	for( int i = 0; i < MAX_RACE; ++i )
	{
		str.Format( "MakeInfo_Race%02d", i );
		mpRaceSkin = pSkin->GetNodeSkin( str );
		if( mpRaceSkin )
		{
			mpRaceImage[i] = new cUIImage( mpRaceSkin->mpTexture );
			unsigned int x = mpRaceSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mpRaceSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			unsigned int tx = mpRaceSkin->mSkinInfo->mTexX;
			unsigned int ty = mpRaceSkin->mSkinInfo->mTexY;
			unsigned int w = mpRaceSkin->mSkinInfo->mWidth;
			unsigned int h = mpRaceSkin->mSkinInfo->mHeight;
			mpRaceImage[i]->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ) );
			mpRaceImage[i]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

			mRacePos.mX = mpRaceSkin->mSkinInfo->mX;
			mRacePos.mY = mpRaceSkin->mSkinInfo->mY;
		}
	}

	/// 직업 이미지 생성
	for( int i = 0; i < MAX_JOB; ++i )
	{
		str.Format( "MakeInfo_Job%02d", i );
		mpJobSkin = pSkin->GetNodeSkin( str );
		if( mpJobSkin )
		{
			mpJobImage[i] = new cUIImage( mpJobSkin->mpTexture );
			unsigned int x = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			unsigned int y = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			unsigned int tx = mpJobSkin->mSkinInfo->mTexX;
			unsigned int ty = mpJobSkin->mSkinInfo->mTexY;
			unsigned int w = mpJobSkin->mSkinInfo->mWidth;
			unsigned int h = mpJobSkin->mSkinInfo->mHeight;
			mpJobImage[i]->SetTextureRect( cUIRect( tx, ty, tx + w, ty + h ) );
			mpJobImage[i]->SetScreenRect( cUIRect( x, y, x + w, y + h ) );

			mJobPos.mX = mpJobSkin->mSkinInfo->mX;
			mJobPos.mY = mpJobSkin->mSkinInfo->mY;
		}
	}
	
	return true;
}

void cMakeInfoWindow::UpdateSkin()
{
	cUIWindow::UpdateSkin();

	/// 해상도에 따른 사이즈 갱신
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return;
	}

	for( int i = 0; i < MAX_RACE; ++i )
	{
		if( mpRaceImage[i] )
		{
			int x = mpRaceSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			int y = mpRaceSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mpRaceImage[i]->SetPos( x, y );
		}
	}

	for( int i = 0; i < MAX_JOB; ++i )
	{
		if( mpJobImage[i] )
		{
			int x = mpJobSkin->mSkinInfo->mX + GetAbsoluteRect().mLeft;
			int y = mpJobSkin->mSkinInfo->mY + GetAbsoluteRect().mTop;
			mpJobImage[i]->SetPos( x, y );
		}
	}

	/*cUINodeSkin* winSkin = pSkin->GetNodeSkin( mSkinName );
	if( winSkin )
	{
		int x = ( GetScreenWidth() * winSkin->mSkinInfo->mX ) / mDefaultWidth;
		int y = ( GetScreenHeight() * winSkin->mSkinInfo->mY ) / mDefaultHeight;
		int w = ( GetScreenWidth() * winSkin->mSkinInfo->mWidth ) / mDefaultWidth;
		int h = ( GetScreenHeight() * winSkin->mSkinInfo->mHeight ) / mDefaultHeight;

		SetRelativeRect( cUIRect( x, y, x+w, y+h) );
	}

	if( mpMakeSelect )
	{
		cUINodeSkin* pskin = pSkin->GetNodeSkin( mpMakeSelect->GetSkinName() );
		if( pskin )
		{
			int x = ( GetScreenWidth() * pskin->mSkinInfo->mX ) / mDefaultWidth;
			int y = ( GetScreenHeight() * pskin->mSkinInfo->mY ) / mDefaultHeight;
			mpMakeSelect->SetRelativePos( cUIPos( x, y ) );
		}
	}*/

	/*for( int i = 0; i < MAX_RACE; ++i )
	{
		if( mpRaceImage[i] )
		{
			int x = ( GetScreenWidth() * mRacePos.mX ) / mDefaultWidth;
			int y = ( GetScreenHeight() * mRacePos.mY ) / mDefaultHeight;
			mpRaceImage[i]->SetPos( x, y );
		}
	}

	for( int i = 0; i < MAX_JOB; ++i )
	{
		if( mpJobImage[i] )
		{
			int x = ( GetScreenWidth() * mJobPos.mX ) / mDefaultWidth;
			int y = ( GetScreenHeight() * mJobPos.mY ) / mDefaultHeight;
			mpJobImage[i]->SetPos( x, y );
		}
	}*/
}

void cMakeInfoWindow::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpRaceImage[mCurrentRace] )
		mpRaceImage[mCurrentRace]->Draw();

	if( mpJobImage[mCurrentJob] )
		mpJobImage[mCurrentJob]->Draw();

}

void cMakeInfoWindow::OnCommand( cUINode*, unsigned int id )
{
	switch( id )
	{
	case eUIID_MAKE_SELECT:
		/// 선택화면
		STAGEMAN->ChangeStage( eStage_Select );
		break;
	default:
		assert(0);
		break;
	}
}

/// 종족 이미지 갱신
void cMakeInfoWindow::UpdateRaceImage( int i )
{
	if( i < MAX_RACE )
		mCurrentRace = i;
}

/// 직업 이미지 갱신
void cMakeInfoWindow::UpdateJobImage( int i )
{
	if( i < MAX_JOB )
		mCurrentJob = i;
}

//////////////////////////////////////////////
cMakeWindow::cMakeWindow()
: mDisplayUpdate(eChange_None)
, mpName(0)
, mpHairSpin(0)
, mpHairColorSpin(0)
, mpFaceSpin(0)
, mpGenderSpin(0)
, mpRaceSpin(0)
, mpJobSpin(0)
, mpCreate(0)
, mUpdaetSize(0, 0)
, mpInfoWindow(0)
{
}

cMakeWindow::~cMakeWindow()
{
}

void cMakeWindow::Open()
{
	cUIWindow::Open();

	SetExceptEvent( true );

	if( mpName )
		mpName->Clear();

	if( mpCreate )
		mpCreate->SetEnabled( false );
}

void cMakeWindow::OnShow()
{
	if( mpName )
	{
		mpName->Clear();
		mpName->SetFocus();
	}

	if( mpJobSpin )
	{
		mpJobSpin->Clear();

		mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1010 ), (void*)ePLAYER_FIGHTER );
		mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1017 ), (void*)ePLAYER_MAGE );
	}

	mDisplayUpdate = eChange_None;

	cUIWindow::OnShow();
}

void cMakeWindow::OnHide()
{
	cUIWindow::OnHide();

	ResetSpinBox();
}

void cMakeWindow::SetEnableCreate( bool enable )
{
	if( mpCreate )
		mpCreate->SetEnabled( enable );
}

///
bool cMakeWindow::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	cUINode* n = GetChild( eUIID_MAKE_BACK );
	n->SetEnabled( false );

	mpName = (cEditBox*)GetChild( eUIID_MAKE_NAMEEDIT );
	mpHairSpin = (cSpinBox*)GetChild( eUIID_MAKE_HAIR_SPIN );
	mpHairColorSpin = (cSpinBox*)GetChild( eUIID_MAKE_HAIRCOLOR_SPIN );
	mpFaceSpin = (cSpinBox*)GetChild( eUIID_MAKE_FACE_SPIN );
	mpGenderSpin = (cSpinBox*)GetChild( eUIID_MAKE_GENDER_SPIN );
	mpRaceSpin = (cSpinBox*)GetChild( eUIID_MAKE_RACE_SPIN );
	mpJobSpin = (cSpinBox*)GetChild( eUIID_MAKE_JOB_SPIN );
	mpCreate = (cButton*)GetChild( eUIID_MAKE_CREATE );
		
	/// 이름에디트 copy & paste 기능 방지
	if( mpName )
	{
		mpName->SetTextColor( COLOR_WHITE );
		mpName->SetCopyPaste( false );
	}

	ResetHairSpin();
	ResetFaceSpin();
	ResetHairColorSpin();

	/// 파일을 744번때 부터 사용
	if( mpRaceSpin )
	{
		mpRaceSpin->AddData( GAMERESOURCEMAN->GetGameText( 745 ) );
		mpRaceSpin->AddData( GAMERESOURCEMAN->GetGameText( 746 ) );
		mpRaceSpin->AddData( GAMERESOURCEMAN->GetGameText( 747 ) );
	}

	if( mpJobSpin )
	{
		mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1010 ), (void*)ePLAYER_FIGHTER );
		mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1017 ), (void*)ePLAYER_MAGE );
	}

	if( mpGenderSpin )
	{
		mpGenderSpin->AddData( GAMERESOURCEMAN->GetGameText( 748 ) );
		mpGenderSpin->AddData( GAMERESOURCEMAN->GetGameText( 749 ) );
	}

	/// 해상도에 따른 위치 갱신
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
	{
		assert(0);
		return false;
	}

	return true;
}

void cMakeWindow::OnCommand( cUINode* pCaller, unsigned int id )
{
	switch( id )
	{
	case eUIID_MAKE_CREATE:
		{
			if( mpName == 0 )
				return;

			LPCTSTR name = mpName->GetText();
			if( ::_tcslen( name ) == 0 )
				return;

			/// 필터 검사
			if( FILTERMAN->CheckPartMatch( name ) == true )
			{
				mpName->Clear();

				UIMAN->CreateMsgBox( eStage_Game, eMSGBOX_MODAL,
					eMBEVENT_NONE, eSKIN_OK,
					GAMERESOURCEMAN->GetGameText( 45 ),
					GAMERESOURCEMAN->GetGameText( 301 ) );
				return;
			}

			cMakeStage* p = (cMakeStage*)STAGEMAN->GetStage( eStage_Make );
			if( p )
				p->SendMakeCharacter( name,
									  (unsigned char) mpGenderSpin->GetSelectedIdx(),
									  (unsigned char) mpHairSpin->GetSelectedIdx(),
									  (unsigned char) mpHairColorSpin->GetSelectedIdx(),
									  (unsigned char) mpFaceSpin->GetSelectedIdx() + 1 );
		}
		break;
	case eSPINBOX_LEFTBTN:
	case eSPINBOX_RIGNTBTN:
		{
			if( pCaller == mpGenderSpin )
			{
				mDisplayUpdate = eChange_Object;
				
				ResetHairSpin();
				ResetFaceSpin();
				ResetHairColorSpin();
				
			}
			else if( pCaller == mpRaceSpin )
			{
				mDisplayUpdate = eChange_Object;

				mpJobSpin->Clear();

				int race = mpRaceSpin->GetSelectedIdx();
				switch( race )
				{
				case eRACE_HUMAN:
					{
						mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1010 ), (void*)ePLAYER_FIGHTER );
						mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1017 ), (void*)ePLAYER_MAGE );
					}
					break;
				case eRACE_ELF:
					{
						mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1022 ), (void*)ePLAYER_WANDERER );
						mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1027 ), (void*)ePLAYER_WARLOCK );
					}
					break;
				case eRACE_BEAST:
					{
						mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1032 ), (void*)ePLAYER_ARMSMAN );
						mpJobSpin->AddData( GAMERESOURCEMAN->GetGameText( 1039 ), (void*)ePLAYER_RANGER );
					}
					break;
				}

				if( mpInfoWindow )
				{
					mpInfoWindow->UpdateRaceImage(race);
					mpInfoWindow->UpdateJobImage( race * 2 );
				}

				ResetHairSpin();
				ResetFaceSpin();
				ResetHairColorSpin();
			}
			else if( pCaller == mpJobSpin )
			{
				mDisplayUpdate = eChange_Form;
				
				if( mpInfoWindow )
				{
					int race = mpRaceSpin->GetSelectedIdx();
					int i = mpJobSpin->GetSelectedIdx();
					mpInfoWindow->UpdateJobImage( (race*2) + i );
				}
				
			}
			else if( pCaller == mpHairSpin )
			{
				mDisplayUpdate = eChange_Form;

				ResetHairColorSpin();
			}
			else if( pCaller == mpHairColorSpin )
			{
				mDisplayUpdate = eChange_Form;
			}
			else
			{
				mDisplayUpdate = eChange_Form;
			}
		}
		break;
	}
}
void cMakeWindow::ResetHairSpin()
{
	if( !mpHairSpin || !mpRaceSpin || !mpGenderSpin )
	{
		assert(0);
		return;
	}

	int nOldSelecteIdx = mpHairSpin->GetSelectedIdx();

	mpHairSpin->Clear();

	unsigned int hairCnt = GAMERESOURCEMAN->GetDefaultHairCount();

	unsigned int nStartIndex = ( mpRaceSpin->GetSelectedIdx() * hairCnt * eGENDER_MAX ) + mpGenderSpin->GetSelectedIdx() * hairCnt;

	/// 파일을 11300번때 부터 사용
	for( unsigned int i=0; i< hairCnt; ++i )
	{
		mpHairSpin->AddData( UIMAN->GetUIText( HAIR_NAMING_START_INDEX + nStartIndex + i ) );
	}

	mpHairSpin->SetSelectIdx( nOldSelecteIdx );
}

void cMakeWindow::ResetFaceSpin()
{
	if( !mpFaceSpin || !mpRaceSpin || !mpGenderSpin )
	{
		assert(0);
		return;
	}

	int nOldSelecteIdx = mpFaceSpin->GetSelectedIdx();

	mpFaceSpin->Clear();

	unsigned int faceCnt = GAMERESOURCEMAN->GetDefaultFaceCount();

	unsigned int nStartIndex = ( mpRaceSpin->GetSelectedIdx() * faceCnt * eGENDER_MAX ) + mpGenderSpin->GetSelectedIdx() * faceCnt;

	/// 파일을 11300번때 부터 사용
	for( unsigned int i=0; i< faceCnt; ++i )
	{
		mpFaceSpin->AddData( UIMAN->GetUIText( FACE_NAMING_START_INDEX + nStartIndex + i ) );
	}

	mpFaceSpin->SetSelectIdx( nOldSelecteIdx );

}

void cMakeWindow::ResetHairColorSpin()
{
	if( !mpHairSpin || !mpRaceSpin || !mpGenderSpin || !mpHairSpin )
	{
		assert(0);
		return;
	}

	int nOldSelecteIdx = mpHairColorSpin->GetSelectedIdx();

	mpHairColorSpin->Clear();

	unsigned int hairColorCnt = GAMERESOURCEMAN->GetHairColorCount();
	unsigned int hairCnt = GAMERESOURCEMAN->GetDefaultHairCount();
	unsigned int nOffset = hairColorCnt * hairCnt;

	unsigned int nStartIndex =	( mpRaceSpin->GetSelectedIdx() * nOffset * eGENDER_MAX ) + 
								( mpGenderSpin->GetSelectedIdx() * nOffset) + 
								( mpHairSpin->GetSelectedIdx()	* hairColorCnt ) ;

	/// 파일을 11400번때 부터 사용
	for( unsigned int i = 0; i < hairColorCnt; ++i )
	{
		mpHairColorSpin->AddData( UIMAN->GetUIText( HAIRCOLOR_NAMING_START_INDEX + nStartIndex + i ) );
	}

	mpHairColorSpin->SetSelectIdx( nOldSelecteIdx );	
}


/// 
void cMakeWindow::ResetSpinBox()
{
	if( mpGenderSpin )
		mpGenderSpin->SetSelectIdx(0);

	if( mpHairSpin )
		mpHairSpin->SetSelectIdx(0);

	if( mpHairColorSpin )
		mpHairColorSpin->SetSelectIdx(0);

	if( mpFaceSpin )
		mpFaceSpin->SetSelectIdx(0);

	if( mpRaceSpin )
		mpRaceSpin->SetSelectIdx(0);

	if( mpJobSpin )
		mpJobSpin->SetSelectIdx(0);

	if( mpInfoWindow )
	{
		mpInfoWindow->UpdateRaceImage(0);
		mpInfoWindow->UpdateJobImage(0);
	}
}

void cMakeWindow::GetDisplayInfo( sPlayerInfo* info )
{
	if( mpGenderSpin )
		info->Gender = (unsigned char)mpGenderSpin->GetSelectedIdx();

	if( mpHairSpin )
		info->HeadInfo[eHEAD_HAIR] = mpHairSpin->GetSelectedIdx();

	if( mpHairColorSpin )
		info->HeadInfo[eHEAD_HAIRCOLOR] = (unsigned short)mpHairColorSpin->GetSelectedIdx();	

	if( mpFaceSpin )
		info->HeadInfo[eHEAD_FACE] = mpFaceSpin->GetSelectedIdx() + 1;

	if( mpRaceSpin )
	{
		info->Race = (unsigned char)mpRaceSpin->GetSelectedIdx();
	}

	if( mpJobSpin )
	{
		int job = (int)mpJobSpin->GetExtraData( mpJobSpin->GetSelectedIdx() );
		info->Job = (ePLAYER_JOB)job;
	}
}

void cMakeWindow::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if ( mVisible == false )
		return;
	if( mpName )
	{
		if( mpName->GetLength() > 0 )
			mpCreate->SetEnabled( true );
		else
			mpCreate->SetEnabled( false );
	}
}
