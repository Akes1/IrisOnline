/* ====================================================================
 * 파    일 :	Sender.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	Created 02/13/07
 * 주의사항 :	
 * =================================================================== */
#ifndef __SENDER_H__
#define __SENDER_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpudpsend.h"

#include "serverProtocol.h"
#include "logprotocol.h"

class cSender :
	public cIocpUdpSend
{
private:
	SOCKADDR_IN      mLogin;
	u_long           mLoginSeq;
	IoContextBuffer* mLoginBuffer;
	DWORD            mLoginRelay;

	SOCKADDR_IN      mGame;
	u_long           mGameSeq;
	IoContextBuffer* mGameBuffer;
	DWORD            mGameRelay;

	SOCKADDR_IN      mLog;
	u_long           mLogSeq;
	IoContextBuffer* mLogBuffer;
	DWORD            mLogRelay;

	SOCKADDR_IN      mRecv;

public:
// Log 관련 메소드.
	bool    PostServerEvent    ( char* message );
	bool    PostMemberEvent    ( char type, char category, long memberIdx, char* message );
	bool    PostCharacterEvent ( char category, long memberIdx, long serverIdx, long characterIdx, char _level, long _exp, char _skillLevel, long _skillExp, long _skillPoint, char* message );
	bool    PostMoneyEvent     ( long serverIdx, long characterIdx, long before, long after, char* message );
	bool    PostDepositEvent   ( long serverIdx, long characterIdx, long before, long after, char* message );
	bool    PostInventoryEvent ( long serverIdx, char category, long inventoryIdx, long characterIdx, long itemDefineIndex, short number, short count, BYTE enhanced, BYTE seal, char* message );
	bool    PostQuestEvent     ( long serverIdx, char category, long characterIdx, long questIdx, char* message );
	bool	PostGuildEvent	   ( long serverIdx, long characterIdx, long guildIdx, char guildPosition, char* message );
	bool	PostDropItemEvent  ( long serverIdx, long channelNum, sDropItem* pDropItem, unsigned long count );
	bool    PostSkillEvent     ( long serverIdx, long characterIdx, bool isReset, long skillIdx, char skillStep );
	bool    PostItemBillEvent  ( long serverIdx, long characterIdx, long inventoryIdx, long before, long after );
	bool    PostInfCashEvent   ( long serverIdx, long characterIdx, char type, sInfluence* pInf, unsigned long count );
	bool    PostReservedEvent  ( long serverIdx, short type, long characterIdx, long memberIdx );
	bool    PostPostEvent      ( long serverIdx, long postIdx, long characterIdx, long inventoryIdx1, long inventoryIdx2, long inventoryIdx3, long money );

public:
// Login 관련 메소드.
	bool    PostChStatus       ( long cid, BYTE status );

public:
// Game 관련 메소드.
	bool    PostServerNotice   ( long cid, wchar_t* message, u_int length );
	bool	PostServerDownCountNotice(long cid, unsigned short count );
	bool    PostChatMegaphone  ( long cid, char* msg, int len );
	bool    PostPVPStatus      ( long cid, BYTE status, unsigned short pvpType, unsigned long aryPos );
	bool    PostPVPNotice      ( long cid, unsigned char leftMinute, unsigned short pvpType, unsigned long aryPos );
	bool    PostPVPPlayerList  ( long cid, char* msg, int len );
	bool    PostThemeStatus    ( long cid, BYTE status, unsigned short mapType );
	bool	PostPvPGmChannelMove( long cid, unsigned long playerIdx, unsigned short mapType );
	bool    PostFriendSync     ( long cid, char* msg, int len );
	bool    PostGuildSync      ( long cid, char* msg, int len );
	bool	PostMonsterSync    ( long cid, char* msg, int len );
	bool	PostChatSync	   ( long cid, char* msg, int len );
	bool	PostPostReceived   ( long cid, long characterIdx );
	bool    PostPartySync      ( long cid, char* msg, int len );
    bool    PostEnhancedSync   ( long cid, wchar_t* name, char* msg, int len );
	bool    PostNoChatSync	   ( long cid, long characterIdx, long validThru );
	bool    PostUniqueItemSync ( long cid, wchar_t* name, char* msg, int len );
	bool	PostAddGMEvent	   ( long cid, unsigned long index, unsigned long influenceIdx, time_t startTime, time_t validThru );
	bool	PostDelGMEvent	   ( long cid, unsigned long dbIndex, unsigned long influenceIdx );
	bool	PostItemDropLimitUpdate( long cid, unsigned long itemIdx, TIMESTAMP_STRUCT endTime );


	bool	PostGMKickEvent	   ( long cid, long characterIdx );

private:
// 서버 관련 메소드.
	bool    SendComplete       ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 클래스 생성 및 공용 메소드.
	cSender(void);

	bool    Initialize         ( char* send, char* recv, u_short numWorkerThreads=2, unsigned int bufferLength=65535 );
	void    Shutdown           ( DWORD maxWait=INFINITE );

	bool    ReSendPost         ( u_long addr, u_short port, u_long seq );
	bool    SendReliable       ( u_long addr, u_short port, u_long seq );

	char*   GetPacket          ( void** handle, char tos, SOCKADDR_IN addr, u_long& seq );
	bool    SendPacket         ( void* handle, unsigned long length=0 );
	void    GetRegisterDate    ( TIMESTAMP_STRUCT* ts );

	DWORD   BackendThread      ( );

public:
// 클래스 파괴 메소드.
	virtual ~cSender(void);
};

#endif // __SENDER_H__
