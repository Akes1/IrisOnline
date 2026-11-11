/* ====================================================================
 * 파    일 :	odbcsql.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	04/10/19
 * 주의사항 :	
 * =================================================================== */
#ifndef __ODBC_SQL_H__
#define __ODBC_SQL_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include <windows.h>
#include <string.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <odbcss.h>

class cSQLConnection;
class cSQLStatement;
class cSQLEnvironment;

// 주의사항
// 1. SQL_SUCCESS_WITH_INFO 또는 SQL_ERROR 일 경우 레코드의 내용을 확인 할 수 있다.
// 2. Returns값이 SQL_NO_DATA일때까지 확인 확인가능.
// 3. Handle Type
//    SQL_HANDLE_ENV
//    SQL_HANDLE_DBC
//    SQL_HANDLE_STMT
//    SQL_HANDLE_DESC
class cSQLDiag
{
friend class cSQLConnection;
friend class cSQLStatement;
friend class cSQLEnvironment;

protected:
	SQLCHAR      mSqlstate[ SQL_SQLSTATE_SIZE + 1 ];
	SQLINTEGER   mNativeErrorPtr;                        
	SQLCHAR      mMessageText[ SQL_MAX_MESSAGE_LENGTH ];   

public:
	cSQLDiag(void) {}
	virtual ~cSQLDiag(void) {}

	bool GetDiagRec( SQLSMALLINT handleType,
					 SQLHANDLE   handle,
					 SQLSMALLINT recNumber=1 )
	{
		SQLSMALLINT bufferLength = SQL_MAX_MESSAGE_LENGTH;
		SQLSMALLINT textLength   = 0;
		SQLRETURN   result       = SQLGetDiagRec( handleType,
												  handle,
												  recNumber,         // [IN ] Status records are numbered from 1.
												  mSqlstate,         // [OUT] SQL Error State string
												  &mNativeErrorPtr,  // [OUT] Native Error code
												  mMessageText,      // [OUT] SQL Error Text string
												  bufferLength,      // [IN ] Length of the *MessageText buffer in characters.
												  &textLength );

		return ( result == SQL_NO_DATA || result == SQL_ERROR ) ? false : true;
	}

	SQLCHAR*   Sqlstate    ( void ) { return mSqlstate; }
	SQLINTEGER NativeError ( void ) { return mNativeErrorPtr; }
	SQLCHAR*   MessageText ( void ) { return mMessageText; }
};

class cSQLEnvironment
{
friend class cSQLConnection;

protected:
	// 환경 핸들
	SQLHENV mHenv;

public:
	cSQLEnvironment(void) : mHenv( SQL_NULL_HENV ) { }
	virtual ~cSQLEnvironment(void) { FreeEnv( ); }

	SQLHENV GetSqlEnv   ( void ) { return mHenv; }

	// 환경 핸들을 할당한다.
	bool AllocEnv ( void )
	{
		// SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &mHenv )
		return (SQLAllocEnv( &mHenv ) == SQL_SUCCESS) ? true : false;
	}

	// 환경 핸들을 해제한다.
	void FreeEnv ( void )
	{
		if ( mHenv != SQL_NULL_HENV )
		{
			// SQLFreeHandle( SQL_HANDLE_ENV, mHenv );
			SQLFreeEnv( mHenv );
			mHenv = SQL_NULL_HENV;
		}
	}

	// 연결 풀링을 설정한다.
	bool ConnectionPooling ( void )
	{
		return (SQLSetEnvAttr( SQL_NULL_HENV, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER ) == SQL_SUCCESS) ? true : false;
	}

	// 환경 속성을 설정한다.
	bool SetEnvAttr ( SQLINTEGER Attribute    = SQL_ATTR_ODBC_VERSION,
					  SQLPOINTER ValuePtr     = (SQLPOINTER)SQL_OV_ODBC3,
					  SQLINTEGER StringLength = SQL_IS_INTEGER )
	{
		return (SQLSetEnvAttr( mHenv, Attribute, ValuePtr, StringLength ) == SQL_SUCCESS) ? true : false;
	}

	bool ConnectionPoolingRelaxedMatch ( void )
	{
		return (SQLSetEnvAttr( mHenv, SQL_ATTR_CP_MATCH, (SQLPOINTER)SQL_CP_RELAXED_MATCH, SQL_IS_INTEGER ) == SQL_SUCCESS) ? true : false;
	}

	// 데이서 소스에 관란 정보를 넘겨준다.
	// SQL_FETCH_FIRST, SQL_FETCH_NEXT ...
	bool DataSources ( SQLUSMALLINT direction,
					   SQLCHAR*     dns,
					   SQLSMALLINT  cbDnsMax,
					   SQLSMALLINT* cbDns,
					   SQLCHAR*     description,
					   SQLSMALLINT  cbDescriptionMax,
					   SQLSMALLINT* cbDescription )
	{
		SQLRETURN result = SQLDataSources( mHenv,
										   direction,
										   dns,
										   cbDnsMax,
										   cbDns,
										   description,
										   cbDescriptionMax,
										   cbDescription );

		if ( result == SQL_NO_DATA || result == SQL_ERROR )
			return false;
		return true;
	}
};

class cSQLConnection
{
friend class cSQLStatement;

protected:
	// 접속 핸들
	SQLHDBC mHdbc;

public:
	cSQLConnection(void) : mHdbc( SQL_NULL_HDBC ) { }

	SQLHDBC GetSqlDbc ( void ) { return mHdbc; }

	// 접속 핸들을 할당한다.
	virtual bool AllocDbc ( cSQLEnvironment* sqlEnvironment )
	{
		// SQLAllocHandle( SQL_HANDLE_DBC, sqlEnvironment->mHenv, &mHdbc )
		return (SQLAllocConnect( sqlEnvironment->mHenv, &mHdbc ) == SQL_SUCCESS) ? true : false;
	}

	// 접속 핸들을 할당한다.
	virtual bool AllocDbc ( SQLHENV henv ) 
	{
		// SQLAllocHandle( SQL_HANDLE_DBC, henv, &mHdbc )
		return (SQLAllocConnect( henv, &mHdbc ) == SQL_SUCCESS) ? true : false;
	}

	// 접속 핸들을 해제한다.
	virtual void FreeDbc ( void )
	{
		if ( mHdbc != SQL_NULL_HDBC )
		{
			// SQLFreeHandle( SQL_HANDLE_DBC, mHdbc )
			SQLFreeConnect( mHdbc );
			mHdbc = SQL_NULL_HDBC;
		}
	}

	// 가장 단순한 연결 함수
	virtual bool Connect ( SQLCHAR* dns, SQLCHAR* uid, SQLCHAR* pwd )
	{
		SQLRETURN retcode;
		retcode = SQLConnect( mHdbc, dns, SQL_NTS, uid, SQL_NTS, pwd, SQL_NTS );

		// if failed to connect, free the allocated mHdbc before return
		// 만약 연결을 실패한다면, 리턴전에 mHdbc 할당을 해제한다.
		if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			//GetDiagRec( SQL_HANDLE_DBC, mHdbc );
			Disconnect( );
			return false;
		}

		/*--
		retcode = SQLSetConnectAttr( mHdbc, SQL_ATTR_CONNECTION_TIMEOUT, (void*)5, 0 );
		if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			//GetDiagRec( SQL_HANDLE_DBC, mHdbc );
			Disconnect( );
			return false;
		}
		*/

		// display any connection information if driver returns SQL_SUCCESS_WITH_INFO
		// if ( retcode == SQL_SUCCESS_WITH_INFO )
		//	GetDiagRec( SQL_HANDLE_DBC, mHdbc );		
		return true;
	}

	// Driver 연결 함수
	virtual bool SQLConnect4XLS ( SQLCHAR* connStrIn, SQLSMALLINT connStrLen, SQLCHAR* connStrOut, SQLSMALLINT connStrOutMax, SQLSMALLINT* connStrOutLen )
	{
		// DRIVER={Microsoft Excel Driver (*.xls)}; FIRSTROWHASNAMES=1; READONLY=0; CREATE_DB=\"test.xls\"; DBQ=test.xls;
		SQLRETURN retcode;
		retcode = SQLDriverConnect( mHdbc
								   ,NULL
								   ,connStrIn
								   ,connStrLen
								   ,connStrOut
								   ,connStrOutMax
								   ,connStrOutLen
								   ,SQL_DRIVER_NOPROMPT );

		// if failed to connect, free the allocated mHdbc before return
		// 만약 연결을 실패한다면, 리턴전에 mHdbc 할당을 해제한다.
		if ( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			//GetDiagRec( SQL_HANDLE_DBC, mHdbc );
			Disconnect( );
			return false;
		}

		return true;
	}

	// SQL_AUTOCOMMIT_OFF = The driver uses manual-commit mode,
	//						and the application must explicitly commit or roll back transactions with SQLEndTran.
	SQLRETURN AutocommitOff ( void )
	{
		return SQLSetConnectAttr( mHdbc, SQL_ATTR_AUTOCOMMIT, (void*)SQL_AUTOCOMMIT_OFF, 0 );
	}
	// SQL_AUTOCOMMIT_ON = The driver uses autocommit mode.
	//	Each statement is committed immediately after it is executed.
	//	This is the default.
	//	Any open transactions on the connection are committed
	//	when SQL_ATTR_AUTOCOMMIT is set to SQL_AUTOCOMMIT_ON to change from manual-commit mode to autocommit mode
	SQLRETURN AutocommitOn ( void )
	{
		return SQLSetConnectAttr( mHdbc, SQL_ATTR_AUTOCOMMIT, (void*)SQL_AUTOCOMMIT_ON, 0 );
	}
	// Completion Type SQL_COMMIT, SQL_ROLLBACK.
	SQLRETURN EndTran ( SQLSMALLINT completionType )
	{
		return SQLEndTran( SQL_HANDLE_DBC, mHdbc, completionType ); 
	}

	// SQL_COPT_SS_CONNECTION_DEAD reports the alive or dead state of a connection to a server.
	// The driver queries the Net-Library for the current state of the connection.

	// SQL_COPT_SS_CONNECTION_DEAD 는 서버와의 접속상태가 유지되는지 또는 끊어졌는지를 알려준다.
	// 드라이버는 현재 접속 상태 정보를 "Net-Library"에서 얻는다.
	virtual bool ConnectionDead ( void )
	{
		SQLINTEGER ValuePtr;
		SQLINTEGER StringLengthPtr;
		// SQL_CD_TRUE:  The connection to the server has been lost.
		// SQL_CD_FALSE: The connection is open and available for statement processing.

		// SQL_CD_TRUE:  서버와의 접속이 상실되었다.
		// SQL_CD_FALSE: 접속이 열려 있으며, 명령문 처리가 가능하다.
		SQLGetConnectAttr( mHdbc, SQL_COPT_SS_CONNECTION_DEAD, &ValuePtr, SQL_IS_INTEGER, &StringLengthPtr );
		return (ValuePtr == SQL_CD_TRUE) ? true : false;
	}

	// 연결을 종료한다.
	virtual void Disconnect ( void )
	{
		if ( mHdbc != SQL_NULL_HDBC )
		{
			SQLDisconnect( mHdbc );
		}
	}

public:
	virtual ~cSQLConnection(void)
	{
		Disconnect( );
		FreeDbc( );
	}
};

class cSQLStatement
{
protected:
	// 명령문 핸들
	SQLHSTMT mHstmt;

public:
	cSQLStatement(void) : mHstmt( SQL_NULL_HSTMT ) { }

	SQLHSTMT GetSqlStmt ( void ) { return mHstmt; }

	// 명령문 핸들을 할당한다.
	virtual bool AllocStmt ( cSQLConnection* sqlConnection )
	{
		// SQLAllocHandle( SQL_HANDLE_STMT, sqlConnection->mHdbc, &mHstmt )
		return (SQLAllocStmt( sqlConnection->mHdbc, &mHstmt ) == SQL_SUCCESS) ? true : false;
	}

	// 명령문 핸들을 할당한다.
	virtual bool AllocStmt ( SQLHDBC hdbc )
	{
		// SQLAllocHandle( SQL_HANDLE_STMT, hdbc, &mHstmt )
		return (SQLAllocStmt( hdbc, &mHstmt ) == SQL_SUCCESS) ? true : false;
	}

	// 명령문 핸들을 해제한다.
	virtual void FreeStmt ( void )
	{
		if ( mHstmt != SQL_NULL_HSTMT )
		{
			// SQLFreeHandle( SQL_HANDLE_STMT, mHstmt )
			SQLFreeStmt( mHstmt, SQL_DROP );
			mHstmt = SQL_NULL_HSTMT;
		}
	}

	virtual bool Sequence ( void )
	{
		SQLCHAR     sqlstate[ SQL_SQLSTATE_SIZE + 1 ];
		SQLINTEGER  nativeErrorPtr;
		SQLCHAR     messageText[ SQL_MAX_MESSAGE_LENGTH ];   
		SQLSMALLINT bufferLength = SQL_MAX_MESSAGE_LENGTH;
		SQLSMALLINT textLength   = 0;

		SQLGetDiagRec( SQL_HANDLE_STMT, mHstmt, 1, sqlstate, &nativeErrorPtr, messageText, bufferLength, &textLength );

		return (strcmp( (char*)sqlstate, "HY010" ) == 0) ? true : false;
	}

	virtual SQLRETURN Test ( char* message )
	{
		SQLCHAR*   statement = (SQLCHAR*)"SELECT ?";
		SQLINTEGER strLenOrInd;
		SQLRETURN  retcode;

		long       messageLen = (long)strlen( message );
		long       cbMessage  = SQL_NTS;

		SQLCHAR    buffer[MAX_PATH];
		long       bufferLen = sizeof(buffer);

		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, messageLen, 0, message, 0, &cbMessage );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
		if ( retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO )
		{
			// 결과 확인.
			if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			{
				SQLGetData( mHstmt, 0x01, SQL_C_CHAR, buffer, bufferLen, &(strLenOrInd=0) );
			}

			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
		return retcode;
	}

public:
	virtual ~cSQLStatement(void)
	{
		FreeStmt( );
	}
};

#endif // __ODBC_SQL_H__
