#pragma once

#include "Lexer.h"

enum
{
	eTOKEN_LOCAL = 100,
};

class cPatchLexer : public cLexer
{
public:
    cPatchLexer( const char* pbuffer, unsigned int size );
};


enum
{
	eTOKEN_WINMODE = 100,
	eTOKEN_RESOLUTION,
	eTOKEN_RAMP,
	eTOKEN_ENVVOLUME,
	eTOKEN_EFFVOLUME,
	eTOKEN_VIDEOMODE,
	eTOKEN_LANGUAGE,
};

class cOptionLexer : public cLexer
{
public:
    cOptionLexer( const char* pbuffer, unsigned int size );
};