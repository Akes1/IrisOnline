#include "StdAfx.h"
#include "CheckBox.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "UIContainer.h"
#include "SoundSystem.h"

cCheckBox::cCheckBox( eUINodeType type )
: cUINode( type )
, mCheckPos(0, 0)
, mNoCheckPos(0, 0)
, mOriginalTexWidth(0)
, mOriginalTexHeight(0)
{
	mCheckSoundIdx = (unsigned long)-1;
	mNoCheckSoundIdx = (unsigned long)-1;
}

cCheckBox::~cCheckBox()
{
}

/// 주의사항 : 
bool cCheckBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_CHECKBOX ) == false )
	{
		assert( 0 && "not checkbox skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	cCheckBoxSkin* p = (cCheckBoxSkin*)pskin;
	/// 눌림 여부 적용
	SetPress( p->mPressed );

	/// 체크된 이미지 좌표
	mCheckPos = p->mCheckPos;
	/// 체크 안된 이미지 좌표
	mNoCheckPos = p->mNoCheckPos;
	/// 텍스쳐 사이즈
	mOriginalTexWidth = mpImage->GetWidth();
	mOriginalTexHeight = mpImage->GetHeight();

	mCheckSoundIdx = p->mCheckSoundIdx;
	mNoCheckSoundIdx = p->mNoCheckSoundIdx;

	/// 상대 좌표 사용 
	/// 초기 스크립트에서 체크 상태를 검사
	if( mPressed )
	{
		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mCheckPos.mX, (unsigned short)mCheckPos.mY, 
								 (unsigned short)(mCheckPos.mX + mOriginalTexWidth), (unsigned short)(mCheckPos.mY + mOriginalTexHeight) );
	}
	else
	{
		mpImage->SetTextureRect( (unsigned short)mNoCheckPos.mX, (unsigned short)mNoCheckPos.mY, 
								 (unsigned short)(mNoCheckPos.mX + mOriginalTexWidth), (unsigned short)(mNoCheckPos.mY + mOriginalTexHeight) );
	}

	return true;
}

/// 주의사항 : 
bool cCheckBox::IsValid() const
{
	if( !mpImage )
	{
		assert( 0 && "invalid image" );
		return false;
	}

	if( cUINode::IsValid() == false )
	{
		return false;
	}
	return true;
}

/// 주의사항 : 
void cCheckBox::OnRender( cUIFontItemKeeper* )
{
	/// 이미지 렌더링
	mpImage->Draw();
}

/// 주의사항 : 
void cCheckBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	mPressed = !mPressed;

	if( mPressed )
	{
		SOUNDSYS->Play2DSound( mCheckSoundIdx );

		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mCheckPos.mX, (unsigned short)mCheckPos.mY, 
			(unsigned short)(mCheckPos.mX + mOriginalTexWidth), (unsigned short)(mCheckPos.mY + mOriginalTexHeight) );
	}
	else
	{
		SOUNDSYS->Play2DSound( mNoCheckSoundIdx );

		mpImage->SetTextureRect( (unsigned short)mNoCheckPos.mX, (unsigned short)mNoCheckPos.mY, 
			(unsigned short)(mNoCheckPos.mX + mOriginalTexWidth), (unsigned short)(mNoCheckPos.mY + mOriginalTexHeight) );
	}

	/// 이벤트 발생
	cUIEvent event;
	event.mType = eUIEVENT_CHECKBOX_CHANGED;
	event.mpCaller = this;
	event.mID = mID;
	mpParent->HandleEvent( event );
}

void cCheckBox::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUINode::OnLButtonDoubleClick( pos );
	mPressed = !mPressed;

	if( mPressed )
	{
		SOUNDSYS->Play2DSound( mCheckSoundIdx );

		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mCheckPos.mX, (unsigned short)mCheckPos.mY, 
			(unsigned short)(mCheckPos.mX + mOriginalTexWidth), (unsigned short)(mCheckPos.mY + mOriginalTexHeight) );
	}
	else
	{
		SOUNDSYS->Play2DSound( mNoCheckSoundIdx );

		mpImage->SetTextureRect( (unsigned short)mNoCheckPos.mX, (unsigned short)mNoCheckPos.mY, 
			(unsigned short)(mNoCheckPos.mX + mOriginalTexWidth), (unsigned short)(mNoCheckPos.mY + mOriginalTexHeight) );
	}

	/// 이벤트 발생
	cUIEvent event;
	event.mType = eUIEVENT_CHECKBOX_CHANGED;
	event.mpCaller = this;
	event.mID = mID;
	mpParent->HandleEvent( event );
}

void cCheckBox::SetPress( bool press )
{
	if( mEnabled == false )
		return;

	mPressed = press;

	if( mPressed )
	{
		// 소스 이미지 바꾸기
		mpImage->SetTextureRect( (unsigned short)mCheckPos.mX, (unsigned short)mCheckPos.mY, 
			(unsigned short)(mCheckPos.mX + mOriginalTexWidth), (unsigned short)(mCheckPos.mY + mOriginalTexHeight) );
	}
	else
	{
		mpImage->SetTextureRect( (unsigned short)mNoCheckPos.mX, (unsigned short)mNoCheckPos.mY, 
			(unsigned short)(mNoCheckPos.mX + mOriginalTexWidth), (unsigned short)(mNoCheckPos.mY + mOriginalTexHeight) );
	}
}


//////////////////////////////////////////////////////////////////////////
cCheckBoxSkin::cCheckBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mPressed( false )
, mCheckPos(0, 0)
, mNoCheckPos(0, 0)
{
	mCheckSoundIdx = (unsigned long)-1;
	mNoCheckSoundIdx = (unsigned long)-1;
}

cCheckBoxSkin::~cCheckBoxSkin()
{
}

/// 주의사항 : 
bool cCheckBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			/// 블록 탈출 ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_ONSOUND:
			{
				/// 체크시 사운드
				mCheckSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_OFFSOUND:
			{
				/// 체크 해제시 사운드
				mNoCheckSoundIdx = parser.ParseInt();
			}
			break;
		case eTOKEN_CHECKPOS:
			{
				mCheckPos.mX = parser.ParseInt();
				mCheckPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_NOCHECKPOS:
			{
				mNoCheckPos.mX = parser.ParseInt();
				mNoCheckPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_CHECKED:
			{
				/// checked 다음에 오는 값을 읽어들이기 위함
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mPressed = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mPressed = false;
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
