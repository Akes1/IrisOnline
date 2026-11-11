/* ====================================================================
 * 파    일 :	sqlWeb.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_WEB_H__
#define __SQL_WEB_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "sqlpool.h"
#include "sqlwebstmt.h"

#ifndef __SQL_REQUEST_WEB_TYPE__
#define __SQL_REQUEST_WEB_TYPE__

enum SQL_REQUEST_WEB_TYPE
{
	MIN_SQL_WEB_PROC = 0x1000, // DEFAULT_SQL_WEB(0x1000) - defined loginsrv.h file
		SQL_WEB_PROC_LAUNCHER,
		SQL_WEB_PROC_CHECK,
		SQL_WEB_PROC_LOGIN,
		SQL_WEB_PROC_TP_SMS,
	MAX_SQL_WEB_PROC,
};

#endif // __SQL_REQUEST_WEB_TYPE__


class cSQLWeb :
	public cSQLPool
{
protected:
// Page Pool 관리 메소드.
	PerSQLConnection* AllocSQLConnection ( void );

private:
// LOG 관련 메소드.
	bool              PostServerEvent    ( LPCTSTR format, ... );

public:
// 클래스 생성 및 공용 메소드.
	cSQLWeb(void);

	bool              Initialize         ( char* dns, char* uid, char* pwd, int numWorkerThreads=2 );
	DWORD             WorkerThread       ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLWeb(void);
};

#endif // __SQL_WEB_H__