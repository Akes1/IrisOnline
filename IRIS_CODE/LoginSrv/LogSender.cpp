// Include
#include "loginsrv.h"

// Local definitions
#pragma warning( disable: 4127 )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4267 )

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
bool cLogSender::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads)
{
	return cIocpUdpSend::Initialize( ipAddr, port, numWorkerThreads );
}

// Shutdown Method
void cLogSender::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	cIocpUdpSend::Shutdown( maxWait );
}

// GetPacket Method
MSGBUF* cLogSender::GetPacket(void** handle, char protocol)
{
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( mSocket, IOCP_REQUEST_WRITE, mServerAddr );
	MSGBUF*       msgBuf       = NULL;

	if ( perIoContext != NULL )
	{
		Packet* packet = (Packet*)perIoContext->buffer;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_LOGIN;
		packet->tlen = packet->hlen;

		(*handle) = perIoContext;
		msgBuf    = (MSGBUF*)(perIoContext->buffer + packet->hlen);
		msgBuf->protocol = protocol;
	}

	return msgBuf;
}

// SendPacket Method
bool cLogSender::SendPacket(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	Packet*       packet       = (Packet*)perIoContext->buffer;

	packet->tlen = packet->hlen + (u_short)length;

	perIoContext->offset = packet->tlen;
	return SendPost( perIoContext );
}

// PostServerEvent Method
bool cLogSender::PostServerEvent(char* message)
{
	HANDLE        handle      = NULL;
	SERVER_EVENT* serverEvent = (SERVER_EVENT*)GetPacket( &handle, MB_SERVER_EVENT );
	if ( serverEvent != NULL )
	{
		char*  source   = g_loginSrv->GetServiceName( );
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(SERVER_EVENT) - sizeof(serverEvent->message);

		strcpy( serverEvent->source, source );
		strcpy( serverEvent->computer, computer );
		strcpy( serverEvent->message, message );
		
		length += strlen(serverEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostMemberEvent Method
bool cLogSender::PostMemberEvent(char type, char category, long memberIdx, char* message)
{
	HANDLE handle = NULL;
	MEMBER_EVENT* memberEvent = (MEMBER_EVENT*)GetPacket( &handle, MB_MEMBER_EVENT );
	if ( memberEvent != NULL )
	{
		char*  source   = g_loginSrv->GetServiceName( );
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(MEMBER_EVENT) - sizeof(memberEvent->message);

		memberEvent->type = type;
		strcpy( memberEvent->source, source );
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
bool cLogSender::PostConcurrentEvent(long serverIdx, long minimum, long maximum, char* message)
{
	HANDLE handle = NULL;
	CONCURRENT_EVENT* concurrentEvent = (CONCURRENT_EVENT*)GetPacket( &handle, MB_CONCURRENT_EVENT );
	if ( concurrentEvent != NULL )
	{
		char*  computer = g_loginSrv->GetHostName( );
		u_long length   = sizeof(CONCURRENT_EVENT) - sizeof(concurrentEvent->message);

		concurrentEvent->serverIdx = serverIdx;
		concurrentEvent->minimum   = minimum;
		concurrentEvent->maximum   = maximum;
		strcpy( concurrentEvent->computer, computer );
		strcpy( concurrentEvent->message, message );

		length += strlen( concurrentEvent->message );
		return SendPacket( handle, length );
	}
	return false;
}
