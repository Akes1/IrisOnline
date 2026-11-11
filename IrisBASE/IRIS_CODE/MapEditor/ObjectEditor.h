/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 오브젝트 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSceneNode;
class cStaticSceneNode;
class cObjectCreating;
class cObjectSelectingInfo;
class cObjectSelecting;
class cObjectDeletingInfo;
class cObjectDeleting;
class cObjectCopying;
class cObjectTranslating;
class cObjectRotating;
class cObjectScaling;
class cObjectTransformDialog;
class cObjectPropertyDialog;
class cObjectInfoDialog;
class cStringTable;

/// 그룹 오브젝트
/// 오브젝트 그룹을 하나로 묶는 역할을 한다.
class cObjectGroup
{
	friend class cObjectEditor;

public:
	cObjectGroup();
	~cObjectGroup();

	/// 지움
	void Clear();

	/// 렌더링
	void Render();

	/// 오브젝트를 선택
	void Select( cStaticSceneNode* node, bool ctrl );
	void Select( const tHashSet<cStaticSceneNode*>& nodeSet );
	void Deselect();

	/// 오브젝트들을 삭제
	void Delete();

	/// 이동
	void SetTranslate( const NiPoint3& trans );
	const NiPoint3& GetTranslate() const;

	/// 회전
	void SetRotate( float xangle, float yangle, float zangle );
	const NiMatrix3& GetRotate() const;

	/// 배율
	void SetScale( float scale );
	float GetScale() const;

	/// 재질 적용
	void SetMaterialApplied( bool matApplied );
	bool GetMaterialApplied() const;

	/// 주변광 반사율
	void SetAmbient( COLORREF rgb );
	COLORREF GetAmbient() const;

	/// 산란광 반사율
	void SetDiffuse( COLORREF rgb );
	COLORREF GetDiffuse() const;

	/// 발산색
	void SetEmissive( COLORREF rgb );
	COLORREF GetEmissive() const;

	/// 이름을 리턴
	const char* GetFileName() const;

	/// 가시 레벨
	void SetVisibleLevel( unsigned int level );
	unsigned int GetVisibleLevel() const;

	/// 차폐 레벨
	void SetOccludeLevel( unsigned int level );
	unsigned int GetOccludeLevel() const;

	/// 안개 적용
	void SetFogApplied( bool fogApplied );
	bool GetFogApplied() const;

	/// 오브젝트 수를 리턴
	unsigned int GetSize() const;

	/// 비어있는지 여부를 리턴
	bool IsEmpty() const;

	/// 선택된 장면 노드 셑을 리턴
	const tHashSet<cStaticSceneNode*>& GetSelectedSet() const;

	typedef tHashSet<cStaticSceneNode*> cSelectedSet;
private:
	/// 노드들을 탈부착
	void DetachAll();
	void AttachAll();

	/// 백업
	/// Undo에 필요한 정보를 수집한다.
	void CollectUnSelecting( cObjectSelectingInfo* info );
	void CollectUnDeleting( cObjectDeletingInfo* info );

	/// 갱신
	/// Redo에 필요한 정보를 수집한다.
	void CollectReSelecting( cObjectSelectingInfo* info );
	void CollectReDeleting( cObjectDeletingInfo* info );

	/// Undo
	void Undo( const cObjectSelecting& doing );
	void Undo( const cObjectDeleting& doing );
	void Undo( const cObjectTranslating& doing );
	void Undo( const cObjectRotating& doing );
	void Undo( const cObjectScaling& doing );

	/// Redo
	void Redo( const cObjectSelecting& doing );
	void Redo( const cObjectDeleting& doing );
	void Redo( const cObjectTranslating& doing );
	void Redo( const cObjectRotating& doing );
	void Redo( const cObjectScaling& doing );

private:
	/// 엔진 노드
	NiNodePtr mNiNode;

	/// 편집 원
	/// 선택된 장면 노드들을 표시하는데 사용한다.
	NiMeshPtr mCircleLines;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;

	/// 선택된 장면 노드 셑
	cSelectedSet mSelectedSet;
};

inline
const NiPoint3& cObjectGroup::GetTranslate() const
{
	return mNiNode->GetTranslate();
}

inline
const NiMatrix3& cObjectGroup::GetRotate() const
{
	return mNiNode->GetRotate();
}

inline
float cObjectGroup::GetScale() const
{
	return mNiNode->GetScale();
}

inline
unsigned int cObjectGroup::GetSize() const
{
	return mSelectedSet.GetSize();
}

inline
bool cObjectGroup::IsEmpty() const
{
	return mSelectedSet.IsEmpty();
}

inline
const tHashSet<cStaticSceneNode*>& cObjectGroup::GetSelectedSet() const
{
	return mSelectedSet;
}

/// 오브젝트 편집자
/// 맵상의 오브젝트(장면 노드)들을 편집하고
/// 장면 파일(*.scene)을 저장, 로딩한다.
class cObjectEditor
{
	static cObjectEditor* mSingleton;
	friend class cMapEditorView;
	friend class cObjectGroup;
	friend class cObjectCreating;
	friend class cObjectSelecting;
	friend class cObjectDeleting;
	friend class cObjectCopying;
	friend class cObjectTranslating;
	friend class cObjectRotating;
	friend class cObjectScaling;

public:
	cObjectEditor();
	~cObjectEditor();

	/// 지움
	void Clear();
	void ClearGarbage();

	/// 렌더링
	void Render( bool group );

	/// 초기화
	void Init();
	void Init( unsigned int gridSize, float metersPerVertex = 1.0f, unsigned int unitsPerMeter = 100 );
	void InitTree();

	/// 로딩
	bool Load( const cString& pathName );
	bool Add( const cString& pathName );
	bool LoadAreaGroup( const cString& pathName );
	bool LoadObjectTransformScript( const cString& pathName );

	/// 저장
	bool Save();
	bool SaveGlobalArea( cFileSaver& saver );
	bool SaveAs( const cString& pathName, bool objectAdded = true, bool lightAdded = true, bool soundAdded = true, bool areaAdded = true );
	bool SaveAreaAs( const cString& pathName );
	bool SaveObjectGroupAs( const cString& pathName );

	/// 로그
	void ExportLog( cStringTable& table, unsigned int& row, unsigned int divides );

	/// 오브젝트 위치, 크기 조절
	void AdjustAllPos( float scale );
	void AdjustAllSize( float scale );

	/// 오브젝트를 생성
	bool CreateObject( const char* pathName );

	/// 오브젝트를 검색
	bool SelectObject( const char* pathName );
	bool Find( cStaticSceneNode* node );

	inline cObjectGroup* GetObjectGroup() { return mObjectGroup; }

	/// 픽킹
	bool Pick( CPoint point, bool ctrl );

	/// 선택
	void SelectAll();
	void DeselectAll();

	/// 오브젝트 그룹
	void AttachGroupToTerrain( CPoint point );
	void AttachGroupToObject( CPoint point );
	void SetGroupTranslate( const NiPoint3& trans, bool updateDialog );
	void SetGroupRotate( float xangle, float yangle, float zangle, bool updateDialog );
	void SetGroupScale( float scale, bool updateDialog );
	void SetGroupAmbient( COLORREF rgb );
	void SetGroupDiffuse( COLORREF rgb );
	void SetGroupEmissive( COLORREF rgb );
	void SetGroupVisibleLevel( unsigned int level );
	void SetGroupOccludeLevel( unsigned int level );
	void SetGroupFogApplied( bool fogApplied );
	void SetGroupMaterialApplied( bool matApplied );
	void DeleteGroup();
	void CopyGroup();

	/// 정보 다이얼로그를 갱신
	void UpdateInfoDialog();

	///
	void SetVisibleLevelToSiblings();
	void SetOccludeLevelToSiblings();
	void SetFogToSiblings();
	void SetMaterialToSiblings();
	void SetAmbientSiblings();
	void SetDiffuseSiblings();
	void SetEmissiveSiblings();

	/// 주변광
	void SetAmbientLightAmbient( unsigned int i, const NiColor& color );
	void SetAmbientLightDiffuse( unsigned int i, const NiColor& color );
	void SetAmbientLightDimmer( unsigned int i, float dimmer );
	NiAmbientLight* GetAmbientLight( unsigned int i ) const;

	/// 안개
	void SetFog( bool enabled, const NiColor& color, float depth );
	void SetFogColor( const NiColor& color );
	void SetFogDepth( float depth );

	/// 다이얼로그를 설정
	void SetTransformDialog( cObjectTransformDialog* dlg );
	void SetPropertyDialog( cObjectPropertyDialog* dlg );
	void SetInfoDialog( cObjectInfoDialog* dlg );

	/// 경로 이름을 리턴
	const cString& GetPathName() const;

private:
	/// Undo
	void Undo( const cObjectCreating& doing );
	void Undo( const cObjectSelecting& doing );
	void Undo( const cObjectDeleting& doing );
	void Undo( const cObjectCopying& doing );
	void Undo( const cObjectTranslating& doing );
	void Undo( const cObjectRotating& doing );
	void Undo( const cObjectScaling& doing );

	/// Redo
	void Redo( const cObjectCreating& doing );
	void Redo( const cObjectSelecting& doing );
	void Redo( const cObjectDeleting& doing );
	void Redo( const cObjectCopying& doing );
	void Redo( const cObjectTranslating& doing );
	void Redo( const cObjectRotating& doing );
	void Redo( const cObjectScaling& doing );

	/// 해당 장면 노드를 제거
	bool Kill( cStaticSceneNode* node );

	/// 해당 장면 노드를 부활
	void Rebirth( cStaticSceneNode* node );

	/// 해당 장면 노드를 복제
	/// 복제된 새로운 장면 노드를 리턴한다.
	cStaticSceneNode* Clone( cStaticSceneNode* node );

	/// 메시지를 처리
	void OnMouseMove( CPoint point );
	void OnLButtonDown_Transform( CPoint point, bool ctrl );
	void OnRButtonDown_Transform( CPoint point, bool ctrl );
	void OnKeyDown( UINT c );

public:
	/// 단일체를 리턴
	static cObjectEditor* GetSingleton();

protected:
	/// 편집 다이얼로그
	cObjectTransformDialog* mTransformDialog;

	/// 속성 다이얼로그
	cObjectPropertyDialog* mPropertyDialog;

	/// 정보 다이얼로그
	cObjectInfoDialog* mInfoDialog;

	/// 생성된 장면 노드 셑
	typedef tHashSet<cStaticSceneNode*> cCreatedSet;
	cCreatedSet mCreatedSet;

	/// 삭제된 장면 노드 셑
	typedef tHashSet<cStaticSceneNode*> cDestroyedSet;
	cDestroyedSet mDestroyedSet;

	/// 픽킹된 장면 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 그룹 오브젝트
	cObjectGroup* mObjectGroup;

	/// 마우스 좌표
	CPoint mMousePos;

	/// 경로 이름
	cString mPathName;
};

inline
void cObjectEditor::SetTransformDialog( cObjectTransformDialog* dlg )
{
	assert( dlg );

	mTransformDialog = dlg;
}

inline
void cObjectEditor::SetPropertyDialog( cObjectPropertyDialog* dlg )
{
	assert( dlg );

	mPropertyDialog = dlg;
}

inline
void cObjectEditor::SetInfoDialog( cObjectInfoDialog* dlg )
{
	assert( dlg );

	mInfoDialog = dlg;
}

inline
const cString& cObjectEditor::GetPathName() const
{
	return mPathName;
}

inline
cObjectEditor* cObjectEditor::GetSingleton()
{
	return mSingleton;
}

#define OBJECTEDIT cObjectEditor::GetSingleton()
