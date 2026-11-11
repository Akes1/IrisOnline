#include "StdAfx.h"
#include "RadioButton.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"

cRadioButton::cRadioButton( eUINodeType type )
: cButton( type )
, mGroupNum(0)
{
	mpPrev = NULL;
	mpNext = NULL;

	mDownSoundIdx = (unsigned long)-1;

	mProcessEnable = false;
}

cRadioButton::~cRadioButton()
{
}

//////////////////////////////////////////////////////////////////////////////
cRadioButtonSkin::cRadioButtonSkin( eUINodeType type )
: cUINodeSkin( type )
, mGroupNum(0)
, mUpPos(0, 0)
, mDownPos(0, 0)
, mUpOverPos(0, 0)
{
	mUpColor = mDefaultColor;
	mDownColor = mDefaultColor;

	mDownSoundIdx = (unsigned long)-1;
}

cRadioButtonSkin::~cRadioButtonSkin()
{
}

/// 주의사항 : 라디오 버튼에서 필요한 내용 따로 파싱
bool cRadioButtonSkin::Load( cParser& parser )
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
		case eTOKEN_DOWNSOUND:
			{
				mDownSoundIdx = parser.ParseInt();
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
		case eTOKEN_GROUPNUM:
			{
				mGroupNum = parser.ParseInt();
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
		case eTOKEN_UPPOS:
			{
				mUpPos.mX = parser.ParseInt();
				mUpPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_DOWNPOS:
			{
				mDownPos.mX = parser.ParseInt();
				mDownPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_UPOVERPOS:
			{
				mUpOverPos.mX = parser.ParseInt();
				mUpOverPos.mY = parser.ParseInt();
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
