#pragma once


// cNpcModelDlg 대화 상자입니다.

class cNpcModelDlg : public CDialog
{
	DECLARE_DYNAMIC(cNpcModelDlg)

public:
	cNpcModelDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cNpcModelDlg();

	unsigned long OpenData();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnCbnSelchangeCombo();
	DECLARE_MESSAGE_MAP()

protected:

	CComboBox	mNpcModelCombo;
};
