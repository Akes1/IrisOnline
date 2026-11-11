#include "StdAfx.h"
#include "Button.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "UIContainer.h"

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

