#include "stdafx.h"
#include "DramaturgyLexer.h"

cDramaturgyLexer::cDramaturgyLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "Header",		eTOKEN_HEADER );
	BindKeyword( "Casting",		eTOKEN_CASTING );
	BindKeyword( "Activity",	eTOKEN_ACTIVITY );
	BindKeyword( "Apply",		eTOKEN_APPLY );

	BindKeyword( "type",		eTOKEN_TYPE );
	BindKeyword( "effect",		eTOKEN_EFFECT );
	BindKeyword( "sound",		eTOKEN_SOUND );
	BindKeyword( "trailtex",	eTOKEN_TRAILTEX );

	BindKeyword( "animation",	eTOKEN_ANI );
	BindKeyword( "action",		eTOKEN_ACTION );
	BindKeyword( "actionEx",	eTOKEN_ACTIONEX );
	BindKeyword( "soundplay",	eTOKEN_SOUNDPLAY );
	BindKeyword( "channeling",	eTOKEN_CHANNELING );

	BindKeyword( "active",		eTOKEN_ACTIVE );
	BindKeyword( "bullet",		eTOKEN_BULLET );
	BindKeyword( "crash",		eTOKEN_CRASH );
	BindKeyword( "damage",		eTOKEN_DAMAGE );
	BindKeyword( "trail",		eTOKEN_TRAIL );
	BindKeyword( "shake",		eTOKEN_SHAKE );
	BindKeyword( "fadeout",		eTOKEN_FADEOUT );
}
