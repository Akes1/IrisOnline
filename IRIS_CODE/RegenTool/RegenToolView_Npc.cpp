#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "NpcDialog.h"
#include "FreeCamera.h"
#include "NaviMesh.h"
#include "SceneManager.h"
#include "NpcSceneNode.h"

int cRegenToolView::CreateNpc( unsigned int index, float x, float y, float zdegree, bool updateDialog, bool updateView )
{
	/// 인덱스 검사
	cNpcInfoMap::cIterator ii = mNpcInfoMap.Find( index );
	if( ii == mNpcInfoMap.End() )
		return 0;

	/// 모델 파일 검사
	cNpcInfo* info = ii->mSecond;
	cModelFileMap::cIterator mi = mModelFileMap.Find( info->mModelIndex );

	/// 장면 노드를 생성
	float z = 0.0f;
	mNaviMesh->CalcHeight( &z, x, y );

	cNpcSceneNodeParam param;

	cString str = mi->mSecond;
	cString pathName;
	pathName.Format( "./Data/Npc/%s", str.Cstr() );
	param.mPathName = pathName;
	param.mTranslate = NiPoint3( x, y, z );
	param.mRotate.FromEulerAnglesXYZ( 0.0f, 0.0f, D3DXToRadian(zdegree) );


	cNpcSceneNode* n = SCENEMAN->CreateNPC( param );
	if( n == 0 )
		return -2;

	n->SetTargetAnimation( 1 );
	n->SetUserData0( index );

	/// 노드 맵에 추가
	mNpcNodeMap.Insert( mNpcListIdx, n );
	mNpcListIdx++;

	/// 다이얼로그를 갱신
	if( updateDialog )
		mNpcDialog->UpdateList( mNpcNodeMap );

	/// 생성된 노드를 선택
	SelectNode( n, updateView );
	return 1;
}

int cRegenToolView::ChangeNpc( unsigned int index )
{
	if( mSelNode == 0 || mSelNode ->GetType() != SCENENODE_NPC )
		return -1;

	if( mSelNode->GetUserData0() == index )
		return -2;

	/// 새로 생성
	float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
	NiPoint3 t = mSelNode->GetWorldTranslate();
	NiMatrix3 r = mSelNode->GetWorldRotate();
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );

	DeleteSel();
	CreateNpc( index, t.x, t.y, D3DXToDegree(zangle), true, true );
	return 1;
}

int cRegenToolView::DeleteNpc( cNpcSceneNode* node )
{
	if( mSelNode == 0 || mSelNode != node )
		return -1;

	/// 선택된 노드를 제거
	DeleteSel();
	return 1;
}

void cRegenToolView::SelectNpc( cNpcSceneNode* node )
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
