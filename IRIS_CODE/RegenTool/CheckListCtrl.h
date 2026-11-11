#pragma once
/////////////////////////////////////////////////////////////////////////////
// CCheckListCtrl window

class CCheckListCtrl : public CListCtrl
{
	// Construction
public:
	CCheckListCtrl();

	// Attributes
public:

	// Operations
private:
	BOOL	m_blInited;
	CImageList	m_checkImgList;


public:
	BOOL Init();

protected:
	//	CCheckHeadCtrl	m_checkHeadCtrl;

public:
	virtual ~CCheckListCtrl();

protected:
//	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);		

	DECLARE_MESSAGE_MAP()
};