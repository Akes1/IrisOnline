#include "StdAfx.h"
#include "BaseIcon.h"
#include "Token.h"
#include "Parser.h"
#include "UIContainer.h"
#include "Item_Common.h"
#include "Lexer.h"

cBaseIcon::cBaseIcon( eUINodeType type )
: cUINode( type )
, mParam( 0 )
, mShiftDown( false )
, mEnableUse( true )
, mIconType( eICON_NORMAL )
, mColor( NiColorA::WHITE )
{
	mProcessEnable = true;
}

cBaseIcon::~cBaseIcon()
{
}

////////////////////////////////////////////////////////////////////////////////////////
cIconSkin::cIconSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cIconSkin::~cIconSkin()
{
}

bool cIconSkin::Load( cParser& parser )
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
			break;

		if( cUINodeSkin::ParseLine( parser, token ) == false )
		{
			return false;
		}
	}
	return true;
}

