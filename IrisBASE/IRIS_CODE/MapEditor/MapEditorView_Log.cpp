#include "stdafx.h"
#include "MapEditorView.h"

#include "TerrainEditor.h"
#include "ObjectEditor.h"
#include "StringTable.h"

bool cMapEditorView::SaveLog( const cString& pathName, unsigned int resolution )
{
	cStringTable table;
	unsigned int row = 0;
	float size = mTerrainEditor->GetResolution() * mTerrainEditor->GetMetersPerVertex() / resolution;

	table.SetValue( row, 0, "----- 아이리스 맵 로그 -----" );
	++row;

	table.SetValue( ++row, 0, "----- 로그 정보 -----" );
	table.SetValue( ++row, 0, "[파일 이름]" );
	table.SetValue( row, 1, pathName );
	table.SetValue( ++row, 0, "[분할 해상도]" );
	table.SetValue( row, 1, resolution );
	table.SetValue( ++row, 0, "[분할 크기]" );
	table.SetValue( row, 1, size );
	table.SetValue( ++row, 0, "----------" );
	++row;

	mTerrainEditor->ExportLog( table, row );
	mObjectEditor->ExportLog( table, row, resolution );

	table.SetValue( ++row, 0, "----- 끝 -----" );

	return table.Save( pathName );
}
