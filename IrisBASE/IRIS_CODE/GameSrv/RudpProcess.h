/* ====================================================================
 * 파    일 :	rudpProcess.h
 * 목    적 :
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/08/31
 * 주의사항 :	
 * =================================================================== */
#ifndef __RUDP_PROCESS_H__
#define __RUDP_PROCESS_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocprudp.h"

#include "serverProtocol.h"
#include "logprotocol.h"

#include "packetpool.h"

class cRudpProcess :
	public IocpRudp
{
private:
	PacketPool*      mPacketPool;

	SOCKADDR_IN      mLogin;
	u_long           mLoginSeq;
	IoContextBuffer* mLoginBuffer;

	SOCKADDR_IN      mGame;
	u_long           mGameSeq;
	IoContextBuffer* mGameBuffer;

	SOCKADDR_IN      mLog;
	u_long           mLogSeq;
	IoContextBuffer* mLogBuffer;

public:
// LOG 관련 메소드.
	bool  PostServerEvent  ( LPCTSTR format, ... );

	bool  PostServerEvent    ( char* message );
	bool  PostMemberEvent    ( char type, char category, long memberIdx, char* message );
	bool  PostCharacterEvent ( char category, long memberIdx, long serverIdx, long characterIdx, char _level, long _exp, char _skillLevel, long _skillExp, long _skillPoint, char* message );
	bool  PostMoneyEvent     ( long serverIdx, long characterIdx, long before, long after, char* message );
	bool  PostDepositEvent   ( long serverIdx, long characterIdx, long before, long after, char* message );
	bool  PostInventoryEvent ( long serverIdx, char category, long inventoryIdx, long characterIdx, long itemDefineIndex, short number, short count, BYTE enhanced, BYTE seal, char* message );
	bool  PostQuestEvent     ( long serverIdx, char category, long characterIdx, long questIdx, char* message );
	bool  PostGuildEvent	 ( long serverIdx, long characterIdx, long guildIdx, char guildPosition, char* message );
	bool  PostDropItemEvent  ( long serverIdx, long channelNum, sDropItem* pDropItem, unsigned long count );
	bool  PostSkillEvent     ( long serverIdx, long characterIdx, bool isReset, long skillIdx, char skillStep );

public:
// Login 관련 메소드.
	bool    PostChStatus       ( long cid, BYTE status );

public:
// Game 관련 메소드.
	bool  PostServerNotice     ( long cid, wchar_t* message, u_int length );
	bool  PostChatShout        ( long cid, unsigned short numOfMap, long characterIdx, wchar_t* name, wchar_t* message, u_int length );
	bool  PostPVPStatus        ( long cid, BYTE status, unsigned char pvpType );
	bool  PostPVPNotice        ( long cid, unsigned char leftMinute, unsigned char pvpType );
	bool  PostPVPPlayerList    ( long cid, char* msg, int len );
	bool  PostThemeStatus      ( long cid, BYTE status );
	bool  PostThemeInvite      ( long cid, short themeIdx, short roomIdx, long requestor, long characterIdx, char* msg, int len );
	bool  PostThemeInviteRes   ( long cid, long requestor, wchar_t* name, char errorCode );
	bool  PostGuildSync        ( long cid, char* msg, int len );
	bool  PostGuildConnectSync ( long cid );
	bool  PostMonsterSync      ( long cid, char* msg, int len );
	bool  PostChatSync	       ( long cid, char* msg, int len );
	bool  PostPostReceived     ( long cid, long characterIdx );

private:
// 서버 관련 메소드 - 보조.
	bool  BatchComplete    ( MSGBUF* msgBuf, int msgLen );

private:
// 서버 관련 메소드 - 주.
	bool  RecvComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	bool  SendComplete     ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

public:
// 클래스 생성 및 공용 메소드.
	cRudpProcess(void);

	bool  Initialize       ( char* recv="", char* send="", unsigned short port=14700, unsigned short numWorkerThreads=2 );
	void  Shutdown         ( DWORD maxWait=INFINITE );

	bool  ReSendExec       ( u_long addr, u_short port, u_long seq );
	bool  SendReliable     ( u_long addr, u_short port, u_long seq );

	char* GetPacket        ( void** handle, char tos, SOCKADDR_IN addr, u_long& seq );
	bool  SendPacket       ( void* handle, unsigned long length=0 );
	void  GetRegisterDate  ( TIMESTAMP_STRUCT* ts );

	DWORD BackendThread    ( );

public:
// 클래스 파괴 메소드.
	virtual ~cRudpProcess(void);
};

#endif // __RUDP_PROCESS_H__
