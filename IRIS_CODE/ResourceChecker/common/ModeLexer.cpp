

#include "stdafx.h"
#include "ModeLexer.h"

cModeLexer::cModeLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "Header",		eTOKEN_HEADER );
	BindKeyword( "Mode",		eTOKEN_MODE );

	BindKeyword( "effect",		eTOKEN_EFFECT );
	BindKeyword( "sound",		eTOKEN_SOUND );
	BindKeyword( "death",		eTOKEN_DEATHDOOR );
	BindKeyword( "deathmonster",eTOKEN_DEATHMONSTER );
	BindKeyword( "portal",		eTOKEN_PORTAL );

	BindKeyword( "applybuff",	eTOKEN_APPLYBUFF );
	BindKeyword( "applyaura",	eTOKEN_APPLYAURA );
	BindKeyword( "genmonster",	eTOKEN_GENMONSTER );
	BindKeyword( "gengathering",eTOKEN_GENGATHERING );
	BindKeyword( "gentotem",	eTOKEN_GENTOTEM );
	BindKeyword( "speech",		eTOKEN_SPEECH );
	BindKeyword( "modestate",	eTOKEN_MODESTATE );
	BindKeyword( "aggro",		eTOKEN_AGGRO );
	BindKeyword( "applyres",	eTOKEN_APPLYRESOURCE );
	BindKeyword( "modeskill",	eTOKEN_MODEINITSKILL );
	BindKeyword( "modeimage",	eTOKEN_MODEIMAGE );

	BindKeyword( "modechange",	eTOKEN_MODECHANGE );
	BindKeyword( "useskill",	eTOKEN_USESKILL );
	BindKeyword( "immune",		eTOKEN_IMMUNE );
}