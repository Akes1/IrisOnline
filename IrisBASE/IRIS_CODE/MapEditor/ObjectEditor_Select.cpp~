#include "stdafx.h"
#include "ObjectEditor.h"

#include "MapEditorView.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"

#include "Doing/DoingManager.h"
#include "Doing/ObjectSelecting.h"

#include "Engine/StaticSceneNode.h"

void cObjectGroup::Select( cStaticSceneNode* node, bool ctrl )
{
	assert( node );

	/// 이미 선택되었는지 검사
	bool alreadySelected = false;
	cSelectedSet::cIterator i = mSelectedSet.Find( node );

	if( i != mSelectedSet.End() )
		alreadySelected = true;

	if( mSelectedSet.GetSize() == 1 && alreadySelected == true )
		return;

	/// Undo 정보를 수집
	cObjectSelectingInfo undo;
	CollectUnSelecting( &undo );
	
	/// 노드들을 탈착
	DetachAll();

	/// 선택된 노드를 추가 또는 해제
	if( ctrl == false )
	{
		mSelectedSet.Clear();
		mSelectedSet.Insert( node );
	}
	else
	{
		if( alreadySelected )
			mSelectedSet.Erase( i );
		else
			mSelectedSet.Insert( node );
	}

	/// 노드들을 부착
	AttachAll();

	/// Redo 정보를 수집
	cObjectSelectingInfo redo;
	CollectReSelecting( &redo );

	/// 실행 정보를 추가
	DOINGMAN->PushObjectSelecting( undo, redo );
}

void cObjectGroup::Select( const tHashSet<cStaticSceneNode*>& nodeSet )
{
	/// Undo 정보를 수집
	cObjectSelectingInfo undo;
	CollectUnSelecting( &undo );

	/// 노드들을 탈착
	DetachAll();

	/// 선택된 노드를 모두 해제
	mSelectedSet.Clear();

	/// 노드들을 추가
	typedef tHashSet<cStaticSceneNode*> cNodeSet;
	cNodeSet::cConstIterator i = nodeSet.Begin();
	cNodeSet::cConstIterator end = nodeSet.End();

	for( ; i != end; ++i )
	{
		mSelectedSet.Insert( *i );
	}

	/// 노드들을 부착
	AttachAll();

	/// Redo 정보를 수집
	cObjectSelectingInfo redo;
	CollectReSelecting( &redo );

	/// 실행 정보를 추가
	DOINGMAN->PushObjectSelecting( undo, redo );
}

void cObjectGroup::Deselect()
{
	/// Undo 정보를 수집
	cObjectSelectingInfo undo;
	CollectUnSelecting( &undo );

	/// 노드들을 탈착
	DetachAll();

	/// 선택된 노드를 모두 해제
	mSelectedSet.Clear();

	/// Redo 정보를 수집
	cObjectSelectingInfo redo;
	CollectReSelecting( &redo );

	/// 실행 정보를 추가
	DOINGMAN->PushObjectSelecting( undo, redo );
}

void cObjectGroup::CollectUnSelecting( cObjectSelectingInfo* pundo )
{
	assert( pundo && "null doing info" );

	unsigned int numNodes = mSelectedSet.GetSize();

	if( numNodes == 0 )
	{
		pundo->mTranslate = mNiNode->GetTranslate();
		pundo->mRotate = mNiNode->GetRotate();
		pundo->mScale = mNiNode->GetScale();
	}
	else
	{
		pundo->mTranslate = mNiNode->GetTranslate();
		pundo->mRotate = mNiNode->GetRotate();
		pundo->mScale = mNiNode->GetScale();
		pundo->mNumObjects = numNodes;
		cObjectSelectingInfo::cObjectInfo* p = pundo->mObjectInfos = new cObjectSelectingInfo::cObjectInfo[numNodes];

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
}

void cObjectGroup::CollectReSelecting( cObjectSelectingInfo* predo )
{
	assert( predo && "null doing info" );

	unsigned int numNodes = mSelectedSet.GetSize();

	if( numNodes == 0 )
	{
		predo->mTranslate = mNiNode->GetTranslate();
		predo->mRotate = mNiNode->GetRotate();
		predo->mScale = mNiNode->GetScale();
	}
	else
	{
		predo->mTranslate = mNiNode->GetTranslate();
		predo->mRotate = mNiNode->GetRotate();
		predo->mScale = mNiNode->GetScale();
		predo->mNumObjects = numNodes;
		cObjectSelectingInfo::cObjectInfo* p = predo->mObjectInfos = new cObjectSelectingInfo::cObjectInfo[numNodes];

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
}

void cObjectGroup::Undo( const cObjectSelecting& doing )
{
	/// 지움
	DetachAll();
	mSelectedSet.Clear();

	/// Undo
	const cObjectSelectingInfo& undo = doing.mUndo;
	mNiNode->SetTranslate( undo.mTranslate );
	mNiNode->SetRotate( undo.mRotate );
	mNiNode->SetScale( undo.mScale );

	unsigned int numNodes = undo.mNumObjects;
	cObjectSelectingInfo::cObjectInfo* p = undo.mObjectInfos;

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
}

void cObjectGroup::Redo( const cObjectSelecting& doing )
{
	/// 지움
	DetachAll();
	mSelectedSet.Clear();

	/// Redo
	const cObjectSelectingInfo& redo = doing.mRedo;
	mNiNode->SetTranslate( redo.mTranslate );
	mNiNode->SetRotate( redo.mRotate );
	mNiNode->SetScale( redo.mScale );

	unsigned int numNodes = redo.mNumObjects;
	cObjectSelectingInfo::cObjectInfo* p = redo.mObjectInfos;

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
}

bool cObjectEditor::SelectObject( const char* pathName )
{
	cString fileName;
	::GetFileName( &fileName, pathName );

	bool found = false;
	bool first = true;
	cCreatedSet::cIterator i = mCreatedSet.Begin();
	cCreatedSet::cIterator iend = mCreatedSet.End();

	for( ; i != iend; ++i )
	{
		cStaticSceneNode* n = *i;

		if( n->GetFileName() == fileName )
		{
			found = true;

			if( first )
			{
				first = false;
				mObjectGroup->Select( n, false );
			}
			else
			{
				mObjectGroup->Select( n, true );
			}
		}
	}

	if( found )
	{
		/// 뷰를 갱신
		VIEW->Update();

		/// 다이얼로그를 갱신
		mTransformDialog->UpdateObject( mObjectGroup );
		mPropertyDialog->UpdateObject( mObjectGroup );
	}

	return found;
}

void cObjectEditor::SelectAll()
{
	if( mCreatedSet.IsEmpty() == false )
	{
		mObjectGroup->Select( mCreatedSet );
		mTransformDialog->UpdateObject( mObjectGroup );
		mPropertyDialog->UpdateObject( mObjectGroup );
	}
}

void cObjectEditor::DeselectAll()
{
	if( mCreatedSet.IsEmpty() == false && mObjectGroup->GetSize() > 0 )
	{
		mObjectGroup->Deselect();
		mTransformDialog->UpdateObject( mObjectGroup );
		mPropertyDialog->UpdateObject( mObjectGroup );
	}
}

void cObjectEditor::Undo( const cObjectSelecting& doing )
{
	mObjectGroup->Undo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
	mPropertyDialog->UpdateObject( mObjectGroup );
}

void cObjectEditor::Redo( const cObjectSelecting& doing )
{
	mObjectGroup->Redo( doing );
	mTransformDialog->UpdateObject( mObjectGroup );
	mPropertyDialog->UpdateObject( mObjectGroup );
}
