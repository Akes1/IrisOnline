#include "StdAfx.h"
#include "WorldManager.h"

#include "..\Client\NaviField.h"
#include "..\Client\pathfinder.h"
#include "baseObject_Common.h"

#include "GameResourceManager.h"

cWorldManager* cWorldManager::mpSingleton = 0;

cWorldManager::cWorldManager()
{
	mpSingleton = this;
}

cWorldManager::~cWorldManager()
{
	cMapInfoMap::cIterator i,end;
	sMapPathInfo* info = NULL;

	i = mMapInfoMap.Begin();
	end = mMapInfoMap.End();
	for( ; i != end; ++i )
	{
		info = (sMapPathInfo*)(*i).mSecond;
		if( info )
		{
			SAFE_DELETE(info->mNaviField);
			SAFE_DELETE(info->mPathFinder);
			info->mMapSize = 0.0f;

			delete info;
			info = 0;
		}
	}
	mMapInfoMap.Clear();
}


bool cWorldManager::Init()
{
	mMapInfoMap.Clear();

	/// mapÁ¤º¸ È¹µæ ¹× »ý¼º
	for(unsigned int i=MAP_MIN; i<MAP_MIN+MAP_COUNT; ++i )
	{
		unsigned int folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( i, 0 );
		if( folderIdx == 0 )
			continue;

		LoadMapData( folderIdx );
	}

	for(unsigned int i=MAP_PVP_MIN; i<MAP_PVP_MIN+MAP_PVP_COUNT; ++i )
	{
		unsigned int folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( i, 0 );
		if( folderIdx == 0 )
			continue;

		LoadMapData( folderIdx );
	}

	for(unsigned int i=MAP_THEME_MIN; i<MAP_THEME_MIN+MAP_THEME_CNT; ++i )
	{
		for( unsigned char mode=0;mode<3;mode++)
		{
			unsigned int folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( i, mode );
			if( folderIdx == 0 )
				continue;

			LoadMapData( folderIdx );
		}
	}

	for(unsigned int i=MAP_ETC_MIN; i<MAP_ETC_MIN+MAP_ETC_CNT; ++i )
	{
		unsigned int folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( i, 0 );
		if( folderIdx == 0 )
			continue;

		LoadMapData( folderIdx );
	}

	return true;
}

bool cWorldManager::LoadMapData( unsigned int folderIdx )
{
	sMapPathInfo* info = new sMapPathInfo;
	if( mMapInfoMap.Insert( folderIdx, info ) == false )
	{
		delete info;
		return false;
	}

	cNaviField* naviField = info->mNaviField =  new cNaviField;
	cPathFinder* pathFinder = info->mPathFinder = new cPathFinder( 4096 );

	cString fileName;
	fileName.Format( "./data/map/%03d.navifield", folderIdx );
	if( naviField->Load( fileName ) == false )
	{
		SAFE_DELETE(info->mNaviField);
		SAFE_DELETE(info->mPathFinder);
		SAFE_DELETE(info);
		return false;
	}

	pathFinder->Init( naviField );

	info->mMapSize = naviField->GetCellCount() * 100.0f;

	return true;
}

bool cWorldManager::CalcHeight( unsigned short folderIdx, float* height, float x, float y )
{
	sMapPathInfo* info = (sMapPathInfo*)mMapInfoMap.GetAt( folderIdx );
	if( info == 0 )
		return false;

	return info->mNaviField->CalcHeight( height, x, y );
}


cPathFinder* cWorldManager::GetPathFinder( unsigned long mapIdx, unsigned char mapMode )
{
	unsigned long folderIdx = GAMERESOURCEMAN->GetMapFolderIdx( mapIdx, mapMode );
	if( folderIdx == 0 )
		return 0;

	return GetPathFinder( folderIdx );
}

cPathFinder* cWorldManager::GetPathFinder( unsigned long folderIdx )
{
	sMapPathInfo* info = (sMapPathInfo*)mMapInfoMap.GetAt( folderIdx );
	if( info == 0 )
		return 0;

	return info->mPathFinder;
}