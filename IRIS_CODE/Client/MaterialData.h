/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.18
* 내    용 : 재질
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma pack( push, 1 )
/// 재질
class cMaterialData
{
public:
	cMaterialData();

public:
	/// 주변광 반사율
	NiColor mAmbient;

	/// 산란광 반사율
	NiColor mDiffuse;

	/// 반영광 반사율
	NiColor mSpecular;

	/// 발산색
	NiColor mEmissive;

	/// 광택도
	float mShininess;

	/// 불투명도
	float mAlpha;
};
#pragma pack( pop )

inline cMaterialData::cMaterialData()
: mAmbient( 0.5f, 0.5f, 0.5f )
, mDiffuse( 0.5f, 0.5f, 0.5f )
, mSpecular( 0.f, 0.f, 0.f )
, mEmissive( 0.5f, 0.5f, 0.5f )
, mShininess( 4.0f )
, mAlpha( 1.0f )
{
}
