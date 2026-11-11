#include "stdafx.h"
#include "ObjectEditor.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"
#include "ObjectInfoDialog.h"

#include "Doing/DoingManager.h"
#include "Doing/ObjectDeleting.h"

#include "Engine/StaticSceneNode.h"

void cObjectGroup::Delete()
{
	assert( mSelectedSet.GetSize() && "should not delete empty group object" );

	/// Undo 정보를 수집
	cObjectDeletingInfo undo;
	CollectUnDeleting( &undo );

	/// 노드들을 탈착
	DetachAll();

	/// 노드들을 삭제
	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i )
	{
		if( OBJECTEDIT->Kill( *i ) == false )
		{
			assert( 0 && "failed to destroy object" );
		}
	}
	mSelectedSet.Clear();

	/// Redo 정보를 수집
	cObjectDeletingInfo redo;
	CollectReDeleting( &redo );

	/// 실행 정보를 추가
	DOINGMAN->PushObjectDeleting( undo, redo );
}

void cObjectGroup::CollectUnDeleting( cObjectDeletingInfo* undo )
{
	assert( undo );

	unsigned int numNodes = mSelectedSet.GetSize();
	assert( numNodes && "should not delete empty group object" );

	undo->mTranslate = mNiNode->GetTranslate();
	undo->mRotate = mNiNode->GetRotate();
	undo->mScale = mNiNode->GetScale();
	undo->mNumObjects = numNodes;
	cObjectDeletingInfo::cObjectInfo* p = undo->mObjectInfos = new cObjectDeletingInfo::cObjectInfo[numNodes];

	cSelectedSet::cIterator i = mSelectedSet.Begin();
	cSelectedSet::cIterator end = mSelectedSet.End();

	for( ; i != end; ++i, ++p )
	{
		cStaticSceneNode* n = *i;

		p->mNode = n;
		p->mTranslate = n->GetTranslate();
		p->mRotate = n->GetRotate();
		p->mScale = n->GetScale();
	}
}

void cObjectGroup::CollectReDeleting( cObjectDeletingInfo* redo )
{
	assert( redo );

	redo->mTranslate = mNiNode->GetTranslate();
	redo->mRotate = mNiNode->GetRotate();
	redo->mScale = mNiNode->GetScale();
}

void cObjectGroup::Undo( const cObjectDeleting& doing )
{
	/// 지움
	DetachAll();
	mSelectedSet.Clear();

	/// Undo
	const cObjectDeletingInfo& undo = doing.mUndo;
	mNiNode->SetTranslate( undo.mTranslate );
	mNiNode->SetRotate( undo.mRotate );
	mNiNode->SetScale( undo.mScale );

	unsigned int numNodes = undo.mNumObjects;
	cObjectDeletingInfo::cObjectInfo* p = undo.mObjectInfos;

	for( unsigned int i = 0; i < numNodes; ++i, ++p )
	{
		cStaticSceneNode* n = p->mNode;

		mSelectedSet.Insert( n );
		mNiNode->AttachChild( n->GetNiObj() );

		n->SetTranslate( p->mTranslate );
		n->SetRotate( p->mRotate );
		n->SetScale( p->mScale );
	}

	mNiNode->Update( 0.0f );

	/// 오브젝트들을 살림
	p = undo.mObjectInfos;

	for( unsigned int i = 0; i < numNodes; ++i, ++p )
	{
		OBJECTEDIT->Rebirth( p->mNode );
	}
}

void cObjectGroup::Redo( const cObjectDeleting& doing )
{
	/// 지움
	DetachAll();
	mSelectedSet.Clear();

	/// Redo
	/// 부활시킨 오브젝트들을 다시 죽임 -_-;
	const cObjectDeletingInfo& undo = doing.mUndo;

	unsigned int numNodes = undo.mNumObjects;
	cObjectDeletingInfo::cObjectInfo* p = undo.mObjectInfos;

	for( unsigned int i = 0; i < numNodes; ++i, ++p )
	{
		OBJECTEDIT->Kill( p->mNode );
	}

	const cObjectDeletingInfo& redo = doing.mRedo;
	mNiNode->SetTranslate( redo.mTranslate );
	mNiNode->SetRotate( redo.mRotate );
	mNiNode->SetScale( redo.mScale );
	mNiNode->Update( 0.0f );
}

void cObjectEditor::DeleteGroup()
{
	if( mObjectGroup->IsEmpty() )
		return;

	mObjectGroup->Delete();
	mTransformDialog->SetEnabled( false );
	mPropertyDialog->SetEnabled( false );
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
}

void cObjectEditor::Undo( const cObjectDeleting& doing )
{
	mObjectGroup->Undo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
	mPropertyDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Redo( const cObjectDeleting& doing )
{
	mObjectGroup->Redo( doing );
	mTransformDialog->SetEnabled( false );
	mPropertyDialog->SetEnabled( false );
}
