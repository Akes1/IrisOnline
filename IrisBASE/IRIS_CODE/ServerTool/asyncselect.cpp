#include "StdAfx.h"
#include "AsyncSelect.h"

cAsyncSelect::cAsyncSelect( )
{
	Init();
}

cAsyncSelect::~cAsyncSelect(void)
{
	::WSACleanup();
	CloseSocket();
}

bool cAsyncSelect::Init()
{
	mSocket		        = INVALID_SOCKET;
	mIsConnect	        = false;
    mIsConnectResult	= false;

	mPort   = 0;
	mSeq    = 0;

	ZeroMemory( mIP, sizeof(mIP) );

	return true;
}

void cAsyncSelect::CloseSocket()
{
	if( mSocket != INVALID_SOCKET )
		closesocket( mSocket );

	mSocket = INVALID_SOCKET;
	mSeq	= 0;

	mIsConnect	        = false;
    mIsConnectResult    = false;
}

bool cAsyncSelect::CreateSocket( HWND hWnd )
{
	if( INVALID_SOCKET != mSocket || mIsConnect )
		CloseSocket();

	/// 家南 积己
	mSocket = socket( AF_INET , SOCK_STREAM , IPPROTO_TCP );
	if( INVALID_SOCKET == mSocket )
		return false;

    int ret = WSAAsyncSelect( mSocket , hWnd , WM_SOCKEVENT , FD_CONNECT | FD_CLOSE );
	if( ret == SOCKET_ERROR )
		return false;

	return true;
}

bool cAsyncSelect::ConnectComplete( HWND hWnd )
{
    if( mIsConnect )
    {
	    int ret = WSAAsyncSelect( mSocket , hWnd , WM_SOCKEVENT , FD_READ | FD_WRITE | FD_CLOSE );
	    if( ret == SOCKET_ERROR )
		    return false;
    }

	return true;
}

bool cAsyncSelect::Connect( HWND hWnd )
{
    if( INVALID_SOCKET == mSocket )
    {
        if( !CreateSocket( hWnd ) )
            return false;
    }

    int errorcode = connect( mSocket, (SOCKADDR*)&mAddr, sizeof(mAddr) );
    if( errorcode == SOCKET_ERROR ) 
    {
        int err = WSAGetLastError();
        if( err != WSAEWOULDBLOCK )
        {
            char str[256] = {0,};
            sprintf_s( str, "cAsyncSelect::Connect - WSAGetLastError [sock:%d] [errCode:%d][ip:%s,port:%d]\n", mSocket, err, mIP, mPort );
            WriteLog( str );

            return false; 
        }
    }

    return true;
}

bool cAsyncSelect::Connect( HWND hWnd, const char* ip, const int port )
{
	CopyMemory( mIP, ip, strlen(ip) );
	mPort = port;

	ZeroMemory( (void*)&mAddr, sizeof(mAddr) );
	mAddr.sin_family        = AF_INET;
	mAddr.sin_addr.s_addr   = inet_addr(mIP);
	mAddr.sin_port          = htons(mPort);

	return Connect( hWnd );
}

bool cAsyncSelect::Send( char* sendMsg, unsigned long length )
{
	if( !mIsConnect )
		return false;

	/// 菩哦狼 sequence Number
	Packet* p	= (Packet*)sendMsg;
	p->seq		= ++mSeq;

	int ret = send( mSocket, sendMsg, length, 0 );
	if( ret == SOCKET_ERROR ) 
	{
		int err = WSAGetLastError();
		if( err != WSAEWOULDBLOCK ) 
		{
            char str[256] = {0,};
            sprintf_s( str, "Send SOCKET ERROR - [sock:%d] [code:%d][ip:%s,port:%d]\n", mSocket, err, mIP, mPort );
            WriteLog( str );

			return false; 
		}
	}
	return true;
}

int cAsyncSelect::Recv( char* recvMsg, unsigned long length )
{
	int ret = recv( mSocket, recvMsg, length, 0 );
	if( ret == SOCKET_ERROR ) 
	{ 
		int err = WSAGetLastError();
		if( err != WSAEWOULDBLOCK ) 
		{
            char str[256] = {0,};
            sprintf_s( str, "Recv SOCKET ERROR - [sock:%d] [code:%d][ip:%s,port:%d]\n", mSocket, err, mIP, mPort );
            WriteLog( str );
		}
	}

	return ret;
}