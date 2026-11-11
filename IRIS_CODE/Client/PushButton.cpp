#include "StdAfx.h"
#include "PushButton.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "UIContainer.h"
#include "FontAgent.h"
#include "SoundSystem.h"

cPushButton::cPushButton( eUINodeType type )
: cButton( type )
{
	mPressed = false;
	mHovered = false;

	mUpColor = mDefaultColor;
	mDownColor = mDefaultColor;

	mProcessEnable = false;
}

cPushButton::~cPushButton()
{
}

void cPushButton::Hide()
{
	cUINode::Hide();
}

/// 주의사항 : 
void cPushButton::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	if( mPressed )
		pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY+1, mDownColor, mOutLine );
	else
		pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY, mUpColor, mOutLine );
}

/// 주의사항 : 호버 노드 이벤트 받음 
void cPushButton::OnMouseMove( const cUIPos& )
{
}

/// 주의사항 : 
void cPushButton::OnMouseHovered( const cUIPos& pos )
{
	cUINode::OnMouseHovered( pos );

	mHovered = true;

	if( mPressed )
	{
		mpImage->SetTextureRect( (unsigned short)mDownOverPos.mX, (unsigned short)mDownOverPos.mY, 
								 (unsigned short)(mDownOverPos.mX + mOriginalTexWidth), (unsigned short)(mDownOverPos.mY + mOriginalTexHeight) );
	}
	else
	{
		mpImage->SetTextureRect( (unsigned short)mUpOverPos.mX, (unsigned short)mUpOverPos.mY, 
								 (unsigned short)(mUpOverPos.mX + mOriginalTexWidth), (unsigned short)(mUpOverPos.mY + mOriginalTexHeight) );
	}
}

/// 주의사항 : 
void cPushButton::OnMouseLeft( const cUIPos& pos )
{
	cUINode::OnMouseLeft( pos );

	mHovered = false;

	if( mPressed )
	{
		mpImage->SetTextureRect( (unsigned short)mDownPos.mX, (unsigned short)mDownPos.mY, 
								 (unsigned short)(mDownPos.mX + mOriginalTexWidth), (unsigned short)(mDownPos.mY + mOriginalTexHeight) );
	}
	else
	{
		mpImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
								 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
	}
}

/// 주의사항 : 
void cPushButton::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{	
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	mPressed = !mPressed;

	if( mPressed )
	{
		SOUNDSYS->Play2DSound( mDownSoundIdx );

		mpImage->SetTextureRect( (unsigned short)mDownOverPos.mX, (unsigned short)mDownOverPos.mY, 
								 (unsigned short)(mDownOverPos.mX + mOriginalTexWidth), (unsigned short)(mDownOverPos.mY + mOriginalTexHeight) );
	}
	else
	{
		SOUNDSYS->Play2DSound( mUpSoundIdx );

		mpImage->SetTextureRect( (unsigned short)mUpOverPos.mX, (unsigned short)mUpOverPos.mY, 
								 (unsigned short)(mUpOverPos.mX + mOriginalTexWidth), (unsigned short)(mUpOverPos.mY + mOriginalTexHeight) );
	}

	/// 이벤트 발생
	cUIEvent event;
	event.mType = eUIEVENT_PUSHBUTTON_PRESSED;
	event.mpCaller = this;
	event.mID = mID;
	mpParent->HandleEvent( event );
}

/// 주의사항 :
void cPushButton::OnLButtonDoubleClick( const cUIPos& pos )
{
	/// cPushButton 의 OnLButtonDown 콜
	OnLButtonDown( pos, 0, 0, 0 );
}

/// 주의사항 : 
void cPushButton::OnLButtonUp( const cUIPos& )
{
}

/// 주의사항 :
void cPushButton::SetPress( bool press )
{
	mPressed = press;

	/// 눌렸는지 안눌렷는지 판단해서 소스 이미지 바꿈
	if( mPressed )
	{
		mpImage->SetTextureRect( (unsigned short)mDownPos.mX, (unsigned short)mDownPos.mY, 
								 (unsigned short)(mDownPos.mX + mOriginalTexWidth), (unsigned short)(mDownPos.mY + mOriginalTexHeight) );
	}
	else 
	{
		mpImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
								 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
	}
}
