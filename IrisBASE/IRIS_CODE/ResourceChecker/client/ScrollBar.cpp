#include "Stdafx.h"
#include "ScrollBar.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"

#include "UIImage.h"
#include "UIContainer.h"
//#include "PlaneObject.h"

#include "Button.h"
//#include "BarButton.h"

cScrollBar::cScrollBar( eUINodeType type )
: cUIWindow( type )
//, mpScrollImage(0)
, mpUpButton(0)
, mpDownButton(0)
, mpBarButton(0)
{	
}

cScrollBar::~cScrollBar()
{
//	SAFE_DELETE( mpScrollImage );

	mpBarButton = 0;
	mpDownButton = 0;
	mpUpButton = 0;
}


/////////////////////////////////////////////////////////////////////////
cScrollBarSkin::cScrollBarSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cScrollBarSkin::~cScrollBarSkin()
{

}

/// 주의사항 : 
bool cScrollBarSkin::Load( cParser& parser )
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
		case eTOKEN_BARBUTTONSKIN:
			{
				mBarButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_UPBUTTONSKIN:
			{
				mUpButtonSkin = parser.ParseString();
			}
			break;
		case eTOKEN_DOWNBUTTONSKIN:
			{
				mDownButtonSkin = parser.ParseString();
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

