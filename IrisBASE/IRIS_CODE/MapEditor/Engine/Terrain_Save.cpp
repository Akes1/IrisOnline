#include "stdafx.h"

#ifdef MAP_EDITOR
#include "Terrain.h"
#include "TerrainNode.h"

static const char* gTerrainFileCode = "IrisTerrain";
static const int gTerrainFileVersion = 6;

bool cTerrain::Save( const cString& pathName )
{
	/// 쓰지않는 텍스처를 제거
	for( unsigned int i = 1; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		cTerrainTexture* tex = mTextures[i];

		if( tex && tex->mRefCount == 0 )
		{
			delete tex;
		}
	}

	/// 파일 열기
	cFileSaver saver;

	if( FileExist( pathName ) )
	{
		cString newPathName = pathName;
		newPathName += ".bak";
		FileRemove( newPathName );
		FileRename( pathName, newPathName );
	}
	if( saver.Open( pathName ) == false )
	{
		assert( 0 && "failed to open file to save terrain" );
		return false;
	}

	/// 파일 헤더 저장
	cTerrainFileHeader header;
	::memcpy( header.mCode, gTerrainFileCode, 12 );
	header.mVersion = gTerrainFileVersion;
	header.mCellCount = mCellCount;
	header.mMetersPerVertex = mMetersPerVertex;
	header.mUnitsPerMeter = mUnitsPerMeter;
	header.mNumTextures = mNumTextures;

	if( saver.Write( &header, sizeof(cTerrainFileHeader) ) != sizeof(cTerrainFileHeader) )
	{
		assert( 0 && "failed to save terrain file header" );
		return false;
	}

	/// 텍스처 배열을 저장
	char cname[64];

	for( unsigned int i = 1; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		cTerrainTexture* tex = mTextures[i];
		
		if( tex )
		{
			::memset( cname, 0, 64 );
			::strncpy_s( cname, tex->GetFileName(), 63 );
			cname[63] = 0;

			saver.WriteUnsignedInt( i );
			saver.Write( cname, 64 );
			--header.mNumTextures;
		}
	}

	if( header.mNumTextures != 0 )
	{
		assert( 0 );
		return false;
	}

	/// 높이맵, 알파맵, 색상맵을 저장
	unsigned int numVerts = mLineCount * mLineCount;
	saver.Write( mHeights, sizeof(float) * numVerts );
	saver.Write( mAlphas, sizeof(NiPoint3) * numVerts );
	saver.Write( mColors, sizeof(NiColor) * numVerts );

	/// 모든 노드들을 저장
	if( mRootNode->Save( saver ) == false )
	{
		assert( 0 && "failed to save terrain nodes" );
		return false;
	}

	/// 마무리~
	mModified = false;
	return true;
}

bool cTerrain::SavePaintAlpha( const cString& pathName )
{
	NiFilename filename(pathName.Cstr());
	filename.SetExt(".paint");

	char buffer[MAX_PATH] = {0,};
	filename.GetFullPath(buffer, MAX_PATH);
	cString path( buffer );

	cFileSaver saver;
	if( FileExist( path ) )
	{
		cString newPathName = path;
		newPathName += ".bak";
		FileRemove( newPathName );
		FileRename( path, newPathName );
	}
	if( saver.Open( path ) == false )
	{
		assert( 0 && "failed to open file to save paint alpha" );
		return false;
	}

	unsigned int numVerts = mLineCount * mLineCount;
	saver.Write( mPaintAlphas, sizeof(NiPoint3) * numVerts );

	mModified = false;
	return true;
}

#endif /// MAP_EDITOR
