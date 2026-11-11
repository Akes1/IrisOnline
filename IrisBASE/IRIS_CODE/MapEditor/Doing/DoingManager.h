/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.05
* 내    용 : 실행 관리자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cDoing;
class cTerrainBuildingInfo;
class cTerrainPaintingInfo;
class cTerrainDetailingInfo;
class cObjectCreatingInfo;
class cObjectSelectingInfo;
class cObjectDeletingInfo;
class cObjectCopyingInfo;
class cNaviMeshBuildingInfo;
class cNaviFieldPaintingInfo;

/// 실행 관리자
class cDoingManager
{
	static cDoingManager* mSingleton;

public:
	cDoingManager();
	~cDoingManager();

	/// 지움
	void ClearTerrain();
	void ClearObject();
	void ClearNaviMesh();
	void ClearNaviField();
	void ClearAll();

	/// Undo
	bool Undo();

	/// Redo
	bool Redo();

	/// Undo 스택이 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 지형 실행을 추가
	void PushTerrainBuilding( const cTerrainBuildingInfo& info );
	void PushTerrainPainting( const cTerrainPaintingInfo& info );
	void PushTerrainDetailing( const cTerrainDetailingInfo& info );

	/// 오브젝트 실행을 추가
	void PushObjectCreating( const cObjectCreatingInfo& undo );
	void PushObjectSelecting( const cObjectSelectingInfo& undo, const cObjectSelectingInfo& redo );
	void PushObjectDeleting( const cObjectDeletingInfo& undo, const cObjectDeletingInfo& redo );
	void PushObjectCopying( const cObjectCopyingInfo& undo );
	void PushObjectTranslating( const NiPoint3& undo, const NiPoint3& redo );
	void PushObjectRotating( const NiMatrix3& undo, const NiMatrix3& redo );
	void PushObjectScaling( float undo, float redo );

	/// 네비메시 실행을 추가
	void PushNaviMeshBuilding( const cNaviMeshBuildingInfo& info );

	/// 네비필드 실행을 추가
	void PushNaviFieldPainting( const cNaviFieldPaintingInfo& info );

protected:
	/// Redo 리스트를 지움
	void ClearRedo();

	/// Undo 수가 10000이 넘으면, 앞쪽의 5000개를 지운다.
	void CheckUndo();

public:
	/// 단일체를 리턴
	static cDoingManager* GetSingleton();

protected:
	/// Undo 리스트
	typedef tList<cDoing*> cUndoList;
	cUndoList mUndoList;

	/// Redo 리스트
	typedef tList<cDoing*> cRedoList;
	cRedoList mRedoList;
};

inline
bool cDoingManager::IsEmpty() const
{
	return mUndoList.IsEmpty();
}

inline
cDoingManager* cDoingManager::GetSingleton()
{
	return mSingleton;
}

#define DOINGMAN cDoingManager::GetSingleton()
