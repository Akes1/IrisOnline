#include "stdafx.h"
#include "Console.h"
#include "File.h"
#include <stdio.h>
#include <stdarg.h>

/* --------------------------------------------------------------------------
* 내    용 : 
* 주의사항 : 
*---------------------------------------------------------------------------*/
void cConsole::PrintLine()
{
	printf( "-----------------------------------------------------------\n" );
}

/* --------------------------------------------------------------------------
* 내    용 : 
* 주의사항 : 
*---------------------------------------------------------------------------*/
void cConsole::Print( const char* pfmt, ... )
{
	if( pfmt == 0 )
		return;

	va_list vl;
	va_start( vl, pfmt );
	vprintf( pfmt, vl );
	va_end( vl );
	printf( "\n" );
}

/* --------------------------------------------------------------------------
* 내    용 : 
* 주의사항 : 
*---------------------------------------------------------------------------*/
void cConsole::Error( const char* pfile, int line, const char* pfmt, ... )
{
	if( pfmt == 0 )
		return;

	cString fileName;
	::GetFileName( &fileName, pfile );
	printf( "Error: %s( %d ): ", fileName, line );
	va_list vl;
	va_start( vl, pfmt );
	vprintf( pfmt, vl );
	va_end( vl );
	printf( "\n" );
}
