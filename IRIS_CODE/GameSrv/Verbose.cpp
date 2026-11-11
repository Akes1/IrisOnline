// Include
#include "gamesrv.h"
#include "packet.h"
#include <sys/timeb.h>

// Global data
cVerbose* cVerbose::mVerbose = NULL;

// cVerbose Constructor.
cVerbose::cVerbose(void)
{
	mVerbose = this;
}

// ~cVerbose Destructor.
cVerbose::~cVerbose(void)
{
	mVerbose = NULL;
}

// WriteLog Method
void cVerbose::WriteLog(char* msg, ...)
{
	if ( g_verbose == true )
	{
		struct timeb timebuffer;
		struct tm *now;
		time_t ltime;
		int    milisec;

		ftime(&timebuffer);
		ltime   = timebuffer.time;
		milisec = timebuffer.millitm;
		now     = localtime(&ltime);

		printf("\n[%02d:%02d:%02d.%03d] ", now->tm_hour, now->tm_min, now->tm_sec, milisec);

		static char buff[1024];
		va_list     vl;

		va_start( vl, msg );
		vsprintf( buff, msg, vl );
		va_end( vl );

		printf( "%s", buff );
	}
}

// AcceptComplete Method
void cVerbose::AcceptComplete(CRITICAL_SECTION* /*cs*/, void* ptr)
{
	if ( g_verbose == false ) return;

	PerSocketContext* perSocketContext = (PerSocketContext*)ptr;
	char*             inetnum          = inet_ntoa( perSocketContext->addr.sin_addr );

	printf( "AcceptComplete\n" );
	printf( "\t[SOCKET:%d] %s (에서) 연결되었습니다.\n", perSocketContext->socket, inetnum );
}

void cVerbose::CloseSocket(CRITICAL_SECTION* /*cs*/, void* ptr)
{
	if ( g_verbose == false ) return;

	PerSocketContext* perSocketContext = (PerSocketContext*)ptr;
	char*             inetnum          = inet_ntoa( perSocketContext->addr.sin_addr );

	printf( "SocketManager\n" );
	printf( "\t[SOCKET:%d] %s (에서) 연결이 종료되었습니다.\n", perSocketContext->socket, inetnum );
}

// BatchComplete Method
void cVerbose::BatchComplete(CRITICAL_SECTION* /*cs*/, void* ptr)
{
	if ( g_packet == false ) return;

	Packet*  packet  = (Packet*)ptr;
	MSGROOT* msgRoot = (MSGROOT*)((char*)ptr + packet->hlen);

	SYSTEMTIME st;
	GetLocalTime( &st );

	// 메시지헤더(Msg Header) 정보출력 - 카테고리/프로토콜.
	printf( "[%02d:%02d:%02d.%03d] GameProcess::BatchComplete:MsgRoot::Category:%d/Protocol:%d.\n",
			st.wHour,
			st.wMinute,
			st.wSecond,
			st.wMilliseconds,
			msgRoot->Category,
			msgRoot->Protocol );
}

// CallbackComplete Method
void cVerbose::CallbackComplete(CRITICAL_SECTION* cs, void* ptr)
{
	if ( g_verbose == false ) return;

	cCSLock       lock( cs );
	PerIoContext* perIoContext = (PerIoContext*)ptr;

	if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
	{
		// 패킷해더(Packet Header) 정보 출력 - 버전/헤더길이/서비스 종류/전체길이.
		printf( "GAMEPROCESS::CALLBACK COMPLETE::iParam(=%d):RequestResult(=%d).\n", perIoContext->iParam, perIoContext->requestResult );
	}
}
