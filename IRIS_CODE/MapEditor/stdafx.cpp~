#include "stdafx.h"


void Sstrncat( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count )
{
	assert( count < destMaxLen -_tcslen(pDest) );
	::_tcsncat_s( pDest, destMaxLen, pSrc, __min(count, destMaxLen -_tcslen(pDest)) );
}

void Sstrncpy( LPTSTR pDest, unsigned int destMaxLen, LPCTSTR pSrc, unsigned int count )
{
	assert( count < destMaxLen );
	unsigned int n = __min(count, destMaxLen);

	::_tcsncpy_s( pDest, destMaxLen, pSrc, n );
	pDest[n] = 0;
}
