#include "stdafx.h"
#include "ObjectEditor.h"

#include "MainFrame.h"
#include "MapEditorView.h"
#include "ObjectTransformDialog.h"
#include "ObjectPropertyDialog.h"
#include "ObjectInfoDialog.h"

#include "Doing/DoingManager.h"
#include "Doing/ObjectCreating.h"

#include "Engine/SceneManager.h"
#include "Engine/StaticSceneNode.h"

bool cObjectEditor::CreateObject( const char* pathName )
{
	/// 현재 경로를 설정
	cString path;
	GetFilePath( &path, pathName );
	SetCurrentDirectory( path.Cstr() );

	/// 장면 노드를 생성
	cStaticSceneNodeParam param;
	param.mPathName = pathName;
	cStaticSceneNode* n = SCENEMAN->CreateStatic( param );

	if( n )
	{
		/// 실행 정보를 수집
		cObjectCreatingInfo undo;
		undo.mNode = n;

		/// 실행 정보를 추가
		DOINGMAN->PushObjectCreating( undo );

		/// 생성 셑에 추가
		mCreatedSet.Insert( n );

		/// 생성된 오브젝트를 선택
		mObjectGroup->Select( n, false );
		mTransformDialog->UpdateObject( mObjectGroup );
		mPropertyDialog->UpdateObject( mObjectGroup );

		/// 뷰를 갱신
		VIEW->Update();

		/// 변경 여부를 설정
		MAIN->SetSceneModified( true );

		/// 정보 다이얼로그를 갱신
		mInfoDialog->UpdateNumObjects( mCreatedSet.GetSize() );
	}
	return n != 0;
}

void cObjectEditor::Undo( const cObjectCreating& doing )
{
	/// 오브젝트를 죽임
	Kill( doing.mUndo.mNode );
}

void cObjectEditor::Redo( const cObjectCreating& doing )
{
	/// 죽인 오브젝트를 다시 살림 -_-;
	Rebirth( doing.mUndo.mNode );
}
