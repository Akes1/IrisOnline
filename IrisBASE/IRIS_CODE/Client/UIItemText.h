/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.08
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Lexer.h"

class cUIItemTextLexer;
class cParser;

/// UI Item 토큰 타입
enum eUIItemTextNodeType
{
	eITEM_TEXTNODE_NULL = 0,
	eITEM_TEXTNODE_STRING,
	eITEM_TEXTNODE_COLOR,
	eITEM_TEXTNODE_VARNAME
};

/// UI Item 문자열 인자
class cUIItemTextParam
{
public:
	cStringT mName;
};

/// UI Item 문자열 노드
#pragma pack( push, 1 )
class cUIItemTextNode
{
	friend class cUIItemText;

public:
	cUIItemTextNode();
	~cUIItemTextNode();

	const cUIItemTextNode* GetNext() const;
	const cUIItemTextNode* Back() const;

public:
	eUIItemTextNodeType mType;

	union
	{
		charT* mpString;
		unsigned long mColor;
	};

private:
	mutable cUIItemTextNode* mpNext;
};
#pragma pack( pop )

inline cUIItemTextNode::cUIItemTextNode()
: mType( eITEM_TEXTNODE_NULL )
, mpNext( 0 )
{
}

inline cUIItemTextNode::~cUIItemTextNode()
{
	if( mType == eITEM_TEXTNODE_STRING )
		delete [] mpString;

	if( mpNext )
		delete mpNext;
}

inline const cUIItemTextNode* cUIItemTextNode::GetNext() const
{
	return mpNext;
}

inline const cUIItemTextNode* cUIItemTextNode::Back() const
{
	if( mpNext )
		return mpNext->Back();
	else
		return this;
}

/// UI Item 문자열
class cUIItemText
{
public:
	cUIItemText();
	~cUIItemText();

	void Clear();

	/// 로딩
	bool Load( const cString& pathName );

	/// 텍스트 노드
	const cUIItemTextNode* GetNode( unsigned int id ) const;

private:
	cUIItemTextNode* NewNode( unsigned int id );
	void PushStringNode( unsigned int id, const cString& str );
	void PushColorNode( unsigned int id, unsigned long color );
	void PushVarNameNode( unsigned int id );

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

	/// 변수
	eTOKEN_TEXT_VARNAME
};

/// UI 문자열 어휘 분석기
class cUIItemTextLexer : public cLexer
{
public:
	cUIItemTextLexer( const char* buffer, unsigned int size );
};
