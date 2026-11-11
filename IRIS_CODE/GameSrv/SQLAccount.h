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
#include "iocontextpool.h"


#ifndef __SQL_REQUEST_ACCOUNT_TYPE__
#define __SQL_REQUEST_ACCOUNT_TYPE__

enum SQL_REQUEST_ACCOUNT_TYPE
{
	// DEFAULT_SQL_ACCOUNT
	SQL_REQUEST_ACCOUNT_TEST = 0x1000,	// DEFAULT_SQL_ACCOUNT(0x1000) - defined gamesrv.h file
	SQL_REQUEST_ACCOUNT_SHUTDOWN,		// Shutdown Account Database

	SQL_GAME_PROCESS_CHANNEL_CHECK,
	SQL_GAME_PROCESS_MEMBER_CHECK,
	SQL_GAME_PROCESS_MEMBER_LOGOUT,
	SQL_GAME_PROCESS_MEMBER_OUT,
	SQL_GAME_PROCESS_LOGIN_UPDATE,
	SQL_GAME_PROCESS_GAME_TO_GAME,
	SQL_GAME_PROCESS_MEMBER_TWITTER,

    // 자동 사냥 신고 
	SQL_GAME_PROCESS_CAPTCHA_OFFSET,
	SQL_GAME_PROCESS_CAPTCHA_SELECT,
	SQL_GAME_PROCESS_CAPTCHA_RELOAD,

	SQL_GAME_PROCESS_MEMBER_TRIAL_SELECT,
    SQL_GAME_PROCESS_MEMBER_TRIAL_CHECK,
	SQL_GAME_PROCESS_MEMBER_TRIAL_UPDATE,
	SQL_GAME_PROCESS_MEMBER_TRIAL_REJECT_UPDATE,

    SQL_GAME_PROCESS_MEMBER_LOGIN_LIMIT,
    SQL_GAME_PROCESS_MEMBER_OBT_EVENT_COMPLETE,

	// MAX
	MAX_SQL_ACCOUNT
};

#endif // __SQL_REQUEST_ACCOUNT_TYPE__


class cSQLAccount :
	public cSQLPool
{
private:
	cIoContextPool*   mIoContextPool;

protected:
// Page Pool 관리 메소드.
	PerSQLConnection* AllocSQLConnection ( void );

private:
// LOG 관련 메소드.
	bool              PostServerEvent    ( LPCTSTR format, ... );

public:
// 클래스 생성 및 공용 메소드.
	cSQLAccount(void);

	bool              Initialize         ( char* dsn, char* uid, char* pwd, unsigned int numWorkerThreads=2, unsigned int bufferLength=65535 );
	void              Shutdown           ( void );

	void              GetIoContextPool   ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize );

	PerIoContext*     Get                ( );
	void              Release            ( PerIoContext* perIoContext );

	DWORD             WorkerThread       ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLAccount(void);
};

#endif // __SQL_ACCOUNT_H__
