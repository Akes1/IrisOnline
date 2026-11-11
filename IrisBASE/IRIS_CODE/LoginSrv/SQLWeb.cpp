// Include
#include "loginsrv.h"

// Local definitions
// memory를 안전하게 해제
#define SafeDelete(P) if (P!=NULL) { delete(P); (P)=NULL; }

// Global data

// cSQLWeb Constructor.
cSQLWeb::cSQLWeb(void)
{
}

// ~cSQLWeb Destructor.
cSQLWeb::~cSQLWeb(void)
{
}

// Initialize Method
bool cSQLWeb::Initialize(char* dns, char* uid, char* pwd, int numWorkerThreads)
{
	return cSQLPool::Initialize( dns, uid, pwd, numWorkerThreads );
}

// AllocSQLConnection Method
PerSQLConnection* cSQLWeb::AllocSQLConnection(void)
{
	PerSQLConnection* perSQLConnection = (PerSQLConnection*)GlobalAlloc( GPTR, sizeof(PerSQLConnection) );

	if ( perSQLConnection != NULL )
	{
		// PerSQLConnection - 초기화
		perSQLConnection->sqlConnection = new cSQLConnection( );  // 1. SQLConnection 생성.
		perSQLConnection->sqlStatement  = new cSQLWebStmt( );     // 2. cSQLWebStmt   생성.
		perSQLConnection->prev          = NULL;                   // 3. 선형리스트의 포인터 - 이전.
		perSQLConnection->next          = NULL;                   // 3. 선형리스트의 포인터 - 다음.

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
bool cSQLWeb::PostServerEvent(LPCTSTR format, ...)
{
	cSender* sender   = g_loginSrv->GetSender( );
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

// WorkerThread Method
DWORD cSQLWeb::WorkerThread(void)
{
	PerSQLConnection* perSQLConnection = NULL;
	cSQLWebStmt*      sqlWebStmt       = NULL;
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
			// 1(순서에 주위), 지연시간 10/1000(s) - CPU 과부화 방지에 사용.
			Sleep( 10 );

			// 2(순서에 주위), 연결이 안 되었을 경우, 다시 시도.
			QueueRequest( completionKey, overlapped, bytesTransfered );
			continue;
		}

		// SQLStatement를 cSQLWebStmt로 캐스팅.
		sqlWebStmt = (cSQLWebStmt*)perSQLConnection->sqlStatement;

		// OVERLAPPED를 PerIoContext로 케스팅.
		perIoContext   = (PerIoContext*)overlapped;

		// 1. SQL를 실행
		// 2. PerIoContext 사용완료 - 결과는 콜백으로 되돌려준다.
		//	  perIoContext->requestType = IOCP_REQUEST_CALLBACK;
		switch ( perIoContext->iParam )
		{
		case SQL_WEB_PROC_LAUNCHER:
			sqlReturn = sqlWebStmt->WebLauncher( (WEB_LAUNCHER*)perIoContext->buffer );
			break;
		case SQL_WEB_PROC_CHECK:
			sqlReturn = sqlWebStmt->WebCheck( (WEB_LOGIN*)perIoContext->buffer );
			break;
		case SQL_WEB_PROC_LOGIN:
			sqlReturn = sqlWebStmt->WebLogin( (WEB_LOGIN*)perIoContext->buffer );
			break;
		case SQL_WEB_PROC_TP_SMS:
			sqlReturn = sqlWebStmt->WebTpSMS( (WEB_TP_SMS*)perIoContext->buffer );
			break;
		default:
			sqlReturn = SQL_ERROR;
			break;
		}

		// 2. 이벤트 - 오류만 기록한다.
		if ( !SQL_SUCCEEDED( sqlReturn ) )
		{
			if ( sqlReturn != SQL_NO_DATA )
			{
				SQLHSTMT    hstmt = sqlWebStmt->GetSqlStmt( );
				SQLCHAR     sqlstate[ SQL_SQLSTATE_SIZE + 1 ]     = "\0";
				SQLINTEGER  nativeErrorPtr                        = 0;
				SQLCHAR     messageText[ SQL_MAX_MESSAGE_LENGTH ] = "\0";
				SQLSMALLINT bufferLength                          = SQL_MAX_MESSAGE_LENGTH;
				SQLSMALLINT textLength                            = 0;
				SQLRETURN   result;

				SDWORD       ss_msgstate = 0;
				SDWORD       ss_severity = 0;
				SQLINTEGER   rownumber   = 0;
				USHORT       ss_line;
				SQLCHAR      ss_procname[ MAXNAME ];
				SQLCHAR      ss_srvname[ MAXNAME ];
				SQLSMALLINT  cb_ss_procname;
				SQLSMALLINT  cb_ss_srvname;

				result = SQLGetDiagRec( SQL_HANDLE_STMT,
										hstmt,
										1,					// [IN ] Status records are numbered from 1.
										sqlstate,			// [OUT] SQL Error State string
										&nativeErrorPtr,	// [OUT] Native Error code
										messageText,		// [OUT] SQL Error Text string
										bufferLength,		// [IN ] Length of the *MessageText buffer in characters.
										&textLength );

				if ( result != SQL_NO_DATA_FOUND )
				{
					result = SQLGetDiagField( SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_ROW_NUMBER,  &rownumber,   SQL_IS_INTEGER, NULL );
					result = SQLGetDiagField( SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SS_LINE,     &ss_line,     SQL_IS_INTEGER, NULL );
					result = SQLGetDiagField( SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SS_MSGSTATE, &ss_msgstate, SQL_IS_INTEGER, NULL );
					result = SQLGetDiagField( SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SS_SEVERITY, &ss_severity, SQL_IS_INTEGER, NULL );
					result = SQLGetDiagField( SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SS_PROCNAME, &ss_procname, sizeof(ss_procname), &cb_ss_procname );
					result = SQLGetDiagField( SQL_HANDLE_STMT, hstmt, 1, SQL_DIAG_SS_SRVNAME,  &ss_srvname,  sizeof(ss_srvname),  &cb_ss_srvname  );
				}

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( hstmt ) == SQL_SUCCESS );

				// 열려있는 결과 닫기. (Close the open result set.)
				SQLCloseCursor( hstmt );

				// 로그전송.
				PostServerEvent( "cSQLWeb::IParam[%d]:Returns[%d] - SQL Error State string[%s] Native Error code[%d] SQL Error Text string[%s]",
								 perIoContext->iParam,
								 sqlReturn,
								 sqlstate,
								 nativeErrorPtr,
								 messageText );

				PostServerEvent( "cSQLWeb::IParam[%d]:Returns[%d] - ODBCRowNumber[%d] SSrvrLine[%d] SSrvrMsgState[%d] SSrvrSeverity[%d] SSrvrProcname[%s] SSrvrSrvname[%s]",
								 perIoContext->iParam,
								 sqlReturn,
								 rownumber,
								 ss_line,
								 ss_msgstate,
								 ss_severity,
								 ss_procname,
								 ss_srvname  );
			}
			else
			{
				// 로그전송.
				PostServerEvent( "cSQLWeb::IParam[%d]:Returns[%d]", perIoContext->iParam, sqlReturn );
			}
		}

		// 3. PerIoContext 사용완료 - 결과는 콜백으로 되돌려준다.
		// perIoContext->requestType   = IOCP_REQUEST_CALLBACK;
		// perIoContext->requestResult = IOCP_REQUEST_SUCCESS;
		cLoginProcess* loginProcess = g_loginSrv->GetLoginProcess( );
		if ( loginProcess != NULL )
		{
			perIoContext->requestResult = (sqlReturn == SQL_SUCCESS ? IOCP_REQUEST_SUCCESS : IOCP_REQUEST_ERROR );
			loginProcess->QueueRequest( completionKey, (OVERLAPPED*)perIoContext, perIoContext->offset );
		}

		// SQLConnection - 내보내기.
		ReleasePool( perSQLConnection );
	}
	return 0L;
}
