/* ====================================================================
 * 파    일 :	AgentProcess.h
 * 목    적 :
 * 작 성 자 :	곽철중
 * 작 성 일 :	09/07/09
 * 주의사항 :	
 * =================================================================== */
#ifndef __AGENT_PROCESS_H__
#define __AGENT_PROCESS_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once
#include "iocpserver.h"

#include "protocol.h"
#include "serverProtocol.h"

#include "commonnetworkmsgstruct.h"


enum eSERVER_TYPE
{
	SRV_TYPE_AGENT = 0,
	SRV_TYPE_LOGIN,
	SRV_TYPE_THEMA,
    SRV_TYPE_PVE,
};

/// 서버 정보
struct sServiceCtr
{
    DWORD           status;
    char            serviceName[MAX_PATH];
    char            ip[50];
};


class AgentProcess :
	public cIocpServer
{

private:

    typedef tPointerArray<void*> PArray;
    PArray  mSrvList;


private:
    // 서비스 관련 메소드
	void  ErrorLook             ( char* msg, DWORD lastError );
	bool  DoStartSvc            ( char* serviceName );
	bool  DoStopSvc             ( char* serviceName );
	bool  StopDependentServices ( SC_HANDLE hManager, SC_HANDLE hService );

private:
	bool PostServerEvent   ( LPCTSTR format, ... );

private:
	// 서버관련 메소드 - 보조.
	bool BatchComplete     ( PerSocketContext* socketContext, void* ptr );

private:
	// 서버관련 메소드 - 주.
	bool RecvComplete      ( PerSocketContext* socketContext, PerIoContext* ioContext, DWORD bytesTransferred );
	bool CallbackComplete  ( PerSocketContext* socketContext, PerIoContext* ioContext, DWORD bytesTransferred );

public:
	// 클래스 생성 및 공용 메소드.
	AgentProcess(void);

	bool Initialize         ( char* addr="", unsigned short port=10000, unsigned short numWorkerThread=2 );
	void Shutdown           ( DWORD maxWait=INFINITE );

	DWORD BackendThread     ( );
	void  BackendThreadFps  ( DWORD currentTick );

	MSGROOT*    GetMsgRoot  ( void** handle, PerSocketContext* perSocketContext, char category=0, char protocol=0 );
	bool        SendMsgRoot ( void* handle, unsigned long length );

    void  InitSrvList       ( );
    void  AddServieCtrl     ( const char* name, const char* ip );
    void  UpdateServiceCtrl ( sServiceCtr* pSrv );

public:
	// 클래스 파괴 메소드.
	virtual ~AgentProcess(void);

};

#endif // __AGENT_PROCESS_H__