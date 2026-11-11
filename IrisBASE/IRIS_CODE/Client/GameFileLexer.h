#pragma once

#include "Lexer.h"

enum
{
	/// 게임파일을 읽을때 사용할 토큰..
	eTOKEN_JOBINFO = 100,
	eTOKEN_JOBTREE,
	eTOKEN_HAIR,
	eTOKEN_HAIRCOLOR,
};


class cGameFileLexer : public cLexer
{
public:
	cGameFileLexer( const char* pbuffer, unsigned int size );
};
