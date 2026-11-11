#pragma once


// cCharAniDlg 대화 상자입니다.

class cCharAniDlg : public CDialog
{
	DECLARE_DYNAMIC(cCharAniDlg)

public:
	cCharAniDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cCharAniDlg();

	/// 
	void OpenData();

	void UpdateList( unsigned char race, unsigned char gender );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

protected:
	afx_msg void OnCbnSelchangeCombo();
	DECLARE_MESSAGE_MAP()

protected:
	CComboBox mAnimationCombo;
};
