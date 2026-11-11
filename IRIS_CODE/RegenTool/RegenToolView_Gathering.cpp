#include "stdafx.h"
#include "RegenToolView.h"

#include "GatheringDialog.h"
#include "FreeCamera.h"
#include "NaviMesh.h"
#include "SceneManager.h"
#include "GatheringSceneNode.h"


void cRegenToolView::SelectGatheringGroup( cGatheringGroup* group )
{
	mSelGatheringGroup = group;
	mSelMonsterGroup = 0;

	mGatheringDialog->SelectGroup( group );

	if( group && group->mNodeList.IsEmpty() == false )
		SelectGathering( *(group->mNodeList.Begin()) );
	else
		SelectGathering( 0 );
}

int cRegenToolView::CreateGatheringGroup( unsigned int index )
{
	cGatheringGroupMap::cIterator i = mGatheringGroupMap.Find( index );

	if( i != mGatheringGroupMap.End() )
		return -1;

	cGatheringGroup* group = new cGatheringGroup( index );
	group->mRegenGroupIndex = index;
	mGatheringGroupMap.Insert( index, group );

	mGatheringDialog->UpdateGroupList( mGatheringGroupMap );
	SelectGatheringGroup( group );
	return 1;
}

int cRegenToolView::DeleteGatheringGroup( unsigned int index )
{
	cGatheringGroupMap::cIterator i = mGatheringGroupMap.Find( index );

	if( i == mGatheringGroupMap.End() )
		return 0;

	delete i->mSecond;
	SelectGatheringGroup( 0 );

	if( mGatheringGroupMap.Erase( index ) )
	{
		mGatheringDialog->UpdateGroupList( mGatheringGroupMap );
		return 1;
	}
	else
		return 0;
}

int cRegenToolView::CreateGathering( unsigned int index, unsigned int x, unsigned int y, int zdegree, float appandZ, bool updateDialog, bool updateView )
{
	if( mSelGatheringGroup == 0 )
		return 0;

	cGatheringGroup* group = mSelGatheringGroup;

	/// 현재 경로를 설정
	//	cString path;
	//	path.Format( "%sData/Monster/", theApp.GetBaseDir().Cstr() );
	//	SetCurrentDirectory( path.Cstr() );

	/// 인덱스 검사
	cGatheringInfoMap::cIterator ii = mGatheringInfoMap.Find( index );

	if( ii == mGatheringInfoMap.End() )
		return 0;

	/// 모델 파일 검사
	cGatheringInfo* info = ii->mSecond;

	/// 장면 노드를 생성
	float z = 0.0f;
	mNaviMesh->CalcHeight( &z, (float)x, (float)y );

	cGatheringSceneNodeParam param;
	cString str = info->mFileName;
	cString pathName;
	pathName.Format( "./Data/Monster/%s", str.Cstr() );
	param.mPathName = pathName;
	param.mAppandZ = appandZ;

	param.mTranslate = NiPoint3( (float)x, (float)y, z );
	param.mRotate.FromEulerAnglesXYZ( 0.0f, 0.0f, D3DXToRadian(zdegree) );
	param.mGatheringIdx = index;

	cGatheringSceneNode* n = SCENEMAN->CreateGathering( param );
	if( n == 0 )
		return -2;

	n->SetUserData0( index );
	n->SetUserData2( (unsigned int)group );

	/// 리스트에 추가
	group->mNodeList.PushBack( n );

	/// 다이얼로그를 갱신
	if( updateDialog )
		mGatheringDialog->UpdateList( group->mNodeList );

	/// 생성된 노드를 선택
	SelectNode( n, updateView );
	return 1;
}

int cRegenToolView::ChangeGathering( unsigned int index )
{
	if( mSelNode == 0 || mSelNode ->GetType() != SCENENODE_GATHERING )
		return -1;

	if( mSelNode->GetUserData0() == index )
		return -2;

	/// 새로 생성
	float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
	NiPoint3 t = mSelNode->GetWorldTranslate();
	NiMatrix3 r = mSelNode->GetWorldRotate();
	float appendZ = ((cGatheringSceneNode*)mSelNode)->GetAppendZ();
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );

	DeleteSel();
	CreateGathering( index, (unsigned int)t.x, (unsigned int)t.y, (int)D3DXToDegree(zangle), appendZ, true, true );
	return 1;
}

int cRegenToolView::DeleteGathering( cGatheringSceneNode* node )
{
	if( mSelNode == 0 || mSelNode != node )
		return -1;

	/// 선택된 노드를 제거
	DeleteSel();
	return 1;
}

void cRegenToolView::SelectGathering( cGatheringSceneNode* node )
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
