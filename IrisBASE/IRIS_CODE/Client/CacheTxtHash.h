#pragma once

struct sCacheKey
{
	HFONT			hFont;
	unsigned long	color;
	unsigned long	strSize;
	TCHAR			szString[1];
};

template <class TPARENT, class TVAL> class cCacheTempHash : 
		public TPARENT
{
public:
	virtual ~cCacheTempHash();

protected:
	cCacheTempHash( unsigned int hashSize = 256 );

	virtual unsigned int KeyToHashIndex( sCacheKey* pcKey ) const;
	virtual bool IsKeysEqual( sCacheKey* pcKey1, sCacheKey* pcKey2 ) const;
};

template <class TVAL> class cCaheTxtHash : 
		public cCacheTempHash<NiTPointerMap<sCacheKey*, TVAL>, TVAL>
{
public:
	cCaheTxtHash( unsigned int hashSize = 256 );
};

#include "CacheTxtHash.inl"