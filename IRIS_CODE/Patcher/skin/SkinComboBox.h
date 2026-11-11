#pragma once

class cSkinComponentResource;
class cSkinComboBoxResource;

//////////////////////////////////////////////////////////////////////////
class cSkinComboBox : public CComboBox
{
	DECLARE_DYNCREATE(cSkinComboBox)
public:
	cSkinComboBox();
	virtual ~cSkinComboBox();

	/// 积己
	BOOL Create( cSkinComponentResource* resource, CWnd* parent, UINT id );

	virtual void DrawItem(LPDRAWITEMSTRUCT ds);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT ms){}

	void InitShow();

protected:
	afx_msg void OnPaint();

	afx_msg void OnCbnCloseup(); 
	afx_msg void OnCbnDropdown(); 
	afx_msg void OnCbnSelChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor); 
	afx_msg void InvalidateCombo();
	DECLARE_MESSAGE_MAP()

protected:
	/// 府家胶
	cSkinComboBoxResource* mResource;

	HWND	mhWndList; 
	HBITMAP	mhBmpScroll; 

	/// 康开
	CRect mRect;

	CRect mRectLeft;
	CRect mRectCenter;
	CRect mRectRight;
};
//////////////////////////////////////////////////////////////////////////
