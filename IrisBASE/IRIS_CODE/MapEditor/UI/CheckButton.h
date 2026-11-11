/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.11
* 내    용 : 체크 버튼
* 주의사항 : 
*===========================================================================*/
#pragma once

class cCheckButtonGroup;

/// 체크 버튼
class cCheckButton : public CButton
{
	friend class cCheckButtonGroup;

public:
	cCheckButton();
	virtual ~cCheckButton();

	/// 그룹을 설정
	void SetGroup( cCheckButtonGroup* group );

	/// 체크 여부 설정
	void SetChecked( bool checked );

	/// 체크 여부 리턴
	bool IsChecked() const;

protected:
	virtual void DrawItem( LPDRAWITEMSTRUCT ds );

protected:
	void DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawCaption( CDC* dc, LPDRAWITEMSTRUCT ds );

protected:
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );
	DECLARE_MESSAGE_MAP()

protected:
	/// 그룹
	cCheckButtonGroup* mGroup;

	/// 체크 여부
	bool mChecked;

	/// 그룹 내부 인덱스
	unsigned int mIndex;
};

inline
bool cCheckButton::IsChecked() const
{
	return mChecked;
}
