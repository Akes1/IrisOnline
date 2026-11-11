// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// ResourceChecker.pch는 미리 컴파일된 헤더가 됩니다.
// stdafx.obj에는 미리 컴파일된 형식 정보가 포함됩니다.

#include "stdafx.h"


int ConvertToUnicode( const char* str, wchar_t* uniStr, unsigned int buffersize )
{
	size_t len = ::strlen(str);
	if( len == 0 )
		return 0;

	int iLen = ::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, str, len, NULL, 0 );
	if (iLen > 0)
	{
		assert( iLen+1 <= (int)buffersize );
		iLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, (LPWSTR)uniStr, iLen+1 );
		assert(iLen != 0);
	}
	else
	{
		iLen --; // return -1
	}
	return iLen;
}

int ConvertToAscii( const wchar_t* uniStr, char* str, unsigned int buffersize )
{
	size_t wideLen = ::wcslen(uniStr);
	if (wideLen == 0)
		return 0;

	// iLen does not include terminating character
	int iLen = ::WideCharToMultiByte(CP_ACP, 0, uniStr, (int)wideLen, (LPSTR)str, 0, NULL, NULL);
	if (iLen > 0)
	{
		assert( iLen+1 <= (int)buffersize );
		iLen = ::WideCharToMultiByte(CP_ACP, 0, uniStr , (int)-1, (LPSTR)str, iLen+1, NULL, NULL);
		assert(iLen != 0);
	}
	else
	{
		iLen --;
	}
	return iLen;
}

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

void GameErrorLog( char* format, ... )
{
#ifdef _DEVSYS
	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;

	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) + 1;
	msgBuf       = malloc( bufferLength );

	vsprintf_s( (char*)msgBuf, bufferLength, format, args );

	va_end( args );

	if ( msgBuf != NULL )
	{
		FILE*      stream = NULL;
		char       filename[ MAX_PATH ];
		SYSTEMTIME systemtime;
		char       buffer[ 1024 ];

		GetLocalTime( &systemtime );

		sprintf_s( filename,
			"GameLog_%04d_%02d_%02d.log",
			systemtime.wYear,
			systemtime.wMonth,
			systemtime.wDay );

		if ( fopen_s( &stream, filename, "at" ) != 0 )
		{
			sprintf_s( buffer,
				"%04d %02d %02d::%02d %02d %02d : %s\n",
				systemtime.wYear,
				systemtime.wMonth,
				systemtime.wDay,
				systemtime.wHour,
				systemtime.wMinute,
				systemtime.wSecond,
				(char*)msgBuf );

			fputs( buffer, stream );
			fclose( stream );
		}

		free( msgBuf );
	}
#endif
}