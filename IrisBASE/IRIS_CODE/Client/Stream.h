/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.04.28
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#include <NiSearchPath.h>


/// 스트림
class cStream : public NiStream
{
public:
	cStream( const cString& pathName, NiTexturePalette* texPalette );
};

inline
cStream::cStream( const cString& pathName, NiTexturePalette* texPalette )
{
	m_pkSearchPath->Reset();
	m_pkSearchPath->SetReferencePath(pathName.Cstr());

	SetTexturePalette( texPalette );
}