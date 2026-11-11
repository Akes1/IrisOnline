#pragma once

#include "PackFileLoader.h"
#include "afxwin.h"
#include "afxcmn.h"

#define MSGBOX_YESNO(msg)		( AfxMessageBox( msg, MB_YESNO	) == IDYES	)
#define MSGBOX_OK(msg)			( AfxMessageBox( msg, MB_OK		) == IDOK	)

class cExtractDlg : public CDialog
{
	DECLARE_DYNAMIC(cExtractDlg)

public:
	cExtractDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cExtractDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_EXTRACT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    afx_msg void OnExtract();
    afx_msg void OnSelFolder();
	DECLARE_MESSAGE_MAP()

public:
    void Init( cPackFile* pack, TCHAR* folder );
    
    void SetProgress( int min, int max );
    void SetPackFilePtr( cPackFile* p )         { mpPackFile = p;           }
    void SetExtPath( TCHAR* path )              { strcpy_s( mDir, path);    }

    static int CALLBACK BrowseInfoCall( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

private:

    CEdit           mPath;
    cPackFile*      mpPackFile;
    TCHAR           mDir[_MAX_PATH];
    TCHAR           mFullPath[_MAX_FNAME];

    CProgressCtrl   mProgress;

    CEdit mPwd;
};
