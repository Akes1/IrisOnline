/* ====================================================================
 * 파    일 :	BillConnector.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	Created 01/08/10
 * 주의사항 :	
 * =================================================================== */
#ifndef __BILL_CONNECTOR_H__
#define __BILL_CONNECTOR_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpclient.h"

class cBillConnector :
	public cIocpClient
{
private:
// LOG 관련 메소드.
	bool  PostServerEvent  ( LPCTSTR format, ... );

private:
// 서버 관련 메소드 - 보조.
	void  PacketResultAuth  ( long result );
	void  PacketResultAlert ( long result );

private:
// 서버 관련 메소드 - 주.
	bool  RecvComplete  ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	void  Close         ( ULONG_PTR completionKey, PerIoContext* perIoContext );
public:
	bool  ServerConn    ( char* Game_Server, long Game_No=4 );
	bool  ClientMove    ( char* Game_Server, char* Session, char* User_ID, char* User_IP, char* User_Status, long Game_No=4 );
	bool  ServerReset   ( char* Game_Server, long Game_No=4 );
	bool  BillingAuth   ( char* Session, char* User_ID, char* User_IP, long Game_No=4 );
	bool  GameStart     ( char* Game_Server, char* Session, char* User_ID, char* User_IP, long Game_No=4 );
	bool  UserSync      ( long Packet_Result, char* Session, char* User_ID, char* User_IP, char* User_Status );
	bool  GameEnd       ( char* Session, char* User_ID, char* User_IP, char* User_Status, long Game_No=4 );
	bool  ServerAlive   ( long Game_No=4 );

public:
	cBillConnector(void);

	bool  Initialize    ( char* ipAddr="", unsigned short port=5001, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535 );
	void  Shutdown      ( DWORD maxWait=INFINITE );

	DWORD BackendThread ( );

public:
	virtual ~cBillConnector(void);

public:
	static cBillConnector* mBillConnector;
};

#define PGBILLING     cBillConnector::mBillConnector

#endif // __BILL_CONNECTOR_H__