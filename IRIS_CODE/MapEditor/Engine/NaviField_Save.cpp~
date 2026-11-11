#include "stdafx.h"
#include "NaviField.h"

#include "../NaviFieldPaintDialog.h"

static const char* gpNaviFieldFileCode = "IrisNaviField";
static const int gNaviFieldFileVersion = 4;

bool cNaviField::Save( const cString& pathName )
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
		assert( 0 && "failed to open file to save navifield" );
		return false;
	}

	/// 파일 헤더를 저장
	cNaviFieldFileHeader header;
	memcpy( header.mCode, gpNaviFieldFileCode, 14 );
	header.mVersion = gNaviFieldFileVersion;
	header.mCellCount = mCellCount;
	header.mMetersPerVertex = mMetersPerVertex;
	header.mUnitsPerMeter = mUnitsPerMeter;

	if( saver.Write( &header, sizeof(cNaviFieldFileHeader) ) != sizeof(cNaviFieldFileHeader) )
	{
		assert( 0 && "failed to write navifield file header" );
		return false;
	}

	/// 필드값 배열을 저장
	unsigned int numVerts = mLineCount * mLineCount;
	saver.Write( mValues, sizeof(unsigned char) * numVerts );

	/// 높이값 배열을 저장
	saver.Write( mHeights, sizeof(float) * numVerts );
	return true;
}
