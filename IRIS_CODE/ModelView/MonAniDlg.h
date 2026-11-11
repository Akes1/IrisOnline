#pragma once

// cMonAniDlg 대화 상자입니다.

class cMonAniDlg : public CDialog
{
	DECLARE_DYNAMIC(cMonAniDlg)

public:
	cMonAniDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cMonAniDlg();

	void OpenData( unsigned long baseModelIdx );

	unsigned int UpdateList( unsigned long modelIdx );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnCbnSelchangeCombo();
	DECLARE_MESSAGE_MAP()

protected:
	CComboBox mAnimationCombo;
};
