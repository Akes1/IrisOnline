#include "StdAfx.h"
#include "Button.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "UIContainer.h"
#include "FontAgent.h"
#include "SoundSystem.h"

cButton::cButton( eUINodeType type )
: cUINode( type )
, mOriginalTexWidth( 0 )
, mOriginalTexHeight( 0 )
, mTextPosX( 0 )
, mTextPosY( 0 )
, mLastTick( 0 )
, mTimeFlag( true )
, mUseEnable( false ) 
, mUpPos(0, 0)
, mDownPos(0, 0)
, mDownOverPos(0, 0)
, mEnablePos(0, 0)
, mContinuEvent( false )
{
	mUpdateTime = gOneMoveTime;

	mUpColor = mDefaultColor;
	mDownColor = mDefaultColor;
	mOverColor = mDefaultColor;

	mDownSoundIdx = (unsigned long)-1;
	mUpSoundIdx = (unsigned long)-1;

	mProcessEnable = true;
}

cButton::~cButton()
{
	ReleaseCapture();
	ReleaseHover();
}

/// 주의사항 : 
bool cButton::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_BUTTON ) == false )
	{
		assert( 0 && "not button skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cButtonSkin* p = (cButtonSkin*)pskin;
	if( p->mText.IsEmpty() == false )
		mText = p->mText;

	mOriginalTexWidth = mpImage->GetWidth();
	mOriginalTexHeight = mpImage->GetHeight();
	
	/// 버튼의 이미지 변환좌표 저장 
	mUpPos = p->mUpPos;
	mDownPos = p->mDownPos;
	mDownOverPos = p->mDownOverPos;
	mEnablePos = p->mEnablePos;
	mUseEnable = p->mUseEnable;
	if( p->mUpOverPos.mX == 0 && p->mUpOverPos.mY == 0 )
		mUpOverPos = p->mUpPos;
	else
		mUpOverPos = p->mUpOverPos;

	/// 버튼 상태 텍스트 컬러
	mUpColor = p->mUpColor;
	mDownColor = p->mDownColor;

	mDownSoundIdx = p->mDownSoundIdx;
	mUpSoundIdx = p->mUpSoundIdx;

	/// 텍스트 좌표 업데이트 
	UpdateText();

	return true;
}

/// 주의사항 : 
void cButton::UpdateRect()
{
	cUINode::UpdateRect();

	/// 텍스트 좌표 업데이트 
	UpdateText();
}

/// 주의사항 : 
void cButton::OnProcess( unsigned long , unsigned long accumTime )
{
	if( mVisible == false )
		return;

	/// 버튼이 계속 눌려있음을 검사 
	if( mPressed )
	{
		if( mTimeFlag )
		{
			mLastTick = accumTime;
			mTimeFlag = false;
		}		

		unsigned long elapsedTime = accumTime - mLastTick;
		if( elapsedTime >= mUpdateTime )
		{
			mUpdateTime = gContinueMoveTime;

			/// 일정시간이 지나면 버튼이 눌렸다는 메시지를 계속보낸다
			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mType = eUIEVENT_BUTTON_DOWNED;
			mpParent->HandleEvent( event );

			mLastTick = accumTime;
		}

		/// 연속적인 이벤트를 받기원하면
		if( mContinuEvent )
		{
			/// 눌려있는동안 계속보낸다.
			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mType = eUIEVENT_BUTTON_CONTINUEDOWNED;
			mpParent->HandleEvent( event );
		}
	}
}

/// 주의사항 : 
void cButton::OnRender( cUIFontItemKeeper* pKeeper )
{
	/// 이미지 렌더링
	mpImage->Draw();

	if( mPressed )
	{
		pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY+1, mDownColor, mOutLine );
	}
	else
	{
		pKeeper->AddFontItem( cFontAgent::eFont_UI, const_cast<LPTSTR>(mText.Cstr()), mTextPosX, mTextPosY, mUpColor, mOutLine );
	}
}

/// 주의사항 : 
void cButton::OnMouseMove( const cUIPos& pos )
{
	if( mAbsoluteRect.ContainPoint( pos ) == false )
	{
		ReleaseCapture();
		mPressed = false;
	}
}

/// 주의사항 : 호버 노드 이벤트 받음 
void cButton::OnMouseHovered( const cUIPos& pos )
{
	cUINode::OnMouseHovered( pos );

	// 소스 이미지 바꾸기
	mpImage->SetTextureRect( (unsigned short)mUpOverPos.mX, (unsigned short)mUpOverPos.mY, 
						     (unsigned short)(mUpOverPos.mX + mOriginalTexWidth), (unsigned short)(mUpOverPos.mY + mOriginalTexHeight) );
}

/// 주의사항 : 호버 노드 이벤트 받음 
void cButton::OnMouseLeft( const cUIPos& pos )
{
	cUINode::OnMouseLeft( pos );

	// 소스 이미지 바꾸기
	mpImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
							 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
}

/// 주의사항 : 
void cButton::OnLButtonDoubleClick( const cUIPos& pos )
{
	cButton::OnLButtonDown( pos, false, false, false );
}

/// 주의사항 : 
void cButton::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );
	
	///
	SOUNDSYS->Play2DSound( mDownSoundIdx );

	/// 캡쳐가 없으면 캡쳐를 갖는다
	if( GetCapture() != this )
	{
		SetCapture();
	}

	// 소스 이미지 바꾸기
	mpImage->SetTextureRect( (unsigned short)mDownPos.mX, (unsigned short)mDownPos.mY, 
							 (unsigned short)(mDownPos.mX + mOriginalTexWidth), (unsigned short)(mDownPos.mY + mOriginalTexHeight) );
	mPressed = true;
	mTimeFlag = true;
}


/// 주의사항 : 
void cButton::OnLButtonUp( const cUIPos& )
{
	if( mPressed && mEnabled == true )
	{
		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
								 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );

		mUpdateTime = gOneMoveTime;
		mPressed = false;

		/// 이벤트 발생
		cUIEvent event;
		event.mType = eUIEVENT_COMMAND;
		event.mpCaller = this;
		event.mID = mID;
		mpParent->HandleEvent( event );
	}
}

/// 주의사항 : 텍스트 좌표 업데이트 
void cButton::UpdateText()
{
	if( mText )
	{
		/// 텍스트가 뿌려질 위치 계산 
		cUIRect rc = GetAbsoluteRect();

		mTextPosX = rc.mLeft + (int)(( rc.GetWidth() - FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, mText.Cstr(), mText.GetLength() )) * 0.5f );
		/// +1 은 위치 보정 
		mTextPosY = rc.mTop + (int)(( rc.GetHeight() - FONTAGENT->GetTextHeight(cFontAgent::eFont_UI) + 1) * 0.5f);
	}	
}

/// 주의사항 : 버튼의 활성화 설정  
void cButton::SetEnabled( bool enabled )
{
	ReleaseCapture();

	if( mEnabled == enabled )
		return;

	if( mUseEnable && enabled == false )
	{	
		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mEnablePos.mX, (unsigned short)mEnablePos.mY, 
								 (unsigned short)(mEnablePos.mX + mOriginalTexWidth), (unsigned short)(mEnablePos.mY + mOriginalTexHeight) );

	}
	else
	{
		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mUpPos.mX, (unsigned short)mUpPos.mY, 
							 	 (unsigned short)(mUpPos.mX + mOriginalTexWidth), (unsigned short)(mUpPos.mY + mOriginalTexHeight) );
	}

	cUINode::SetEnabled( enabled );
}


void cButton::SetText( LPCTSTR text )
{
	mText = text;

	UpdateText();
}

void cButton::Hide()
{
	cUINode::Hide();

//	ReleaseCapture();
	mPressed = false;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
cButtonSkin::cButtonSkin( eUINodeType type )
: cUINodeSkin( type )
, mUpPos(0, 0)
, mUpOverPos(0, 0)
, mDownPos(0, 0)
, mDownOverPos(0, 0)
, mUpSize(0, 0)
, mDownSize(0, 0)
, mEnablePos(0, 0)
{
	mUseEnable = false;

	mUpColor = mDefaultColor;
	mDownColor = mDefaultColor;

	mDownSoundIdx = (unsigned long)-1;
	mUpSoundIdx = (unsigned long)-1;
}

cButtonSkin::~cButtonSkin()
{
}

/// 주의사항 : 버튼에서 필요한 내용 따로 파싱
bool cButtonSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
			break;

		switch( token.mType )
		{
		case eTOKEN_DOWNSOUND:
			{
				/// 클릭시 사운드
				mDownSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_UPSOUND:
			{
				/// 업 사운드 
				mUpSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_UPCOLOR:
			{
				/// 다음 토큰은 "(" 이므로 넘어간다 
				if( parser.ExpectTokenString( "(" ) == false )
					return false;

				unsigned char r = (unsigned char)parser.ParseInt();
				unsigned char g = (unsigned char)parser.ParseInt();
				unsigned char b = (unsigned char)parser.ParseInt();

				if( parser.ExpectTokenString( ")" ) == false )
					return false;

				/// 변환 ( R,G,B 순서로 )
				unsigned long color = (unsigned long)( (DWORD)((BYTE)r << 16) | (WORD)((BYTE)g << 8) | (BYTE)b );

				/// down 컬러는 기본으로 up 컬러랑 동일하게 한다
				mUpColor = mDownColor = color | 0xff000000;
			}
			break;
		case eTOKEN_DOWNCOLOR:
			{
				/// 다음 토큰은 "(" 이므로 넘어간다 
				if( parser.ExpectTokenString( "(" ) == false )
					return false;

				unsigned char r = (unsigned char)parser.ParseInt();
				unsigned char g = (unsigned char)parser.ParseInt();
				unsigned char b = (unsigned char)parser.ParseInt();

				if( parser.ExpectTokenString( ")" ) == false )
					return false;

				/// 변환 ( R,G,B 순서로 )
				unsigned long color = (unsigned long)( (DWORD)((BYTE)r << 16) | (WORD)((BYTE)g << 8) | (BYTE)b );

				mDownColor = color | 0xff000000;
			}
			break;
		case eTOKEN_TEXT:
			{
				int i = parser.ParseInt();
				if( UIMAN->GetUIText( &mText, i ) == false )
				{
					assert ( 0 && "failed to interface - button text" );
					return false;
				}
			}
			break;
		case eTOKEN_UPPOS:
			{
				mUpPos.mX = parser.ParseInt();
				mUpPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_UPOVERPOS:
			{
				mUpOverPos.mX = parser.ParseInt();
				mUpOverPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_DOWNPOS:
			{
				mDownPos.mX = parser.ParseInt();
				mDownPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_DOWNOVERPOS:
			{
				mDownOverPos.mX = parser.ParseInt();
				mDownOverPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_ENABLEPOS:
			{
				mEnablePos.mX = parser.ParseInt();
				mEnablePos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_UPSIZE:
			{
				mUpSize.mWidth = parser.ParseInt();
				mUpSize.mHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_DOWNSIZE:
			{
				mDownSize.mWidth = parser.ParseInt();
				mDownSize.mHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_USEENABLE:
			{
				/// mUseEnable 다음에 오는 값을 읽어들이기 위함
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mUseEnable = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mUseEnable = false;
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

