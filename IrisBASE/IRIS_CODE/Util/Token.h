/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.21
* 내    용 : 토큰
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 토큰 타입
enum
{
	eTOKEN_ERROR = -1, /// 에러
	eTOKEN_NULL = 0,   /// 널
	eTOKEN_INT,        /// 정수
	eTOKEN_FLOAT,      /// 실수
	eTOKEN_VAR,        /// 변수
	eTOKEN_STR,        /// 문자열
	eTOKEN_NOT,        /// !
	eTOKEN_SHARP,      /// #
	eTOKEN_DOLLAR,     /// $
	eTOKEN_PERCENT,    /// %
	eTOKEN_AND,        /// &
	eTOKEN_LPAREN,     /// ( 
	eTOKEN_RPAREN,     /// )
	eTOKEN_MUL,        /// *
	eTOKEN_PLUS,       /// +
	eTOKEN_COMMA,      /// ,
	eTOKEN_DOT,        /// .
	eTOKEN_DIV,        /// /
	eTOKEN_COLON,      /// :
	eTOKEN_SEMICOLON,  /// ;
	eTOKEN_LANGLE,     /// <
	eTOKEN_ASSIGN,     /// =
	eTOKEN_RANGLE,     /// >
	eTOKEN_QUESTION,   /// ?
	eTOKEN_AT,         /// @
	eTOKEN_LSQUARE,    /// [
	eTOKEN_RSQUARE,    /// ]
	eTOKEN_XOR,        /// ^
	eTOKEN_LCURLY,     /// {
	eTOKEN_OR,         /// |
	eTOKEN_RCURLY,     /// }
	eTOKEN_TILD,       /// ~
};

/// 토큰
class cToken : public cString
{
public:
	cToken();
	explicit cToken( int type );
	cToken( int type, const char* str );

	/// 대입 연산자
	void operator = ( const cString& str );
	void operator = ( const char* str );

public:
	/// 타입
	int mType;

	/// 줄번호
	int mLine;
};

inline cToken::cToken()
: mType( eTOKEN_NULL )
, mLine( 0 )
{
}

inline cToken::cToken( int type )
: mType( type )
, mLine( 0 )
{
}

inline cToken::cToken( int type, const char* str )
: cString( str )
, mType( type )
, mLine( 0 )
{
}

inline void cToken::operator = ( const cString& str )
{
	*static_cast<cString*>( this ) = str;
}

inline void cToken::operator = ( const char* str )
{
	*static_cast<cString*>( this ) = str;
}

//--------------------------------------------------------
/// 토큰
class cTokenW : public cStringW
{
public:
	cTokenW();
	explicit cTokenW( int type );
	cTokenW( int type, const wchar_t* str );

	/// 대입 연산자
	void operator = ( const cStringW& str );
	void operator = ( const wchar_t* str );

public:
	/// 타입
	int mType;

	/// 줄번호
	int mLine;
};

inline cTokenW::cTokenW()
: mType( eTOKEN_NULL )
, mLine( 0 )
{
}

inline cTokenW::cTokenW( int type )
: mType( type )
, mLine( 0 )
{
}

inline cTokenW::cTokenW( int type, const wchar_t* str )
: cStringW( str )
, mType( type )
, mLine( 0 )
{
}

inline void cTokenW::operator = ( const cStringW& str )
{
	*static_cast<cStringW*>( this ) = str;
}

inline void cTokenW::operator = ( const wchar_t* str )
{
	*static_cast<cStringW*>( this ) = str;
}
