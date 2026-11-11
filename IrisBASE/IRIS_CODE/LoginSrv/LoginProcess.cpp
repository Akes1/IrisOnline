// Include
#include "loginsrv.h"
#include "packet.h"

#include <time.h>

// Local definitions
#define MAX_ERROR_COUNT_LOGIN					5	// 로그인 시도 가능 횟수

/* macro for SQL Class checking */
#define SQL_WEB_CHECK(sqlWeb)                   \
	sqlWeb = g_loginSrv->GetSQLWeb( );          \
	if ( sqlWeb == NULL )                       \
		return false;

#define SQL_ACCOUNT_CHECK(sqlAccount)           \
	sqlAccount = g_loginSrv->GetSQLAccount( );  \
	if ( sqlAccount == NULL )                   \
		return false;


#pragma pack(1) // 1Byte 정렬 설정

struct Login2Game : Packet
{
	unsigned long  ipv4;
	unsigned short port;
};

#pragma pack( ) // 1Byte 정렬 해제


struct ClientInfo
{
	union
	{
		struct
		{
			// 0 off / 1 on
			bool login      : 1;
			bool logout     : 1;
			bool gameToGame : 1;
		} db;
		long dbData;
	};
	union
	{
		struct
		{
			// 0 off / 1 on
			bool launcher     : 1;
			bool check        : 1;
			bool login        : 1;
			bool serverList   : 1;
			bool gameToGame   : 1;
		} request;
		long requestData;
	};
	union
	{
		struct
		{
			// 0 off / 1 on
			bool login        : 1;
			bool gameToGame   : 1;
		} complete;
		long completeData;
	};
	struct
	{
		DWORD launcher;
		DWORD check;
		DWORD login;
		DWORD loginUpdate;
		DWORD serverList;
		DWORD gameToGame;
	} delay;

	long error_count;

	long login_idx;
	long member_idx;
	char member_uid[ 17 ];
	char member_pwd[ 17 ];
	char member_sec[  5 ];
	char member_slevel;
	long member_pc;
	char member_age;

	// 일본OBT 이벤트 아이템 지급 관련
	// 값 - 0:대상아님/1:이벤트대상/2:아이템지급완료
//	BYTE		cbtUser;					// CBT 참여 여부
//	BYTE		obtUser;					// OBT 신규 유저 여부
//	BYTE		fourGamer;					// 4gamer 유저 여부

	long last_server;
	long last_channel;
};

// Global data
#define MAX_RC5_KEY				16

RC5_32_KEY RC5_32_KEY_TABLE[MAX_RC5_KEY];


BYTE SY_RKEY_TABLE[16][4]=
{
	{ 0x0F,0xAD,0x37,0x6C },	//6
	{ 0x5C,0x55,0x6A,0xD8 },	//10
	{ 0x6C,0xA9,0xDB,0x64 },	//11
	{ 0x5A,0xBE,0xF2,0xF6 },	//12
	{ 0xB3,0xB7,0xEC,0x7B },	//8
	{ 0xE8,0x9B,0x3A,0x8A },	//5
	{ 0xD9,0x64,0x28,0x5C },	//7
	{ 0xA6,0x4D,0x8F,0xB9 },	//13
	{ 0x17,0xb3,0x67,0xaa },	//0
	{ 0x4B,0x1C,0x15,0x11 },	//4
	{ 0x9C,0x13,0xFE,0xF5 },	//9
	{ 0x53,0xf2,0x74,0xb1 },	//1
	{ 0x67,0x9F,0x7D,0xF6 },	//14
	{ 0x98,0xea,0x55,0x29 },	//3
	{ 0x8c,0x72,0x59,0x32 },	//2
	{ 0xCA,0x6A,0xA9,0xEF },	//15
};

BYTE SY_KEY_TABLE[16][4]=
{
	{ 0xC0,0x3C,0x06,0xEA },	//0
	{ 0x68,0xCC,0x56,0x3A },	//10
	{ 0xB9,0x78,0xB3,0x45 },	//1
	{ 0xA1,0x8A,0x0B,0x70 },	//12
	{ 0x3B,0xB1,0x60,0x0B },	//8
	{ 0x88,0x48,0xC0,0x71 },	//2
	{ 0x43,0xC8,0x53,0x63 },	//7
	{ 0x5C,0xF8,0x90,0x4A },	//11
	{ 0x54,0x1B,0x0F,0x9C },	//15
	{ 0x44,0x95,0xAF,0x99 },	//6
	{ 0x85,0x4C,0xB6,0xE7 },	//3
	{ 0x28,0x45,0x80,0xB1 },	//13
	{ 0x2D,0x83,0x06,0x55 },	//5
	{ 0x98,0xAE,0xD9,0xBF },	//4
	{ 0x71,0x9D,0x4A,0x80 },	//14
	{ 0x19,0x2D,0x57,0xC6 },	//9
};

cLoginProcess* cLoginProcess::mLoginProcess = NULL;

// cLoginProcess Constructor.
cLoginProcess::cLoginProcess(void) : mServerManager(NULL)
{
	mLoginProcess = this;

	RC5_32_KEY*   rc532Key  = RC5_32_KEY_TABLE;		//
	unsigned char tiv1[ 4 ] = {'s','L','e','e'};	//
	unsigned char tiv2[ 4 ] = {'p','H','A','h'};	//
	DWORD         skey[ 2 ];						// 4Byte * 2  = 64bit

	memset( RC5_32_KEY_TABLE, 0, sizeof(RC5_32_KEY_TABLE) );

	for ( int i = 0; i < MAX_RC5_KEY; i++, rc532Key++ )
	{
		skey[ 0 ] = (*(DWORD*)(SY_RKEY_TABLE + i)) ^ (*(DWORD*)tiv1);
		skey[ 1 ] = (*(DWORD*)(SY_RKEY_TABLE + i)) ^ (*(DWORD*)tiv2);

		RC5_32_set_key( rc532Key, sizeof(skey), (unsigned char*)&skey, RC5_8_ROUNDS );
	}

	mRequestData  = 0;
	mCompleteData = 0;

	mFrame   = 0;
	mFPSTick = 0;
	mCCUTick = 0;

	mServerManager = new cServerManager( );
	mSecurityLevel = SRVTOOL_SECURITY_LEVEL0;
}

// ~cLoginProcess Destructor.
cLoginProcess::~cLoginProcess(void)
{
	delete mServerManager;
	mServerManager = NULL;

	mLoginProcess = NULL;
}

// Initialize Method
bool cLoginProcess::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads)
{
	// IocpServer 초기화.
	return cIocpServer::Initialize( ipAddr, port, numWorkerThreads );
}

// Shutdown Method
void cLoginProcess::Shutdown(DWORD maxWait)
{
	// IocpServer 종료.
	cIocpServer::Shutdown( maxWait );
}

// PostServerEvent Method
bool cLoginProcess::PostServerEvent(LPCTSTR format, ...)
{
	cSender* sender   = g_loginSrv->GetSender( );
	bool     retvalue = false;

	if ( sender != NULL )
	{
		LPVOID  msgBuf = NULL;
		DWORD   bufferLength;

		va_list args;

		va_start( args, format );

		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );

		vsprintf( (char*)msgBuf, format, args );

		va_end( args );

		if ( msgBuf != NULL )
		{
			retvalue = sender->PostServerEvent( (char*)msgBuf );
			free( msgBuf );
		}
	}
	return retvalue;
}

// PostConcurrentEvent Method
bool cLoginProcess::PostConcurrentEvent(long serverIdx, long minimum, long maximum, LPCTSTR format, ...)
{
	cSender* sender = g_loginSrv->GetSender( );

	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;

	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) + 1;
	msgBuf       = malloc( bufferLength );

	vsprintf( (char*)msgBuf, format, args );

	va_end( args );

	bool retvalue = false;

	if ( msgBuf != NULL )
	{
		retvalue = sender->PostConcurrentEvent( serverIdx, minimum, maximum, (char*)msgBuf );
		free( msgBuf );
	}

	return retvalue;
}

// PostMemberEvent Method
bool cLoginProcess::PostMemberEvent(char type, char category, long memberIdx, LPCTSTR format, ...)
{
	cSender* sender = g_loginSrv->GetSender( );

	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;

	va_list args;

	va_start( args, format );

	bufferLength = _vscprintf( format, args ) + 1;
	msgBuf       = malloc( bufferLength );

	vsprintf( (char*)msgBuf, format, args );

	va_end( args );

	bool retvalue = false;

	if ( msgBuf != NULL )
	{
		retvalue = sender->PostMemberEvent( type, category, memberIdx, (char*)msgBuf );
		free( msgBuf );
	}

	return retvalue;
}

// RecvChStatus Method
void cLoginProcess::ChannelStatus(long cid, BYTE status)
{
	cCSLock lock( &mCs );
	bool    eventLog = true;

	WORD sNO = HIWORD(cid);
	WORD cNO = LOWORD(cid);
	char buffer[MAX_PATH];

	switch ( status )
	{
	case _E_STATUS_RUNABLE_:
	case _E_STATUS_RUNNING_S1_:
	case _E_STATUS_RUNNING_S2_:
	case _E_STATUS_RUNNING_S3_:
	case _E_STATUS_RUNNING_S4_:
		{
			ServerTable* serverTable = mServerManager->GetServerTable( cid );

			if ( serverTable != NULL )
			{
				PerChannel* ch = mServerManager->SearchChannel( serverTable, cid );

				if ( ch == NULL )
				{
					ch = mServerManager->GetChannel( serverTable, cid );
				}

				if ( ch != NULL )
				{
					if ( ch->status != status )
					{
						ch->Status( status );
						ChannelUpdate( sNO, cNO, status );
					}
					else
						eventLog = false;

					ch->TimeToLive( (GetTickCount( ) + STATUS_SYNC_WAIT) );

					if ( status == _E_STATUS_RUNABLE_ )
					{
						sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_RUNABLE_" );
						g_loginSrv->GetSender( )->PostChList( serverTable );
					}
					else
					{
						sprintf( buffer, "S(%02d)CH(%02d)=_RUNNING_S%d_", sNO, cNO, (status - _E_STATUS_RUNABLE_) );
						g_loginSrv->GetSender( )->PostChStatus( cid, status );
					}
				}
				else
					sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_CRITICAL_ERROR_" );
			}
			else
				sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_CRITICAL_ERROR_" );
		}
		break;

	case _E_STATUS_CLOSING_:
		mServerManager->ReleaseChannel( cid );
		sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_CLOSING_" );
		break;
	case _E_STATUS_ERROR_:
		mServerManager->ReleaseChannel( cid );
		sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_ERROR_" );
		break;
	default:
		sprintf( buffer, "S(%02d)CH(%02d)=%s", sNO, cNO, "_UNKNOWN_" );
		break;
	}

	if ( eventLog )
		PostServerEvent( "cLoginProcess::ChannelStatus:%s", buffer );
}

// GetMsgRoot Method
MSGROOT* cLoginProcess::GetMsgRoot(void** handle, PerSocketContext* perSocketContext)
{
	PerIoContext*     perIoContext     = NULL;
	Packet*           packet           = NULL;
	char*             buffer           = NULL;

	perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	if ( perIoContext != NULL )
	{
		packet       = (Packet*)perIoContext->buffer;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_LOGIN;
		packet->tlen = packet->hlen;
		packet->seq  = (++perSocketContext->seq);

		(*handle) = perIoContext;
		buffer = (perIoContext->buffer + packet->hlen);
		return (MSGROOT*)buffer;
	}

	return NULL;
}

// SendMsgRoot Method
bool cLoginProcess::SendMsgRoot(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	Packet*       packet       = (Packet*)perIoContext->buffer;

	packet->tlen = packet->hlen + (u_short)length;

	perIoContext->offset = packet->tlen;
	return SendPost( perIoContext );
}

// SendMsgError Method
bool cLoginProcess::SendMsgError(PerSocketContext* perSocketContext, char category, char protocol, int result )
{
	HANDLE     handle  = NULL;
	MSG_ERROR* sendMsg = (MSG_ERROR*)GetMsgRoot( &handle, perSocketContext );

	if ( sendMsg != NULL )
	{
		sendMsg->Category  = category;
		sendMsg->Protocol  = protocol;
		sendMsg->ErrorCode = result;

		return SendMsgRoot( handle, sizeof(MSG_ERROR) );
	}

	return false;
}

// MatrixCreate Method
bool cLoginProcess::MatrixCreate(long maxServer, long maxChannel)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext*  cbIoContext  = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	MATRIX_CREATE* matrixCreate = (MATRIX_CREATE*)cbIoContext->buffer;
	BOOL           retvalue;

	cbIoContext->offset = sizeof(MATRIX_CREATE);
	cbIoContext->iParam = SQL_SERVER_PROC_MATRIX_CREATE;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( NULL, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// ServerList Method
bool cLoginProcess::ServerList( )
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext* cbIoContext  = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	SERVER_LIST*  serverList = (SERVER_LIST*)cbIoContext->buffer;
	BOOL          retvalue;

	cbIoContext->offset = sizeof(SERVER_LIST) - sizeof(serverList->table);
	cbIoContext->iParam = SQL_SERVER_PROC_SERVER_LIST;

	// SQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( NULL, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// ServerDown Method
bool cLoginProcess::ServerDown(long serverNum, long channelNum)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext* cbIoContext = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	SERVER_DOWN*  serverDown  = (SERVER_DOWN*)cbIoContext->buffer;
	BOOL          retvalue;

	serverDown->serverNum  = serverNum;
	serverDown->channelNum = channelNum;

	cbIoContext->offset = sizeof(SERVER_DOWN);
	cbIoContext->iParam = SQL_SERVER_PROC_SERVER_DOWN;

	// SQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( NULL, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// ServerUpdate Method
bool cLoginProcess::ChannelUpdate(long serverNum, long channelNum, BYTE status)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext*   cbIoContext  = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	CHANNEL_UPDATE* channelUpdate = (CHANNEL_UPDATE*)cbIoContext->buffer;
	BOOL            retvalue;

	channelUpdate->serverNum  = serverNum;
	channelUpdate->channelNum = channelNum;
	channelUpdate->status     = status;
	channelUpdate->retvalue   = 0;

	cbIoContext->offset = sizeof(CHANNEL_UPDATE);
	cbIoContext->iParam = SQL_SERVER_PROC_CHANNEL_UPDATE;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( NULL, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// ConcurrentUser Method
bool cLoginProcess::ConcurrentUser(long serverNum)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext*    cbIoContext    = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	CONCURRENT_USER* concurrentUser = (CONCURRENT_USER*)cbIoContext->buffer;
	BOOL             retvalue;

	concurrentUser->serverIdx = serverNum;

	cbIoContext->offset = sizeof(CONCURRENT_USER);
	cbIoContext->iParam = SQL_SERVER_PROC_CONCURRENT_USER;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( NULL, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// WebLauncher Method
bool cLoginProcess::WebLauncher(PerSocketContext* perSocketContext, long loginIdx)
{
	cSQLWeb* sqlWeb = NULL;
	SQL_WEB_CHECK( sqlWeb );

	PerIoContext* cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	WEB_LAUNCHER* launcher    = (WEB_LAUNCHER*)cbIoContext->buffer;
	BOOL          retvalue;

	launcher->loginIdx = loginIdx;

	cbIoContext->offset = sizeof(WEB_LAUNCHER);
	cbIoContext->iParam = SQL_WEB_PROC_LAUNCHER;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlWeb->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// WebCheck Method
bool cLoginProcess::WebCheck(PerSocketContext* perSocketContext, char* uid, char* pwd, char* sec)
{
	cSQLWeb* sqlWeb = NULL;
	SQL_WEB_CHECK( sqlWeb );

	PerIoContext* cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	WEB_LOGIN*    login       = (WEB_LOGIN*)cbIoContext->buffer;
	BOOL          retvalue;

	login->idx = 0;
	strcpy( login->loginid,   uid );
	strcpy( login->passwd,    pwd );
	strcpy( login->passwdsec, sec );
	login->serversec = (mSecurityLevel == SRVTOOL_SECURITY_LEVEL0) ? 1 : 0;
	login->retvalue = 0;

	cbIoContext->offset = sizeof(WEB_LOGIN);
	cbIoContext->iParam = SQL_WEB_PROC_CHECK;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlWeb->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// WebLogin Method
bool cLoginProcess::WebLogin(PerSocketContext* perSocketContext, char* uid, char* pwd, char* sec, char* ip)
{
	cSQLWeb* sqlWeb = NULL;
	SQL_WEB_CHECK( sqlWeb );

	PerIoContext* cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	WEB_LOGIN*    login       = (WEB_LOGIN*)cbIoContext->buffer;
	BOOL          retvalue;

	login->idx = 0;
	strcpy( login->loginid,   uid );
	strcpy( login->passwd,    pwd );
	strcpy( login->passwdsec, sec );
	login->serversec = (mSecurityLevel == SRVTOOL_SECURITY_LEVEL0) ? 1 : 0;
	strcpy( login->ip,        ip  );
	login->retvalue = 0;

	cbIoContext->offset = sizeof(WEB_LOGIN);
	cbIoContext->iParam = SQL_WEB_PROC_LOGIN;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlWeb->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// WebLogin Method
bool cLoginProcess::WebSMS(char* msg)
{
	cSQLWeb* sqlWeb = NULL;
	SQL_WEB_CHECK( sqlWeb );

	PerIoContext* cbIoContext = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	WEB_TP_SMS*   webTpSMS    = (WEB_TP_SMS*)cbIoContext->buffer;
	BOOL          retvalue;

	strcpy( webTpSMS->serverName, msg);

	cbIoContext->offset = sizeof(WEB_TP_SMS);
	cbIoContext->iParam = SQL_WEB_PROC_TP_SMS;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlWeb->QueueRequest( (ULONG_PTR)NULL, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// MemberLogin Method
bool cLoginProcess::MemberLogin(PerSocketContext* perSocketContext, long memberIdx, char slevel, long pc)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	SOCKADDR_IN&  addr        = perSocketContext->addr;

	PerIoContext* cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	MEMBER_LOGIN* memberLogin = (MEMBER_LOGIN*)cbIoContext->buffer;
	char*         inetnum     = inet_ntoa( perSocketContext->addr.sin_addr );
	BOOL          retvalue;

	memberLogin->memberIdx    = memberIdx;
	memberLogin->memberSlevel = slevel;
	memberLogin->memberPc     = pc;

	strcpy( memberLogin->inetnum, inetnum );

	cbIoContext->offset = sizeof(MEMBER_LOGIN);
	cbIoContext->iParam = SQL_CLIENT_PROC_MEMBER_LOGIN;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// MemberLogout Method
bool cLoginProcess::MemberLogout(PerSocketContext* perSocketContext, long loginIdx, long memberIdx)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext*  cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	MEMBER_LOGOUT* memberLogout = (MEMBER_LOGOUT*)cbIoContext->buffer;
	BOOL           retvalue;

	memberLogout->loginIdx  = loginIdx;
	memberLogout->memberIdx = memberIdx;
	memberLogout->retvalue  = 0;

	cbIoContext->offset = sizeof(MEMBER_LOGOUT);
	cbIoContext->iParam = SQL_CLIENT_PROC_MEMBER_LOGOUT;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// LoginUpdate Method
bool cLoginProcess::LoginUpdate(PerSocketContext* perSocketContext, long loginIdx)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext*  cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	LOGIN_UPDATE*  loginUpdate = (LOGIN_UPDATE*)cbIoContext->buffer;
	BOOL           retvalue;

	loginUpdate->loginIdx = loginIdx;

	cbIoContext->offset = sizeof(LOGIN_UPDATE);
	cbIoContext->iParam = SQL_CLIENT_PROC_LOGIN_UPDATE;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// GameToGame Method
bool cLoginProcess::GameToGame(PerSocketContext* perSocketContext, long loginIdx, short serverNum, short channelNum, short lastChannel)
{
	cSQLAccount* sqlAccount = NULL;
	SQL_ACCOUNT_CHECK( sqlAccount );

	PerIoContext*  cbIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_CALLBACK );
	GAME_TO_GAME*  gameToGame  = (GAME_TO_GAME*)cbIoContext->buffer;
	BOOL           retvalue;

	gameToGame->loginIdx    = loginIdx;
	gameToGame->serverNum   = serverNum;
	gameToGame->channelNum  = channelNum;
	gameToGame->lastChannel = lastChannel;

	cbIoContext->offset = sizeof(GAME_TO_GAME);
	cbIoContext->iParam = SQL_CLIENT_PROC_GAME_TO_GAME;

	// cSQLAccount를 통해 SQLServer로 요청.
	retvalue = sqlAccount->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)cbIoContext, cbIoContext->offset );
	return (retvalue == TRUE) ? true : false;
}

// LoginSuccess Method
bool cLoginProcess::LoginSuccess(PerSocketContext* perSocketContext, long result, long memberIdx, long loginIdx, char* twitterUid, char* twitterPwd )
{
	HANDLE             handle  = NULL;
	MSG_DIST_USERINFO* sendMsg = (MSG_DIST_USERINFO*)GetMsgRoot( &handle, perSocketContext );

	if ( sendMsg != NULL )
	{
		sendMsg->Category      = NM_USER;
		sendMsg->Protocol      = NM_USER_LOGIN_RES;
		sendMsg->ErrorCode     = ERROR_DIST_USERINFO_SUCCESS;
		sendMsg->connectionIdx = (u_long)perSocketContext->cid;
		sendMsg->UserIdx       = memberIdx;
		sendMsg->LoginIdx      = loginIdx;
		strncpy( sendMsg->twitterUid, twitterUid, MAX_TWITTER_UID );
		strncpy( sendMsg->twitterPwd, twitterPwd, MAX_TWITTER_PWD );

		return SendMsgRoot( handle, sizeof(MSG_DIST_USERINFO) );
	}

	return false;
}

// ServerList Method
bool cLoginProcess::ServerList(PerSocketContext* perSocketContext, ServerTable* serverTable, char category, char protocol)
{
	HANDLE              handle  = NULL;
	MSG_RES_SERVERLIST* sendMsg = (MSG_RES_SERVERLIST*)GetMsgRoot( &handle, perSocketContext );

	if ( sendMsg != NULL )
	{
		ClientInfo*    clientInfo = (ClientInfo*)perSocketContext->buffer;
		unsigned long  length     = sizeof(MSG_RES_SERVERLIST) - sizeof(sendMsg->Servers);
		unsigned long& rowCount   = sendMsg->RowCount;

		sendMsg->Category = category;
		sendMsg->Protocol = protocol;
		sendMsg->ServerNo = clientInfo->last_server;

		for ( int i = 0; i < MAX_SERVER; i++, serverTable++ )
		{
			if ( serverTable->idx > 0 && serverTable->channelCounter > 0 )
			{
				sendMsg->Servers[ rowCount ].NumOfServer = serverTable->idx;
				wcscpy( sendMsg->Servers[ rowCount ].Name, serverTable->name );
				sendMsg->Servers[ rowCount ].Status = serverTable->status;
				sendMsg->Servers[ rowCount ].Age    = serverTable->age;
				rowCount++;
			}
		}

		length += (rowCount * sizeof(sendMsg->Servers) );
		return SendMsgRoot( handle, length );
	}

	return false;
}

// LoginToGame Method
bool cLoginProcess::LoginToGame(PerSocketContext* perSocketContext, long ipv4, short port)
{
	PerIoContext* sendIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	Login2Game*   packet        = (Login2Game*)sendIoContext->buffer;

	packet->ver  = PHVer;
	packet->hlen = PHLen;
	packet->tos  = TOS_LOGIN_2_GAME;
	packet->tlen = sizeof(Login2Game);
	packet->seq  = (++perSocketContext->seq);
	packet->ipv4 = ipv4;
	packet->port = port;

	sendIoContext->offset = packet->tlen;
	return SendPost( sendIoContext );
}

// Decrypt Method
void cLoginProcess::Decrypt(BYTE* ptr, RC5_32_KEY key)
{
	unsigned char tiv[8] = {'I','R','I','S','I','R','I','S'};
	int           num    = 0;

	Packet* packet     = (Packet*)ptr;
	LONG    decryptlen = packet->tlen;
	BYTE*   decryptptr = ptr;

	decryptlen -= packet->hlen;
	decryptptr += packet->hlen;

	if ( decryptlen > 0 )
	{
		RC5_32_cfb64_encrypt( decryptptr, decryptptr, decryptlen, &key, tiv, &num, RC5_DECRYPT );
	}
}

// Encrypt Method
void cLoginProcess::Encrypt(BYTE* ptr, RC5_32_KEY key)
{
	unsigned char tiv[8] = {'I','R','I','S','I','R','I','S'};
	int           num    = 0;

	Packet* packet     = (Packet*)ptr;
	LONG    encryptlen = packet->tlen;
	BYTE*   encryptptr = ptr;

	encryptlen -= packet->hlen;
	encryptptr += packet->hlen;

	if ( encryptlen > 0 )
	{
		RC5_32_cfb64_encrypt( encryptptr, encryptptr, encryptlen, &key, tiv, &num, RC5_ENCRYPT);
	}
}

// BatchComplete Method
bool cLoginProcess::BatchComplete(PerSocketContext* perSocketContext, void* ptr)
{
	ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

	Packet*     packet     = (Packet*)ptr;
	u_long      packetLen  = packet->tlen - packet->hlen;
	MSGROOT*    msgRoot    = (MSGROOT*)((char*)ptr + packet->hlen);

	if ( g_packet == true )
	{
		printf( "LoginProcess::BatchComplete:MSGROOT::Category:%d/Protocol:%d\n", msgRoot->Category, msgRoot->Protocol );
	}

	switch( msgRoot->Category )
	{
	default:
		switch ( msgRoot->Protocol )
		{
		case NM_USER_WEB_LAUNCHER_REQ:
			if ( packetLen == sizeof(MSG_REQ_WEB_LAUNCHER) )
			{
				try
				{
					MSG_REQ_WEB_LAUNCHER* msg = (MSG_REQ_WEB_LAUNCHER*)msgRoot;

					if ( clientInfo->complete.login || clientInfo->request.login || clientInfo->db.login )
						throw ERROR_WEB_LAUNCHER_FAIL;

					clientInfo->login_idx = msg->LoginIdx;

					clientInfo->request.launcher = true;
					clientInfo->delay.launcher = GetTickCount( ) + 500;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, error );
				}
			}
			break;
		case NM_USER_SECURITY_NUM_REQ:
			if ( packetLen == sizeof(MSG_REQ_SECURITY_NUM) )
			{
				try
				{
					MSG_REQ_SECURITY_NUM* msg = (MSG_REQ_SECURITY_NUM*)msgRoot;

					if ( clientInfo->complete.login || clientInfo->request.login || clientInfo->db.login )
						throw ERROR_DIST_USERINFO_FAIL;

					msg->sec[ MAX_SEC_SIZE ] = 0;
					strcpy( clientInfo->member_sec, msg->sec );

					clientInfo->request.check = true;
					clientInfo->delay.check = GetTickCount( ) + 500;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, error );
				}
			}
			break;

		case NM_USER_LOGIN_REQ:
			if ( packetLen == sizeof(MSG_IDPASS) )
			{
				try
				{
					MSG_IDPASS* msg = (MSG_IDPASS*)msgRoot;

					if ( clientInfo->complete.login || clientInfo->request.login || clientInfo->db.login )
						throw ERROR_DIST_USERINFO_FAIL;

					msg->uid[ MAX_UID_SIZE ] = 0;
					msg->pwd[ MAX_PWD_SIZE ] = 0;

					strcpy( clientInfo->member_uid, msg->uid );
					strcpy( clientInfo->member_pwd, msg->pwd );
					strcpy( clientInfo->member_sec, "0000" );  // 일본 서버는 보안번호를 서버에서 셋팅해주는 방식으로 변경 [4/21/2010 Jo_Client]
					
					clientInfo->request.login = true;
					clientInfo->delay.login = GetTickCount( ) + 500;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, error );
				}
			}
			break;

		case NM_USER_SERVER_LIST_REQ:
			if ( packetLen == sizeof(MSGROOT) )
			{
				if ( clientInfo->complete.login && !clientInfo->request.serverList )
				{
					clientInfo->request.serverList = true;
					clientInfo->delay.serverList = GetTickCount( ) + 500;
				}
			}
			break;

		case NM_USER_COME_IN_GAME_REQ:
			if ( packetLen == sizeof(MSG_REQ_COMEIN_GAME) )
			{
				try
				{
					if ( clientInfo->complete.login && !clientInfo->request.gameToGame )
					{
						MSG_REQ_COMEIN_GAME* msg = (MSG_REQ_COMEIN_GAME*)msgRoot;
						long                 cid;
						long                 result;

						if ( clientInfo->last_server == msg->NumOfServer )
							cid = MAKECID( (WORD)msg->NumOfServer, (WORD)clientInfo->last_channel );
						else
							cid = MAKECID( (WORD)msg->NumOfServer, 0);

						ServerTable* serverTable = mServerManager->GetServerTable( cid );

						if ( serverTable == NULL )
							throw ERROR_COMEIN_GAME_FAIL;

						if ( !(clientInfo->member_age >= serverTable->age) )
							throw ERROR_COMEIN_GAME_AGE;

						result = mServerManager->FindChannel( cid );

						if ( result == 0 )
							throw ERROR_COMEIN_GAME_FAIL;

						clientInfo->last_server  = HIWORD( result );
						clientInfo->last_channel = LOWORD( result );

						clientInfo->request.gameToGame = true;
						clientInfo->delay.gameToGame = GetTickCount( ) + 500;							
					}
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_COME_IN_GAME_RES, error );
				}
			}
			break;
		default:
			return false;
		}
		break;
	case NM_SERVERTOOL:
		/*-- TOS_SERVERTOOL 메세지는 암호화 체크를 하지 않아 tos검사를 해준다.
		*/
		if( packet->tos != TOS_SERVERTOOL )
			return false;

		switch( msgRoot->Protocol )
		{
		case NM_SERVERTOOL_PING_REQ:
			if ( packetLen == sizeof(MSG_REQ_SRVTOOL_PING) )
			{
				MSG_REQ_SRVTOOL_PING* reqMsg = (MSG_REQ_SRVTOOL_PING*)msgRoot;

				HANDLE     handle  = NULL;
				MSG_RES_SRVTOOL_PING* sendMsg = (MSG_RES_SRVTOOL_PING*)GetMsgRoot( &handle, perSocketContext );

				if ( sendMsg != NULL )
				{
					sendMsg->Category       = NM_SERVERTOOL;
					sendMsg->Protocol       = NM_SERVERTOOL_PING_RES;
					sendMsg->mToolSendTime  = reqMsg->mToolSendTime;
					sendMsg->mSecurityLevel = mSecurityLevel;
					return SendMsgRoot( handle, sizeof(MSG_RES_SRVTOOL_PING) );
				}
				return false;
			}
			break;
		case NM_SERVERTOOL_GAMEMASTER_ON_REQ:
			if ( packetLen == sizeof(MSGROOT) )
			{
				mSecurityLevel = SRVTOOL_SECURITY_LEVEL0;
			}
			break;
		case NM_SERVERTOOL_TESTER_ON_REQ:
			if ( packetLen == sizeof(MSGROOT) )
			{
				mSecurityLevel = SRVTOOL_SECURITY_LEVEL1;
			}
			break;
		case NM_SERVERTOOL_PATHFINDER_ON_REQ:
			if ( packetLen == sizeof(MSGROOT) )
			{
				mSecurityLevel = SRVTOOL_SECURITY_LEVEL2;
			}
			break;
		case NM_SERVERTOOL_USER_ON_REQ:
			if ( packetLen == sizeof(MSGROOT) )
			{
				mSecurityLevel = SRVTOOL_SECURITY_LEVEL3;
			}
			break;
		default:
			return false;
		}
		break;
	}

	return true;
}

// SendExec Method
bool cLoginProcess::SendExec(PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  sendBytes = 0;
	DWORD  flags     = 0;
	int    retcode;

	wsaBuf.len = perIoContext->offset;
	wsaBuf.buf = perIoContext->buffer;

	if( *(perIoContext->buffer + PHLen) != NM_SERVERTOOL )
	{
		BYTE key = (BYTE)(rand( ) & 0x0f);
		(*wsaBuf.buf) = ((*wsaBuf.buf) & 0xf0) | key;

		Encrypt( (BYTE*)wsaBuf.buf, RC5_32_KEY_TABLE[ ((*wsaBuf.buf) & 0x0f) ] );
	}

	retcode = WSASend( perIoContext->socket,
					   &wsaBuf,
					   1,
					   &sendBytes,
					   flags,
					   &(perIoContext->wsaOverlapped),
					   NULL );

	if ( (retcode == SOCKET_ERROR) && (WSAGetLastError() != WSA_IO_PENDING) )
	{
		mIoContextPool->ReleaseIoContext( perIoContext );
		return false;
	}
	return true;
}

// AcceptComplete Method
bool cLoginProcess::AcceptComplete(PerSocketContext* perSocketContext)
{
	cCSLock lock( &mCs );

	if ( g_verbose == true )
	{
		char* inetnum = inet_ntoa( perSocketContext->addr.sin_addr );

		printf( "cLoginProcess::AcceptComplete\n" );
		printf( "\t[SOCKET:%d] %s (IN) Was connected.\n", perSocketContext->socket, inetnum );
	}

	// 클라이언트 메모리(버퍼) 사용량 설정.
	perSocketContext->offset = (long)sizeof(ClientInfo);

	// 로그인서버 패킷정보.
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	Packet*       packet       = (Packet*)perIoContext->buffer;

	packet->ver  = PHVer;
	packet->hlen = PHLen;
	packet->tos  = TOS_LOGIN_ACCEPT;
	packet->tlen = packet->hlen;
	packet->seq  = (++perSocketContext->seq);

	perIoContext->offset = packet->tlen;
	return SendPost( perIoContext );
}

// RecvComplete Method
bool cLoginProcess::RecvComplete(PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &mCs );
	char*   recvBuf    = (perSocketContext->buffer + (perSocketContext->offset + perSocketContext->InternalHigh));
	DWORD   recvBufLen = (perSocketContext->length - (perSocketContext->offset + perSocketContext->InternalHigh));

	// 오프셋의 크기를 누적한다.
	perIoContext->offset += bytesTransferred;

	DWORD length = perIoContext->offset; // 수신된 누적버퍼의 크기
	DWORD offset = 0;                    // 수신된 누적버퍼의 오프셋

	while ( length >= PHLen )
	{
		Packet* packet    = (Packet*)(perIoContext->buffer + offset);
		DWORD   packetLen = packet->tlen;

		// 헤더길이가 정의된 길이와 다를 경우 오류 처리한다.
		if ( packet->hlen != PHLen )
		{
			Close( perSocketContext, perIoContext );
			return false;
		}

		// 데이터의 길이가 최소 단위보다 작을 경우 오류 처리해야 한다.
		if ( packetLen < PHLen )
		{
			Close( perSocketContext, perIoContext );
			return false;
		}

		// 데이터의 길이가 완전할때까지 이어 받기를 해야 한다.
		if ( packetLen > length )
			break;

		// Sequence Number & Acknowledgment Number 검사.
		if ( packet->seq != (++perSocketContext->ack) )
		{
			Close( perSocketContext, perIoContext );
			return false;
		}

		if ( g_packet == true )
		{
			// 패킷해더(Packet Header) 정보 출력 - 버전/헤더길이/서비스 종류/전체길이.
			printf( "LoginProcess::RecvComplete::VER:%02x/HLEN:%02x/TOS:%02x/TLEN:%04x/SEQ:%08x\n",
					packet->ver,
					packet->hlen,
					packet->tos,
					packet->tlen,
					packet->seq );
		}

		// Type of service를 검사한다.
		switch ( packet->tos )
		{
		case TOS_CLIENT:
			Decrypt( (BYTE*)packet, RC5_32_KEY_TABLE[ packet->ver ] );
		case TOS_SERVERTOOL:
			// Receive Data 복사.
			if ( packetLen <= recvBufLen )
			{
				memcpy( recvBuf, packet, packetLen );

				recvBuf                        += packetLen;
				recvBufLen                     -= packetLen;
				perSocketContext->InternalHigh += packetLen;
			}
			else
			{
				// Buffer Overflow (버퍼 오버플로).
				Close( perSocketContext, perIoContext );
				return false;
			}
			break;
		case TOS_TTL:
			perSocketContext->timeToLive = (GetTickCount( ) + MAX_TTL);     // 최대 03분
			break;
		default:
			// Error.
			Close( perSocketContext, perIoContext );
			return false;
		}

		// 처리후 Offset 및 Length를 다시 계산한다.
		length -= packetLen;
		offset += packetLen;
	}

	// 이어 받기를 위해 데이터를 보관한한다.
	if ( offset > 0 )
	{
		PerIoContext* recvIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_READ );

		if ( length > 0 )
		{
			memcpy( recvIoContext->buffer, (perIoContext->buffer + offset), length );
			recvIoContext->offset = length;
		}

		mIoContextPool->ReleaseIoContext( perIoContext );
		perIoContext = recvIoContext;
	}

	// 수신을 위해 I/O Context를 준비한다.
	if ( RecvPost( perIoContext ) == false )
	{
		Close( perSocketContext );
		return false;
	}
	return true;
}

// CallbackComplete Method - cCSLock lock( &mCs )는 개별 사용한다.
bool cLoginProcess::CallbackComplete(PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	switch ( perIoContext->iParam )
	{
	case SQL_SERVER_PROC_MATRIX_CREATE:
		if ( mRequest.serverMatrix )
		{
			MATRIX_CREATE* matrixCreate = (MATRIX_CREATE*)perIoContext->buffer;
			if ( g_verbose == true )
			{
				cCSLock lock( &mCs );
				printf( "SQL_SERVER_PROC_MATRIX_CREATE - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
			}
			mRequest.serverMatrix = false;
			mComplete.serverMatrix = (perIoContext->requestResult == 0) ? true : false;
		}
		break;
	case SQL_SERVER_PROC_SERVER_LIST:
		if ( mRequest.serverTable )
		{
			cCSLock      lock( &mCs );
			SERVER_LIST* serverList = (SERVER_LIST*)perIoContext->buffer;

			if ( g_verbose == true )
			{
				char serverName[51];

				printf( "SQL_SERVER_PROC_SERVER_LIST - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
				printf( "\tRow Count:...%d\n", serverList->rowCount );

				TB_SERVER* table = serverList->table;
				for ( int i = 0; i < serverList->rowCount; i++, table++ )
				{
					WideCharToMultiByte( CP_ACP,
										 0,
										 table->name,
										 -1,
										 serverName,
										 sizeof(serverName)/sizeof(char),
										 NULL,
										 NULL );

					printf( "\t\tIDX:.........%d\n", table->idx         );
					printf( "\t\tNAME:..     .%s\n", serverName         );
					printf( "\t\tDESCRIPTION:.%s\n", table->description );
					printf( "\t\tSTATUS:......%d\n", table->status      );
					printf( "\t\tAGE:.........%d\n", table->age         );
				}
			}

			if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
			{
				TB_SERVER* table = serverList->table;
				for ( int i = 0; i < serverList->rowCount; i++, table++ )
				{
					long         cid         = MAKECID( (WORD)table->idx, 0 );
					ServerTable* serverTable = mServerManager->GetServerTable( cid );
					if ( serverTable != NULL )
					{
						serverTable->idx    = table->idx;
						wcscpy( serverTable->name, table->name );
						serverTable->status = table->status;
						serverTable->age    = table->age;
					}
				}
			}

			mRequest.serverTable  = false;
			mComplete.serverTable = (perIoContext->requestResult == 0) ? true : false;
		}
		break;
	case SQL_SERVER_PROC_SERVER_DOWN:
		{
			SERVER_DOWN* serverDown = (SERVER_DOWN*)perIoContext->buffer;
			if ( g_verbose == true )
			{
				printf( "SQL_SERVER_PROC_SERVER_DOWN - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
				printf( "\tServer Number:....%d\n", serverDown->serverNum );
				printf( "\tChannel Number:...%d\n", serverDown->channelNum );
			}
		}
		break;
	case SQL_SERVER_PROC_CHANNEL_UPDATE:
		{
			CHANNEL_UPDATE* channelUpdate = (CHANNEL_UPDATE*)perIoContext->buffer;
			if ( g_verbose == true )
			{
				cCSLock lock( &mCs );
				printf( "SQL_SERVER_PROC_CHANNEL_UPDATE - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
				printf( "\tServer Num:.....%d\n", channelUpdate->serverNum );
				printf( "\tChannel Num:....%d\n", channelUpdate->channelNum );
				printf( "\tStatus:.........%d\n", channelUpdate->status );
				printf( "\tRetvalue:.......%d\n", channelUpdate->retvalue );
			}
		}
		break;
	case SQL_SERVER_PROC_CONCURRENT_USER:
		{
			cCSLock          lock( &mCs );
			CONCURRENT_USER* concurrentUser = (CONCURRENT_USER*)perIoContext->buffer;
			ServerTable*     serverInfo     = mServerManager->GetServerTable( MAKECID( (WORD)concurrentUser->serverIdx, 0 ) );

			if ( serverInfo != NULL )
			{
				serverInfo->concurrentUser = concurrentUser->value;
			}

			if ( g_verbose == true )
			{
				printf( "SQL_SERVER_PROC_CONCURRENT_USER - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
				printf( "\tServer Num:.....%d\n", concurrentUser->serverIdx );
				printf( "\tValue:..........%d\n", concurrentUser->value     );
			}
		}
		break;
	case SQL_WEB_PROC_LAUNCHER:
		if ( perSocketContext->socket == perIoContext->socket )
		{
			cCSLock       lock( &mCs );
			ClientInfo*   clientInfo = (ClientInfo*)perSocketContext->buffer;
			WEB_LAUNCHER* launcher   = (WEB_LAUNCHER*)perIoContext->buffer;

			if ( clientInfo->db.login )
			{
				clientInfo->db.login = false;	// DB; 사용종료.

				if ( strlen( launcher->loginId ) && strlen( launcher->loginPwd ) )
				{
					clientInfo->member_idx = launcher->propid;
					strcpy( clientInfo->member_uid, launcher->loginId );
					strcpy( clientInfo->member_pwd, launcher->loginPwd );
					SendMsgError( perSocketContext, NM_USER, NM_USER_WEB_LAUNCHER_RES, ERROR_WEB_LAUNCHER_SUCCESS );
				}
				else
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_WEB_LAUNCHER_RES, ERROR_WEB_LAUNCHER_FAIL );
				}

				if ( g_verbose == true )
				{
					printf( "SQL_WEB_PROC_LAUNCHER  - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
					printf( "\tpropid:           %d\n", launcher->propid   );
					printf( "\tLogin_ID:         %s\n", launcher->loginId  );
					printf( "\tLogin_PWD:        %s\n", launcher->loginPwd );
				}
			}
		}
		break;
	case SQL_WEB_PROC_CHECK:
	case SQL_WEB_PROC_LOGIN:
		if ( perSocketContext->socket == perIoContext->socket )
		{
			cCSLock     lock( &mCs );
			ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
			WEB_LOGIN*  login      = (WEB_LOGIN*)perIoContext->buffer;

			if ( clientInfo->db.login )
			{
				clientInfo->db.login = false;	// DB; 사용종료.

				// 웹 로그인 사용.
				if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS && login->retvalue == 0 )
				//if ( login->retvalue == 0 )
				{
					bool success = true;

					switch ( mSecurityLevel )
					{
					case SRVTOOL_SECURITY_LEVEL1:	// 테스터-TESTER
						if ( !(login->slevel == WEB_LOGIN_SLEVEL1 || login->slevel == WEB_LOGIN_SLEVEL5) )
							success = false;
						break;
					case SRVTOOL_SECURITY_LEVEL2:	// 개척자-PATH FINDER
						if ( !(login->slevel == WEB_LOGIN_SLEVEL1 || login->slevel == WEB_LOGIN_SLEVEL5 || login->slevel == WEB_LOGIN_SLEVEL4) )
							success = false;
						break;
					}
					if ( success == true )
					{
						clientInfo->member_idx    = login->idx;
						clientInfo->member_slevel = login->slevel;
						clientInfo->member_age    = (BYTE)login->retage;
						clientInfo->member_pc     = login->retpc;

						clientInfo->db.login = MemberLogin( perSocketContext, 
															clientInfo->member_idx, 
															clientInfo->member_slevel, 
															clientInfo->member_pc );

						if ( g_verbose == true )
						{
							printf( "SQL_WEB_PROC_LOGIN  - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
							printf( "\t INETNUM :          %s\n", login->ip			);
							printf( "\tERROR CODE:         %d\n", login->retvalue   );
						}

					}
					else
					{
						SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, ERROR_DIST_USERINFO_SLEVEL );
					}
				}
				else if ( clientInfo->error_count < MAX_ERROR_COUNT_LOGIN )
				{
					++clientInfo->error_count;
					SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, login->retvalue );
				}
				else
				{
					PostMemberEvent( EVENT_MEMBER_FAILURE_AUDIT
									,EVENT_MEMBER_LOGIN
									,login->idx
									,"Inetnum(=%s) - ErrorCode(=%d)"
									,inet_ntoa( perSocketContext->addr.sin_addr )
									,login->retvalue );

					Close( perSocketContext );
				}

				if ( g_verbose == true )
				{
					printf( "SQL_WEB_PROC_LOGIN  - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
					printf( "\tid_idx:           %d\n", login->idx        );
					printf( "\tid_loginid:       %s\n", login->loginid    );
					printf( "\tid_passwd:        %s\n", login->passwd     );
					printf( "\tid_passwd_sec:    %s\n", login->passwdsec  );
					printf( "\tERROR CODE:       %d\n", login->retvalue   );
				}
			}
		}
		break;
	case SQL_WEB_PROC_TP_SMS:
		{
			WEB_TP_SMS* sms = (WEB_TP_SMS*)perIoContext->buffer;

			if ( g_verbose == true )
			{
				printf( "SQL_WEB_PROC_TP_SMS  - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
				printf( "\tSERVER_NAME:      %s\n", sms->serverName );
			}
		}
		break;
	case SQL_CLIENT_PROC_MEMBER_LOGIN:
		if ( perSocketContext->socket == perIoContext->socket )
		{
			cCSLock       lock( &mCs );
			ClientInfo*   clientInfo = (ClientInfo*)perSocketContext->buffer;
			MEMBER_LOGIN* login      = (MEMBER_LOGIN*)perIoContext->buffer;

			if ( clientInfo->db.login )
			{
				clientInfo->db.login = false;	// DB; 사용종료.

				if ( (perIoContext->requestResult == 0) && (login->retvalue == 0) )
				{
					// 로그인성공.
					clientInfo->complete.login = true;	// 로그인 완료.
					clientInfo->delay.loginUpdate = GetTickCount( ) + 90000;

					clientInfo->login_idx    = login->loginIdx;
					clientInfo->last_server  = login->lastServerNum;
					clientInfo->last_channel = login->lastChannelNum;

					// Member Event.
					PostMemberEvent( EVENT_MEMBER_SUCCESS_AUDIT
									,EVENT_MEMBER_LOGIN
									,login->memberIdx
									,"Inetnum(=%s) - ErrorCode(=%d)"
									,login->inetnum
									,login->retvalue );
					LoginSuccess( perSocketContext, login->retvalue, login->memberIdx, login->loginIdx, login->twitterUid, login->twitterPwd );
				}
				else if ( perIoContext->requestResult != 0 )
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, ERROR_DIST_USERINFO_FAIL );
					PostServerEvent( "ERROR SQL_CLIENT_PROC_MEMBER_LOGIN" );
				}
				else
				{
					// Member Event.
					PostMemberEvent( EVENT_MEMBER_FAILURE_AUDIT
									,EVENT_MEMBER_LOGIN
									,login->memberIdx
									,"Inetnum(=%s) - ErrorCode(=%d)"
									,login->inetnum
									,login->retvalue );
					SendMsgError( perSocketContext, NM_USER, NM_USER_LOGIN_RES, login->retvalue );
				}

				if ( g_verbose == true )
				{
					printf( "SQL_CLIENT_PROC_MEMBER_LOGIN  - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
					printf( "\tMEMBER_IDX:       %d\n", login->memberIdx      );
					printf( "\tLOGIN_IDX:        %d\n", login->loginIdx       );
					printf( "\tINETNUM:          %s\n", login->inetnum        );
					printf( "\tLAST SERVER NUM:  %d\n", login->lastServerNum  );
					printf( "\tLAST CHANNEL NUM: %d\n", login->lastChannelNum );
					printf( "\tTWITTER UID:      %s\n", login->twitterUid     );
					printf( "\tTWITTER PWD:      %s\n", login->twitterPwd     );
					printf( "\tERROR CODE:       %d\n", login->retvalue       );
				}
			}
		}
		break;
	case SQL_CLIENT_PROC_MEMBER_LOGOUT:
		if ( perSocketContext->socket == perIoContext->socket )
		{
			cCSLock        lock( &mCs );
			ClientInfo*    clientInfo = (ClientInfo*)perSocketContext->buffer;
			MEMBER_LOGOUT* logout     = (MEMBER_LOGOUT*)perIoContext->buffer;

			if ( clientInfo->db.logout )
			{
				clientInfo->db.logout = false;		// DB; 사용종료.
				clientInfo->complete.login = false;	// 로그아웃 완료.

				PostMemberEvent( EVENT_MEMBER_SUCCESS_AUDIT
								,EVENT_MEMBER_LOGOUT
								,logout->memberIdx
								,"Inetnum(=%s) - ErrorCode(=%d)"
								,inet_ntoa( perSocketContext->addr.sin_addr )
								,logout->retvalue );

				if ( g_verbose == true )
				{
					printf( "SQL_CLIENT_PROC_MEMBER_LOGOUT - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
					printf( "\tLOGIN_IDX:  %d\n", logout->loginIdx  );
					printf( "\tMEMBER_IDX: %d\n", logout->memberIdx );
					printf( "\tERROR CODE: %d\n", logout->retvalue  );
				}
			}
		}
		break;
	case SQL_CLIENT_PROC_LOGIN_UPDATE:
		break;
	case SQL_CLIENT_PROC_GAME_TO_GAME:
		if ( perSocketContext->socket == perIoContext->socket )
		{
			cCSLock       lock( &mCs );
			ClientInfo*   clientInfo = (ClientInfo*)perSocketContext->buffer;
			GAME_TO_GAME* gameToGame = (GAME_TO_GAME*)perIoContext->buffer;

			if ( clientInfo->db.gameToGame )
			{
				clientInfo->db.gameToGame = false;	// DB; 사용종료.
				SendMsgError( perSocketContext, NM_USER, NM_USER_COME_IN_GAME_RES, gameToGame->retvalue );

				if ( gameToGame->retvalue == 0 )
				{
					// 게임서버 이동이므로, 로그아웃 처리를 무시.
					clientInfo->complete.login      = false;	// 로그아웃 무시.
					clientInfo->complete.gameToGame = true;		// 서버이동 완료.
					LoginToGame( perSocketContext, gameToGame->ipv4, gameToGame->port );
				}

				if ( g_verbose == true )
				{
					in_addr addr;
					addr.S_un.S_addr = gameToGame->ipv4;
					printf( "SQL_CLIENT_PROC_COMEIN_GAME  - %s\n", (perIoContext->requestResult == 0) ? "SUCCESS" : "FAILED" );
					printf( "\tLOGIN IDX:   %d\n", gameToGame->loginIdx   );
					printf( "\tSERVER NUM:  %d\n", gameToGame->serverNum  );
					printf( "\tCHANNEL NUM: %d\n", gameToGame->channelNum );
					printf( "\tINETNUM:     %d.%d.%d.%d:%d\n"
						   ,addr.S_un.S_un_b.s_b1
						   ,addr.S_un.S_un_b.s_b2
						   ,addr.S_un.S_un_b.s_b3
						   ,addr.S_un.S_un_b.s_b4
						   ,gameToGame->port );
					printf( "\tERROR CODE:  %d\n", gameToGame->retvalue   );
				}
			}
		}
		break;
	}

	// 사용이 완료된 I/O Context는 회수한다.
	mIoContextPool->ReleaseIoContext( perIoContext );
	return true;
}

// BackendThreadBegin Method
void cLoginProcess::BackendThreadBegin(void)
{
	DWORD currTickCount = 0;
	while ( true )
	{
		currTickCount = GetTickCount( );
		{
			cCSLock lock( &mCs );

			// 클라이언트 종료, Thread를 나간다.
			if ( mEndServer == true )
				break; // while ( true )

			if ( mComplete.startLog == false )
			{
				mComplete.startLog = PostServerEvent( "To start from the cLoginProcess '%s:%d'.", inet_ntoa( mAddr.sin_addr ), mPort );
			}
			else if ( mComplete.serverMatrix == false )
			{
				if ( mRequest.serverMatrix == false )
				{
					mRequest.serverMatrix = MatrixCreate( MAX_SERVER, MAX_CHANNEL );
				}
			}
			else if ( mComplete.serverTable == false )
			{
				if ( mRequest.serverTable == false )
				{
					mRequest.serverTable = ServerList( );
				}
			}
			else if ( mComplete.serverSync == false )
			{
				mComplete.serverSync = g_loginSrv->GetSender( )->PostChSync( );
			}
			else
				break; // while ( true )

		}

		// I/O Context 처리.
		IoContextPresent( );

		// FPS 처리.
		BackendThreadFps( currTickCount );
	} // while ( true )
}

// BackendThreadEnd Method
void cLoginProcess::BackendThreadEnd(void)
{
	PostServerEvent( "Begin - cLoginProcess::BackendThreadEnd Close And Update" );
	Sleep( 100 );

	// 1. 업데이트.
	if ( mSocketContextPool != NULL )
	{
		PerSocketContext* socketContext = NULL;
		PerSocketContext* next;
		ClientInfo*       clientInfo;
		DWORD             currTickCount;

		// 1-1. 통신차단
		CSBlock( &mCs )
		{
			socketContext = mSocketContextPool->GetPagedPoolUsage( );

			while ( socketContext != NULL )
			{
				Close( socketContext );
				socketContext = socketContext->next;
			}
		}

		// 1-2. 안전종료.
		do {
			currTickCount = GetTickCount( );
			CSBlock( &mCs )
			{
				socketContext = mSocketContextPool->GetPagedPoolUsage( );
				while ( socketContext != NULL )
				{
					next       = socketContext->next;
					clientInfo = (ClientInfo*)socketContext->buffer;

					// DB 사용중.
					if ( clientInfo->dbData != 0 )
						;
					// 로그아웃 처리.
					else if ( clientInfo->complete.login )
						clientInfo->db.logout = MemberLogout( socketContext, 
															  clientInfo->login_idx, 
															  clientInfo->member_idx );
					// 종료
					else
						socketContext->status.closeSocket = 1;

					if ( socketContext->status.closeSocket )
						mSocketContextPool->ReleasePerSocketContext( socketContext );

					socketContext = next;
				}
			}
			BackendThreadFps( currTickCount );
		} while ( mSocketContextPool->GetPagedPoolUsage( ) );
	}

	PostServerEvent( "End - cLoginProcess::BackendThreadEnd Close And Update" );
	Sleep( 100 );

	// 2. 서버종료.
	PostServerEvent( "cLoginProcess('%s:%d') - Have received a request from the manager to stop the game server is shutting down."
					 ,inet_ntoa( mAddr.sin_addr )
					 ,mPort );
	Sleep( 1000 );
}

// BackendThreadProc Method
void cLoginProcess::BackendThreadProc(void)
{
	static DWORD lastTickCount = 0;
	DWORD        currTickCount = 0;
	while ( true )
	{
		currTickCount = GetTickCount( );

		// 클라이언트 종료, Thread를 나간다.
		if ( mEndServer == true )
			break; // while ( true )

		// 클라이언트의 소켓 관리.
		if ( mSocketContextPool != NULL )
		{
			cCSLock           lock( &mCs );
			PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
			PerSocketContext* next          = NULL;

			ClientInfo*       clientInfo    = NULL;
			char*             recvBuf       = NULL;

			while ( socketContext != NULL )
			{
				next       = socketContext->next;
				clientInfo = (ClientInfo*)socketContext->buffer;
				recvBuf    = (socketContext->buffer + socketContext->offset);

				// 계정관리.
				if ( clientInfo->complete.login && clientInfo->delay.loginUpdate < currTickCount )
				{
					LoginUpdate( socketContext, clientInfo->login_idx );
					clientInfo->delay.loginUpdate = currTickCount + 90000;
				}

				// Receive 데이터 처리를 위한 코드.
				if ( socketContext->InternalHigh )
				{
					u_long& offset = socketContext->Internal;
					u_long& length = socketContext->InternalHigh;

					if ( socketContext->status.connectionDead == false )
					{
						do {
							Packet* packet = (Packet*)(recvBuf + offset);

							if ( BatchComplete( socketContext, packet ) == false )
							{
								Close( socketContext );
								break;
							}

							offset += packet->tlen;
						} while ( offset < length );
					}

					memset( recvBuf, 0, socketContext->InternalHigh );
					socketContext->Internal     = 0;
					socketContext->InternalHigh = 0;
				}

				// Close Socket On.
				if ( socketContext->status.connectionDead == false )
				{
					// Time To Live.
					if ( socketContext->timeToLive > currTickCount )
					{
						//
						if ( clientInfo->request.launcher && clientInfo->delay.launcher < currTickCount )
						{
							// HACK - CLIENT - 웹 런쳐 사용.
							clientInfo->db.login = WebLauncher( socketContext, clientInfo->login_idx );
							clientInfo->request.launcher = !clientInfo->db.login;
						}
						if ( clientInfo->request.check && clientInfo->delay.check < currTickCount )
						{
							clientInfo->db.login = WebCheck( socketContext
															,clientInfo->member_uid
															,clientInfo->member_pwd
															,clientInfo->member_sec );
							clientInfo->request.check = !clientInfo->db.login;
						}
						// 로그인인증 요청 처리.
						if ( clientInfo->request.login && clientInfo->delay.login < currTickCount )
						{
							int  uidLen = (int)strlen( clientInfo->member_uid );
							int  pwdLen = (int)strlen( clientInfo->member_pwd );
							bool result = true;

							// SQL Injection를 방지 하기 위해 특수문자나 예외문자를 반드시 제어.
							for ( int i = 0; i < uidLen && result == true; i++ )
							{
								result  = (isalnum( clientInfo->member_uid[ i ] ) ? true : false);
							}

							// 결과확인
							if ( result == true )
							{
								char* inetnum = inet_ntoa( socketContext->addr.sin_addr );

								// HACK - CLIENT - 웹 로그인 사용.
								clientInfo->db.login = WebLogin( socketContext
																,clientInfo->member_uid
																,clientInfo->member_pwd
																,clientInfo->member_sec
																,inetnum );
								clientInfo->request.login = !clientInfo->db.login;
							}
							else
							{
								clientInfo->request.login = false;
								// 특수문자 및 예외문자로 인하여 인증실패.
								SendMsgError( socketContext, NM_USER, NM_USER_LOGIN_RES, ERROR_DIST_USERINFO_FAIL );
							}
						}
						// 서버리스트 요청 처리.
						if ( clientInfo->request.serverList && clientInfo->delay.serverList < currTickCount )
						{
							ServerTable* serverTable = mServerManager->GetServerTable( );
							ServerList( socketContext, serverTable, NM_USER, NM_USER_SERVER_LIST_RES );
							clientInfo->request.serverList = false;
						}
						// 게임서버 접속요청 처리.
						if ( clientInfo->request.gameToGame && clientInfo->delay.gameToGame < currTickCount )
						{
							// 채널 자동검색.
							long cid    = MAKECID( (WORD)clientInfo->last_server, (WORD)clientInfo->last_channel );
							long result = mServerManager->FindChannel( cid );
							if ( result > 0 )
							{
								clientInfo->last_server  = HIWORD( result );
								clientInfo->last_channel = LOWORD( result );
								clientInfo->db.gameToGame = GameToGame( socketContext,
																		clientInfo->login_idx,
																		(short)clientInfo->last_server,
																		(short)clientInfo->last_channel,
																		0 );
							}
							else
							{
								SendMsgError( socketContext, NM_USER, NM_USER_COME_IN_GAME_RES, ERROR_COMEIN_GAME_FAIL );
							}

							clientInfo->request.gameToGame = false;
						}
					}
					else
						Close( socketContext );
				}
				/*-- DB 사용중     --*/
				else if ( clientInfo->dbData != 0 )
					;
				/*-- 로그아웃 처리 --*/
				else if ( clientInfo->complete.login )
				{
					clientInfo->db.logout = (MemberLogout( socketContext, 
														   clientInfo->login_idx, 
														   clientInfo->member_idx ) == TRUE);
				}
				/*-- 종료          --*/
				else
					socketContext->status.closeSocket = 1;

				// SocketContext 회수.
				if ( socketContext->status.closeSocket )
				{
					if ( g_verbose == true )
					{
						char* inetnum = inet_ntoa( socketContext->addr.sin_addr );

						printf( "cLoginProcess::BackendThread\n" );
						printf( "\t[SOCKET:%d] %s (IN) Connection has been terminated.\n", socketContext->socket, inetnum );
					}

					mSocketContextPool->ReleasePerSocketContext( socketContext );
				}

				socketContext = next;
			} // while ( socketContext != NULL )
		}

		// 동시접속자 처리.
		if ( mComplete.serverTable && mCCUTick <= currTickCount )
		{
			cCSLock      lock( &mCs );
			ServerTable* serverTable = mServerManager->GetServerTable( );

			for ( u_long i = 0; i < MAX_SERVER; i++, serverTable++ )
			{
				if ( serverTable->channelCounter > 0 )
				{
					serverTable->maxConcurrentUser = max( serverTable->maxConcurrentUser, serverTable->concurrentUser );
					serverTable->minConcurrentUser = min( serverTable->minConcurrentUser, serverTable->concurrentUser );

					PostConcurrentEvent( serverTable->idx,
										 serverTable->minConcurrentUser,
										 serverTable->maxConcurrentUser,
										 "User(=%d)/Channel(=%d)/Status(%d)",
										 serverTable->concurrentUser,
										 serverTable->channelCounter,
										 serverTable->status );

					serverTable->maxConcurrentUser = serverTable->concurrentUser;
					serverTable->minConcurrentUser = serverTable->concurrentUser;

					ConcurrentUser( serverTable->idx );
				}
			}

			mCCUTick = currTickCount + 600000; // 10분
		}

		// 서버관리(채널)
		if ( mComplete.serverSync )
		{
			cCSLock lock( &mCs );
			mServerManager->UpdateProcess( currTickCount );
		}

		// I/O Context 처리.
		IoContextPresent( );

		// FPS 처리.
		BackendThreadFps( currTickCount );
	} // while ( true )
}

// BackendThread Method
DWORD cLoginProcess::BackendThread( )
{
	// 1. 로그인 서버 시작중.
	try {
		BackendThreadBegin( );
	} catch ( char* str ) {
		PostServerEvent( "Caught 'BackendThreadBegin' exception type: '%s'. Throwing 'cLoginProcess::BackendThreadBegin' exception", str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In BackendThreadBegin Method. Throwing 'cLoginProcess::BackendThread' exception." );
		Sleep( 100 );
		throw;
	}

	// 2. 로그인 서버 처리중.
	try {
		BackendThreadProc( );
	} catch ( char* str ) {
		PostServerEvent( "Caught 'BackendThreadBegin' exception type: '%s'. Throwing 'cLoginProcess::BackendThreadProc' exception", str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In BackendThreadProc Method. Throwing 'cLoginProcess::BackendThread' exception." );
		Sleep( 100 );
		throw;
	}

	// 3. 로그인 서버 종료중.
	try {
		BackendThreadEnd( );
	} catch ( char* str ) {
		PostServerEvent( "Caught 'BackendThreadBegin' exception type: '%s'. Throwing 'cLoginProcess::BackendThreadEnd' exception", str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In BackendThreadEnd Method. Throwing 'cLoginProcess::BackendThread' exception." );
		Sleep( 100 );
		throw;
	}
	return 0;
}

// BackendThreadFps Method
void cLoginProcess::BackendThreadFps(DWORD currentTick)
{
	if ( mFPSTick < currentTick )
	{
		// Console 모드 사용시 FPS 출력.
		if ( g_fps == true )
		{
			float fps = (float)mFrame / (float)(currentTick - (mFPSTick - 1000)) * 1000;
			printf( "FPS:%6.2f / FRAME:%d\tClientProcess Thread\n", fps, mFrame );
		}

		// Frame 변수 초기화.
		mFPSTick = currentTick + 1000;
		mFrame   = 0;
	}
	else
		mFrame++;

	// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
	DWORD elapsedTick = GetTickCount( ) - currentTick;

	if ( elapsedTick < 16 )
	{
		Sleep( 16 - elapsedTick );
	}
}
