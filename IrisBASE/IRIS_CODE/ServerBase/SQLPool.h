/* ====================================================================
 * 파    일 :	sqlPool.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/10
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_POOL_H__
#define __SQL_POOL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "odbcsql.h"
#include "threadpool.h"
#include "criticalsectionlock.h"


#ifndef PER_SQL_CONNECTION
#define PER_SQL_CONNECTION

struct PerSQLConnection
{
	cSQLConnection*          sqlConnection;     // SQL Connection
	cSQLStatement*           sqlStatement;      // SQL Statement
	struct PerSQLConnection* next;              // Dual Linked List 참조 포인터
	struct PerSQLConnection* prev;              // Dual Linked List 참조 포인터
};

#endif // PER_SQL_CONNECTION


class cSQLPool :
	public cThreadPool
{
protected:
// 환경변수.
	CRITICAL_SECTION          mCs;
	cSQLEnvironment*          mSqlEnv;

	PerSQLConnection*         mPagedPoolUsage;         // 페이지 된 풀
	PerSQLConnection*         mNonPagedPoolUsage;      // 페이지 안된 풀
	SIZE_T                    mQuotaPagedPoolUsage;    // 현재 페이지 된 풀   - 사용량
	SIZE_T                    mQuotaNonPagedPoolUsage; // 현재 페이지 안된 풀 - 사용량
	SIZE_T                    mWorkingSetSize;         // 현재 작업 설정크기.
	SIZE_T                    mPeakWorkingSetSize;     // 최대 작업 설정크기.

	char                      mDsn[MAX_PATH];          // DataSource
	char                      mUid[MAX_PATH];          // UserID
	char                      mPwd[MAX_PATH];          // Password

protected:
// Page Pool 관리 메소드.
	virtual PerSQLConnection* AllocSQLConnection   ( void );
	virtual void              FreeSQLConnection    ( PerSQLConnection** perSQLConnection );

	void                      AttachPool           ( PerSQLConnection** pool, PerSQLConnection* perSQLConnection );
	void                      DetachPool           ( PerSQLConnection** pool, PerSQLConnection* perSQLConnection );

	PerSQLConnection*         GetPool              ( void );
	void                      ReleasePool          ( PerSQLConnection* perSQLConnection );

public:
// 클래스 생성 및 공용 메소드.
	cSQLPool(void);

	virtual void              GetProcessMemoryInfo ( SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize );

	virtual bool              Initialize           ( char* dsn, char* uid, char* pwd, int numWorkerThreads=2 );
	virtual void              Shutdown             ( void );

	virtual DWORD             WorkerThread         ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLPool(void);
};

#endif // __SQL_POOL_H__