// Include
#include "loginsrv.h"

// Local definitions
// memory를 안전하게 해제
#define SafeDelete(P) if (P!=NULL) { delete(P); (P)=NULL; }

// Global data

// cSQLAccount Constructor.
cSQLAccount::cSQLAccount(void)
{
}

// ~cSQLAccount Destructor.
cSQLAccount::~cSQLAccount(void)
{
}

// Initialize Method
bool cSQLAccount::Initialize(char* dns, char* uid, char* pwd, int numWorkerThreads)
{
	return cSQLPool::Initialize( dns, uid, pwd, numWorkerThreads );
}

// InsertCaptcha Method
bool cSQLAccount::InsertCaptcha(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLAccountStmt* sqlStatement  = new cSQLAccountStmt( );
	FILE*            stream        = fopen( "captcha\\captcha.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			throw false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			throw false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			throw false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			throw false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			throw false;

		if ( sqlStatement->TruncateCaptcha( ) != SQL_SUCCESS )
			throw false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char           buffer[4096];
		int            bufferLen = sizeof(buffer)/sizeof(char);

		char           seps[] = "\t \r \n";     // 파싱구문 Parsing.
		char*          token;                   // 파싱토근 Parsing Token.

		TB_CAPTCHA     captcha;
		SQLRETURN      sqlReturn;
		long           retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &captcha, 0, sizeof(TB_CAPTCHA) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// 이름(NAME)
			token = strtok( buffer, seps );
			if ( token == NULL ) throw false;
			MultiByteToWideChar( CP_ACP, 0, token, (int)strlen(token)+1, captcha.name, (int)(sizeof(captcha.name)/sizeof(wchar_t)) );

			token = strtok( NULL, seps );
			if ( token != NULL )
			{
				char  filename[MAX_PATH];
				FILE* image;
				long  read;

				sprintf( filename, "captcha\\%s", token );
				image = fopen( filename, "rb" );
				if ( image != NULL )
				{
					fseek( image, 0, SEEK_END );
					captcha.size = (long)ftell( image );
					fseek( image, 0, SEEK_SET );

					read = fread( captcha.image, sizeof(char), captcha.size, image );
					if ( read != captcha.size )
						throw false;
				}
				else
					throw false;
			}
			else
				throw false;

			sqlReturn = sqlStatement->InsertCaptcha( &captcha, retvalue=0 );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &captcha, 0, sizeof(TB_CAPTCHA) );
		}
		throw true;
	}
	catch ( bool retcode )
	{
		if ( stream != NULL )
		{
			fclose( stream );
			stream = NULL;
		}

		SafeDelete( sqlStatement );
		SafeDelete( sqlConnection );
		SafeDelete( sqlEnv );

		return retcode;
	}
}

// AllocSQLConnection Method
PerSQLConnection* cSQLAccount::AllocSQLConnection(void)
{
	PerSQLConnection* perSQLConnection = (PerSQLConnection*)GlobalAlloc( GPTR, sizeof(PerSQLConnection) );

	if ( perSQLConnection != NULL )
	{
		// PerSQLConnection - 초기화
		perSQLConnection->sqlConnection = new cSQLConnection( );  // 1. SQLConnection  생성.
		perSQLConnection->sqlStatement  = new cSQLAccountStmt( ); // 2. cSQLAccountStmt 생성.
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
bool cSQLAccount::PostServerEvent(LPCTSTR format, ...)
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
DWORD cSQLAccount::WorkerThread(void)
{
	PerSQLConnection* perSQLConnection = NULL;
	cSQLAccountStmt*  sqlAccountStmt   = NULL;
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

		// SQLStatement를 cSQLAccountStmt로 캐스팅.
		sqlAccountStmt = (cSQLAccountStmt*)perSQLConnection->sqlStatement;

		// OVERLAPPED를 PerIoContext로 케스팅.
		perIoContext   = (PerIoContext*)overlapped;

		// 1. SQL를 실행
		// 2. PerIoContext 사용완료 - 결과는 콜백으로 되돌려준다.
		//	  perIoContext->requestType = IOCP_REQUEST_CALLBACK;
		switch ( perIoContext->iParam )
		{
		case SQL_SERVER_PROC_MATRIX_CREATE:
			sqlReturn = sqlAccountStmt->MatrixCreate( (MATRIX_CREATE*)perIoContext->buffer );
			break;
		case SQL_SERVER_PROC_SERVER_LIST:
			sqlReturn = sqlAccountStmt->ServerList( (SERVER_LIST*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_SERVER_PROC_CHANNEL_UPDATE:
			sqlReturn = sqlAccountStmt->ChannelUpdate( (CHANNEL_UPDATE*)perIoContext->buffer );
			break;
		case SQL_SERVER_PROC_CONCURRENT_USER:
			sqlReturn = sqlAccountStmt->ConcurrentUser( (CONCURRENT_USER*)perIoContext->buffer );
			break;
		case SQL_SERVER_PROC_SERVER_DOWN:
			sqlReturn = sqlAccountStmt->ServerDown( (SERVER_DOWN*)perIoContext->buffer );
			break;
		case SQL_CLIENT_PROC_MEMBER_LOGIN:
			sqlReturn = sqlAccountStmt->MemberLogin( (MEMBER_LOGIN*)perIoContext->buffer );
			break;
		case SQL_CLIENT_PROC_MEMBER_LOGOUT:
			sqlReturn = sqlAccountStmt->MemberLogout( (MEMBER_LOGOUT*)perIoContext->buffer );
			break;
		case SQL_CLIENT_PROC_LOGIN_UPDATE:
			sqlReturn = sqlAccountStmt->LoginUpdate( (LOGIN_UPDATE*)perIoContext->buffer );
			break;
		case SQL_CLIENT_PROC_GAME_TO_GAME:
			sqlReturn = sqlAccountStmt->GameToGame( (GAME_TO_GAME*)perIoContext->buffer );
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
				SQLHSTMT    hstmt = sqlAccountStmt->GetSqlStmt( );
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
				PostServerEvent( "cSQLAccount::IParam[%d]:Returns[%d] - SQL Error State string[%s] Native Error code[%d] SQL Error Text string[%s]",
								 perIoContext->iParam,
								 sqlReturn,
								 sqlstate,
								 nativeErrorPtr,
								 messageText );

				PostServerEvent( "cSQLAccount::IParam[%d]:Returns[%d] - ODBCRowNumber[%d] SSrvrLine[%d] SSrvrMsgState[%d] SSrvrSeverity[%d] SSrvrProcname[%s] SSrvrSrvname[%s]",
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
				PostServerEvent( "cSQLAccount::IParam[%d]:Returns[%d]", perIoContext->iParam, sqlReturn );
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
