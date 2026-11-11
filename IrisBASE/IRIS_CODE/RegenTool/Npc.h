/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.01.17
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

/// Npc 정보
class cNpcInfo
{
public:
	cNpcInfo( unsigned int nameIndex, unsigned int modelIndex );

public:
	/// 이름 인덱스
	unsigned int mNameIndex;

	/// 모델 인덱스
	unsigned int mModelIndex;
};

inline
cNpcInfo::cNpcInfo( unsigned int nameIndex, unsigned int modelIndex )
: mNameIndex( nameIndex )
, mModelIndex( modelIndex )
{
}
