/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.08
* 내    용 : 텍스트 라인, 부분 컬러등을 적용할수 잇는 텍스트 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Lexer.h"

/// UI 기능 텍스트 토큰 타입
enum eUIFuncTextNodeType
{
	eTEXTNODE_NULL = 0,
	eTEXTNODE_STRING,
	eTEXTNODE_COLOR,
	eTEXTNODE_VARNAME,
	eTEXTNODE_NEWLINE,
};

/// UI 기능텍스트 문자열 인자
class cUIFuncTextParam
{
public:
	cStringT mName;
};

/// UI 문자열 노드
#pragma pack( push, 1 )
class cUIFuncTextNode
{
	friend class cUIFuncText;

public:
	cUIFuncTextNode();
	~cUIFuncTextNode();

	cUIFuncTextNode* GetNext();
	cUIFuncTextNode* Back();

public:
	eUIFuncTextNodeType mType;

	TCHAR*	mpString;
	unsigned long mColor;
	
private:
	cUIFuncTextNode* mpNext;
};
#pragma pack( pop )

inline cUIFuncTextNode::cUIFuncTextNode()
: mType( eTEXTNODE_NULL )
, mpNext( 0 )
, mpString( 0 )
, mColor( 0 )
{
}

inline cUIFuncTextNode::~cUIFuncTextNode()
{
	if( mType == eTEXTNODE_STRING )
		delete [] mpString;

	if( mpNext )
		delete mpNext;
}

inline cUIFuncTextNode* cUIFuncTextNode::GetNext()
{
	return mpNext;
}

inline cUIFuncTextNode* cUIFuncTextNode::Back()
{
	if( mpNext )
		return mpNext->Back();
	else
		return this;
}

/// UI 기능 문자열
class cUIFuncText
{
public:
	cUIFuncText();
	~cUIFuncText();

	void Clear();

	/// 로딩
	bool Load( const cStringW& pathName );

	/// 텍스트 노드
	cUIFuncTextNode* GetNode( unsigned int id );

private:
	cUIFuncTextNode* NewNode( unsigned int id );
	void PushStringNode( unsigned int id, const cStringW& str );
	void PushColorNode( unsigned int id, unsigned long color );
	void PushVarNameNode( unsigned int id );
	void PushNewLineNode( unsigned int id );

private:
	typedef tHashMap<unsigned int, void*> cNodeMap;
	cNodeMap mNodeMap;
};

/// UI 문자열 토큰 타입
enum
{
	/// 글자색
	eTOKEN_TEXT_BLACK = 100,
	eTOKEN_TEXT_WHITE,
	eTOKEN_TEXT_GRAY,
	eTOKEN_TEXT_RED,
	eTOKEN_TEXT_GREEN,
	eTOKEN_TEXT_BLUE,
	eTOKEN_TEXT_YELLOW,
	eTOKEN_TEXT_BROWN,

	/// 변수
	eTOKEN_TEXT_VARNAME,

	/// 라인넘기기
	eTOKEN_TEXT_NEWLINE,
};

/// UI 문자열 어휘 분석기
class cUIFuncTextLexer : public cLexerW
{
public:
	cUIFuncTextLexer( const wchar_t* buffer, unsigned int size );
};
