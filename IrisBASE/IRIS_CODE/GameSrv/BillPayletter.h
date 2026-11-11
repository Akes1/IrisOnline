/* ====================================================================
 * 파    일 :	BillPayletter.h
 * 목    적 :	페이레터 빌링 연결
 * 작 성 자 :	박경희
 * 작 성 일 :	Created 05/11/10
 * 주의사항 :	
 * =================================================================== */
#ifndef __BILL_PAYLETTER_H__
#define __BILL_PAYLETTER_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpclient.h"

class cBillPayletter :
	public cIocpClient
{
private:
// LOG 관련 메소드.
	bool  PostServerEvent  ( LPCTSTR format, ... );

private:
// 서버 관련 메소드 - 주.
	bool		RecvComplete( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	void		Close( ULONG_PTR completionKey, PerIoContext* perIoContext );
public:
	bool		ServerConnect();
	bool		UserAuthentication( unsigned long ipAddr, unsigned long memberIdx );
	bool		ServerAlive();
	bool		UserAlive( unsigned long memberIdx );
	bool		UserOut( unsigned long memberIdx );
	bool		ServerDisConn();

public:
	cBillPayletter(void);

	bool		BPInitialize( char* ipAddr="", unsigned short port=5001, unsigned short numWorkerThreads=2, unsigned int bufferLength=65535,
		unsigned short serverNum = 1, unsigned short channelNum = 1 );
	void		Shutdown      ( DWORD maxWait=INFINITE );

	DWORD		BackendThread( );

public:
	virtual ~cBillPayletter(void);

private:
	unsigned long		mUniqueIdx;
	unsigned short		mServerNum;
	unsigned short		mChannelNum;

	bool				mServerAuth;
	unsigned long		mNextServerCheckTime;

public:
	static cBillPayletter* mBillPayletter;
};

#define PGBILLPAYLATTER     cBillPayletter::mBillPayletter

#endif // __BILL_PAYLETTER_H__