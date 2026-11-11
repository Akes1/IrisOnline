#include "Stdafx.h"
#include "Label.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIContainer.h"
#include "FontAgent.h"

cLabel::cLabel( eUINodeType type )
: cUINode( type )
, mTextPosX( 0 )
, mTextPosY( 0 )
, mEditPos( 0, 0 )
, mpSkin(0)
{	
	mTextAlign = eALIGN_NONE;
	mTextColor = mDefaultColor;
	mDescNode = 0;
	
	mFontType = cFontAgent::eFont_UI;
}

cLabel::~cLabel()
{
	mTextArr.Clear();
}

void cLabel::Clear()
{
	mTextArr.Clear();
	mText.Clear();
}

/// 주의사항 : 
bool cLabel::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_LABEL ) == false )
	{
		assert( 0 && "not label skin type" );
		return false;
	}

	/// 노드 스킨의 적합성 검사
	if( pskin->IsValid() == false )
	{
		assert( 0 && "invalid skin" );
		return false;
	}

	/// 이미지 설정
	if( pskin->mpTexture )
	{
		if( mpImage == 0 )
		{
			mpImage = new cUIImage( pskin->mpTexture );
		}
		else
		{
			assert(0);
		}

		/// 컬러를 지정
		SetColor( pskin->mColor );
//		mpImage->SetColor( pskin->mColor );

		/// 텍스처 영역 설정
		unsigned int tx = pskin->mSkinInfo->mTexX;
		unsigned int ty = pskin->mSkinInfo->mTexY;
		unsigned int tw = pskin->mSkinInfo->mTexWidth;
		unsigned int th = pskin->mSkinInfo->mTexHeight;

		mpImage->SetTextureRect( tx, ty, tx + tw, ty + th );
	}
	else
	{
		delete mpImage;
		mpImage = 0;
	}

	unsigned int x = pskin->mSkinInfo->mX;
	unsigned int y = pskin->mSkinInfo->mY;
	unsigned int w = pskin->mSkinInfo->mWidth;
	unsigned int h = pskin->mSkinInfo->mHeight;
	SetRelativeRect( cUIRect(x, y, x + w, y + h) );

	/// 
	cLabelSkin* p = (cLabelSkin*)pskin;
	mpSkin = (cUINodeSkin*)pskin;
	mTextAlign = p->mTextAlign;

	/// 사용자가 설정한 좌표가 있으면 지정
	/// 텍스트 정렬이 우선순위를가진다 
	mEditPos.mX = p->mEditPos.mX;
	mEditPos.mY = p->mEditPos.mY;

	/// 
	mHoverTipIndex = pskin->mSkinInfo->mHoverTipIndex;

	/// 텍스트 좌표 업데이트
	if( p->mText )
	{
		SetText( (LPCTSTR)p->mText.Cstr() );
		UpdateText();
	}	
	return true;
}

/// 주의사항 : 
bool cLabel::HandleEvent( const cUIEvent& event )
{
	if( !mpImage )
	{
		/// 이미지가 없는 라벨인 경우, 해당 이벤트만 처리한다
		if( !(event.mType == eUIEVENT_MOUSE_HOVERED || event.mType == eUIEVENT_MOUSE_LEFT) )
		{
			return mpParent->HandleEvent( event );
		}
	}	
	/// 이미지가 있는 라벨인 경우 마우스 좌클릭/좌더블클릭 은 부모에게 넘긴다.
	else if( event.mType == eUIEVENT_LBUTTON_DOWN || event.mType == eUIEVENT_LBUTTON_DBLCLK )
		return mpParent->HandleEvent( event );

	return cUINode::HandleEvent( event );
}

void cLabel::SetTranslate( short x, short y )
{
	if( mpImage )
	{
		float hw = (float)GetAbsoluteRect().GetWidth()/(float)GetScreenWidth() * 0.5f;
		float hh = (float)GetAbsoluteRect().GetHeight()/(float)GetScreenHeight() * 0.5f;

		unsigned int px = x + GetAbsoluteRect().mLeft;
		unsigned int py = y + GetAbsoluteRect().mTop;

		mpImage->SetTranslate( (float)px/(float)GetScreenWidth() + hw, (float)py/GetScreenHeight() + hh );

		SetRelativePos( cUIPos( x, y ) );
	}
}

void cLabel::SetRotate( float angle )
{
	if( mpImage )
		mpImage->SetRotate( angle );
}

void cLabel::SetTexture( NiTexture* pTexture, unsigned int texX, unsigned int texY )
{
	if( mpSkin )
	{
		if( mpSkin->mpTexture == pTexture )
			return;

		mpSkin->mpTexture = pTexture;

		if( mpImage == 0 )
		{
			mpImage = new cUIImage( mpSkin->mpTexture );
		}
		else
		{
			assert(0);
			delete mpImage;
			mpImage = new cUIImage( mpSkin->mpTexture );
		}

		/// 컬러를 지정
		SetColor( mpSkin->mColor );

		unsigned int w = mpSkin->mSkinInfo->mTexWidth;
		unsigned int h = mpSkin->mSkinInfo->mTexHeight;

		if( mpImage )
			mpImage->SetTextureRect( texX, texY, texX + w, texY + h );
	}
}

void cLabel::ChangeImage( cUINodeSkin* pSkin )
{
	if( mpSkin == pSkin )
		return;

	if( pSkin )
	{
		mpSkin = pSkin;

		unsigned int texX = mpSkin->mSkinInfo->mTexX;
		unsigned int texY = mpSkin->mSkinInfo->mTexY;
		unsigned int w = mpSkin->mSkinInfo->mTexWidth;
		unsigned int h = mpSkin->mSkinInfo->mTexHeight;

		if( mpImage )
			mpImage->SetTextureRect( texX, texY, texX + w, texY + h );
	}
	else
	{
		assert(0);
	}
}

void cLabel::ChangeImage( unsigned int texX, unsigned int texY )
{
	if( mpSkin == 0 )
		return;

	unsigned int w = mpSkin->mSkinInfo->mTexWidth;
	unsigned int h = mpSkin->mSkinInfo->mTexHeight;

	if( mpImage )
		mpImage->SetTextureRect( texX, texY, texX + w, texY + h );
}

void cLabel::ChangeImage( unsigned int texX, unsigned int texY, unsigned int texW, unsigned int texH )
{
	if( mpSkin == 0 )
	{
		assert(0);
		return;
	}

	if( mpImage )
		mpImage->SetTextureRect( texX, texY, texX + texW, texY + texH );
}

void cLabel::OnMouseHovered( const cUIPos& pos )
{
	cUINode::OnMouseHovered( pos );

	//if( mpParent && mpImage )
	if( mpParent )
	{
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mPos = pos;
		event.mType = eUIEVENT_MOUSE_HOVERED;
		mpParent->HandleEvent( event );
	}
}

void cLabel::OnMouseLeft( const cUIPos& pos )
{
	cUINode::OnMouseLeft( pos );

	//if( mpParent && mpImage )
	if( mpParent )
	{
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_MOUSE_LEFT;
		mpParent->HandleEvent( event );
	}
}

/// 주의사항 : 
void cLabel::SetText( LPCTSTR str )
{
	/// 문자열 폭 구하기
	unsigned int len = ::_tcslen( str );
	if( len <= 0 )
		return;

	mText = str;
	mTextArr.Clear();

	unsigned int width = FONTAGENT->GetTextExtent( mFontType, str, len );
	
	/// 라벨 폭
	unsigned int descWidth = 0;
	if( mDescNode )
	{
		descWidth = mDescNode->GetAbsoluteRect().GetWidth();
	}
	else if( GetAbsoluteRect().GetWidth() > 0 )
	{
		/// 기준이 없으면 자신의 사이즈가 기준
		descWidth = GetAbsoluteRect().GetWidth();
	}
	else
	{
		/// 기준이 없으면 그냥 삽입 
		descWidth = width;
	}

	/// 검사한 폭과 길이
	unsigned int cutWidth = 0;
	unsigned int cutlen = 0;
	unsigned int startIndex = 0;

	/// 삽입용 문자열
	TCHAR	text[256] = {0,};
	LPTSTR newtext = text;

	/// 사이즈보다 크면 잘라낸다.
	while( len != cutlen )
	{
		/// 현재까지 검사한 문자의 폭 구하기
		cutlen++;
		cutWidth = FONTAGENT->GetTextExtent( mFontType, str + startIndex, cutlen );

		/// 현재 문자검사를 한다
		TCHAR temp[3] = {0,};
		::_tcsncpy_s( temp, &str[startIndex + cutlen], 2 );

		/// 엔터부호가 있으면 줄넘김
		if( ::_tcsncmp( temp, _T("\\n"), 2 ) == 0 )
		{
			/// 삽입
			Sstrncpy( newtext, 256, str + startIndex, cutlen );
			mTextArr.PushBack( newtext );

			///
			width -= cutWidth; 
			startIndex += cutlen + 2;
			len -= (cutlen + 2);
			cutWidth = 0;
			cutlen = 0;
		}
		else
		{
			if( cutWidth > descWidth )	
			{
				if( cutlen > 1 )
				{
					/// UNICODE 니까 무조건 길이를 1씩 
					int minusWidth = FONTAGENT->GetTextExtent( mFontType, str + startIndex + cutlen - 1, 1 );
					cutWidth -= minusWidth;
					cutlen -= 1;
				}

				Sstrncpy( newtext, 256, str + startIndex, cutlen );
				mTextArr.PushBack( newtext );

				///
				width -= cutWidth;
				startIndex += cutlen;
				len -= cutlen;
				cutWidth = 0;
				cutlen = 0;
			}
		}
	}

	if( len > 0 )
	{
		Sstrncpy( newtext, 256, str + startIndex, len );
		mTextArr.PushBack( newtext );
	}

	///
	UpdateText();
}

/// 주의사항 : 
void cLabel::OnProcess( unsigned long, unsigned long /*accumTime*/ )
{
}

void cLabel::UpdateRect()
{
	cUINode::UpdateRect();

	///
	UpdateText();
}

void cLabel::UpdateText()
{
	if( mTextArr.GetSize() <= 0 )
		return;

	cUIRect rc;
	/// 기준이 있으면 
	if( mDescNode )
	{
		rc = mDescNode->GetAbsoluteRect();
	}
	else
	{
		/// 기준이 없으면 자신의 사이즈가 기준
		rc = GetAbsoluteRect();
	}

	switch( mTextAlign )
	{
	case eALIGN_NONE:
		{
			mTextPosX = rc.mLeft + mEditPos.mX;
			mTextPosY = rc.mTop + mEditPos.mY;
		}
		break;
	case eALIGN_LEFT:
		{
			mTextPosX = rc.mLeft;
			mTextPosY = rc.mTop;
		}
		break;
	case eALIGN_RIGHT:
		{
			mTextPosX = rc.mRight - FONTAGENT->GetTextExtent( mFontType, (LPTSTR)mTextArr[0].Cstr(), mTextArr[0].GetLength());
			mTextPosY = rc.mTop + ( rc.GetHeight() - FONTAGENT->GetTextHeight(mFontType) ) / 2;
		}
		break;
	case eALIGN_CENTER:		/// 넓이, 높이 정렬이면
		{
			mTextPosX = rc.mLeft + ( rc.GetWidth() - FONTAGENT->GetTextExtent( mFontType, (LPTSTR)mTextArr[0].Cstr(), mTextArr[0].GetLength()) ) / 2;
			mTextPosY = rc.mTop + ( rc.GetHeight() - FONTAGENT->GetTextHeight( mFontType ) ) / 2;
		}
		break;
	}
}

/// 주의사항 : 
void cLabel::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mpImage )
		mpImage->Draw();

	if( mTextArr.GetSize() <= 0 )
		return;

	/// 일반적으로 하나만 출력하는 라벨이면 그냥 출력
	if( mTextArr.GetSize() == 1 )
	{
		pKeeper->AddFontItem( mFontType, (LPTSTR)mTextArr[0].Cstr(), mTextPosX, mTextPosY, mTextColor, mOutLine );
	}	
	else
	{
		cUIRect rc;

		/// 여러줄을 출력하는 라벨 
		for( unsigned int i = 0; i < mTextArr.GetSize(); ++i )
		{
			LPTSTR text = (LPTSTR)mTextArr[i].Cstr();
			int len = ::_tcslen( text );

			/// 기준이 있으면 
			if( mDescNode )
			{
				rc = mDescNode->GetAbsoluteRect();
			}
			else
			{
				/// 기준이 없으면 자신의 사이즈가 기준
				rc = GetAbsoluteRect();
			}

			int editX = rc.mLeft;
			int editY = rc.mTop + ( i * FONTAGENT->GetTextHeight( mFontType ) );

			/// 중앙정렬이면
			if( mTextAlign == eALIGN_CENTER )
			{
				editX += ( rc.GetWidth() - FONTAGENT->GetTextExtent( mFontType, text, len ) ) / 2;
				editY += ( rc.GetHeight() - (int)(FONTAGENT->GetTextHeight( mFontType ) * mTextArr.GetSize()) ) / 2;
			}
			///
			else if( mTextAlign == eALIGN_RIGHT )
			{
				editX = rc.mRight - FONTAGENT->GetTextExtent( mFontType, text, len );
				editY = rc.mTop + ( rc.GetHeight() - (int)(FONTAGENT->GetTextHeight( mFontType ) * mTextArr.GetSize()) ) / 2;
			}

			pKeeper->AddFontItem( mFontType, text, editX, editY, mTextColor, mOutLine );
		}
	}
}

///
void cLabel::SetValue( int value )
{
	cStringT buf;
	buf.Format( _T("%d"), value );
	SetText( (LPCTSTR)buf.Cstr() );
}

void cLabel::SetHoverTip( unsigned long tipIndex )
{
	mHoverTipIndex = tipIndex;
}

void cLabel::EnableBoldText( bool set )
{
	cUINode::EnableBoldText( set );

	unsigned char fontIdx = (unsigned char)(mUseBoldText? cFontAgent::eFont_UI_Bold : cFontAgent::eFont_UI);
	mFontType = fontIdx;
}


///////////////////////////////////////////////////////////////////////////
/// 주의사항 : 
cLabelSkin::cLabelSkin( eUINodeType type )
: cUINodeSkin( type )
, mTextAlign( eALIGN_NONE )
, mEditPos(0, 0)
{
}

cLabelSkin::~cLabelSkin()
{

}

/// 주의사항 : 
bool cLabelSkin::Load( cParser& parser )
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
		case eTOKEN_EDITPOS:
			{
				/// 문자열 출력위치 설정 
				mEditPos.mX = parser.ParseInt();
				mEditPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_TEXT_ALIGN:
			{
				///
				lexer->GetNextToken( &token );

				/// 정렬 
				mTextAlign = eALIGN_LEFT;

				if( token.mType == eTOKEN_CENTER )
				{
					mTextAlign = eALIGN_CENTER;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					mTextAlign = eALIGN_RIGHT;
				}
			}
			break;
		case eTOKEN_TEXT:
			{
				int i = parser.ParseInt();
				if( UIMAN->GetUIText( &mText, i ) == false )
				{
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
