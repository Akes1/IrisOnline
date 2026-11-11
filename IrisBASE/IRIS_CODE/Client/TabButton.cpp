#include "StdAfx.h"
#include "TabButton.h"

#include "UIContainer.h"
#include "FontAgent.h"
#include "PlaneObject.h"
#include "SoundSystem.h"

cTabButton::cTabButton( eUINodeType type )
: cPushButton( type )
, mpTabButtonImage(0)
, mScreenX(0)
, mScreenY(0)
, mUpSize(0, 0)
, mDownSize(0, 0)
{
	mPressed = false;
	mHovered = false;

	mUpColor = mDefaultColor;
	mDownColor = mDefaultColor;

	mProcessEnable = false;
}

cTabButton::~cTabButton()
{
	SAFE_DELETE( mpTabButtonImage );
}

bool cTabButton::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_TABBUTTON ) == false )
	{
		assert( 0 && "not button skin type" );
		return false;
	}

	/// 이미지 설정
	if( pskin->mpTexture )
	{
		mScreenX = (short)pskin->mSkinInfo->mX;
		mScreenY = (short)pskin->mSkinInfo->mY;
		
		/// 텍스처 영역 설정
		mUpPos.mX = pskin->mSkinInfo->mTexX;
		mUpPos.mY = pskin->mSkinInfo->mTexY;
		mOriginalTexWidth = pskin->mSkinInfo->mTexWidth;
		mOriginalTexHeight = pskin->mSkinInfo->mTexHeight;

		if( mpTabButtonImage == 0 )
		{
			mpTabButtonImage = new cPlaneObject;

			if( mpTabButtonImage->Create( pskin->mpTexture, mScreenX, mScreenY, (unsigned short)mOriginalTexWidth, (unsigned short)mOriginalTexHeight, 
				(unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
				(unsigned short)mUpPos.mX + (unsigned short)mOriginalTexWidth, 
				(unsigned short)mUpPos.mY + (unsigned short)mOriginalTexHeight ) == false )
			{
				assert( 0 && "failed to create gauge image");
				return false;
			}
		}
		else
		{
			mpTabButtonImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
											  (unsigned short)mUpPos.mX + (unsigned short)mOriginalTexWidth, 
											  (unsigned short)mUpPos.mY + (unsigned short)mOriginalTexHeight );
		}
	}

	cUINode::SetRelativeRect( cUIRect( mScreenX, mScreenY, mScreenX + mOriginalTexWidth, mScreenY + mOriginalTexHeight) );

	cButtonSkin* p = (cButtonSkin*)pskin;
	if( p->mText.IsEmpty() == false )
		mText = p->mText;

	/// 버튼의 이미지 변환좌표 저장 
	mUpPos = p->mUpPos;
	mDownPos = p->mDownPos;
	if( p->mUpOverPos.mX == 0 && p->mUpOverPos.mY == 0 )
		mUpOverPos = p->mUpPos;
	else
		mUpOverPos = p->mUpOverPos;

	/// 버튼 상태 텍스트 컬러
	mUpColor = p->mUpColor;
	mDownColor = p->mDownColor;

	/// 탭버튼은 이미지에 따른 사이즈자체가 다르므로 따로저장.
 	mUpSize = p->mUpSize;
	mDownSize = p->mDownSize;
	mDownSoundIdx = p->mDownSoundIdx;

	mHoverTipIndex = p->mSkinInfo->mHoverTipIndex;

	if( mUpSize.mWidth == 0 || mUpSize.mHeight == 0 )
	{
		mUpSize.mWidth = mOriginalTexWidth;
		mUpSize.mHeight = mOriginalTexHeight;
	}

	if( mDownSize.mWidth == 0 || mDownSize.mHeight == 0 )
	{
		mDownSize.mWidth = mOriginalTexWidth;
		mDownSize.mHeight = mOriginalTexHeight;
	} 

	/// 텍스트 좌표 업데이트 
	UpdateText();

	///
	SetPress( false );

	return true;
}

void cTabButton::UpdateRect()
{
	cUINode::UpdateRect();

	if( mpTabButtonImage )
	{
		mpTabButtonImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
	}

	UpdateText();
}

///
void cTabButton::UpdateSkin()
{
	/// 부모에 대한 상대적인 위치이므로, 변경하지 않는다. 
}


/// 주의사항 : 
void cTabButton::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpTabButtonImage )
		mpTabButtonImage->Draw();
	
	/// 텍스트 렌더링
	pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY, mUpColor, mOutLine );
}


/// 주의사항 : 
void cTabButton::OnMouseHovered( const cUIPos& pos )
{
	cUINode::OnMouseHovered( pos );


	/// 라디오 버튼이 안 눌려 있을때만 업 오버 이미지로 갱신 
	if( mPressed == false )
	{
		mpTabButtonImage->SetTextureRect( (unsigned short)mUpOverPos.mX, (unsigned short)mUpOverPos.mY, 
										  (unsigned short)(mUpOverPos.mX + mUpSize.mWidth ), (unsigned short)(mUpOverPos.mY + mUpSize.mHeight ) );
	}
}

/// 주의사항 : 호버 노드 이벤트 받음 
void cTabButton::OnMouseLeft( const cUIPos& pos )
{
	cUINode::OnMouseLeft( pos );

	mHovered = false;

	if( mPressed )
	{
		mpTabButtonImage->SetTextureRect( (unsigned short)mDownPos.mX, (unsigned short)mDownPos.mY, 
						(unsigned short)(mDownPos.mX + mDownSize.mWidth), (unsigned short)(mDownPos.mY + mDownSize.mHeight) );
	}
	else
	{
		mpTabButtonImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
						(unsigned short)(mUpPos.mX + mUpSize.mWidth), (unsigned short)(mUpPos.mY + mUpSize.mHeight) );
	}
}

/// 주의사항 : 
void cTabButton::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );
	
	SOUNDSYS->Play2DSound( mDownSoundIdx );
	SetPress( true );

	cUIEvent event;
	event.mpCaller = this;
	event.mID = mID;
	event.mType = eUIEVENT_TABBUTTON_PRESSED;
	mpParent->HandleEvent( event );
}


/// 주의사항 :
void cTabButton::SetPress( bool press )
{
	cUINode::SetPress( press );

	if( mPressed == true )
	{
		/// 라디오 버튼이 눌렸으면 이미지를 갱신한다
		mpTabButtonImage->SetScreenWH( (unsigned short)mDownSize.mWidth, (unsigned short)mDownSize.mHeight );
		mpTabButtonImage->SetTextureRect( (unsigned short)(mDownPos.mX), (unsigned short)(mDownPos.mY), 
										 (unsigned short)(mDownPos.mX + mDownSize.mWidth ), (unsigned short)(mDownPos.mY + mDownSize.mHeight ) );

		/// 사이즈 변경시 위치 이동
		if( mDownSize.mHeight - mUpSize.mHeight > 0 )
		{
			cUIPos pos( mScreenX, mScreenY );
			cUINode::SetRelativePos( pos );

			mpTabButtonImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
		}
	}
	else if( mPressed == false )
	{
		/// 라디오 버튼이 안 눌린 이미지를 갱신한다 
		mpTabButtonImage->SetScreenWH( (unsigned short)mUpSize.mWidth, (unsigned short)mUpSize.mHeight );
		mpTabButtonImage->SetTextureRect( (unsigned short)(mUpPos.mX), (unsigned short)(mUpPos.mY), 
										 (unsigned short)(mUpPos.mX + mUpSize.mWidth ), (unsigned short)(mUpPos.mY + mUpSize.mHeight ) );

		/// 사이즈 변경시 위치 이동
		if( mDownSize.mHeight - mUpSize.mHeight > 0 )
		{
			int height = mDownSize.mHeight - mUpSize.mHeight;

			cUIPos pos( mScreenX, mScreenY + height );
			cUINode::SetRelativePos( pos );

			mpTabButtonImage->SetScreenXY( (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop );
		}
	}
}

void cTabButton::SetRelativeRect( const cUIRect& rect )
{
	cUINode::SetRelativeRect( rect );

	mScreenX = (short)rect.mLeft;
	mScreenY = (short)rect.mTop;
}

void cTabButton::SetRelativePos( const cUIPos& pos  )
{
	cUINode::SetRelativePos( pos );

	mScreenX = (short)pos.mX;
	mScreenY = (short)pos.mY;
}
