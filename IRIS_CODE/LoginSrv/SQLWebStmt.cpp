// Include
#include "sqlwebstmt.h"

// Local definitions

// Global data

// cSQLWebStmt Constructor.
cSQLWebStmt::cSQLWebStmt(void)
{
}

// ~cSQLWebStmt Destructor.
cSQLWebStmt::~cSQLWebStmt(void)
{
}

// WebLogin Method
// sp_member_check.sql 파일참조.
SQLRETURN cSQLWebStmt::WebLauncher(WEB_LAUNCHER* launcher)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call WebLauncher_Login_Info_Proc(?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &launcher->loginIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLINTEGER loginID   = sizeof(launcher->loginId);
		SQLINTEGER cbLoginID = 0;

		SQLINTEGER loginPWD   = sizeof(launcher->loginPwd);
		SQLINTEGER cbLoginPWD = 0;

		if ( (retcode = SQLFetch( mHstmt )) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &launcher->propid,  0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_CHAR,  launcher->loginId,  loginID,  &cbLoginID       );
			SQLGetData( mHstmt, ++param, SQL_C_CHAR,  launcher->loginPwd, loginPWD, &cbLoginPWD      );
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

// WebLogin Method
// sp_member_check.sql 파일참조.
SQLRETURN cSQLWebStmt::WebCheck(WEB_LOGIN* login)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call sp_member_check(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long       loginidLen   = (long)sizeof( login->loginid );
	long       cbLoginid    = SQL_NTS;
	long       passwdLen    = (long)sizeof( login->passwd );
	long       cbPasswd     = SQL_NTS;
	long       passwdSecLen = (long)sizeof( login->passwdsec );
	long       cbPasswdSec  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->idx,        0,            &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_CHAR,    SQL_VARCHAR, 20, 0, login->loginid,     loginidLen,   &cbLoginid       );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_CHAR,    SQL_VARCHAR, 20, 0, login->passwd,      passwdLen,    &cbPasswd        );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_CHAR,    SQL_VARCHAR, 4,  0, login->passwdsec,   passwdSecLen, &cbPasswdSec     );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0,  0, &login->slevel,     0,            &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0,  0, &login->serversec,  0,            &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->retvalue,   0,            &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

// WebLogin Method
// sp_member_login.sql 파일참조.
SQLRETURN cSQLWebStmt::WebLogin(WEB_LOGIN* login)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call sp_member_login(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long       loginidLen   = (long)sizeof( login->loginid );
	long       cbLoginid    = SQL_NTS;
	long       passwdLen    = (long)sizeof( login->passwd );
	long       cbPasswd     = SQL_NTS;
	long       passwdSecLen = (long)sizeof( login->passwdsec );
	long       cbPasswdSec  = SQL_NTS;
	long       ipLen        = (long)sizeof( login->ip );
	long       cbIp         = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->idx,        0,            &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_CHAR,    SQL_VARCHAR, 16, 0, login->loginid,     loginidLen,   &cbLoginid       );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_CHAR,    SQL_VARCHAR, 16, 0, login->passwd,      passwdLen,    &cbPasswd        );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_CHAR,    SQL_VARCHAR, 4,  0, login->passwdsec,   passwdSecLen, &cbPasswdSec     );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_TINYINT, SQL_TINYINT, 0,  0, &login->slevel,     0,            &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_TINYINT, SQL_TINYINT, 0,  0, &login->serversec,  0,            &(strLenOrInd=0) );
	//retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_CHAR,    SQL_VARCHAR, 23, 0, login->ip,          ipLen,        &cbIp            );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->retvalue,   0,            &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

// WebTpSMS Method
SQLRETURN cSQLWebStmt::WebTpSMS(WEB_TP_SMS* webTpSMS)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call TP_SMS_insert(?)}";
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long serverNameLen = (long)sizeof(webTpSMS->serverName);
	long cbServerName  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_CHAR, SQL_VARCHAR, 32, 0, webTpSMS->serverName, serverNameLen, &cbServerName );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}
