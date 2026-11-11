#include "stdafx.h"
#include "Parser.h"
#include "Lexer.h"
#ifdef _CLIENT
	#include "Console.h"
#endif;

cParser::cParser()
: mpLexer( 0 )
{
}

cParser::cParser( cLexer* lexer, const cString& fileName )
: mpLexer( lexer )
, mFileName( fileName )
{
}

cParser::~cParser()
{
}

bool cParser::ExpectTokenString( const char* str )
{
	mpLexer->GetNextString( &mToken );

	if( mToken != str )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: expected \"%s\" but found \"%s\"", mFileName.Cstr(), mToken.mLine, str, mToken.Cstr() );
		return false;
	}
	return true;
}

bool cParser::ExpectTokenType( int type )
{
	if( mpLexer->GetNextToken( &mToken ) != type )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: expected mToken type %d but found %d ( \"%s\" )", mFileName.Cstr(), mToken.mLine, type, mToken.mType, mToken.Cstr() );
		return false;
	}
	return true;
}

cString cParser::ParseString()
{
	mpLexer->GetNextString( &mToken );


	return mToken;
}

bool cParser::ParseString( cString* str )
{
	mpLexer->GetNextString( &mToken );
	*str = mToken;
	return true;
}

int cParser::ParseInt()
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not integer", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		assert( 0 && "token is not integer" );
		return 0;
	}
	return mToken.ToInt();
}

bool cParser::ParseInt( int* out )
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not integer, mToken type:%d", mFileName.Cstr(), mToken.mLine, mToken.Cstr(), type );
		assert( 0 && "token is not integer" );
		return false;
	}
	*out = mToken.ToInt();
	return true;
}

float cParser::ParseFloat()
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT && type != eTOKEN_FLOAT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not number", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		assert( 0 && "token is not float" );
		return 0.0f;
	}
	return mToken.ToFloat();
}

bool cParser::ParseFloat( float* out )
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT && type != eTOKEN_FLOAT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not number", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		assert( 0 && "token is not float" );
		return false;
	}
	*out = mToken.ToFloat();
	return true;
}

///-----------------------------------------
//------------------------------------------------------------------------
cParserW::cParserW()
: mpLexer( 0 )
{
}

cParserW::cParserW( cLexerW* lexer, const cStringW& fileName )
: mpLexer( lexer )
, mFileName( fileName )
{
}

cParserW::~cParserW()
{
}

bool cParserW::ExpectTokenString( const wchar_t* str )
{
	mpLexer->GetNextString( &mToken );

	if( mToken != str )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: expected \"%s\" but found \"%s\"", mFileName.Cstr(), mToken.mLine, str, mToken.Cstr() );
		return false;
	}
	return true;
}

bool cParserW::ExpectTokenType( int type )
{
	if( mpLexer->GetNextToken( &mToken ) != type )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: expected mToken type %d but found %d ( \"%s\" )", mFileName.Cstr(), mToken.mLine, type, mToken.mType, mToken.Cstr() );
		return false;
	}
	return true;
}

cStringW cParserW::ParseString()
{
	mpLexer->GetNextString( &mToken );

	return mToken;
}

bool cParserW::ParseString( cStringW* str )
{
	mpLexer->GetNextString( &mToken );
	*str = mToken;
	return true;
}

int cParserW::ParseInt()
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not integer", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		assert( 0 && "token is not integer" );
		return 0;
	}
	return mToken.ToInt();
}

bool cParserW::ParseInt( int* out )
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not integer, mToken type:%d", mFileName.Cstr(), mToken.mLine, mToken.Cstr(), type );
		assert( 0 && "token is not integer" );
		return false;
	}
	*out = mToken.ToInt();
	return true;
}

float cParserW::ParseFloat()
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT && type != eTOKEN_FLOAT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not number", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		assert( 0 && "token is not float" );
		return 0.0f;
	}
	return mToken.ToFloat();
}

bool cParserW::ParseFloat( float* out )
{
	int type = mpLexer->GetNextString( &mToken );

	if( type != eTOKEN_INT && type != eTOKEN_FLOAT )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: %s is not number", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		assert( 0 && "token is not float" );
		return false;
	}
	*out = mToken.ToFloat();
	return true;
}
