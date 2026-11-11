#include "stdafx.h"
#include "ObjectEditor.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"

#include "Doing/DoingManager.h"
#include "Doing/ObjectTransforming.h"

#include "Engine/SceneManager.h"
#include "Engine/StaticSceneNode.h"
#include "Engine/Terrain.h"

void cObjectGroup::SetTranslate( const NiPoint3& trans )
{
	/// 실행 정보를 추가
	DOINGMAN->PushObjectTranslating( mNiNode->GetTranslate(), trans );

	/// 이동
	mNiNode->SetTranslate( trans );
	mNiNode->Update( 0.0f );

	/// 선택된 장면 노드들을 갱신
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetNeedUpdateTransform( true );
	}
}

void cObjectGroup::SetRotate( float xangle, float yangle, float zangle )
{
	NiMatrix3 r;
	r.FromEulerAnglesXYZ( xangle, yangle, zangle );

	/// 실행 정보를 추가
	DOINGMAN->PushObjectRotating( mNiNode->GetRotate(), r );

	/// 회전
	mNiNode->SetRotate( r );
	mNiNode->Update( 0.0f );

	/// 선택된 장면 노드들을 갱신
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetNeedUpdateTransform( true );
	}
}

void cObjectGroup::SetScale( float scale )
{
	/// 실행 정보를 추가
	DOINGMAN->PushObjectScaling( mNiNode->GetScale(), scale );

	/// 배율
	mNiNode->SetScale( scale );
	mNiNode->Update( 0.0f );

	/// 선택된 장면 노드들을 갱신
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		(*i)->SetNeedUpdateTransform( true );
	}
}

void cObjectGroup::Undo( const cObjectTranslating& doing )
{
	mNiNode->SetTranslate( doing.mUndo );
	mNiNode->Update( 0.0f );
}

void cObjectGroup::Undo( const cObjectRotating& doing )
{
	mNiNode->SetRotate( doing.mUndo );
	mNiNode->Update( 0.0f );
}

void cObjectGroup::Undo( const cObjectScaling& doing )
{
	mNiNode->SetScale( doing.mUndo );
	mNiNode->Update( 0.0f );
}

void cObjectGroup::Redo( const cObjectTranslating& doing )
{
	mNiNode->SetTranslate( doing.mRedo );
	mNiNode->Update( 0.0f );
}

void cObjectGroup::Redo( const cObjectRotating& doing )
{
	mNiNode->SetRotate( doing.mRedo );
	mNiNode->Update( 0.0f );
}

void cObjectGroup::Redo( const cObjectScaling& doing )
{
	mNiNode->SetScale( doing.mRedo );
	mNiNode->Update( 0.0f );
}

bool cObjectEditor::Pick( CPoint point, bool ctrl )
{
	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
	{
		cStaticSceneNode* n = (cStaticSceneNode*)mPickedArray[0];

		mObjectGroup->Select( n, ctrl );
		mTransformDialog->UpdateObject( mObjectGroup );
		mPropertyDialog->UpdateObject( mObjectGroup );
		return true;
	}
	else
	{
		return false;
	}
}

void cObjectEditor::AttachGroupToTerrain( CPoint point )
{
	if( mObjectGroup->IsEmpty() )
		return;

	NiPoint3 pos;

	if( TERRAIN->Pick( &pos, point.x, point.y ) )
	{
		mObjectGroup->SetTranslate( pos );
		mTransformDialog->UpdateObject( mObjectGroup );

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::AttachGroupToObject( CPoint point )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mPickedArray.Clear();

	if( SCENEMAN->Pick( &mPickedArray, point.x, point.y, true, SCENENODE_STATIC ) )
	{
		cSceneNode* n = mPickedArray[0];
		mObjectGroup->SetTranslate( n->GetPickPos() );
		mTransformDialog->UpdateObject( mObjectGroup );

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );
	}
}

void cObjectEditor::SetGroupTranslate( const NiPoint3& trans, bool updateDialog )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetTranslate( trans );

	if( updateDialog )
		mTransformDialog->UpdateObject( mObjectGroup );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupRotate( float xangle, float yangle, float zangle, bool updateDialog )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetRotate( xangle, yangle, zangle );

	if( updateDialog )
		mTransformDialog->UpdateObject( mObjectGroup );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::SetGroupScale( float scale, bool updateDialog )
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->SetScale( scale );

	if( updateDialog )
		mTransformDialog->UpdateObject( mObjectGroup );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );
}

void cObjectEditor::Undo( const cObjectTranslating& doing )
{
	mObjectGroup->Undo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Undo( const cObjectRotating& doing )
{
	mObjectGroup->Undo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Undo( const cObjectScaling& doing )
{
	mObjectGroup->Undo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Redo( const cObjectTranslating& doing )
{
	mObjectGroup->Redo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Redo( const cObjectRotating& doing )
{
	mObjectGroup->Redo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Redo( const cObjectScaling& doing )
{
	mObjectGroup->Redo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
}
