#pragma once

#include "AsyncSelect.h"
#include "../Common/Protocol.h"
#include "../Common/CommonNetworkMsgStruct.h"
#include "../Common/Cheat_Common.h"

#include "../Util/STL/Queue.h"
#include "../Util/STL/PointerList.h"
#include "../Util/STL/PointerArray.h"

extern bool WriteLog(const char*  msg);


const int MAX_LOGIN_SERVER		= 255;
const int MAX_CHANNEL_SERVER	= 255;
const int MAX_AGENT_SERVER		= 255;

const int REQUEST_CONNECT_MAX	= 5;

#define MAX_BUFFER 0x1000

enum eSERVER_TYPE
{
	SRV_TYPE_AGENT = 0,
	SRV_TYPE_LOGIN,
	SRV_TYPE_CHANNEL,
	SRV_TYPE_PVP,
	SRV_TYPE_THEMA,
	SRV_TYPE_TUTORIAL,

    SRV_TYPE_END,
};

const unsigned short SRV_TYPE_BEGIN = SRV_TYPE_AGENT;

class CServerToolDlg;

class cServer : public cAsyncSelect
{
public:
	cServer( unsigned short srvType );
	~cServer();

public:

	cServer*		GetAgent()		{	return mpAgent;		}
	unsigned short	GetSrvType()	{	return mSrvType;	}
    const char*     GetName()       {   return mName;       }

    
    bool IsAgentOn( )		            {	return mpAgent->IsConnectResult();      }
	bool IsSend( )			            {	return mIsSend;					        }
    bool IsConnect( )		            {	return mIsConnect;	                    }
	bool IsConnectResult( )		        {	return mIsConnectResult;	            }
	bool IsSendOff( )		            {	return mIsSendOff;	                    }

    void IsSend             ( bool isSend   )   {	mIsSend             = isSend;   }
	void IsConnect          ( bool isCon    )	{	mIsConnect          = isCon;    }
	void IsConnectResult    ( bool isCon    )	{	mIsConnectResult    = isCon;    }
    void IsSendOff          ( bool isOff    )   {   mIsSendOff          = isOff;    }

	void SetAgent( cServer* p )		    {	mpAgent = p;		        }
    void SetName( const char* name )    {   strcpy_s( mName, name );    }


private:
	cServer*			mpAgent;

    bool				mIsSend;
    bool                mIsSendOff;

	unsigned short		mSrvType;
    char                mName[MAX_PATH];


public:
	int					mConnect;
	DWORD				mConnReqTime;
    
    unsigned long       mSrvNo;
    unsigned long       mChnNo;
};

typedef tPointerArray<cServer*> cSrvList;

class cNetwork
{

public:
	cNetwork( HWND hwnd );
	~cNetwork(void);

public:
	/// 초기화
	void        Init();
	bool        InitWinSock( HWND hwnd );
	cServer*    CloseSocket( SOCKET sock );
	bool        CloseSocket( cServer* srv );

	void        SetServerDlg( CServerToolDlg* dlg )	{	mDlg = dlg;		}


	/// 서버 관련 메소드
	cServer*    AddServer           ( unsigned short srvType, const char* ip, int port, cServer* agent = NULL );
	void        AddSrvList          ( cServer* srv );

    cServer*    FindSrv             ( SOCKET sock );
	cServer*    FindSrv             ( cServer* pAgent, const char* name );
    cServer*    FindDisCon( );
	cServer*    FindAgent           ( const char* ip );
	void        UpdateSendOn        ( unsigned short srvType, unsigned int index );


	bool SendToAgent( char* msg, int msgLen, unsigned short srvType );
	bool SendToAgent( char* msg, int msgLen );
	bool SendToSrv  ( char* msg, int msgLen, unsigned short srvType = -1 );
	bool SendToSrv  ( char* msg, int msgLen, cSrvList* srvList, bool sendAll = false );
	bool SendToSrv  ( char* msg, int msgLen, cServer* srv );

	bool Recv( SOCKET sock );
	
	void CheckConnect();
    void CheckConnect( unsigned short srvType );

    bool ConnectComplete( cServer* srv );

	/// Send Message
	bool MakeSendMsg( char* msg, int len );
	char* GetPacket();

	void SendHeartBeat();
	void SendPingChekReq();
	void SendSrvListReq( cServer* srv );
	void SendChnInfoReq( cServer* srv );
    void SendSrvStateReq( cServer* srv );

	void SendUserOn();
	void SendPathFinderOn();
	void SendGMOn();
    void SendTesterOn();


	cSrvList*       GetSrvList      ( unsigned short srvType );
    unsigned int    GetSrvCount     ( unsigned short srvType )  {   return mSrvList[srvType].GetSize(); }
    int             GetConnectCnt   ( unsigned short srvType );
    short           GetSrvType      ( char* name );

private:

	HWND				mHwnd;

    cSrvList            mSrvList[SRV_TYPE_END];

	char 				mBackBuffer[MAX_BUFFER];
	char 				mRecvBuffer[MAX_BUFFER];
	char 				mSendBuffer[MAX_BUFFER];

	int					mBackOffset;
	int					mRecvOffset;
	int					mSendOffset;

	CServerToolDlg*		mDlg;
};


inline bool cNetwork::ConnectComplete( cServer* srv )
{
    return srv->ConnectComplete( mHwnd ); 
}

inline cSrvList* cNetwork::GetSrvList( unsigned short srvType )
{
    cSrvList* pList = &mSrvList[srvType];
    return pList;
}

inline void cNetwork::AddSrvList( cServer* srv )
{
    unsigned short srvType = srv->GetSrvType();
    if( &mSrvList[srvType] )
        mSrvList[srvType].PushBack( srv );
}

inline short cNetwork::GetSrvType( char* name )
{
    if( NULL == name )
        return -1;

    CString lowerName( name );
    lowerName.MakeLower();
    
         if( wcsncmp(lowerName.GetBuffer(), _T("login"),    wcslen(_T("login")   )) == 0 ) return SRV_TYPE_LOGIN;
    else if( wcsncmp(lowerName.GetBuffer(), _T("game" ),    wcslen(_T("game" )   )) == 0 ) return SRV_TYPE_CHANNEL;
    else if( wcsncmp(lowerName.GetBuffer(), _T("pvp"  ),    wcslen(_T("pvp"  )   )) == 0 ) return SRV_TYPE_PVP;
    else if( wcsncmp(lowerName.GetBuffer(), _T("them" ),    wcslen(_T("them" )   )) == 0 ) return SRV_TYPE_THEMA;
    else if( wcsncmp(lowerName.GetBuffer(), _T("tutorial"), wcslen(_T("tutorial"))) == 0 ) return SRV_TYPE_TUTORIAL;
    else return -1;

    return -1;
}