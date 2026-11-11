#include "stdafx.h"
#include "Lexer.h"
#include "Token.h"
#include "MemFile.h"

#include <assert.h>

/// 렉서 상태
enum
{
	eIN_NULL = 0,
	eIN_START,
	eIN_DONE,
	eIN_NUM,
	eIN_FLOAT,
	eIN_VAR,
	eIN_STRING,
	eIN_MINUS,
	eIN_DIV,
	eIN_COMMENT
};

/// 렉서 테이블 ( 첫 문자의 시작 상태와 토큰 타입을 저장 )
struct sLexTable
{
	int mType;
	int mState;
};

static sLexTable gLexTable[] =
{
	eTOKEN_NULL,       eIN_NULL,    /// 0x00
	eTOKEN_NULL,       eIN_NULL,    /// 0x01
	eTOKEN_NULL,       eIN_NULL,    /// 0x02
	eTOKEN_NULL,       eIN_NULL,    /// 0x03
	eTOKEN_NULL,       eIN_NULL,    /// 0x04
	eTOKEN_NULL,       eIN_NULL,    /// 0x05
	eTOKEN_NULL,       eIN_NULL,    /// 0x06
	eTOKEN_NULL,       eIN_NULL,    /// 0x07
	eTOKEN_NULL,       eIN_NULL,    /// 0x08
	eTOKEN_NULL,       eIN_NULL,    /// 0x09
	eTOKEN_NULL,       eIN_NULL,    /// 0x0a
	eTOKEN_NULL,       eIN_NULL,    /// 0x0b
	eTOKEN_NULL,       eIN_NULL,    /// 0x0c
	eTOKEN_NULL,       eIN_NULL,    /// 0x0d
	eTOKEN_NULL,       eIN_NULL,    /// 0x0e
	eTOKEN_NULL,       eIN_NULL,    /// 0x0f
	eTOKEN_NULL,       eIN_NULL,    /// 0x10
	eTOKEN_NULL,       eIN_NULL,    /// 0x11
	eTOKEN_NULL,       eIN_NULL,    /// 0x12
	eTOKEN_NULL,       eIN_NULL,    /// 0x13
	eTOKEN_NULL,       eIN_NULL,    /// 0x14
	eTOKEN_NULL,       eIN_NULL,    /// 0x15
	eTOKEN_NULL,       eIN_NULL,    /// 0x16
	eTOKEN_NULL,       eIN_NULL,    /// 0x17
	eTOKEN_NULL,       eIN_NULL,    /// 0x18
	eTOKEN_NULL,       eIN_NULL,    /// 0x19
	eTOKEN_NULL,       eIN_NULL,    /// 0x1a
	eTOKEN_NULL,       eIN_NULL,    /// 0x1b
	eTOKEN_NULL,       eIN_NULL,    /// 0x1c
	eTOKEN_NULL,       eIN_NULL,    /// 0x1d
	eTOKEN_NULL,       eIN_NULL,    /// 0x1e
	eTOKEN_NULL,       eIN_NULL,    /// 0x1f
	eTOKEN_NULL,       eIN_NULL,    /// ' '
	eTOKEN_NOT,        eIN_NULL,    /// '!'
	eTOKEN_NULL,       eIN_STRING,  /// '"'
	eTOKEN_NULL,       eIN_VAR,     /// '#'
	eTOKEN_DOLLAR,     eIN_NULL,    /// '$'
	eTOKEN_PERCENT,    eIN_NULL,    /// '%'
	eTOKEN_AND,        eIN_NULL,    /// '&'
	eTOKEN_NULL,       eIN_STRING,  /// '\''
	eTOKEN_LPAREN,     eIN_NULL,    /// '( '
	eTOKEN_RPAREN,     eIN_NULL,    /// ' )'
	eTOKEN_MUL,        eIN_NULL,    /// '*'
	eTOKEN_PLUS,       eIN_NULL,    /// '+'
	eTOKEN_COMMA,      eIN_NULL,    /// ','
	eTOKEN_NULL,       eIN_MINUS,   /// '-'
	eTOKEN_DOT,        eIN_NULL,    /// '.'
	eTOKEN_NULL,       eIN_DIV,     /// '/'
	eTOKEN_NULL,       eIN_NUM,     /// '0'
	eTOKEN_NULL,       eIN_NUM,     /// '1'
	eTOKEN_NULL,       eIN_NUM,     /// '2'
	eTOKEN_NULL,       eIN_NUM,     /// '3'
	eTOKEN_NULL,       eIN_NUM,     /// '4'
	eTOKEN_NULL,       eIN_NUM,     /// '5'
	eTOKEN_NULL,       eIN_NUM,     /// '6'
	eTOKEN_NULL,       eIN_NUM,     /// '7'
	eTOKEN_NULL,       eIN_NUM,     /// '8'
	eTOKEN_NULL,       eIN_NUM,     /// '9'
	eTOKEN_COLON,      eIN_NULL,    /// ':'
	eTOKEN_SEMICOLON,  eIN_NULL,    /// ';'
	eTOKEN_LANGLE,     eIN_NULL,    /// '<'
	eTOKEN_ASSIGN,     eIN_NULL,    /// '='
	eTOKEN_RANGLE,     eIN_NULL,    /// '>'
	eTOKEN_QUESTION,   eIN_NULL,    /// '?'
	eTOKEN_AT,         eIN_NULL,    /// '@'
	eTOKEN_NULL,       eIN_VAR,     /// 'A'
	eTOKEN_NULL,       eIN_VAR,     /// 'B'
	eTOKEN_NULL,       eIN_VAR,     /// 'C'
	eTOKEN_NULL,       eIN_VAR,     /// 'D'
	eTOKEN_NULL,       eIN_VAR,     /// 'E'
	eTOKEN_NULL,       eIN_VAR,     /// 'F'
	eTOKEN_NULL,       eIN_VAR,     /// 'G'
	eTOKEN_NULL,       eIN_VAR,     /// 'H'
	eTOKEN_NULL,       eIN_VAR,     /// 'I'
	eTOKEN_NULL,       eIN_VAR,     /// 'J'
	eTOKEN_NULL,       eIN_VAR,     /// 'K'
	eTOKEN_NULL,       eIN_VAR,     /// 'L'
	eTOKEN_NULL,       eIN_VAR,     /// 'M'
	eTOKEN_NULL,       eIN_VAR,     /// 'N'
	eTOKEN_NULL,       eIN_VAR,     /// 'O'
	eTOKEN_NULL,       eIN_VAR,     /// 'P'
	eTOKEN_NULL,       eIN_VAR,     /// 'Q'
	eTOKEN_NULL,       eIN_VAR,     /// 'R'
	eTOKEN_NULL,       eIN_VAR,     /// 'S'
	eTOKEN_NULL,       eIN_VAR,     /// 'T'
	eTOKEN_NULL,       eIN_VAR,     /// 'U'
	eTOKEN_NULL,       eIN_VAR,     /// 'V'
	eTOKEN_NULL,       eIN_VAR,     /// 'W'
	eTOKEN_NULL,       eIN_VAR,     /// 'X'
	eTOKEN_NULL,       eIN_VAR,     /// 'Y'
	eTOKEN_NULL,       eIN_VAR,     /// 'Z'
	eTOKEN_LSQUARE,    eIN_NULL,    /// '['
	eTOKEN_NULL,       eIN_NULL,    /// '\\'
	eTOKEN_RSQUARE,    eIN_NULL,    /// ']'
	eTOKEN_XOR,        eIN_NULL,    /// '^'
	eTOKEN_NULL,       eIN_VAR,     /// '_'
	eTOKEN_NULL,       eIN_NULL,    /// '`'
	eTOKEN_NULL,       eIN_VAR,     /// 'a'
	eTOKEN_NULL,       eIN_VAR,     /// 'b'
	eTOKEN_NULL,       eIN_VAR,     /// 'c'
	eTOKEN_NULL,       eIN_VAR,     /// 'd'
	eTOKEN_NULL,       eIN_VAR,     /// 'e'
	eTOKEN_NULL,       eIN_VAR,     /// 'f'
	eTOKEN_NULL,       eIN_VAR,     /// 'g'
	eTOKEN_NULL,       eIN_VAR,     /// 'h'
	eTOKEN_NULL,       eIN_VAR,     /// 'i'
	eTOKEN_NULL,       eIN_VAR,     /// 'j'
	eTOKEN_NULL,       eIN_VAR,     /// 'k'
	eTOKEN_NULL,       eIN_VAR,     /// 'l'
	eTOKEN_NULL,       eIN_VAR,     /// 'm'
	eTOKEN_NULL,       eIN_VAR,     /// 'n'
	eTOKEN_NULL,       eIN_VAR,     /// 'o'
	eTOKEN_NULL,       eIN_VAR,     /// 'p'
	eTOKEN_NULL,       eIN_VAR,     /// 'q'
	eTOKEN_NULL,       eIN_VAR,     /// 'r'
	eTOKEN_NULL,       eIN_VAR,     /// 's'
	eTOKEN_NULL,       eIN_VAR,     /// 't'
	eTOKEN_NULL,       eIN_VAR,     /// 'uint'
	eTOKEN_NULL,       eIN_VAR,     /// 'v'
	eTOKEN_NULL,       eIN_VAR,     /// 'w'
	eTOKEN_NULL,       eIN_VAR,     /// 'x'
	eTOKEN_NULL,       eIN_VAR,     /// 'y'
	eTOKEN_NULL,       eIN_VAR,     /// 'z'
	eTOKEN_LCURLY,     eIN_NULL,    /// '{'
	eTOKEN_OR,         eIN_NULL,    /// '|'
	eTOKEN_RCURLY,     eIN_NULL,    /// '}'
	eTOKEN_TILD,       eIN_NULL,    /// '~'
	eTOKEN_NULL,       eIN_NULL     /// 0x7f
};

cLexer::cLexer()
{
	mpBuffer = 0;
	mBufferLen = 0;
	mPos = 0;
	mLine = 1;
}

cLexer::cLexer( cMemFileToRead* pfile )
{
	assert( pfile );
	mpBuffer = ( const char* )pfile->GetBufferPtr();
	mBufferLen = pfile->GetSize();
	mPos = 0;
	mLine = 1;
}

cLexer::cLexer( const char* pbuffer, unsigned int size )
{
	assert( pbuffer );
	mpBuffer = pbuffer;
	mBufferLen = size;
	mPos = 0;
	mLine = 1;
}

cLexer::~cLexer()
{
}

void cLexer::SetSource( cMemFileToRead* pfile )
{
	assert( pfile );
	mpBuffer = ( const char* )pfile->GetBufferPtr();
	mBufferLen = pfile->GetSize();
	mPos = 0;
	mLine = 1;
}

void cLexer::SetSource( const char* pbuffer, unsigned int size )
{
	assert( pbuffer );
	mpBuffer = pbuffer;
	mBufferLen = size;
	mPos = 0;
	mLine = 1;
}

int cLexer::GetNextToken( cToken* ptoken )
{
	int type = GetNextString( ptoken );

	if( type == eTOKEN_VAR )
	{
		type = GetTokenType( *ptoken );
		ptoken->mType = type;
	}
	return type;
}

int cLexer::GetNextString( cToken* ptoken )
{
	char lexeme[512] = { 0, };
	int state = eIN_START;
	int type = eTOKEN_NULL;
	int i = 0;
	char c;
	bool save;

	while( state != eIN_DONE )
	{
		c = GetNextChar();
		save = false;

		if( c == 0 )
		{
			/// 토큰 설정
			lexeme[i] = 0;
			//lexeme[255] = 0;
			*ptoken = lexeme;
			ptoken->mType = type;
			ptoken->mLine = mLine;
			return type;
		}

		switch( state )
		{
		case eIN_START:
			if( gLexTable[c].mType != eTOKEN_NULL )
			{
				save = true;
				state = eIN_DONE;
				type = gLexTable[c].mType;
			}
			else if( gLexTable[c].mState != eIN_NULL )
			{
				state = gLexTable[c].mState;
				if( state != eIN_STRING )
					save = true;
			}
			i = 0;
			break;
		case eIN_NUM:	
			if( isdigit( c ) )		
			{
				save = true;
				if( type == eTOKEN_NULL )
					type = eTOKEN_INT;
			}
			else if( c == '.' )
			{
				save = true;
				state = eIN_FLOAT;
				type = eTOKEN_FLOAT;
			}
			else
			{
				UngetNextChar();
				state = eIN_DONE;
				type = eTOKEN_INT;
			}
			break;

		case eIN_FLOAT:
			if( isdigit( c ) )
			{
				save = true;
			}
			else
			{
				UngetNextChar();
				state = eIN_DONE;
				type = eTOKEN_FLOAT;
			}
			break;
		case eIN_VAR:
			if( isalpha( c ) || isdigit( c ) || c == '_' || c == '#')
			{
				save = true;
			}
			else				
			{	
				UngetNextChar();
				state = eIN_DONE;
				type = eTOKEN_VAR;
			}
			break;
		case eIN_STRING:
			if( c != '"' )
			{
				save = true;
			}
			else
			{
				state = eIN_DONE;
				type = eTOKEN_STR;
			}
			break;
		case eIN_MINUS:
			if( isdigit( c ) )
			{
				save = true;
				state = eIN_NUM;
			}
			else if( c == '-' )
			{
				state = eIN_COMMENT;
			}
			break;
		case eIN_DIV:
			if( c == '/' )
			{
				state = eIN_COMMENT;
			}
			else
			{
				save = true;
				state = eIN_DONE;
			}
			break;
		case eIN_COMMENT:
			if( c == '\n' )
			{
				state = eIN_START;
			}
			break;

		default:
			save = true;
			state = eIN_DONE;
			type = eTOKEN_ERROR;
			break;
		}

		if( save && i < 511 )
		{
			lexeme[i] = c;
			++i;
			//lexeme[++i] = 0;
		}
	}

	/// 토큰 설정
	lexeme[i] = 0;
	*ptoken = lexeme;
	ptoken->mType = type;
	ptoken->mLine = mLine;
	return type;
}

int cLexer::GetTokenType( const cString& keyword )
{
	cKeywordMap::cIterator i = mKeywordMap.Find( keyword );

	if( i != mKeywordMap.End() )
	{
		return i->mSecond;
	}
	return eTOKEN_VAR;
}

bool cLexer::IsEnd()
{
	/// 마지막 리턴에 대한 보정..
	while( mPos < mBufferLen )
	{
		char c = *( mpBuffer + mPos );
		if( c == '\r' )
		{
			mPos++;
		}
		else if( c == ' ' )
		{
			mPos++;
		}
		else if( c == '\n' )
		{
			mPos++;
			mLine++;
		}
		else if( c == '	' )
		{
			mPos++;
		}
		else
		{
			break;
		}
	}

	return ( mPos >= mBufferLen );
}

char cLexer::GetNextChar()
{
	if( mPos < mBufferLen )
	{
		char c = *( mpBuffer + mPos );
		if( c == '\n' )
		{
			++mLine;
		}
		++mPos;
		return c;
	}
	return 0;
}

void cLexer::UngetNextChar()
{
	--mPos;
	if( *( mpBuffer + mPos ) == '\n' )
	{
		--mLine;
	}
}

void cLexer::BindKeyword( const cString& keyword, int tokenType )
{
	mKeywordMap.Insert( keyword, tokenType );
}

//----------------------------------------------------
//////////////////////////////////////////////////////////////////////////
cLexerW::cLexerW()
{
	mpBuffer = 0;
	mBufferLen = 0;
	mPos = 0;
	mLine = 1;
}

cLexerW::cLexerW( cMemFileToReadW* pfile )
{
	assert( pfile );
	mpBuffer = ( const wchar_t* )pfile->GetBufferPtr();
	mBufferLen = pfile->GetSize();
	mPos = 0;
	mLine = 1;
}

cLexerW::cLexerW( const wchar_t* pbuffer, unsigned int size )
{
	assert( pbuffer );
	mpBuffer = pbuffer;
	mBufferLen = size;
	mPos = 0;
	mLine = 1;
}

cLexerW::~cLexerW()
{
}

void cLexerW::SetSource( cMemFileToReadW* pfile )
{
	assert( pfile );
	mpBuffer = ( const wchar_t* )pfile->GetBufferPtr();
	mBufferLen = pfile->GetSize();
	mPos = 0;
	mLine = 1;
}

void cLexerW::SetSource( const wchar_t* pbuffer, unsigned int size )
{
	assert( pbuffer );
	mpBuffer = pbuffer;
	mBufferLen = size;
	mPos = 0;
	mLine = 1;
}

int cLexerW::GetNextToken( cTokenW* ptoken )
{
	int type = GetNextString( ptoken );

	if( type == eTOKEN_VAR )
	{
		type = GetTokenType( *ptoken );
		ptoken->mType = type;
	}
	return type;
}

int cLexerW::GetNextString( cTokenW* ptoken )
{
	wchar_t lexeme[512] = { 0, };
	int state = eIN_START;
	int type = eTOKEN_NULL;
	int i = 0;
	wchar_t c;
	bool save;

	while( state != eIN_DONE )
	{
		c = GetNextChar();
		save = false;

		if( c == 0 || c == 65279 )
		{
			/// 토큰 설정
			lexeme[i] = 0;
			*ptoken = lexeme;
			ptoken->mType = type;
			ptoken->mLine = mLine;
			return type;
		}

		switch( state )
		{
		case eIN_START:
			if( gLexTable[c].mType != eTOKEN_NULL )
			{
				save = true;
				state = eIN_DONE;
				type = gLexTable[c].mType;
			}
			else if( gLexTable[c].mState != eIN_NULL )
			{
				state = gLexTable[c].mState;
				if( state != eIN_STRING )
					save = true;
			}
			i = 0;
			break;
		case eIN_NUM:	
			if( isdigit( c ) )		
			{
				save = true;
				if( type == eTOKEN_NULL )
					type = eTOKEN_INT;
			}
			else if( c == L'.' )
			{
				save = true;
				state = eIN_FLOAT;
				type = eTOKEN_FLOAT;
			}
			else
			{
				UngetNextChar();
				state = eIN_DONE;
				type = eTOKEN_INT;
			}
			break;

		case eIN_FLOAT:
			if( isdigit( c ) )
			{
				save = true;
			}
			else
			{
				UngetNextChar();
				state = eIN_DONE;
				type = eTOKEN_FLOAT;
			}
			break;
		case eIN_VAR:
			if( isalpha( c ) || isdigit( c ) || c == L'_' || c == L'#' )
			{
				save = true;
			}
			else				
			{	
				UngetNextChar();
				state = eIN_DONE;
				type = eTOKEN_VAR;
			}
			break;
		case eIN_STRING:
			if( c != L'"' )
			{
				save = true;
			}
			else
			{
				state = eIN_DONE;
				type = eTOKEN_STR;
			}
			break;
		case eIN_MINUS:
			if( isdigit( c ) )
			{
				save = true;
				state = eIN_NUM;
			}
			else if( c == L'-' )
			{
				state = eIN_COMMENT;
			}
			break;
		case eIN_DIV:
			if( c == L'/' )
			{
				state = eIN_COMMENT;
			}
			else
			{
				save = true;
				state = eIN_DONE;
			}
			break;
		case eIN_COMMENT:
			if( c == L'\n' )
			{
				state = eIN_START;
			}
			break;

		default:
			save = true;
			state = eIN_DONE;
			type = eTOKEN_ERROR;
			break;
		}

		if( save && i < 511 )
		{
			lexeme[i] = c;
			++i;
			//lexeme[++i] = 0;
		}
	}

	/// 토큰 설정
	lexeme[i] = 0;
	*ptoken = lexeme;
	ptoken->mType = type;
	ptoken->mLine = mLine;
	return type;
}

int cLexerW::GetTokenType( const cStringW& keyword )
{
	cKeywordMap::cIterator i = mKeywordMap.Find( keyword );

	if( i != mKeywordMap.End() )
	{
		return i->mSecond;
	}
	return eTOKEN_VAR;
}

bool cLexerW::IsEnd()
{
	/// 마지막 리턴에 대한 보정..
	while( mPos < mBufferLen )
	{
		wchar_t c = *( mpBuffer + mPos );
		if( c == L'\r' )
		{
			mPos++;
		}
		else if( c == L' ' )
		{
			mPos++;
		}
		else if( c == L'\n' )
		{
			mPos++;
			mLine++;
		}
		else if( c == L'	' )
		{
			mPos++;
		}
		else
		{
			break;
		}
	}

	return ( mPos >= mBufferLen );
}

wchar_t cLexerW::GetNextChar()
{
	if( mPos < mBufferLen )
	{
		wchar_t c = *( mpBuffer + mPos );
		if( c == L'\n' )
		{
			++mLine;
		}
		++mPos;
		return c;
	}
	return 0;
}

void cLexerW::UngetNextChar()
{
	--mPos;
	if( *( mpBuffer + mPos ) == L'\n' )
	{
		--mLine;
	}
}

void cLexerW::BindKeyword( const cStringW& keyword, int tokenType )
{
	mKeywordMap.Insert( keyword, tokenType );
}