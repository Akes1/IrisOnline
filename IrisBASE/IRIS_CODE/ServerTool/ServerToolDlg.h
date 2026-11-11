// ServerToolDlg.h : 헤더 파일
//
#pragma once
#include "resource.h"
#include "Network.h"


//#define	WM_SOCKEVENT			(WM_USER + 0x10)
#define MSGBOX_YESNO(msg)		( AfxMessageBox( msg, MB_YESNO	) == IDYES	)
#define MSGBOX_OK(msg)			( AfxMessageBox( msg, MB_OK		) == IDOK	)
#define SAFE_DELETE(P)			if (P!=NULL) { delete(P);      (P)=NULL; }

/// time
const int SECOND			= 1000;
const int MINUTE			= 60 * SECOND;
const int HOUR				= 60 * MINUTE;

const int SECOND_TIME		= 1 * SECOND;
const int SECOND_TIME_30	= 30 * SECOND;
const int MINUTE_TIME		= 1 * MINUTE;

/// port
const int AGENT_PORT	= 14900;

/// Login Server Columns
enum eLOGIN_COLUMNS
{
	LOGCOL_CHECK = 0,
	LOGCOL_STATE,
	LOGCOL_AGENT,
	LOGCOL_USERIN,
	LOGCOL_ADDR,
	LOGCOL_PING,
	LOGCOL_ETC,
};

/// Channel Server Columns
enum eCHANNEL_COLUMNS
{
	CHNCOL_CHECK = 0,
	CHNCOL_STATE,
	CHNCOL_AGENT,
	CHNCOL_SERVERNUM,
	CHNCOL_CHANNELNUM,
	CHNCOL_SERVICENAME,
	CHNCOL_USERCNT,
    CHNCOL_ADDR,
	CHNCOL_PING,
	CHNCOL_ETC,
};

struct sSrvData
{
    char        orderName[MAX_PATH];
    cServer*    pSrv;
};


static int          mChnOrderCol;           /// 정렬한 컬럼
static int CALLBACK CompareFunc( LPARAM lParam1, LPARAM lParam2, LPARAM lParam3 );
bool                WriteLog(const char*  msg);


// CServerToolDlg 대화 상자
class CServerToolDlg : public CDialog
{
// 생성입니다.
public:
	CServerToolDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SERVERTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnSockEvent( WPARAM, LPARAM );
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClickSrvOn();
	afx_msg void OnClickSrvOff();
	afx_msg void OnClickUserOn();
    afx_msg void OnClickPathFinderOn();
	afx_msg void OnClickGMOn();
    afx_msg void OnClickTesterOn();
	afx_msg void OnDestroy();
	afx_msg void OnClickLoadChnlist();
    afx_msg void OnAgent();
    afx_msg void OnClose();
    afx_msg void OnClickChnColumn(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
public:

	/// 초기화
	void Init();
	void InitListCtrl(void);

	/// agent 정보 Load
	void LoadServerInfo(void);

	/// 각각의 리스트에 데이타 추가
	void AddLoginSrv( const char* ip, int port, cServer* login );
	void AddChnSrv( const char* ip, int port, cServer* chn );

	/// update column values
	void UpdateState( cServer* srv );

	/// agent on/off msg 변경
	void UpdateAgentOnOff( cServer* agent, PTCHAR msg );

	/// column 변경
	void UpdateColumn( cServer* srv, int dlg, int column, PTCHAR msg );
	void UpdateColumn( cServer* srv, int column, PTCHAR msg );

	/// 체크된 서버( = 패킷 보낼 서버 )
	void CheckSrv( int dlg, int column, unsigned short srvType );

	/// 같은 소켓 찾기
	int FindItem( cServer* srv, int dlg );
//	int FindItem( cServer* srv, unsigned short srvType );

	int GetDlgType( unsigned short srvType )		{	return (srvType == SRV_TYPE_LOGIN) ? IDC_LOGIN_LIST : IDC_CHN_LIST;		}
	int GetStateColumn( unsigned short srvType )	{	return (srvType == SRV_TYPE_LOGIN) ? LOGCOL_STATE   : CHNCOL_STATE;		}
	int GetPingColumn( unsigned short srvType )		{	return (srvType == SRV_TYPE_LOGIN) ? LOGCOL_PING    : CHNCOL_PING;		}
	int GetEtcColumn( unsigned short srvType )		{	return (srvType == SRV_TYPE_LOGIN) ? LOGCOL_ETC     : CHNCOL_ETC;		}

	TCHAR* GetSecurityLevel( unsigned long level )
	{
		if( level == SRVTOOL_SECURITY_LEVEL0)		return _T("GM");
		else if( level == SRVTOOL_SECURITY_LEVEL1)	return _T("TESTER");
		else if( level == SRVTOOL_SECURITY_LEVEL2)	return _T("PF");
		else if( level == SRVTOOL_SECURITY_LEVEL3)	return _T("USER");

		return _T("Error");
	}

	bool RecvMsgParser( cServer* srv, const char* msg );
    void UpdateConState( );
    int  GetUserTotal( );
    void ServerOn( int dlg );
    void ServerOff( int dlg );

public:
	cFileSystem*		mpFileSystem;
	cNetwork*			mpNetwork;

    BOOL                mChnCheck;
    BOOL                mChnAsc;                /// 정렬 순서
    int                 mDisConCount;           /// 연결이 끊어진 갯수
    int                 mCloseCount;
};
