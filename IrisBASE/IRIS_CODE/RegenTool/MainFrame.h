/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.02
* 내    용 : 메인 프레임
* 주의사항 : 
*===========================================================================*/
#pragma once

class cTabWindow;
class cRegenToolView;
class cObjectEditor;

/// 메인 프레임
class cMainFrame : public CFrameWnd
{
	static cMainFrame* mSingleton;
	DECLARE_DYNAMIC(cMainFrame)

public:
	cMainFrame();
	virtual ~cMainFrame();

	/// 맵을 로딩: 지형, 네비필드, 네비메시, 장면 ...
	void LoadMap( cString pathName );

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
	afx_msg void OnSaveAll();
	afx_msg void OnEscape();
	DECLARE_MESSAGE_MAP()

public:
	/// 단일체를 리턴
	static cMainFrame* GetSingleton();

protected:
	///// 탭 윈도우
	//cTabWindow* mTabWindow;

	///// 뷰
	//cRegenToolView* mView;

	/// 스플리터
	CSplitterWnd mSplitter;

	/// 파일 이름
	//cString mTerrainName;
};

inline
cMainFrame* cMainFrame::GetSingleton()
{
	return mSingleton;
}

#define MAIN cMainFrame::GetSingleton()
