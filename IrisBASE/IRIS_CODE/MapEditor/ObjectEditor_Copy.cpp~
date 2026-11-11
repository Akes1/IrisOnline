#include "stdafx.h"
#include "ObjectEditor.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "ObjectInfoDialog.h"

#include "Engine/StaticSceneNode.h"

#include "Doing/DoingManager.h"
#include "Doing/ObjectCopying.h"

void cObjectEditor::CopyGroup()
{
	if( mObjectGroup->IsEmpty() )
		return;

	/// 노드들을 복사하면서 실행 정보를 수집
	typedef tHashSet<cStaticSceneNode*> cSelectedSet;
	const cSelectedSet& selectedSet = mObjectGroup->GetSelectedSet();

	unsigned int numNodes = selectedSet.GetSize();

	cObjectCopyingInfo undo;
	undo.mNumObjects = numNodes;
	cStaticSceneNode** p = undo.mNodes = new cStaticSceneNode*[numNodes];

	cSelectedSet::cConstIterator i = selectedSet.Begin();
	cSelectedSet::cConstIterator end = selectedSet.End();

	for( ; i != end; ++i, ++p )
	{
		cStaticSceneNode* n = Clone( (cStaticSceneNode*)(*i) );
		assert( n && "failed to clone object" );

		*p = n;
	}

	/// 실행 정보를 추가
	DOINGMAN->PushObjectCopying( undo );

	/// 원본 그룹 위치 이동
	SetGroupTranslate( mObjectGroup->GetTranslate() + NiPoint3(0.0f, 0.0f, 300.0f), true );

	/// 뷰를 갱신
	VIEW->Update();

	/// 변경 여부를 설정
	MAIN->SetSceneModified( true );

	/// 정보 다이얼로그를 갱신
	mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
}

void cObjectEditor::Undo( const cObjectCopying& doing )
{
	/// 오브젝트들을 죽임
	unsigned int numNodes = doing.mNumObjects;
	cStaticSceneNode** p = doing.mNodes;

	for( unsigned int i = 0; i < numNodes; ++i, ++p )
	{
		OBJECTEDIT->Kill( *p );
	}
}

void cObjectEditor::Redo( const cObjectCopying& doing )
{
	/// 죽인 오브젝트들을 다시 살림 -_-;
	unsigned int numNodes = doing.mNumObjects;
	cStaticSceneNode** p = doing.mNodes;

	for( unsigned int i = 0; i < numNodes; ++i, ++p )
	{
		OBJECTEDIT->Rebirth( *p );
	}
}
