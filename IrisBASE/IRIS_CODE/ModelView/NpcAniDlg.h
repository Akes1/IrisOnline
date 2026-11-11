#pragma once

// cNpcAniDlg 대화 상자입니다.

class cNpcAniDlg : public CDialog
{
	DECLARE_DYNAMIC(cNpcAniDlg)

public:
	cNpcAniDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cNpcAniDlg();

	void OpenData( unsigned long baseModelIdx );

	unsigned int UpdateList( unsigned long modelIdx );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnCbnSelchangeCombo();
	DECLARE_MESSAGE_MAP()

protected:
	CComboBox mAnimationCombo;
};
