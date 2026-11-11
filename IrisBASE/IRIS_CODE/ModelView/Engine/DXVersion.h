#pragma once

class cDXVersion
{
public:
	HRESULT GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion );

protected:

	HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
	HRESULT GetDirectXVersionViaFileVersions( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );
	HRESULT GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion );
	ULARGE_INTEGER MakeInt64( WORD a, WORD b, WORD c, WORD d );
	int CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 );
};
