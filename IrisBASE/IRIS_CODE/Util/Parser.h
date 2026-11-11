/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.21
* 
* 내    용 : 파서
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 텍스트 파일을 파싱하기 위해 렉서를 필요로 한다.
*===========================================================================*/
#pragma once

#include "Token.h"

class cLexer;
class cLexerW;


/// 파서
/// 텍스트 파일을 파싱하기 위해 렉서를 필요로 한다.
class cParser
{
public:
	cParser();
	cParser( cLexer* lexer, const cString& fileName );
	virtual ~cParser();

	/// 렉서 설정
	void SetLexer( cLexer* lexer );

	/// 파일 이름 설정
	void SetFileName( const cString& fileName );

	/// 예상한 토큰이 맞는 지를 리턴
	bool ExpectTokenString( const char* str );
	bool ExpectTokenType( int type );

	/// 문자열 파싱
	cString ParseString();
	bool ParseString( cString* str );

	/// 정수 파싱
	int ParseInt();
	bool ParseInt( int* out );

	/// 실수 파싱
	float ParseFloat();
	bool ParseFloat( float* out );

	/// 렉서를 리턴
	cLexer* GetLexer() const;

	/// 파일 이름을 리턴: 웅주
	const char* GetFileName() const;

	/// 줄 번호를 리턴
	int GetLineNum() const;

protected:
	/// 렉서
	cLexer* mpLexer;

	/// 파싱중인 파일 이름
	cString mFileName;

	/// 토큰
	cToken mToken;
};

inline void cParser::SetLexer( cLexer* lexer )
{
	mpLexer = lexer;
}

inline void cParser::SetFileName( const cString& fileName )
{
	mFileName = fileName;
}

inline cLexer* cParser::GetLexer() const
{
	return mpLexer;
}

inline int cParser::GetLineNum() const
{
	return mToken.mLine;
}


inline const char* cParser::GetFileName() const
{
	return mFileName.Cstr();
}

//---------------------------------------------------------------
class cParserW
{
public:
	cParserW();
	cParserW( cLexerW* lexer, const cStringW& fileName );
	virtual ~cParserW();

	/// 렉서 설정
	void SetLexer( cLexerW* lexer );

	/// 파일 이름 설정
	void SetFileName( const cStringW& fileName );

	/// 예상한 토큰이 맞는 지를 리턴
	bool ExpectTokenString( const wchar_t* str );
	bool ExpectTokenType( int type );

	/// 문자열 파싱
	cStringW ParseString();
	bool ParseString( cStringW* str );

	/// 정수 파싱
	int ParseInt();
	bool ParseInt( int* out );

	/// 실수 파싱
	float ParseFloat();
	bool ParseFloat( float* out );

	/// 렉서를 리턴
	cLexerW* GetLexer() const;

	/// 파일 이름을 리턴: 웅주
	const wchar_t* GetFileName() const;

	/// 줄 번호를 리턴
	int GetLineNum() const;

protected:
	/// 렉서
	cLexerW* mpLexer;

	/// 파싱중인 파일 이름
	cStringW mFileName;

	/// 토큰
	cTokenW mToken;
};

inline void cParserW::SetLexer( cLexerW* lexer )
{
	mpLexer = lexer;
}

inline void cParserW::SetFileName( const cStringW& fileName )
{
	mFileName = fileName;
}

inline cLexerW* cParserW::GetLexer() const
{
	return mpLexer;
}

inline int cParserW::GetLineNum() const
{
	return mToken.mLine;
}

inline const wchar_t* cParserW::GetFileName() const
{
	return mFileName.Cstr();
}