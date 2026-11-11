/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 네비메시 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSceneNode;
class cNaviMesh;
class cNaviMeshLeafNode;
class cNaviMeshBuildDialog;

/// 네비메시 편집자
/// 맵상의 네비메시 리프 노드들을 편집하고
/// 네비메시 파일(*.navimesh)을 저장, 로딩한다.
class cNaviMeshEditor
{
	static cNaviMeshEditor* mSingleton;
	friend class cMapEditorView;

public:
	cNaviMeshEditor();
	~cNaviMeshEditor();

	/// 지움
	void Clear();

	/// 처리
	void Process();

	/// 렌더링
	void Render();

	/// 네비메시를 새로 만듦
	void Init( unsigned int resolution, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );

	/// 네비메시를 로딩
	bool Load( const cString& pathName );

	/// 네비메시를 저장
	bool Save();
	bool SaveAs( const cString& pathName );

	/// 지형에 맞춤
	void SyncAllToTerrain();

	/// 건축 다이얼로그를 설정
	void SetBuildDialog( cNaviMeshBuildDialog* dlg );

	/// 픽킹 위치 변경 여부를 설정
	void SetPickPosChanged( bool changed );

	/// 경로 이름을 리턴
	const cString& GetPathName() const;

	/// 해상도를 리턴
	unsigned int GetResolution() const;

	/// 정점당 미터수를 리턴
	float GetMetersPerVertex() const;

private:
	/// 메시지를 처리
	void OnTimer_Build();
	void OnMouseMove( CPoint point );
	void OnLButtonDown_Build( CPoint point );
	void OnLButtonUp();

public:
	/// 단일체를 리턴
	static cNaviMeshEditor* GetSingleton();

protected:
	/// 건축 다이얼로그
	cNaviMeshBuildDialog* mBuildDialog;

	/// 네비메시
	cNaviMesh* mNaviMesh;

	/// 픽킹된 장면 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 드로잉 여부
	bool mDrawing;

	/// 픽킹 위치
	NiPoint3 mPickPos;
	NiPoint3 mPickOldPos;
	bool mPickPosChanged;

	/// 편집 원
	/// 선택 범위를 표시하는데 사용한다.
	NiMeshPtr mCircleLines;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;

	/// 마우스 좌표
	CPoint mMousePos;

	/// 경로 이름
	cString mPathName;
};

inline
void cNaviMeshEditor::SetBuildDialog( cNaviMeshBuildDialog* dlg )
{
	assert( dlg && "null navimesh build dialog" );

	mBuildDialog = dlg;
}

inline
void cNaviMeshEditor::SetPickPosChanged( bool changed )
{
	mPickPosChanged = changed;
}

inline
const cString& cNaviMeshEditor::GetPathName() const
{
	return mPathName;
}

inline
cNaviMeshEditor* cNaviMeshEditor::GetSingleton()
{
	return mSingleton;
}

#define NAVIMESHEDIT cNaviMeshEditor::GetSingleton()
