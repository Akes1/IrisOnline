#include "StdAfx.h"
#include "cPatchLexer.h"

cPatchLexer::cPatchLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "local",   eTOKEN_LOCAL    );
}

cOptionLexer::cOptionLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "windowed", eTOKEN_WINMODE );
	BindKeyword( "resolution", eTOKEN_RESOLUTION );
	BindKeyword( "ramp", eTOKEN_RAMP );
	BindKeyword( "envvolume", eTOKEN_ENVVOLUME );
	BindKeyword( "effvolume", eTOKEN_EFFVOLUME );
	BindKeyword( "video", eTOKEN_VIDEOMODE );
	BindKeyword( "language", eTOKEN_LANGUAGE );
}