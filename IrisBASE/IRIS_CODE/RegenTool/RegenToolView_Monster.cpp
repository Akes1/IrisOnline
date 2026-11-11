#include "stdafx.h"
#include "RegenToolView.h"

#include "RegenToolApp.h"
#include "MonsterDialog.h"
#include "FreeCamera.h"
#include "NaviMesh.h"
#include "SceneManager.h"
#include "MonsterSceneNode.h"

void cRegenToolView::SelectMonsterGroup( cMonsterGroup* group )
{
	mSelMonsterGroup = group;
	mSelGatheringGroup = 0;

	mMonsterDialog->SelectGroup( group );

	if( group && group->mNodeList.IsEmpty() == false )
		SelectMonster( *(group->mNodeList.Begin()) );
	else
		SelectMonster( 0 );
}

int cRegenToolView::CreateMonsterGroup( unsigned int index )
{
	cMonsterGroupMap::cIterator i = mMonsterGroupMap.Find( index );

	if( i != mMonsterGroupMap.End() )
		return -1;

	cMonsterGroup* group = new cMonsterGroup( index );
	group->mNextRegenGroupMap.Insert( index, index );
	mMonsterGroupMap.Insert( index, group );

	mMonsterDialog->UpdateGroupList( mMonsterGroupMap );
	SelectMonsterGroup( group );
	return 1;
}

int cRegenToolView::DeleteMonsterGroup( unsigned int index )
{
	cMonsterGroupMap::cIterator i = mMonsterGroupMap.Find( index );

	if( i == mMonsterGroupMap.End() )
		return 0;

	delete i->mSecond;
	SelectMonsterGroup( 0 );

	if( mMonsterGroupMap.Erase( index ) )
	{
		cMonsterGroupMap::cIterator b = mMonsterGroupMap.Begin();
		cMonsterGroupMap::cIterator bend = mMonsterGroupMap.End();
		for( ; b!=bend; ++b )
		{
			cMonsterGroup* group = b->mSecond;
			if( group )
				group->mNextRegenGroupMap.Erase( index );
		}

		mMonsterDialog->UpdateGroupList( mMonsterGroupMap );
		return 1;
	}
	else
		return 0;
}

int cRegenToolView::CreateMonster( unsigned int index, unsigned int x, unsigned int y, int zdegree, unsigned int regenWaitTime, unsigned int regenLifeTime, 
								  bool updateDialog, bool updateView, unsigned long regenFamilyNum, unsigned long patternIdx )
{
	if( mSelMonsterGroup == 0 )
		return 0;

	cMonsterGroup* group = mSelMonsterGroup;

	/// 현재 경로를 설정
//	cString path;
//	path.Format( "%sData/Monster/", theApp.GetBaseDir().Cstr() );
//	SetCurrentDirectory( path.Cstr() );

	/// 인덱스 검사
	cMonsterInfoMap::cIterator ii = mMonsterInfoMap.Find( index );

	if( ii == mMonsterInfoMap.End() )
		return 0;

	/// 모델 파일 검사
	cMonsterInfo* info = ii->mSecond;
	cModelFileMap::cIterator mi = mModelFileMap.Find( info->mModelIndex );

	/// 장면 노드를 생성
	float z = 0.0f;
	mNaviMesh->CalcHeight( &z, (float)x, (float)y );

	cMonsterSceneNodeParam param;
	cString str = mi->mSecond;
	cString pathName;
	pathName.Format( "./Data/Monster/%s", str.Cstr() );
	param.mPathName = pathName;

	param.mTranslate = NiPoint3( (float)x, (float)y, z );
	param.mRotate.FromEulerAnglesXYZ( 0.0f, 0.0f, D3DXToRadian(zdegree) );

	cMonsterSceneNode* n = SCENEMAN->CreateMonster( param );
	if( n == 0 )
		return -2;

	n->SetTargetAnimation( 1 );
	n->SetUserData0( index );
	n->SetUserData1( regenWaitTime );
	n->SetUserData2( (unsigned int)group );
	n->SetUserData3( regenLifeTime );
	n->SetUserData4( regenFamilyNum );
	n->SetUserData5( patternIdx );

	/// 리스트에 추가
	group->mNodeList.PushBack( n );

	/// 다이얼로그를 갱신
	if( updateDialog )
		mMonsterDialog->UpdateList( group->mNodeList );

	/// 생성된 노드를 선택
	SelectNode( n, updateView );
	return 1;
}

int cRegenToolView::ChangeMonster( unsigned int index )
{
	if( mSelNode == 0 || mSelNode ->GetType() != SCENENODE_MONSTER )
		return -1;

	if( mSelNode->GetUserData0() == index )
		return -2;

	/// 새로 생성
	float xangle = 0.0f, yangle = 0.0f, zangle = 0.0f;
	NiPoint3 t = mSelNode->GetWorldTranslate();
	NiMatrix3 r = mSelNode->GetWorldRotate();
	r.ToEulerAnglesXYZ( xangle, yangle, zangle );
	unsigned int regenWaitTime = mSelNode->GetUserData1();
	unsigned int regenLifeTime = mSelNode->GetUserData3();
	unsigned long regenFamilyNum = mSelNode->GetUserData4();
	unsigned long regenPatternIdx = mSelNode->GetUserData5();

	DeleteSel();
	CreateMonster( index, (unsigned int)t.x, (unsigned int)t.y, (int)D3DXToDegree(zangle), regenWaitTime, regenLifeTime, true, true, 
		regenFamilyNum, regenPatternIdx );
	return 1;
}

int cRegenToolView::DeleteMonster( cMonsterSceneNode* node )
{
	if( mSelNode == 0 || mSelNode != node )
		return -1;

	/// 선택된 노드를 제거
	DeleteSel();
	return 1;
}

void cRegenToolView::SelectMonster( cMonsterSceneNode* node )
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
