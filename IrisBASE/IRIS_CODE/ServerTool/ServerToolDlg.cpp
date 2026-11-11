// ServerToolDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "ServerTool.h"
#include "ServerToolDlg.h"
#include "AgentListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/// 로그 파일
bool WriteLog(const char*  msg)
{
    if( NULL == msg )
    {
        OutputDebugString( _T("[CServerToolDlg::WriteLog] msg == NULL \n") );
        return false;
    }

    FILE* file = NULL;
    const char* FILE_NAME = "./ServerTool_Log.txt";

    if( FileExist( FILE_NAME ) == false )
    {
        if( fopen_s( &file, FILE_NAME, "wt" ) != 0 )
        {
            OutputDebugString( _T("[CServerToolDlg::WriteLog] ServerTool_Log.txt  생성 실패!! \n") );
            return false;
        }
    }

    if( !file )
    {
       if( fopen_s( &file, FILE_NAME, "a+" ) != 0 )
        {
            OutputDebugString( _T("[CServerToolDlg::WriteLog] ServerTool_Log.txt  열기 실패!! \n") );
            return false;
        }
    }

    char buff[1024] = {0,};
    //_tcscpy_s( buff, msg );

    CTime time = CTime::GetCurrentTime();
    if( file )
    {
        sprintf_s( buff, "[%04d/%02d/%02d %02d:%02d] %s", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), msg );
        fputs( buff, file );
        fclose( file );
    }
    return true;
}


// CServerToolDlg 대화 상자
CServerToolDlg::CServerToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_IRIS_ICON);
}

void CServerToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CServerToolDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE( WM_SOCKEVENT, OnSockEvent )
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_USER_ON, &CServerToolDlg::OnClickUserOn)
	ON_BN_CLICKED(IDC_LOGIN_ON, &CServerToolDlg::OnClickSrvOn)
	ON_BN_CLICKED(IDC_LOGIN_OFF, &CServerToolDlg::OnClickSrvOff)
	ON_BN_CLICKED(IDC_PATH_FINDERON_BUTTON, &CServerToolDlg::OnClickPathFinderOn)
	ON_BN_CLICKED(IDC_GAME_MASTER_ON_BUTTON, &CServerToolDlg::OnClickGMOn)
    ON_BN_CLICKED(IDC_TESTER_ON_BUTTON, &CServerToolDlg::OnClickTesterOn)
ON_COMMAND(ID_AGENT, &CServerToolDlg::OnAgent)
ON_COMMAND(32772, &CServerToolDlg::OnClose)
ON_NOTIFY(LVN_COLUMNCLICK, IDC_CHN_LIST, &CServerToolDlg::OnClickChnColumn)
END_MESSAGE_MAP()


// CServerToolDlg 메시지 처리기

BOOL CServerToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	Init();

	SetTimer( SECOND_TIME,		SECOND_TIME,	NULL );
	SetTimer( SECOND_TIME_30,	SECOND_TIME_30, NULL );
	SetTimer( MINUTE_TIME,		MINUTE_TIME,	NULL );

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}


// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CServerToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CServerToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CServerToolDlg::OnClose()
{
    PostMessage( WM_CLOSE );
}

void CServerToolDlg::OnDestroy()
{
	CDialog::OnDestroy();

	SAFE_DELETE( mpFileSystem );
	SAFE_DELETE( mpNetwork );
}

void CServerToolDlg::OnTimer(UINT_PTR nIDEvent)
{
	switch( nIDEvent )
	{
	case SECOND_TIME:
		mpNetwork->SendPingChekReq();
		break;
	case SECOND_TIME_30:
		mpNetwork->CheckConnect();
		break;
	case MINUTE_TIME:
		mpNetwork->SendHeartBeat();
		break;
	default : break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CServerToolDlg::Init()
{
    mChnCheck       = FALSE;
    mChnOrderCol    = 0;
    mDisConCount    = 0;
    mCloseCount     = 0;

	mpFileSystem	= new cFileSystem( );
	mpNetwork		= new cNetwork( m_hWnd );

	InitListCtrl();

	/// 서버 정보 Load
	LoadServerInfo();
}

void CServerToolDlg::InitListCtrl(void)
{
	/// 로그인 서버 리스트
	CListCtrl* pLogInListC = (CListCtrl*)GetDlgItem( IDC_LOGIN_LIST );

	pLogInListC->InsertColumn( LOGCOL_CHECK,	_T(" "		), LVCFMT_CENTER, 20	);
	pLogInListC->InsertColumn( LOGCOL_STATE,	_T("State"	), LVCFMT_CENTER, 45	);
	pLogInListC->InsertColumn( LOGCOL_AGENT,	_T("Agent"	), LVCFMT_CENTER, 50	);
	pLogInListC->InsertColumn( LOGCOL_USERIN,	_T("UserIn"	), LVCFMT_CENTER, 50	);
	pLogInListC->InsertColumn( LOGCOL_ADDR,		_T("Address"), LVCFMT_CENTER, 135	);
	pLogInListC->InsertColumn( LOGCOL_PING,		_T("Ping"	), LVCFMT_CENTER, 45	);
    pLogInListC->InsertColumn( LOGCOL_ETC,		_T("Description"), LVCFMT_LEFT, 150	);

	pLogInListC->SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES ); 

	/// 채널 서버 리스트
	CListCtrl* pChannelListC = (CListCtrl*)GetDlgItem( IDC_CHN_LIST );
	pChannelListC->InsertColumn( CHNCOL_CHECK,		_T("□"		), LVCFMT_CENTER, 25	);
	pChannelListC->InsertColumn( CHNCOL_STATE,		_T("State"	), LVCFMT_CENTER, 45	);
	pChannelListC->InsertColumn( CHNCOL_AGENT,		_T("Agent"	), LVCFMT_CENTER, 50	);
	pChannelListC->InsertColumn( CHNCOL_SERVERNUM,	_T("SrvNo"	), LVCFMT_CENTER, 45	);
	pChannelListC->InsertColumn( CHNCOL_CHANNELNUM,	_T("ChnNo"	), LVCFMT_CENTER, 55	);
	pChannelListC->InsertColumn( CHNCOL_SERVICENAME,_T("Name"	), LVCFMT_CENTER, 110	);
	pChannelListC->InsertColumn( CHNCOL_USERCNT,	_T("User"   ), LVCFMT_CENTER, 50	);
    pChannelListC->InsertColumn( CHNCOL_ADDR,		_T("Address"), LVCFMT_CENTER, 135	);
	pChannelListC->InsertColumn( CHNCOL_PING,		_T("Ping"	), LVCFMT_CENTER, 50	);
	pChannelListC->InsertColumn( CHNCOL_ETC,		_T("Description"), LVCFMT_LEFT, 150	);

	pChannelListC->SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES ); 
}

LRESULT CServerToolDlg::OnSockEvent( WPARAM wParam, LPARAM lParam )
{
	SOCKET sock = (SOCKET)wParam;

	/// 소켓 검사
	if( INVALID_SOCKET == sock)
		return false;

	cServer* srv = mpNetwork->FindSrv( sock );
	if( NULL == srv )
		return false;

	/// 이벤트
	int lEvent = WSAGETSELECTEVENT(lParam);
	switch( lEvent )
	{
	case FD_CONNECT:
        {
            if( srv->IsConnect() )
            {
                if( WSAGETSELECTERROR( lParam ) == 0 )
                    mpNetwork->ConnectComplete( srv );
            }
        } 
        break;
    case FD_READ:
        {
          mpNetwork->SetServerDlg( this );
          mpNetwork->Recv( sock );
        } 
        break;
	case FD_WRITE:
        {
            if( srv->IsConnectResult() == false )
            {
                srv->IsConnectResult( true );

                //char str[256]={0,};
                //sprintf_s( str, "FD_CONNECT [Socket:%d][ip-%s:%d] [srvType:%d]\n", sock, srv->GetIp(), srv->GetPort(), srv->GetSrvType() );
                //WriteLog( str );

                unsigned short srvType = srv->GetSrvType();
                int dlgType = GetDlgType( srvType );
                UpdateColumn( srv, dlgType, GetEtcColumn( srvType ), _T("") );

                int column = GetStateColumn( srv->GetSrvType() );
                UpdateColumn( srv, column, _T("ON") );
                //UpdateConState( );

                if( srv->GetSrvType() == SRV_TYPE_AGENT )
                    mpNetwork->SendSrvListReq( srv );
                else if( srv->GetSrvType() > SRV_TYPE_LOGIN && srv->GetSrvType() < SRV_TYPE_END )
                    mpNetwork->SendChnInfoReq( srv );
            }
        } 
        break;
	case FD_CLOSE:
		{
            if( srv->IsConnectResult() && srv->IsSendOff() == false )
            {
                ++mCloseCount;

                int lError = WSAGETSELECTERROR( lParam );
                if( lError == 10053 )
                    srv->IsSendOff( true );
                else
                {
                    char str[256]={0,};
                    sprintf_s( str, "FD_CLOSE [Socket:%d][lError:%d][ip-%s:%d] [srvType:%d] [DisConnect:%d]\n", 
                                                sock, lError, srv->GetIp(), srv->GetPort(), srv->GetSrvType(), mCloseCount );
                    WriteLog( str );
                }
            }

            mpNetwork->CloseSocket( srv );

            int column = GetStateColumn( srv->GetSrvType() );
            UpdateColumn( srv, column, _T("OFF") );
            //UpdateConState( );

            if( srv->GetSrvType() != SRV_TYPE_AGENT )
                mpNetwork->SendSrvStateReq( srv );
		}
		break;
	default :
        {
            int lError      = WSAGETSELECTERROR( lParam );
            char str[256]   = {0,};
            sprintf_s( str, "switch( lEvent ) [Socket:%d][lEvent:%d][error:%d][ip-%s:%d] [srvType:%d]\n", 
                                                sock, lEvent, lError, srv->GetIp(), srv->GetPort(), srv->GetSrvType() );
            WriteLog( str );

            mpNetwork->CloseSocket( sock );

            int column = GetStateColumn( srv->GetSrvType() );
            UpdateColumn( srv, column, _T("Error") );
            //UpdateConState( );
        } break;
	}

	return 0;
}

void CServerToolDlg::LoadServerInfo(void)
{
	cFileLoader loader;
	cString pathName = "ServerTool.txt";

	if( loader.Open( pathName, true ) == false )
	{                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
		MessageBox( _T("ServerTool.txt Error"), _T("ERROR") );
		AfxGetMainWnd()->PostMessage(WM_CLOSE);
		return;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );

	cString				str;
	char				ip[50];
	int					port = 0;
	unsigned short		flag = -1;

	while( tokenizer.IsEnd() == false )
	{
		/// ip
		if( tokenizer.GetNext( &str ) == false )	{	return;	}
		sprintf_s( ip, "%s", str.Cstr() ); 

		/// agent
        mpNetwork->AddServer( SRV_TYPE_AGENT, ip, 14900 );
	}
}

void CServerToolDlg::AddLoginSrv( const char* ip, int port, cServer* login )
{
	CListCtrl* pLogInListC = (CListCtrl*)GetDlgItem( IDC_LOGIN_LIST );
	if( pLogInListC->GetItemCount() >= MAX_LOGIN_SERVER )
	{
		MessageBox( _T("Max LoginServer Overflow"), _T("ERROR") );
		AfxGetMainWnd()->PostMessage(WM_CLOSE);
		return;
	}

    int count = pLogInListC->GetItemCount();

	/// insert column values
	pLogInListC->InsertItem( count , _T(" "), LOGCOL_CHECK );
	pLogInListC->SetItemText( count, LOGCOL_STATE,	_T("OFF")	);
	pLogInListC->SetItemText( count, LOGCOL_AGENT,	_T("OFF")	);
	pLogInListC->SetItemText( count, LOGCOL_USERIN, _T("OFF")	);
	pLogInListC->SetItemText( count, LOGCOL_PING,	_T("0")		);

	TCHAR wIp[100];
	MultiByteToWideChar( CP_ACP, 0, ip, (int)strlen(ip)+1, (PTCHAR)&wIp, 16 );

	CString addr;
	addr.Format( _T("%s : %d"), wIp, port );
	pLogInListC->SetItemText( count, LOGCOL_ADDR, addr );

	/// set server data
	pLogInListC->SetItemData( count, (DWORD_PTR)login );
}

void CServerToolDlg::AddChnSrv( const char* ip, int port, cServer* chn )
{
	CListCtrl* pChnListC = (CListCtrl*)GetDlgItem( IDC_CHN_LIST );
	if( pChnListC->GetItemCount() >= MAX_CHANNEL_SERVER )
	{
		MessageBox( _T("Max ChannelServer Overflow"), _T("ERROR") );
		AfxGetMainWnd()->PostMessage(WM_CLOSE);
		return;
	}

    int count = pChnListC->GetItemCount();

	/// insert column values
	pChnListC->InsertItem( count , _T(" "), CHNCOL_CHECK );
	pChnListC->SetItemText( count, CHNCOL_STATE,		_T("OFF")	);
	pChnListC->SetItemText( count, CHNCOL_AGENT,		_T("OFF")	);
	pChnListC->SetItemText( count, CHNCOL_SERVERNUM,	_T("0")		);
	pChnListC->SetItemText( count, CHNCOL_CHANNELNUM,   _T("0")		);
	pChnListC->SetItemText( count, CHNCOL_USERCNT,	    _T("0")		);
	pChnListC->SetItemText( count, CHNCOL_PING,		    _T("0")		);

	TCHAR wStr[100];
	CString cStr;

    /// ip
	MultiByteToWideChar( CP_ACP, 0, ip, (int)strlen(ip)+1, (PTCHAR)&wStr, 16 );
	cStr.Format( _T("%s : %d"), wStr, port );
	pChnListC->SetItemText( count, CHNCOL_ADDR, cStr );

    /// service Name
	MultiByteToWideChar( CP_ACP, 0, chn->GetName(), (int)strlen(chn->GetName())+1, (PTCHAR)&wStr, 16 );
    cStr.Format( _T("%s"), wStr ); 
    pChnListC->SetItemText( count, CHNCOL_SERVICENAME, cStr );

	/// set server data
	pChnListC->SetItemData( count, (DWORD_PTR)chn );
}

int CServerToolDlg::FindItem( cServer* srv, int dlg )
{
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem( dlg );

	for( int i = 0 ; i < pListCtrl->GetItemCount() ; ++i )
	{
		cServer* pSrv = (cServer*)pListCtrl->GetItemData( i );
		if( pSrv == srv )
			return i;
	}

	return -1;
}

/// service current state
#define SERVICE_STOPPED                        0x00000001
#define SERVICE_START_PENDING                  0x00000002
#define SERVICE_STOP_PENDING                   0x00000003
#define SERVICE_RUNNING                        0x00000004
#define SERVICE_CONTINUE_PENDING               0x00000005
#define SERVICE_PAUSE_PENDING                  0x00000006
#define SERVICE_PAUSED                         0x00000007

bool CServerToolDlg::RecvMsgParser( cServer* srv, const char* msg )
{
	MSGROOT* pRecvRoot = (MSGROOT*)msg;
    if( srv == NULL )
        return false;

	if( pRecvRoot->Category != NM_SERVERTOOL )
	{
		char str[256];
        sprintf_s( str, "CServerToolDlg::RecvMsgParser - [Category:%d] [%s:%d]\n", pRecvRoot->Category, srv->GetIp(), srv->GetPort() );
		WriteLog( str );
		return false;
	}

	switch( pRecvRoot->Protocol )
	{
	case NM_SERVERTOOL_PING_RES:
		{
			MSG_RES_SRVTOOL_PING* recvMsg = (MSG_RES_SRVTOOL_PING*)msg;
			unsigned long currTime = GetTickCount();

			TCHAR str[100];
			wsprintf( str, _T("%ld"), GetTickCount() - recvMsg->mToolSendTime );

			int dlgType = GetDlgType( srv->GetSrvType() );

			/// ping
			UpdateColumn( srv, dlgType, GetPingColumn( srv->GetSrvType() ), str );
			
			/// login security level
			if( srv->GetSrvType() == SRV_TYPE_LOGIN)
			{
				if( recvMsg->mSecurityLevel > SRVTOOL_SECURITY_LEVEL3 )
				{
					//TCHAR wIp[100];
					//MultiByteToWideChar( CP_ACP, 0, srv->GetIp(), (int)strlen(srv->GetIp())+1, (PTCHAR)&wIp, 16 );
                    char buff[256] = {0,};
                    sprintf_s( buff, "NM_SERVERTOOL_PING_RES - [securityLevel:%d], [ip-%s:%d] [srvType:%d]\n", 
                                                                recvMsg->mSecurityLevel, srv->GetIp(), srv->GetPort(), srv->GetSrvType() );
					WriteLog(buff);
				}

				UpdateColumn( srv, dlgType, LOGCOL_USERIN, GetSecurityLevel(recvMsg->mSecurityLevel) );
			}
			/// user count
			else if( srv->GetSrvType() >= SRV_TYPE_CHANNEL && srv->GetSrvType() < SRV_TYPE_END )
			{
				wsprintf( str, _T("%ld"),  recvMsg->mUserCnt );
				UpdateColumn( srv, dlgType, CHNCOL_USERCNT, str);
			}

            UpdateConState();
		} 
        break;
	case NM_SERVERTOOL_SERVER_LIST_RES:
		{
            MSG_RES_SRVTOOL_SRVLIST* recvMsg = (MSG_RES_SRVTOOL_SRVLIST*)msg;
			for( unsigned char i = 0; i < recvMsg->rowCount; ++i )
			{
                cServer* pSrv = mpNetwork->FindSrv( srv, recvMsg->Servers[i].serviceName );
                if( pSrv )
                    continue;

                short srvType = mpNetwork->GetSrvType( recvMsg->Servers[i].serviceName );

                if( srvType == SRV_TYPE_LOGIN )
                {
                    cServer* pSrv = mpNetwork->AddServer( SRV_TYPE_LOGIN, recvMsg->Servers[i].ip, 14400, srv );
                    pSrv->SetName( recvMsg->Servers[i].serviceName );

                    AddLoginSrv( recvMsg->Servers[i].ip, 14400, pSrv );
			        UpdateColumn( srv, LOGCOL_AGENT, _T("ON") );
                }
                else if( srvType > SRV_TYPE_LOGIN && srvType < SRV_TYPE_END )
                {
                    cServer* pSrv = mpNetwork->AddServer( srvType, recvMsg->Servers[i].ip, 14600, srv );
                    pSrv->SetName( recvMsg->Servers[i].serviceName );

                    AddChnSrv( recvMsg->Servers[i].ip, 14600, pSrv );
                    UpdateColumn( srv, CHNCOL_AGENT, _T("ON") );
                }
               /* else
                { 
                    TCHAR st[100];
                    wsprintf( st, _T("NM_SERVERTOOL_SERVER_LIST_RES:: [serviceName:%s][srvType:%d]\n"), recvMsg->Servers[i].serviceName, srvType );
                    WriteLog(st);
                }*/
			}
		} 
        break;
	case NM_SERVERTOOL_CHANNEL_INFO_RES: 
		{
 			MSG_RES_SRVTOOL_CHANNEL_INFO* recvMsg = (MSG_RES_SRVTOOL_CHANNEL_INFO*)msg;
			int dlgType = GetDlgType( srv->GetSrvType() );

            srv->mSrvNo = recvMsg->mServerNum;
            srv->mChnNo = recvMsg->mChannelNum;

			TCHAR str[30];
			wsprintf( str, _T("%d"), recvMsg->mServerNum );
			UpdateColumn( srv, dlgType, CHNCOL_SERVERNUM ,	str );
			
			wsprintf( str, _T("%d"), recvMsg->mChannelNum );
			UpdateColumn( srv, dlgType, CHNCOL_CHANNELNUM ,	str );
		} 
        break;
    case NM_SERVERTOOL_SERVER_STATUS_RES:
        {
 			MSG_RES_SRVTOOL_STATUS* recvMsg = (MSG_RES_SRVTOOL_STATUS*)msg;

            cServer* pSrv = mpNetwork->FindSrv( srv, recvMsg->serviceName );
            if( pSrv )
            {
                unsigned short  srvType = srv->GetSrvType();
                int             dlgType = GetDlgType( srvType );

                bool isOFF = false;
                TCHAR msg[30];
                switch( recvMsg->status )
                {
                    case SERVICE_START_PENDING:
                        wcscpy_s( msg, _T("서비스 시작중") );
                        break;
                    case SERVICE_STOPPED:
                    case SERVICE_STOP_PENDING:
                        wcscpy_s( msg, _T("서비스 정지") );
                        isOFF = true;
                        break;
                    case SERVICE_PAUSE_PENDING:
                    case SERVICE_PAUSED:
                        wcscpy_s( msg, _T("서비스 일시정지") );
                        isOFF = true;
                        break;
                    case SERVICE_CONTINUE_PENDING:
                        wcscpy_s( msg, _T("서비스 재시작") );
                        break;
                    case SERVICE_RUNNING:
                        wcscpy_s( msg, _T("서비스 실행중") );
                    default:break;
                }
                
                UpdateColumn( pSrv, dlgType, GetEtcColumn( srvType ), msg );
                if( isOFF && pSrv->IsSendOff() == false )
                {
                    char str[256]={0,};
                    sprintf_s( str, "NM_SERVERTOOL_SERVER_STATUS_RES - [current:%d] [Socket:%d][ip-%s:%d] [srvType:%d] [DisConnect:%d]\n", 
                                    recvMsg->status, pSrv->GetSocket(), pSrv->GetIp(), pSrv->GetPort(), pSrv->GetSrvType(), mCloseCount );
                    WriteLog( str );
                }
                pSrv->IsSendOff( false );
            }
        }
        break;
	default :
		{
			char st[256];
            sprintf_s( st, "NM_SERVERTOOL_CHANNEL_INFO_RES:: [Category:%d, Protocol:%d][ip-%s:%d]\n", 
                                                            pRecvRoot->Category, pRecvRoot->Protocol, srv->GetIp(), srv->GetPort() );
			WriteLog(st);
		} break;
	}

	return true;
}

void CServerToolDlg::UpdateColumn( cServer* srv, int column, PTCHAR msg )
{
	if( srv->GetSrvType() == SRV_TYPE_AGENT )
		UpdateAgentOnOff( srv, msg );
	else
	{
		int dlg = GetDlgType( srv->GetSrvType() );

        CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem( dlg );
        if( pListCtrl )
        {
            for( int i = 0 ; i < pListCtrl->GetItemCount() ; ++i )
            {
                cServer* pSrv = (cServer*)pListCtrl->GetItemData( i );
                if( pSrv == srv )
                    pListCtrl->SetItemText( i, column, msg );
            } // for
        }
	}
}

void CServerToolDlg::UpdateColumn( cServer* srv, int dlg, int column, PTCHAR msg )
{
	CListCtrl* pSrvList = (CListCtrl*)GetDlgItem( dlg );

	for( int i = 0 ; i < pSrvList->GetItemCount() ; ++i )
	{
		cServer* pSrv = (cServer*)pSrvList->GetItemData( i );
		if( pSrv == srv )
			pSrvList->SetItemText( i, column, msg );
	}
}

void CServerToolDlg::UpdateAgentOnOff( cServer* agent, PTCHAR msg )
{
	/// update login server on/off msg
	CListCtrl* pLoginList = (CListCtrl*)GetDlgItem( IDC_LOGIN_LIST );

	for( int i = 0 ; i < pLoginList->GetItemCount() ; ++i )
	{
		cServer* pLogin = (cServer*)pLoginList->GetItemData( i );
		if( pLogin && pLogin->GetAgent() == agent )
			pLoginList->SetItemText( i, LOGCOL_AGENT, msg );
	}

	/// update channel server on/off msg
	CListCtrl* pChnList = (CListCtrl*)GetDlgItem( IDC_CHN_LIST );

	for( int i = 0 ; i < pChnList->GetItemCount() ; ++i )
	{
		cServer* pChn = (cServer*)pChnList->GetItemData( i );
		if( pChn && pChn->GetAgent() == agent )
			pChnList->SetItemText( i, CHNCOL_AGENT, msg );
	}
}

void CServerToolDlg::ServerOn( int dlg )
{
    CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem( dlg );
    if( NULL == pListCtrl )
        return;

    for( int i = 0 ; i < pListCtrl->GetItemCount() ; ++i )
    {
        if( pListCtrl->GetCheck( i ) )
        {
            cServer* pSrv = (cServer*)pListCtrl->GetItemData( i );
            if( pSrv )                   
            {
                MSG_REQ_SRVTOOL_AGENT_ONOFF sendMsg;
                sendMsg.Category = NM_SERVERTOOL;
                sendMsg.Protocol = NM_SERVERTOOL_AGENT_ON_REQ;
                strcpy_s( sendMsg.serviceName, pSrv->GetName() );

                mpNetwork->SendToSrv( (char*)&sendMsg, sizeof(sendMsg), pSrv->GetAgent() );
            }

            pListCtrl->SetCheck( i, FALSE );
        }// GetCheck
    }
}

void CServerToolDlg::ServerOff( int dlg )
{
    CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem( dlg );
    if( NULL == pListCtrl )
        return;

    for( int i = 0 ; i < pListCtrl->GetItemCount() ; ++i )
    {
        if( pListCtrl->GetCheck( i ) )
        {
            cServer* pSrv = (cServer*)pListCtrl->GetItemData( i );
            if( pSrv )
            {
                cServer* pAgent = pSrv->GetAgent();

                MSG_REQ_SRVTOOL_AGENT_ONOFF sendMsg;
                sendMsg.Category = NM_SERVERTOOL;
                sendMsg.Protocol = NM_SERVERTOOL_AGENT_OFF_REQ;
                strcpy_s( sendMsg.serviceName, pSrv->GetName() );

                mpNetwork->SendToSrv( (char*)&sendMsg, sizeof(sendMsg), pAgent );
                pSrv->IsSendOff( true );
            }

            pListCtrl->SetCheck( i, FALSE );
        }// GetCheck
    }
}

void CServerToolDlg::OnClickSrvOn()
{
	if( !MSGBOX_YESNO(_T("Server ON ?")) )
		return;

    ServerOn( IDC_LOGIN_LIST    );
    Sleep(1500);
    ServerOn( IDC_CHN_LIST      );
}

void CServerToolDlg::OnClickSrvOff()
{
	if( !MSGBOX_YESNO(_T("Server OFF ?")) )
		return;

    ServerOff( IDC_CHN_LIST     );
    Sleep(1500);
    ServerOff( IDC_LOGIN_LIST   );
}

void CServerToolDlg::CheckSrv( int dlg, int column, unsigned short srvType )
{
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem( dlg );
	if( NULL == pListCtrl )
		return;

	for( int i = 0 ; i < pListCtrl->GetItemCount() ; ++i )
	{
		if( pListCtrl->GetCheck( i ) )
			mpNetwork->UpdateSendOn( srvType, i );
	}
}

void CServerToolDlg::OnClickPathFinderOn()
{
	if( !MSGBOX_YESNO( _T("PathFinder Login Ok?") ) )
		return;

	CheckSrv( IDC_LOGIN_LIST, LOGCOL_USERIN, SRV_TYPE_LOGIN );
	mpNetwork->SendPathFinderOn();
}

void CServerToolDlg::OnClickUserOn()
{
	if( !MSGBOX_YESNO(_T("User Login Ok?")) )
		return;

	CheckSrv( IDC_LOGIN_LIST, LOGCOL_USERIN, SRV_TYPE_LOGIN );
	mpNetwork->SendUserOn( );
}

void CServerToolDlg::OnClickGMOn()
{
	if( !MSGBOX_YESNO( _T("Game Master Login Ok?") ) )
		return;

	CheckSrv( IDC_LOGIN_LIST, LOGCOL_USERIN, SRV_TYPE_LOGIN );
	mpNetwork->SendGMOn();
}

void CServerToolDlg::OnClickTesterOn()
{
	if( !MSGBOX_YESNO( _T("Tester Login Ok?") ) )
		return;

	CheckSrv( IDC_LOGIN_LIST, LOGCOL_USERIN, SRV_TYPE_LOGIN );
	mpNetwork->SendTesterOn();
}

void CServerToolDlg::OnAgent()
{
    cAgentListDlg* pDlg = new cAgentListDlg;
    pDlg->Init( m_hWnd, mpNetwork->GetSrvList( SRV_TYPE_AGENT ) );
    if( pDlg->DoModal() )
    {
    }

    if( pDlg )
        SAFE_DELETE( pDlg );
}

void CServerToolDlg::UpdateConState( )
{
    CEdit* pMsg = (CEdit*)GetDlgItem( IDC_CONNECT_STATE );

    TCHAR msg[256] = {0,};
    int len = wsprintf( msg, _T("\r\n  [ CONNECTION COUNT ]\r\n\r\n") 
                             _T("＊AGENT\t   : %2d / %d \r\n") 
                             _T("＊LOGIN\t   : %2d / %d \r\n\r\n")
                             _T("＊CHANNEL: %2d / %d \r\n")
                             _T("＊PVP\t   : %2d / %d \r\n")
                             _T("＊THEMA\t   : %2d / %d \r\n")
                             _T("＊TUTORIAL: %2d / %d \r\n")
                             _T("\r\n\r\n")
                             _T("＊USER TOTAL: %d \r\n")
                             , mpNetwork->GetConnectCnt( SRV_TYPE_AGENT     ), mpNetwork->GetSrvCount( SRV_TYPE_AGENT   )
                             , mpNetwork->GetConnectCnt( SRV_TYPE_LOGIN     ), mpNetwork->GetSrvCount( SRV_TYPE_LOGIN   )
                             , mpNetwork->GetConnectCnt( SRV_TYPE_CHANNEL   ), mpNetwork->GetSrvCount( SRV_TYPE_CHANNEL )
                             , mpNetwork->GetConnectCnt( SRV_TYPE_PVP       ), mpNetwork->GetSrvCount( SRV_TYPE_PVP     )
                             , mpNetwork->GetConnectCnt( SRV_TYPE_THEMA     ), mpNetwork->GetSrvCount( SRV_TYPE_THEMA   )
                             , mpNetwork->GetConnectCnt( SRV_TYPE_TUTORIAL  ), mpNetwork->GetSrvCount( SRV_TYPE_TUTORIAL)
                             , GetUserTotal() );

    pMsg->SetWindowText( msg );
}

int CServerToolDlg::GetUserTotal()
{
    CListCtrl* pList = (CListCtrl*)GetDlgItem( IDC_CHN_LIST );
    if( !pList ) 
        return 0;

    int userTotal = 0;
    for( int i = 0; i < pList->GetItemCount(); i++ )
    {
        CString sUserCnt = pList->GetItemText( i, CHNCOL_USERCNT );
        userTotal += _ttoi( sUserCnt );
    }

    return userTotal;
}

void CServerToolDlg::OnClickChnColumn(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    int column = pNMLV->iSubItem;

    CListCtrl* pList = (CListCtrl*)GetDlgItem( IDC_CHN_LIST );
    if( !pList ) 
       return;

    /// checkbox column click
    if( column == CHNCOL_CHECK )
    {
        mChnCheck = !mChnCheck;
        for( int i = 0; i < pList->GetItemCount(); i++ )
            pList->SetCheck( i, mChnCheck );
    }
    /// data column click
    else
    {
        if( mChnOrderCol == column )
            mChnAsc = !mChnAsc;
        else
            mChnAsc = TRUE;

        mChnOrderCol = column;
        pList->SortItems( CompareFunc, (LPARAM)mChnAsc );
    }
}

static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3 )
{
    cServer*  pSrv1 = (cServer*)lParam1;
    cServer*  pSrv2 = (cServer*)lParam2;
    BOOL      IsAsc = (BOOL)lParam3;
    int       ret   = 0;

    if( !pSrv1 || !pSrv2 )
        return ret;

    if( mChnOrderCol == CHNCOL_STATE )
        ret = strcmp( pSrv1->IsConnectResult()?"ON":"OFF", pSrv2->IsConnectResult()?"ON":"OFF" );
    else if( mChnOrderCol == CHNCOL_AGENT )
    {
        cServer* pAgent1 = pSrv1->GetAgent();
        cServer* pAgent2 = pSrv2->GetAgent();
        if( !pAgent1 && !pAgent2 )
            return ret;

        ret = strcmp( pAgent1->IsConnectResult()?"ON":"OFF", pAgent2->IsConnectResult()?"ON":"OFF" );
    }
    else if( mChnOrderCol == CHNCOL_SERVERNUM )
    {
        char srvNo1[20] = {0,};
        char srvNo2[20] = {0,};
    
        sprintf_s( srvNo1, "%3d", pSrv1->mSrvNo );
        sprintf_s( srvNo2, "%3d", pSrv2->mSrvNo );

        ret = strcmp( srvNo1, srvNo2 );
    }
    else if( mChnOrderCol == CHNCOL_CHANNELNUM )
    {
        char chnNo1[20] = {0,};
        char chnNo2[20] = {0,};
    
        sprintf_s( chnNo1, "%3d", pSrv1->mChnNo );
        sprintf_s( chnNo2, "%3d", pSrv2->mChnNo );

        ret = strcmp( chnNo1, chnNo2 );
    }
    else if( mChnOrderCol == CHNCOL_SERVICENAME )
        ret = strcmp( pSrv1->GetName(), pSrv2->GetName() );
//    else if( mChnOrderCol == CHNCOL_USERCNT )
//        ret = strcmp( pSrv1->GetName(), pSrv2->GetName() );
    else if( mChnOrderCol == CHNCOL_ADDR )
        ret = strcmp( pSrv1->GetIp(), pSrv2->GetIp() );
    else
        ret = 0;

    if( IsAsc )
        return ret;
    else
        return -ret;
}
