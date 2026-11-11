/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.21
* 내    용 : 어휘 분석기
* 주의사항 : 
*===========================================================================*/
#pragma once

class cToken;
class cMemFileToRead;

class cTokenW;
class cMemFileToReadW;

/// 초간단 어휘 분석기
/// 텍스트 파일을 위한 어휘 분석기이다.
class cLexer
{
public:
	cLexer();
	cLexer( cMemFileToRead* pfile );
	cLexer( const char* pbuffer, unsigned int size );
	virtual ~cLexer();

	/// 소스 설정
	virtual void SetSource( cMemFileToRead* pfile );
	virtual void SetSource( const char* pbuffer, unsigned int size );

	/// 다음 토큰 리턴
	/// 인자로 넘긴 토큰에 값( 타입, 줄번호 등 )을 설정하고, 토큰의 타입을 리턴한다.
	virtual int GetNextToken( cToken* ptoken );

	/// 다음 토큰 문자열 리턴
	/// 예약어 검사는 생략하고 바로 문자열만 리턴한다.
	virtual int GetNextString( cToken* ptoken );

	/// 키워드의 토큰 타입을 리턴
	/// 키워드가 아니면 TOKEN_VAR를 리턴한다.
	virtual int GetTokenType( const cString& keyword );

	/// 종료 여부를 리턴
	virtual bool IsEnd();

protected:
	/// 한 글자를 리턴하고 다음 위치로 이동
	char GetNextChar();

	/// 한 글자를 물림
	void UngetNextChar();

	/// 키워드에 토큰 타입을 연결
	void BindKeyword( const cString& keyword, int tokenType );

protected:
	/// 버퍼 포인터
	const char* mpBuffer;

	/// 버퍼 길이
	unsigned int mBufferLen;

	/// 문자를 읽을 위치
	unsigned int mPos;

	/// 줄 번호
	int mLine;

	/// 키워드 맵
	typedef tHashMap<cString, int> cKeywordMap;
	cKeywordMap mKeywordMap;
};


//---------------------------------------------------------------------
class cLexerW
{
public:
	cLexerW();
	cLexerW( cMemFileToReadW* pfile );
	cLexerW( const wchar_t* pbuffer, unsigned int size );
	virtual ~cLexerW();

	/// 소스 설정
	virtual void SetSource( cMemFileToReadW* pfile );
	virtual void SetSource( const wchar_t* pbuffer, unsigned int size );

	/// 다음 토큰 리턴
	/// 인자로 넘긴 토큰에 값( 타입, 줄번호 등 )을 설정하고, 토큰의 타입을 리턴한다.
	virtual int GetNextToken( cTokenW* ptoken );

	/// 다음 토큰 문자열 리턴
	/// 예약어 검사는 생략하고 바로 문자열만 리턴한다.
	virtual int GetNextString( cTokenW* ptoken );

	/// 키워드의 토큰 타입을 리턴
	/// 키워드가 아니면 TOKEN_VAR를 리턴한다.
	virtual int GetTokenType( const cStringW& keyword );

	/// 종료 여부를 리턴
	virtual bool IsEnd();

protected:
	/// 한 글자를 리턴하고 다음 위치로 이동
	wchar_t GetNextChar();

	/// 한 글자를 물림
	void UngetNextChar();

	/// 키워드에 토큰 타입을 연결
	void BindKeyword( const cStringW& keyword, int tokenType );

protected:
	/// 버퍼 포인터
	const wchar_t* mpBuffer;

	/// 버퍼 길이
	unsigned int mBufferLen;

	/// 문자를 읽을 위치
	unsigned int mPos;

	/// 줄 번호
	int mLine;

	/// 키워드 맵
	typedef tHashMap<cStringW, int> cKeywordMap;
	cKeywordMap mKeywordMap;
};
