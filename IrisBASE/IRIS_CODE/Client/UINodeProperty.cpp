#include "StdAfx.h"
#include "UINodeProperty.h"

#include "UISkinLexer.h"
#include "Parser.h"
#include "UIProperty.h"

cUINodeProperty::cUINodeProperty( const cString& name, eUINodeType type )
: mType( type )
, mPropertyName( name )
{
}

cUINodeProperty::~cUINodeProperty()
{
}

bool cUINodeProperty::Load( cParser& parser )
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
			break;
		}

		if( ParseLine( parser, token ) == false )
		{
			return false;
		}
	}
	return true;
}

/// 주의사항 : 
bool cUINodeProperty::ParseLine( cParser& parser, cToken& token )
{
	cUIProperty* prop = UIMAN->GetProperty();
	assert( prop && "null ui property" );

	switch( token.mType )
	{
	case eTOKEN_SKINNAME:
		{
			mSkinName = parser.ParseString();
		}
		break;
	case eTOKEN_PARENT:
		{
			cString parentName;
			if( parser.ParseString( &parentName ) == false )
				return false;

			/// 부모의 속성을 찾아서 부모에 세팅 
			cUINodeProperty* parent = prop->GetNodeProperty( parentName );
			if( parent == 0 )
			{
				assert( 0 && "failed to get ui node property" );
				return false;
			}

			parent->AddChild( this );
		}
		break;
	default:
		assert( 0 && "invalid keyword" );
		return false;
	}
	
	return true;
}

/// 주의사항 : 
bool cUINodeProperty::IsValid() const
{
	if( mSkinName.IsEmpty() )
	{
		assert( 0 && "skinname is empty!" );
		return false;
	}

	return true;
}

void cUINodeProperty::AddChild( cUINodeProperty* child )
{
	if( !child )
	{
		assert( 0 && "child property is null" );
		return;
	}

	mChildList.PushBack( child );
}
