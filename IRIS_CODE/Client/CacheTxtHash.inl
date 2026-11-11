
template <class TVAL> inline
cCaheTxtHash<TVAL>::cCaheTxtHash( unsigned int hashSize ) 
	: cCacheTempHash<NiTPointerMap<sCacheKey*, TVAL>, TVAL>(hashSize)
{
}

template <class TPARENT, class TVAL> inline
cCacheTempHash<TPARENT, TVAL>::cCacheTempHash( unsigned int hashSize ) 
	: TPARENT( hashSize )
{
} 

template <class TPARENT, class TVAL> inline
cCacheTempHash<TPARENT, TVAL>::~cCacheTempHash()
{
}

template <class TPARENT, class TVAL> inline
unsigned int cCacheTempHash<TPARENT, TVAL>::KeyToHashIndex(sCacheKey* pcKey) const
{
	unsigned int uiHash = 0;

	LPCTSTR str = pcKey->szString;

	for(unsigned int i=0;i<pcKey->strSize;i++)
	{
		uiHash = (uiHash << 5) + uiHash + *str++;
	}

	return uiHash % TPARENT::m_uiHashSize;
}

template <class TPARENT, class TVAL> inline
bool cCacheTempHash<TPARENT, TVAL>::IsKeysEqual(sCacheKey* pcKey1, sCacheKey* pcKey2) const
{
	if( pcKey1->strSize != pcKey2->strSize )
		return false;

	if( pcKey1->hFont != pcKey2->hFont )
		return false;

	if ( pcKey1->color != pcKey2->color )
		return false;

//	if( ::memcmp( pcKey1->szString, pcKey2->szString, pcKey2->strSize*sizeof(TCHAR) ) )
	if( ::_tcscmp(pcKey1->szString, pcKey2->szString) )
		return false;

	return true;
}