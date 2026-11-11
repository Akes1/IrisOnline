// Include
#include "sqlaccountstmt.h"

// Local definitions

// Global data

// cSQLAccountStmt Constructor.
cSQLAccountStmt::cSQLAccountStmt(void)
{
}

// ~cSQLAccountStmt Destructor.
cSQLAccountStmt::~cSQLAccountStmt(void)
{
}

// TruncateCaptcha Method
SQLRETURN cSQLAccountStmt::TruncateCaptcha(void)
{
	SQLRETURN retcode = SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_CAPTCHA", SQL_NTS );
	SQLCloseCursor( mHstmt );
	return retcode;
}

SQLRETURN cSQLAccountStmt::InsertCaptcha(TB_CAPTCHA* captcha, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CAPTCHA_INSERT(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	long       nameSize = sizeof(captcha->name);
	long       cbName   = wcslen(captcha->name) ? SQL_NTS : SQL_NULL_DATA;

	long       dataLen  = captcha->size;
	long       cbData   = SQL_LEN_DATA_AT_EXEC(captcha->size);

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_WCHAR,  SQL_WVARCHAR,  50,      0, captcha->name,  nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_LONG,   SQL_INTEGER,   0,       0, &captcha->size, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_BINARY, SQL_VARBINARY, dataLen, 0, (VOID*)1,       0,        &cbData          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG,   SQL_INTEGER,   0,       0, &retvalue,      0,        &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		void* parmID = NULL;

		// SQL 데이터 입력.
		retcode = SQLParamData( mHstmt, &parmID );
		if ( retcode == SQL_NEED_DATA )
		{
			retcode = SQLPutData( mHstmt, captcha->image, dataLen );
		}
		// Make Final SQLParamData call.
		retcode = SQLParamData( mHstmt, &parmID );
	}

	SQLCloseCursor( mHstmt );
	return retcode;
}

// MatrixCreate Method.
// GP_MATRIX_CREATE.SQL 파일참조.
//   @MAX_SERVER  [smallint]    --최대 서버번호
//   @MAX_CHANNEL [smallint]    --최대 채널번호
SQLRETURN cSQLAccountStmt::MatrixCreate(MATRIX_CREATE* matrixCreate)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MATRIX_CREATE(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &matrixCreate->retvalue,   0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}

// ServerList Method.
//  GP_SERVER_LIST.SQL 파일참조.
SQLRETURN cSQLAccountStmt::ServerList(SERVER_LIST* serverList, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SERVER_LIST}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_SERVER* table     = serverList->table;
		SQLINTEGER tableSize = sizeof(serverList->table);

		SQLINTEGER nameSize  = sizeof(table->name);
		SQLINTEGER cbName    = 0;

		SQLINTEGER descSize  = sizeof(table->description);
		SQLINTEGER cbDesc    = 0;

		while ( (retcode = SQLFetch( mHstmt )) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 0x01, SQL_C_SLONG,   &table->idx,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 0x02, SQL_C_WCHAR,   table->name,        nameSize, &cbName          );
			SQLGetData( mHstmt, 0x03, SQL_C_WCHAR,   table->description, descSize, &cbDesc          );
			SQLGetData( mHstmt, 0x04, SQL_C_TINYINT, &table->status,     0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 0x05, SQL_C_TINYINT, &table->age,        0,        &(strLenOrInd=0) );
			table++;

			serverList->rowCount++;
			cbLength += tableSize;
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	return SQL_SUCCESS;
}

// ChannelUpdate Method.
//  GP_CHANNEL_UPDATE.SQL 파일참조.
//   @SERVER_NUM  [smallint]  --서버번호
//   @CHANNEL_NUM [smallint]  --채널번호
//   @STATUS      [tinyint]   --상태(0원활/1보통/2혼잡)
//   @RETVALUE    [int]OUTPUT --반환값
SQLRETURN cSQLAccountStmt::ChannelUpdate(CHANNEL_UPDATE* channelUpdate)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHANNEL_UPDATE(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &channelUpdate->serverNum,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &channelUpdate->channelNum, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &channelUpdate->status,     0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x4, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &channelUpdate->retvalue,   0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}

// ConcurrentUser Method.
// GP_CONCURRENT_USER.SQL 파일참조.
SQLRETURN cSQLAccountStmt::ConcurrentUser(CONCURRENT_USER* concurrentUser)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CONCURRENT_USER(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER, 0, 0, &concurrentUser->serverIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER, 0, 0, &concurrentUser->value,     0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}

// ServerDown Method.
// GP_SERVERDOWN.SQL 파일참조.
//   @SERVER_NUM  [smallint]  --서버번호
//   @CHANNEL_NUM [smallint]  --채널번호
SQLRETURN cSQLAccountStmt::ServerDown(SERVER_DOWN* serverDown)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SERVERDOWN(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &serverDown->serverNum,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &serverDown->channelNum, 0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}

// MemberLogin Method
// GP_MEMBER_LOGIN.SQL 파일참조.
//   @MEMBER_IDX       [int]OUTPUT   --고유번호 - 회원 테이블
//   @SLEVEL           [tinyint]	 --보안등급
//   @LOGON_IDX        [int]OUTPUT   --고유번호 - 로그 테이블
//   @LAST_SERVER_NUM  [int]OUTPUT   --서버번호(마지막 접속)
//   @LAST_CHANNEL_NUM [int]OUTPUT   --채널번호(마지막 접속)
//   @INETNUM          [varchar](20) --IP주소(IPv4)
//   @RETVALUE         [int]OUTPUT   --반환값
SQLRETURN cSQLAccountStmt::MemberLogin(MEMBER_LOGIN* login)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_MEMBER_LOGIN(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	SQLINTEGER tUidSize  = sizeof(login->twitterUid);
	SQLINTEGER cbTUid    = 0;

	SQLINTEGER tPwdSize  = sizeof(login->twitterPwd);
	SQLINTEGER cbTPwd    = 0;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->memberIdx,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_TINYINT, SQL_TINYINT, 0,  0, &login->memberSlevel,   0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->memberPc,       0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->loginIdx,       0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->lastServerNum,  0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->lastChannelNum, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_CHAR,    SQL_VARCHAR, 20, 0, &login->twitterUid,     tUidSize, &(cbTUid=0)      );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_CHAR,    SQL_VARCHAR, 30, 0, &login->twitterPwd,     tPwdSize, &(cbTPwd=0)      );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER, 0,  0, &login->retvalue,       0,        &(strLenOrInd=0) );
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

// MemberLogout Method
// GP_MEMBER_LOGOUT.SQL 파일참조.
//   @LOGIN_IDX   [int]       --고유번호 - 로그 테이블
//   @MEMBER_IDX  [int]       --고유번호 - 회원 테이블
//   @RETVALUE    [int]OUTPUT --반환값
SQLRETURN cSQLAccountStmt::MemberLogout(MEMBER_LOGOUT* memberLogout)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_MEMBER_LOGOUT(?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &memberLogout->loginIdx,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &memberLogout->memberIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &memberLogout->retvalue,  0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}

// LoginUpdate Method.
SQLRETURN cSQLAccountStmt::LoginUpdate(LOGIN_UPDATE* loginUpdate)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_LOGIN_UPDATE(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &loginUpdate->loginIdx, 0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}

// GameToGame Method.
// GP_GAME_TO_GAME.SQL 파일참조.
SQLRETURN cSQLAccountStmt::GameToGame(GAME_TO_GAME* gameToGame)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_GAME_TO_GAME(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->loginIdx,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->serverNum,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->channelNum,     0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->lastChannel,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->characterIdx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->instantDungeon, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->ipv4,           0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SSHORT, SQL_SMALLINT, 0,  0, &gameToGame->port,           0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0,  0, &gameToGame->retvalue,       0, &(strLenOrInd=0) );
		retcode = SQLExecute( mHstmt );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

			// 열려있는 결과 닫기. (Close the open result set.)
			SQLCloseCursor( mHstmt );
		}
	}

	return retcode;
}
