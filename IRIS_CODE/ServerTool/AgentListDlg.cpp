// AgentListDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "ServerTool.h"
#include "AgentListDlg.h"


// cAgentListDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(cAgentListDlg, CDialog)

cAgentListDlg::cAgentListDlg(CWnd* pParent /*=NULL*/)
	: CDialog(cAgentListDlg::IDD, pParent)
{

}

cAgentListDlg::~cAgentListDlg()
{
}

void cAgentListDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_AGENT_LIST, mAgentList);
}


BEGIN_MESSAGE_MAP(cAgentListDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON2, &cAgentListDlg::OnClose)
    ON_BN_CLICKED(IDC_BUTTON1, &cAgentListDlg::OnConnect)
    ON_BN_CLICKED(IDC_BUTTON3, &cAgentListDlg::OnRefresh)
END_MESSAGE_MAP()


// cAgentListDlg 메시지 처리기입니다.

BOOL cAgentListDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

	/// 로그인 서버 리스트
	mAgentList.InsertColumn( 0,	_T(" IP "), LVCFMT_CENTER, 122	);
	mAgentList.InsertColumn( 1,	_T("STATE"), LVCFMT_CENTER, 65	);

	mAgentList.SetExtendedStyle( LVS_EX_FULLROWSELECT ); 
    
    Load();

    return TRUE;  // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void cAgentListDlg::Init( HWND hwnd, cSrvList* pSrv )
{
    mHwnd       = hwnd;
    mAgentSrv   = pSrv;
}

void cAgentListDlg::Load()
{
    mAgentList.DeleteAllItems();

	/// insert column values
    for( unsigned int i = 0; i < mAgentSrv->GetSize(); i++ )
    {
        cServer* p = (cServer*)(*mAgentSrv)[i];
        if( p )
        {
            TCHAR wIp[100];
            MultiByteToWideChar( CP_ACP, 0, p->GetIp(), (int)strlen(p->GetIp())+1, (PTCHAR)&wIp, 16 );

            mAgentList.InsertItem( i, wIp );
            mAgentList.SetItemData( i, (DWORD_PTR)p );

            if( p->IsConnectResult() )
                mAgentList.SetItemText( i, 1, _T("ON") );
            else
                mAgentList.SetItemText( i, 1, _T("OFF") );
        }
    }
}

void cAgentListDlg::OnClose()
{
    PostMessage( WM_CLOSE );
}

void cAgentListDlg::OnConnect()
{
    POSITION pos = mAgentList.GetFirstSelectedItemPosition();
    while( pos )
    {
        int item = mAgentList.GetNextSelectedItem( pos );

        cServer* p = (cServer*)mAgentList.GetItemData( item );
        if( p && p->IsConnectResult() == false )
        {
            bool connect = p->Connect( mHwnd );
            p->IsConnect( connect );
            PostMessage( WM_SOCKEVENT, (WPARAM)FD_CONNECT, (LPARAM)0L );
        }
    }
}

void cAgentListDlg::OnRefresh()
{
    Load();
}
