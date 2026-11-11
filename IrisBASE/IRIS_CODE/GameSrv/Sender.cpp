// Include
#include "gamesrv.h"
#include "packet.h"

// Local definitions
#pragma warning( disable: 4127 )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4267 )

#define RELAY_DEALY	1000

// Global data

// cSender Constructor
cSender::cSender(void) : mLoginBuffer(NULL), mGameBuffer(NULL), mLogBuffer(NULL)
{}

// ~cSender Destructor
cSender::~cSender(void)
{}

// Initialize Method
bool cSender::Initialize(char* send, char* recv, u_short numWorkerThreads, unsigned int bufferLength)
{
	PHOSTENT phe;

	// Login Server IPv4 주소를 만든다.
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

	// Game Server IPv4 주소를 만든다.
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
	mRecv.sin_port   = htons( g_gamePort );
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
	if ( mLoginBuffer )
	{
		GlobalFree( mLoginBuffer );
		mLoginBuffer = NULL;
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

	if ( htons( g_loginPort ) == port )
	{
		ioContext = mLoginBuffer->buffer[ mask ];
	}
	else if ( htons( g_gamePort ) == port )
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
char* cSender::GetPacket(void** handle, char tos, SOCKADDR_IN addr, u_long& seq)
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

		(*handle)  = perIoContext;
		retvalue   = perIoContext->buffer + rudp->hlen;
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
bool cSender::PostMemberEvent(char type, char category, long memberIdx, char* message)
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
bool cSender::PostCharacterEvent(char category, long memberIdx, long serverIdx, long characterIdx, char _level, long _exp, char _skillLevel, long _skillExp, long _skillPoint, char* message)
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
bool cSender::PostMoneyEvent(long serverIdx, long characterIdx, long before, long after, char* message)
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
bool cSender::PostDepositEvent(long serverIdx, long characterIdx, long before, long after, char* message)
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
bool cSender::PostInventoryEvent(long serverIdx, char category, long inventoryIdx, long characterIdx, long itemDefineIndex, short number, short count, BYTE enhanced, BYTE seal, char* message)
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
bool cSender::PostQuestEvent(long serverIdx, char category, long characterIdx, long questIdx, char* message)
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
bool cSender::PostGuildEvent(long serverIdx, long characterIdx, long guildIdx, char guildPosition, char* message)
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
bool cSender::PostDropItemEvent(long serverIdx, long channelNum, sDropItem* pDropItem, unsigned long count)
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
bool cSender::PostSkillEvent(long serverIdx, long characterIdx, bool isReset, long skillIdx, char skillStep)
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

// PostItemBillEvent Method
bool cSender::PostItemBillEvent(long serverIdx, long characterIdx, long inventoryIdx, long before, long after)
{
	HANDLE           handle        = NULL;
	ITEM_BILL_EVENT* itemBillEvent = (ITEM_BILL_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( itemBillEvent != NULL )
	{
		char* computer = g_gameSrv->GetHostName();

		itemBillEvent->protocol     = MB_ITEM_BILL_EVENT;
		GetRegisterDate( &itemBillEvent->registerDate );
		itemBillEvent->serverIdx    = serverIdx;
		itemBillEvent->characterIdx = characterIdx;
		itemBillEvent->inventoryIdx = inventoryIdx;
		itemBillEvent->before       = before;
		itemBillEvent->after        = after;
		strcpy( itemBillEvent->computer, computer );

		return SendPacket( handle, sizeof(ITEM_BILL_EVENT) );
	}
	return false;
}

// PostItemBillEvent Method
bool cSender::PostInfCashEvent(long serverIdx, long characterIdx, char type, sInfluence* pInf, unsigned long count )
{
	HANDLE                 handle  = NULL;
	MB_SYN_INFCASH_SAVE* postMsg = (MB_SYN_INFCASH_SAVE*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );
	unsigned long length = sizeof(MB_SYN_INFCASH_SAVE) - sizeof(char);

	if ( postMsg != NULL )
	{
		char* computer = g_gameSrv->GetHostName();

		postMsg->protocol = MB_INFCASH_EVENT;
		postMsg->mServerNum = serverIdx;
		postMsg->mCharacterIdx = characterIdx;
		postMsg->mType = type;
		postMsg->mCount = count;
		strcpy( postMsg->mComputer, computer );
		for( unsigned long i = 0 ; i < count ; ++i )
		{
			postMsg->mInfCash[i].mAttackerDamageMax = pInf[i].mAttackerDamageMax;
			postMsg->mInfCash[i].mUniqueIdx = pInf[i].mUniqueIdx;
			postMsg->mInfCash[i].mDbIdx = pInf[i].mDbIdx;
			postMsg->mInfCash[i].mInfluenceClassIdx = pInf[i].mInfluenceClassIdx;
			postMsg->mInfCash[i].mAttackerIdx = pInf[i].mAttacker.index;
			postMsg->mInfCash[i].mAttackerType = pInf[i].mAttacker.type;
			postMsg->mInfCash[i].mIsRealTime = pInf[i].mIsRealTime;
			postMsg->mInfCash[i].mRestTime = pInf[i].mRestTime;
			postMsg->mInfCash[i].mEndDate = pInf[i].mEndDate;
			postMsg->mInfCash[i].mTime1 = pInf[i].mTime1;
			postMsg->mInfCash[i].mTime2 = pInf[i].mTime2;
			postMsg->mInfCash[i].mInfluenceValue1 = pInf[i].mInfluenceValue1;
			postMsg->mInfCash[i].mInfluenceValue2 = pInf[i].mInfluenceValue2;
			postMsg->mInfCash[i].mAttriButeType = pInf[i].mAttriButeType;
			postMsg->mInfCash[i].mAttackerLv = pInf[i].mAttackerLv;
			postMsg->mInfCash[i].mAttackerDamageMin = pInf[i].mAttackerDamageMin;
			postMsg->mInfCash[i].mAttackerDamageMax = pInf[i].mAttackerDamageMax;
			postMsg->mInfCash[i].mHeal = pInf[i].mHeal;
			postMsg->mInfCash[i].mLeftGuardCnt = pInf[i].mLeftGuardCnt;
			postMsg->mInfCash[i].mLeftGuardValue = pInf[i].mLeftGuardValue;
			postMsg->mInfCash[i].mDel = pInf[i].mDel;
		}

		length += sizeof(sInfluence)*count;
		return SendPacket( handle, length );
	}
	return false;
}

// PostReservedEvent Method
bool cSender::PostReservedEvent(long serverIdx, short type, long characterIdx, long memberIdx)
{
	HANDLE          handle        = NULL;
	RESERVED_EVENT* reservedEvent = (RESERVED_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( reservedEvent != NULL )
	{
		char* computer = g_gameSrv->GetHostName();

		reservedEvent->protocol      = MB_RESERVED_EVENT;
		GetRegisterDate( &reservedEvent->registerDate );
		reservedEvent->serverIdx    = serverIdx;
		reservedEvent->type         = type;
		reservedEvent->characterIdx = characterIdx;
		reservedEvent->memberIdx    = memberIdx;
		strcpy( reservedEvent->computer, computer );

		return SendPacket( handle, sizeof(RESERVED_EVENT) );
	}
	return false;
}

// PostPostEvent Method
bool cSender::PostPostEvent(long serverIdx, long postIdx, long characterIdx, long inventoryIdx1, long inventoryIdx2, long inventoryIdx3, long money)
{
	HANDLE      handle    = NULL;
	POST_EVENT* postEvent = (POST_EVENT*)GetPacket( &handle, TOS_GAME_LOG, mLog, mLogSeq );

	if ( postEvent != NULL )
	{
		char* computer = g_gameSrv->GetHostName();

		postEvent->protocol      = MB_POST_EVENT;
		GetRegisterDate( &postEvent->registerDate );
		postEvent->serverIdx     = serverIdx;
		postEvent->postIdx       = postIdx;
		postEvent->characterIdx  = characterIdx;
		postEvent->inventoryIdx1 = inventoryIdx1;
		postEvent->inventoryIdx2 = inventoryIdx2;
		postEvent->inventoryIdx3 = inventoryIdx3;
		postEvent->money         = money;
		strcpy( postEvent->computer, computer );

		return SendPacket( handle, sizeof(POST_EVENT) );
	}
	return false;
}

// PostChStatus Method
bool cSender::PostChStatus(long cid, BYTE status)
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
bool cSender::PostServerNotice(long cid, wchar_t* message, u_int length)
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

// PostServerDownCountNotice Method
bool cSender::PostServerDownCountNotice(long cid, unsigned short count )
{
	HANDLE                handle       = NULL;
	MB_SYN_SERVER_DOWN_COUNT_NOTICE* serverNotice = (MB_SYN_SERVER_DOWN_COUNT_NOTICE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( serverNotice != NULL )
	{
		serverNotice->protocol = MB_SERVER_DOWN_COUNT_NOTICE_SYN;
		serverNotice->cid      = cid;
		serverNotice->count	   = count;

		return SendPacket( handle, sizeof(MB_SYN_SERVER_DOWN_COUNT_NOTICE) );
	}
	return false;
}

// PostChatMegaphone Method
bool cSender::PostChatMegaphone(long cid, char* msg, int len)
{
	HANDLE                 handle  = NULL;
	MB_SYN_CHAT_MEGAPHONE* sendMsg = (MB_SYN_CHAT_MEGAPHONE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( sendMsg != NULL )
	{
		unsigned long length = sizeof(MB_SYN_CHAT_MEGAPHONE) - sizeof(sendMsg->msg);

		sendMsg->protocol = MB_CHAT_MEGAPHONE_SYN;
		sendMsg->cid      = cid;
		memcpy( sendMsg->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

// PostPVPJoin Method
bool cSender::PostPVPStatus(long cid, BYTE status, unsigned short pvpType, unsigned long aryPos )
{
	HANDLE             handle  = NULL;
	MB_SYN_PVP_STATUS* postMsg = (MB_SYN_PVP_STATUS*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol = MB_PVP_STATUS_SYN;
		postMsg->cid      = cid;
		postMsg->status   = status;
		postMsg->pvpType  = pvpType;
		postMsg->aryPos   = aryPos;
		return SendPacket( handle, sizeof(MB_SYN_PVP_STATUS) );
	}
	return false;
}

// PostPVPJoin Method
bool cSender::PostPVPNotice( long cid, unsigned char leftMinute, unsigned short pvpType, unsigned long aryPos )
{
	HANDLE             handle  = NULL;
	MB_SYN_PVP_NOTICE* postMsg = (MB_SYN_PVP_NOTICE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol   = MB_PVP_NOTICE_SYN;
		postMsg->cid        = cid;
		postMsg->leftMinute = leftMinute;
		postMsg->pvpType    = pvpType;
		postMsg->aryPos     = aryPos;

		return SendPacket( handle, sizeof(MB_SYN_PVP_NOTICE) );
	}
	return false;
}

bool cSender::PostPVPPlayerList(long cid, char* msg, int len)
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
bool cSender::PostThemeStatus(long cid, BYTE status, unsigned short mapType )
{
	HANDLE             handle  = NULL;
	MB_SYN_THEME_STATUS* postMsg = (MB_SYN_THEME_STATUS*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol = MB_THEME_STATUS_SYN;
		postMsg->cid      = cid;
		postMsg->status   = status;
		postMsg->mapType  = mapType;

		return SendPacket( handle, sizeof(MB_SYN_THEME_STATUS) );
	}
	return false;
}

bool cSender::PostPvPGmChannelMove( long cid, unsigned long playerIdx, unsigned short mapType )
{
	HANDLE             handle  = NULL;
	MB_SYN_PVPGM_CHANNELMOVE* postMsg = (MB_SYN_PVPGM_CHANNELMOVE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postMsg != NULL )
	{
		postMsg->protocol   = MB_PVP_GMCHANNELMOVE_SYN;
		postMsg->cid        = cid;
		postMsg->playerIdx  = playerIdx;
		postMsg->mapType    = mapType;

		return SendPacket( handle, sizeof(MB_SYN_PVPGM_CHANNELMOVE) );
	}
	return false;
}

bool cSender::PostFriendSync( long cid, char* msg, int len )
{
	HANDLE			   handle = NULL;
	MB_SYN_FRIEND_SYN* friendSync = (MB_SYN_FRIEND_SYN*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );
	
	if( friendSync != NULL )
	{
		unsigned long length = sizeof(MB_SYN_FRIEND_SYN)-sizeof(friendSync->msg);
			
			friendSync->protocol = MB_FRIEND_SYN;
			friendSync->cid		 = cid;
			memcpy( friendSync->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

// PostGuildSync Method
bool cSender::PostGuildSync(long cid, char* msg, int len)
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

// PostMonsterSync Method
bool cSender::PostMonsterSync(long cid, char* msg, int len)
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
bool cSender::PostChatSync(long cid, char* msg, int len)
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
bool cSender::PostPostReceived(long cid, long characterIdx)
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

// PostPartySync Method
bool cSender::PostPartySync(long cid, char* msg, int len)
{
	HANDLE        handle = NULL;
	MB_SYN_PARTY* msgBuf = (MB_SYN_PARTY*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		unsigned long length = sizeof(MB_SYN_PARTY)-sizeof(msgBuf->msg);

		msgBuf->protocol     = MB_PARTY_SYN;
		msgBuf->cid          = cid;
		memcpy( msgBuf->msg, msg, len );

		length += len;
		return SendPacket( handle, length );
	}
	return false;
}

bool cSender::PostEnhancedSync(long cid, wchar_t* name, char* msg, int len)
{
	HANDLE                  handle = NULL;
	MB_SYN_ENHANCED_RESULT* msgBuf = (MB_SYN_ENHANCED_RESULT*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		unsigned long length = sizeof(MB_SYN_ENHANCED_RESULT)-sizeof(msgBuf->msg);

		msgBuf->protocol     = MB_ENHANCED_RESULT_SYN;
		msgBuf->cid          = cid;
		memcpy( msgBuf->msg, msg, len );

		if ( name )
		{
			wcscpy( msgBuf->name, name );
		}

		length += len;
		return SendPacket( handle, length );
	}

	return false;
}

bool cSender::PostNoChatSync( long cid, long characterIdx, long validThru )
{
	HANDLE                  handle = NULL;
	MB_SYN_NO_CHAT*			msgBuf = (MB_SYN_NO_CHAT*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		msgBuf->protocol     = MB_NO_CHAT_SYN;
		msgBuf->cid          = cid;
		msgBuf->characterIdx = characterIdx;
		msgBuf->validThru    = validThru;

		return SendPacket( handle, sizeof(MB_SYN_NO_CHAT) );
	}

	return false;
}

bool cSender::PostUniqueItemSync(long cid, wchar_t* name, char* msg, int len)
{
	HANDLE                  handle = NULL;
	MB_SYN_UNIQUEITEM_GET*  msgBuf = (MB_SYN_UNIQUEITEM_GET*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		unsigned long length = sizeof(MB_SYN_UNIQUEITEM_GET)-sizeof(msgBuf->msg);

		msgBuf->protocol     = MB_UNIQUEITEM_GET_SYN;
		msgBuf->cid          = cid;
		memcpy( msgBuf->msg, msg, len );

		if ( name )
			wcscpy( msgBuf->name, name );

		length += len;
		return SendPacket( handle, length );

	}

	return false;
}

bool cSender::PostAddGMEvent( long cid, unsigned long index, unsigned long influenceIdx, time_t startTime, time_t validThru )
{
	HANDLE                  handle = NULL;
	MB_SYN_ADD_GM_EVENT*	msgBuf = (MB_SYN_ADD_GM_EVENT*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		msgBuf->protocol     = MB_ADD_GM_EVENT_SYN;
		msgBuf->cid          = cid;
 		msgBuf->index		 = index;
 		msgBuf->influenceIdx = influenceIdx;
		msgBuf->startTime	 = startTime;
		msgBuf->validThru    = validThru;

		return SendPacket( handle, sizeof(MB_SYN_ADD_GM_EVENT) );
	}

	return false;
}

bool cSender::PostDelGMEvent( long cid, unsigned long dbIndex, unsigned long influenceIdx )
{
	HANDLE                  handle = NULL;
	MB_SYN_DEL_GM_EVENT*	msgBuf = (MB_SYN_DEL_GM_EVENT*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		msgBuf->protocol     = MB_DEL_GM_EVENT_SYN;
		msgBuf->cid          = cid;
		msgBuf->index		 = dbIndex;
		msgBuf->influenceIdx = influenceIdx;

		return SendPacket( handle, sizeof(MB_SYN_DEL_GM_EVENT) );
	}

	return false;
}

bool cSender::PostGMKickEvent( long cid, long characterIdx )
{
	HANDLE                  handle = NULL;
	MB_SYN_GM_KICK_EVENT*	msgBuf = (MB_SYN_GM_KICK_EVENT*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( msgBuf != NULL )
	{
		msgBuf->protocol     = MB_GM_KICK_EVENT_SYN;
		msgBuf->cid          = cid;
		msgBuf->characterIdx = characterIdx;

		return SendPacket( handle, sizeof(MB_SYN_GM_KICK_EVENT) );
	}

	return false;
}

// PostPostReceived Method
bool cSender::PostItemDropLimitUpdate( long cid, unsigned long itemIdx, TIMESTAMP_STRUCT endTime )
{
	HANDLE                handle       = NULL;
	MB_SYN_DROPITEM_UPDATE* postReceived = (MB_SYN_DROPITEM_UPDATE*)GetPacket( &handle, TOS_GAME, mGame, mGameSeq );

	if ( postReceived != NULL )
	{
		postReceived->protocol = MB_ITEM_DROPLIMIT_UPDATE;
		postReceived->mCid = cid;
		postReceived->mItemIdx = itemIdx;
		postReceived->mDropEndTime = endTime;
		
		return SendPacket( handle, sizeof(MB_SYN_DROPITEM_UPDATE) );
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
