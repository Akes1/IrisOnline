// Include
#include "loginsrv.h"
#include "packet.h"

// Local definitions
#pragma warning( disable: 4127 )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4267 )

#define RELAY_DEALY	1000

// Global data

// cSender Constructor
cSender::cSender(void)
{
}

// ~cSender Destructor
cSender::~cSender(void)
{
}

// Initialize Method
bool cSender::Initialize(char* send, char* recv, u_short numWorkerThreads, unsigned int bufferLength)
{
	PHOSTENT phe;

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

	// LogDemon IPv4 주소를 만든다.
	ZeroMemory( (void*)&mLog, sizeof(SOCKADDR_IN) );
	mLog.sin_family = AF_INET;
	mLog.sin_port   = htons( g_logPort );
	mLog.sin_addr.s_addr = inet_addr( send );

	if ( mLog.sin_addr.s_addr == INADDR_NONE )
	{
		// the host name for the server is not in dot format, therefore try it just as a string
		if ( (phe = gethostbyname( send )) != NULL )
			CopyMemory( &mLog.sin_addr, phe->h_addr_list[0], phe->h_length );
		else
			return false;
	}
	mLogSeq    = 0;
	mLogBuffer = (IoContextBuffer*)GlobalAlloc( GPTR, sizeof(IoContextBuffer) );
	mLogRelay  = 0;

	// Receive IPv4 주소를 만든다.
	ZeroMemory( (void*)&mRecv, sizeof(SOCKADDR_IN) );
	mRecv.sin_family = AF_INET;
	mRecv.sin_port   = htons( g_loginPort );
	mRecv.sin_addr.s_addr = inet_addr( recv );

	if ( mLog.sin_addr.s_addr == INADDR_NONE )
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
void cSender::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	cIocpUdpSend::Shutdown( maxWait );

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
}

// SendComplete Method
bool cSender::SendComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &mCs );
	RUDP*   rudp = (RUDP*)perIoContext->buffer;
	long    mask = (rudp->seq & SM_ClassD);

	if ( rudp->tos != TOS_RUDP )
	{
		if ( perIoContext->addr.sin_port == mGame.sin_port )
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
		else if ( perIoContext->addr.sin_port == mLog.sin_port )
		{
			PerIoContext** temp     = &mLogBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = cIocpUdpSend::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mLogBuffer->offset = max( mLogBuffer->offset, mask );
			mLogRelay = GetTickCount( ) + RELAY_DEALY;
			return retvalue;
		}
	}

	return cIocpUdpSend::SendComplete( completionKey, perIoContext, bytesTransferred );
}

// ReSendPost Method
bool cSender::ReSendPost(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &mCs );
	PerIoContext* ioContext = NULL;
	long          mask      = (seq & SM_ClassD);
	bool          retvalue  = false;

	if ( htons( g_gamePort ) == port )
	{
		ioContext = mGameBuffer->buffer[ mask ];
	}
	else if ( htons( g_logPort ) == port )
	{
		ioContext = mLogBuffer->buffer[ mask ];
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
bool cSender::SendReliable(u_long addr, u_short port, u_long seq)
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
char* cSender::GetPacket(void** handle, char tos, SOCKADDR_IN addr, u_short& seq)
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
bool cSender::SendPacket(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	RUDP*         rudp         = (RUDP*)perIoContext->buffer;

	rudp->tlen = rudp->hlen + (u_short)length;

	perIoContext->offset = rudp->tlen;
	return SendPost( perIoContext );
}

// GetRegisterDate Method
void cSender::GetRegisterDate(TIMESTAMP_STRUCT* ts)
{
	SYSTEMTIME st;
	GetLocalTime( &st );
	ts->year     = st.wYear;
	ts->month    = st.wMonth;
	ts->day      = st.wDay;
	ts->hour     = st.wHour;
	ts->minute   = st.wMinute;
	ts->second   = st.wSecond;
	ts->fraction = st.wMilliseconds * 1000000;
}

// PostServerEvent Method
bool cSender::PostServerEvent(char* message)
{
	HANDLE        handle      = NULL;
	SERVER_EVENT* serverEvent = (SERVER_EVENT*)GetPacket( &handle, TOS_LOGIN_LOG, mLog, mLogSeq );
	if ( serverEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(SERVER_EVENT) - sizeof(serverEvent->message);

		serverEvent->protocol = MB_SERVER_EVENT;
		GetRegisterDate( &serverEvent->registerDate );
		strcpy( serverEvent->source, g_serviceName );
		strcpy( serverEvent->computer, computer );
		strcpy( serverEvent->message, message );
		
		length += strlen(serverEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostMemberEvent Method
bool cSender::PostMemberEvent(char type, char category, long memberIdx, char* message)
{
	HANDLE handle = NULL;
	MEMBER_EVENT* memberEvent = (MEMBER_EVENT*)GetPacket( &handle, TOS_LOGIN_LOG, mLog, mLogSeq );
	if ( memberEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(MEMBER_EVENT) - sizeof(memberEvent->message);

		memberEvent->protocol = MB_MEMBER_EVENT;
		GetRegisterDate( &memberEvent->registerDate );
		memberEvent->type = type;
		strcpy( memberEvent->source, g_serviceName );
		memberEvent->category = category;
		memberEvent->memberIdx = memberIdx;
		strcpy( memberEvent->computer, computer );
		strcpy( memberEvent->message, message );

		length += strlen( memberEvent->message );
		return SendPacket( handle, length );
	}
	return false;
}

// PostConcurrentEvent Method
bool cSender::PostConcurrentEvent(long serverIdx, long minimum, long maximum, char* message)
{
	HANDLE handle = NULL;
	CONCURRENT_EVENT* concurrentEvent = (CONCURRENT_EVENT*)GetPacket( &handle, TOS_LOGIN_LOG, mLog, mLogSeq );
	if ( concurrentEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(CONCURRENT_EVENT) - sizeof(concurrentEvent->message);

		concurrentEvent->protocol = MB_CONCURRENT_EVENT;
		GetRegisterDate( &concurrentEvent->registerDate );
		concurrentEvent->serverIdx = serverIdx;
		concurrentEvent->minimum = minimum;
		concurrentEvent->maximum = maximum;
		strcpy( concurrentEvent->computer, computer );
		strcpy( concurrentEvent->message, message );

		length += strlen( concurrentEvent->message );
		return SendPacket( handle, length );
	}
	return false;
}

// PostChSync Method
bool cSender::PostChSync( )
{
	HANDLE          handle    = NULL;
	MB_SYN_CH_SYNC* synChSync = (MB_SYN_CH_SYNC*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	if ( synChSync != NULL )
	{
		synChSync->protocol = MB_CH_SYNC_SYN;
		return SendPacket( handle, sizeof(MB_SYN_CH_SYNC) );
	}
	return false;
}

// PostChList Method
bool cSender::PostChList(ServerTable* root)
{
	HANDLE          handle    = NULL;
	MB_SYN_CH_LIST* synChList = (MB_SYN_CH_LIST*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	if ( synChList != NULL )
	{
		synChList->protocol = MB_CH_LIST_SYN;

		PerChannel* ch = root->pool;
		for ( BYTE i = 0; i < root->channelCounter; i++ )
		{
			synChList->channels[ i ].cid    = ch->cid;
			synChList->channels[ i ].status = ch->status;
			ch = ch->next;

			synChList->rowCount++;
		}
		return SendPacket( handle, synChList->Length( ) );
	}
	return false;
}

// PostChStatus Method
bool cSender::PostChStatus(long cid, BYTE status)
{
	HANDLE            handle      = NULL;
	MB_SYN_CH_STATUS* synChStatus = (MB_SYN_CH_STATUS*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	if ( synChStatus != NULL )
	{
		synChStatus->protocol = MB_CH_STATUS_SYN;
		synChStatus->cid      = cid;
		synChStatus->status   = status;
		return SendPacket( handle, sizeof(MB_SYN_CH_STATUS) );
	}
	return false;
}

// BackendThread Method
DWORD cSender::BackendThread( )
{
	DWORD  currentTick = 0;
	DWORD  elapsedTick = 0;
	HANDLE handle      = NULL;

	while ( true )
	{
		currentTick = GetTickCount( );

		if ( mEndServer == true )
			break;

		if ( mGameRelay < currentTick )
		{
			cCSLock lock( &mCs );
			if ( GetPacket( &handle, TOS_RELAY, mGame, mGameSeq ) != NULL )
				SendPacket( handle );
			mGameRelay = currentTick + RELAY_DEALY;
		}
		if ( mLogRelay < currentTick )
		{
			cCSLock lock( &mCs );
			if ( GetPacket( &handle, TOS_RELAY, mLog, mLogSeq ) != NULL )
				SendPacket( handle );
			mLogRelay = currentTick + RELAY_DEALY;
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
