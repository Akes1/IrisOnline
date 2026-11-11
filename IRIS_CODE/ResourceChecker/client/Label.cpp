#include "Stdafx.h"
#include "Label.h"
#include "UIImage.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIContainer.h"
//#include "FontAgent.h"

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
	
//	mFontIndex = cFontAgent::eFont_UI;
}

cLabel::~cLabel()
{
	mTextArr.Clear();
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
					cString msg;
					msg.Format("[%s] [Line:%d]", parser.GetFileName(), token.mLine );
					MessageBoxA( NULL, msg.Cstr(), "Bad interface message index", MB_OK | MB_ICONERROR );
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
