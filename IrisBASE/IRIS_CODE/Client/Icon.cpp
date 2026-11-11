#include "StdAfx.h"
#include "Icon.h"
#include "Token.h"
#include "Parser.h"
#include "UIContainer.h"
#include "PlaneObject.h"
#include "FontAgent.h"

cIcon::cIcon( eUINodeType type )
: cBaseIcon( type )
, mImageFont( 0 )
, mNumber( 0 )
, mShowGauge( true )
, mShowNumber( true )
, mpEdgeImage(0)
, mpCountImage(0)
, mpFlashImage(0)
, mAniEnable( true )
, mpTipImage(0)
{
	mTipType = eTOOLTIP_NONE;
	mProcessEnable = true;
}

cIcon::~cIcon()
{
	SAFE_DELETE( mpFlashImage );
	SAFE_DELETE( mpTipImage );
	SAFE_DELETE( mpCountImage );
	SAFE_DELETE( mpEdgeImage );
}

/// 툴팁 이미지 지정
void cIcon::SetTipType( unsigned int type )
{
	if( mTipType == (eTooltipType)type )
		return;

	mTipType = (eTooltipType)type;

	if( type == eTOOLTIP_NONE )
		return;

	cIconParam* pTipParam = UIMAN->GetIconParam( 10 );
	if( pTipParam )
	{
		unsigned int w = GetAbsoluteRect().GetWidth();
		unsigned int h = GetAbsoluteRect().GetWidth();
		unsigned int tx = pTipParam->mTexPos.mX + ( type * w );
		unsigned int ty = pTipParam->mTexPos.mY;

		if( mpTipImage )
			mpTipImage->SetTextureRect( tx, ty, tx + w, ty + h );
	}
}

void cIcon::ShowGauge( bool show )
{
	mShowGauge = show;

	if( mShowGauge == true )
		mGauge.SetPos( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
}

void cIcon::ChangeGauge( cUIRoundGauge& gauge )
{
	/// 필요한 속성만 대입 
	mGauge = gauge;
	mShowGauge = true;
}

void cIcon::UpdateGauge( unsigned long startTime, unsigned long elapsedTime, unsigned long maintainTime, bool active )
{
	mGauge.UpdateGauge( startTime, elapsedTime, maintainTime, active );
}

/// 오라스킬 세팅
void cIcon::SetAuraOn( bool set )
{
	mAniParam[eANI_AURA].mAniOn = set;
	mAniParam[eANI_AURA].mLastAccumTime = mAniParam[eANI_AURA].mAccumTime;

	if( set == true )
	{
		/// 텍스쳐 좌표 세팅
		unsigned int w = mAniParam[eANI_AURA].mWidth;
		unsigned int h = mAniParam[eANI_AURA].mHeight;
		unsigned int tx = mAniParam[eANI_AURA].mTexX;
		unsigned int ty = mAniParam[eANI_AURA].mTexY;

		/// 오라 타입
		mAniParam[eANI_AURA].mCurFrame = 0;
	
		if( mpEdgeImage )
			mpEdgeImage->SetTextureRect( tx, ty, tx + w, ty + h );
	}
}

bool cIcon::IsAuraOn()
{
	return mAniParam[eANI_AURA].mAniOn;
}

/// 애니메이션 보기 설정
void cIcon::SetAniOn( eAniType type, short frame )
{	
	/// 텍스쳐 좌표 세팅
	unsigned int w = mAniParam[type].mWidth;
	unsigned int h = mAniParam[type].mHeight;
	unsigned int tx = mAniParam[type].mTexX;
	unsigned int ty = mAniParam[type].mTexY;

	/// 카운트 타입 / 반짝이기 타입
	if( type == eANI_COUNT )	
	{
		mAniParam[eANI_COUNT].mAniOn = true;
		mAniParam[eANI_COUNT].mLastAccumTime = mAniParam[eANI_COUNT].mAccumTime;

		tx = mAniParam[eANI_COUNT].mTexX + ( frame * w );
		mAniParam[eANI_COUNT].mCurFrame = frame;

		if( mpCountImage )
			mpCountImage->SetTextureRect( tx, ty, tx + w, ty + h );
	}
	else if( type == eANI_FLASH )
	{
		mAniParam[eANI_FLASH].mAniOn = true;
		mAniParam[eANI_FLASH].mLastAccumTime = mAniParam[eANI_FLASH].mAccumTime;

		mAniParam[eANI_FLASH].mCurFrame = 0;
		mAniParam[eANI_FLASH].mReturn = false;

		if( mpFlashImage )
			mpFlashImage->SetTextureRect( tx, ty, tx + w, ty + h );
	}
}

void cIcon::SetAniParam( sAniParam* param )
{
	if( param == 0 )
	{
		for( unsigned int i = 0; i < eANI_MAX; ++i )
		{
			mAniParam[i].mAniOn = false;
		}
	}
	else
	{
		for( unsigned int i = 0; i < eANI_MAX; ++i, ++param )
		{
			mAniParam[i].mFrame = (*param).mFrame;
			mAniParam[i].mTexX = (*param).mTexX;
			mAniParam[i].mTexY = (*param).mTexY;
			mAniParam[i].mWidth = (*param).mWidth;
			mAniParam[i].mHeight = (*param).mHeight;
			mAniParam[i].mCurFrame = (*param).mCurFrame;
			mAniParam[i].mAniOn = (*param).mAniOn;
			mAniParam[i].mTimeOut = (*param).mTimeOut;
			mAniParam[i].mAccumTime = (*param).mAccumTime;
			mAniParam[i].mLastAccumTime = (*param).mLastAccumTime;
			mAniParam[i].mReturn = (*param).mReturn;
		}
	}
}

bool cIcon::OnCreate( cUINodeProperty* )
{
	mImageFont = UIMAN->GetImageFont( eIAMGEFONT_ITEMCOUNT );
	
	/// 원형 게이지 생성 및 세팅
	{
		cIconParam* pParam = UIMAN->GetIconParam( 1 );
		if( pParam == 0 )
		{
			assert(0);
			return false;
		}

		/// 생성
		unsigned int x = pParam->mTexPos.mX;
		unsigned int y = pParam->mTexPos.mY;
		unsigned int w = GetAbsoluteRect().GetWidth();
		unsigned int h = GetAbsoluteRect().GetHeight();
		mGauge.Create( pParam->mpTexture, (unsigned short)x, (unsigned short)y, (unsigned short)(x + w), (unsigned short)(y + h));
		mGauge.SetPos( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
	}

	/// 애니메이션 관련 설정
	{
        /// 오라스킬 정보 세팅
		mAniParam[eANI_AURA].mTimeOut = 40;
		mAniParam[eANI_AURA].mLastAccumTime = 0;
		mAniParam[eANI_AURA].mFrame = 15;
		mAniParam[eANI_AURA].mTexX = 3;
		mAniParam[eANI_AURA].mTexY = 1024;
		mAniParam[eANI_AURA].mWidth = 30;
		mAniParam[eANI_AURA].mHeight = 30;
		mAniParam[eANI_AURA].mAniOn = false;
		mAniParam[eANI_AURA].mCurFrame = 0;
		mAniParam[eANI_AURA].mReturn = false;
		
		NiTexture* pTex = UIMAN->GetSkin()->GetTexture( 5002 );
	
		unsigned int edgeTexX = mAniParam[eANI_AURA].mTexX;
		unsigned int edgeTexY = mAniParam[eANI_AURA].mTexY;
		unsigned int edgeX = GetAbsoluteRect().mLeft - 1;
		unsigned int edgeY = GetAbsoluteRect().mTop - 1;
		unsigned int edgeW = mAniParam[eANI_AURA].mWidth;
		unsigned int edgeH = mAniParam[eANI_AURA].mHeight;
		
		mpEdgeImage = new cPlaneObject;
		if( mpEdgeImage->Create( pTex, edgeX, edgeY, edgeW, edgeH, edgeTexX, edgeTexY, edgeTexX + edgeW, edgeTexY + edgeH ) == false )
		{
			assert( 0 && "failed to create mpEdgeImage" );
			return false;
		}


		/// 카운트용 이미지 세팅
		cIconParam* pCountParam = UIMAN->GetIconParam( 19 );
		if( pCountParam )
		{
			mAniParam[eANI_COUNT].mTimeOut = 1;
			mAniParam[eANI_COUNT].mLastAccumTime = 0;
			mAniParam[eANI_COUNT].mFrame = 9;
			mAniParam[eANI_COUNT].mTexX = pCountParam->mTexPos.mX;
			mAniParam[eANI_COUNT].mTexY = pCountParam->mTexPos.mY;
			mAniParam[eANI_COUNT].mWidth = 28;
			mAniParam[eANI_COUNT].mHeight = 28;
			mAniParam[eANI_COUNT].mAniOn = false;
			mAniParam[eANI_COUNT].mCurFrame = 9;
			mAniParam[eANI_COUNT].mReturn = false;
		}

		unsigned int countTexX = mAniParam[eANI_COUNT].mTexX;
		unsigned int countTexY = mAniParam[eANI_COUNT].mTexY;
		unsigned int countX = GetAbsoluteRect().mLeft;
		unsigned int countY = GetAbsoluteRect().mTop;
		unsigned int countW = mAniParam[eANI_COUNT].mWidth;
		unsigned int countH = mAniParam[eANI_COUNT].mHeight;

		mpCountImage = new cPlaneObject;
		if( mpCountImage->Create( pCountParam->mpTexture, countX, countY, countW, countH, 
								  countTexX, countTexY, countTexX + countW, countTexY + countH ) == false )
		{
			assert( 0 && "failed to create mpCountImage" );
			return false;
		}

		/// 반짝이는용 이미지 생성
		NiTexture* flashTex = UIMAN->GetSkin()->GetTexture( 3010 );
		if( flashTex )
		{
			mAniParam[eANI_FLASH].mTimeOut = 5;
			mAniParam[eANI_FLASH].mLastAccumTime = 0;
			mAniParam[eANI_FLASH].mFrame = 32;
			mAniParam[eANI_FLASH].mTexX = 0;
			mAniParam[eANI_FLASH].mTexY = 0;
			mAniParam[eANI_FLASH].mWidth = 28;
			mAniParam[eANI_FLASH].mHeight = 28;
			mAniParam[eANI_FLASH].mAniOn = false;
			mAniParam[eANI_FLASH].mCurFrame = 0;
			mAniParam[eANI_FLASH].mReturn = false;
		}

		unsigned int flashTexX = mAniParam[eANI_FLASH].mTexX;
		unsigned int flashTexY = mAniParam[eANI_FLASH].mTexY;
		unsigned int flashX = GetAbsoluteRect().mLeft;
		unsigned int flashY = GetAbsoluteRect().mTop;
		unsigned int flashW = mAniParam[eANI_FLASH].mWidth;
		unsigned int flashH = mAniParam[eANI_FLASH].mHeight;

		mpFlashImage = new cPlaneObject;
		if( mpFlashImage->Create( flashTex, flashX, flashY, flashW, flashH, 
								  flashTexX, flashTexY, flashTexX + flashW, flashTexY + flashH ) == false )
		{
			assert( 0 && "failed to create mpCountImage" );
			return false;
		}

		NiAlphaProperty* ap = mpFlashImage->GetAlphaProp();
		if( ap )
		{
			ap->SetSrcBlendMode( NiAlphaProperty::ALPHA_ONE );
			ap->SetDestBlendMode( NiAlphaProperty::ALPHA_ONE );
		}

	}

	/// 레어도 관련 이미지 추가
	{
		cIconParam* pTipParam = UIMAN->GetIconParam( 10 );
		if( pTipParam == 0 )
		{
			assert(0);
			return false;
		}

		mpTipImage = new cUIImage( pTipParam->mpTexture );

		unsigned int tx = pTipParam->mTexPos.mX;
		unsigned int ty = pTipParam->mTexPos.mY;
		unsigned int w = GetAbsoluteRect().GetWidth();
		unsigned int h = GetAbsoluteRect().GetWidth();
		mpTipImage->SetTextureRect( tx, ty, tx + w, ty + h );
		mpTipImage->SetScreenRect( mAbsoluteRect );
	}
	return true;
}

void cIcon::UpdateRect()
{
	cUINode::UpdateRect();

	/// 이미지 영역을 갱신
	if( mParam && mpImage )
		mpImage->SetScreenRect( mAbsoluteRect );

	mGauge.SetPos( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
	
	if( mpEdgeImage )
		mpEdgeImage->SetTranslate( (short)GetAbsoluteRect().mLeft - 1, (short)GetAbsoluteRect().mTop - 1 );

	if( mpCountImage )
		mpCountImage->SetTranslate( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );

	if( mpFlashImage )
		mpFlashImage->SetTranslate( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );

	if( mpTipImage )
	{
		unsigned int x = GetAbsoluteRect().mLeft;
		unsigned int y = GetAbsoluteRect().mTop;
		mpTipImage->SetPos( x, y );
	}
}

bool cIcon::SetSkin( const cUINodeSkin* skin )
{
	if( skin->IsKindof( eUINODE_ICON ) == false )
	{
		assert( 0 && "not icon skin type" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( skin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( skin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage(skin->mpTexture);
		}
		else
		{
			assert(0);
		}

		/// 컬러를 지정
		SetColor( skin->mColor );

		/// 텍스처 영역 설정
		unsigned int tx = skin->mSkinInfo->mTexX;
		unsigned int ty = skin->mSkinInfo->mTexY;
		unsigned int tw = skin->mSkinInfo->mTexWidth;
		unsigned int th = skin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return false;
	}

	unsigned int x = skin->mSkinInfo->mX;
	unsigned int y = skin->mSkinInfo->mY;
	unsigned int w = skin->mSkinInfo->mWidth;
	unsigned int h = skin->mSkinInfo->mHeight;

	/// 부모가 루트 타입이면 모두 갱신
	if( mpParent->GetType() == eUINODE_ROOT )
	{
		x = ( GetScreenWidth() * skin->mSkinInfo->mX ) / mDefaultWidth;
		y = ( GetScreenHeight() * skin->mSkinInfo->mY ) / mDefaultHeight;

		/// 오른쪽이나 아랫쪽에 윈도우가 붙어있으면 해상도 변경시에도
		/// 붙어있도록 한다 
		if( (skin->mSkinInfo->mX + skin->mSkinInfo->mWidth) >= (int)mDefaultWidth )
		{
			x = GetScreenWidth() - w;
		}

		if( (skin->mSkinInfo->mY + skin->mSkinInfo->mHeight) >= (int)mDefaultHeight )
		{
			y = GetScreenHeight() - h;
		}
	}

	/// 적용
	SetRelativeRect( cUIRect( x, y, x + w, y + h) );

	/// 
	mHoverTipIndex = skin->mSkinInfo->mHoverTipIndex;
	return true;
}

void cIcon::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mParam == 0 )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 원형 게이지 렌더링
	if( mShowGauge && mGauge.IsActive() )
	{
		mGauge.Draw();
	}

	/// 번호
	if( mShowNumber && mNumText.IsEmpty() == false )
	{
		const cUIRect& rc = GetAbsoluteRect();

		if( mNumber > 999 )
		{
			cStringT str = _T("*");
			int w = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, const_cast<LPTSTR>(str.Cstr()), str.GetLength() );
			int h = FONTAGENT->GetTextHeight( cFontAgent::eFont_UI );
			pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(str.Cstr()), rc.mRight - w - 3, rc.mBottom - h + 1, COLOR_WHITE, mOutLine );
		}
		else
		{
			if( mImageFont )
				mImageFont->DrawText( rc.mRight - mImageFont->GetTextWidth(mNumber) - 1, rc.mBottom - mImageFont->GetTextHeight() - 1, mNumText.Cstr() );
		}
	}

	if( mAniEnable )
	{
		if( mpCountImage && mAniParam[eANI_COUNT].mAniOn == true )
			mpCountImage->Draw();

		if( mpFlashImage && mAniParam[eANI_FLASH].mAniOn == true )
			mpFlashImage->Draw();

		if( mpEdgeImage && mAniParam[eANI_AURA].mAniOn == true )
			mpEdgeImage->Draw();
	}

	/// 레어도 관련 이미지
	if( mTipType != eTOOLTIP_NONE && mpTipImage )
		mpTipImage->Draw();
}

void cIcon::OnProcess( unsigned long /*deltaTime*/, unsigned long accumTime )
{
	if( mAniEnable )
	{
		/// 타이머
		for( unsigned int i = 0; i <= eANI_FLASH; ++i )
		{
			/// 타이머
			mAniParam[i].mAccumTime = accumTime;

			unsigned long elasedTime = mAniParam[i].mAccumTime - mAniParam[i].mLastAccumTime;
			if( mAniParam[i].mAniOn && elasedTime > mAniParam[i].mTimeOut )
			{
				/// 타입에 따른 호출
				OnTimer(i);
				mAniParam[i].mLastAccumTime = mAniParam[i].mAccumTime;
			}
		}
	}

	if( mGauge.IsActive() )
	{
		unsigned int state = mGauge.Update( accumTime );
		if( state == cUIRoundGauge::eGaugeState_End )
		{
			/// 끝나는 순간에 아이콘 반짝이는 이미지 필요
			mAniParam[eANI_COUNT].mAniOn = false;
			SetAniOn( eANI_FLASH );
		}
		else if( mGauge.GetRestTime() > 0 && mGauge.GetRestTime() < 9000 )
		{
			/// 쿨타임 카운트 이미지
			short frame = (short)mGauge.GetRestTime()/1000;
			SetAniOn( eANI_COUNT, frame );
		}
	}
}

/// 애니메이션 돌리기 
void cIcon::OnTimer( unsigned int type )
{
	if( type == eANI_AURA )
	{
		/// 오라 애니메이션 프레임 돌리기 
		if( mAniParam[eANI_AURA].mAniOn == true  )
		{
			mAniParam[eANI_AURA].mCurFrame++;

			/// 프레임을 넘어가면 초기화
			if( mAniParam[eANI_AURA].mCurFrame >= mAniParam[eANI_AURA].mFrame )
				mAniParam[eANI_AURA].mCurFrame = 0;

			/// 텍스쳐 좌표 세팅
			unsigned int w = mAniParam[eANI_AURA].mWidth;
			unsigned int h = mAniParam[eANI_AURA].mHeight;
			unsigned int tx = mAniParam[eANI_AURA].mTexX + ( mAniParam[eANI_AURA].mCurFrame * w );
			unsigned int ty = mAniParam[eANI_AURA].mTexY;

			if( mpEdgeImage )
				mpEdgeImage->SetTextureRect( tx, ty, tx + w, ty + h );
		}

	}
	else if( type == eANI_FLASH )
	{
		/// 반짝이는 이미지 프레임 돌리기 
		if( mAniParam[eANI_FLASH].mAniOn == true )
		{
			mAniParam[eANI_FLASH].mCurFrame++;

			/// max 넘어가면 끄기 
			if( mAniParam[eANI_FLASH].mCurFrame >= mAniParam[eANI_FLASH].mFrame )
				mAniParam[eANI_FLASH].mAniOn = false;

			int line = (int)(mAniParam[eANI_FLASH].mCurFrame / 9);
			int num = (int)(mAniParam[eANI_FLASH].mCurFrame % 9);
			
			unsigned int w = mAniParam[eANI_FLASH].mWidth;
			unsigned int h = mAniParam[eANI_FLASH].mHeight;
			unsigned int tx = mAniParam[eANI_FLASH].mTexX + ( num * w );
			unsigned int ty = mAniParam[eANI_FLASH].mTexY + ( line * h );
			
			if( mpFlashImage )
				mpFlashImage->SetTextureRect( tx, ty, tx + w, ty + h );
		}
	}
}

void cIcon::StartDelay( unsigned long delayTime )
{
	mGauge.Activate( delayTime );
}

void cIcon::StopDelay()
{
	mGauge.DeActivate();

	SetAniParam(0);
}

void cIcon::SetElapsedTime( unsigned long delta )
{
	mGauge.SetElapsedTime( delta );
}

bool cIcon::IsActive()
{
	return mGauge.IsActive();
}

void cIcon::ClearCooltime()
{
	if( IsAuraOn() == false )
		StopDelay( );	
	SetAniOn( eANI_FLASH );
}
