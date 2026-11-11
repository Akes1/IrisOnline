/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.11
* 내    용 : 탭 버튼
* 주의사항 : 
*===========================================================================*/
#pragma once

class cTabButtonGroup;

/// 탭 버튼
class cTabButton : public CButton
{
	friend class cTabButtonGroup;

public:
	cTabButton();
	virtual ~cTabButton();

	/// 그룹을 설정
	void SetGroup( cTabButtonGroup* group );

	/// 윈도우를 설정
	void SetWindow( CWnd* wnd );

	/// 체크 여부 설정
	void SetChecked( bool checked );

	/// 체크 여부 리턴
	bool IsChecked() const;

public:
	virtual void DrawItem( LPDRAWITEMSTRUCT ds );

protected:
	void DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawCaption( CDC* dc, LPDRAWITEMSTRUCT ds );

protected:
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	DECLARE_MESSAGE_MAP()

protected:
	/// 그룹
	cTabButtonGroup* mGroup;

	/// 윈도우
	CWnd* mWindow;

	/// 체크 여부
	bool mChecked;

	/// 그룹 내부 인덱스
	unsigned int mIndex;
};

inline void cTabButton::SetWindow( CWnd* wnd )
{
	mWindow = wnd;
}

inline bool cTabButton::IsChecked() const
{
	return mChecked;
}
