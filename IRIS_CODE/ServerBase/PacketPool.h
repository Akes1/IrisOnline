/* ====================================================================
 * 파    일 :	PacketPool.h
 * 목    적 :	
 * 작 성 자 :	
 * 작 성 일 :	09/08/24
 * 주의사항 :	
 * =================================================================== */
#ifndef __PACKET_POOL_H__
#define __PACKET_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

// 윈도우즈 헤더
#include <winsock2.h>
#include <windows.h>
#include "iocontextpool.h"

#define MAX_PACKET_BUFFER_LEN	0x100					// 0x00 ~ 0xff = 0x100

struct PacketBuffer
{
	u_long               pid;							// Connection Index (Server & Channel Number)
	u_long               seq;							// Sequence Number
	u_long               ack;							// Acknowledgment Number
	PerIoContext*        buffer[MAX_PACKET_BUFFER_LEN];	// I/O Context Buffer
	u_long               offset;						// Offset
	DWORD                update;						// Update

	struct PacketBuffer* next;							// Linked List - next
	struct PacketBuffer* prev;							// "           - prev

	struct PacketBuffer* parent;						// Binary Tree - Parent
	struct PacketBuffer* left;							// "           - Child, left
	struct PacketBuffer* right;							// "           - Child, right
};

class PacketPool
{
private:
	PacketBuffer* mPagedPoolUsage;			// 페이지 된 풀.
	PacketBuffer* mNonPagedPoolUsage;		// 페이지 안된 풀.
	PacketBuffer* mBstRoot;					// BST - 루트.

	SIZE_T        mQuotaPagedPoolUsage;      // 현재 페이지 된 풀   - 사용량.
	SIZE_T        mQuotaNonPagedPoolUsage;   // 현재 페이지 안된 풀 - 사용량.
	SIZE_T        mWorkingSetSize;           // 현재 작업 설정크기.

	PacketBuffer* AllocPacketBuffer   ( );
	void          FreePacketBuffer    ( PacketBuffer** packetBuffer );

	int           Compare             ( long pid1, long pid2 );

	void          AttachPool          ( PacketBuffer** pool, PacketBuffer* packetBuffer );
	void          DetachPool          ( PacketBuffer** pool, PacketBuffer* packetBuffer );

	bool          AttachBst           ( PacketBuffer* packetBuffer );
	bool          DetachBst           ( PacketBuffer* packetBuffer );

	PacketBuffer* GetPool             ( long pid );
	void          ReleasePool         ( PacketBuffer* packetBuffer );

public:
	PacketPool( );

	void          Shutdown            ( );

	PacketBuffer* GetPagedPoolUsage   ( ) { return mPagedPoolUsage; }
	PacketBuffer* SearchPacketBuffer  ( long pid );

	PacketBuffer* GetPacketBuffer     ( long pid );
	void          ReleasePacketBuffer ( PacketBuffer* packetBuffer );

public:
	virtual ~PacketPool(void);
};

#endif // __PACKET_POOL_H__