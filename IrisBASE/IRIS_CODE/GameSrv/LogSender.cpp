// Include
#include "gamesrv.h"

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
		packet->tos  = TOS_GAME;
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
		char*  source   = g_gameSrv->GetServiceName( );
		char*  computer = g_gameSrv->GetHostName( );
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
	HANDLE        handle      = NULL;
	MEMBER_EVENT* memberEvent = (MEMBER_EVENT*)GetPacket( &handle, MB_MEMBER_EVENT );
	if ( memberEvent != NULL )
	{
		char*  source   = g_gameSrv->GetServiceName( );
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(MEMBER_EVENT) - sizeof(memberEvent->message);

		memberEvent->type      = type;
		strcpy( memberEvent->source, source );
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
bool cLogSender::PostCharacterEvent(char category, long memberIdx, long serverIdx, long characterIdx, char* message)
{
	HANDLE           handle         = NULL;
	CHARACTER_EVENT* characterEvent = (CHARACTER_EVENT*)GetPacket( &handle, MB_CHARACTER_EVENT );
	if ( characterEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(CHARACTER_EVENT) - sizeof(characterEvent->message);

		characterEvent->category     = category;
		characterEvent->memberIdx    = memberIdx;
		characterEvent->serverIdx    = serverIdx;
		characterEvent->characterIdx = characterIdx;
		strcpy( characterEvent->computer, computer );
		strcpy( characterEvent->message, message );

		length += strlen(characterEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}

// PostMoneyEvent Method
bool cLogSender::PostMoneyEvent(long serverIdx, long characterIdx, long before, long after, char* message)
{
	HANDLE       handle     = NULL;
	MONEY_EVENT* moneyEvent = (MONEY_EVENT*)GetPacket( &handle, MB_MONEY_EVENT );
	if ( moneyEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(MONEY_EVENT) - sizeof(moneyEvent->message);

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
bool cLogSender::PostDepositEvent(long serverIdx, long characterIdx, long before, long after, char* message)
{
	HANDLE         handle = NULL;
	DEPOSIT_EVENT* depositEvent = (DEPOSIT_EVENT*)GetPacket( &handle, MB_DEPOSIT_EVENT );
	if ( depositEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(DEPOSIT_EVENT) - sizeof(depositEvent->message);

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
bool cLogSender::PostInventoryEvent(long serverIdx, char category, long inventoryIdx, long characterIdx, long itemDefineIndex, short number, short count, BYTE enhanced, BYTE seal, char* message)
{
	HANDLE           handle = NULL;
	INVENTORY_EVENT* inventoryEvent = (INVENTORY_EVENT*)GetPacket( &handle, MB_INVENTORY_EVENT );
	if ( inventoryEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName( );
		u_long length   = sizeof(INVENTORY_EVENT) - sizeof(inventoryEvent->message);

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

bool cLogSender::PostQuestEvent( long serverIdx, char category, long characterIdx, long questIdx, char* message )
{
	HANDLE       handle     = NULL;
	QUEST_EVENT* questEvent = (QUEST_EVENT*)GetPacket( &handle, MB_QUEST_EVENT );
	if ( questEvent != NULL )
	{
		char*  computer = g_gameSrv->GetHostName();
		u_long length   = sizeof(QUEST_EVENT) - sizeof(questEvent->message);

		questEvent->serverIdx	 = serverIdx;
		questEvent->category	 = category;
		questEvent->characterIdx = characterIdx;
		questEvent->questIdx	 = questIdx;
		strcpy( questEvent->computer, computer );
		strcpy( questEvent->message, message );

		length += strlen(questEvent->message);
		return SendPacket( handle, length );
	}
	return false;
}