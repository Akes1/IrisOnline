/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 메인 프레임
* 주의사항 : 
*===========================================================================*/
#pragma once

class cTabWindow;
class cMapEditorView;
class cObjectEditor;

/// 메인 프레임
class cMainFrame : public CFrameWnd
{
	static cMainFrame* mSingleton;
	DECLARE_DYNAMIC(cMainFrame)

public:
	cMainFrame();
	virtual ~cMainFrame();

	/// 프로젝트
	void LoadProject( cString pathName );
	void SaveProject( cString pathName );
	void SetCameraFileName( const cString& fileName );
	void SetPaletteFileName( const cString& fileName );

	/// 지형
	void LoadTerrain( cString pathName, bool loadNavi = true );

	/// 장면
	void LoadScene( cString pathName );
	void AddScene( cString pathName );

	/// 사운드
	void LoadSound( cString pathName );

	/// 네비메시
	void LoadNaviMesh( cString pathName );

	/// 네비필드
	void LoadNaviField( cString pathName );

	/// 영역 그룹
	void LoadAreaGroup( const cString& pathName );

	/// 오브젝트 배치 스크립트
	void LoadOscr( const cString& pathName );

	/// 변경 여부
	void SetTerrainModified( const cString& pathName, bool modified );
	void SetTerrainModified( bool modified );
	void SetSceneModified( const cString& pathName, bool modified );
	void SetSceneModified( bool modified );
	void SetSoundSceneModified( const cString& pathName, bool modified );
	void SetSoundSceneModified( bool modified );
	void SetNaviMeshModified( const cString& pathName, bool modified );
	void SetNaviMeshModified( bool modified );
	void SetNaviFieldModified( const cString& pathName, bool modified );
	void SetNaviFieldModified( bool modified );

public:
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
	virtual BOOL OnCreateClient( LPCREATESTRUCT cs, CCreateContext* pcontext );

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnClose();
	afx_msg void OnLoadProject();
	afx_msg void OnSaveProject();
	afx_msg void OnSaveScreenShot();
	afx_msg void OnSaveLog();
	afx_msg void OnTerrainNew();
	afx_msg void OnTerrainOpen();
	afx_msg void OnTerrainSave();
	afx_msg void OnTerrainSaveAs();
	afx_msg void OnTerrainClearHeightMap();
	afx_msg void OnTerrainImportHeightMap();
	afx_msg void OnTerrainExportHeightMap();
	afx_msg void OnTerrainClearAlphaMap();
	afx_msg void OnTerrainImportAlphaMap();
	afx_msg void OnTerrainExportAlphaMap();
	afx_msg void OnTerrainClearColorMap();
	afx_msg void OnTerrainImportColorMap();
	afx_msg void OnTerrainExportColorMap();
	afx_msg void OnTerrainAdjustHeight();
	afx_msg void OnSceneNew();
	afx_msg void OnSceneOpen();
	afx_msg void OnSceneAdd();
	afx_msg void OnSceneSave();
	afx_msg void OnSceneSaveAs();
	afx_msg void OnSceneSaveAreaAs();
	afx_msg void OnSceneSaveObjectGroupAs();
	afx_msg void OnSceneLoadAreaGroup();
	afx_msg void OnSceneLoadObjectTransformScript();
	afx_msg void OnSceneAdjustAllPos();
	afx_msg void OnSceneAdjustAllSize();
	afx_msg void OnNaviMeshSave();
	afx_msg void OnNaviMeshSaveAs();
	afx_msg void OnNaviFieldSave();
	afx_msg void OnNaviFieldSaveAs();
	afx_msg void OnSoundNew();
	afx_msg void OnSoundOpen();
	afx_msg void OnSoundSave();
	afx_msg void OnSoundSaveAs();
	afx_msg void OnEditCopy();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditDeselectAll();
	afx_msg void OnEditChangeBackgroundColor();
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnEscape();
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cMainFrame* GetSingleton();

protected:
	/// 탭 윈도우
	cTabWindow* mTabWindow;

	/// 뷰
	cMapEditorView* mView;

	/// 스플리터
	CSplitterWnd mSplitter;
	CSplitterWnd mLeftSplitter;

	/// 파일 이름
	cString mTerrainName;
	cString mSceneName;
	cString mSoundSceneName;
	cString mNaviMeshName;
	cString mNaviFieldName;
	cString mCameraFileName;
	cString mPaletteFileName;

	/// 파일 변경 여부
	bool mTerrainModified;
	bool mSceneModified;
	bool mSoundModified;
	bool mNaviMeshModified;
	bool mNaviFieldModified;
};

inline
void cMainFrame::SetCameraFileName( const cString& fileName )
{
	mCameraFileName = fileName;
}

inline
void cMainFrame::SetPaletteFileName( const cString& fileName )
{
	mPaletteFileName = fileName;
}

inline
cMainFrame* cMainFrame::GetSingleton()
{
	return mSingleton;
}

#define MAIN cMainFrame::GetSingleton()
