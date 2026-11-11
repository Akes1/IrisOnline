#include "stdafx.h"

#ifdef MAP_EDITOR
#include "TerrainNode.h"
#include "Terrain.h"

bool cTerrainBranchNode::Save( cFileSaver& saver )
{
	/// 경계 상자를 저장
	if( saver.Write( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	/// 자식 노드들을 저장
	if( mChild[0]->Save( saver ) == false )
		return false;
	if( mChild[1]->Save( saver ) == false )
		return false;
	if( mChild[2]->Save( saver ) == false )
		return false;
	if( mChild[3]->Save( saver ) == false )
		return false;
	return true;
}

bool cTerrainLeafNode::Save( cFileSaver& saver )
{
	/// 경계 상자를 저장
	if( saver.Write( &mBoundBox, sizeof(cBox) ) != sizeof(cBox) )
		return false;

	/// 디테일 텍스처 인덱스를 저장
	unsigned int ti0 = mTexture0->mIndexByTerrain;
	unsigned int ti1 = mTexture1->mIndexByTerrain;
	unsigned int ti2 = mTexture2->mIndexByTerrain;

	if( ti0 >= TERRAIN_TEXTURE_COUNT || ti1 >= TERRAIN_TEXTURE_COUNT || ti2 >= TERRAIN_TEXTURE_COUNT )
	{
		assert( 0 );
		return false;
	}

	saver.WriteUnsignedInt( ti0 );
	saver.WriteUnsignedInt( ti1 );
	saver.WriteUnsignedInt( ti2 );

	/// 가시 여부를 저장
	saver.WriteUnsignedInt( mVisible ? 1 : 0 );
	return true;
}
#endif
