#pragma once

// cMonModelDlg 대화 상자입니다.

class cMonModelDlg : public CDialog
{
	DECLARE_DYNAMIC(cMonModelDlg)

public:
	cMonModelDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cMonModelDlg();

	unsigned long OpenData();

	unsigned long GetMonsterIndex() { return mCurrentMonsterIdx; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnCbnSelchangeCombo();
	DECLARE_MESSAGE_MAP()

protected:

	CComboBox	mMonsterModelCombo;

	unsigned long mCurrentMonsterIdx;
};
