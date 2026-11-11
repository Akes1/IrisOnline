#include "stdafx.h"
#include "TerrainEditor.h"

#include "StringTable.h"
#include "Engine/Terrain.h"

void cTerrainEditor::ExportLog( cStringTable& table, unsigned int& row )
{
	unsigned int resolution = TERRAIN->GetCellCount();
	table.SetValue( ++row, 0, "----- 지형 정보 -----" );

	table.SetValue( ++row, 0, "[파일 이름]" );
	table.SetValue( row, 1, mPathName );
	table.SetValue( ++row, 0, "[해상도]" );
	table.SetValue( row, 1, resolution );
	table.SetValue( ++row, 0, "[정점당 미터]" );
	table.SetValue( row, 1, TERRAIN->GetMetersPerVertex() );
	table.SetValue( ++row, 0, "[실제 크기]" );
	table.SetValue( row, 1, resolution * TERRAIN->GetMetersPerVertex() );

	table.SetValue( ++row, 0, "[타일 텍스처]" );

	/// 텍스처 배열 정보
	for( unsigned int i = 0; i < TERRAIN_TEXTURE_COUNT; ++i )
	{
		table.SetValue( ++row, 0, i );

		cTerrainTexture* tex = TERRAIN->GetTexture( i );
		
		if( tex )
			table.SetValue( row, 1, tex->GetFileName() );
	}
	table.SetValue( ++row, 0, "----------" );
	++row;
}
