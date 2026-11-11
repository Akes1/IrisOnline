// stdafx.cpp : 표준 포함 파일만 들어 있는 소스 파일입니다.
// DummyClient.pch는 미리 컴파일된 헤더가 됩니다.
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