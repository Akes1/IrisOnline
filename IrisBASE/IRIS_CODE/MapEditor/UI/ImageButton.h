/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.11
* 내    용 : 이미지 버튼
* 주의사항 : 
*===========================================================================*/
#pragma once

class cImageButtonGroup;

/// 이미지 버튼
class cImageButton : public CButton
{
	friend class cImageButtonGroup;

public:
	cImageButton();
	virtual ~cImageButton();

	/// 그룹을 설정
	void SetGroup( cImageButtonGroup* group );

	/// 이미지를 설정
	void SetImage( CBitmap* bitmap, unsigned int width, unsigned int height );

	/// 체크 여부
	void SetCheckEnabled( bool enabled );
	void SetChecked( bool checked );
	bool IsChecked() const;

protected:
	virtual void DrawItem( LPDRAWITEMSTRUCT ds );

protected:
	void DrawImage( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawFrame( CDC* dc, LPDRAWITEMSTRUCT ds );
	void DrawBorder( CDC* dc, LPDRAWITEMSTRUCT ds );

protected:
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	DECLARE_MESSAGE_MAP()

protected:
	/// 그룹
	cImageButtonGroup* mGroup;

	/// 그룹 내부 인덱스
	unsigned int mIndex;

	/// 체크 여부
	bool mCheckEnabled;
	bool mChecked;

	/// 이미지
	CBitmap* mBitmap;
	int mBitmapWidth;
	int mBitmapHeight;
};

inline
void cImageButton::SetCheckEnabled( bool enabled )
{
	mCheckEnabled = enabled;
}

inline
bool cImageButton::IsChecked() const
{
	return mChecked;
}
