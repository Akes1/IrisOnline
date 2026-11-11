// Include
#include <winsock2.h>

#include "sqlpool.h"
#include "iocontextpool.h"

// Local definitions
#pragma warning( disable: 4127 )

// memory를 안전하게 해제
#define SafeDelete(P) if (P!=NULL) { delete(P); (P)=NULL; }

// Global data

// cSQLPool Constructor.
cSQLPool::cSQLPool(void)
{
	// Critical Section를 초기화.
	InitializeCriticalSection( &mCs );

	// SQLEnvironment를 초기화.
	mSqlEnv                 = NULL;

	// SQLConnection를 초기화.
	mPagedPoolUsage         = NULL;
	mNonPagedPoolUsage      = NULL;

	// 메모리 카운터 초기화.
	mQuotaPagedPoolUsage    = 0;
	mQuotaNonPagedPoolUsage = 0;
	mWorkingSetSize         = 0;
	mPeakWorkingSetSize     = 0;

	memset( mDsn, 0, sizeof(mDsn) );
	memset( mUid, 0, sizeof(mUid) );
	memset( mPwd, 0, sizeof(mPwd) );
}

// ~cSQLPool Destructor.
cSQLPool::~cSQLPool(void)
{
	// cSQLPool ThreadPool를 종료한다.
	Shutdown( );

	// Critical Section를 해제.
	DeleteCriticalSection( &mCs );
}

// GetProcessMemoryInfo Method.
void cSQLPool::GetProcessMemoryInfo(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize)
{
	cCSLock lock( &mCs );
	quotaPagedPoolUsage    = mQuotaPagedPoolUsage;
	quotaNonPagedPoolUsage = mQuotaNonPagedPoolUsage;
	workingSetSize         = mWorkingSetSize;
}

// Initialize Method - ThreadPool::Initialize 위치에 주위!
bool cSQLPool::Initialize(char* dsn, char* uid, char* pwd, int numWorkerThreads)
{
	// SQLServer 사용을 위해 ODBC 환경을 설정.
	mSqlEnv  = new cSQLEnvironment( );

	// Audit 복사.
	strcpy( mDsn, dsn );
	strcpy( mUid, uid );
	strcpy( mPwd, pwd );

	// 연결 풀링 활성화.
	if ( mSqlEnv->ConnectionPooling( ) == false )
		return false;

	// ODBC, SQL 사용을 초기화.
	if ( mSqlEnv->AllocEnv( ) == false )
		return false;

	// SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
	if ( mSqlEnv->SetEnvAttr( ) == false )
		return false;

	// 연결 풀링 조건설정.
	if ( mSqlEnv->ConnectionPoolingRelaxedMatch( ) == false )
		return false;

	// ThreadPool Class를 초기화.
	// 3번째 매개 함수값 numWorkerThreads를 CPU * 4로 지정.
	return cThreadPool::Initialize( INVALID_HANDLE_VALUE, 0, numWorkerThreads );
}

// Shutdown Method - ThreadPool::Shutdown 위치에 주위!
void cSQLPool::Shutdown(void)
{
	// ThreadPool Class를 종료.
	cThreadPool::Shutdown( );

	// SQL Context를 해제.
	cCSLock           lock( &mCs );
	PerSQLConnection* temp;

	while ( mPagedPoolUsage != NULL )
	{
		temp               = mPagedPoolUsage;
		mPagedPoolUsage    = mPagedPoolUsage->next;
		FreeSQLConnection( &temp );
	}
	while ( mNonPagedPoolUsage != NULL )
	{
		temp               = mNonPagedPoolUsage;
		mNonPagedPoolUsage = mNonPagedPoolUsage->next;
		FreeSQLConnection( &temp );
	}

	// ODBC, SQL 사용을 종료.
	if ( mSqlEnv != NULL )
	{
		mSqlEnv->FreeEnv( );
	}

	// ODBC, SQL 메모리를 해제.
	SafeDelete( mSqlEnv );
}

// AllocSQLConnection Method
PerSQLConnection* cSQLPool::AllocSQLConnection(void)
{
	PerSQLConnection* perSQLConnection = (PerSQLConnection*)GlobalAlloc( GPTR, sizeof(PerSQLConnection) );

	if ( perSQLConnection != NULL )
	{
		// PerSQLConnection - 초기화
		perSQLConnection->sqlConnection = new cSQLConnection( );  // 1. SQLConnection 생성.
		perSQLConnection->sqlStatement  = new cSQLStatement( );   // 2. SQLStatement  생성.
		perSQLConnection->prev          = NULL;                   // 선형리스트의 포인터 - 이전.
		perSQLConnection->next          = NULL;                   // 선형리스트의 포인터 - 다음.

		// 클래스 생성 확인.
		if ( perSQLConnection->sqlConnection != NULL && perSQLConnection->sqlStatement != NULL )
		{
			// SQLEnvironment 클래스를 참조하여 핸들 생성.
			if ( perSQLConnection->sqlConnection->AllocDbc( mSqlEnv ) == true )
			{
				// DBC 연결.
				if ( perSQLConnection->sqlConnection->Connect( (SQLCHAR*)mDsn, (SQLCHAR*)mUid, (SQLCHAR*)mPwd ) == true )
				{
					// SQLConnection클래스를 참조하여 핸들 생성.
					if ( perSQLConnection->sqlStatement->AllocStmt( perSQLConnection->sqlConnection ) == true )
					{
						// mWorkingSetSize를 증가.
						mWorkingSetSize++;
						return perSQLConnection;
					}
				}
			}
		}
	}

	FreeSQLConnection( &perSQLConnection );
	return perSQLConnection;
}

// FreeSQLConnection Method
void cSQLPool::FreeSQLConnection(PerSQLConnection** perSQLConnection)
{
	if ( perSQLConnection != NULL )
	{
		SafeDelete( (*perSQLConnection)->sqlStatement );  // 1. SQLStatement  해제.
		SafeDelete( (*perSQLConnection)->sqlConnection ); // 2. SQLConnection 해제.

		GlobalFree( (*perSQLConnection) );
		(*perSQLConnection) = NULL;

		// mWorkingSetSize를 감소.
		mWorkingSetSize--;
	}
}

// AttachPool Method - POOL 에 노드 연결.
void cSQLPool::AttachPool(PerSQLConnection** pool, PerSQLConnection* perSQLConnection)
{
	if ( (*pool) != NULL )
	{
		(*pool)->prev = perSQLConnection;

		perSQLConnection->prev = NULL;
		perSQLConnection->next = (*pool);
	}
	(*pool) = perSQLConnection;
}

// DetachPool Method - POOL 에 노드 연결해제.
void cSQLPool::DetachPool(PerSQLConnection** pool, PerSQLConnection* perSQLConnection)
{
	PerSQLConnection* prev = perSQLConnection->prev;
	PerSQLConnection* next = perSQLConnection->next;

	if ( prev == NULL && next == NULL )
	{
		(*pool) = NULL;
	}
	else if ( prev == NULL && next != NULL )
	{
		next->prev = NULL;
		(*pool) = next;
	}
	else if ( prev != NULL && next == NULL )
	{
		prev->next = NULL;
	}
	else if ( prev != NULL && next != NULL )
	{
		prev->next = next;
		next->prev = prev;
	}

	perSQLConnection->prev = NULL;
	perSQLConnection->next = NULL;
}

// GetPool Method
// ReleasePool과 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
PerSQLConnection* cSQLPool::GetPool(void)
{
	cCSLock           lock( &mCs );
	PerSQLConnection* perSQLConnection = mNonPagedPoolUsage;

	if ( perSQLConnection != NULL )
	{
		// 페이지 안된 풀 사용량를 차감.
		DetachPool( &mNonPagedPoolUsage, perSQLConnection );
		mQuotaNonPagedPoolUsage--;

		// SQLEnvironment 클래스를 참조하여 핸들 생성.
		if ( perSQLConnection->sqlConnection->AllocDbc( mSqlEnv ) == false )
		{
			FreeSQLConnection( &perSQLConnection );
		}
		// DBC 연결.
		else if ( perSQLConnection->sqlConnection->Connect( (SQLCHAR*)mDsn, (SQLCHAR*)mUid, (SQLCHAR*)mPwd ) == false )
		{
			FreeSQLConnection( &perSQLConnection );
		}
		// SQLConnection클래스를 참조하여 핸들 생성.
		else if ( perSQLConnection->sqlStatement->AllocStmt( perSQLConnection->sqlConnection ) == false )
		{
			FreeSQLConnection( &perSQLConnection );
		}
	}

	if ( perSQLConnection == NULL )
	{
		// 페이지가 없을 경우 새로 생성.
		perSQLConnection = AllocSQLConnection( );
	}

	if ( perSQLConnection != NULL )
	{
		// 페이지 된 풀 사용량을 증감.
		AttachPool( &mPagedPoolUsage, perSQLConnection );
		mQuotaPagedPoolUsage++;
	}

	return perSQLConnection;
}

// ReleasePool Method
// GetPool과 함께 LIFO를 이룬다. LIFO는 Context Switching를 최소화 하기 위해 사용
void cSQLPool::ReleasePool(PerSQLConnection* perSQLConnection)
{
	cCSLock lock( &mCs );

	// ODBC 정리.
	perSQLConnection->sqlStatement->FreeStmt( );
	perSQLConnection->sqlConnection->Disconnect( );
	perSQLConnection->sqlConnection->FreeDbc( );

	// 페이지 된 풀 사용량을 차감.
	DetachPool( &mPagedPoolUsage, perSQLConnection );
	mQuotaPagedPoolUsage--;

	// 페이지 안된 풀 사용량을 증감.
	AttachPool( &mNonPagedPoolUsage, perSQLConnection );
	mQuotaNonPagedPoolUsage++;
}

// WorkerThread Method
DWORD cSQLPool::WorkerThread(void)
{
	PerSQLConnection* perSQLConnection = NULL;
	PerIoContext*     perIoContext     = NULL;
	BOOL              retValue;

	DWORD             bytesTransfered;
	ULONG_PTR         completionKey;
	OVERLAPPED*       overlapped;

	while ( true )
	{
		// 대기중 I/O 가져오기.
		retValue = GetQueuedCompletionStatus( mRequestQueue, &bytesTransfered, &completionKey, &overlapped, INFINITE );

		// Shutdown.
		if ( overlapped == POOL_SHUTDOWN )
			break;

		// Error - 오류.
		if ( retValue == FALSE || bytesTransfered == 0 ) 
			continue;

		// SQLConnection - 가져오기.
		perSQLConnection = GetPool( );

		// SQLConnection - 연결확인.
		if ( perSQLConnection == NULL )
		{
			// 1(순서에 주위), 지연시간 10/1000(s) - CPU 과부화 방지에 사용.
			Sleep( 10 );

			// 2(순서에 주위), 연결이 안 되었을 경우, 다시 시도.
			QueueRequest( completionKey, overlapped, bytesTransfered );
			continue;
		}

		// OVERLAPPED를 PerIoContext로 케스팅.
		perIoContext = (PerIoContext*)overlapped;

		// 1. SQL를 실행
		// 2. PerIoContext 사용안료.

		// SQLConnection - 내보내기.
		ReleasePool( perSQLConnection );
	}
	return 0L;
}
