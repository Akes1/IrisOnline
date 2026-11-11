/* ====================================================================
 * 파    일 :	IOCPCLient.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/03/20
 * 수 정 일 :	08/04/23 - cOverlappedResult에서 cIOCPClient로 변경 및
                           내용 수정.
 * 주의사항 :	
 * =================================================================== */
#ifndef __IOCP_CLIENT_H__
#define __IOCP_CLIENT_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#ifndef CLIENT_VER_VER
#define CLIENT_VER_VER 0x0001
#else
#if defined(CLIENT_VER_VER) && (CLIENT_VER_VER < 0x0001)
#error CLIENT_VER_VER setting conflicts
#endif
#endif

#pragma once

#include <winsock2.h>
#include <windows.h>
#include <lmerr.h>

#include "iocontextpool.h"
#include "rc5.h"

#define	WM_SOCKEVENTMSG2	(WM_USER+0x10)

enum IOCP_CLIENT_TYPE
{
	ICT_CONNECT = 1,
	ICT_DISCONNECT,
	ICT_CONNECTION_DEAD,

	ICT_RECV,
};

#define ICT_SUCCESS 0
#define ICT_FAIL    1

#define IOCP_MAX_WORKER_THREAD  16
#define IOCP_SHUTDOWN           ((OVERLAPPED*)((__int64)-1))

enum LOGIN_CLIENT_REQUEST_TYPE
{
	/// DEFAULT_LOGIN_CLIENT
	LOGIN_CLIENT_CONNECT = 0x8000,
};

class cIOCPClient
{
protected: // 환경변수.
	CRITICAL_SECTION    mCs;

	HWND                mWnd;
	DWORD               mMessageID;

	HANDLE              mIocp;
	SOCKET              mSocket;
	SOCKADDR_IN         mAddr;
	unsigned short      mPort;
	unsigned long       mSeq;

	int                 mIocpWorkerThreadNumber;
	HANDLE              mIocpWorkerThread[IOCP_MAX_WORKER_THREAD];

	cIoContextPool*     mIoContextPool;

	IoContextBuffer*    mIoContextFrontBuffer;
	IoContextBuffer*    mIoContextBackBuffer;

	HANDLE              mConnectThread;
	bool                mConnected;
	bool                mConnectionResult;
	int                 mConnectRetry;

	WORD                mVersionRequested;
	WSADATA             mWsaData;

	bool                mRC5Enable;

	char                mLastTos;
	char                mLastCategory;
	char                mLastProtocol;

	// 웹런쳐 인증코드 send
	bool				mWepLuncherInit;

protected: // 주요 메소드.
	virtual void        Decrypt            ( BYTE* ptr, RC5_32_KEY key );
	virtual void        Encrypt            ( BYTE* ptr, RC5_32_KEY key );

	virtual void        Decrypt            ( BYTE key, long len, char* ptr );
	virtual void        Encrypt            ( BYTE key, long len, char* ptr );

protected: // 주요 메소드.
	virtual bool        SendPost           ( PerIoContext* perIoContext );
	virtual bool        RecvPost           ( PerIoContext* perIoContext );

	virtual bool        SendComplete       ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );
	virtual bool        RecvComplete       ( ULONG_PTR completionKey, PerIoContext* perIoContext, DWORD bytesTransferred );

	virtual void        Close              ( ULONG_PTR completionKey, PerIoContext* perIoContext );

public: // 클래스 생성 및 공용 메소드.
	cIOCPClient(HWND wnd, DWORD messageID);

	virtual bool       Initialize          ( unsigned short numWorkerThreads=2 );
	virtual void       Shutdown            ( DWORD maxWait=INFINITE );

	virtual bool       PostConnect         ( char* addr, unsigned short port );
	virtual bool       PostConnect         ( unsigned long ipv4, unsigned short port );
	virtual bool       PostConnect         ( );
	virtual bool       GetConnectionResult ( void );

	virtual bool       IsConnected         ( void ) { return mConnectionResult; }
	virtual bool       Disconnect          ( void );

	virtual bool       SendNetworkMsg      ( char* buf, u_short len );
	virtual bool       SendHackShieldMsg   ( char* buf, u_short len );
	virtual bool       ReleaseNetworkMsg   ( PerIoContext* perIoContext );

	virtual bool       SendHeartbeat       ( );

	virtual bool       Process             ( WPARAM wParam, LPARAM lParam );

	virtual DWORD      ConnectThread       ( void );
	virtual DWORD      WorkerThread        ( void );

	bool			   GetWepLuncherInit   ( void ) { return mWepLuncherInit; }

public: // 클래스 파괴 메소드.
	virtual ~cIOCPClient(void);

public:
	static cIOCPClient* mIOCPClient;

public: // 쓰레드 시작 포인터.
	static DWORD WINAPI ConnectThreadStartingPoint ( void* ptr );
	static DWORD WINAPI WorkerThreadStartingPoint  ( void* ptr );
};

#define NETWORK	cIOCPClient::mIOCPClient

#endif // __IOCP_CLIENT_H__
