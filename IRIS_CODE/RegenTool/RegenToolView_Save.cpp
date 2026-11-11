#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "NpcSceneNode.h"
#include "PortalSceneNode.h"
#include <direct.h>

bool cRegenToolView::SaveNpcRegen()
{
	if( mMapIndex == 0 )
		return false;

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	cString folder;
	folder.Format( "%s%s", theApp.GetBaseDir().Cstr(), "Script\\Resource\\");
	::_mkdir(folder.Cstr());

	/// 파일 열기
	cString pathName;
	pathName.Format( "Script/Resource/NpcRegen%d_%d.txt", mMapIndex, mMapDegree );
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
	{
		MessageBox( "Failed to open npc regen.", "Map" );
		return false;
	}

	cString str;
	str = "// index x y zdegree\r\n";
	saver.Write( str.Cstr(), str.GetSize() );

	cNpcNodeMap::cIterator i = mNpcNodeMap.Begin();
	cNpcNodeMap::cIterator iend = mNpcNodeMap.End();

	for( ; i != iend; ++i )
	{
		cNpcSceneNode* node = i->mSecond;
		if( node == 0 )
			continue;

		unsigned int index = node->GetUserData0();
		const NiPoint3& t = node->GetWorldTranslate();
		const NiMatrix3& r = node->GetWorldRotate();
		float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
		r.ToEulerAnglesXYZ( xangle, yangle, zangle );

		str.Format( "%d\t%.2f\t%.2f\t%.2f\r\n", index, t.x, t.y, D3DXToDegree(zangle) );
		saver.Write( str.Cstr(), str.GetSize() );
	}
	return true;
}

bool cRegenToolView::SaveMonsterRegen()
{
	if( mMapIndex == 0 )
		return false;

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	cString folder;
	folder.Format( "%s%s", theApp.GetBaseDir().Cstr(), "Script\\Resource\\");
	::_mkdir(folder.Cstr());

	/// 파일 열기
	cString pathName;
	pathName.Format( "Script/Resource/MonsterRegen%d_%d.txt", mMapIndex, mMapDegree );
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
	{
		MessageBox( "Failed to open monster regen.", "Map" );
		return false;
	}

	/// 저장
	cMonsterGroupMap::cIterator i = mMonsterGroupMap.Begin();
	cMonsterGroupMap::cIterator iend = mMonsterGroupMap.End();

	for( ; i != iend; ++i )
	{
		cMonsterGroup* group = i->mSecond;
		group->Save( saver );
	}
	return true;
}

bool cRegenToolView::SaveGatheringRegen()
{
	if( mMapIndex == 0 )
		return false;

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	cString folder;
	folder.Format( "%s%s", theApp.GetBaseDir().Cstr(), "Script\\Resource\\");
	::_mkdir(folder.Cstr());

	/// 파일 열기
	cString pathName;
	pathName.Format( "Script/Resource/gatheringregen%d_%d.txt", mMapIndex, mMapDegree );
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
	{
		MessageBox( "Failed to open gathering regen.", "Map" );
		return false;
	}

	/// 저장
	cGatheringGroupMap::cIterator i = mGatheringGroupMap.Begin();
	cGatheringGroupMap::cIterator iend = mGatheringGroupMap.End();

	for( ; i != iend; ++i )
	{
		cGatheringGroup* group = i->mSecond;
		group->Save( saver );
	}
	return true;
}


bool cRegenToolView::SavePortalRegen()
{
	if( mMapIndex == 0 )
		return false;

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileSaver saver;

	cString folder;
	folder.Format( "%s%s", theApp.GetBaseDir().Cstr(), "Script\\Resource\\");
	::_mkdir(folder.Cstr());

	if( saver.Open( "Script/Resource/MapChangePos.txt" ) == false )
	{
		MessageBox( "Failed to open portal regen.", "Map" );
		return false;
	}

	cString str;
	str = "// index mapIndex x y range targetIndex zdegree_testtest\r\n";
	saver.Write( str.Cstr(), str.GetSize() );

	cPortalInfoMap::cIterator i = mPortalInfoMap.Begin();
	cPortalInfoMap::cIterator iend = mPortalInfoMap.End();

	for( ; i != iend; ++i )
	{
		unsigned int index = i->mFirst;
		cPortalInfo* info = i->mSecond;
		cPortalSceneNode* node = info->mNode;
		float x = 0.0f;
		float y = 0.0f;
		int zdegree = 0;
		float nameHeight = 0;

		if( node )
		{
			x = node->GetWorldTranslate().x;
			y = node->GetWorldTranslate().y;

			const NiMatrix3& r = node->GetWorldRotate();
			float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
			r.ToEulerAnglesXYZ( xangle, yangle, zangle );
			zdegree = (int)D3DXToDegree( zangle );

			nameHeight = node->GetNameHeight();
		}
		else
		{
			x = info->mX;
			y = info->mY;
			zdegree = info->mZDegree;
			nameHeight = info->mNameHeight;
		}

		str.Format( "%d\t%s\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\r\n", index, info->mPotalName.Cstr(), info->mMapIndex, x, y, info->mRange, info->mTargetIndex, zdegree, info->mIsCreateView, (unsigned int)nameHeight );
		saver.Write( str.Cstr(), str.GetSize() );
	}
	return true;
}


bool cRegenToolView::SavePattern()
{
	if( mMapIndex == 0 )
		return false;

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	cString folder;
	folder.Format( "%s%s", theApp.GetBaseDir().Cstr(), "Script\\Resource\\");
	::_mkdir(folder.Cstr());

	/// 파일 열기
	cString pathName;
	pathName.Format( "Script/Resource/monsterpattern%d_%d.txt", mMapIndex, mMapDegree );
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
	{
		MessageBox( "Failed to open Pattern.", "Map" );
		return false;
	}

	/// 저장
	cPatternMap::cIterator i = mPatternMap.Begin();
	cPatternMap::cIterator e = mPatternMap.End();

	for( ; i != e; ++i )
	{
		cPattern* pPattern = i->mSecond;
		if( pPattern != NULL )
			pPattern->SavePattern( saver );
	}
	return true;
}

bool cRegenToolView::SaveDoorRegen()
{
	if( mMapIndex == 0 )
		return false;

	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 파일 열기
	cFileSaver saver;

	cString folder;
	folder.Format( "%s%s", theApp.GetBaseDir().Cstr(), "Script\\Resource\\");
	::_mkdir(folder.Cstr());

	if( saver.Open( "Script/Resource/doorposlist.txt" ) == false )
	{
		MessageBox( "Failed to open door regen.", "Map" );
		return false;
	}

	cString str;
	str = "// index mapIndex filename x y zdegree_testtest\r\n";
	saver.Write( str.Cstr(), str.GetSize() );

	cDoorInfoMap::cIterator i = mDoorInfoMap.Begin();
	cDoorInfoMap::cIterator iend = mDoorInfoMap.End();

	for( ; i != iend; ++i )
	{
		unsigned int index = i->mFirst;
		cDoorInfo* info = i->mSecond;
		cDoorSceneNode* node = info->mNode;
		NiPoint3 pos;
		float appendZ = 0.0f;
		int zdegree = 0;

		NiPoint3* rectPos;
		float rectAppendY = 0.0f;
		if( node )
		{
			pos = node->GetWorldTranslate();
			appendZ = node->GetAppendZ();

			const NiMatrix3& r = node->GetWorldRotate();
			float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
			r.ToEulerAnglesXYZ( xangle, yangle, zangle );
			zdegree = (int)D3DXToDegree( zangle );

			/// rect info
			rectPos = node->GetRectPos();
			rectAppendY = node->GetRectAppendY();
		}
		else
		{
			pos = info->mPos;
			appendZ = info->mAppendZ;
			zdegree = info->mZDegree;

			/// rect info
			rectPos = info->mRectPos;
			rectAppendY = info->mRectAppendY;
		}

		str.Format( "%d\t%d\t%s\t%.2f\t%.2f\t%f\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\r\n", index, info->mMapIndex, info->mFileName.Cstr(), pos.x, pos.y, appendZ, zdegree, rectAppendY,
			rectPos[0].x, rectPos[0].y, rectPos[1].x, rectPos[1].y, rectPos[2].x, rectPos[2].y, rectPos[3].x, rectPos[3].y );
		saver.Write( str.Cstr(), str.GetSize() );
	}

	return true;
}