// Include
#include "logdemon.h"
#include "packet.h"

// Local definitions
#pragma warning( disable: 4127 )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4267 )

#define RELAY_DEALY	1000

// Global data

// cLogSender Constructor
cLogSender::cLogSender(void)
{
}

// ~cLogSender Destructor
cLogSender::~cLogSender(void)
{
}

// Initialize Method
bool cLogSender::Initialize(char* send, char* recv, u_short numWorkerThreads, unsigned int bufferLength)
{
	PHOSTENT phe;

	// LoginServer IPv4 주소를 만든다.
	ZeroMemory( (void*)&mLogin, sizeof(SOCKADDR_IN) );
	mLogin.sin_family = AF_INET;
	mLogin.sin_port   = htons( g_loginPort );
	mLogin.sin_addr.s_addr = inet_addr( send );

	if ( mLogin.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( send )) != NULL )
			CopyMemory( &mLogin.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mLoginSeq    = 0;
	mLoginBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	mLoginRelay  = 0;

	// GameServer IPv4 주소를 만든다.
	ZeroMemory( (void*)&mGame, sizeof(SOCKADDR_IN) );
	mGame.sin_family = AF_INET;
	mGame.sin_port   = htons( g_gamePort );
	mGame.sin_addr.s_addr = inet_addr( send );

	if ( mGame.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( send )) != NULL )
			CopyMemory( &mGame.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mGameSeq    = 0;
	mGameBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	mGameRelay  = 0;

	// Receive IPv4 주소를 만든다.
	ZeroMemory( (void*)&mRecv, sizeof(SOCKADDR_IN) );
	mRecv.sin_family = AF_INET;
	mRecv.sin_port   = htons( g_logPort );
	mRecv.sin_addr.s_addr = inet_addr( recv );

	if ( mRecv.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( recv )) != NULL )
			CopyMemory( &mRecv.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}

	return cIocpUdpSend::Initialize( numWorkerThreads, bufferLength );
}

// Shutdown Method
void cLogSender::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	cIocpUdpSend::Shutdown( maxWait );

	if ( mGameBuffer )
	{
		GlobalFree( mGameBuffer );
		mGameBuffer = NULL;
	}
	if ( mLoginBuffer )
	{
		GlobalFree( mLoginBuffer );
		mLoginBuffer = NULL;
	}
}

// SendComplete Method
bool cLogSender::SendComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &mCs );
	RUDP*   rudp = (RUDP*)perIoContext->buffer;
	long    mask = (rudp->seq & SM_ClassD);

	if ( rudp->tos != TOS_RUDP )
	{
		if ( perIoContext->addr.sin_port == mLogin.sin_port )
		{
			PerIoContext** temp     = &mLoginBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = cIocpUdpSend::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mLoginBuffer->offset = max( mLoginBuffer->offset, mask );
			mLoginRelay = GetTickCount( ) + RELAY_DEALY;
			return retvalue;
		}
		else if ( perIoContext->addr.sin_port == mGame.sin_port )
		{
			PerIoContext** temp     = &mGameBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = cIocpUdpSend::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mGameBuffer->offset = max( mGameBuffer->offset, mask );
			mGameRelay = GetTickCount( ) + RELAY_DEALY;
			return retvalue;
		}
	}

	return cIocpUdpSend::SendComplete( completionKey, perIoContext, bytesTransferred );
}

// ReSendPost Method
bool cLogSender::ReSendPost(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &mCs );
	PerIoContext* ioContext = NULL;
	long          mask      = (seq & SM_ClassD);
	bool          retvalue  = false;

	if ( htons( g_loginPort ) == port )
	{
		ioContext = mLoginBuffer->buffer[ mask ];
	}
	else if ( htons( g_gamePort ) == port )
	{
		ioContext = mGameBuffer->buffer[ mask ];
	}
	else
		return retvalue;

	if ( ioContext != NULL )
	{
		RUDP* rudp = (RUDP*)ioContext->buffer;
		if ( rudp->seq == seq )
		{
			PerIoContext* sendIo = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );

			if ( sendIo != NULL )
			{
				sendIo->addr.sin_family = AF_INET;
				sendIo->addr.sin_addr.s_addr = addr;
				sendIo->addr.sin_port = port;

				memcpy( sendIo->buffer, ioContext->buffer, ioContext->offset );
				sendIo->offset = ioContext->offset;

				retvalue = SendPost( sendIo );
			}
		}
	}

	return retvalue;
}

// SendReliable Method
bool cLogSender::SendReliable(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &mCs );
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE );

	if ( perIoContext != NULL )
	{
		perIoContext->addr.sin_family = AF_INET;
		perIoContext->addr.sin_port   = port;
		perIoContext->addr.sin_addr.s_addr = addr;

		RUDP* rudp = (RUDP*)perIoContext->buffer;

		rudp->ver  = RUDP_PHVer;
		rudp->hlen = RUDP_PHLen;
		rudp->tos  = TOS_RUDP;
		rudp->tlen = rudp->hlen;
		rudp->seq  = seq;
		rudp->addr = mRecv.sin_addr.s_addr;
		rudp->port = mRecv.sin_port;

		perIoContext->offset = rudp->tlen;
		return SendPost( perIoContext );
	}
	return false;
}

// GetPacket Method
char* cLogSender::GetPacket(void** handle, char tos, SOCKADDR_IN addr, u_short& seq)
{
	cCSLock       lock( &mCs );
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE, addr );
	char*         retvalue     = NULL;

	if ( perIoContext != NULL )
	{
		RUDP* rudp = (RUDP*)perIoContext->buffer;

		rudp->ver  = RUDP_PHVer;
		rudp->hlen = RUDP_PHLen;
		rudp->tos  = tos;
		rudp->tlen = rudp->hlen;
		rudp->seq  = (++seq);
		rudp->addr = mRecv.sin_addr.s_addr;
		rudp->port = mRecv.sin_port;

		(*handle) = perIoContext;
		retvalue  = perIoContext->buffer + rudp->hlen;
	}

	return retvalue;
}

// SendPacket Method
bool cLogSender::SendPacket(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	RUDP*         rudp         = (RUDP*)perIoContext->buffer;

	rudp->tlen = rudp->hlen + (u_short)length;

	perIoContext->offset = rudp->tlen;
	return SendPost( perIoContext );
}

// BackendThread Method
DWORD cLogSender::BackendThread( )
{
	DWORD  currentTick = 0;
	DWORD  elapsedTick = 0;
	HANDLE handle      = NULL;

	while ( true )
	{
		currentTick = GetTickCount( );

		if ( mEndServer == true )
			break;

		if ( mLoginRelay < currentTick )
		{
			cCSLock lock( &mCs );
			if ( GetPacket( &handle, TOS_RELAY, mLogin, mLoginSeq ) != NULL )
				SendPacket( handle );
			mLoginRelay = currentTick + RELAY_DEALY;
		}
		if ( mGameRelay < currentTick )
		{
			cCSLock lock( &mCs );
			if ( GetPacket( &handle, TOS_RELAY, mGame, mGameSeq ) != NULL )
				SendPacket( handle );
			mGameRelay = currentTick + RELAY_DEALY;
		}

		IoContextPresent( );

		// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
		elapsedTick = GetTickCount( ) - currentTick;
		if ( elapsedTick < 16 )
		{
			Sleep( (16 - elapsedTick) );
		}
	}
	return 0;
}
