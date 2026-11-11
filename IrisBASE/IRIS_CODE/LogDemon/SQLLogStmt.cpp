// Include
#include "sqllogstmt.h"

// Local definitions

// Global data

// cSQLLogStmt Constructor.
cSQLLogStmt::cSQLLogStmt(void)
{
}

// ~cSQLLogStmt Destructor.
cSQLLogStmt::~cSQLLogStmt(void)
{
}

// ServerEvent Method.
SQLRETURN cSQLLogStmt::ServerEvent(SERVER_EVENT* server)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SERVER_EVENT(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       sourceLen   = (long)strlen( server->source );
	long       cbSource    = SQL_NTS;

	long       messageLen  = (long)strlen( server->message );
	long       cbMessage   = SQL_LEN_DATA_AT_EXEC(messageLen);

	void*      parmID;
	long       cbBatch     = (long)strlen( server->message );

	long       computerLen = (long)strlen( server->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &server->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, &server->source,       sourceLen,   &cbSource        );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,              0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &server->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	retcode = SQLParamData( mHstmt, &parmID );
	if ( retcode == SQL_NEED_DATA )
	{
		while ( messageLen > cbBatch )
		{
			SQLPutData( mHstmt, server->message, cbBatch );
			messageLen -= cbBatch;
		}
		// Put final batch.
		SQLPutData( mHstmt, server->message, messageLen ); 
	}

	// Make final SQLParamData call.
	retcode = SQLParamData( mHstmt, &parmID );

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// MemberEvent Method.
SQLRETURN cSQLLogStmt::MemberEvent(MEMBER_EVENT* member)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MEMBER_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLCHAR* type = NULL;

	switch ( member->type )
	{
	case EVENT_MEMBER_SUCCESS_AUDIT:
		type = (SQLCHAR*)"Success Audit";
		break;
	case EVENT_MEMBER_FAILURE_AUDIT:
		type = (SQLCHAR*)"Failure Audit";
		break;
	case EVENT_MEMBER_TIMEOUT:
		type = (SQLCHAR*)"Timeout";
		break;
	default:
		type = (SQLCHAR*)"Unknown";
		break;
	}

	SQLCHAR* category = NULL;

	switch ( member->category )
	{
	case EVENT_MEMBER_LOGIN:
		category = (SQLCHAR*)"Login";
		break;
	case EVENT_MEMBER_LOGOUT:
		category = (SQLCHAR*)"Logout";
		break;
	default:
		category = (SQLCHAR*)"Unknown";
		break;
	}

	long       typeLen     = (long)strlen( (char*)type );
	long       cbType      = SQL_NTS;

	long       sourceLen   = (long)strlen( member->source );
	long       cbSource    = SQL_NTS;

	long       categoryLen = (long)strlen( (char*)category );
	long       cbCategory  = SQL_NTS;

	long       messageLen  = (long)strlen( member->message );
	long       cbMessage   = SQL_LEN_DATA_AT_EXEC(messageLen);

	void*      parmID;
	long       cbBatch     = (long)strlen( member->message );

	long       computerLen = (long)strlen( member->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &member->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, type,                  typeLen,     &cbType          );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, member->source,        sourceLen,   &cbSource        );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, category,              categoryLen, &cbCategory      );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &member->memberIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,              0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &member->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	retcode = SQLParamData( mHstmt, &parmID );
	if ( retcode == SQL_NEED_DATA )
	{
		while ( messageLen > cbBatch )
		{
			SQLPutData( mHstmt, member->message, cbBatch );
			messageLen -= cbBatch;
		}
		// Put final batch.
		SQLPutData( mHstmt, member->message, messageLen ); 
	}

	// Make final SQLParamData call.
	retcode = SQLParamData( mHstmt, &parmID );

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// CharacterEvent Method.
SQLRETURN cSQLLogStmt::CharacterEvent(CHARACTER_EVENT* character)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_EVENT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	SQLCHAR* category    = NULL;

	switch ( character->category )
	{
	case EVENT_CHARACTER_CREATE:
		category = (SQLCHAR*)"Create";
		break;
	case EVENT_CHARACTER_DELETE:
		category = (SQLCHAR*)"Delete";
		break;
	case EVENT_CHARACTER_UPDATE:
		category = (SQLCHAR*)"Update";
		break;
	case EVENT_CHARACTER_LOGIN:
		category = (SQLCHAR*)"Login";
		break;
	case EVENT_CHARACTER_LOGOUT:
		category = (SQLCHAR*)"Logout";
		break;
	case EVENT_CHARACTER_DELETE_WAIT:
		category = (SQLCHAR*)"Wait";
		break;
	case EVENT_CHARACTER_DELETE_CANCEL:
		category = (SQLCHAR*)"Cancel";
		break;
	default:
		category = (SQLCHAR*)"Unknow";
		break;
	}

	long       sourceLen   = (long)strlen( character->source );
	long       cbSource    = SQL_NTS;

	long       categoryLen = (long)strlen( (char*)category );
	long       cbCategory  = SQL_NTS;

	long       messageLen  = (long)strlen( character->message );
	long       cbMessage   = SQL_LEN_DATA_AT_EXEC(messageLen);

	void*      parmID;
	long       cbBatch     = (long)strlen( character->message );

	long       computerLen = (long)strlen( character->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &character->registerDate,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, character->source,               sourceLen,   &cbSource        );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, category,                        categoryLen, &cbCategory      );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &character->memberIdx,           0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &character->serverIdx,           0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &character->characterIdx,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &character->characterLevel,      0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &character->characterExp,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &character->characterSkillLevel, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &character->characterSkillExp,   0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &character->characterSkillPoint, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,                        0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, character->computer,             computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	retcode = SQLParamData( mHstmt, &parmID );
	if ( retcode == SQL_NEED_DATA )
	{
		while ( messageLen > cbBatch )
		{
			SQLPutData( mHstmt, character->message, cbBatch );
			messageLen -= cbBatch;
		}
		// Put final batch.
		SQLPutData( mHstmt, character->message, messageLen ); 
	}

	// Make final SQLParamData call.
	retcode = SQLParamData( mHstmt, &parmID );

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// MoneyEvent Method.
SQLRETURN cSQLLogStmt::MoneyEvent(MONEY_EVENT* money)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_MONEY_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       messageLen  = (long)strlen( money->message );
	long       cbMessage   = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch     = (long)strlen( money->message );

	long       computerLen = (long)strlen( money->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &money->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &money->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &money->characterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &money->before,       0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &money->after,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,             0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &money->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, money->message, cbBatch );
				messageLen -= cbBatch;
			} while ( messageLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, money->message, messageLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}
	}

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// DepositEvent Method.
SQLRETURN cSQLLogStmt::DepositEvent(DEPOSIT_EVENT* deposit)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_DEPOSIT_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       messageLen  = (long)strlen( deposit->message );
	long       cbMessage   = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch     = (long)strlen( deposit->message );

	long       computerLen = (long)strlen( deposit->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &deposit->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &deposit->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &deposit->characterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &deposit->before,       0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &deposit->after,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,               0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &deposit->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, deposit->message, cbBatch );
				messageLen -= cbBatch;
			} while ( messageLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, deposit->message, messageLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}
	}

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// InventoryEvent Method.
SQLRETURN cSQLLogStmt::InventoryEvent(INVENTORY_EVENT* inventory)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_INVENTORY_EVENT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLCHAR* category = NULL;

	switch ( inventory->category )
	{
	case EVENT_INVENTORY_CREATE:
		category = (SQLCHAR*)"Create";
		break;
	case EVENT_INVENTORY_DELETE:
		category = (SQLCHAR*)"Delete";
		break;
	case EVENT_INVENTORY_UPDATE:
		category = (SQLCHAR*)"Update";
		break;
	default:
		category = (SQLCHAR*)"Unknown";
		break;
	}
	long       categoryLen = (long)strlen( (char*)category );
	long       cbCategory  = SQL_NTS;

	long       messageLen  = (long)strlen( inventory->message );
	long       cbMessage   = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch     = (long)strlen( inventory->message );

	long       computerLen = (long)strlen( inventory->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1,  SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &inventory->registerDate,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2,  SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &inventory->serverIdx,       0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3,  SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, category,                    categoryLen, &cbCategory      );
	retcode = SQLBindParameter( mHstmt, 4,  SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &inventory->inventoryIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5,  SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &inventory->characterIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6,  SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &inventory->itemDefineIndex, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 7,  SQL_PARAM_INPUT, SQL_C_SHORT,          SQL_SMALLINT,       0,                 0, &inventory->count,           0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 8,  SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &inventory->enhanced,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 9,  SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &inventory->seal,            0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 10, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,                    0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 11, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &inventory->computer,        computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, inventory->message, cbBatch );
				messageLen -= cbBatch;
			} while ( messageLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, inventory->message, messageLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}
	}

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// ConcurrentEvent Method.
SQLRETURN cSQLLogStmt::ConcurrentEvent(CONCURRENT_EVENT* concurrent)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CONCURRENT_EVENT(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       messageLen  = (long)strlen( concurrent->message );
	long       cbMessage   = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch     = (long)strlen( concurrent->message );

	long       computerLen = (long)strlen( concurrent->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &concurrent->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &concurrent->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &concurrent->minimum,      0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &concurrent->maximum,      0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,                       0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &concurrent->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, concurrent->message, cbBatch );
				messageLen -= cbBatch;
			} while ( messageLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, concurrent->message, messageLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}
	}

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// QuestEvent Method.
SQLRETURN cSQLLogStmt::QuestEvent(QUEST_EVENT* questEvent)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_QUEST_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLCHAR* category = NULL;

	switch ( questEvent->category )
	{
	case EVENT_QUEST_ADD:
		category = (SQLCHAR*)"Add";
		break;
	case EVENT_QUEST_UPDATE:
		category = (SQLCHAR*)"Update";
		break;
	case EVENT_QUEST_DELETE:
		category = (SQLCHAR*)"Delete";
		break;
	case EVENT_QUEST_COMPLETE:
		category = (SQLCHAR*)"Complete";
		break;
	case EVENT_QUEST_REWARD:
		category = (SQLCHAR*)"Reward";
		break;
	case EVENT_QUEST_REMOVE:
		category = (SQLCHAR*)"Remove";
		break;
	case EVENT_QUEST_TIMESELECT:
		category = (SQLCHAR*)"Timeselect";
		break;
	default:
		category = (SQLCHAR*)"Unknown";
		break;
	}
	long  categoryLen = (long)strlen( (char*)category );
	long  cbCategory  = SQL_NTS;

	long  messageLen  = (long)strlen( questEvent->message );
	long  cbMessage   = SQL_DATA_AT_EXEC;

	void* parmID;
	long  cbBatch     = (long)strlen( questEvent->message );

	long  computerLen = (long)strlen( questEvent->computer );
	long  cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &questEvent->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &questEvent->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        50,                0, category,                  categoryLen, &cbCategory      );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &questEvent->characterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &questEvent->questIdx,     0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,                  0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &questEvent->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, questEvent->message, cbBatch );
				messageLen -= cbBatch;
			} while ( messageLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, questEvent->message, messageLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}
	}

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// GuildEvent Method.
SQLRETURN cSQLLogStmt::GuildEvent(GUILD_EVENT* guildEvent)
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILD_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;

	SQLCHAR*   category = NULL;
	
	long       messageLen  = (long)strlen( guildEvent->message );
	long       cbMessage   = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch     = (long)strlen( guildEvent->message );

	long       computerLen = (long)strlen( guildEvent->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &guildEvent->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &guildEvent->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &guildEvent->characterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &guildEvent->guildIdx,     0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &guildEvent->guildPosition,0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_LONGVARCHAR,    messageLen,        0, (VOID*)1,                  0,           &cbMessage       );
	retcode = SQLBindParameter( mHstmt, 7, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &guildEvent->computer,     computerLen, &cbComputer      );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, guildEvent->message, cbBatch );
				messageLen -= cbBatch;
			} while ( messageLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, guildEvent->message, messageLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}
	}

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// DropItemEvent Method.
SQLRETURN cSQLLogStmt::DropItemEvent(DROPITEM_EVENT* pDropEvent)
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_DROPITEM_EVENT(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT n;

	SQLINTEGER	monsterIdx;
	SQLINTEGER	dieCnt;
	SQLINTEGER	moneyCnt;
	SQLINTEGER	itemIdx;
	SQLINTEGER	itemCnt;
	SQLCHAR     type;

	long computerLen = (long)strlen( pDropEvent->mComputer );
	long cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, n=1, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &monsterIdx,              0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &dieCnt,                  0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &moneyCnt,                0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &itemIdx,                 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &itemCnt,                 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0,   0, &type,                    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &pDropEvent->mServerIdx,  0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &pDropEvent->mChannelNum, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_CHAR,    SQL_VARCHAR, 132, 0, &pDropEvent->mComputer,   computerLen, &cbComputer      );
	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		for ( long i = 0; i < pDropEvent->mCount; ++i )
		{
			sDropItem* pDropItem = &pDropEvent->mDropItem[i];

			monsterIdx = pDropItem->mMonsterIdx;
			dieCnt     = pDropItem->mDieCnt;
			moneyCnt   = pDropItem->mMoneyCnt;
			itemIdx    = pDropItem->mItemIdx;
			itemCnt    = pDropItem->mItemCnt;
			type       = pDropItem->mDropType;
			retcode    = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );	// 저장프로시저 쿼리 마무리.
			}
			else
				return retcode;
		}

		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );		// 저장프로시저 쿼리 마무리.
		SQLCloseCursor( mHstmt );									// 열려있는 결과 닫기.
	}

	return retcode;
}

// SkillEvent Method.
SQLRETURN cSQLLogStmt::SkillEvent( SKILL_EVENT* skillEvent )
{
	SQLCHAR* statement = (SQLCHAR*)"{call GP_SKILL_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN retcode;
	SQLSMALLINT n;

	long computerLen = (long)strlen( skillEvent->mComputer );
	long cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, n=1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &skillEvent->mRegisterDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &skillEvent->mCharacterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &skillEvent->mIsReset,      0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &skillEvent->mSkillIdx,     0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT,        SQL_TINYINT,        0,                 0, &skillEvent->mSkillStep,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &skillEvent->mServerIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &skillEvent->mComputer,     computerLen, &cbComputer      );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );	// 저장프로시저 쿼리 마무리.
		SQLCloseCursor( mHstmt );								// 열려있는 결과 닫기.
	}

	return retcode;
}

// ItemBillEvent Method.
SQLRETURN cSQLLogStmt::ItemBillEvent(ITEM_BILL_EVENT* itemBill)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_BILL_EVENT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	long       computerLen = (long)strlen( itemBill->computer );
	long       cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &itemBill->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &itemBill->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &itemBill->characterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &itemBill->inventoryIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &itemBill->before,       0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &itemBill->after,        0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &itemBill->computer,     computerLen, &cbComputer      );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	// 저장프로시저 쿼리 마무리.
	while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

// ItemBillEvent Method.
SQLRETURN cSQLLogStmt::InfCashEvent(MB_SYN_INFCASH_SAVE* infCashEvent)
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_INFCASH_EVENT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT n;
	

	SQLCHAR* category = NULL;

	switch ( infCashEvent->mType )
	{
	case EVENT_INFCASH_CYCLE:
		category = (SQLCHAR*)"Cycle";
		break;
	case EVENT_INFCASH_INSERT:
		category = (SQLCHAR*)"Insert";
		break;
	case EVENT_INFCASH_DELETE:
		category = (SQLCHAR*)"Delete";
		break;
	case EVENT_INFCASH_LOGOUT:
		category = (SQLCHAR*)"LogOut";
		break;
	default:
		category = (SQLCHAR*)"Unknown";
		break;
	}

	long  categoryLen = (long)strlen( (char*)category );
	long  cbCategory  = SQL_NTS;

	long computerLen = (long)strlen( infCashEvent->mComputer );
	long cbComputer  = SQL_NTS;

	SQLINTEGER	dbIdx;
	SQLINTEGER	infIdx;
	SQLINTEGER	attackerIdx;
	SQLINTEGER	attackerType;
	SQLCHAR		isRealTime;
	SQLINTEGER	restTime;
	SQL_TIMESTAMP_STRUCT	endTime;
	SQLINTEGER	time1;
	SQLINTEGER	time2;
	SQLINTEGER	value1;
	SQLINTEGER	value2;
	SQLINTEGER	attributeType;
	SQLCHAR		attackerLevel;
	SQLINTEGER	attackerDamageMin;
	SQLINTEGER	attackerDamageMax;
	SQLINTEGER	attackerHeal;
	SQLCHAR		guardCnt;
	SQLSMALLINT	guardValue;
	SQLCHAR		isDel;

	retcode = SQLBindParameter( mHstmt, n=1, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &infCashEvent->mServerNum, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &infCashEvent->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_CHAR,    SQL_VARCHAR, 50,  0, category, categoryLen, &cbCategory );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &dbIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &infIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &attackerIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &attackerType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT,SQL_TINYINT, 0,   0, &isRealTime, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &restTime, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN,   0, &endTime, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &time1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &time2, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &value1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &value2, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &attributeType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT,SQL_TINYINT, 0,   0, &attackerLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &attackerDamageMin, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &attackerDamageMax, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG,    SQL_INTEGER, 0,   0, &attackerHeal, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT,SQL_TINYINT, 0,   0, &guardCnt, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SSHORT,  SQL_SMALLINT, 0,   0, &guardValue, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT,SQL_TINYINT, 0,   0, &isDel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_CHAR,SQL_VARCHAR,132,  0, &infCashEvent->mComputer,     computerLen, &cbComputer      );
	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		for ( long i = 0; i < infCashEvent->mCount; ++i )
		{
			sLogInf* pLogInf = &infCashEvent->mInfCash[i];

			dbIdx = pLogInf->mDbIdx;
			infIdx = pLogInf->mInfluenceClassIdx;
			attackerIdx = pLogInf->mAttackerIdx;
			attackerType = pLogInf->mAttackerType;
			isRealTime = pLogInf->mIsRealTime;
			restTime = pLogInf->mRestTime;
			endTime = pLogInf->mEndDate;
			time1 = pLogInf->mTime1;
			time2 = pLogInf->mTime2;
			value1 = pLogInf->mInfluenceValue1;
			value2 = pLogInf->mInfluenceValue2;
			attributeType = pLogInf->mAttriButeType;
			attackerLevel = pLogInf->mAttackerLv;
			attackerDamageMin = pLogInf->mAttackerDamageMin;
			attackerDamageMax = pLogInf->mAttackerDamageMax;
			attackerHeal = pLogInf->mHeal;
			guardCnt = pLogInf->mLeftGuardCnt;
			guardValue = pLogInf->mLeftGuardValue;
			isDel = pLogInf->mDel;
			retcode = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );	// 저장프로시저 쿼리 마무리.
			}
			else
				return retcode;
		}

		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );		// 저장프로시저 쿼리 마무리.
		SQLCloseCursor( mHstmt );									// 열려있는 결과 닫기.
	}

	return retcode;
}

// ReservedEvent Method.
SQLRETURN cSQLLogStmt::ReservedEvent(RESERVED_EVENT* reserved)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_RESERVED_EVENT(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	long computerLen = (long)strlen( reserved->computer );
	long cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &reserved->registerDate, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &reserved->serverIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SHORT,          SQL_SMALLINT,       0,                 0, &reserved->type,         0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &reserved->characterIdx, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &reserved->memberIdx,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &reserved->computer,     computerLen, &cbComputer      );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );	// 저장프로시저 쿼리 마무리.
		SQLCloseCursor( mHstmt );							// 열려있는 결과 닫기.
	}

	return retcode;
}


// ReservedEvent Method.
SQLRETURN cSQLLogStmt::PostEvent(POST_EVENT* post)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_EVENT(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	long computerLen = (long)strlen( post->computer );
	long cbComputer  = SQL_NTS;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &post->registerDate,  0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->serverIdx,     0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->postIdx,       0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->characterIdx,  0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->inventoryIdx1, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->inventoryIdx2, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->inventoryIdx3, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_LONG,           SQL_INTEGER,        0,                 0, &post->money,         0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_CHAR,           SQL_VARCHAR,        132,               0, &post->computer,      computerLen, &cbComputer      );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );	// 저장프로시저 쿼리 마무리.
		SQLCloseCursor( mHstmt );							// 열려있는 결과 닫기.
	}

	return retcode;
}
