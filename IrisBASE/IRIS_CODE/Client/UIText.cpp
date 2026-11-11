#include "StdAfx.h"
#include "UIText.h"

#include "Lexer.h"
#include "Parser.h"
#include "Token.h"

cUIText::cUIText()
: mErrorText( _T("Error!") )
{
}

cUIText::~cUIText()
{
}

bool cUIText::Load( const cStringW& pathName )
{
	cFileLoaderW loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// ·º¼­¿¡ ¹äÁÖ°í ÆÄ½Ì
	cTokenW token;
	cLexerW lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParserW parser( &lexer, pathName );
	unsigned int index = 0;
	cStringW text;

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		if( token.mType == eTOKEN_ERROR )
			return false;

		if( token.mType == eTOKEN_NULL )
			continue;

		///
		index = token.ToInt();
		parser.ParseString( &text );

		/// ui ÅØ½ºÆ® ¸Ê¿¡ Ãß°¡
		if( mStringMap.Insert( index, text ) == false )
		{
			assert( 0 && "failed to insert ui text index, maybe already exist" );
			return false;
		}
	}
	return true;
}

///
bool cUIText::GetText( cStringT* ptext, unsigned int index )
{
	cStringMap::cConstIterator i = mStringMap.Find( index );
	if( i == mStringMap.End() )
	{
		assert( 0 && "failed to find UIText" );
		return false;
	}

	*ptext = i->mSecond;
	return true;
}

const cStringT& cUIText::GetText( unsigned int id ) const
{
	cStringMap::cConstIterator i = mStringMap.Find( id );

	if( i == mStringMap.End() )
	{
//		assert( 0 && "failed to find ui text" );
		return mErrorText;
	}
	else
		return i->mSecond;
}
