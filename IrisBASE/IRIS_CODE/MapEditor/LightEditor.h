/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 조명 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSceneNode;
class cLightSceneNode;
class cLightTransformDialog;
class cLightPropertyDialog;
class cLightInfoDialog;

/// 조명 편집자
/// 맵상의 조명들을 편집한다.
class cLightEditor
{
	static cLightEditor* mSingleton;
	friend class cMapEditorView;
	friend class cObjectEditor;

public:
	cLightEditor();
	~cLightEditor();

	/// 지움
	void Clear();

	/// 렌더링
	void Render();

	/// 초기화
	void Init();

	/// 저장
	bool Save( cFileSaver& saver );

	/// 조명을 생성
	bool CreateLight();

	/// 픽킹
	bool Pick( CPoint point );

	/// 선택
	void DeselectAll();

	/// 조명을 지형에 부착
	void AttachLightToTerrain( CPoint point );

	/// 조명을 오브젝트에 부착
	void AttachLightToObject( CPoint point );

	/// 조명을 삭제
	void DeleteLight();

	/// 조명을 복사
	void CopyLight();

	/// 정보 다이얼로그를 갱신
	void UpdateInfoDialog();

	/// 조명의 이동
	void SetLightTranslate( const NiPoint3& trans, bool updateDialog );

	/// 조명의 반지름
	void SetLightRadius( float scale, bool updateDialog );

	/// 조명의 색상
	void SetLightAmbient( COLORREF rgb );
	void SetLightDiffuse0( COLORREF rgb );
	void SetLightDiffuse1( COLORREF rgb );
	void SetLightDiffuseAnimTime( float time );
	void SetLightSpecular( COLORREF rgb );

	/// 조명의 감쇠 계수
	void SetLightConstantAtten0( float value );
	void SetLightConstantAtten1( float value );
	void SetLightConstantAttenAnimTime( float value );
	void SetLightLinearAtten( float value );
	void SetLightQuadricAtten( float value );

	/// 다이얼로그
	void SetTransformDialog( cLightTransformDialog* dlg );
	void SetPropertyDialog( cLightPropertyDialog* dlg );
	void SetInfoDialog( cLightInfoDialog* dlg );

	/// 조명 수
	unsigned int GetNumLights() const;

private:
	/// 해당 조명을 제거
	bool Kill( cLightSceneNode* node );

	/// 해당 조명을 복제
	/// 복제된 새로운 조명을 리턴한다.
	cLightSceneNode* Clone( cLightSceneNode* node );

	/// 메시지를 처리
	void OnMouseMove( CPoint point );
	void OnLButtonDown_Transform( CPoint point );
	void OnRButtonDown_Transform( CPoint point, bool ctrl );
	void OnKeyDown( UINT c );

public:
	/// 단일체를 리턴
	static cLightEditor* GetSingleton();

protected:
	/// 배치 다이얼로그
	cLightTransformDialog* mTransformDialog;

	/// 속성 다이얼로그
	cLightPropertyDialog* mPropertyDialog;

	/// 정보 다이얼로그
	cLightInfoDialog* mInfoDialog;

	/// 생성된 조명 셑
	typedef tHashSet<cLightSceneNode*> cCreatedSet;
	cCreatedSet mCreatedSet;

	/// 픽킹된 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 선택된 조명
	cLightSceneNode* mSelectedNode;

	/// 편집 원
	/// 조명 노드들을 표시하는데 사용한다.
	NiMeshPtr mCircleLines;
	//NiPoint3* mPosCoords;
	//NiBool* mFlags;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;

	/// 마우스 좌표
	CPoint mMousePos;
};

inline
void cLightEditor::SetTransformDialog( cLightTransformDialog* dlg )
{
	assert( dlg && "null light transform dialog" );

	mTransformDialog = dlg;
}

inline
void cLightEditor::SetPropertyDialog( cLightPropertyDialog* dlg )
{
	assert( dlg && "null light property dialog" );

	mPropertyDialog = dlg;
}

inline
void cLightEditor::SetInfoDialog( cLightInfoDialog* dlg )
{
	assert( dlg && "null light info dialog" );

	mInfoDialog = dlg;
}

inline
unsigned int cLightEditor::GetNumLights() const
{
	return mCreatedSet.GetSize();
}

inline
cLightEditor* cLightEditor::GetSingleton()
{
	return mSingleton;
}

#define LIGHTEDIT cLightEditor::GetSingleton()
