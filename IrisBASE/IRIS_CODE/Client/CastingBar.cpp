#include "StdAfx.h"
#include "CastingBar.h"
#include "Gauge.h"

#include "SkillScript.h"
#include "GatheringScript.h"
#include "Gathering_Common.h"
#include "GameResourceManager.h"
#include "Icon.h"
#include "Label.h"
#include "PlaneObject.h"

cCastingBar::cCastingBar()
: mpCastingBar(0)
, mType(eCasting_Max)
, mCastingIndex(0)
, mpName(0)
, mpTime(0)
, mpIconImage(0)
, mIconPos(0, 0)
, mStartTime(0)
, mRestTime(0)
, mpCastingBarEndEffect(0)
{
}

cCastingBar::~cCastingBar()
{
	SAFE_DELETE( mpIconImage );
	SAFE_DELETE( mpCastingBarEndEffect );
}

void cCastingBar::SetTime( unsigned long index, eCASTINGTYPE type, unsigned long sec )
{
	if( sec <= 0 )
	{
		assert(0);
		return;
	}
	if( type >= eCasting_Max )
	{
		assert(0);
		return;
	}

	if( mpCastingBar )
	{
		float speed = 100.0f / ((float)sec*0.001f);

		if( speed == 0.0f )
			return;

		mpCastingBar->SetGaugeSpeed( speed );

		mType = type;
		mCastingIndex = index;

		mStartTime = ULONG_MAX;
		mRestTime = sec;

		///
		UIMAN->GotoFrontNode( this );
		SetVisible( true );

		OnShow();
	}
}

void cCastingBar::Clear( eCASTINGTYPE type )
{
	if( type == mType )
	{
		SetVisible( false );
		OnHide();
	}
}

void cCastingBar::Open()
{

}

void cCastingBar::Close()
{
	cUIWindow::Close();

	if( mpCastingBar )
		mpCastingBar->ClearFront();

	mStartTime = ULONG_MAX;
	mRestTime = 0;

	mCastingIndex = 0;
	mType = eCasting_Max;
}

void cCastingBar::OnShow()
{
	if( mpCastingBar )
		mpCastingBar->SetGaugeAni( 100.0f );

	switch( mType )
	{
	case eCasting_Skill:
		{
			/// 스킬 정보 세팅
			sPlayerSkillBaseInfo* info = SKILLSCRIPT->GetPlayerSkillInfo( mCastingIndex );
			if( info )
			{
				/// 스킬명
				LPCTSTR name = GAMERESOURCEMAN->GetSkillNameText( info->mNameIdx );
				if( mpName )
					mpName->SetText( name );

				/// 스킬 아이콘
				cIconParam* pParam = UIMAN->GetIconParam( info->mIconIdx );
				if( pParam )
				{
					if( mpIconImage == 0 )
						mpIconImage = new cUIImage( pParam->mpTexture );
					else
						mpIconImage->SetTexture( pParam->mpTexture );

					unsigned int x = GetAbsoluteRect().mLeft + mIconPos.mX;
					unsigned int y = GetAbsoluteRect().mTop + mIconPos.mY;
					unsigned int tx = pParam->mTexPos.mX;
					unsigned int ty = pParam->mTexPos.mY;
					mpIconImage->SetTextureRect( tx, ty, tx + 28, ty + 28 );
					mpIconImage->SetScreenRect( cUIRect( x, y, x + 28, y + 28 ) );
				}
				else
					assert(0);
			}
		}
		break;
	case eCasting_Gather:
		{
			sGatheringList* info = GATHERINGSCRIPT->GetGatheringInfo( mCastingIndex );
			if( info )
			{
				LPCTSTR name = GAMERESOURCEMAN->GetGatheringName( info->mNameIdx );
				if( mpName )
					mpName->SetText( name );

				cIconParam* pParam = UIMAN->GetIconParam( info->mIconIndex );
				if( pParam )
				{
					if( mpIconImage == 0 )
						mpIconImage = new cUIImage( pParam->mpTexture );
					else
						mpIconImage->SetTexture( pParam->mpTexture );

					unsigned int x = GetAbsoluteRect().mLeft + mIconPos.mX;
					unsigned int y = GetAbsoluteRect().mTop + mIconPos.mY;
					unsigned int tx = pParam->mTexPos.mX;
					unsigned int ty = pParam->mTexPos.mY;
					mpIconImage->SetTextureRect( tx, ty, tx + 28, ty + 28 );
					mpIconImage->SetScreenRect( cUIRect( x, y, x + 28, y + 28 ) );
				}
				else
					assert(0);
			}
		}
		break;
	default:
		{
			assert(0);
			cUIWindow::Hide();
		}
		return;
	}

	cUIWindow::OnShow();
}

void cCastingBar::OnHide()
{
	cUIWindow::OnHide();

	if( mpCastingBar )
		mpCastingBar->ClearFront();

	mStartTime = ULONG_MAX;
	mRestTime = 0;

	mCastingIndex = 0;
	mType = eCasting_Max;
}

/// 주의사항 :
bool cCastingBar::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	mpCastingBar = (cGauge*)GetChild( eUIID_GAME_CASTINGBAR_GAUGEBAR );
	mpName = (cLabel*)GetChild( eUIID_GAME_CASTINGBAR_SKILLNAME );
	mpTime = (cLabel*)GetChild( eUIID_GAME_CASTINGBAR_SKILLTIME );

	if( mpCastingBar )
		mpCastingBar->SetMaxValue( 100.0f );

	/// 스킬 아이콘 이미지 생성
	cUISkin* pSkin = UIMAN->GetSkin();
	if( !pSkin )
		return false;

	cUINodeSkin* pIconSkin = pSkin->GetNodeSkin( "Casting_SkillIcon" );
	if( pIconSkin )
	{
		mIconPos.mX = pIconSkin->mSkinInfo->mX;
		mIconPos.mY = pIconSkin->mSkinInfo->mY;
	}
	else
	{
		assert(0);
		return false;
	}

	cUINodeSkin* pCastingBarEndSkin = pSkin->GetNodeSkin( "Casting_GuageEnd" );
	if( pCastingBarEndSkin )
	{
		unsigned int tx = pCastingBarEndSkin->mSkinInfo->mTexX;
		unsigned int ty = pCastingBarEndSkin->mSkinInfo->mTexY;
		unsigned int tw = pCastingBarEndSkin->mSkinInfo->mWidth;
		unsigned int th = pCastingBarEndSkin->mSkinInfo->mHeight;

		mpCastingBarEndEffect = new cPlaneObject;
		if( mpCastingBarEndEffect->Create( pCastingBarEndSkin->mpTexture, 0, 0, tw, th, tx, ty, tx+tw,ty+th ) == false )
		{
			assert( 0 && "failed to create back image");
			return false;
		}

		cPlaneObject* pGaugeBar = mpCastingBar->GetPlaneObject();
		if( pGaugeBar )
		{			
			mpCastingBarEndEffect->SetScreenY( pGaugeBar->GetScreenY() );
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

void cCastingBar::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpIconImage )
	{
		unsigned int x = GetAbsoluteRect().mLeft + mIconPos.mX;
		unsigned int y = GetAbsoluteRect().mTop + mIconPos.mY;
		mpIconImage->SetPos( x, y );
	}
}

void cCastingBar::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	if( mType >= eCasting_Max )
		return;

	if( mRestTime == 0 )
		return;

	if( mStartTime == ULONG_MAX )
		mStartTime = accumTime;

	unsigned long elapsed = accumTime - mStartTime;

	if( mRestTime > elapsed )
	{
		mRestTime -= elapsed;
		mStartTime = accumTime;
	}
	else
	{
		mRestTime = 0;
		mStartTime = ULONG_MAX;
	}

	/// 시간 출력
	float s = (float)( mRestTime * 0.001 );
	SetTimeText( s );

	cPlaneObject* pGaugeBar = mpCastingBar->GetPlaneObject();
	if( pGaugeBar )
	{	
		int nGaugeEndPosX = pGaugeBar->GetScreenX() + pGaugeBar->GetScreenW();
		int nWidth = (int)(mpCastingBarEndEffect->GetScreenW() * 0.5f);

		nGaugeEndPosX -= nWidth;

		mpCastingBarEndEffect->SetScreenX(nGaugeEndPosX );
		mpCastingBarEndEffect->SetScreenY( pGaugeBar->GetScreenY() );
	}	
}

void cCastingBar::OnRender( cUIFontItemKeeper* pKeeper )
{
	cUIWindow::OnRender( pKeeper );

	if( mpIconImage )
		mpIconImage->Draw();

	if( mpCastingBarEndEffect )
		mpCastingBarEndEffect->Draw();
}

void cCastingBar::OnGaugeEnd( cUINode* n , unsigned int )
{
	if( mpCastingBar && n == mpCastingBar )
	{
		cUIWindow::Hide();
	}
}

float cCastingBar::GetPercent()
{
	if( mpCastingBar )
		return mpCastingBar->GetPercent();

	return 100.0f;
}

void cCastingBar::SetTimeText( float time )
{
	if( mpTime )	
	{
		cStringT str1;
		str1.Format( GAMERESOURCEMAN->GetGameText( 6211 ), time );

		mpTime->SetText( (LPCTSTR)str1.Cstr() );
	}
}
