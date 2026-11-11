/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.03.09
* 내    용 : 영역 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSceneNode;
class cAreaSceneNode;
class cAreaTransformDialog;
class cAreaLightDialog;
class cAreaFogDialog;
class cAreaEtcDialog;

/// 영역 편집자
/// 맵상의 영역들을 편집한다.
class cAreaEditor
{
	static cAreaEditor* mSingleton;
	friend class cMapEditorView;
	friend class cObjectEditor;

public:
	cAreaEditor();
	~cAreaEditor();

	/// 지움
	void Clear();

	/// 렌더링
	void Render();

	/// 초기화
	void Init();

	/// 저장
	bool Save( cFileSaver& saver );

	/// 영역을 생성
	bool CreateArea();

	/// 영역을 복사
	void CopyArea();

	/// 픽킹
	bool Pick( CPoint point );

	/// 선택
	void DeselectAll();
	void SelectArea( cAreaSceneNode* node, bool force = false );

	/// 영역을 지형에 부착
	void AttachAreaToTerrain( CPoint point );

	/// 영역을 오브젝트에 부착
	void AttachAreaToObject( CPoint point );

	/// 영역을 삭제
	void DeleteArea();

	/// 영역 이동
	void SetAreaTranslate( const NiPoint3& trans, bool updateDialog );

	/// 영역 반지름
	void SetAreaRadius( float scale, bool updateDialog );

	/// 영역 인덱스
	void SetAreaGroupIndex( unsigned int groupIndex );

	/// 주변광
	void SetSkyAmbient( const NiColor& color );
	//void SetSkyDiffuse( const NiColor& color );
	void SetSkyDimmer( float dimmer );
	void SetTerrainAmbient( const NiColor& color );
	//void SetTerrainDiffuse( const NiColor& color );
	void SetTerrainDimmer( float dimmer );
	void SetStaticObjectAmbient( const NiColor& color );
	//void SetStaticObjectDiffuse( const NiColor& color );
	void SetStaticObjectDimmer( float dimmer );
	void SetDynamicObjectAmbient( const NiColor& color );
	//void SetDynamicObjectDiffuse( const NiColor& color );
	void SetDynamicObjectDimmer( float dimmer );

	/// 안개
	void SetFog( bool enabled, const NiColor& color, float depth );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );

	/// 다이얼로그
	void SetTransformDialog( cAreaTransformDialog* dlg );
	void SetLightDialog( cAreaLightDialog* dlg );
	void SetFogDialog( cAreaFogDialog* dlg );
	void SetEtcDialog( cAreaEtcDialog* dlg );

	/// 영역의 수
	unsigned int GetNumAreas() const;

private:
	/// 해당 영역을 제거
	bool Kill( cAreaSceneNode* node );

	/// 해당 장면 노드를 복제
	/// 복제된 새로운 장면 노드를 리턴한다.
	cAreaSceneNode* Clone( cAreaSceneNode* node );

private:
	/// 메시지 처리
	void OnMouseMove( CPoint point );
	void OnLButtonDown_Transform( CPoint point );
	void OnRButtonDown_Transform( CPoint point, bool ctrl );
	void OnKeyDown( UINT c );

public:
	/// 단일체를 리턴
	static cAreaEditor* GetSingleton();

protected:
	/// 배치 다이얼로그
	cAreaTransformDialog* mTransformDialog;

	/// 주변광 다이얼로그
	cAreaLightDialog* mLightDialog;

	/// 안개 다이얼로그
	cAreaFogDialog* mFogDialog;

	/// 기타 속성 다이얼로그
	cAreaEtcDialog* mEtcDialog;

	/// 생성된 영역 셑
	typedef tHashSet<cAreaSceneNode*> cCreatedSet;
	cCreatedSet mCreatedSet;

	/// 충돌 검사용 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 선택된 영역
	cAreaSceneNode* mSelectedNode;

	/// 경계 구
	/// 조명 노드들을 표시하는데 사용한다.
	NiMeshPtr mSphereGeom;
	NiMaterialPropertyPtr mMatProp;

	/// 마우스 좌표
	CPoint mMousePos;
};

inline
void cAreaEditor::SetTransformDialog( cAreaTransformDialog* dlg )
{
	assert( dlg && "null area transform dialog" );

	mTransformDialog = dlg;
}

inline
void cAreaEditor::SetLightDialog( cAreaLightDialog* dlg )
{
	assert( dlg && "null area light dialog" );

	mLightDialog = dlg;
}

inline
void cAreaEditor::SetFogDialog( cAreaFogDialog* dlg )
{
	assert( dlg && "null area fog dialog" );

	mFogDialog = dlg;
}

inline
void cAreaEditor::SetEtcDialog( cAreaEtcDialog* dlg )
{
	assert( dlg && "null area etc dialog" );

	mEtcDialog = dlg;
}

inline
unsigned int cAreaEditor::GetNumAreas() const
{
	return mCreatedSet.GetSize();
}

inline
cAreaEditor* cAreaEditor::GetSingleton()
{
	return mSingleton;
}

#define AREAEDIT cAreaEditor::GetSingleton()
