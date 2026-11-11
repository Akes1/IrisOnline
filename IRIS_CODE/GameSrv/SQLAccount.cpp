// Include
#include "gamesrv.h"

// Local definitions
// memory를 안전하게 해제
#define SafeDelete(P) if (P!=NULL) { delete(P); (P)=NULL; }

#define MAX_QUERY_LOG	30000

// Global data

// cSQLAccount Constructor.
cSQLAccount::cSQLAccount(void) : mIoContextPool(NULL)
{
}

// ~cSQLAccount Destructor.
cSQLAccount::~cSQLAccount(void)
{
}

// Initialize Method
bool cSQLAccount::Initialize(char* dsn, char* uid, char* pwd, unsigned int numWorkerThreads, unsigned int bufferLength)
{
	mIoContextPool = new cIoContextPool( bufferLength );
	return cSQLPool::Initialize( dsn, uid, pwd, numWorkerThreads );
}

// Shutdown Method
void cSQLAccount::Shutdown(void)
{
	// SQL 종료.
	PostServerEvent( "cSQLAccount - Have received a request from the manager to stop the game server is shutting down." );

	// 메모리 사용량
	SIZE_T quotaPagedPoolUsage;
	SIZE_T quotaNonePagedPoolUsage;
	SIZE_T workingSetSize;

	mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
	PostServerEvent( "cSQLAccount - IoContextPool Working Set Size (=%d), Quota Paged Pool Usage (=%d), Quota None Paged Pool Usage (=%d)."
					,workingSetSize
					,quotaPagedPoolUsage
					,quotaNonePagedPoolUsage );

	cSQLPool::Shutdown( );
	SafeDelete( mIoContextPool );

	Sleep( 100 );
	PostServerEvent( "cSQLAccount - Shutdown has completed." );
}

// AllocSQLConnection Method
PerSQLConnection* cSQLAccount::AllocSQLConnection(void)
{
	PerSQLConnection* perSQLConnection = (PerSQLConnection*)GlobalAlloc( GPTR, sizeof(PerSQLConnection) );

	if ( perSQLConnection != NULL )
	{
		perSQLConnection->sqlConnection = new cSQLConnection( );  // 1. SQLConnection  생성.
		perSQLConnection->sqlStatement  = new cSQLAccountStmt( ); // 2. cSQLAccountStmt 생성.
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

// PostServerEvent Method
bool cSQLAccount::PostServerEvent(LPCTSTR format, ...)
{
	cSender* sender   = g_gameSrv->GetSender( );
	bool     retvalue = false;

	if ( sender != NULL )
	{
		LPVOID  msgBuf = NULL;
		DWORD   bufferLength;

		va_list args;

		va_start( args, format );

		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );

		vsprintf( (char*)msgBuf, format, args );

		va_end( args );

		if ( msgBuf != NULL )
		{
			retvalue = sender->PostServerEvent( (char*)msgBuf );
			free( msgBuf );
		}
	}
	return retvalue;
}

// GetIoContextPool Method
void cSQLAccount::GetIoContextPool(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize)
{
	mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );
}

// Get Method
PerIoContext* cSQLAccount::Get( )
{
	return mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
}

// Release Method
void cSQLAccount::Release(PerIoContext* perIoContext)
{
	DWORD error = 0;
	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cSQLAccount::Release:Error(=0x%08xh)", error );
}

// WorkerThread Method
DWORD cSQLAccount::WorkerThread(void)
{
	PerSQLConnection* perSQLConnection = NULL;
	cSQLAccountStmt*  sqlAccountStmt   = NULL;
	SQLRETURN         sqlReturn;

	PerIoContext*     perIoContext     = NULL;
	DWORD             delayTick;
	BOOL              retValue;

	DWORD             bytesTransfered;
	ULONG_PTR         completionKey;
	OVERLAPPED*       overlapped;

	// [warning C4127 조건식이 상수].
	bool boolean = true;
	while ( boolean )
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

		// SQLStatement를 cSQLAccountStmt로 캐스팅.
		sqlAccountStmt = (cSQLAccountStmt*)perSQLConnection->sqlStatement;

		// OVERLAPPED를 PerIoContext로 케스팅.
		perIoContext = (PerIoContext*)overlapped;
		perIoContext->lParam = GetTickCount( );

		// 1. SQL를 실행
		switch ( perIoContext->iParam )
		{
		case SQL_REQUEST_ACCOUNT_SHUTDOWN:
			sqlReturn = sqlAccountStmt->Shutdown( (SHUTDOWN_ACCOUNT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHANNEL_CHECK:
			sqlReturn = sqlAccountStmt->ChannelCheck( (CHANNEL_CHECK*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_MEMBER_CHECK:
			sqlReturn = sqlAccountStmt->MemberCheck( (MEMBER_CHECK*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_MEMBER_LOGOUT:
			sqlReturn = sqlAccountStmt->MemberLogout( (MEMBER_LOGOUT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_MEMBER_OUT:
			sqlReturn = sqlAccountStmt->MemberOut( (MEMBER_OUT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_LOGIN_UPDATE:
			sqlReturn = sqlAccountStmt->LoginUpdate( (LOGIN_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GAME_TO_GAME:
			sqlReturn = sqlAccountStmt->GameToGame( (GAME_TO_GAME*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_MEMBER_TWITTER:
			sqlReturn = sqlAccountStmt->MemberTwitter( (MEMBER_TWITTER*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_CAPTCHA_OFFSET:
			sqlReturn = sqlAccountStmt->CaptchaOffset( (CAPTCHA_OFFSET*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_CAPTCHA_SELECT:
			sqlReturn = sqlAccountStmt->CaptchaSelect( (CAPTCHA_SELECT*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_CAPTCHA_RELOAD:
			sqlReturn = sqlAccountStmt->CaptchaReload( (CAPTCHA_RELOAD*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_SELECT:
			sqlReturn = sqlAccountStmt->TrialSelect( (TRIAL_SELECT*)perIoContext->buffer );
            break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_CHECK:
			sqlReturn = sqlAccountStmt->TrialCheck( (MEMBER_TRIAL*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_UPDATE:
			sqlReturn = sqlAccountStmt->TrialUpdate( (MEMBER_TRIAL*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_REJECT_UPDATE:
			sqlReturn = sqlAccountStmt->TrialRejectUpdate( (TRIAL_REJECT*)perIoContext->buffer );
            break;
        case SQL_GAME_PROCESS_MEMBER_LOGIN_LIMIT:
			sqlReturn = sqlAccountStmt->LoginLimit( (LOGIN_LIMIT*)perIoContext->buffer );
            break;
		case SQL_GAME_PROCESS_MEMBER_OBT_EVENT_COMPLETE:
			sqlReturn = sqlAccountStmt->ObtEventComplete( (MEMBER_OBT_EVENT_COMPLETE*)perIoContext->buffer );
            break;
		default:
			sqlReturn = SQL_ERROR;
			break;
		}

		// Query 지연시간 기록.
		delayTick = GetTickCount( ) - (DWORD)perIoContext->lParam;
		if ( delayTick > MAX_QUERY_LOG )
			PostServerEvent( "QUERY - cSQLAccount::iParam(=%d) Delay Time(=%d)", perIoContext->iParam, delayTick );

		// 2. 이벤트 - 오류만 기록.
		if ( !SQL_SUCCEEDED( sqlReturn ) )
		{
			if ( sqlReturn != SQL_NO_DATA )
			{
				SQLHSTMT    hstmt = sqlAccountStmt->GetSqlStmt( );
				SQLCHAR     sqlstate[ SQL_SQLSTATE_SIZE + 1 ]     = "\0";
				SQLINTEGER  nativeErrorPtr                        = 0;;
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

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( hstmt ) == SQL_SUCCESS );

				// 열려있는 결과 닫기. (Close the open result set.)
				SQLCloseCursor( hstmt );

				// 로그전송.
				PostServerEvent( "cSQLAccount::IParam[%d]:Returns[%d] - SQL Error State string[%s] Native Error code[%d] SQL Error Text string[%s]",
								 perIoContext->iParam,
								 sqlReturn,
								 sqlstate,
								 nativeErrorPtr,
								 messageText );
			}
			else
			{
				// 로그전송.
				PostServerEvent( "cSQLAccount::IParam[%d]:Returns[%d]", perIoContext->iParam, sqlReturn );
			}
		}

		// 3. PerIoContext 사용완료 - 결과는 콜백으로 되돌려준다.
		// perIoContext->requestType   = IOCP_REQUEST_CALLBACK;
		// perIoContext->requestResult = IOCP_REQUEST_SUCCESS;
		cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );
		if ( gameProcess != NULL )
		{
			perIoContext->requestResult = (sqlReturn == SQL_SUCCESS || sqlReturn == SQL_SUCCESS_WITH_INFO) ? IOCP_REQUEST_SUCCESS : IOCP_REQUEST_ERROR;
			gameProcess->QueueRequest( completionKey, (OVERLAPPED*)perIoContext, perIoContext->offset );
		}

		// SQLConnection - 내보내기.
		ReleasePool( perSQLConnection );
	}
	return 0L;
}
