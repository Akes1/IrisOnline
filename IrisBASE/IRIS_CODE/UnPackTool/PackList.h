#pragma once


// CPackList 대화 상자입니다.

class CPackList : public CDialog
{
	DECLARE_DYNAMIC(CPackList)

public:
	CPackList(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CPackList();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_UNPACKTOOL_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDropFiles(HDROP hDropInfo);
    virtual BOOL OnInitDialog();
};
