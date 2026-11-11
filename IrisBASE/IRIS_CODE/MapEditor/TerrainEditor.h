/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 지형 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Lines.h"

class cBox;
class cFreeCamera;
class cSceneNode;
class cTerrainLeafNode;
class cTerrainBuildDialog;
class cTerrainPaintDialog;
class cTerrainOptionDialog;
class cTerrainInfoDialog;
class cStringTable;

/// 지형 렌더링 모드
enum eTerrainViewMode
{
	eTERRAIN_VIEW_WIREFRAME,
	eTERRAIN_VIEW_SOLID,
	eTERRAIN_VIEW_TEXTURED,
	eTERRAIN_VIEW_WIRETEXTURED
};

/// 타일 그룹
/// 선택된 지형 리프 노드들을 하나로 묶는 역할을 한다.
class cTileGroup
{
	friend class cTerrainEditor;

public:
	cTileGroup();
	~cTileGroup();

	/// 지움
	void Clear();

	/// 렌더링
	void Render();

	/// 리프 노드를 선택
	bool Select( cTerrainLeafNode* node );
	bool Add( cTerrainLeafNode* node );
	bool Remove( cTerrainLeafNode* node );

	/// 디테일 텍스처
	int GetTextureIndex0() const;
	int GetTextureIndex1() const;
	int GetTextureIndex2() const;

	/// 경계 상자를 리턴
	cBox GetBoundBox() const;

	/// 리프 노드 수를 리턴
	unsigned int GetSize() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 선택된 리프 노드 셑을 리턴
	const tHashSet<cTerrainLeafNode*>& GetSelectedSet() const;

private:
	/// 가시 여부
	void SetVisible( bool visible );

	/// 디테일 텍스처
	void SetDetailTextures( unsigned int index0, unsigned int index1, unsigned int index2 );

private:
	/// 경계 상자
	/// 선택된 리프 노드들을 표시하는데 사용한다.
	NiMeshPtr mBoxLines;
	NiMaterialPropertyPtr mMatProp;
	NiWireframePropertyPtr mWireProp;

	/// 선택된 리프 노드 셑
	typedef tHashSet<cTerrainLeafNode*> cSelectedSet;
	cSelectedSet mSelectedSet;
};

inline
unsigned int cTileGroup::GetSize() const
{
	return mSelectedSet.GetSize();
}

inline
bool cTileGroup::IsEmpty() const
{
	return mSelectedSet.IsEmpty();
}

inline
const tHashSet<cTerrainLeafNode*>& cTileGroup::GetSelectedSet() const
{
	return mSelectedSet;
}

/// 지형 편집자
/// 맵상의 지형 리프 노드들을 편집하고
/// 지형 파일(*.terrain)을 저장, 로딩한다.
class cTerrainEditor
{
	static cTerrainEditor* mSingleton;
	friend class cMapEditorView;

public:
	cTerrainEditor();
	~cTerrainEditor();

	/// 지움
	void Clear();

	/// 처리
	void Process();

	/// 렌더링
	void Render( bool group );

	/// 초기화
	void Init( unsigned int resolution, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );

	/// 로딩
	bool Load( const cString& pathName );

	/// 지형
	bool Save();
	bool SaveAs( const cString& pathName );

	/// 로그
	void ExportLog( cStringTable& table, unsigned int& row );

	/// 줌 카메라
	void ZoomPersp( cFreeCamera* cam );
	void ZoomOrtho( cFreeCamera* cam );

	/// 높이맵
	void ClearHeightMap();
	bool ImportHeightMap( const cString& pathName );
	bool ExportHeightMap( const cString& pathName );

	/// 알파맵
	void ClearAlphaMap();
	bool ImportAlphaMap( const cString& pathName );
	bool ExportAlphaMap( const cString& pathName );

	/// 색상맵
	void ClearColorMap();
	bool ImportColorMap( const cString& pathName );
	bool ExportColorMap( const cString& pathName );

	/// 스케일
	void AdjustHeight( float scale );

	/// 네비메시
	void SyncAllToNaviMesh();

	/// 다이얼로그
	void SetBuildDialog( cTerrainBuildDialog* dlg );
	void SetPaintDialog( cTerrainPaintDialog* dlg );
	void SetOptionDialog( cTerrainOptionDialog* dlg );
	void SetInfoDialog( cTerrainInfoDialog* dlg );

	/// Level of detail
	void SetLodEnabled( bool enabled );

	/// 가시 여부
	void SetVisible( bool visible );

	/// 텍스처
	bool LoadTexture( const cString& pathName );
	void UnLoadTexture( unsigned int index );
	void SetDetailTextures( unsigned int index0, unsigned int index1, unsigned int index2 );
	void SetDetailTexturesToAll( unsigned int index0, unsigned int index1, unsigned int index2 );

	/// 주변광
	void SetAmbientLightAmbient( const NiColor& color );
	void SetAmbientLightDiffuse( const NiColor& color );
	void SetAmbientLightDimmer( float dimmer );
	NiAmbientLight* GetAmbientLight() const;

	/// 안개
	void SetFog( bool enabled, const NiColor& color, float depth );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );

	/// 뷰 모드
	void SetViewMode( eTerrainViewMode viewMode );

	/// 픽킹 위치 변경 여부
	void SetPickPosChanged( bool changed );

	/// 경로 이름
	const cString& GetPathName() const;

	/// 해상도
	unsigned int GetResolution() const;

	/// 정점당 미터수
	float GetMetersPerVertex() const;

	/// 미터당 단위수
	unsigned int GetUnitsPerMeter() const;

private:
	/// 메시지를 처리
	void OnTimer_Build();
	void OnTimer_Paint();
	void OnMouseMove_Build( CPoint point );
	void OnMouseMove_Paint( CPoint point );
	void OnLButtonDown_Build( CPoint point );
	void OnLButtonDown_Paint( CPoint point );
	void OnLButtonUp();
	void OnKeyDown( UINT c );

public:
	/// 단일체
	static cTerrainEditor* GetSingleton();

protected:
	/// 다이얼로그
	cTerrainBuildDialog* mBuildDialog;
	cTerrainPaintDialog* mPaintDialog;
	cTerrainOptionDialog* mOptionDialog;
	cTerrainInfoDialog* mInfoDialog;

	/// 픽킹된 장면 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 그룹 지형
	cTileGroup* mGroup;

	/// 드로잉 여부
	bool mDrawing;

	/// 픽킹 위치
	NiPoint3 mPickPos;
	NiPoint3 mPickOldPos;
	bool mPickPosChanged;

	/// 편집 원
	/// 선택 범위를 표시하는데 사용한다.
	cCircleLines mInnerLines;
	cCircleLines mOuterLines;

	/// 마우스 좌표
	CPoint mMousePos;

	/// 경로 이름
	cString mPathName;
};

inline
void cTerrainEditor::SetBuildDialog( cTerrainBuildDialog* dlg )
{
	assert( dlg );

	mBuildDialog = dlg;
}

inline
void cTerrainEditor::SetPaintDialog( cTerrainPaintDialog* dlg )
{
	assert( dlg );

	mPaintDialog = dlg;
}

inline
void cTerrainEditor::SetOptionDialog( cTerrainOptionDialog* dlg )
{
	assert( dlg );

	mOptionDialog = dlg;
}

inline
void cTerrainEditor::SetInfoDialog( cTerrainInfoDialog* dlg )
{
	assert( dlg );

	mInfoDialog = dlg;
}

inline
void cTerrainEditor::SetPickPosChanged( bool changed )
{
	mPickPosChanged = changed;
}

inline
const cString& cTerrainEditor::GetPathName() const
{
	return mPathName;
}

inline
cTerrainEditor* cTerrainEditor::GetSingleton()
{
	return mSingleton;
}

#define TERRAINEDIT	cTerrainEditor::GetSingleton()
