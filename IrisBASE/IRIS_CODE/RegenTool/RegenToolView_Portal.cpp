#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "PortalDialog.h"
#include "FreeCamera.h"
#include "NaviMesh.h"
#include "SceneManager.h"
#include "PortalSceneNode.h"
#include "MonsterSceneNode.h"

int cRegenToolView::CreatePortal( unsigned int index, const char* potalName, float x, float y, unsigned int range, unsigned int targetIndex, int zdegree, float nameHeight, bool isCreated, bool updateDialog, bool updateView )
{
	/// 현재 경로를 설정
	SetCurrentDirectory( theApp.GetBaseDir().Cstr() );

	/// 중복 검사
	cPortalInfoMap::cIterator i = mPortalInfoMap.Find( index );

	if( i != mPortalInfoMap.End() )
		return -1;

	/// 장면 노드를 생성
	float z = 0.0f;
	mNaviMesh->CalcHeight( &z, x, y );

	cPortalSceneNodeParam param;

	cString pathName;
	pathName.Format( "./Data/Effect/%s", potalName );
	param.mPathName = pathName;

	param.mTranslate = NiPoint3( x, y, z );
	param.mRotate.FromEulerAnglesXYZ( 0.0f, 0.0f, D3DXToRadian(zdegree) );
	param.mNameHeight = nameHeight;

	cPortalSceneNode* n = SCENEMAN->CreatePortal( param );
	if( n == 0 )
		return -2;

	n->SetUserData0( index );

	cPortalInfo* info = new cPortalInfo( potalName, mMapIndex, x, y, range, targetIndex, zdegree, nameHeight, isCreated, n);
	n->SetUserData1( info->mTargetIndex );

	if( mDummyNode && info->mTargetIndex == 0 )
	{
		mDummyNode->SetTranslate( param.mTranslate );
		mDummyNode->SetRotate( param.mRotate );
		mDummyNode->Update();
	}

	/// 정보 맵에 추가
	mPortalInfoMap.Insert( index, info );

	/// 다이얼로그를 갱신
	if( updateDialog )
		mPortalDialog->UpdateList( mPortalInfoMap );

	/// 생성된 노드를 선택
	SelectNode( n, updateView );
	return 1;
}

int cRegenToolView::DeletePortal( cPortalSceneNode* node )
{
	if( mSelNode == 0 || mSelNode != node )
		return -1;

	/// 선택된 노드를 제거
	DeleteSel();
	return 1;
}

void cRegenToolView::SelectPortal( cPortalSceneNode* node )
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
