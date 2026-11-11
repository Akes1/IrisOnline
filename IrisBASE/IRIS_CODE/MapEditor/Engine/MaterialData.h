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
	cMaterialData( const NiColor& ambient, const NiColor& diffuse, const NiColor& specular, const NiColor& emissive, float shininess, float alpha, NiMaterialProperty* prop );

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

	/// 재질 속성
	NiMaterialProperty* mProp;
};
#pragma pack( pop )

inline
cMaterialData::cMaterialData()
: mAmbient( 0.5f, 0.5f, 0.5f )
, mDiffuse( NiColor::WHITE )
, mSpecular( NiColor::BLACK )
, mEmissive( NiColor::BLACK )
, mShininess( 1.0f )
, mAlpha( 1.0f )
, mProp( 0 )
{
}

inline
cMaterialData::cMaterialData( const NiColor& ambient, const NiColor& diffuse, const NiColor& specular, const NiColor& emissive, float shininess, float alpha, NiMaterialProperty* prop )
: mAmbient( ambient )
, mDiffuse( diffuse )
, mSpecular( specular )
, mEmissive( emissive )
, mShininess( shininess )
, mAlpha( alpha )
, mProp( prop )
{
}

#pragma pack( push, 1 )
/// 재질
class cMaterialData_Ver9
{
public:
	cMaterialData_Ver9();

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

inline
cMaterialData_Ver9::cMaterialData_Ver9()
: mAmbient( 0.5f, 0.5f, 0.5f )
, mDiffuse( NiColor::WHITE )
, mSpecular( NiColor::BLACK )
, mEmissive( NiColor::BLACK )
, mShininess( 1.0f )
, mAlpha( 1.0f )
{
}
