#include "StdAfx.h"
#include "ServerToolDlg.h"
#include "Network.h"

class CServerToolDlg;

cServer::cServer( unsigned short srvType )
{
	mpAgent			= NULL;
	mIsSend			= false;
    mIsSendOff      = false;
	mSrvType		= srvType;
	mConnect		= 0;
	mConnReqTime	= 0;

    mSrvNo          = 0;
    mChnNo          = 0;
}

cServer::~cServer()
{

}

cNetwork::cNetwork( HWND hwnd )
{
	Init();
	InitWinSock( hwnd );
}

cNetwork::~cNetwork(void)
{
	Init();
}

void cNetwork::Init()
{
	mHwnd = NULL;

	ZeroMemory( mSendBuffer,    sizeof(mSendBuffer) );
	ZeroMemory( mRecvBuffer,    sizeof(mRecvBuffer) );
	ZeroMemory( mBackBuffer,    sizeof(mBackBuffer) );

	mBackOffset	= 0;
	mRecvOffset	= 0;
	mSendOffset	= 0;

    for( int i = 0; i < SRV_TYPE_END; i++ )
    {
        cSrvList::cIterator istart	= mSrvList[i].Begin();
        cSrvList::cIterator iend	= mSrvList[i].End();

        for( ; istart != iend ; ++istart )
            SAFE_DELETE( *istart );

        mSrvList[i].Clear();
    }
}

bool cNetwork::InitWinSock( HWND hwnd )
{
	mHwnd = hwnd;

	/// 윈속 버전 초기화
	WSADATA wsaData;
	int nRet = WSAStartup( MAKEWORD(2,2) , &wsaData );

	if( 0 != nRet )
		return false;

	return true;
}

cServer* cNetwork::CloseSocket( SOCKET sock )
{
	if( INVALID_SOCKET == sock )
		return NULL;

	cServer* srv = FindSrv( sock );
	if( NULL == srv )
		return NULL;

	if( CloseSocket( srv ) == false )
		return NULL;

	return srv;
}

bool cNetwork::CloseSocket( cServer* srv )
{
	if( NULL == srv )
		return false;

	srv->CloseSocket();
	return true;
}

cServer* cNetwork::AddServer( unsigned short srvType, const char* ip, int port, cServer* agent /* = NULL  */ )
{
	cServer* pAddSrv = new cServer( srvType );
	ASSERT( pAddSrv );

	bool connect = pAddSrv->Connect( mHwnd, ip, port );
    pAddSrv->IsConnect( connect );

	if( srvType != SRV_TYPE_AGENT )
		pAddSrv->SetAgent( agent );

	AddSrvList( pAddSrv );
 
	return pAddSrv;
}

cServer* cNetwork::FindAgent( const char* ip )
{
	if( NULL == *ip )
		return NULL;

    cSrvList* pAgent = &mSrvList[SRV_TYPE_AGENT];
    if( !pAgent )
        return NULL;

	/// find agent
	cSrvList::cIterator istart	= pAgent->Begin();
	cSrvList::cIterator iend	= pAgent->End();

	for( ; istart != iend ; ++istart )
	{
        cServer* agent = (*istart);
        if( agent )
        {
            if( strcmp(agent->GetIp(), ip) == false )
            {
                return *istart;
                break;
            }
        }
	}

	return NULL;
}

cServer* cNetwork::FindSrv( SOCKET sock )
{
	if( INVALID_SOCKET == sock )
		return NULL;

    for( int i = 0; i < SRV_TYPE_END; i++ )
    {
        cSrvList::cIterator istart	= mSrvList[i].Begin();
        cSrvList::cIterator iend	= mSrvList[i].End();

        for( ; istart != iend ; ++istart )
        {
            cServer* pSrv = (*istart);
            if( pSrv && pSrv->GetSocket() == sock )
            {
                return *istart;
                break;
            }
        } // istart for
    }
	return NULL;
}


cServer* cNetwork::FindDisCon( )
{
    for( int i = 0; i < SRV_TYPE_END; i++ )
    {
        cSrvList::cIterator istart	= mSrvList[i].Begin();
        cSrvList::cIterator iend	= mSrvList[i].End();

        for( ; istart != iend ; ++istart )
        {
            cServer* pSrv = (*istart);
            if( !pSrv )
                continue;

            if( pSrv->GetSocket() != INVALID_SOCKET && pSrv->IsConnectResult() == false )
            {
                return *istart;
                break;
            }
        } // istart for
    }
	return NULL;
}


cServer* cNetwork::FindSrv( cServer* pAgent, const char* name )
{
	if( NULL == name )
		return NULL;

    for( int i = SRV_TYPE_LOGIN; i < SRV_TYPE_END; i++ )
    {
        /// find agent
        cSrvList::cIterator istart	= mSrvList[i].Begin();
        cSrvList::cIterator iend	= mSrvList[i].End();

        for( ; istart != iend ; ++istart )
        {
            cServer* pSrv = (*istart);
            if( !pSrv ) continue;
            
            if( pSrv->GetAgent() == pAgent && strcmp( pSrv->GetName(), name ) == 0 )
                return *istart;
        }
    }

	return NULL;
}

bool cNetwork::Recv( SOCKET sock )
{
	cServer* srv = FindSrv( sock );
	if ( NULL == srv )
		return false;

	/// RecvBuffer에 처리할 패킷을 담는다.
	ZeroMemory( mRecvBuffer, sizeof(mRecvBuffer) );
	mRecvOffset = 0;

    /// BackBuffer에 recv
	int recvLen	= sizeof(mBackBuffer) - mBackOffset;
	int ret		= srv->Recv( mBackBuffer + mBackOffset, recvLen ); 
    mBackOffset += ret;

	if( mBackOffset > 0 )
	{
		CopyMemory( mRecvBuffer, mBackBuffer, mBackOffset );
		mRecvOffset = mBackOffset;
	}

	/// RecvBuffer의 패킷 처리
	unsigned int remainLen  = mRecvOffset;
	unsigned int offset	    = 0;
    unsigned int tlen       = 0;

	char str[256];
	while( mRecvOffset >= PHLen )
	{
		Packet* packet = (Packet*)(mRecvBuffer + offset);

/*
		/// 이어받기 확인 로그 출력.
		if( mBackOffset > 0 )
		{
			MSGROOT* rMsg = (MSGROOT*)(mRecvBuffer + offset + packet->hlen);
			wsprintf( str, _T("이어받기 확인 로그 출력 : remainLen(%d) < packet->tlen(%d)::%d, Category:%d, Protocol:%d] [offset:%d]\n"),  
                                                         remainLen, packet->tlen, packet->tos, rMsg->Category, rMsg->Protocol, offset );
			WriteLog(str); 
		}
*/
		if( packet->hlen != PHLen )
		{
            sprintf_s( str, "[cNetwork::Recv] packet->hlen(%d) < PHLen ::[%s:%d] %d\n",  packet->hlen, packet->tos, srv->GetIp(), srv->GetPort() );
			WriteLog( str );
			remainLen = 0;
			break;
		}

		if( packet->tlen < PHLen )
		{
			sprintf_s( str, "[cNetwork::Recv] packet->tlen(%d) < PHLen ::[%s:%d] %d\n",  packet->tlen, packet->tos, srv->GetIp(), srv->GetPort() );
			WriteLog(str);
			remainLen = 0;
			break;
		}

		/// 버퍼에 남은 패킷이 다 받지 않은 경우
		if( remainLen < packet->tlen )
		{
			MSGROOT* rMsg = (MSGROOT*)(mRecvBuffer + offset + packet->hlen);
			sprintf_s( str, "[cNetwork::Recv] remainLen(%d) < packet->tlen(%d)::%d, Category:%d, Protocol:%d] [offset:%d][%s:%d]\n",  
                                                    remainLen, packet->tlen, packet->tos, rMsg->Category, rMsg->Protocol, offset, srv->GetIp(), srv->GetPort() );
			WriteLog( str );
			break;
		}

		/// tos 체크
		switch( packet->tos )
		{
		case TOS_LOGIN_ACCEPT:
		case TOS_GAME_ACCEPT:
		case TOS_TTL:
			break;
		case TOS_LOGIN:
		case TOS_GAME:
		case TOS_SERVERTOOL:
			mDlg->RecvMsgParser( srv, (char*)(mRecvBuffer + offset + packet->hlen) );
			break;
		default : break;
		}
	
		offset		+= packet->tlen;
		remainLen	-= packet->tlen; 
        tlen        = packet->tlen;


        if( MAX_BUFFER - offset < remainLen )
        {
            MSGROOT* rMsg = (MSGROOT*)(mRecvBuffer + offset + packet->hlen);
            sprintf_s( str, ("[cNetwork::Recv] MAX_BUFFER - offset(%d) < remainLen(%d):: tlen(%d) Category:%d, Protocol:%d][%s:%d]\n"), 
                                            offset, remainLen, tlen, rMsg->Category, rMsg->Protocol, srv->GetIp(), srv->GetPort() );
            WriteLog(str);          
            return false;
        }

        /// 남은 패킷이 없다..
        if( remainLen <= 0 )
            break;
    }

    if( MAX_BUFFER - offset < remainLen )
    {
        sprintf_s( str, "[cNetwork::Recv] MAX_BUFFER - offset(%d) < remainLen(%d):: tlen(%d)[%s:%d] \n", offset, remainLen, tlen, srv->GetIp(), srv->GetPort() );
        WriteLog(str);          
        return false;
    }

    /// BackBuffer 초기화
    ZeroMemory( mBackBuffer, sizeof(mBackBuffer) );
    mBackOffset = 0;

	/// 남은 패킷은 이어받도록 BackBuffer에 다시 옮겨둔다.
	if( remainLen > 0 )
	{
		CopyMemory( mBackBuffer, (mRecvBuffer + offset), remainLen );
		mBackOffset = remainLen;
		//WriteLog( "[cNetwork::Recv] [이어받기!]\n" );
	}

	return true;
}

void cNetwork::CheckConnect()
{
    for( unsigned short i = 0; i < SRV_TYPE_END; i++ )
	    CheckConnect( i );
}

void cNetwork::CheckConnect( unsigned short srvType )
{
    cSrvList* pList = &mSrvList[srvType];
    if( !pList )
        return ;

	cSrvList::cIterator istart	= pList->Begin();
	cSrvList::cIterator iend	= pList->End();

	for( ; istart != iend ; ++istart )
	{
		if( (*istart)->mConnect >= REQUEST_CONNECT_MAX )
		{
			if( (*istart)->mConnReqTime <  GetTickCount() )
			{
				(*istart)->mConnReqTime = 0;
				(*istart)->mConnect = 0;
			}
			else
				continue;
		}

		if( (*istart)->IsConnectResult() == false )
		{
			bool connect = (*istart)->Connect( mHwnd );
            (*istart)->IsConnect( connect );

			(*istart)->mConnect++;
			(*istart)->mConnReqTime = GetTickCount() + ( 1 * MINUTE );

            PostMessage( mHwnd, WM_SOCKEVENT, (WPARAM)FD_CONNECT, (LPARAM)0L );
		}
	}
}

int cNetwork::GetConnectCnt( unsigned short srvType ) 
{
    cSrvList* pList = &mSrvList[srvType];
    if( !pList )
        return 0;

    int connect = 0;
	cSrvList::cIterator istart	= pList->Begin();
	cSrvList::cIterator iend	= pList->End();

	for( ; istart != iend ; ++istart )
	{
        if( (*istart)->IsConnectResult() )
            ++connect;
    }
    return connect;
}

bool cNetwork::SendToAgent( char* msg, int msgLen )
{
    cSrvList* pList = &mSrvList[SRV_TYPE_AGENT];
    if( !pList )
        return false;;

	MakeSendMsg( msg, msgLen );

	cSrvList::cIterator istart	= pList->Begin();
	cSrvList::cIterator iend	= pList->End();

	for( ; istart != iend ; ++istart )
	{
		cServer* pAgent = (*istart);
		if( pAgent )
        {
            if( pAgent->IsConnectResult() != true )
			    continue;
		    pAgent->Send( mSendBuffer, mSendOffset );
        }
	}

	ZeroMemory( mSendBuffer, sizeof(mSendBuffer) );
	return true;
}

bool cNetwork::SendToAgent( char* msg, int msgLen, unsigned short srvType )
{
    if( srvType <= SRV_TYPE_BEGIN )
        return false;

    cSrvList* pList = &mSrvList[srvType];
    if( !pList )
        return false;;

    MakeSendMsg( msg, msgLen );

	cSrvList::cIterator istart	= pList->Begin();
    cSrvList::cIterator iend	= pList->End();

    for( ; istart != iend ; ++istart )
    {
        if( *istart )
        {
            if( (*istart)->IsSend() != true )
                continue;

            cServer* pAgent = (*istart)->GetAgent();
            if( pAgent && pAgent->IsConnectResult() )
            {
                pAgent->Send( mSendBuffer, mSendOffset );
                (*istart)->IsSend( false );
            }
        }
	}

	ZeroMemory( mSendBuffer, sizeof(mSendBuffer) );
	return true;
}

char* cNetwork::GetPacket()
{
	Packet* packet = (Packet*)(mSendBuffer + mSendOffset);

	if( NULL == packet )
		return NULL;

	packet->ver		= PHVer;
	packet->hlen	= PHLen;
	packet->tos		= TOS_SERVERTOOL;
	packet->tlen	= packet->hlen;

	return (char*)(mSendBuffer + mSendOffset);
}

bool cNetwork::MakeSendMsg( char* msg, int len )
{
	ZeroMemory( mSendBuffer, sizeof(mSendBuffer) );
	mSendOffset = 0;

	Packet* packet	= (Packet*)GetPacket();
	packet->tlen	= packet->hlen + len;
	mSendOffset		+= packet->hlen;

	CopyMemory( mSendBuffer + mSendOffset, msg, len );
	mSendOffset	+= len;

	return true;
}

bool cNetwork::SendToSrv( char* msg, int msgLen, cServer* srv )
{
    if( NULL == srv )
        return false;

	MakeSendMsg( msg, msgLen );
	bool ret = srv->Send( mSendBuffer, mSendOffset );

    ZeroMemory( mSendBuffer, sizeof(mSendBuffer) );
    return ret;
}

bool cNetwork::SendToSrv( char* msg, int msgLen, unsigned short srvType )
{
    if( srvType >= SRV_TYPE_BEGIN && srvType < SRV_TYPE_END )
    {
        cSrvList* pList = &mSrvList[srvType];
        if( !pList )
            return false;

        MakeSendMsg( msg, msgLen );
        SendToSrv( mSendBuffer, mSendOffset, pList, false );
    }
    else
    {
        MakeSendMsg( msg, msgLen );
        for( unsigned short i = SRV_TYPE_BEGIN; i < SRV_TYPE_END; i++ )
            SendToSrv( mSendBuffer, mSendOffset, &mSrvList[i],	true );
    }

    ZeroMemory( mSendBuffer, sizeof(mSendBuffer) );
    return true;
}

bool cNetwork::SendToSrv( char* msg, int msgLen, cSrvList* srvList, bool sendAll )
{
    if( !srvList )
        return false;

	cSrvList::cIterator i		= srvList->Begin();
	cSrvList::cIterator iend	= srvList->End();

	for( ; i != iend ; ++i )
	{
        cServer* srv = (*i);
        if( srv )
        {
            if( srv->IsConnectResult() != true )
                continue;

            if( sendAll || srv->IsSend() )
            {
                srv->Send( msg, msgLen );
                srv->IsSend( false );
            }
        }
	}

	return true;
}

void cNetwork::UpdateSendOn( unsigned short srvType, unsigned int index )	
{  
    cSrvList* pList = &mSrvList[srvType];
    if( !pList )
        return;

	if( index > pList->GetSize() )
		return;

	cServer* srv = (cServer*)*(pList->Begin() + index);
	if( NULL == srv )
		return;

	if( pList->IsEmpty() == false )
		srv->IsSend( true );
}

void cNetwork::SendSrvListReq( cServer* srv )
{
	MSGROOT sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_SERVER_LIST_REQ;

    SendToSrv( (char*)&sendMsg, sizeof(sendMsg), srv );
}

void cNetwork::SendChnInfoReq( cServer* srv )
{
	MSGROOT sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_CHANNEL_INFO_REQ;

	SendToSrv( (char*)&sendMsg, sizeof(sendMsg), srv );
}

void cNetwork::SendSrvStateReq( cServer* srv )
{
    if( srv == NULL )
        return;

    if( srv->GetSrvType() == SRV_TYPE_AGENT )
        return;

	MSG_REQ_SRVTOOL_STATUS sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_SERVER_STATUS_REQ;
    strcpy_s( sendMsg.serviceName, srv->GetName() );

    SendToSrv( (char*)&sendMsg, sizeof(sendMsg), srv->GetAgent() );
}

void cNetwork::SendHeartBeat()
{
	Packet packet;

	packet.ver	= PHVer;
	packet.hlen	= PHLen;
	packet.tos	= TOS_TTL;
	packet.tlen	= packet.hlen;

    for( unsigned short i = SRV_TYPE_BEGIN; i < SRV_TYPE_END; i++ )
        SendToSrv( (char*)&packet, sizeof(Packet), &mSrvList[i], true );
}

void cNetwork::SendPingChekReq()
{
	MSG_REQ_SRVTOOL_PING sendMsg;
 	sendMsg.Category		= NM_SERVERTOOL;
	sendMsg.Protocol		= NM_SERVERTOOL_PING_REQ;
	sendMsg.mToolSendTime	= GetTickCount(); 

	SendToSrv( (char*)&sendMsg, sizeof(sendMsg) );
}

void cNetwork::SendUserOn()
{
	MSGROOT sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_USER_ON_REQ;

	SendToSrv( (char*)&sendMsg, sizeof(sendMsg), SRV_TYPE_LOGIN );
}

void cNetwork::SendPathFinderOn()
{
	MSGROOT sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_PATHFINDER_ON_REQ;

	SendToSrv( (char*)&sendMsg, sizeof(sendMsg), SRV_TYPE_LOGIN );
}

void cNetwork::SendGMOn()
{
	MSGROOT sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_GAMEMASTER_ON_REQ;

	SendToSrv( (char*)&sendMsg, sizeof(sendMsg), SRV_TYPE_LOGIN );
}

void cNetwork::SendTesterOn()
{
	MSGROOT sendMsg;
	sendMsg.Category = NM_SERVERTOOL;
	sendMsg.Protocol = NM_SERVERTOOL_TESTER_ON_REQ;

	SendToSrv( (char*)&sendMsg, sizeof(sendMsg), SRV_TYPE_LOGIN );
}