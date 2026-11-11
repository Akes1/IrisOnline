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
		cString msg;
		msg.Format("file \"%s\", line %d: expected \"%s\" but found \"%s\"", mFileName.Cstr(), mToken.mLine, str, mToken.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "file \"%s\", line %d: expected \"%s\" but found \"%s\"", mFileName.Cstr(), mToken.mLine, str, mToken.Cstr() );
		return false;
	}
	return true;
}

bool cParser::ExpectTokenType( int type )
{
	if( mpLexer->GetNextToken( &mToken ) != type )
	{
		cString msg;
		msg.Format("file \"%s\", line %d: expected mToken type %d but found %d ( \"%s\" )", mFileName.Cstr(), mToken.mLine, type, mToken.mType, mToken.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "invalid token", MB_OK | MB_ICONERROR );
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
		cString msg;
		msg.Format("file \"%s\", line %d: %s is not integer", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "token is not integer", MB_OK | MB_ICONERROR );
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
		cString msg;
		msg.Format("file \"%s\", line %d: %s is not integer, mToken type:%d", mFileName.Cstr(), mToken.mLine, mToken.Cstr(), type );
		MessageBoxA( NULL, msg.Cstr(), "token is not integer", MB_OK | MB_ICONERROR );

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
		cString msg;
		msg.Format("file \"%s\", line %d: %s is not number", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "token is not float", MB_OK | MB_ICONERROR );
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
		cString msg;
		msg.Format("file \"%s\", line %d: %s is not number", mFileName.Cstr(), mToken.mLine, mToken.Cstr() );
		MessageBoxA( NULL, msg.Cstr(), "token is not float", MB_OK | MB_ICONERROR );
		return false;
	}
	*out = mToken.ToFloat();
	return true;
}
