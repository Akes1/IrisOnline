// Include
#include "logdemon.h"

// Local definitions
// memory를 안전하게 해제
#define SafeDelete(P) if (P!=NULL) { delete(P); (P)=NULL; }

// Global data

// cSQLLog Constructor.
cSQLLog::cSQLLog(void)
{
}

// ~cSQLLog Destructor.
cSQLLog::~cSQLLog(void)
{
}

// Initialize Method
bool cSQLLog::Initialize(char* dsn, char* uid, char* pwd, int numWorkerThreads)
{
	return cSQLPool::Initialize( dsn, uid, pwd, numWorkerThreads );
}

// AllocSQLConnection Method
PerSQLConnection* cSQLLog::AllocSQLConnection(void)
{
	PerSQLConnection* perSQLConnection = (PerSQLConnection*)GlobalAlloc( GPTR, sizeof(PerSQLConnection) );

	if ( perSQLConnection != NULL )
	{
		// PerSQLConnection - 초기화.
		perSQLConnection->sqlConnection = new cSQLConnection( );  // 1. SQLConnection 생성.
		perSQLConnection->sqlStatement  = new cSQLLogStmt( );     // 2. cSQLLogStmt   생성.
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

// WorkerThread Method
DWORD cSQLLog::WorkerThread(void)
{
	PerSQLConnection* perSQLConnection = NULL;
	cSQLLogStmt*      sqlLogStmt       = NULL;
	SQLRETURN         sqlReturn;

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
			// 1(순서에 주위), 지연시간 50/1000(s) - CPU 과부화 방지에 사용.
			Sleep( 50 );

			// 2(순서에 주위), 연결이 안 되었을 경우, 다시 시도.
			QueueRequest( completionKey, overlapped, bytesTransfered );
			continue;
		}

		// cSQLLogStmt를 SQLAccountStmt로 캐스팅.
		sqlLogStmt = (cSQLLogStmt*)perSQLConnection->sqlStatement;

		// OVERLAPPED를 PerIoContext로 케스팅.
		perIoContext = (PerIoContext*)overlapped;

		// 1. SQL를 실행
		switch ( perIoContext->iParam )
		{
		case SQL_LOG_SERVER_EVENT:
			sqlReturn = sqlLogStmt->ServerEvent( (SERVER_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_MEMBER_EVENT:
			sqlReturn = sqlLogStmt->MemberEvent( (MEMBER_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_CHARACTER_EVENT:
			sqlReturn = sqlLogStmt->CharacterEvent( (CHARACTER_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_MONEY_EVENT:
			sqlReturn = sqlLogStmt->MoneyEvent( (MONEY_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_DEPOSIT_EVENT:
			sqlReturn = sqlLogStmt->DepositEvent( (DEPOSIT_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_INVENTORY_EVENT:
			sqlReturn = sqlLogStmt->InventoryEvent( (INVENTORY_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_CONCURRENT_EVENT:
			sqlReturn = sqlLogStmt->ConcurrentEvent( (CONCURRENT_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_QUEST_EVENT:
			sqlReturn = sqlLogStmt->QuestEvent( (QUEST_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_GUILD_EVENT:
			sqlReturn = sqlLogStmt->GuildEvent( (GUILD_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_DROPITEM_EVENT:
			sqlReturn = sqlLogStmt->DropItemEvent( (DROPITEM_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_SKILL_EVENT:
			sqlReturn = sqlLogStmt->SkillEvent( (SKILL_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_ITEM_BILL_EVENT:
			sqlReturn = sqlLogStmt->ItemBillEvent( (ITEM_BILL_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_INFCASH_EVENT:
			sqlReturn = sqlLogStmt->InfCashEvent( (MB_SYN_INFCASH_SAVE*)perIoContext->buffer );
			break;
		case SQL_LOG_RESERVED_EVENT:
			sqlReturn = sqlLogStmt->ReservedEvent( (RESERVED_EVENT*)perIoContext->buffer );
			break;
		case SQL_LOG_POST_EVENT:
			sqlReturn = sqlLogStmt->PostEvent( (POST_EVENT*)perIoContext->buffer );
			break;
		}

		// 2. 이벤트 - 오류만 기록.
		if ( !SQL_SUCCEEDED( sqlReturn ) )
		{
			// 파일기록.
			SYSTEMTIME st;
			char       buf[MAX_PATH];

			GetLocalTime( &st );
			sprintf( buf, "L%04d%02d%02d.log", st.wYear, st.wMonth, st.wDay );

			FILE* stream = fopen( buf, "at" );
			if ( stream != NULL )
			{
				if ( sqlReturn != SQL_NO_DATA )
				{
					SQLHSTMT    hstmt = sqlLogStmt->GetSqlStmt( );
					SQLCHAR     sqlstate[ SQL_SQLSTATE_SIZE + 1 ]     = "\0";
					SQLINTEGER  nativeErrorPtr                        = 0;
					SQLCHAR     messageText[ SQL_MAX_MESSAGE_LENGTH ] = "\0";
					SQLSMALLINT bufferLength                          = SQL_MAX_MESSAGE_LENGTH;
					SQLSMALLINT textLength                            = 0;
					SQLRETURN   result;

					result = SQLGetDiagRec( SQL_HANDLE_STMT,
											hstmt,
											1,					// [IN ] Status records are numbered from 1.
											sqlstate,			// [OUT] SQL Error State string
											&nativeErrorPtr,	// [OUT] Native Error code
											messageText,		// [OUT] SQL Error Text string
											bufferLength,		// [IN ] Length of the *MessageText buffer in characters.
											&textLength );

					fprintf( stream,
							 "%04d-%02d-%02d %02d:%02d:%02d cSQLLog::lParam[%d]:Returns[%d] - SQL Error State string[%s] - Native Error code[%d] - SQL Error Text string[%s]\n",
							 st.wYear,
							 st.wMonth,
							 st.wDay,
							 st.wHour,
							 st.wMinute,
							 st.wSecond,
							 perIoContext->iParam,
							 sqlReturn,
							 sqlstate,
							 nativeErrorPtr,
							 messageText );
				}
				else
				{
					fprintf( stream,
							 "%04d-%02d-%02d %02d:%02d:%02d cSQLLog::lParam[%d]:Returns[%d]\n",
							 st.wYear,
							 st.wMonth,
							 st.wDay,
							 st.wHour,
							 st.wMinute,
							 st.wSecond,
							 perIoContext->iParam,
							 sqlReturn );
				}
				fclose( stream );
			}
		}

		// 3. PerIoContext 사용완료 - 결과는 콜백으로 되돌려준다.
		// perIoContext->requestType   = IOCP_REQUEST_CALLBACK;
		// perIoContext->requestResult = IOCP_REQUEST_SUCCESS;
		cLogRecver* logRecver = g_logDemon->GetLogRecver( );
		if ( logRecver != NULL )
		{
			perIoContext->requestResult = (SQL_SUCCEEDED( sqlReturn ) ? IOCP_REQUEST_SUCCESS : IOCP_REQUEST_ERROR );
			logRecver->QueueRequest( completionKey, (OVERLAPPED*)perIoContext, perIoContext->offset );
		}

		// SQLConnection - 내보내기.
		ReleasePool( perSQLConnection );
	}
	return 0L;
}
