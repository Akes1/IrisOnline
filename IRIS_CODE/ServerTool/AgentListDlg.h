#pragma once
#include "afxcmn.h"
#include "Network.h"

// cAgentListDlg 대화 상자입니다.

class cAgentListDlg : public CDialog
{
	DECLARE_DYNAMIC(cAgentListDlg)

public:
	cAgentListDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~cAgentListDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_AGENT_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    afx_msg void OnConnect();
    afx_msg void OnRefresh();
	DECLARE_MESSAGE_MAP()


public:
    void        Load();
    void        Init( HWND hwnd, cSrvList* p );
    void        SetAgentSrv( cSrvList* p )      {   mAgentSrv = p;    }

private:
    CListCtrl       mAgentList;
    cSrvList*		mAgentSrv;

    HWND            mHwnd;

};
