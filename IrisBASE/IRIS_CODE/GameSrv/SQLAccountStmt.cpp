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

// Shutdown Method.
// GP_SERVERDOWN.SQL 파일참조.
SQLRETURN cSQLAccountStmt::Shutdown(SHUTDOWN_ACCOUNT* shutdownAccount)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SERVERDOWN(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &shutdownAccount->serverNum,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &shutdownAccount->channelNum, 0, &(strLenOrInd=0) );
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

// ChannelCheck Method.
// GP_CHANNEL_CHECK.SQL 파일참조.
SQLRETURN cSQLAccountStmt::ChannelCheck(CHANNEL_CHECK* channelCheck)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHANNEL_CHECK(?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       serverNameLen = (long)sizeof( channelCheck->serverName );
	long       cbServerName  = SQL_NTS;

	long       serverDescLen = (long)sizeof( channelCheck->serverDesc );
	long       cbServerDesc  = SQL_NTS;

	long       inetnumLen = (long)sizeof( channelCheck->inetnum );
	long       cbInetnum  = SQL_NTS;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0,   0, &channelCheck->serverNum,  0,             &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_OUTPUT,       SQL_C_WCHAR,  SQL_WVARCHAR, 50,  0, channelCheck->serverName,  serverNameLen, &cbServerName    );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_OUTPUT,       SQL_C_CHAR,   SQL_VARCHAR,  200, 0, channelCheck->serverDesc,  serverDescLen, &cbServerDesc    );
		retcode = SQLBindParameter( mHstmt, 0x4, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0,   0, &channelCheck->channelNum, 0,             &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x5, SQL_PARAM_INPUT,        SQL_C_CHAR,   SQL_VARCHAR,  20,  0, channelCheck->inetnum,     inetnumLen,    &cbInetnum       );
		retcode = SQLBindParameter( mHstmt, 0x6, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0,   0, &channelCheck->ipv4,       0,             &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x7, SQL_PARAM_INPUT,        SQL_C_SHORT,  SQL_SMALLINT, 0,   0, &channelCheck->port,       0,             &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x8, SQL_PARAM_OUTPUT,       SQL_C_SLONG,  SQL_INTEGER,  0,   0, &channelCheck->retvalue,   0,             &(strLenOrInd=0) );
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

// MemberCheck Method.
SQLRETURN cSQLAccountStmt::MemberCheck(MEMBER_CHECK* check)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_MEMBER_CHECK(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
    SQLUSMALLINT param;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->loginIdx,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->memberIdx,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->serverNum,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->channelNum,     0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->lastChannel,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->characterIdx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &check->instantDungeon, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &check->slevel,         0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->pc,             0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &check->retvalue,       0, &(strLenOrInd=0) );
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

// MemberLogout Method.
SQLRETURN cSQLAccountStmt::MemberLogout(MEMBER_LOGOUT* logout)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_LOGOUT(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &logout->loginIdx,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &logout->memberIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &logout->retvalue,  0, &(strLenOrInd=0) );
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

// MemberOut Method.
SQLRETURN cSQLAccountStmt::MemberOut(MEMBER_OUT* out)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_OUT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &out->loginIdx,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &out->memberIdx, 0, &(strLenOrInd=0) );
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
// GP_LOGIN_UPDATE.SQL 파일참조.
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
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->loginIdx,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->serverNum,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->channelNum,     0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->lastChannel,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->characterIdx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &gameToGame->instantDungeon, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->ipv4,           0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &gameToGame->port,           0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0,  0, &gameToGame->retvalue,       0, &(strLenOrInd=0) );
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

// MemberTwitter Method.
SQLRETURN cSQLAccountStmt::MemberTwitter(MEMBER_TWITTER* memberTwitter)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_TWITTER(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		long tUidSize = sizeof(memberTwitter->twitterUid);
		long cbTUid   = strlen(memberTwitter->twitterUid) ? SQL_NTS : SQL_NULL_DATA;

		long tPwdSize = sizeof(memberTwitter->twitterPwd);
		long cbTPwd   = strlen(memberTwitter->twitterPwd) ? SQL_NTS : SQL_NULL_DATA;

		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0,   0, &memberTwitter->memberIdx,  0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT, SQL_C_CHAR,  SQL_VARCHAR, 100, 0, &memberTwitter->twitterUid, tUidSize, &cbTUid          );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT, SQL_C_CHAR,  SQL_VARCHAR, 100, 0, &memberTwitter->twitterPwd, tPwdSize, &cbTPwd          );
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

// CaptchaOffset Method.
SQLRETURN cSQLAccountStmt::CaptchaOffset( CAPTCHA_OFFSET* captcha )
{

	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CAPTCHA_OFFSET(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
        retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_OUTPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &captcha->offset, 0, &(strLenOrInd=0) );
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

// TrialSelect Method.
SQLRETURN cSQLAccountStmt::TrialSelect(TRIAL_SELECT* trialSelect)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_TRIAL_SELECT( ?, ?, ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
        retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trialSelect->memberIdx,       0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trialSelect->trialTime,       0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trialSelect->trialRejectTime, 0, &(strLenOrInd=0) );
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

// TrialCheck Method.
SQLRETURN cSQLAccountStmt::TrialCheck(MEMBER_TRIAL* trial)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_TRIAL_CHECK(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trial->memberIdx, 0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trial->trialTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trial->retvalue,  0, &(strLenOrInd=0) );
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

// TrialUpdate Method.
SQLRETURN cSQLAccountStmt::TrialUpdate(MEMBER_TRIAL* trial)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_TRIAL_UPDATE(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &trial->memberIdx, 0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &trial->trialTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &trial->retvalue,  0, &(strLenOrInd=0) );
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

// TrialRejectUpdate Method.
SQLRETURN cSQLAccountStmt::TrialRejectUpdate(TRIAL_REJECT* rejectUpdate)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_TRIAL_REJECT_UPDATE(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &rejectUpdate->memberIdx,  0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &rejectUpdate->rejectTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &rejectUpdate->retvalue,   0, &(strLenOrInd=0) );
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

// CaptchaSelect Method.
SQLRETURN cSQLAccountStmt::CaptchaSelect( CAPTCHA_SELECT* captchaSelect ) 
{
    SQLINTEGER strLenOrInd;
    SQLRETURN  retcode;

    /// 재사용 시간 3분 저장
    {
        SQLCHAR* statement = (SQLCHAR*)"{call GP_MEMBER_TRIAL_UPDATE(?, ?, ?)}";
        retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
        if ( SQL_SUCCEEDED( retcode ) )
        {
            long trialTime = 180;
            retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER, 0, 0, &captchaSelect->tiralMemberIdx, 0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &trialTime,                 0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &captchaSelect->retvalue,   0, &(strLenOrInd=0) );
            retcode = SQLExecute( mHstmt );

            if ( SQL_SUCCEEDED( retcode ) )
            {
                // 저장프로시저 쿼리 마무리.
                while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

                // 열려있는 결과 닫기. (Close the open result set.)
                SQLCloseCursor( mHstmt );
            }
            else
                return retcode;
        }
        else
            return retcode;
    }

    /// captcha 이미지 SELECT
    {
        SQLCHAR* statement = (SQLCHAR*)"{call GP_CAPTCHA_SELECT( ? )}";

        retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &captchaSelect->captchaOffset, 0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( SQL_SUCCEEDED( retcode ) )
        {
            long NameLen    = sizeof(captchaSelect->captchaName);
            long captchaLen = sizeof(captchaSelect->captcha);

            if( SQLFetch( mHstmt ) == SQL_SUCCESS )
            {
                SQLGetData( mHstmt, 1, SQL_C_WCHAR,  &captchaSelect->captchaName, NameLen,	&(strLenOrInd=0) );
                SQLGetData( mHstmt, 2, SQL_C_USHORT, &captchaSelect->captchaSize,       0,	&(strLenOrInd=0) );
                SQLGetData( mHstmt, 3, SQL_C_BINARY, &captchaSelect->captcha,  captchaLen,  &(strLenOrInd=0) );
            }
            while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

            SQLCloseCursor( mHstmt );
        }
        else
            return retcode;
    }

    return retcode;
}

// CaptchaReload Method.
SQLRETURN cSQLAccountStmt::CaptchaReload( CAPTCHA_RELOAD* captchaReload ) 
{
    SQLINTEGER strLenOrInd;
    SQLRETURN  retcode;

    SQLCHAR* statement = (SQLCHAR*)"{call GP_CAPTCHA_SELECT( ? )}";

    retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &captchaReload->captchaOffset, 0, &(strLenOrInd=0) );
    retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

    if ( SQL_SUCCEEDED( retcode ) )
    {
        long NameLen    = sizeof(captchaReload->captchaName);
        long captchaLen = sizeof(captchaReload->captcha);

        if( SQLFetch( mHstmt ) == SQL_SUCCESS )
        {
            SQLGetData( mHstmt, 1, SQL_C_WCHAR,  &captchaReload->captchaName, NameLen,	&(strLenOrInd=0) );
            SQLGetData( mHstmt, 2, SQL_C_USHORT, &captchaReload->captchaSize,       0,	&(strLenOrInd=0) );
            SQLGetData( mHstmt, 3, SQL_C_BINARY, &captchaReload->captcha,  captchaLen,  &(strLenOrInd=0) );
        }
        while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

        SQLCloseCursor( mHstmt );
    }

    return retcode;
}

// LoginLimit Method.
SQLRETURN cSQLAccountStmt::LoginLimit(LOGIN_LIMIT* loginLimit)
{
    SQLINTEGER strLenOrInd;
    SQLRETURN  retcode;

    // 제재 카운트 
    {
        SQLCHAR* statement = (SQLCHAR*)"{call GP_MEMBER_LOGIN_LIMIT_COUNT(?, ?, ?)}";
        retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
        if ( SQL_SUCCEEDED( retcode ) )
        {
            retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &loginLimit->memberIdx, 0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &loginLimit->count,     0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &loginLimit->retvalue,  0, &(strLenOrInd=0) );
            retcode = SQLExecute( mHstmt );

            if ( SQL_SUCCEEDED( retcode ) )
            {
                while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

                SQLCloseCursor( mHstmt );
            }
            else
                return retcode;
        }
        else
            return retcode;
    }

    // 로그인 제한 시간
    if ( loginLimit->count > 0 )
    {
        SQLCHAR* statement = (SQLCHAR*)"{call GP_MEMBER_LOGIN_LIMIT_TIME( ?, ? )}";

        retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &loginLimit->memberIdx, 0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &loginLimit->retvalue,  0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( SQL_SUCCEEDED( retcode ) )
        {
            while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

            SQLCloseCursor( mHstmt );
        }
        else
            return retcode;
    }

    return retcode;
}

// ObtEventComplete Method.
SQLRETURN cSQLAccountStmt::ObtEventComplete(MEMBER_OBT_EVENT_COMPLETE* obtEvent)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_OBT_EVENT_COMPLETE(?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER, 0, 0, &obtEvent->memberIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x2, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &obtEvent->cbtUser,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x3, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &obtEvent->obtUser,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x4, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &obtEvent->fourGamer, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 0x5, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &obtEvent->retvalue,  0, &(strLenOrInd=0) );
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