// Include
#include "gamesrv.h"
#include "packet.h"

// Local definitions
#pragma warning( disable: 4127 )

// Global data

// cRudpProcess Constructor
cRudpProcess::cRudpProcess(void) : mPacketPool(NULL), mLoginBuffer(NULL), mGameBuffer(NULL), mLogBuffer(NULL)
{}

// ~cRudpProcess Destructor
cRudpProcess::~cRudpProcess(void)
{}

// Initialize Method
bool cRudpProcess::Initialize(char* recv, char* send, unsigned short port, unsigned short numWorkerThreads)
{
	mPacketPool = new PacketPool( );

	PHOSTENT phe;

	// Login Server IPv4 주소를 만든다.
	ZeroMemory( (void*)&mLogin, sizeof(SOCKADDR_IN) );
	mLogin.sin_family = AF_INET;
	mLogin.sin_port   = htons( U_LOGIN_SPORT );
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

	// Game Server IPv4 주소를 만든다.
	ZeroMemory( (void*)&mGame, sizeof(SOCKADDR_IN) );
	mGame.sin_family = AF_INET;
	mGame.sin_port   = htons( U_GAME_SPORT );
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

	// LogDemon IPv4 주소를 만든다.
	ZeroMemory( (void*)&mLog, sizeof(SOCKADDR_IN) );
	mLog.sin_family = AF_INET;
	mLog.sin_port   = htons( U_LOG_SPORT );
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

	return IocpRudp::Initialize( recv, port, numWorkerThreads );
}

// Shutdown Method
void cRudpProcess::Shutdown(DWORD maxWait)
{
	Sleep( 50 );
	IocpRudp::Shutdown( maxWait );

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

	mPacketPool->Shutdown( );
}

// ReSendExec Method
bool cRudpProcess::ReSendExec(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &m_cs );
	PerIoContext* ioContext = NULL;
	long          mask      = (seq & SM_ClassD);
	bool          retvalue  = false;

	if ( htons( U_LOGIN_SPORT ) == port )
	{
		ioContext = mLoginBuffer->buffer[ mask ];
	}
	else if ( htons( U_GAME_SPORT ) == port )
	{
		ioContext = mGameBuffer->buffer[ mask ];
	}
	else if ( htons( U_LOG_SPORT ) == port )
	{
		ioContext = mLogBuffer->buffer[ mask ];
	}

	if ( ioContext != NULL )
	{
		RUDP* rudp = (RUDP*)ioContext->buffer;
		if ( rudp->seq == seq )
		{
			SOCKADDR_IN saddr;

			ZeroMemory( (void*)&saddr, sizeof(SOCKADDR_IN) );
			saddr.sin_family = AF_INET;
			saddr.sin_port   = port;
			saddr.sin_addr.s_addr = addr;

			PerIoContext* sendIoContext = m_ioContextPool->GetIoContext( m_send, IOCP_REQUEST_WRITE, saddr );

			memcpy( sendIoContext->buffer, ioContext->buffer, ioContext->offset );
			sendIoContext->offset = ioContext->offset;

			retvalue = SendExec( NULL, sendIoContext );
		}
	}

	return retvalue;
}

// SendReliable Method
bool cRudpProcess::SendReliable(u_long addr, u_short port, u_long seq)
{
	cCSLock       lock( &m_cs );
	SOCKADDR_IN   saddr;

	ZeroMemory( (void*)&saddr, sizeof(SOCKADDR_IN) );
	saddr.sin_family = AF_INET;
	saddr.sin_port   = port;
	saddr.sin_addr.s_addr = addr;

	PerIoContext* perIoContext = m_ioContextPool->GetIoContext( m_send, IOCP_REQUEST_WRITE, saddr );

	if ( perIoContext != NULL )
	{
		RUDP* rudp = (RUDP*)perIoContext->buffer;

		rudp->ver  = RUDP_PHVer;
		rudp->hlen = RUDP_PHLen;
		rudp->tos  = TOS_RUDP;
		rudp->tlen = rudp->hlen;
		rudp->seq  = seq;
		rudp->addr = m_recvAddr.sin_addr.s_addr;
		rudp->port = m_recvAddr.sin_port;

		perIoContext->offset = rudp->tlen;
		return SendExec( NULL, perIoContext );
	}
	return false;
}


// GetPacket Method
char* cRudpProcess::GetPacket(void** handle, char tos, SOCKADDR_IN addr, u_long& seq)
{
	cCSLock       lock( &m_cs );
	PerIoContext* perIoContext = m_ioContextPool->GetIoContext( m_send, IOCP_REQUEST_WRITE, addr );
	char*         retvalue     = NULL;

	if ( perIoContext != NULL )
	{
		RUDP* rudp = (RUDP*)perIoContext->buffer;

		rudp->ver  = RUDP_PHVer;
		rudp->hlen = RUDP_PHLen;
		rudp->tos  = tos;
		rudp->tlen = rudp->hlen;
		rudp->seq  = (++seq);
		rudp->addr = m_recvAddr.sin_addr.s_addr;
		rudp->port = m_recvAddr.sin_port;

		(*handle) = perIoContext;
		retvalue  = perIoContext->buffer + rudp->hlen;
	}

	return retvalue;
}

// SendPacket Method
bool cRudpProcess::SendPacket(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	RUDP*         rudp         = (RUDP*)perIoContext->buffer;

	rudp->tlen = rudp->hlen + (u_short)length;

	perIoContext->offset = rudp->tlen;
	return SendExec( NULL, perIoContext );
}

// PostServerEvent Method
bool cRudpProcess::PostServerEvent(LPCTSTR format, ...)
{
	bool retvalue = false;
	if ( g_gameSrv != NULL )
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
			retvalue = g_gameSrv->PostServerEvent( (char*)msgBuf );
			free( msgBuf );
		}
	}
	return retvalue;
}

// GetRegisterDate Method
void cRudpProcess::GetRegisterDate(TIMESTAMP_STRUCT* ts)
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
bool cRudpProcess::PostServerEvent(char* message)
{
	HANDLE        handle      = NULL;
	SERVER_EVENT* serverEvent = (SERVER_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );
	if ( serverEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
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
bool cRudpProcess::PostMemberEvent(char type, char category, long memberIdx, char* message)
{
	HANDLE        handle      = NULL;
	MEMBER_EVENT* memberEvent = (MEMBER_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );
	if ( memberEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(MEMBER_EVENT) - sizeof(memberEvent->message);

		memberEvent->protocol  = MB_MEMBER_EVENT;
		GetRegisterDate( &memberEvent->registerDate );
		memberEvent->type      = type;
		strcpy( memberEvent->source, g_serviceName );
		memberEvent->category  = category;
		memberEvent->memberIdx = memberIdx;
		strcpy( memberEvent->computer, computer );
		strcpy( memberEvent->message, message );

		length += strlen(memberEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostCharacterEvent Method
bool cRudpProcess::PostCharacterEvent(char category, long memberIdx, long serverIdx, long characterIdx, char _level, long _exp, char _skillLevel, long _skillExp, long _skillPoint, char* message)
{
	HANDLE           handle         = NULL;
	CHARACTER_EVENT* characterEvent = (CHARACTER_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( characterEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(CHARACTER_EVENT) - sizeof(characterEvent->message);

		characterEvent->protocol            = MB_CHARACTER_EVENT;
		GetRegisterDate( &characterEvent->registerDate );
		strcpy( characterEvent->source, g_serviceName );
		characterEvent->category            = category;
		characterEvent->memberIdx           = memberIdx;
		characterEvent->serverIdx           = serverIdx;
		characterEvent->characterIdx        = characterIdx;
		characterEvent->characterLevel      = _level;
		characterEvent->characterExp        = _exp;
		characterEvent->characterSkillLevel = _skillLevel;
		characterEvent->characterSkillExp   = _skillExp;
		characterEvent->characterSkillPoint = _skillPoint;
		strcpy( characterEvent->computer, computer );
		strcpy( characterEvent->message, message );

		length += strlen(characterEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostMoneyEvent Method
bool cRudpProcess::PostMoneyEvent(long serverIdx, long characterIdx, long before, long after, char* message)
{
	HANDLE       handle     = NULL;
	MONEY_EVENT* moneyEvent = (MONEY_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( moneyEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(MONEY_EVENT) - sizeof(moneyEvent->message);

		moneyEvent->protocol     = MB_MONEY_EVENT;
		GetRegisterDate( &moneyEvent->registerDate );
		moneyEvent->serverIdx    = serverIdx;
		moneyEvent->characterIdx = characterIdx;
		moneyEvent->before       = before;
		moneyEvent->after        = after;
		strcpy( moneyEvent->computer, computer );
		strcpy( moneyEvent->message, message );

		length += strlen(moneyEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostDepositEvent Method
bool cRudpProcess::PostDepositEvent(long serverIdx, long characterIdx, long before, long after, char* message)
{
	HANDLE         handle = NULL;
	DEPOSIT_EVENT* depositEvent = (DEPOSIT_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( depositEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(DEPOSIT_EVENT) - sizeof(depositEvent->message);

		depositEvent->protocol     = MB_DEPOSIT_EVENT;
		GetRegisterDate( &depositEvent->registerDate );
		depositEvent->serverIdx    = serverIdx;
		depositEvent->characterIdx = characterIdx;
		depositEvent->before       = before;
		depositEvent->after        = after;
		strcpy( depositEvent->computer, computer );
		strcpy( depositEvent->message, message );

		length += strlen(depositEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostInventoryEvent Method
bool cRudpProcess::PostInventoryEvent(long serverIdx, char category, long inventoryIdx, long characterIdx, long itemDefineIndex, short number, short count, BYTE enhanced, BYTE seal, char* message)
{
	HANDLE           handle = NULL;
	INVENTORY_EVENT* inventoryEvent = (INVENTORY_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( inventoryEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(INVENTORY_EVENT) - sizeof(inventoryEvent->message);

		inventoryEvent->protocol = MB_INVENTORY_EVENT;
		GetRegisterDate( &inventoryEvent->registerDate );
		inventoryEvent->serverIdx       = serverIdx;
		inventoryEvent->category        = category;
		inventoryEvent->inventoryIdx    = inventoryIdx;
		inventoryEvent->characterIdx    = characterIdx;
		inventoryEvent->itemDefineIndex = itemDefineIndex;
		inventoryEvent->number          = number;
		inventoryEvent->count           = count;
		inventoryEvent->enhanced        = enhanced;
		inventoryEvent->seal            = seal;
		strcpy( inventoryEvent->computer, computer );
		strcpy( inventoryEvent->message, message );

		length += strlen(inventoryEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostQuestEvent Method
bool cRudpProcess::PostQuestEvent(long serverIdx, char category, long characterIdx, long questIdx, char* message)
{
	HANDLE       handle     = NULL;
	QUEST_EVENT* questEvent = (QUEST_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( questEvent != NULL )
	{
		u_long length   = sizeof(QUEST_EVENT) - sizeof(questEvent->message);
		char*  computer = g_gameSrv->GetHostName();

		questEvent->protocol     = MB_QUEST_EVENT;
		GetRegisterDate( &questEvent->registerDate );
		questEvent->serverIdx    = serverIdx;
		questEvent->category     = category;
		questEvent->characterIdx = characterIdx;
		questEvent->questIdx     = questIdx;
		strcpy( questEvent->computer, computer );
		strcpy( questEvent->message, message );

		length += strlen(questEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostGuildEvent Method
bool cRudpProcess::PostGuildEvent(long serverIdx, long characterIdx, long guildIdx, char guildPosition, char* message)
{
	HANDLE       handle     = NULL;
	GUILD_EVENT* guildEvent = (GUILD_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( guildEvent != NULL )
	{
		u_long length  = sizeof(GUILD_EVENT) - sizeof(guildEvent->message);
		char* computer = g_gameSrv->GetHostName();

		guildEvent->protocol        = MB_GUILD_EVENT;
		GetRegisterDate( &guildEvent->registerDate );
		guildEvent->serverIdx       = serverIdx;
		guildEvent->characterIdx    = characterIdx;
		guildEvent->guildIdx        = guildIdx;
		guildEvent->guildPosition   = guildPosition;
		strcpy( guildEvent->computer, computer );
		strcpy( guildEvent->message, message );

		length += strlen(guildEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostDropItemEvent Method
bool cRudpProcess::PostDropItemEvent(long serverIdx, long channelNum, sDropItem* pDropItem, unsigned long count)
{
	HANDLE          handle     = NULL;
	DROPITEM_EVENT* pDropEvent = (DROPITEM_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( pDropEvent != NULL )
	{
		char* computer = g_gameSrv->GetHostName();

		pDropEvent->protocol    = MB_DROPITEM_EVENT;
		pDropEvent->mServerIdx  = serverIdx;
		pDropEvent->mChannelNum = channelNum;
		pDropEvent->mCount      = count;
		strcpy( pDropEvent->mComputer, computer );
		memcpy( pDropEvent->mDropItem, pDropItem, sizeof(sDropItem) * count );

		return SendPacket( handle, pDropEvent->GetMsgLength() );
	}
	return false;
}

// PostSkillEvent Method
bool cRudpProcess::PostSkillEvent(long serverIdx, long characterIdx, bool isReset, long skillIdx, char skillStep)
{
	HANDLE       handle      = NULL;
	SKILL_EVENT* pSkillEvent = (SKILL_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( pSkillEvent  != NULL )
	{
		char* computer = g_gameSrv->GetHostName();

		pSkillEvent->protocol      = MB_SKILL_EVENT;
		GetRegisterDate( &pSkillEvent->mRegisterDate );
		pSkillEvent->mServerIdx    = serverIdx;
		pSkillEvent->mCharacterIdx = characterIdx;
		pSkillEvent->mIsReset      = isReset;
		pSkillEvent->mSkillIdx     = skillIdx;
		pSkillEvent->mSkillStep    = skillStep;
		strcpy( pSkillEvent->mComputer, computer );

		return SendPacket( handle, sizeof(SKILL_EVENT) );
	}
	return false;
}

// PostChStatus Method
bool cRudpProcess::PostChStatus(long cid, BYTE status)
{
	HANDLE            handle      = NULL;
	MB_SYN_CH_STATUS* synChStatus = (MB_SYN_CH_STATUS*)GetPacket( &handle, TOS_GAME, mLogin, mLoginSeq );

	if ( synChStatus != NULL )
	{
		synChStatus->protocol = MB_CH_STATUS_SYN;
		synChStatus->cid      = cid;
		synChStatus->status   = status;

		return SendPacket( handle, sizeof(MB_SYN_CH_STATUS) );
	}
	return false;
}

// PostServerNotice Method
bool cRudpProcess::PostServerNotice(long cid, wchar_t* message, u_int length)
{
	HANDLE                handle       = NULL;
	MB_SYN_SERVER_NOTICE* serverNotice = (MB_SYN_SERVER_NOTICE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( serverNotice != NULL )
	{
		serverNotice->protocol = MB_SERVER_NOTICE_SYN;
		serverNotice->cid      = cid;
		memcpy( serverNotice->message, message, length );

		return SendPacket( handle, sizeof(MB_SYN_SERVER_NOTICE) );
	}
	return false;
}

// PostChatShout Method
bool cRudpProcess::PostChatShout(long cid, unsigned short numOfMap, long characterIdx, wchar_t* name, wchar_t* message, u_int length)
{
	HANDLE             handle    = NULL;
	MB_SYN_CHAT_SHOUT* chatShout = (MB_SYN_CHAT_SHOUT*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( chatShout != NULL )
	{
		chatShout->protocol     = MB_CHAT_SHOUT_SYN;
		chatShout->cid          = cid;
		chatShout->numOfMap     = numOfMap;
		chatShout->characterIdx = characterIdx;
		wcscpy( chatShout->name, name );
		memcpy( chatShout->message, message, length );

		return SendPacket( handle, sizeof(MB_SYN_CHAT_SHOUT) );
	}
	return false;
}

// PostPVPJoin Method
bool cRudpProcess::PostPVPStatus(long cid, BYTE status, unsigned char pvpType )
{
	HANDLE             handle  = NULL;
	MB_SYN_PVP_STATUS* postMsg = (MB_SYN_PVP_STATUS*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol = MB_PVP_STATUS_SYN;
		postMsg->cid      = cid;
		postMsg->status   = status;
		postMsg->pvpType  = pvpType;

		return SendPacket( handle, sizeof(MB_SYN_PVP_STATUS) );
	}
	return false;
}

// PostPVPJoin Method
bool cRudpProcess::PostPVPNotice( long cid, unsigned char leftMinute, unsigned char pvpType )
{
	HANDLE             handle  = NULL;
	MB_SYN_PVP_NOTICE* postMsg = (MB_SYN_PVP_NOTICE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol   = MB_PVP_NOTICE_SYN;
		postMsg->cid        = cid;
		postMsg->leftMinute = leftMinute;
		postMsg->pvpType    = pvpType;

		return SendPacket( handle, sizeof(MB_SYN_PVP_NOTICE) );
	}
	return false;
}

bool cRudpProcess::PostPVPPlayerList(long cid, char* msg, int len)
{
	HANDLE                 handle  = NULL;
	MB_SYN_PVP_PLAYERLIST* postMsg = (MB_SYN_PVP_PLAYERLIST*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		unsigned long length = sizeof(MB_SYN_THEME_INVITE) - sizeof(postMsg->msg);

			postMsg->protocol = MB_PVP_PLAYERLIST_SYN;
			postMsg->cid      = cid;
			memcpy( postMsg->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

// PostPVPJoin Method
bool cRudpProcess::PostThemeStatus(long cid, BYTE status)
{
	HANDLE             handle  = NULL;
	MB_SYN_THEME_STATUS* postMsg = (MB_SYN_THEME_STATUS*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol = MB_THEME_STATUS_SYN;
		postMsg->cid      = cid;
		postMsg->status   = status;

		return SendPacket( handle, sizeof(MB_SYN_THEME_STATUS) );
	}
	return false;
}

// PostThemeInvite Method
bool cRudpProcess::PostThemeInvite(long cid, short themeIdx, short roomIdx, long requestor, long characterIdx, char* msg, int len)
{
	HANDLE               handle  = NULL;
	MB_SYN_THEME_INVITE* postMsg = (MB_SYN_THEME_INVITE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		unsigned long length = sizeof(MB_SYN_THEME_INVITE)-sizeof(postMsg->msg);

			postMsg->protocol     = MB_THEME_INVITE_SYN;
			postMsg->cid          = cid;
			postMsg->themeIdx     = themeIdx;
			postMsg->roomIdx      = roomIdx;
			postMsg->requestor    = requestor;
			postMsg->characterIdx = characterIdx;
			memcpy( postMsg->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}


// PostThemeInviteRes Method
bool cRudpProcess::PostThemeInviteRes( long cid, long requestor, wchar_t* name, char errorCode )
{
	HANDLE                       handle  = NULL;
	MB_SYN_THEME_INVATE_RESPONE* postMsg = (MB_SYN_THEME_INVATE_RESPONE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol  = MB_THEME_INVITE_RESPONE_SYN;
		postMsg->cid       = cid;
		postMsg->requestor = requestor;
		wcscpy( postMsg->name, name );
		postMsg->errorCode = errorCode;

		return SendPacket( handle, sizeof(MB_SYN_THEME_INVATE_RESPONE) );
	}
	return false;
}


// PostGuildSync Method
bool cRudpProcess::PostGuildSync(long cid, char* msg, int len)
{
	HANDLE             handle    = NULL;
	MB_SYN_GUILD_SYNC* guildSync = (MB_SYN_GUILD_SYNC*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( guildSync != NULL )
	{
		unsigned long length = sizeof(MB_SYN_GUILD_SYNC)-sizeof(guildSync->msg);

			guildSync->protocol = MB_GUILD_SYNC_SYN;
			guildSync->cid      = cid;
			memcpy( guildSync->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

// PostGuildConnectSync
bool cRudpProcess::PostGuildConnectSync(long cid)
{
	HANDLE                    handle    = NULL;
	MB_SYN_GUILD_CONNECTSYNC* guildSync = (MB_SYN_GUILD_CONNECTSYNC*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( guildSync != NULL )
	{
		guildSync->protocol = MB_GUILD_CONNECTSYNC_SYN;
		guildSync->cid      = cid;

		return SendPacket( handle, sizeof(MB_SYN_GUILD_CONNECTSYNC) );
	}
	return false;
}

// PostMonsterSync Method
bool cRudpProcess::PostMonsterSync(long cid, char* msg, int len)
{
	HANDLE               handle = NULL;
	MB_SYN_MONSTER_SYNC* pSync  = (MB_SYN_MONSTER_SYNC*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( pSync != NULL )
	{
		unsigned long length = sizeof(MB_SYN_MONSTER_SYNC)-sizeof(pSync->msg);

			pSync->protocol = MB_MONSTER_SYN_SYN;
			pSync->cid      = cid;
			memcpy( pSync->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

// PostChatSync Method
bool cRudpProcess::PostChatSync(long cid, char* msg, int len)
{
	HANDLE            handle   = NULL;
	MB_SYN_CHAT_SYNC* chatSync = (MB_SYN_CHAT_SYNC*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( chatSync != NULL )
	{
		unsigned long length = sizeof(MB_SYN_CHAT_SYNC)-sizeof(chatSync->msg);

			chatSync->protocol = MB_CHAT_SYN;
			chatSync->cid      = cid;
			memcpy( chatSync->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

// PostPostReceived Method
bool cRudpProcess::PostPostReceived(long cid, long characterIdx)
{
	HANDLE                handle       = NULL;
	MB_SYN_POST_RECEIVED* postReceived = (MB_SYN_POST_RECEIVED*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postReceived != NULL )
	{
		postReceived->protocol     = MB_POST_RECEIVED_SYN;
		postReceived->cid          = cid;
		postReceived->characterIdx = characterIdx;

		return SendPacket( handle, sizeof(MB_SYN_POST_RECEIVED) );
	}
	return false;
}

// BatchComplete Method
bool cRudpProcess::BatchComplete(MSGBUF* msgBuf, int msgLen)
{
	cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );

	if ( gameProcess == NULL )
		return false;
	else if ( gameProcess->IsChannelCheck( ) == false )
		return false;

	switch ( msgBuf->protocol )
	{
	case MB_CHAT_SHOUT_SYN:
		{
			MB_SYN_CHAT_SHOUT* recvMsg = (MB_SYN_CHAT_SHOUT*)msgBuf;
			gameProcess->ChatShout( recvMsg->cid, recvMsg->numOfMap, recvMsg->characterIdx, recvMsg->name, recvMsg->message );
		}
		break;
	case MB_CHAT_SYN:
		{
			MB_SYN_CHAT_SYNC* recvMsg = (MB_SYN_CHAT_SYNC*)msgBuf;
			int				  len	  = msgLen - (sizeof(MB_CHAT_SYN) - sizeof(recvMsg->msg) );
			gameProcess->ChatSync( recvMsg->cid, recvMsg->msg, len );
		}
		break;
	case MB_SERVER_NOTICE_SYN:
		{
			MB_SYN_SERVER_NOTICE* recvMsg = (MB_SYN_SERVER_NOTICE*)msgBuf;
			gameProcess->ServerNotice( recvMsg->cid, recvMsg->message );
		}
		break;
	case MB_GUILD_SYNC_SYN:
		{
			MB_SYN_GUILD_SYNC* recvMsg = (MB_SYN_GUILD_SYNC*)msgBuf;
			int                len     = msgLen - (sizeof(MB_SYN_GUILD_SYNC) - sizeof(recvMsg->msg));
			gameProcess->GuildSync( recvMsg->cid, recvMsg->msg, len );
		}
		break;
	case MB_GUILD_CONNECTSYNC_SYN:
		{
			MB_SYN_GUILD_CONNECTSYNC* recvMsg = (MB_SYN_GUILD_CONNECTSYNC*)msgBuf;
			gameProcess->GuildConnectSync( recvMsg->cid );
		}
		break;
	case MB_PVP_STATUS_SYN:
		{
			MB_SYN_PVP_STATUS* recvMsg = (MB_SYN_PVP_STATUS*)msgBuf;
			gameProcess->PVPStatus( recvMsg->cid, recvMsg->status, recvMsg->pvpType );
		}
		break;
	case MB_PVP_NOTICE_SYN:
		{
			MB_SYN_PVP_NOTICE* recvMsg = (MB_SYN_PVP_NOTICE*)msgBuf;
			gameProcess->PVPNotice( recvMsg->cid, recvMsg->leftMinute, recvMsg->pvpType );
		}
		break;
	case MB_PVP_PLAYERLIST_SYN:
		{
			MB_SYN_PVP_PLAYERLIST* recvMsg = (MB_SYN_PVP_PLAYERLIST*)msgBuf;
			gameProcess->PVPPlayerList( recvMsg );
		}
		break;
	case MB_THEME_STATUS_SYN:
		{
			MB_SYN_PVP_STATUS* recvMsg = (MB_SYN_PVP_STATUS*)msgBuf;
			gameProcess->ThemeStatus( recvMsg->cid, recvMsg->status );
		}
		break;
	case MB_THEME_INVITE_SYN:
		gameProcess->ThemeInvite( (MB_SYN_THEME_INVITE*)msgBuf );
		break;
	case MB_THEME_INVITE_RESPONE_SYN:
		gameProcess->ThemeInviteRespone( (MB_SYN_THEME_INVATE_RESPONE*)msgBuf );
		break;
	case MB_CH_STATUS_SYN:
		{
			MB_SYN_CH_STATUS* recvMsg = (MB_SYN_CH_STATUS*)msgBuf;
			gameProcess->ChannelStatus( recvMsg->cid, recvMsg->status );
		}
		break;
	case MB_CH_LIST_SYN:
		{
			MB_SYN_CH_LIST* recvMsg = (MB_SYN_CH_LIST*)msgBuf;
			for ( BYTE i = 0; i < recvMsg->rowCount; i++ )
				gameProcess->ChannelStatus( recvMsg->channels[ i ].cid, recvMsg->channels[ i ].status );
		}
		break;
	case MB_MONSTER_SYN_SYN:
		{
			MB_SYN_MONSTER_SYNC* pRecMsg = (MB_SYN_MONSTER_SYNC*)msgBuf;
			int len = msgLen - (sizeof(MB_SYN_MONSTER_SYNC) - sizeof(pRecMsg->msg));
			gameProcess->MonsterSync( pRecMsg->cid, pRecMsg->msg, len );
		}
		break;
	case MB_CH_SYNC_SYN:
		gameProcess->ChannelSync( );
		break;
	case MB_POST_RECEIVED_SYN:
		gameProcess->PostReceived( (MB_SYN_POST_RECEIVED*)msgBuf );
		break;
	default:
		return false;
	}
	return true;
}

// RecvComplete Method
//
// UDP:.......User Datagram Protocol.
// Datagram:..패킷 교환에서, 데이터 단말 장치와 망과의 사전 접속 절차에 의하지 않고,
//            하나하나의 패킷이 발신 데이터 단말 장치와 수신처 데이터 단말 장치 간의
//            경로 지정을 위한 충분한 정보를 가지고 있는 패킷.
bool cRudpProcess::RecvComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &m_cs );

	// 수신된 데이터 처리.
	perIoContext->offset = bytesTransferred;

	// 더블버퍼링 처리.
	PerIoContext** buffer = m_ioContextBackBuffer->buffer;
	long&          offset = m_ioContextBackBuffer->offset;

	if ( offset < MAX_IO_CONTEXT_BUFFER_LEN )
	{
		buffer[ offset ] = perIoContext;
		offset++;
	}
	else
	{
		m_ioContextPool->ReleaseIoContext( perIoContext );
		PostServerEvent( "Warning - cRudpProcess::RecvComplete Method - Overflow I/O Context Buffer." );
	}

	// 수신을 위해 I/O Context를 준비한다.
	perIoContext = m_ioContextPool->GetIoContext( m_recv, IOCP_REQUEST_READ );
	if ( RecvPost( completionKey, perIoContext ) == false )
	{
		PostServerEvent( "Error - cRudpProcess::RecvPost Method - Return value is false." );
		return false;
	}
	return true;
}

// SendComplete Method
bool cRudpProcess::SendComplete(ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &m_cs );
	RUDP*   rudp = (RUDP*)perIoContext->buffer;
	long    mask = (rudp->seq & SM_ClassD);

	if ( rudp->tos != TOS_RUDP )
	{
		if ( mLoginBuffer != NULL && perIoContext->addr.sin_port == mLogin.sin_port )
		{
			PerIoContext** temp     = &mLoginBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = IocpRudp::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mLoginBuffer->offset = max( mLoginBuffer->offset, mask );
			return retvalue;
		}
		else if ( mGameBuffer != NULL && perIoContext->addr.sin_port == mGame.sin_port )
		{
			PerIoContext** temp     = &mGameBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = IocpRudp::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mGameBuffer->offset = max( mGameBuffer->offset, mask );
			return retvalue;
		}
		else if ( mLogBuffer != NULL && perIoContext->addr.sin_port == mLog.sin_port )
		{
			PerIoContext** temp     = &mLogBuffer->buffer[ mask ];
			bool           retvalue = true;

			if ( (*temp) != NULL )
			{
				retvalue = IocpRudp::SendComplete( completionKey, (*temp), (*temp)->offset );
			}
			(*temp) = perIoContext;

			mLogBuffer->offset = max( mLogBuffer->offset, mask );
			return retvalue;
		}
	}
	return IocpRudp::SendComplete( completionKey, perIoContext, bytesTransferred );
}

// BackendThread Method
DWORD cRudpProcess::BackendThread( )
{
	DWORD currentTick;
	DWORD elapsedTick;

	DWORD relayTick = 0;

	try {
		while ( true )
		{
			currentTick = GetTickCount( );

			// 서버종료, Thread를 나간다.
			if ( m_endServer == true )
				break;

			// Double Buffering 처리.
			EnterCriticalSection( &m_cs );

				IoContextBuffer* temp  = m_ioContextBackBuffer;
				m_ioContextBackBuffer  = m_ioContextFrontBuffer;
				m_ioContextFrontBuffer = temp;

			LeaveCriticalSection( &m_cs );

			// 수신된 데이터 처리.
			PerIoContext** buffer = m_ioContextFrontBuffer->buffer;
			long&          offset = m_ioContextFrontBuffer->offset;

			if ( offset > 0 )
			{
				do {
					try {
						PerIoContext* ioContext = (*buffer);
						RUDP*         rudp      = (RUDP*)ioContext->buffer;

						if ( g_udpFrom == true )
						{
							printf( "#Packets: Addr(%d.%d.%d.%d):Port(%d)\n"
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b1
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b2
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b3
									,ioContext->addr.sin_addr.S_un.S_un_b.s_b4
									,ioContext->addr.sin_port );
						}
						if ( g_udpPacket == true )
						{
							// 패킷해더(Packet Header) 정보 출력 - 버전/헤더길이/서비스 종류/전체길이.
							printf( "#Packets: VER:%02x/HLEN:%02x/TOS:%02x/TLEN:%04x/SEQ:%08x\n"
									,rudp->ver
									,rudp->hlen
									,rudp->tos
									,rudp->tlen
									,rudp->seq );
						}

						if ( rudp->tos == TOS_RUDP )
						{
							bool retvalue = ReSendExec( rudp->addr, rudp->port, rudp->seq );

							if ( g_udp == true )
								printf( "#Packets: ReSend->Seq(%08x) = %d\n", rudp->seq, retvalue );

							m_ioContextPool->ReleaseIoContext( ioContext );
						}
						else if ( rudp->ver == RUDP_PHVer && rudp->hlen == RUDP_PHLen )
						{
							u_long        pid          = (ioContext->addr.sin_addr.s_imp << 0x10) | ioContext->addr.sin_port;
							PacketBuffer* packetBuffer = mPacketPool->SearchPacketBuffer( pid );

							if ( packetBuffer == NULL )
							{
								packetBuffer = mPacketPool->GetPacketBuffer( pid );
							}
							if ( packetBuffer != NULL )
							{
								// Packet 흐름 검사를 위한 버퍼링.
								if ( packetBuffer->seq < rudp->seq )
								{
									if ( g_udp == true )
										printf( "PacketBuffer->Seq(%08x) < Packet->Seq(%08x)\n", packetBuffer->seq, rudp->seq );

									if ( packetBuffer->seq != 0 )
									{
										u_long lost = (rudp->seq - packetBuffer->ack);

										packetBuffer->seq = rudp->seq;
										if ( lost >= MAX_PACKET_BUFFER_LEN )
										{
											if ( g_udp == true )
												printf( "\tPackets: Buffer Overflow = %08x\n", lost );
										}
										else if ( lost > 0 )
										{
											for ( u_long ack = packetBuffer->ack; ack < packetBuffer->seq; ack++ )
											{
												u_long        mask     = (ack & SM_ClassD);
												PerIoContext* temp     = packetBuffer->buffer[ mask ];
												bool          reliable = true;

												if ( temp != NULL )
												{
													RUDP* rudp = (RUDP*)temp->buffer;
													reliable = (ack != rudp->seq);
												}
												if ( reliable )
												{
													SendReliable( rudp->addr, rudp->port, ack );
													if ( g_udp == true )
														printf( "\tPackets: Lost = %08x\n", ack );
												}
											}
										}
									}
									else
									{
										packetBuffer->seq = rudp->seq;
										packetBuffer->ack = rudp->seq;
									}
								}
								else if ( packetBuffer->seq > rudp->seq )
								{
									if ( packetBuffer->ack <= rudp->seq )
									{
										if ( g_udp == true )
											printf( "\tPackets: Recovery = %08x\n", rudp->seq );
									}
								}
								// Packet 흐름 정리를 위한 버퍼링.
								if ( packetBuffer->ack <= rudp->seq )
								{
									u_long         mask = (rudp->seq & SM_ClassD);
									PerIoContext** temp = &packetBuffer->buffer[ mask ];

									if ( (*temp) != NULL )
									{
										m_ioContextPool->ReleaseIoContext( (*temp) );
									}
									(*temp) = ioContext;

									packetBuffer->offset = max( packetBuffer->offset, mask );
									packetBuffer->update = currentTick + 180000;// 최대3분대기.
								}
								// Packet 흐름 처리를 위한 버퍼링.
								while ( packetBuffer->ack <= packetBuffer->seq )
								{
									u_long         mask = (packetBuffer->ack & SM_ClassD);
									PerIoContext** temp = &packetBuffer->buffer[ mask ];

									if ( (*temp) == NULL )
										break;

									RUDP* rudp = (RUDP*)(*temp)->buffer;

									if ( rudp->seq != packetBuffer->ack )
										break;

									// 서비스 타입이 [TOS_LOGIN] 또는 [TOS_GAME] 아닐경우 오류 처리.
									if ( rudp->tos == TOS_LOGIN || rudp->tos == TOS_GAME )
									{
										MSGBUF* msgBuf = (MSGBUF*)((char*)rudp + rudp->hlen);
										int     msgLen = rudp->tlen - rudp->hlen;

										BatchComplete( msgBuf, msgLen );
									}

									m_ioContextPool->ReleaseIoContext( (*temp) );
									(*temp) = NULL;

									packetBuffer->ack++;
								}
							}
						}
						else
						{
							m_ioContextPool->ReleaseIoContext( ioContext );
						}
					} catch ( char* str ) {
						PostServerEvent( "Caught 'BatchComplete' exception type: '%s'. Throwing 'cRudpProcess::BackendThread' exception", str );
						Sleep( 100 );
						throw;
					} catch ( ... ) {
						PostServerEvent( "In BatchComplete Method. Throwing 'cRudpProcess::BackendThread' exception." );
						Sleep( 100 );
						throw;
					}
					buffer++;
					offset--;
				} while ( offset > 0 );
			}
			// PacketPool 데이터 처리.
			if ( mPacketPool != NULL )
			{
				PacketBuffer* packetBuffer = mPacketPool->GetPagedPoolUsage( );
				PacketBuffer* next         = NULL;

				while ( packetBuffer )
				{
					next = packetBuffer->next;
						if ( packetBuffer->update < currentTick )
						{
							for ( u_long i = 0; i <= packetBuffer->offset; i++ )
							{
								PerIoContext* ioContext = packetBuffer->buffer[ i ];
								if ( ioContext != NULL )
								{
									m_ioContextPool->ReleaseIoContext( ioContext );
								}
							}
							mPacketPool->ReleasePacketBuffer( packetBuffer );
						}
					packetBuffer = next;
				}
			}
			// Relay Packet 발송.
			if ( relayTick < currentTick )
			{
				HANDLE handle = NULL;

				if ( GetPacket( &handle, TOS_RELAY, mLogin, mLoginSeq ) != NULL )
					SendPacket( handle );

				if ( GetPacket( &handle, TOS_RELAY, mGame, mGameSeq ) != NULL )
					SendPacket( handle );

				if ( GetPacket( &handle, TOS_RELAY, mLog, mLogSeq ) != NULL )
					SendPacket( handle );

				relayTick = currentTick + 1000;
			}

			// 지연시간 약16(ms) - CPU 과부화 방지 & 내부 동기화.
			elapsedTick = GetTickCount( ) - currentTick;
			if ( elapsedTick < 16 )
			{
				Sleep( (16 - elapsedTick) );
			}
		}
	} catch ( char* str ) {
		PostServerEvent( "Caught 'while ( true ) { ... }' exception type: '%s'. Throwing 'cRudpProcess::BackendThread' exception", str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In while ( true ) { ... }. Throwing 'cRudpProcess::BackendThread' exception." );
		Sleep( 100 );
		throw;
	}
	return 0;
}
