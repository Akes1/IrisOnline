// CModelView.h : CModelView 클래스의 인터페이스
//
#pragma once

class cApplication;
class cLookAtCamera;

// CModelView 창

class CModelView : public CView
{
	static CModelView* mSingleton;
	DECLARE_DYNCREATE( CModelView )

	// 생성
public:
	CModelView();

	enum
	{
		eSTAGE_CHARACTER = 0,
		eSTAGE_MONSTER,
		eSTAGE_NPC,
	};

	void Init();

	/// 화면 크기
	void Resize( unsigned int width, unsigned int height );

	/// 갱신
	void Update();

	void ChangeStage( unsigned char stage );

	void CreateCharacter( unsigned long modelIndex, unsigned char race, unsigned char gender );
	void CreateMonster( unsigned long monIdx, unsigned long modelIndex );
	void CreateNpc( unsigned long modelIndex );

	/// 
	bool ChangeCharacterParts( unsigned long partIdx, const char* filename, bool hideEar = false );
	bool ChangeCharacterPartsTexture( unsigned long partIdx, const char* filename );
	unsigned int LinkCharacterWeapon( unsigned long linkIdx, const char* filename, float scale );
	void UnLinkCharacterWeapon( unsigned int left, unsigned int right );

	unsigned int LinkCharacterCostume( unsigned long linkIdx, const char* filename );
	void UnLinkCharacterCostume( unsigned int costume1, unsigned int costume2 );

	///
	void ChangeAnimation( unsigned int seq );

	void ChangeBackColor( NiColor cr );

	inline unsigned char GetCurStage() { return mCurStage; }

	// 특성
public:

	// 작업
public:

	// 재정의
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void OnDraw( CDC* dc );

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

	// 구현
public:
	virtual ~CModelView();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL()
protected:
	virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
	//}}AFX_VIRTUAL

	// 메시지 맵 함수를 생성했습니다.
protected:
	afx_msg int OnCreate( LPCREATESTRUCT cs );
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize( UINT type, int cx, int cy );
	afx_msg void OnMouseMove( UINT flags, CPoint point );
	afx_msg BOOL OnMouseWheel( UINT flags, short zdelta, CPoint point );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );
	afx_msg void OnRButtonDown( UINT flags, CPoint point );
	afx_msg void OnRButtonUp( UINT flags, CPoint point );
	afx_msg void OnMButtonDown( UINT flags, CPoint point );
	afx_msg void OnMButtonUp( UINT flags, CPoint point );
	afx_msg void OnContextMenu( CWnd* pWnd, CPoint pos );
//	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static CModelView* GetSingleton();

private:
	/// 어플리케이션
	cApplication* mApplication;

	cLookAtCamera* mLookAtCamera;

	bool mInit;

	CMenu		m_cmenuCtxt;

	unsigned char mCurStage;
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

inline
CModelView* CModelView::GetSingleton()
{
	return mSingleton;
}

#define VIEW CModelView::GetSingleton()
