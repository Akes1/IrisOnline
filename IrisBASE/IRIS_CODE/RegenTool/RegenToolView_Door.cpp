#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "DoorDialog.h"
#include "SceneManager.h"
#include "PortalSceneNode.h"
#include "FreeCamera.h"
#include "NaviMesh.h"




int cRegenToolView::CreateDoor( unsigned int index, const char* fileName, NiPoint2 pos, float appendZ, int zdegree, float* rectWidth, float* rectHeight, float rectAppendY, bool updateDialog, bool updateView )
{
	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 중복 검사
	cDoorInfoMap::cIterator i = mDoorInfoMap.Find( index );

	if( i != mDoorInfoMap.End() )
		return -1;

	/// 장면 노드를 생성
	float z = 0.0f;
	mNaviMesh->CalcHeight( &z, pos.x, pos.y );

	cDoorSceneNodeParam param;

	cString pathName;
	pathName.Format( "./Data/Effect/%s", fileName );
	param.mPathName = pathName;

	param.mTranslate = NiPoint3( pos.x, pos.y, z );
	param.mRotate.FromEulerAnglesXYZ( 0.0f, 0.0f, D3DXToRadian(zdegree) );
	param.mAppendZ = appendZ;

	param.mRectTopWidth = rectWidth[0];
	param.mRectBottomWidth = rectWidth[1];
	param.mRectLeftHeight = rectHeight[0];
	param.mRectRightHeight = rectHeight[1];
	param.mRectAppendY = rectAppendY;

	cDoorSceneNode* n = SCENEMAN->CreateDoor( param );
	if( n == 0 )
		return -2;

	n->SetUserData0( index );
	n->SetRectInfo( rectWidth[0], rectWidth[1], rectHeight[0], rectHeight[1] );
	n->SetRectAppendY( rectAppendY );

	cDoorInfo* info = new cDoorInfo;
	info->mNode = n;
	info->mFileName = fileName;
	info->mMapIndex = mMapIndex;
	info->mPos.x = pos.x;
	info->mPos.y = pos.y;
	info->mZDegree = zdegree;
	info->mAppendZ = appendZ;
	info->mRectAppendY = rectAppendY;

	/// 정보 맵에 추가
	mDoorInfoMap.Insert( index, info );

	/// 다이얼로그를 갱신
	if( updateDialog )
		mDoorDialog->UpdateList( mDoorInfoMap );

	/// 생성된 노드를 선택
	SelectNode( n, updateView );
	return 1;
}

int cRegenToolView::DeleteDoor( cDoorSceneNode* node )
{
	if( mSelNode == 0 || mSelNode != node )
		return -1;

	/// 선택된 노드를 제거
	DeleteSel();
	return 1;
}

void cRegenToolView::SelectDoor( cDoorSceneNode* node )
{
	if( node )
	{
		NiPoint3 at = node->GetCenter();
		NiPoint3 dir = node->GetWorldRotate() * -NiPoint3::UNIT_Y;
		dir.z = 0.0f;
		dir.Unitize();
		NiPoint3 pos = at + dir * 1200.0f;
		pos.z += 300.0f;
		mCamera->SetTranslate( pos );
		mCamera->LookAt( at );
	}

	SelectNode( node );
}
