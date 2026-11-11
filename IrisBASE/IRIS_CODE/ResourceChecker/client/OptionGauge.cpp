#include "StdAfx.h"
#include "OptionGauge.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIImage.h"
//#include "BarButton.h"

cOptionGauge::cOptionGauge( eUINodeType type )
: cUIWindow( type )
, mCurPercent(0)
, mpBarButton(0)
{
	
}

cOptionGauge::~cOptionGauge()
{
	mpBarButton = 0;
}


/////////////////////////////////////////////////////////////////////
cOptionGaugeSkin::cOptionGaugeSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cOptionGaugeSkin::~cOptionGaugeSkin()
{
}

/// 주의사항 : 버튼에서 필요한 내용 따로 파싱
bool cOptionGaugeSkin::Load( cParser& parser )
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