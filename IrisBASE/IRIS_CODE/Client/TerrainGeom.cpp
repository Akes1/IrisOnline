#include "stdafx.h"
#include "TerrainGeom.h"

cTerrainStripData::cTerrainStripData( unsigned short numVerts,
									 NiPoint3* posCoords, NiPoint3* alphas, NiColorA* colors, NiPoint2* texCoords,
									 unsigned short numTris, unsigned short numStrips,
									 unsigned short* stripLength, unsigned short* stripIndex,
									 const NiPoint3& center, float radius,
									 eConsistency consistency )
{
	assert( posCoords );
	assert( alphas );
	assert( colors );
	assert( texCoords );

	m_usVertices = numVerts;
	m_usActiveVertices = numVerts;
	m_pkVertex = posCoords;
	m_pkNormal = alphas;
	m_pkColor = colors;
	m_pkTexture = texCoords;

	m_usTriangles = numTris;
	m_usActiveTriangles = numTris;
	m_usStrips = numStrips;
	m_pusStripLengths = stripLength;
	m_pusStripLists = stripIndex;

	SetNumTextureSets( 1 );
	SetNormalBinormalTangentMethod( NiGeometryData::NBT_METHOD_NONE );

	m_kBound.SetCenterAndRadius( center, radius );

	switch( consistency )
	{
	case MUTABLE:
		SetConsistency( NiGeometryData::MUTABLE );
		break;
	case STATIC:
		SetConsistency( NiGeometryData::STATIC );
		break;
	case VOLATILE:
		SetConsistency( NiGeometryData::VOLATILE );
		break;
	default:
		assert( 0 );
	}
}

cTerrainStripData::~cTerrainStripData()
{
	NiRenderer::RemoveGeometryData( this );

	if( GetGroup() )
	{
		GetGroup()->DecRefCount();
		GetGroup()->DecRefCount();
		GetGroup()->DecRefCount();
		GetGroup()->DecRefCount();
		GetGroup()->DecRefCount();
		GetGroup()->DecRefCount();
	}

	m_pkVertex = 0;
	m_pkNormal = 0;
	m_pkColor = 0;
	m_pkTexture = 0;
	m_pusStripLengths = 0;
	m_pusStripLists = 0;
}

cTerrainStrip::cTerrainStrip( unsigned short numVerts,
							 NiPoint3* posCoords, NiPoint3* alphas, NiColorA* colors, NiPoint2* texCoords,
							 unsigned short numTris, unsigned short numStrips,
							 unsigned short* stripLength, unsigned short* stripIndex,
							 const NiPoint3& center, float radius,
							 cTerrainStripData::eConsistency consistency )
							 : NiTriStrips( NiNew cTerrainStripData( numVerts,
							 posCoords, alphas, colors, texCoords,
							 numTris, numStrips,
							 stripLength, stripIndex,
							 center, radius,
							 consistency ) )
{
	m_kWorldBound.SetCenterAndRadius( center, radius );
}

void cTerrainStrip::UpdateBound( const NiPoint3& center, float radius )
{
	m_kWorldBound.SetCenterAndRadius( center, radius );
}
