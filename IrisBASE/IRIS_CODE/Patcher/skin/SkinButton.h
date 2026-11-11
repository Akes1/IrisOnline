/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.20
* 
* 내    용 : 스킨 버튼
* 추가내용 :
*			작업일		작업자	작업내용
*
* 주의사항 : 
*===========================================================================*/
#pragma once

class cSkinComponentResource;
class cSkinButtonResource;

/// 스킨 버튼
class cSkinButton : public CButton
{
public:
	cSkinButton();
	virtual ~cSkinButton();

	/// 생성
	BOOL Create( cSkinComponentResource* resource, CWnd* parent, UINT id );

	/// 체크 여부 설정
	void SetChecked( bool checked );

	/// 체크 여부 리턴
	bool IsChecked();

public:
	virtual void DrawItem( LPDRAWITEMSTRUCT ds );

protected:
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd( CDC* dc );
	afx_msg void OnMouseMove( UINT flags, CPoint point );
	afx_msg void OnLButtonDown( UINT flags, CPoint point );
	afx_msg void OnLButtonUp( UINT flags, CPoint point );
	DECLARE_MESSAGE_MAP()

protected:
	/// 리소스
	cSkinButtonResource* mResource;

	/// 영역
	CRect mRect;

	/// 크기
	int mWidth;
	int mHeight;

	/// 스타일 (0 - 푸시, 1 - 체크 박스)
	int mStyle;

	/// 체크 여부
	bool mChecked;
};

inline
void cSkinButton::SetChecked( bool checked )
{
	mChecked = checked;
}

inline
bool cSkinButton::IsChecked()
{
	return mChecked;
}
