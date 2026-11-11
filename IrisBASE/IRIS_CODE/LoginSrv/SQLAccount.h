/* ====================================================================
 * 파    일 :	sqlAccount.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_ACCOUNT_H__
#define __SQL_ACCOUNT_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "sqlpool.h"
#include "sqlaccountstmt.h"

#ifndef __SQL_REQUEST_ACCOUNT_TYPE__
#define __SQL_REQUEST_ACCOUNT_TYPE__

enum SQL_REQUEST_ACCOUNT_TYPE
{
	MIN_SQL_SERVER_PROC = 0x2000, // DEFAULT_SQL_ACCOUNT(0x2000) - defined loginsrv.h file
		SQL_SERVER_PROC_MATRIX_CREATE,
		SQL_SERVER_PROC_SERVER_LIST,
		SQL_SERVER_PROC_CHANNEL_UPDATE,
		SQL_SERVER_PROC_CONCURRENT_USER,
		SQL_SERVER_PROC_SERVER_DOWN,
	MAX_SQL_SERVER_PROC,

	MIN_SQL_CLIENT_PROC,
		SQL_CLIENT_PROC_MEMBER_LOGIN,
		SQL_CLIENT_PROC_MEMBER_LOGOUT,
		SQL_CLIENT_PROC_LOGIN_UPDATE,
		SQL_CLIENT_PROC_GAME_TO_GAME,
	MAX_SQL_CLIENT_PROC,
};

#endif // __SQL_REQUEST_ACCOUNT_TYPE__


class cSQLAccount :
	public cSQLPool
{
protected:
// Page Pool 관리 메소드.
	PerSQLConnection* AllocSQLConnection ( void );

private:
// LOG 관련 메소드.
	bool              PostServerEvent    ( LPCTSTR format, ... );

public:
// InsertCaptcha 메소드(Script).
	bool              InsertCaptcha      ( char* dsn, char* uid, char* pwd );

public:
// 클래스 생성 및 공용 메소드.
	cSQLAccount(void);

	bool              Initialize         ( char* dns, char* uid, char* pwd, int numWorkerThreads=2 );
	DWORD             WorkerThread       ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLAccount(void);
};

#endif // __SQL_ACCOUNT_H__