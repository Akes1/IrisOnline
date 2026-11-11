#include "StdAfx.h"
#include "UIText.h"

#include "Lexer.h"
#include "Parser.h"

cUIText::cUIText()
: mErrorText( "Error!" )
{
}

cUIText::~cUIText()
{
}

/// 주의사항 : 
bool cUIText::Load( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, pathName );
	unsigned int index = 0;
	cString text;

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

		/// ui 텍스트 맵에 추가
		if( mStringMap.Insert( index, text.Cstr() ) == false )
		{
			assert( 0 && "failed to insert ui text index, maybe already exist" );
			return false;
		}
	}
	return true;
}

///
bool cUIText::GetText( cString* ptext, unsigned int index )
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

const cString& cUIText::GetText( unsigned int id ) const
{
	cStringMap::cConstIterator i = mStringMap.Find( id );

	if( i == mStringMap.End() )
	{
		assert( 0 && "failed to find ui text" );
		return mErrorText;
	}
	else
		return i->mSecond;
}
