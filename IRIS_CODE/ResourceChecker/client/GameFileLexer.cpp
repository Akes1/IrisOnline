#include "StdAfx.h"
#include ".\gamefilelexer.h"

cGameFileLexer::cGameFileLexer( const char* pbuffer, unsigned int size )
: cLexer( pbuffer, size )
{
	BindKeyword( "jobinfo", eTOKEN_JOBINFO );
	BindKeyword( "jobtree", eTOKEN_JOBTREE );

	BindKeyword( "hair", eTOKEN_HAIR );
	BindKeyword( "haircolor", eTOKEN_HAIRCOLOR );
}
