// UnPackToolDlg.h : 헤더 파일
//

#pragma once
#include "ExtractDlg.h"
#include "PackList.h"

//#include "afxcmn.h"

#define MSGBOX_YESNO(msg)		( AfxMessageBox( msg, MB_YESNO	) == IDYES	)
#define MSGBOX_OK(msg)			( AfxMessageBox( msg, MB_OK		) == IDOK	)


// CUnPackToolDlg 대화 상자
class CUnPackToolDlg : public CDialog
{
// 생성입니다.
public:
	CUnPackToolDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_UNPACKTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
    afx_msg void OnDestroy();
    afx_msg void OnFileOpen();
    afx_msg void OnExtract();
    afx_msg void OnSelectPack(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnViewExtFile(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
    afx_msg void OnClose();
    afx_msg void OnKeyDownFromPackList(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnOrderFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    void SetStatusBar(const TCHAR* path);
    bool FindPackList(const TCHAR* fileName);

private:

    CStatusBarCtrl  mStatusBar;
    cPackFile*      mpPackFile;

    CListCtrl       mPackList;
    CListCtrl       mFileList;

    CPackList       mPack;

    int             mSortColumn;
    BOOL            mIsAsc;

public:
    afx_msg void OnDropFiles(HDROP hDropInfo);
};

static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3 );