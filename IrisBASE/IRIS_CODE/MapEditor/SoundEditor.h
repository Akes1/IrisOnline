/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.13
* 내    용 : 사운드 편집자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSceneNode;
class cSoundSceneNode;
class cSoundTransformDialog;
class cSoundPropertyDialog;
class cSoundInfoDialog;

/// 사운드 편집자
/// 맵상의 사운드들을 편집한다.
class cSoundEditor
{
	static cSoundEditor* mSingleton;
	friend class cMapEditorView;
	friend class cObjectEditor;

public:
	cSoundEditor();
	~cSoundEditor();

	/// 지움
	void Clear();

	/// 렌더링
	void Render();

	/// 초기화
	void Init();

	///// 로딩
	bool Load( const cString& pathName );

	///// 저장
	bool Save();
	bool SaveAs( const cString& pathName );

	/// 저장
	bool Save( cFileSaver& saver );

	/// 사운드를 생성
	bool CreateSound( const char* pathName );

	/// 픽킹
	bool Pick( CPoint point );

	/// 선택
	void DeselectAll();

	/// 사운드를 지형에 부착
	void AttachSoundToTerrain( CPoint point );

	/// 사운드를 오브젝트에 부착
	void AttachSoundToObject( CPoint point );

	/// 사운드를 삭제
	void DeleteSound();

	/// 사운드를 복사
	void CopySound();

	/// 정보 다이얼로그를 갱신
	void UpdateInfoDialog();

	/// 사운드 이동
	void SetSoundTranslate( const NiPoint3& trans, bool updateDialog );

	/// 사운드 반지름
	void SetSoundRadius( float scale, bool updateDialog );

	/// 사운드 감쇠 거리
	void SetSoundAttenDistance( float dist, bool updateDialog );

	/// 사운드 속성
	void SetSoundLoopCount( int count );
	void SetSoundLoopInterval( float interval );
	void SetSoundVolumeRatio( float ratio );

	/// 다이얼로그
	void SetTransformDialog( cSoundTransformDialog* dlg );
	void SetPropertyDialog( cSoundPropertyDialog* dlg );
	void SetInfoDialog( cSoundInfoDialog* dlg );

	/// 경로 이름을 리턴
	const cString& GetPathName() const;

	/// 사운드 수
	unsigned int GetNumSounds() const;

private:
	/// 해당 사운드를 제거
	bool Kill( cSoundSceneNode* node );

	/// 해당 사운드를 복제
	/// 복제된 새로운 사운드를 리턴한다.
	cSoundSceneNode* Clone( cSoundSceneNode* node );

	/// 메시지를 처리
	void OnMouseMove( CPoint point );
	void OnLButtonDown_Transform( CPoint point );
	void OnRButtonDown_Transform( CPoint point, bool ctrl );
	void OnKeyDown( UINT c );

public:
	/// 단일체를 리턴
	static cSoundEditor* GetSingleton();

protected:
	/// 배치 다이얼로그
	cSoundTransformDialog* mTransformDialog;

	/// 속성 다이얼로그
	cSoundPropertyDialog* mPropertyDialog;

	/// 정보 다이얼로그
	cSoundInfoDialog* mInfoDialog;

	/// 생성된 사운드 셑
	typedef tHashSet<cSoundSceneNode*> cCreatedSet;
	cCreatedSet mCreatedSet;

	/// 픽킹된 노드 배열
	typedef tArray<cSceneNode*> cPickedArray;
	cPickedArray mPickedArray;

	/// 선택된 사운드
	cSoundSceneNode* mSelectedNode;

	/// 편집 원
	/// 사운드 노드들을 표시하는데 사용한다.
	NiMeshPtr mCircleLines;
	//NiPoint3* mPosCoords;
	//NiBool* mFlags;
	NiMaterialPropertyPtr mMatProp;
	NiVertexColorPropertyPtr mVertColorProp;
	NiWireframePropertyPtr mWireProp;

	/// 마우스 좌표
	CPoint mMousePos;

	/// 경로 이름
	cString mPathName;
};

inline
void cSoundEditor::SetTransformDialog( cSoundTransformDialog* dlg )
{
	assert( dlg && "null sound transform dialog" );

	mTransformDialog = dlg;
}

inline
void cSoundEditor::SetPropertyDialog( cSoundPropertyDialog* dlg )
{
	assert( dlg && "null sound property dialog" );

	mPropertyDialog = dlg;
}

inline
void cSoundEditor::SetInfoDialog( cSoundInfoDialog* dlg )
{
	assert( dlg && "null sound info dialog" );

	mInfoDialog = dlg;
}

inline
unsigned int cSoundEditor::GetNumSounds() const
{
	return mCreatedSet.GetSize();
}

inline
cSoundEditor* cSoundEditor::GetSingleton()
{
	return mSingleton;
}

inline
const cString& cSoundEditor::GetPathName() const
{
	return mPathName;
}

#define SOUNDEDIT cSoundEditor::GetSingleton()
