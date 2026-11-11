#include "stdafx.h"
#include "ObjectEditor.h"

#include "TerrainEditor.h"
#include "StringTable.h"
#include "Engine/StaticSceneNode.h"

void cObjectEditor::ExportLog( cStringTable& table, unsigned int& row, unsigned int divides )
{
	typedef tArray<cStaticSceneNode*> cArray;
	typedef tHashMap<cString, cArray> cMap;
	cMap map;
	cString name;

	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator end = mCreatedSet.End();

	for( ; i != end; ++i )
	{
		cStaticSceneNode* n = *i;
		name = n->GetFileName();

		map.Insert( name, cArray() );
		map[name].PushBack( n );
	}

	table.SetValue( ++row, 0, "----- 장면 정보 -----" );

	table.SetValue( ++row, 0, "[파일 이름]" );
	table.SetValue( row, 1, mPathName );
	table.SetValue( ++row, 0, "[오브젝트 수]" );
	table.SetValue( row, 1, mCreatedSet.GetSize() );
	table.SetValue( ++row, 0, "----------" );
	++row;

	table.SetValue( ++row, 0, "----- 오브젝트 파일 정보 -----" );
	table.SetValue( ++row, 0, "[파일 이름]" );
	table.SetValue( row, 1, "[정점 수]" );
	table.SetValue( row, 2, "[애니메이션 가능]" );
	table.SetValue( row, 3, "[지오메트리 수]" );
	table.SetValue( row, 4, "[총 사용 오브젝트 수]" );
	table.SetValue( row, 5, "[총 사용 지오메트리 수]" );
	{
		cMap::cIterator i = map.Begin();
		cMap::cIterator end = map.End();

		for( ; i != end; ++i )
		{
			cArray& array = i->mSecond;
			cStaticSceneNode* n = array[0];

			table.SetValue( ++row, 0, i->mFirst );
			table.SetValue( row, 1, n->GetNumVerts() );
			table.SetValue( row, 2, unsigned int(n->IsAnimated() ? 1 : 0) );
			table.SetValue( row, 3,	n->GetNumGeoms() );
			table.SetValue( row, 4, array.GetSize() );
			table.SetValue( row, 5, array.GetSize() * n->GetNumGeoms() );
		}
	}
	table.SetValue( ++row, 0, "----------" );
	++row;

	table.SetValue( ++row, 0, "----- 분할 정보 -----" );
	++row;

	float size = TERRAINEDIT->GetResolution() * TERRAINEDIT->GetMetersPerVertex() / divides;
	size *= TERRAINEDIT->GetUnitsPerMeter();
	float minx, miny;
	float maxx, maxy;
	cString rc;

	for( unsigned int r = 0; r < divides; ++r )
	{
		miny = float(r) * size;
		maxy = miny + size;

		for( unsigned int c = 0; c < divides; ++c )
		{
			minx = float(c) * size;
			maxx = minx + size;
			unsigned int numObjects = 0;

			cCreatedSet::cIterator i = mCreatedSet.Begin();
			cCreatedSet::cIterator end = mCreatedSet.End();

			for( ; i != end; ++i )
			{
				cStaticSceneNode* n = *i;
				const NiPoint3& pos = n->GetWorldTranslate();

				if( pos.x >= minx && pos.x <= maxx && pos.y >= miny && pos.y <= maxy )
				{
					++numObjects;
				}
			}

			rc.Format( "[구역 %2d:%2d]", r, c );
			table.SetValue( ++row, 0, rc );
			table.SetValue( row, 1, "[오브젝트 수]" );
			table.SetValue( row, 2, numObjects );
		}
	}
	++row;

	table.SetValue( ++row, 0, "----- 구역 정보 -----" );
	++row;

	for( unsigned int r = 0; r < divides; ++r )
	{
		miny = float(r) * size;
		maxy = miny + size;

		for( unsigned int c = 0; c < divides; ++c )
		{
			minx = float(c) * size;
			maxx = minx + size;
			unsigned int numObjects = 0;
			
			map.Clear();

			cCreatedSet::cIterator i = mCreatedSet.Begin();
			cCreatedSet::cIterator end = mCreatedSet.End();

			for( ; i != end; ++i )
			{
				cStaticSceneNode* n = *i;
				const NiPoint3& pos = n->GetWorldTranslate();

				if( pos.x >= minx && pos.x <= maxx && pos.y >= miny && pos.y <= maxy )
				{
					name = n->GetFileName();

					map.Insert( name, cArray() );
					map[name].PushBack( n );
					++numObjects;
				}
			}

			rc.Format( "[구역 %2d:%2d]", r, c );
			table.SetValue( ++row, 0, rc );
			table.SetValue( row, 1, "[오브젝트 수]" );
			table.SetValue( row, 2, numObjects );

			if( numObjects == 0 )
			{
				++row;
				continue;
			}

			table.SetValue( ++row, 0, "[파일 이름]" );
			table.SetValue( row, 1, "[정점 수]" );
			table.SetValue( row, 2, "[총 사용 오브젝트 수]" );
			table.SetValue( row, 3, "[총 사용 지오메트리 수]" );
			table.SetValue( row, 4, "[총 사용 정점 수]" );
			{
				cMap::cIterator i = map.Begin();
				cMap::cIterator end = map.End();

				for( ; i != end; ++i )
				{
					cArray& array = i->mSecond;
					cStaticSceneNode* n = array[0];

					table.SetValue( ++row, 0, i->mFirst );
					table.SetValue( row, 1, n->GetNumVerts() );
					table.SetValue( row, 2, array.GetSize() );
					table.SetValue( row, 3, array.GetSize() * n->GetNumGeoms() );
					table.SetValue( row, 4, array.GetSize() * n->GetNumVerts() );
				}
			}

			table.SetValue( ++row, 0, "[오브젝트 이름]" );
			table.SetValue( row, 1, "[가시 레벨]" );
			table.SetValue( row, 2, "[차폐 레벨]" );
			{
				cMap::cIterator i = map.Begin();
				cMap::cIterator end = map.End();

				for( ; i != end; ++i )
				{
					cArray& array = i->mSecond;

					for( unsigned int j = 0, jend = array.GetSize(); j < jend; ++j )
					{
						cStaticSceneNode* n = array[j];

						table.SetValue( ++row, 0, n->GetFileName() );
						table.SetValue( row, 1, n->GetVisibleLevel() );
						table.SetValue( row, 2, n->GetOccludeLevel() );
					}
				}
			}
			++row;
		}
	}
}
