/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 맵 에디터 뷰
* 주의사항 : 
*===========================================================================*/
#pragma once

class cDoingManager;
class cTerrainEditor;
class cObjectEditor;
class cLightEditor;
class cSoundEditor;
class cAreaEditor;
class cEnvEditor;
class cNaviMeshEditor;
class cNaviFieldEditor;

class cDialog;
class cTerrainBuildDialog;
class cTerrainPaintDialog;
class cTerrainOptionDialog;
class cTerrainInfoDialog;
class cObjectTransformDialog;
class cObjectPropertyDialog;
class cObjectInfoDialog;
class cLightTransformDialog;
class cLightPropertyDialog;
class cLightInfoDialog;
class cSoundTransformDialog;
class cSoundPropertyDialog;
class cSoundInfoDialog;
class cAreaTransformDialog;
class cAreaLightDialog;
class cAreaFogDialog;
class cAreaEtcDialog;
class cEnvCameraDialog;
class cEnvLevelDistanceDialog;
class cEnvSkyDialog;
class cEnvDirLightDialog;
class cNaviMeshBuildDialog;
class cNaviFieldPaintDialog;
class cNaviFieldInfoDialog;
class cSettingViewDialog;
class cSettingControlDialog;

class cApplication;
class cFreeCamera;
class cLookAtCamera;
class cHero;

const float GAME_CAM_FAR_DIST_METER = 500.0f;
const float GAME_CAM_FAR_DIST_UNIT = 50000.0f;

/// 편집 모드
enum eEditMode
{
	eEDIT_NULL = -1,
	eEDIT_TERRAIN_BUILD = 0,
	eEDIT_TERRAIN_PAINT,
	eEDIT_OBJECT_TRANSFORM,
	eEDIT_LIGHT_TRANSFORM,
	eEDIT_SOUND_TRANSFORM,
	eEDIT_AREA_TRANSFORM,
	eEDIT_NAVIMESH_BUILD,
	eEDIT_NAVIFIELD_PAINT,
	eEDIT_MAX_MODES
};

/// 맵 에디터 뷰
class cMapEditorView : public CView
{
	static cMapEditorView* mSingleton;
	DECLARE_DYNCREATE( cMapEditorView )

public:
	typedef tArray<cFreeCamera*> cCameraArray;

public:
	cMapEditorView();
	virtual ~cMapEditorView();

	/// 초기화
	void Init();

	/// 갱신
	void Update();

	/// 스크린샷
	bool SaveScreenShot( const cString& pathName, unsigned int width, unsigned int height, bool onlyTerrain = false );

	/// 로그
	bool SaveLog( const cString& pathName, unsigned int resolution );

	/// 화면 크기
	void Resize( unsigned int width, unsigned int height );

	/// 다이얼로그
	void SetTerrainBuildDialog( cTerrainBuildDialog* dlg );
	void SetTerrainPaintDialog( cTerrainPaintDialog* dlg );
	void SetTerrainOptionDialog( cTerrainOptionDialog* dlg );
	void SetTerrainInfoDialog( cTerrainInfoDialog* dlg );
	void SetObjectTransformDialog( cObjectTransformDialog* dlg );
	void SetObjectPropertyDialog( cObjectPropertyDialog* dlg );
	void SetObjectInfoDialog( cObjectInfoDialog* dlg );
	void SetLightTransformDialog( cLightTransformDialog* dlg );
	void SetLightPropertyDialog( cLightPropertyDialog* dlg );
	void SetLightInfoDialog( cLightInfoDialog* dlg );
	void SetSoundTransformDialog( cSoundTransformDialog* dlg );
	void SetSoundPropertyDialog( cSoundPropertyDialog* dlg );
	void SetSoundInfoDialog( cSoundInfoDialog* dlg );
	void SetAreaTransformDialog( cAreaTransformDialog* dlg );
	void SetAreaLightDialog( cAreaLightDialog* dlg );
	void SetAreaFogDialog( cAreaFogDialog* dlg );
	void SetAreaEtcDialog( cAreaEtcDialog* dlg );
	void SetEnvCameraDialog( cEnvCameraDialog* dlg );
	void SetEnvLevelDistanceDialog( cEnvLevelDistanceDialog* dlg );
	void SetEnvSkyDialog( cEnvSkyDialog* dlg );
	void SetEnvDirLightDialog( cEnvDirLightDialog* dlg );
	void SetNaviMeshBuildDialog( cNaviMeshBuildDialog* dlg );
	void SetNaviFieldPaintDialog( cNaviFieldPaintDialog* dlg );
	void SetNaviFieldInfoDialog( cNaviFieldInfoDialog* dlg );
	void SetSettingViewDialog( cSettingViewDialog* dlg );
	void SetSettingControlDialog( cSettingControlDialog* dlg );

	/// 편집 모드
	void SetEditMode( eEditMode mode );
	eEditMode GetEditMode() const;

	/// 카메라
	void SelectCamera( unsigned int i, bool makeCurrent = false );
	void ZoomPersp();
	void ZoomOrtho();
	unsigned int NewCamera();
	void DeleteCamera();
	unsigned int LoadCamera( const cString& pathName );
	bool SaveCameraAs( const cString& pathName );

	void ChangeToFreeCamera();
	void ChangeToLookAtCamera();
	void MoveFreeCameraToHero();
	void MoveFreeCameraToSelectObject( bool next );
	void SetDefaultCameraFarDistance( float dist );
	void SetLookAtCameraFarDistance( float dist );

	cFreeCamera* GetDefaultCamera() const;
	cFreeCamera* GetFreeCamera() const;
	cLookAtCamera* GetLookAtCamera() const;

	bool IsPlayerMode();

	/// 주인공
	cHero* GetHero() const;

protected:
	BOOL PreCreateWindow( CREATESTRUCT& cs );
	void OnDraw( CDC* dc );

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

public:
	void OnEditCopy();
	void OnEditDeselectAll();
	void OnEditChangeBackgroundColor();

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize( UINT type, int cx, int cy );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
	afx_msg void OnMouseMove( UINT flags, CPoint point );
	afx_msg BOOL OnMouseWheel( UINT flags, short zdelta, CPoint point );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );
	afx_msg void OnRButtonDown( UINT flags, CPoint point );
	afx_msg void OnRButtonUp( UINT flags, CPoint point );
	afx_msg void OnMButtonDown( UINT flags, CPoint point );
	afx_msg void OnMButtonUp( UINT flags, CPoint point );
	afx_msg void OnKeyDown(UINT c, UINT repeats, UINT flags);
	afx_msg void OnKeyUp(UINT c, UINT repeats, UINT flags);
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cMapEditorView* GetSingleton();

private:
	/// 어플리케이션
	cApplication* mApplication;

	/// 실행 관리자
	cDoingManager* mDoingManager;

	/// 편집자
	cTerrainEditor* mTerrainEditor;
	cObjectEditor* mObjectEditor;
	cLightEditor* mLightEditor;
	cSoundEditor* mSoundEditor;
	cAreaEditor* mAreaEditor;
	cEnvEditor* mEnvEditor;
	cNaviMeshEditor* mNaviMeshEditor;
	cNaviFieldEditor* mNaviFieldEditor;

	/// 다이얼로그
	cTerrainBuildDialog* mTerrainBuildDialog;
	cTerrainPaintDialog* mTerrainPaintDialog;
	cObjectTransformDialog* mObjectTransformDialog;
	cObjectPropertyDialog* mObjectPropertyDialog;
	cLightTransformDialog* mLightTransformDialog;
	cLightPropertyDialog* mLightPropertyDialog;
	cSoundTransformDialog* mSoundTransformDialog;
	cSoundPropertyDialog* mSoundPropertyDialog;
	cAreaTransformDialog* mAreaTransformDialog;
	cNaviMeshBuildDialog* mNaviMeshBuildDialog;
	cNaviFieldPaintDialog* mNaviFieldPaintDialog;
	cSettingViewDialog* mSettingViewDialog;
	cSettingControlDialog* mSettingControlDialog;

	/// 편집 모드
	eEditMode mEditMode;
	cDialog* mEditDialogs[eEDIT_MAX_MODES];

	/// 테스트 여부 (true이면 시점 고정, false이면 시점 자유)
	bool mPlaying;

	/// 카메라
	cCameraArray mCameraArray;
	cFreeCamera* mDefaultCamera;
	cFreeCamera* mFreeCamera;
	cLookAtCamera* mLookAtCamera;

	/// 주인공
	cHero* mHero;
	bool mHeroLines;
};

inline
eEditMode cMapEditorView::GetEditMode() const
{
	return mEditMode;
}

inline
cHero* cMapEditorView::GetHero() const
{
	return mHero;
}

inline
cFreeCamera* cMapEditorView::GetDefaultCamera() const
{
	return mDefaultCamera;
}

inline
cFreeCamera* cMapEditorView::GetFreeCamera() const
{
	return mFreeCamera;
}

inline
cLookAtCamera* cMapEditorView::GetLookAtCamera() const
{
	return mLookAtCamera;
}

inline
cMapEditorView* cMapEditorView::GetSingleton()
{
	return mSingleton;
}

inline
bool cMapEditorView::IsPlayerMode()
{
	return mPlaying;
}

#define VIEW cMapEditorView::GetSingleton()
