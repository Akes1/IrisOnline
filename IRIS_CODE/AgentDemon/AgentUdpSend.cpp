// Include
#include "agentdemon.h"
#include "packet.h"

// Local definitions

// Global data
void GetRegisterDate( TIMESTAMP_STRUCT* ts )
{
	SYSTEMTIME si;
	GetLocalTime( &si );
	ts->year     = si.wYear;
	ts->month    = si.wMonth;
	ts->day      = si.wDay;
	ts->hour     = si.wHour;
	ts->minute   = si.wMinute;
	ts->second   = si.wSecond;
	ts->fraction = si.wMilliseconds * 1000000;
}

// AgentUdpSend Constructor.
AgentUdpSend::AgentUdpSend(void)
{
}

// ~AgentUdpSend Destructor.
AgentUdpSend::~AgentUdpSend(void)
{
}

// Initialize Method
bool AgentUdpSend::Initialize(char* addr, unsigned short numWorkerThreads, unsigned int bufferLength)
{
	PHOSTENT phe;

	// Login Server IPv4 林家甫 父电促.
	ZeroMemory( (void*)&mLogin, sizeof(SOCKADDR_IN) );
	mLogin.sin_family = AF_INET;
	mLogin.sin_port   = htons( g_loginPort );
	mLogin.sin_addr.s_addr = inet_addr( addr );

	if ( mLogin.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( addr )) != NULL )
			CopyMemory( &mLogin.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mLoginSeq = 0;
	mLoginBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );

	// Game Server IPv4 林家甫 父电促.
	ZeroMemory( (void*)&mGame, sizeof(SOCKADDR_IN) );
	mGame.sin_family = AF_INET;
	mGame.sin_port   = htons( g_gamePort );
	mGame.sin_addr.s_addr = inet_addr( addr );

	if ( mGame.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( addr )) != NULL )
			CopyMemory( &mGame.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mGameSeq = 0;
	mGameBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );

	// Game Server IPv4 林家甫 父电促.
	ZeroMemory( (void*)&mLog, sizeof(SOCKADDR_IN) );
	mLog.sin_family = AF_INET;
	mLog.sin_port   = htons( g_logPort );
	mLog.sin_addr.s_addr = inet_addr( addr );

	if ( mLog.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( addr )) != NULL )
			CopyMemory( &mLog.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mLogSeq = 0;
	mLogBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );

	return cIocpUdpSend::Initialize( numWorkerThreads, bufferLength );
}

// Shutdown Method
void AgentUdpSend::Shutdown(DWORD maxWait)
{
	EnterCriticalSection( &mCs );

	if ( mLogBuffer )
	{
		GlobalFree( mLogBuffer );
		mLogBuffer = NULL;
	}
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

	LeaveCriticalSection( &mCs );

	Sleep( 50 );
	cIocpUdpSend::Shutdown( maxWait );
}

// SendComplete Method
bool AgentUdpSend::SendComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &mCs );
	if ( mLoginBuffer != NULL && memcmp( &perIoContext->addr, &mLogin, sizeof(SOCKADDR_IN) ) == 0 )
	{
		Packet* packet = (Packet*)perIoContext->buffer;
		long    mask   = (packet->seq & 0xff);

		mLoginBuffer->offset = max( mLoginBuffer->offset, mask );
		if ( mLoginBuffer->buffer[ mask ] != NULL )
		{
			PerIoContext* temp = mLoginBuffer->buffer[ mask ];
			mLoginBuffer->buffer[ mask ] = perIoContext;
			return cIocpUdpSend::SendComplete( completionKey, temp, temp->offset );
		}
		else
		{
			mLoginBuffer->buffer[ mask ] = perIoContext;
			return true;
		}
	}
	else if ( mGameBuffer != NULL && memcmp( &perIoContext->addr, &mGame, sizeof(SOCKADDR_IN) ) == 0 )
	{
		Packet* packet = (Packet*)perIoContext->buffer;
		long    mask   = (packet->seq & 0xff);

		mGameBuffer->offset = max( mGameBuffer->offset, mask );
		if ( mGameBuffer->buffer[ mask ] != NULL )
		{
			PerIoContext* temp = mGameBuffer->buffer[ mask ];
			mGameBuffer->buffer[ mask ] = perIoContext;
			return cIocpUdpSend::SendComplete( completionKey, temp, temp->offset );
		}
		else
		{
			mGameBuffer->buffer[ mask ] = perIoContext;
			return true;
		}
	}
	else if ( mLogBuffer != NULL && memcmp( &perIoContext->addr, &mLog, sizeof(SOCKADDR_IN) ) == 0 )
	{
		Packet* packet = (Packet*)perIoContext->buffer;
		long    mask   = (packet->seq & 0xff);

		mLogBuffer->offset = max( mLogBuffer->offset, mask );
		if ( mLogBuffer->buffer[ mask ] != NULL )
		{
			PerIoContext* temp = mLogBuffer->buffer[ mask ];
			mLogBuffer->buffer[ mask ] = perIoContext;
			return cIocpUdpSend::SendComplete( completionKey, temp, temp->offset );
		}
		else
		{
			mLogBuffer->buffer[ mask ] = perIoContext;
			return true;
		}
	}
	else
		return cIocpUdpSend::SendComplete( completionKey, perIoContext, bytesTransferred );
}

// GetPacket Method
MSGBUF* AgentUdpSend::GetPacket(void** handle, char tos, char protocol, SOCKADDR_IN addr, u_long& seq)
{
	PerIoContext* ioContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE, addr );
	MSGBUF*       msgBuf    = NULL;

	if ( ioContext != NULL )
	{
		Packet* packet = (Packet*)ioContext->buffer;

		packet->ver  = RUDP_PHVer;
		packet->hlen = RUDP_PHLen;
		packet->tos  = tos;
		packet->tlen = packet->hlen;
		packet->seq  = (++seq);

		(*handle) = ioContext;
		msgBuf = (MSGBUF*)(ioContext->buffer + packet->hlen);
		msgBuf->protocol = protocol;
	}

	return msgBuf;
}

// SendPacket Method
bool AgentUdpSend::SendPacket(void* handle, u_long length)
{
	PerIoContext* ioContext = (PerIoContext*)handle;
	Packet*       packet    = (Packet*)ioContext->buffer;

	packet->tlen = packet->hlen + (u_short)length;

	ioContext->offset = packet->tlen;
	return SendPost( ioContext );
}

// PostServerEvent Method
bool AgentUdpSend::PostServerEvent(char* message)
{
	HANDLE        handle      = NULL;
	SERVER_EVENT* serverEvent = (SERVER_EVENT*)GetPacket( &handle, TOS_GAME_LOG, MB_SERVER_EVENT, mLog, mLogSeq );
	if ( serverEvent )
	{
		char*  computer = g_agentDemon->GetHostName( );
		u_long length   = sizeof(SERVER_EVENT) - sizeof(serverEvent->message);

		GetRegisterDate( &serverEvent->registerDate );
		strcpy( serverEvent->source, g_serviceName );
		strcpy( serverEvent->computer, computer );
		strcpy( serverEvent->message, message );

		length += (u_long)strlen( serverEvent->message );
		return SendPacket( handle, length );
	}
	return false;
}
