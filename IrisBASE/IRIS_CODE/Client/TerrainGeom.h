/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.12.17
* 내    용 : 지형 기하
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 지형 기하 데이타
class cTerrainStripData : public NiTriStripsData
{
public:
	enum eConsistency
	{
		MUTABLE		= 0,
		STATIC		= 1,
		VOLATILE	= 2,
	};

public:
	cTerrainStripData( unsigned short numVerts,
		NiPoint3* posCoords, NiPoint3* alphas, NiColorA* colors, NiPoint2* texCoords,
		unsigned short numTris, unsigned short numStrips,
		unsigned short* stripLength, unsigned short* stripIndex,
		const NiPoint3& center, float radius,
		eConsistency consistency = STATIC );

	~cTerrainStripData();
};

/// 지형 기하
class cTerrainStrip : public NiTriStrips
{
public:
	cTerrainStrip( unsigned short numVerts,
		NiPoint3* posCoords, NiPoint3* alphas, NiColorA* colors, NiPoint2* texCoords,
		unsigned short numTris, unsigned short numStrips,
		unsigned short* stripLength, unsigned short* stripIndex,
		const NiPoint3& center, float radius,
		cTerrainStripData::eConsistency consistency = cTerrainStripData::STATIC );

	void UpdateBound( const NiPoint3& center, float radius );
};

typedef NiPointer<cTerrainStrip> cTerrainStripPtr;
