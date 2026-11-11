#include "stdafx.h"
#include "NaviMesh.h"

#include "NaviMeshNode.h"

static const char* gNaviMeshFileCode = "IrisNaviMesh";
static const int gNaviMeshFileVersion = 3;

bool cNaviMesh::Save( const cString& pathName )
{
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
		assert( 0 && "failed to open file to save navimesh" );
		return false;
	}

	/// 파일 헤더 저장
	cNaviMeshFileHeader header;
	::memcpy( header.mCode, gNaviMeshFileCode, 13 );
	header.mVersion = gNaviMeshFileVersion;
	header.mCellCount = mCellCount;
	header.mMetersPerVertex = mMetersPerVertex;
	header.mUnitsPerMeter = mUnitsPerMeter;

	if( saver.Write( &header, sizeof(cNaviMeshFileHeader) ) != sizeof(cNaviMeshFileHeader) )
	{
		assert( 0 && "failed to save navimesh file header" );
		return false;
	}

	/// 높이맵을 저장
	unsigned int numVerts = mLineCount * mLineCount;
	saver.Write( mHeights, sizeof(float) * numVerts );

	/// 모든 노드들을 저장
	if( mRootNode->Save( saver ) == false )
	{
		assert( 0 && "failed to save navimesh nodes" );
		return false;
	}

	/// 마무리~
	mModified = false;
	return true;
}
