#pragma once


// cCharDramaDlg 대화 상자입니다.

class cCharDramaDlg : public CDialog
{
	DECLARE_DYNAMIC(cCharDramaDlg)

public:
	cCharDramaDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cCharDramaDlg();

	/// 
	void OpenData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

protected:
	afx_msg void OnBnClickedDramaReplay();
	afx_msg void OnCbnSelchangeCombo();
	DECLARE_MESSAGE_MAP()

protected:
	CComboBox mDramaCombo;
};
