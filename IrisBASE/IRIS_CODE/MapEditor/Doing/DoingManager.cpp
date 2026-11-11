#include "stdafx.h"
#include "DoingManager.h"

#include "Doing.h"
#include "TerrainBuilding.h"
#include "TerrainPainting.h"
#include "TerrainDetailing.h"
#include "ObjectCreating.h"
#include "ObjectSelecting.h"
#include "ObjectDeleting.h"
#include "ObjectCopying.h"
#include "ObjectTransforming.h"
#include "NaviMeshBuilding.h"
#include "NaviFieldPainting.h"

#include "../MapEditorView.h"

cDoingManager* cDoingManager::mSingleton = 0;

cDoingManager::cDoingManager()
{
	assert( mSingleton == 0 && "bad singleton!" );
	mSingleton = this;
}

cDoingManager::~cDoingManager()
{
	ClearAll();

	mSingleton = 0;
}

void cDoingManager::ClearTerrain()
{
	cUndoList::cIterator i = mUndoList.Begin();
	cUndoList::cIterator end = mUndoList.End();

	for( ; i != end; )
	{
		cDoing* d = *i;

		if( d->mType & eDOING_TERRAIN )
		{
			delete d;
			i = mUndoList.Erase( i );
		}
		else
		{
			++i;
		}
	}
}

void cDoingManager::ClearObject()
{
	cUndoList::cIterator i = mUndoList.Begin();
	cUndoList::cIterator end = mUndoList.End();

	for( ; i != end; )
	{
		cDoing* d = *i;

		if( d->mType & eDOING_OBJECT )
		{
			delete d;
			i = mUndoList.Erase( i );
		}
		else
		{
			++i;
		}
	}
}

void cDoingManager::ClearNaviMesh()
{
	cUndoList::cIterator i = mUndoList.Begin();
	cUndoList::cIterator end = mUndoList.End();

	for( ; i != end; )
	{
		cDoing* d = *i;

		if( d->mType & eDOING_NAVIMESH )
		{
			delete d;
			i = mUndoList.Erase( i );
		}
		else
		{
			++i;
		}
	}
}

void cDoingManager::ClearNaviField()
{
	cUndoList::cIterator i = mUndoList.Begin();
	cUndoList::cIterator end = mUndoList.End();

	for( ; i != end; )
	{
		cDoing* d = *i;

		if( d->mType & eDOING_NAVIFIELD )
		{
			delete d;
			i = mUndoList.Erase( i );
		}
		else
		{
			++i;
		}
	}
}

void cDoingManager::ClearAll()
{
	cUndoList::cIterator i = mUndoList.Begin();
	cUndoList::cIterator end = mUndoList.End();

	for( ; i != end; ++i )
	{
		delete (*i);
	}

	mUndoList.Clear();
	ClearRedo();
}

void cDoingManager::ClearRedo()
{
	cUndoList::cIterator i = mRedoList.Begin();
	cUndoList::cIterator end = mRedoList.End();

	for( ; i != end; ++i )
	{
		delete (*i);
	}

	mRedoList.Clear();
}

void cDoingManager::CheckUndo()
{
	if( mUndoList.GetSize() > 10000 )
	{
		cUndoList::cIterator i = mUndoList.Begin();

		for( unsigned int c = 0 ; c < 5000 ; ++c )
		{
			delete (*i);
			i = mUndoList.Erase( i );
		}
	}
}

bool cDoingManager::Undo()
{
	if( mUndoList.IsEmpty() )
		return false;

	/// Undo
	cDoing* d = mUndoList.Back();
	mUndoList.PopBack();
	d->Undo();

	/// Redo 리스트에 추가
	mRedoList.PushBack( d );

	/// 뷰를 갱신
	VIEW->Update();
	return true;
}

bool cDoingManager::Redo()
{
	if( mRedoList.IsEmpty() )
		return false;

	/// Redo
	cDoing* d = mRedoList.Back();
	mRedoList.PopBack();
	d->Redo();

	/// Undo 리스트에 추가
	mUndoList.PushBack( d );

	/// 뷰를 갱신
	VIEW->Update();
	return true;
}

void cDoingManager::PushTerrainBuilding( const cTerrainBuildingInfo& info )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cTerrainBuilding* d = new cTerrainBuilding( info );
	mUndoList.PushBack( d );
}

void cDoingManager::PushTerrainPainting( const cTerrainPaintingInfo& info )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cTerrainPainting* d = new cTerrainPainting( info );
	mUndoList.PushBack( d );
}

void cDoingManager::PushTerrainDetailing( const cTerrainDetailingInfo& info )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cTerrainDetailing* d = new cTerrainDetailing( info );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectCreating( const cObjectCreatingInfo& undo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectCreating* d = new cObjectCreating( undo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectSelecting( const cObjectSelectingInfo& undo, const cObjectSelectingInfo& redo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectSelecting* d = new cObjectSelecting( undo, redo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectDeleting( const cObjectDeletingInfo& undo, const cObjectDeletingInfo& redo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectDeleting* d = new cObjectDeleting( undo, redo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectCopying( const cObjectCopyingInfo& undo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectCopying* d = new cObjectCopying( undo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectTranslating( const NiPoint3& undo, const NiPoint3& redo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectTranslating* d = new cObjectTranslating( undo, redo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectRotating( const NiMatrix3& undo, const NiMatrix3& redo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectRotating* d = new cObjectRotating( undo, redo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushObjectScaling( float undo, float redo )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cObjectScaling* d = new cObjectScaling( undo, redo );
	mUndoList.PushBack( d );
}

void cDoingManager::PushNaviMeshBuilding( const cNaviMeshBuildingInfo& info )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cNaviMeshBuilding* d = new cNaviMeshBuilding( info );
	mUndoList.PushBack( d );
}

void cDoingManager::PushNaviFieldPainting( const cNaviFieldPaintingInfo& info )
{
	/// Undo 리스트를 검사
	CheckUndo();

	/// Redo 리스트를 비움
	ClearRedo();

	/// 실행 정보를 생성해서 Undo 리스트에 추가
	cNaviFieldPainting* d = new cNaviFieldPainting( info );
	mUndoList.PushBack( d );
}
