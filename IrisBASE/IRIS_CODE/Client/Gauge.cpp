#include "StdAfx.h"
#include "Gauge.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"
#include "FontAgent.h"
#include "PlaneObject.h"
#include "RenderSystem.h"

cGauge::cGauge( eUINodeType type )
: cUINode( type )
, mpGaugeImage(0)
, mValue(0)
, mDestValue(0)
, mMaxValue(0)
, mPercent(0)
, mSpeed(0)
, mState( eGaugeState_None )
, mStartTime(0)
, mShowPercent( false )
, mShowText( true )
, mTextX(0)
, mTextY(0)
, mpImageTexture(0)
, mImagePos(0, 0)
, mImageSize(0, 0)
, mImageTexpos(0, 0)
, mGaugeSize(0, 0)
, mImageStart( true )
, mRightDecrease( false )
{
	mProcessEnable = true;

	mUseRenderTarget = false;

	mpRenderTargetGroup = 0;
	mpRenderedTexture = 0;
	mpGaugeTarget = 0;

	mOutLine = true;
}

cGauge::~cGauge()
{
	SAFE_DELETE( mpGaugeTarget );
	SAFE_DELETE( mpGaugeImage );

	mpRenderTargetGroup = 0;
	mpRenderedTexture = 0;
}

void cGauge::Clear()
{
	mMaxValue = 0;
	mValue = 0;
	mDestValue = 0;
	mPercent = 0;
	mText.Clear();

	///
	UpdateGauge();
}

void cGauge::ClearFront()
{
	mValue = 0;
	mDestValue = 0;
	mState = eGaugeState_End;
	mText.Clear();

	///
	UpdateGauge();
}

bool cGauge::OnCreate( cUINodeProperty* pproperty )
{
	if( cUINode::OnCreate( pproperty ) == false )
		return false;

	if( mUseRenderTarget == true )
	{
		if( mpImage )
		{
			//////////////////////////////////////////////////////////////////////////
			cRenderer* r = RENDERSYS->GetRenderer();
			if( r == 0 )
				return false;

			NiTexture::FormatPrefs prefs;
			prefs.m_ePixelLayout = NiTexture::FormatPrefs::TRUE_COLOR_32;
			prefs.m_eAlphaFmt = NiTexture::FormatPrefs::BINARY;
			prefs.m_eMipMapped = NiTexture::FormatPrefs::NO;

			unsigned short width = (unsigned short)mpImage->GetWidth();
			unsigned short height = (unsigned short)mpImage->GetHeight();

			mpRenderedTexture = NiRenderedTexture::Create( width, height, r, prefs );
			if( mpRenderedTexture == 0 )
				return false;

			mpRenderTargetGroup = NiRenderTargetGroup::Create( mpRenderedTexture->GetBuffer(), r, true, false );

			mpGaugeTarget = new cPlaneObject;
			mpGaugeTarget->Create( mpRenderedTexture, 0, 0, width, height, 0, 0, width, height );
			//////////////////////////////////////////////////////////////////////////

			mpImage->SetRenderTargetWH( width, height );
		}
	}

	if( mpImageTexture )
	{
		mpGaugeImage = new cPlaneObject;
		if( mpGaugeImage->Create( mpImageTexture, 0, 0, mImageSize.mWidth, mImageSize.mHeight, 
			mImageTexpos.mX, mImageTexpos.mY, 
			mImageTexpos.mX + mImageSize.mWidth, 
			mImageTexpos.mY + mImageSize.mHeight ) == false )
		{
			assert( 0 && "failed to create gauge image");
			return false;
		}

		int x = GetAbsoluteRect().mLeft + mImagePos.mX;
		int y = GetAbsoluteRect().mTop + mImagePos.mY;

		if( mpGaugeTarget )
		{
			mpGaugeTarget->SetScreenXY( x, y );
			mpGaugeImage->SetRenderTargetWH( mpGaugeTarget->GetScreenW(), mpGaugeTarget->GetScreenH() );
		}
		else
		{
			mpGaugeImage->SetScreenXY( x, y );
		}
	}

	return true;
}

void cGauge::ChangeTexturePos( const cUIPos& texPos )
{
	if( !mpGaugeImage )
	{
		assert(0);
		return;
	}

	mImageTexpos = texPos;

	mpGaugeImage->SetTextureRect(	mImageTexpos.mX, 
									mImageTexpos.mY, 
									mImageTexpos.mX + mImageSize.mWidth, 
									mImageTexpos.mY + mImageSize.mHeight );
}

bool cGauge::SetSkin( const cUINodeSkin* pskin )
{
	if( !pskin )
	{
		assert(0);
		return false;
	}

	if( pskin->IsKindof( eUINODE_GAUGE ) == false )
	{
		assert( 0 && "not gauge skin type" );
		return false;
	}
	
	if( cUINode::SetSkin( pskin ) == false )
		return false;

	//cUISkin* pUISkin = UIMAN->GetSkin();
	//if( !pUISkin )
	//{
	//	assert(0);
	//	return false;
	//}

	cGaugeSkin* p = (cGaugeSkin*)pskin;
	mSpeed = p->mGaugeSpeed;
	mShowPercent = p->mShowPercent;
	mShowText = p->mShowText;

	mpImageTexture = p->mpImageTexture;
	mImagePos = p->mImagePos;
	mImageSize = p->mImageSize;
	mImageTexpos = p->mImageTexpos;
	mGaugeSize = p->mGaugeSize;
	mImageStart = p->mImageStart;

	UpdateText();
	return true;
}

void cGauge::UpdateRect()
{
	cUINode::UpdateRect();

	if( mpGaugeTarget )
	{
		if( mpImage )
			mpImage->SetPos( 0, 0 );

		mpGaugeTarget->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );

		if( mpGaugeImage )
			mpGaugeImage->SetScreenXY( (short)mImagePos.mX, (short)mImagePos.mY );

		if( mRightDecrease )
		{
			int x = mImagePos.mX;
			x += (mGaugeSize.mWidth - mpGaugeImage->GetScreenW());

			if( mpGaugeImage )
				mpGaugeImage->SetScreenX( x );
		}
	}
	else
	{
		if( mpGaugeImage )
			mpGaugeImage->SetScreenXY( (short)GetAbsoluteRect().mLeft + mImagePos.mX, (short)GetAbsoluteRect().mTop + mImagePos.mY );

		if( mRightDecrease )
		{
			int x = GetAbsoluteRect().mLeft + mImagePos.mX;
			x += (mGaugeSize.mWidth - mpGaugeImage->GetScreenW());

			if( mpGaugeImage )
				mpGaugeImage->SetScreenX( x );
		}
	}

	///
	UpdateText();
}

void cGauge::UpdateSkin()
{
	cUINode::UpdateSkin();

	if( mpGaugeTarget )
		mpGaugeTarget->UpdateImage();

	if( mpGaugeImage )
		mpGaugeImage->UpdateImage();
}

///
void cGauge::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mState == eGaugeState_End || mState == eGaugeState_None )
		return;

	if( mState == eGaugeState_Start )
		mStartTime = accumTime;

	if( mValue == mDestValue )
	{
		mStartTime = 0;
		mState = eGaugeState_End;
	}

	float elapsedTime = (float)(accumTime - mStartTime) * 0.001f;
	mStartTime = accumTime;

	/// 실제 게이지 값이 증가치까지 못갔으면, 일정량씩 더해줌.
	if( mValue < mDestValue )
	{
		mValue += (elapsedTime * mSpeed);
		mState = eGaugeState_Process;

		/// 초과했으면 보정해준다 
		if( mValue >= mDestValue )
		{
			mStartTime = 0;
			mValue = mDestValue;
			mState = eGaugeState_End;

			cUIEvent event;
			event.mpCaller = this;
			event.mType = eUIEVENT_GAUGE_END;
			mpParent->HandleEvent( event );
		}
	}
	/// 증가치가 더 적을경우에는 감소시킨다 
	else if( mValue > mDestValue )
	{
		mValue = mDestValue;
		mState = eGaugeState_End;
		mStartTime = 0;
	}

	/// 현재게이지가 갱신되었으면 실제 사이즈에 적용시킨다
	UpdateGauge();

	if( mRightDecrease )
	{
		if( mpGaugeTarget )
		{
			int x = mImagePos.mX;
			x += (mGaugeSize.mWidth - mpGaugeImage->GetScreenW());

			if( mpGaugeImage )
				mpGaugeImage->SetScreenX( x );
		}
		else
		{
			int x = GetAbsoluteRect().mLeft + mImagePos.mX;
			x += (mGaugeSize.mWidth - mpGaugeImage->GetScreenW());

			if( mpGaugeImage )
				mpGaugeImage->SetScreenX( x );
		}
	}
}

void cGauge::OnPreRender()
{
	if( mpGaugeTarget )
	{
		cRenderer* r = RENDERSYS->GetRenderer();
		if( r == 0 )
			return;

		if( r->BeginUsingRenderTargetGroup( mpRenderTargetGroup, NiRenderer::CLEAR_ALL ) == true )
		{
			r->SetScreenSpaceCameraData();

			if( mpGaugeImage )
				mpGaugeImage->Draw();
			if( mpImage )
				mpImage->Draw();

			r->EndUsingRenderTargetGroup();
		}
	}
}

void cGauge::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mpGaugeTarget )
	{
		mpGaugeTarget->Draw();
	}
	else
	{
		if( mpImage )
			mpImage->Draw();

		/// 이미지 렌더링
		if( mpGaugeImage )
			mpGaugeImage->Draw();
	}

	/// 글씨 출력
	if( mShowText )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mText.Cstr()), mTextX, mTextY, COLOR_WHITE, mOutLine );
}

/// 목표값을 정해서 애니메이션 시킨다
void cGauge::SetGaugeAni( double value )
{
	if( mDestValue == value )
		return;

	if( value > mMaxValue )
		value = mMaxValue;
	else if( value < 0 )
		value = 0;

	mDestValue = value;

	/// 이미 프로세스중이면 그냥 이어서 한다 
	if( mState != eGaugeState_Process)
		mState = eGaugeState_Start;

	UpdateDestGauge();
}

/// 게이지 애니메이션 없이 세팅
void cGauge::SetGaugeNoAni( double value )
{
	if( mValue == value )
		return;

	if( value > mMaxValue )
		value = mMaxValue;
	else if( value < 0 )
		value = 0;

	mValue = value;
	mDestValue = value;

	/// 게이지에 적용
	UpdateGauge();
}

///
void cGauge::SetMaxValue( double value )
{
	if( value <= 0 )
		return;

	if( value != mMaxValue )
	{
		mMaxValue = value;

		if( mValue > mMaxValue )
			mValue = mMaxValue;

		if( mDestValue > mMaxValue )
			mDestValue = mMaxValue;

		UpdateGauge();
	}
}

void cGauge::UpdateGauge()
{
	if( mMaxValue > 0 )
	{
		mPercent = (float)(mValue/mMaxValue) * 100.0f;

		int maxSize = ( (int)GetScreenWidth() < mGaugeSize.mWidth ) ? GetScreenWidth() : mGaugeSize.mWidth;

		/// 사이즈 적용
		unsigned short w = (unsigned short)( maxSize * mPercent * 0.01f );

		if( mpGaugeImage )
			mpGaugeImage->SetScreenWH( w, mGaugeSize.mHeight );

		/// 정보 갱신
		UpdateText();
	}
}

/// 주의사항 : 게이지 위에 뿌려지는 수치값 업데이트 
void cGauge::UpdateText()
{
	if( mShowText )
	{
		/// 수치 그대로 출력하려한다면,
		if( mShowPercent == false )
			mText.Format( _T("%lu / %lu"), (unsigned long)mValue, (unsigned long)mMaxValue );
		/// 퍼센트 를 붙여서 출력하려 한다면,
		else
			mText.Format( _T("%.2f%%"), (int)(mPercent * 100) / 100.f );

		/// 위치 계산
		cUIRect rc = GetAbsoluteRect();
		mTextX = rc.mLeft + ( rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_System, mText.Cstr(), mText.GetLength() )) / 2;
		mTextY = rc.mTop + ( rc.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_System)) / 2 + 1;
	}
}

bool cGauge::HandleEvent( const cUIEvent& event )
{
	return mpParent->HandleEvent( event );
}

/////////////////////////////////////////////////////////////////////////
/// 이중게이지
cDoubleGauge::cDoubleGauge( eUINodeType type )
: cGauge( type )
, mpDestGaugeImage(0)
{

}

cDoubleGauge::~cDoubleGauge()
{
	SAFE_DELETE( mpDestGaugeImage );
	SAFE_DELETE( mpGaugeImage );
}

void cDoubleGauge::Clear()
{
	cGauge::Clear();

	///
	UpdateDestGauge();
}

void cDoubleGauge::ClearFront()
{
	cGauge::ClearFront();

	///
	UpdateDestGauge();
}

bool cDoubleGauge::OnCreate( cUINodeProperty* pproperty )
{
	if( cUINode::OnCreate( pproperty ) == false )
		return false;

	if( mpImageTexture )
	{
		int x = GetAbsoluteRect().mLeft + mImagePos.mX;
		int y = GetAbsoluteRect().mTop + mImagePos.mY;

		mpGaugeImage = new cPlaneObject;
		if( mpGaugeImage->Create( mpImageTexture, x, y, mImageSize.mWidth, mImageSize.mHeight, 
									mImageTexpos.mX, mImageTexpos.mY, 
									mImageTexpos.mX + mImageSize.mWidth, 
									mImageTexpos.mY + mImageSize.mHeight ) == false )
		{
			assert( 0 && "failed to create gauge image");
			return false;
		}
	}

	if( mpDestImageTexture )
	{
		int x = GetAbsoluteRect().mLeft + mDestImagePos.mX;
		int y = GetAbsoluteRect().mTop + mDestImagePos.mY;

		mpDestGaugeImage = new cPlaneObject;
		if( mpDestGaugeImage->Create( mpDestImageTexture, x, y, mDestImageSize.mWidth, mDestImageSize.mHeight, 
										mDestImageTexpos.mX, mDestImageTexpos.mY, 
										mDestImageTexpos.mX + mDestImageSize.mWidth, 
										mDestImageTexpos.mY + mDestImageSize.mHeight ) == false )
		{
			assert( 0 && "failed to create gauge image");
			return false;
		}
	}

	return true;
}

bool cDoubleGauge::SetSkin( const cUINodeSkin* pskin )
{
	if( !pskin )
	{
		assert(0);
		return false;
	}

	if( pskin->IsKindof( eUINODE_DOUBLEGAUGE ) == false )
	{
		assert( 0 && "not gaugebar skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	//cUISkin* pUISkin = UIMAN->GetSkin();
	//if( !pskin )
	//{
	//	assert(0);
	//	return false;
	//}

	cGaugeSkin* p = (cGaugeSkin*)pskin;
	mSpeed = p->mGaugeSpeed;
	mShowPercent = p->mShowPercent;
	mShowText = p->mShowText;

	mpImageTexture	= p->mpImageTexture;
	mImagePos		= p->mImagePos;
	mImageSize		= p->mImageSize;
	mImageTexpos	= p->mImageTexpos;
	mGaugeSize		= p->mGaugeSize;
	mImageStart		= p->mImageStart;

	mpDestImageTexture	= p->mpDestImageTexture;
	mDestImagePos		= p->mDestImagePos;
	mDestImageSize		= p->mDestImageSize;
	mDestImageTexpos	= p->mDestImageTexpos;
	mDestGaugeSize		= p->mDestGaugeSize;
	mDestImageStart		= p->mDestImageStart;

	UpdateText();
	return true;
}

void cDoubleGauge::UpdateRect()
{
	cGauge::UpdateRect();

	if( mpDestGaugeImage )
	{
		mpDestGaugeImage->SetScreenXY( (short)GetAbsoluteRect().mLeft + mDestImagePos.mX, 
									   (short)GetAbsoluteRect().mTop + mDestImagePos.mY );
	}

	if( mRightDecrease )
	{
		int x = GetAbsoluteRect().mLeft + mImagePos.mX;
		x += (mGaugeSize.mWidth - mpDestGaugeImage->GetScreenW());

		if( mpDestGaugeImage )
			mpDestGaugeImage->SetScreenX( x );
	}
}

void cDoubleGauge::UpdateSkin()
{
	cGauge::UpdateSkin();
	
	if( mpDestGaugeImage )
		mpDestGaugeImage->UpdateImage();
}

void cDoubleGauge::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mRightDecrease )
	{
		int x = GetAbsoluteRect().mLeft + mImagePos.mX;
		x += (mGaugeSize.mWidth - mpDestGaugeImage->GetScreenW());

		if( mpDestGaugeImage )
			mpDestGaugeImage->SetScreenX( x );
	}

	cGauge::OnProcess( deltaTime, accumTime );
}

void cDoubleGauge::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mpDestGaugeImage )
		mpDestGaugeImage->Draw();

	cGauge::OnRender( pKeeper );
}

/// 이중애니메이션 없이 동시 세팅
void cDoubleGauge::SetGaugeNoAni( float value )
{
	cGauge::SetGaugeNoAni( value );
	UpdateDestGauge();
}

void cDoubleGauge::SetMaxValue( float value )	
{	
	cGauge::SetMaxValue( value );

	/// 목표치 게이지 갱신
	UpdateDestGauge();
}

void cDoubleGauge::UpdateDestGauge()
{
	if( mMaxValue > 0 )
	{
		float percent = (float)(mDestValue/mMaxValue) * 100.0f;//(mDestValue * 100) / mMaxValue;
		unsigned short w = (unsigned short)( mDestGaugeSize.mWidth * percent * 0.01f );

 		if( mpDestGaugeImage )
		{
			mpDestGaugeImage->SetScreenWH( w, mDestGaugeSize.mHeight );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
cGaugeTB::cGaugeTB( eUINodeType type )
: cGauge( type )
{
	mOutLine = true;
}

cGaugeTB::~cGaugeTB()
{

}

bool cGaugeTB::OnCreate( cUINodeProperty* pproperty )
{
	if( cUINode::OnCreate( pproperty ) == false )
		return false;

	if( mpImageTexture )
	{
		int x = GetAbsoluteRect().mLeft + mImagePos.mX;
		int y = GetAbsoluteRect().mBottom - mImagePos.mY;

		mpGaugeImage = new cPlaneObject;
		if( mpGaugeImage->Create( mpImageTexture, x, y, mImageSize.mWidth, mImageSize.mHeight, 
			mImageTexpos.mX, mImageTexpos.mY, 
			mImageTexpos.mX + mImageSize.mWidth, 
			mImageTexpos.mY + mImageSize.mHeight ) == false )
		{
			assert( 0 && "failed to create gauge image");
			return false;
		}
	}

	return true;
}

bool cGaugeTB::SetSkin( const cUINodeSkin* pskin )
{
	if( !pskin )
	{
		assert(0);
		return false;
	}

	if( pskin->IsKindof( eUINODE_GAUGETB ) == false )
	{
		assert( 0 && "not gauge skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	//cUISkin* pUISkin = UIMAN->GetSkin();
	//if( !pskin )
	//{
	//	assert(0);
	//	return false;
	//}

	cGaugeSkin* p = (cGaugeSkin*)pskin;
	mSpeed = p->mGaugeSpeed;
	mShowPercent = p->mShowPercent;
	mShowText = p->mShowText;

	mpImageTexture = p->mpImageTexture;
	mImagePos = p->mImagePos;
	mImageSize = p->mImageSize;
	mImageTexpos = p->mImageTexpos;
	mGaugeSize = p->mGaugeSize;
	mImageStart = p->mImageStart;

	return true;
}

void cGaugeTB::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mState == eGaugeState_End || mState == eGaugeState_None )
		return;

	if( mState == eGaugeState_Start )
		mStartTime = accumTime;

	if( mValue == mDestValue )
	{
		mStartTime = 0;
		mState = eGaugeState_End;
	}

	float elapsedTime = (float)(accumTime - mStartTime) * 0.001f;
	mStartTime = accumTime;

	/// 실제 게이지 값이 증가치까지 못갔으면, 일정량씩 더해줌.
	if( mValue < mDestValue )
	{
		mValue += (elapsedTime * mSpeed);
		mState = eGaugeState_Process;

		/// 초과했으면 보정해준다 
		if( mValue >= mDestValue )
		{
			mStartTime = 0;
			mValue = mDestValue;
			mState = eGaugeState_End;

			cUIEvent event;
			event.mpCaller = this;
			event.mType = eUIEVENT_GAUGE_END;
			mpParent->HandleEvent( event );
		}
	}
	/// 증가치가 더 적을경우에는 감소시킨다 
	else if( mValue > mDestValue )
	{
		mValue = mDestValue;
		mState = eGaugeState_End;
		mStartTime = 0;
	}

	/// 현재게이지가 갱신되었으면 실제 사이즈에 적용시킨다
	UpdateGauge();

	///
	if( mpGaugeImage )
	{
		int y = GetAbsoluteRect().mBottom - mImagePos.mY;
		y -= mpGaugeImage->GetScreenH();

		mpGaugeImage->SetScreenY( y );
	}
}

void cGaugeTB::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mpGaugeImage )
		mpGaugeImage->Draw();

	if( mpImage )
		mpImage->Draw();

	/// 글씨 출력
	if( mShowText )
		pKeeper->AddFontItem( cFontAgent::eFont_System, const_cast<LPTSTR>(mText.Cstr()), mTextX, mTextY, mDefaultColor, mOutLine );
}


void cGaugeTB::UpdateRect()
{
	cUINode::UpdateRect();

	if( mpGaugeImage )
	{
		mpGaugeImage->SetScreenXY( (short)GetAbsoluteRect().mLeft + mImagePos.mX, (short)GetAbsoluteRect().mBottom );

		///
		int y = GetAbsoluteRect().mBottom - mImagePos.mY;
		y -= mpGaugeImage->GetScreenH();

		mpGaugeImage->SetScreenY( y );
	}

	///
	UpdateText();
}

void cGaugeTB::UpdateGauge()
{
	if( mMaxValue > 0 )
	{
		mPercent = (float)(mValue/mMaxValue) * 100.0f;

		/// 사이즈 적용
		unsigned short h = (unsigned short)( mGaugeSize.mHeight * mPercent * 0.01f );
		if( mpGaugeImage )
			mpGaugeImage->SetScreenWH( mGaugeSize.mWidth, h );

		/// 정보 갱신
		UpdateRect();
	}
}

/////////////////////////////////////////////////////////////////////////////
cGaugeSkin::cGaugeSkin( eUINodeType type )
: cUINodeSkin( type )
, mGaugeSpeed( 0 )
, mShowPercent( false )
, mShowText( true )
, mpImageTexture(0)
, mImagePos(0, 0)
, mImageSize(0, 0)
, mImageTexpos(0, 0)
, mGaugeSize(0, 0)
, mImageStart( true )
, mpDestImageTexture(0)
, mDestImagePos(0, 0)
, mDestImageSize(0, 0)
, mDestImageTexpos(0, 0)
, mDestGaugeSize(0, 0)
, mDestImageStart( true )
{	
}

cGaugeSkin::~cGaugeSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cGaugeSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	/// 그다음토큰을 계속 검사
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_GAUGEIMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpImageTexture = UIMAN->GetSkin()->GetTexture( i );
			}
			break;
		case eTOKEN_GAUGEPOS:
			{
				mImagePos.mX = (unsigned short)parser.ParseInt();
				mImagePos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESIZE:
			{
				mImageSize.mWidth = (unsigned short)parser.ParseInt();
				mImageSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGETEXPOS:
			{
				mImageTexpos.mX = (unsigned short)parser.ParseInt();
				mImageTexpos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEWIDTH:
			{
				mGaugeSize.mWidth = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEHEIGTH:
			{
				mGaugeSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESTART:
			{
				///
				lexer->GetNextToken( &token );

				if( token.mType == eALIGN_LEFT )
				{
					mImageStart = true;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mImageStart = false;
				}
			}
			break;
		case eTOKEN_GAUGEIMAGEINDEX_DEST:
			{
				int i = parser.ParseInt();
				mpDestImageTexture = UIMAN->GetSkin()->GetTexture( i );
			}
			break;
		case eTOKEN_GAUGEPOS_DEST:
			{
				mDestImagePos.mX = (unsigned short)parser.ParseInt();
				mDestImagePos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESIZE_DEST:
			{
				mDestImageSize.mWidth = (unsigned short)parser.ParseInt();
				mDestImageSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGETEXPOS_DEST:
			{
				mDestImageTexpos.mX = (unsigned short)parser.ParseInt();
				mDestImageTexpos.mY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEWIDTH_DEST:
			{
				mDestGaugeSize.mWidth = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGEHEIGTH_DEST:
			{
				mDestGaugeSize.mHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_GAUGESTART_DEST:
			{
				///
				lexer->GetNextToken( &token );

				if( token.mType == eALIGN_LEFT )
				{
					mDestImageStart = true;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mDestImageStart = false;
				}
			}
			break;
		case eTOKEN_GAUGE_SPEED:
			{
				mGaugeSpeed = parser.ParseFloat();
			}
			break;
		case eTOKEN_SHOW_LABEL_PERCENT:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mShowPercent = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mShowPercent = false;
				}
				else
				{
					assert( 0 && "invalid token" );
					return false;
				}
			}
			break;
		case eTOKEN_SHOW_TEXT:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mShowText = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mShowText = false;
				}
				else
				{
					assert( 0 && "invalid token" );
					return false;
				}
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}
	}
	return true;
}

