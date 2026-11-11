/* ====================================================================
 * 파    일 :	sqlLog.h
 * 목    적 :	전송받은 데이터를 MSSQLServer에 기록
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_LOG_H__
#define __SQL_LOG_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "sqlpool.h"
#include "sqllogstmt.h"


#ifndef __SQL_REQUEST_LOG_TYPE__
#define __SQL_REQUEST_LOG_TYPE__

enum SQL_REQUEST_LOG_TYPE
{
	SQL_LOG_SERVER_EVENT = 0x1000,			// DEFAULT_SQL_LOG(0x1000) - defined logdemon.h file
	SQL_LOG_MEMBER_EVENT,
	SQL_LOG_CHARACTER_EVENT,
	SQL_LOG_MONEY_EVENT,
	SQL_LOG_DEPOSIT_EVENT,
	SQL_LOG_INVENTORY_EVENT,
	SQL_LOG_CONCURRENT_EVENT,
	SQL_LOG_QUEST_EVENT,
	SQL_LOG_GUILD_EVENT,
	SQL_LOG_DROPITEM_EVENT,
	SQL_LOG_SKILL_EVENT,
	SQL_LOG_ITEM_BILL_EVENT,
	SQL_LOG_INFCASH_EVENT,
	SQL_LOG_RESERVED_EVENT,
	SQL_LOG_POST_EVENT,
};

#endif // __SQL_REQUEST_LOG_TYPE__


class cSQLLog :
	public cSQLPool
{
protected:
// Page Pool 관리 메소드.
	PerSQLConnection* AllocSQLConnection ( void );

public:
// 클래스 생성 및 공용 메소드.
	cSQLLog(void);

	bool              Initialize         ( char* dsn, char* uid, char* pwd, int numWorkerThreads=2 );
	DWORD             WorkerThread       ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLLog(void);
};

#endif // __SQL_LOG_H__
