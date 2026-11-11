/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.20
* 내    용 : 색 상자
* 주의사항 : 
*===========================================================================*/
#pragma once

class cColorBoxGroup;

/// 색 상자
class cColorBox : public CButton
{
	friend class cColorBoxGroup;

public:
	cColorBox();
	virtual ~cColorBox();

	/// 그룹을 설정
	void SetGroup( cColorBoxGroup* group );

	/// 색을 설정
	void SetColor( COLORREF color );

	/// 색을 리턴
	COLORREF GetColor() const;

	/// 체크 여부
	void SetChecked( bool checked );
	bool IsChecked() const;

public:
	virtual void DrawItem( LPDRAWITEMSTRUCT ds );

protected:
	void DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds );

protected:
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	afx_msg void OnLButtonDblClk( UINT flags, CPoint point );
	afx_msg void OnRButtonDown( UINT flags, CPoint point );
	DECLARE_MESSAGE_MAP()

protected:
	/// 그룹
	cColorBoxGroup* mGroup;

	/// 그룹 내부 인덱스
	unsigned int mIndex;

	/// 체크 여부
	bool mChecked;

	/// 색
	COLORREF mColor;
};

inline void cColorBox::SetColor( COLORREF color )
{
	mColor = color;
	Invalidate( FALSE );
}

inline COLORREF cColorBox::GetColor() const
{
	return mColor;
}

inline
bool cColorBox::IsChecked() const
{
	return mChecked;
}
