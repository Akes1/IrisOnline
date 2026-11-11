// Include
#include "stdafx.h"
#include "sqlgamestmt.h"
#include "QuestDefine.h"
#include "Theme_Common.h"
#include "Npc_Common.h"
// Local definitions

// Global data

// cSQLGameStmt Constructor.
cSQLGameStmt::cSQLGameStmt(void)
{
}

// ~cSQLGameStmt Destructor.
cSQLGameStmt::~cSQLGameStmt(void)
{
}

// Init Method
SQLRETURN cSQLGameStmt::Init(INIT_GAME* initGame)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_INIT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &initGame->channelNum,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &initGame->removeCoolTimeCount,  0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// Shutdown Method
SQLRETURN cSQLGameStmt::Shutdown(SHUTDOWN_GAME* shutdownGame)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SHUTDOWN(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &shutdownGame->channelNum,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &shutdownGame->removeCoolTimeCount,  0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemeCreate Method
SQLRETURN cSQLGameStmt::ThemeCreate(THEME_CREATE* create)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_CREATE(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT,          SQL_SMALLINT,  0,                 0, &create->themeIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT,        SQL_TINYINT,   0,                 0, &create->fixTime,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT,        SQL_TINYINT,   0,                 0, &create->varTime,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT,          SQL_SMALLINT,  0,                 0, &create->maxRoom,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &create->lastDate,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &create->validThru, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,          SQL_INTEGER,   0,                 0, &create->retvalue,  0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemeRoomCheck Method
SQLRETURN cSQLGameStmt::ThemeRoomCheck(THEME_ROOM_CHECK* themeRoomCheck)
{
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;
	{
		SQLCHAR*   statement = (SQLCHAR*)"{call GP_THEME_USER_TEST(?, ?, ?, ?)}";

		SQLINTEGER characterIdx;
		SQLINTEGER retvalue;

		retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
		if ( SQL_SUCCEEDED( retcode ) )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT,  SQL_SMALLINT, 0, 0, &themeRoomCheck->themeIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT,SQL_TINYINT, 0, 0, &themeRoomCheck->themeMode,0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER, 0, 0, &characterIdx,             0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER, 0, 0, &retvalue,                 0, &(strLenOrInd=0) );

			characterIdx = themeRoomCheck->characterIdx;
			retvalue     = themeRoomCheck->retvalue;
			retcode = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
				themeRoomCheck->retvalue = retvalue;
			else
				return retcode;

			SQLClose( mHstmt );
		}
		else
			return retcode;
	}
	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_THEME_ROOM_CHECK(?, ?, ?, ?)}";

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT,   SQL_SMALLINT, 0, 0, &themeRoomCheck->themeIdx,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SHORT,   SQL_SMALLINT, 0, 0, &themeRoomCheck->roomIdx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &themeRoomCheck->themeMode, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &themeRoomCheck->retvalue,  0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		SQLClose( mHstmt );
	}
	return retcode;
}

// ThemeUserCheck Method
SQLRETURN cSQLGameStmt::ThemeUserCheck(THEME_USER_CHECK* themeUserCheck)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_USER_CHECK(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserCheck->themeIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserCheck->roomIdx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TINYINT,SQL_TINYINT, 0, 0, &themeUserCheck->themeMode,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserCheck->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemeUserInsert Method
SQLRETURN cSQLGameStmt::ThemeUserInsert(THEME_USER_INSERT* themeUserInsert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_USER_INSERT(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserInsert->themeIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT,SQL_TINYINT, 0, 0, &themeUserInsert->themeMode,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserInsert->roomIdx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserInsert->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserInsert->characterMaxCnt, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserInsert->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemeUserSelect Method
SQLRETURN cSQLGameStmt::ThemeUserSelect(THEME_USER_SELECT* themeUserSelect, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_USER_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &themeUserSelect->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		THEME_USER* table     = themeUserSelect->table;
		u_long      tableSize = sizeof(themeUserSelect->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SHORT,          &table->themeIdx,  0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SHORT,          &table->roomIdx,   0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &table->themeMode, 0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->lastDate,  SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->validThru, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			themeUserSelect->rowCount++;
		}
		cbLength += (themeUserSelect->rowCount * tableSize);

		SQLClose( mHstmt );
	}
	return retcode;
}

// ThemeUserUpdate Method
SQLRETURN cSQLGameStmt::ThemeUserUpdate(THEME_USER_UPDATE* themeUserUpdate)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_USER_UPDATE(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserUpdate->themeIdx,        0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT,SQL_TINYINT, 0, 0, &themeUserUpdate->themeMode,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserUpdate->roomIdx,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserUpdate->characterIdx,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserUpdate->characterMaxCnt, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserUpdate->retvalue,        0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemeUserRemove Method
SQLRETURN cSQLGameStmt::ThemeUserRemove(THEME_USER_REMOVE* themeUserRemove)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_USER_DELETE(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserRemove->themeIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT,SQL_TINYINT, 0, 0, &themeUserRemove->themeMode,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &themeUserRemove->roomIdx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserRemove->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserRemove->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemeUserSave Method
SQLRETURN cSQLGameStmt::ThemeUserSave(THEME_USER_SAVE* themeUserSave)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_USER_SAVE(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT,          SQL_SMALLINT,  0,                 0, &themeUserSave->themeIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TINYINT,        SQL_TINYINT,   0,                 0, &themeUserSave->themeMode,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT,          SQL_SMALLINT,  0,                 0, &themeUserSave->roomIdx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &themeUserSave->lastDate,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &themeUserSave->validThru,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,          SQL_INTEGER,   0,                 0, &themeUserSave->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,          SQL_INTEGER,   0,                 0, &themeUserSave->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ThemePartySelect Method
SQLRETURN cSQLGameStmt::ThemePartySelect( THEME_PARTYINFO_SELECT* pPartySelect, u_long& cbLength )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_THEME_PARTY_SELECT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER characterIdx;
	SQLINTEGER themeIdx = pPartySelect->mThemeIdx;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( !SQL_SUCCEEDED( retcode ) )
		return retcode;

	SQLSMALLINT n = 0;
	SQLSMALLINT param = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,     SQL_INTEGER, 0, 0, &characterIdx,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SSHORT,    SQL_SMALLINT, 0, 0, &themeIdx,       0, &(strLenOrInd=0) );

	sThemePartyInfoDB* table = pPartySelect->mPartyInfo;
	for ( unsigned long i = 0; i < pPartySelect->mPartyCnt ; ++i )
	{
		characterIdx = pPartySelect->mCharacterIdx[i];

		retcode = SQLExecute( mHstmt );

		if ( (SQL_SUCCEEDED( retcode )) )
		{
			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; ++i, ++table )
			{
				table->mPlayerIdx = characterIdx;
				SQLGetData( mHstmt, param=1, SQL_C_SHORT, &table->mMode, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->mIsJoin, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->mIsDependency, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG, &table->mRoomIdx, 0, &(strLenOrInd=0) );
				++pPartySelect->mCount;
			}

			// 저장프로시저 쿼리 마무리.
			while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		}
		else
			return retcode;
	}

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	cbLength = pPartySelect->GetMsgLength();

	return retcode;
}

// ThemePartyRoomSelect Method
SQLRETURN cSQLGameStmt::ThemePartyRoomSelect( THEME_PARTYROOM_SELECT* pRoomSelect )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_THEME_PARTYROOM_SELECT(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	SQLINTEGER characterIdx;
	SQLINTEGER retvalue;

	unsigned short roomIdx[MAX_PARTY_USER*eTHEME_MODE_MAX];
	unsigned char count = 0;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &pRoomSelect->mThemeIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &pRoomSelect->mMode, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &retvalue, 0, &(strLenOrInd=0) );

		for ( long i = 0; i < pRoomSelect->mPartyCnt; i++ )
		{
			characterIdx = pRoomSelect->mPartyUserIdx[i];
			retcode = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				if( retvalue != 0 )
				{
					roomIdx[count] = (unsigned short)retvalue;
					++count;
				}
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	else
		return retcode;

	if( count == 0 )
		pRoomSelect->mRoomIdx = 0;
	else
	{
		int randSelect = rand();
		randSelect = randSelect % count;

		pRoomSelect->mRoomIdx = roomIdx[randSelect];
	}

	return retcode;
}

// ThemeUserSelect Method
SQLRETURN cSQLGameStmt::ThemeGMUserSelect(THEME_GM_USER_SELECT* themeUserSelect, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_THEME_GM_USER_SELECT(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;
	SQLINTEGER  retvalue = 0;

	long       nameSize = sizeof(themeUserSelect->characterName);
	long       cbName   = wcslen(themeUserSelect->characterName) ? SQL_NTS : SQL_NULL_DATA;
	
	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, themeUserSelect->characterName, nameSize, &cbName );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &themeUserSelect->characterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if( retvalue == 0 )
		{
			THEME_USER* table     = themeUserSelect->table;
			u_long      tableSize = sizeof(themeUserSelect->table);

			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
			{
				SQLGetData( mHstmt, param=1, SQL_C_SHORT,          &table->themeIdx,  0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SHORT,          &table->roomIdx,   0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &table->themeMode, 0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->lastDate,  SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->validThru, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
				themeUserSelect->rowCount++;
			}
			cbLength += (themeUserSelect->rowCount * tableSize);
		}
		
		SQLClose( mHstmt );
	}
	return retcode;
}


// TruncateItemDefine Method
SQLRETURN cSQLGameStmt::TruncateItemDefine(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_DEFINE", SQL_NTS ), mHstmt );
}

// TruncateItemAbility Method
SQLRETURN cSQLGameStmt::TruncateItemAbility(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ABILITY", SQL_NTS ), mHstmt );
}

// TruncateItemLimit Method
SQLRETURN cSQLGameStmt::TruncateItemLimit(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_LIMIT", SQL_NTS ), mHstmt );
}

// TruncateItemCard Method
SQLRETURN cSQLGameStmt::TruncateItemCard(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_CARD", SQL_NTS ), mHstmt );
}

// TruncateItemCardSlot Method
SQLRETURN cSQLGameStmt::TruncateItemCardSlot(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_CARD_SLOT", SQL_NTS ), mHstmt );
}

// TruncateItemTarot Method
SQLRETURN cSQLGameStmt::TruncateItemTarot(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_TAROT", SQL_NTS ), mHstmt );
}

// TruncateItemTarotResult Method
SQLRETURN cSQLGameStmt::TruncateItemTarotResult(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_TAROT_RESULT", SQL_NTS ), mHstmt );
}

// TruncateItemSpread Method
SQLRETURN cSQLGameStmt::TruncateItemSpread(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_SPREAD", SQL_NTS ), mHstmt );
}

// TruncateItemDisjoint Method
SQLRETURN cSQLGameStmt::TruncateItemDisjoint(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_DISJOINT", SQL_NTS ), mHstmt );
}

// TruncateItemEnhanced Method
SQLRETURN cSQLGameStmt::TruncateItemEnhanced(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ENHANCED", SQL_NTS ), mHstmt );
}

// TruncateItemEnhancedValue Method
SQLRETURN cSQLGameStmt::TruncateItemEnhancedValue(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ENHANCED_VALUE", SQL_NTS ), mHstmt );
}

// TruncateItemEnhancedRate Method
SQLRETURN cSQLGameStmt::TruncateItemEnhancedRate(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ENHANCED_RATE", SQL_NTS ), mHstmt );
}

// TruncateItemEnhancedSuccess Method
SQLRETURN cSQLGameStmt::TruncateItemEnhancedSuccess(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ENHANCED_SUCCESS", SQL_NTS ), mHstmt );
}

// TruncateItemEnhancedDown Method
SQLRETURN cSQLGameStmt::TruncateItemEnhancedDown(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ENHANCED_DOWN", SQL_NTS ), mHstmt );
}

// TruncateItemEnhancedDestruct Method
SQLRETURN cSQLGameStmt::TruncateItemEnhancedDestruct(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_ENHANCED_DESTRUCT", SQL_NTS ), mHstmt );
}

// TruncateItemChange Method
SQLRETURN cSQLGameStmt::TruncateItemChange(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_CHANGE", SQL_NTS ), mHstmt );
}

// TruncateItemChangeItems Method
SQLRETURN cSQLGameStmt::TruncateItemChangeItems(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_ITEM_CHANGE_ITEMS", SQL_NTS ), mHstmt );
}

// TruncateDefaultItems Method
SQLRETURN cSQLGameStmt::TruncateDefaultItems(void)
{
	return SQLClose( SQLExecDirect( mHstmt, (SQLCHAR*)"TRUNCATE TABLE TB_DEFAULT_ITEMS", SQL_NTS ), mHstmt );
}

// InsertItemDefine Method
SQLRETURN cSQLGameStmt::InsertItemDefine(TB_ITEM_DEFINE* tbItemDefine, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_DEFINE_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	long       nameSize = sizeof(tbItemDefine->name);
	long       cbName   = wcslen(tbItemDefine->name) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->index,             0,        &(strLenOrInd=0) );//01
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_WCHAR,   SQL_WVARCHAR, 50, 0, tbItemDefine->name,               nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->type,              0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->subType,           0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->searchType,        0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &tbItemDefine->searchSubType,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &tbItemDefine->weight,            0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &tbItemDefine->capacity,          0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->sellType,          0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->sellPrice,         0,        &(strLenOrInd=0) );//10
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->buyType,           0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->buyPrice,          0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->maxPresent,        0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->exchange,          0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->maxInventory,      0,        &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->licenseClass,      0,        &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->licenseValidThru,  0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->licenseType,       0,        &(strLenOrInd=0) );
	//retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &tbItemDefine->sealcount,         0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->abilityIndex,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->limitIndex,        0,        &(strLenOrInd=0) );//20
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->cardIndex,         0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->cardSlotIndex,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->tarotIndex,        0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->spreadIndex,       0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->enhancedIndex,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->enhancedRateIndex, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->disjointIndex,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->changeIndex,       0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->titleIndex,        0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->setIndex,          0,        &(strLenOrInd=0) );//30
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &tbItemDefine->iconIndex,         0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->rareLevel,         0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &tbItemDefine->webPage,           0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0,  0, &retvalue,                        0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemAbility Method
SQLRETURN cSQLGameStmt::InsertItemAbility(TB_ITEM_ABILITY* tbItemAbility, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_ABILITY_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &tbItemAbility->index,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->def_class,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &tbItemAbility->range_type,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &tbItemAbility->target_type,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->use_range,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->p_def,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->m_def,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->atk_range,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->atk_spd,       0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &tbItemAbility->cool_time1,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &tbItemAbility->cool_time2,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->min_p_atk,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->max_p_atk,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->min_m_atk,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->max_m_atk,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->heal,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt01_index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt01_value,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt02_index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt02_value,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt03_index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt03_value,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt04_index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt04_value,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt05_index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemAbility->opt05_value,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &tbItemAbility->influence_idx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &tbItemAbility->active_index,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &retvalue,                     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemLimit Method
SQLRETURN cSQLGameStmt::InsertItemLimit(TB_ITEM_LIMIT* tbItemLimit, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_LIMIT_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemLimit->index,        0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->charRace,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->charGender,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemLimit->charJob1,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemLimit->charJob2,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->charLevelMin, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->charLevelMax, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->serverType,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->forceType,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemLimit->makeSkill,    0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_ULONG,   SQL_INTEGER, 0, 0, &tbItemLimit->makeSkillExp, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &retvalue,                  0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemCard Method
SQLRETURN cSQLGameStmt::InsertItemCard(TB_ITEM_CARD* tbItemCard, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_CARD_INSERT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemCard->index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemCard->episode, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemCard->number,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCard->level,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCard->type,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCard->subType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &retvalue,            0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemCardSlot Method
SQLRETURN cSQLGameStmt::InsertItemCardSlot(TB_ITEM_CARD_SLOT* tbItemCardSlot, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_CARD_SLOT_INSERT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemCardSlot->index, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCardSlot->slot1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCardSlot->slot2, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCardSlot->slot3, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCardSlot->slot4, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemCardSlot->slot5, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &retvalue,              0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemCard Method
SQLRETURN cSQLGameStmt::InsertItemTarot(TB_ITEM_TAROT* tbItemTarot, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_TAROT_INSERT(?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemTarot->index,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemTarot->number,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemTarot->level,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemTarot->strValue, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemTarot->invValue, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemTarot->prop,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemTarot->episode,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &retvalue,              0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemTarotResult Method
SQLRETURN cSQLGameStmt::InsertItemTarotResult(TB_ITEM_TAROT_RESULT* tbItemTarotResult, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_TAROT_RESULT_INSERT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemTarotResult->index,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemTarotResult->min_value,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemTarotResult->max_value,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemTarotResult->influence_idx1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemTarotResult->influence_idx2, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemTarotResult->influence_idx3, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &retvalue,                          0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemSpread Method
SQLRETURN cSQLGameStmt::InsertItemSpread(TB_ITEM_SPREAD* tbItemSpread, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_SPREAD_INSERT(?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	long       calcSize = sizeof(tbItemSpread->calc);
	long       cbCalc   = strlen(tbItemSpread->calc) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &tbItemSpread->index,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &tbItemSpread->axisArray, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemSpread->xPos,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemSpread->yPos,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &tbItemSpread->dir,       0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_CHAR,    SQL_CHAR,     2, 0, &tbItemSpread->calc,      calcSize, &cbCalc          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &tbItemSpread->value,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &retvalue,                0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemDisjoint Method
SQLRETURN cSQLGameStmt::InsertItemDisjoint(TB_ITEM_DISJOINT* tbItemDisjoint, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_DISJOINT_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->index,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->price,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->itemDefineIndex1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbItemDisjoint->inventoryCount1,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->itemDefineIndex2, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbItemDisjoint->inventoryCount2,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->itemDefineIndex3, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbItemDisjoint->inventoryCount3,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->itemDefineIndex4, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbItemDisjoint->inventoryCount4,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->itemDefineIndex5, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbItemDisjoint->inventoryCount5,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbItemDisjoint->itemDefineIndex6, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbItemDisjoint->inventoryCount6,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &retvalue,                         0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InsertItemEnhanced Method
SQLRETURN cSQLGameStmt::InsertItemEnhanced(TB_ITEM_ENHANCED* tbItemEnhanced)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_INSERT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemEnhanced->index, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_VALUE_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
		short*   value;

		for ( int i0 = 0; i0 < tbItemEnhanced->row; i0++ )
		{
			value   = tbItemEnhanced->value[ i0 ].value;
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemEnhanced->index, 0, &(strLenOrInd=0) );
			//retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemEnhanced->value[ i0 ].equip, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemEnhanced->value[ i0 ].type, 0, &(strLenOrInd=0) );
			for ( int i1 = 0; i1 < MAX_ITEM_ENHANCED; i1++, value++ )
			{
				retcode = SQLBindParameter( mHstmt
										   ,++param
										   ,SQL_PARAM_INPUT
										   ,SQL_C_SHORT
										   ,SQL_SMALLINT
										   ,0
										   ,0
										   ,value
										   ,0
										   ,&(strLenOrInd=0) );
			}
			retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
			if ( !SQL_SUCCEEDED( retcode ) )
				return retcode;
		}
	}

	return SQLClose( retcode, mHstmt );
}

// InsertItemEnhancedRate Method
SQLRETURN cSQLGameStmt::InsertItemEnhancedRate(TB_ITEM_ENHANCED_RATE* tbItemEnhancedRate)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_RATE_INSERT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemEnhancedRate->index, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		for ( int i0 = 0; i0 < 2; i0++ )
		{
			// SUCCESS
			{
				SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_SUCCESS_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
				long*    value;

				value   = tbItemEnhancedRate->value[ i0 ].success;
				retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemEnhancedRate->index,            0, &(strLenOrInd=0) );
				retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemEnhancedRate->value[ i0 ].type, 0, &(strLenOrInd=0) );
				for ( int i1 = 0; i1 < MAX_ITEM_ENHANCED; i1++, value++ )
				{
					retcode = SQLBindParameter( mHstmt
											   ,++param
											   ,SQL_PARAM_INPUT
											   ,SQL_C_LONG
											   ,SQL_INTEGER
											   ,0
											   ,0
											   ,value
											   ,0
											   ,&(strLenOrInd=0) );
				}
				retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
				if ( !SQL_SUCCEEDED( retcode ) )
					return retcode;
			}
			// DOWN
			{
				SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_DOWN_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
				long*    value;

				value   = tbItemEnhancedRate->value[ i0 ].down;
				retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemEnhancedRate->index,            0, &(strLenOrInd=0) );
				retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemEnhancedRate->value[ i0 ].type, 0, &(strLenOrInd=0) );
				for ( int i1 = 0; i1 < MAX_ITEM_ENHANCED; i1++, value++ )
				{
					retcode = SQLBindParameter( mHstmt
											   ,++param
											   ,SQL_PARAM_INPUT
											   ,SQL_C_LONG
											   ,SQL_INTEGER
											   ,0
											   ,0
											   ,value
											   ,0
											   ,&(strLenOrInd=0) );
				}
				retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
				if ( !SQL_SUCCEEDED( retcode ) )
					return retcode;
			}
			// DESTRUCT
			{
				SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_DESTRUCT_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
				long*    value;

				value   = tbItemEnhancedRate->value[ i0 ].destruct;
				retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &tbItemEnhancedRate->index,            0, &(strLenOrInd=0) );
				retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &tbItemEnhancedRate->value[ i0 ].type, 0, &(strLenOrInd=0) );
				for ( int i1 = 0; i1 < MAX_ITEM_ENHANCED; i1++, value++ )
				{
					retcode = SQLBindParameter( mHstmt
											   ,++param
											   ,SQL_PARAM_INPUT
											   ,SQL_C_LONG
											   ,SQL_INTEGER
											   ,0
											   ,0
											   ,value
											   ,0
											   ,&(strLenOrInd=0) );
				}
				retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
				if ( !SQL_SUCCEEDED( retcode ) )
					return retcode;
			}
		}
	}

	return SQLClose( retcode, mHstmt );
}

// InsertItemChange Method
SQLRETURN cSQLGameStmt::InsertItemChange(TB_ITEM_CHANGE* tbItemChange)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_CHANGE_INSERT(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemChange->index, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemChange->minMoney, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemChange->maxMoney, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tbItemChange->rate,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		long*  itemDefineIndex = tbItemChange->itemDefineIndex;
		short* itemCount       = tbItemChange->itemCount;
		short* enhanced		   = tbItemChange->enhanced;
		long * changeRate      = tbItemChange->changeRate;

		for ( int i = 0; i < tbItemChange->row; i++, itemDefineIndex++, itemCount++, changeRate++ )
		{
			SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_CHANGE_ITEMS_INSERT(?, ?, ?, ?)}";

			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &tbItemChange->index, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, itemDefineIndex,      0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, itemCount,            0, &(strLenOrInd=0) );
			//retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, enhanced,             0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, changeRate,           0, &(strLenOrInd=0) );

			retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
			if ( !SQL_SUCCEEDED( retcode ) )
				return retcode;
		}
	}

	return SQLClose( retcode, mHstmt );
}

// InsertDefaultItems Method
SQLRETURN cSQLGameStmt::InsertDefaultItems(TB_DEFAULT_ITEMS* tbDefaultItems, long& retvalue)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_DEFAULT_ITEMS_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbDefaultItems->characterJob,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &tbDefaultItems->itemDefineIndex, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbDefaultItems->inventoryNumber, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &tbDefaultItems->inventoryCount,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &retvalue,                        0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemDefine Method
SQLRETURN cSQLGameStmt::VerifyItemDefine(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_DEFINE(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemAbility Method
SQLRETURN cSQLGameStmt::VerifyItemAbility(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_ABILITY(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemCooltime2 Method
SQLRETURN cSQLGameStmt::VerifyItemCooltime2(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_COOLTIME2(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemLimit Method
SQLRETURN cSQLGameStmt::VerifyItemLimit(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_LIMIT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemCard Method
SQLRETURN cSQLGameStmt::VerifyItemCard(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_CARD(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemCardSlot Method
SQLRETURN cSQLGameStmt::VerifyItemCardSlot(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_CARD_SLOT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemTarot Method
SQLRETURN cSQLGameStmt::VerifyItemTarot(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_TAROT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemEnhanced Method
SQLRETURN cSQLGameStmt::VerifyItemEnhanced(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_ENHANCED(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemEnhancedRate Method
SQLRETURN cSQLGameStmt::VerifyItemEnhancedRate(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_ENHANCED_RATE(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemDisjoint Method
SQLRETURN cSQLGameStmt::VerifyItemDisjoint(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_DISJOINT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// VerifyItemChange Method
SQLRETURN cSQLGameStmt::VerifyItemChange(VERIFY_ITEM_TABLE* verifyItemTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_ITEM_CHANGE(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->count,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyItemTable->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ItemDefine Method
// GP_ITEM_DEFINE_SELECT 참조.
//   @IDX [int] --인덱스
SQLRETURN cSQLGameStmt::ItemDefine(ITEM_DEFINE_TABLE* itemDefineTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_DEFINE_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemDefineTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_DEFINE* table    = itemDefineTable->table;
		SQLINTEGER      nameSize = sizeof(itemDefineTable->table->name);

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG,   &table->idx,               0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxAbility,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxLimit,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxCard,           0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxCardSlot,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxTarot,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxEnhanced,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxEnhancedRate,   0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxDisjoint,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->idxChange,         0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->index,             0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR,   table->name,               nameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->type,              0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->subType,           0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->searchType,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SSHORT,  &table->searchSubType,     0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SSHORT,  &table->weight,            0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SSHORT,  &table->capacity,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->sellType,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->sellPrice,         0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->buyType,           0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->buyPrice,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->present,           0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->maxPresent,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->exchange,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->maxInventory,      0,        &(strLenOrInd=0) );
            SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->licenseClass,      0,        &(strLenOrInd=0) );
            SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->licenseValidThru,  0,        &(strLenOrInd=0) );
            SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->licenseType,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->abilityIndex,      0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->limitIndex,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->cardIndex,         0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->cardSlotIndex,     0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->tarotIndex,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->spreadIndex,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->enhancedIndex,     0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->enhancedRateIndex, 0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->disjointIndex,     0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->changeIndex,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->titleIndex,        0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->setIndex,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->rareLevel,         0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply,             0,        &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemAbility Method
// GP_ITEM_ABILITY_SELECT 참조.
SQLRETURN cSQLGameStmt::ItemAbility(ITEM_ABILITY_TABLE* itemAbilityTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_ABILITY_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemAbilityTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_ABILITY* table = itemAbilityTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1,  SQL_C_SLONG,   &table->idx,           0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2,  SQL_C_SLONG,   &table->index,         0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3,  SQL_C_SSHORT,  &table->def_class,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4,  SQL_C_TINYINT, &table->range_type,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5,  SQL_C_TINYINT, &table->target_type,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 6,  SQL_C_SSHORT,  &table->use_range,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 7,  SQL_C_SSHORT,  &table->p_def,         0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 8,  SQL_C_SSHORT,  &table->m_def,         0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 9,  SQL_C_SSHORT,  &table->atk_range,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 10, SQL_C_SSHORT,  &table->atk_spd,       0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 11, SQL_C_SLONG,   &table->cool_time1,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 12, SQL_C_SLONG,   &table->cool_time2,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 13, SQL_C_SSHORT,  &table->min_p_atk,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 14, SQL_C_SSHORT,  &table->max_p_atk,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 15, SQL_C_SSHORT,  &table->min_m_atk,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 16, SQL_C_SSHORT,  &table->max_m_atk,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 17, SQL_C_SSHORT,  &table->heal,          0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 18, SQL_C_SSHORT,  &table->opt01_index,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 19, SQL_C_SSHORT,  &table->opt01_value,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 20, SQL_C_SSHORT,  &table->opt02_index,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 21, SQL_C_SSHORT,  &table->opt02_value,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 22, SQL_C_SSHORT,  &table->opt03_index,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 23, SQL_C_SSHORT,  &table->opt03_value,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 24, SQL_C_SSHORT,  &table->opt04_index,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 25, SQL_C_SSHORT,  &table->opt04_value,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 26, SQL_C_SSHORT,  &table->opt05_index,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 27, SQL_C_SSHORT,  &table->opt05_value,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 28, SQL_C_SLONG,   &table->influence_idx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 29, SQL_C_SLONG,   &table->active_index,  0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemCooltime2 Method
// GP_ITEM_COOLTIME2_SELECT 참조.
SQLRETURN cSQLGameStmt::ItemCooltime2(ITEM_COOLTIME2_TABLE* itemCooltime2Table, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_COOLTIME2_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 0x1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemCooltime2Table->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long     tableSize = sizeof(itemCooltime2Table->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, &itemCooltime2Table->table[ i ].itemDefineIdx,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG, &itemCooltime2Table->table[ i ].itemDefineIndex, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_SLONG, &itemCooltime2Table->table[ i ].cooltime,        0, &(strLenOrInd=0) );
			itemCooltime2Table->rowCount++;
		}
		cbLength += (itemCooltime2Table->rowCount * tableSize);

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemLimit Method
// GP_ITEM_LIMIT_SELECT 참조.
SQLRETURN cSQLGameStmt::ItemLimit(ITEM_LIMIT_TABLE* itemLimitTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_LIMIT_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemLimitTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_LIMIT* table = itemLimitTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG,   &table->idx,          0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->index,        0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->charRace,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->charGender,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->charJob1,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->charJob2,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->charLevelMin, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->charLevelMax, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->serverType,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->forceType,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->makeSkill,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_ULONG,   &table->makeSkillExp, 0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemCard Method
// GP_ITEM_CARD_SELECT 참조.
SQLRETURN cSQLGameStmt::ItemCard(ITEM_CARD_TABLE* itemCardTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_CARD_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemCardTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_CARD* table = itemCardTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, &table->idx,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG, &table->index, 0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemCardSlot Method
// GP_ITEM_CARD_SLOT_SELECT 참조.
SQLRETURN cSQLGameStmt::ItemCardSlot(ITEM_CARD_SLOT_TABLE* itemCardSlotTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_CARD_SLOT_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemCardSlotTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_CARD_SLOT* table = itemCardSlotTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &table->idx,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG,   &table->index, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_TINYINT, &table->slot1, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_TINYINT, &table->slot2, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_TINYINT, &table->slot3, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 6, SQL_C_TINYINT, &table->slot4, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 7, SQL_C_TINYINT, &table->slot5, 0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemTarot Method
// GP_ITEM_TAROT_SELECT 참조.
SQLRETURN cSQLGameStmt::ItemTarot(ITEM_TAROT_TABLE* itemTarotTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_TAROT_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemTarotTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_TAROT* table = itemTarotTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &table->idx,      0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG,   &table->index,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_SLONG,   &table->number,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_TINYINT, &table->level,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_SLONG,   &table->strValue, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 6, SQL_C_SLONG,   &table->invValue, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 7, SQL_C_SLONG,   &table->prop,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 8, SQL_C_SLONG,   &table->episode,  0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemEnhanced Method
SQLRETURN cSQLGameStmt::ItemEnhanced(ITEM_ENHANCED_TABLE* itemEnhancedTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT param;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemEnhancedTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_ENHANCED* table = itemEnhancedTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, &table->idx,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG, &table->index, 0, &(strLenOrInd=0) );
			SQLClose( mHstmt );

			if ( table->index != 0 )
			{
				SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_VALUE_SELECT(?)}";
				short*   value     = NULL;

				retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->index, 0, &(strLenOrInd=0) );
				retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					for ( int i0 = 0; i0 < 3 && SQLFetch( mHstmt ) == SQL_SUCCESS; i0++ )
					{
						value = table->value[ i0 ].value;

						SQLGetData( mHstmt, param=1, SQL_C_SLONG, &table->value[ i0 ].type,  0, &(strLenOrInd=0) );
						for ( int i1 = 0; i1 < MAX_ITEM_ENHANCED; i1++, value++ )
						{
							SQLGetData( mHstmt, ++param, SQL_C_SLONG, value, 0, &(strLenOrInd=0) );
						}

						table->row++;
					}
					SQLClose( mHstmt );
				}
			}
		}
	}
	return retcode;
}

// ItemEnhancedRate Method
SQLRETURN cSQLGameStmt::ItemEnhancedRate(ITEM_ENHANCED_RATE_TABLE* itemEnhancedRateTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_RATE_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT param;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemEnhancedRateTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_ENHANCED_RATE* table = itemEnhancedRateTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, &table->idx,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG, &table->index, 0, &(strLenOrInd=0) );
			SQLClose( mHstmt );

			if ( table->index != 0 )
			{
				for ( BYTE i0=1, i1=0; i0 <= 2; i0++, i1++ )
				{
					// SUCCESS
					{
						SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_SUCCESS_SELECT(?, ?)}";
						long*    value     = NULL;

						retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &table->index, 0, &(strLenOrInd=0) );
						retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &i0,           0, &(strLenOrInd=0) );
						retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

						if ( SQL_SUCCEEDED( retcode ) )
						{
							if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
							{
								value = table->value[ i1 ].success;

								table->value[ i1 ].type = i0;
								for ( short i2 = 1; i2 <= MAX_ITEM_ENHANCED; i2++, value++ )
								{
									SQLGetData( mHstmt, i2, SQL_C_SLONG, value, 0, &(strLenOrInd=0) );
								}
							}
							SQLClose( mHstmt );
						}
					}
					// DOWN
					{
						SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_DOWN_SELECT(?, ?)}";
						long*    value     = NULL;

						retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &table->index, 0, &(strLenOrInd=0) );
						retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &i0,           0, &(strLenOrInd=0) );
						retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

						if ( SQL_SUCCEEDED( retcode ) )
						{
							if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
							{
								value = table->value[ i1 ].down;

								table->value[ i1 ].type = i0;
								for ( short i2 = 1; i2 <= MAX_ITEM_ENHANCED; i2++, value++ )
								{
									SQLGetData( mHstmt, i2, SQL_C_SLONG, value, 0, &(strLenOrInd=0) );
								}
							}
							SQLClose( mHstmt );
						}
					}
					// DESTRUCT
					{
						SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_ENHANCED_DESTRUCT_SELECT(?, ?)}";
						long*    value     = NULL;

						retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &table->index, 0, &(strLenOrInd=0) );
						retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &i0,           0, &(strLenOrInd=0) );
						retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

						if ( SQL_SUCCEEDED( retcode ) )
						{
							if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
							{
								value = table->value[ i1 ].destruct;

								table->value[ i1 ].type = i0;
								for ( short i2 = 1; i2 <= MAX_ITEM_ENHANCED; i2++, value++ )
								{
									SQLGetData( mHstmt, i2, SQL_C_SLONG, value, 0, &(strLenOrInd=0) );
								}
							}
							SQLClose( mHstmt );
						}
					}
				}
			}
		}
	}
	return retcode;
}

// ItemDisjoint Method
SQLRETURN cSQLGameStmt::ItemDisjoint(ITEM_DISJOINT_TABLE* itemDisjointTable)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_DISJOINT_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemDisjointTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_DISJOINT* table        = itemDisjointTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1,  SQL_C_SLONG, &table->idx,         0,            &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2,  SQL_C_SLONG, &table->index,       0,            &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3,  SQL_C_SLONG, &table->price,       0,            &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4,  SQL_C_SLONG, &table->itemDefineIndex1, 0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5,  SQL_C_SHORT, &table->inventoryCount1,  0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 6,  SQL_C_SLONG, &table->itemDefineIndex2, 0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 7,  SQL_C_SHORT, &table->inventoryCount2,  0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 8,  SQL_C_SLONG, &table->itemDefineIndex3, 0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 9,  SQL_C_SHORT, &table->inventoryCount3,  0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 10, SQL_C_SLONG, &table->itemDefineIndex4, 0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 11, SQL_C_SHORT, &table->inventoryCount4,  0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 12, SQL_C_SLONG, &table->itemDefineIndex5, 0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 13, SQL_C_SHORT, &table->inventoryCount5,  0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 14, SQL_C_SLONG, &table->itemDefineIndex6, 0,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, 15, SQL_C_SHORT, &table->inventoryCount6,  0,       &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemChange Method
SQLRETURN cSQLGameStmt::ItemChange(ITEM_CHANGE_TABLE* itemChangeTable)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_CHANGE_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT param;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemChangeTable->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_CHANGE* table = itemChangeTable->table;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &table->idx,      0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->index,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->minMoney, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->maxMoney, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->rate,     0, &(strLenOrInd=0) );
			SQLClose( mHstmt );

			if ( table->index != 0 )
			{
				SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_CHANGE_ITEMS_SELECT(?)}";
				long&    row             = table->row;
				long*    itemDefineIndex = table->itemDefineIndex;
				short*   itemCount       = table->itemCount;
				long*    changeRate      = table->changeRate;

				retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->index, 0, &(strLenOrInd=0) );
				retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					for ( int i0 = 0; i0 < MAX_ITEM_CHANGE_DATA && SQLFetch( mHstmt ) == SQL_SUCCESS; i0++ )
					{
						SQLGetData( mHstmt, param=1, SQL_C_SLONG, itemDefineIndex, 0, &(strLenOrInd=0) );
						SQLGetData( mHstmt, ++param, SQL_C_SLONG, itemCount,       0, &(strLenOrInd=0) );
						SQLGetData( mHstmt, ++param, SQL_C_SLONG, changeRate,      0, &(strLenOrInd=0) );

						row++;
						itemDefineIndex++;
						itemCount++;
						changeRate++;
					}
					SQLClose( mHstmt );
				}
			}
		}
	}
	return retcode;
}

// VerifyDefaultItems Method
SQLRETURN cSQLGameStmt::VerifyDefaultItems(VERIFY_DEFAULT_ITEMS* verifyDefaultItems)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_VERIFY_DEFAULT_ITEMS(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyDefaultItems->rowCount, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyDefaultItems->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterList Method
// GP_CHARACTER_LIST 참조.
//   @USER_IDX [int]
SQLRETURN cSQLGameStmt::CharacterList(CHARACTER_LIST* characterList, u_long& cbLength)
{
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_LIST(?, ?)}";

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterList->userIdx,  0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterList->retvalue, 0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			u_long             tableSize = sizeof(characterList->table);
			SQLINTEGER         nameSize  = sizeof(characterList->table->name);
			TB_CHARACTER_LIST* character = characterList->table;

			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, character++ )
			{
				SQLGetData( mHstmt, param=1, SQL_C_SLONG,   &character->idx,          0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_WCHAR,   character->name,          nameSize, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->level ,       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->race,         0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->gender,       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->job,          0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->hair,         0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SSHORT,  &character->hairColor,    0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->face,         0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->activeWeapon, 0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++param, SQL_C_SSHORT,  &character->mapNum,       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->standIdx,     0,        &(strLenOrInd=0) );

                SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->exp,            0,      &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->skillLevel,     0,      &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->skillExp,       0,      &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->skillPointTotal,0,      &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->guild_index,    0,      &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &character->delWaitTime,    0,      &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &character->apply,          0,      &(strLenOrInd=0) );

                characterList->rowCount++;
			}

			cbLength += (characterList->rowCount * tableSize);
			SQLClose( mHstmt );
		}
		else
			return retcode;
	}

	/// 장비 SELECT
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_EQUIP_SELECT(?)}";

		SQLINTEGER  itemDefineIndex = 0;
		SQLSMALLINT number          = 0;
		SQLCHAR     enhanced        = 0;

		TB_CHARACTER_LIST* character = characterList->table;

		for ( int i = 0; i < characterList->rowCount; i++, character++ )
		{
			retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &character->idx, 0, &(strLenOrInd=0) );
			retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				{
					SQLGetData( mHstmt, param=1, SQL_C_SLONG,   &itemDefineIndex, 0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_SSHORT,  &number,          0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &enhanced,        0, &(strLenOrInd=0) );

					if ( number >= INVENTORY_EQUIP_BEGIN && number <= INVENTORY_EQUIP_END )
					{
						character->equipItem[ number ] = itemDefineIndex;
						character->enhancedItem[ number ] = enhanced;
					}
				}
				SQLClose( mHstmt );
			}
		}
	}

	return retcode;
}

// CharacterName Method
SQLRETURN cSQLGameStmt::CharacterName(CHARACTER_NAME* characterName)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_NAME(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       nameSize = sizeof(characterName->name);
	long       cbName   = wcslen(characterName->name) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, characterName->name,      nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0,  0, &characterName->retvalue, 0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterInsert Method
// GP_CHARACTER_INSERT 참조.
SQLRETURN cSQLGameStmt::CharacterInsert(CHARACTER_INSERT* characterInsert, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       nameSize = sizeof(characterInsert->name);
	long       cbName   = wcslen(characterInsert->name) ? SQL_NTS : SQL_NULL_DATA;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->idx,       0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_WCHAR,   SQL_WVARCHAR, 16, 0, characterInsert->name,       nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &characterInsert->race,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &characterInsert->gender,    0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->job,       0,        &(strLenOrInd=0) );
	
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->hair,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &characterInsert->hairColor, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->face,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_FLOAT,   SQL_REAL,     0,  0, &characterInsert->xPos,      0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_FLOAT,   SQL_REAL,     0,  0, &characterInsert->yPos,      0,        &(strLenOrInd=0) );
	
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->hp,            0,    &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->mp,            0,    &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_USHORT,  SQL_SMALLINT, 0,  0, &characterInsert->bagEnd,        0,    &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_USHORT,  SQL_SMALLINT, 0,  0, &characterInsert->wearEnd,       0,    &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_USHORT,  SQL_SMALLINT, 0,  0, &characterInsert->wareHouseEnd,  0,    &(strLenOrInd=0) );
    
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_USHORT,  SQL_SMALLINT, 0,  0, &characterInsert->makeSkillEnd,  0,    &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0,  0, &characterInsert->standIdx,      0,    &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->userIdx,       0,    &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0,  0, &characterInsert->retvalue,      0,    &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table     = characterInsert->table;
		u_long        tableSize = sizeof(characterInsert->table);

        for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetFullInv( mHstmt, table );
			characterInsert->rowCount++;
		}
		cbLength += (characterInsert->rowCount * tableSize);

		SQLClose( mHstmt );
	}
	return retcode;
}

// CharacterDeleteWait Method
// GP_CHARACTER_DELETE_WAIT 참조.
SQLRETURN cSQLGameStmt::CharacterDeleteWait(CHARACTER_DELETE_WAIT* characterDelWait )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_DELETE_WAIT(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelWait->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelWait->userIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelWait->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_TINYINT, &characterDelWait->level,           0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelWait->exp,             0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &characterDelWait->skillLevel,      0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelWait->skillExp,        0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelWait->skillPointTotal, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelWait->guild_index,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &characterDelWait->standIdx,        0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelWait->delWaitTime,     0, &(strLenOrInd=0) );
		}
		SQLClose( mHstmt );
	}

	return retcode;
}

// CharacterDeleteCancel Method
// GP_CHARACTER_DELETE_CANCEL 참조.
SQLRETURN cSQLGameStmt::CharacterDeleteCancel(CHARACTER_DELETE_CANCEL* characterDelCancel )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_DELETE_CANCEL(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelCancel->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelCancel->userIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelCancel->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_TINYINT, &characterDelCancel->level,           0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelCancel->exp,             0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &characterDelCancel->skillLevel,      0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelCancel->skillExp,        0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelCancel->skillPointTotal, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelCancel->guild_index,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &characterDelCancel->standIdx,        0, &(strLenOrInd=0) );
		}
		SQLClose( mHstmt );
	}

	return retcode;
}

// CharacterDelete Method
// GP_CHARACTER_DELETE 참조.
SQLRETURN cSQLGameStmt::CharacterDelete(CHARACTER_DELETE* characterDelete)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_DELETE(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,       SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelete->idx,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,       SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelete->userIdx,       0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT,SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelete->guild_index,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,      SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDelete->retvalue,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_TINYINT, &characterDelete->level,           0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelete->exp,             0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &characterDelete->skillLevel,      0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelete->skillExp,        0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &characterDelete->skillPointTotal, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &characterDelete->standIdx,        0, &(strLenOrInd=0) );
		}
		SQLClose( mHstmt );
	}

	return retcode;
}

// CharacterUpdate Method
// GP_CHARACTER_UPDATE 참조.
SQLRETURN cSQLGameStmt::CharacterUpdate(CHARACTER_UPDATE* characterUpdate)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_UPDATE(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->idx,                0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &characterUpdate->mapNumber,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_FLOAT,   SQL_REAL,     0, 0, &characterUpdate->xPos,               0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_FLOAT,   SQL_REAL,     0, 0, &characterUpdate->yPos,               0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->hp,                 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->mp,                 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->tarotPoint,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->titleIndex,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->guildIndex,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &characterUpdate->guildPosition,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &characterUpdate->level,              0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->exp,                0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &characterUpdate->skillLevel,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->skillExp,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->skillPointRemain,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->skillPointTotal,    0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_ULONG,   SQL_INTEGER,  0, 0, &characterUpdate->makeSkill1_Exp,     0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_ULONG,   SQL_INTEGER,  0, 0, &characterUpdate->makeSkill2_Exp,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->expRecover,        0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &characterUpdate->expRecoverendDate, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterUpdate->retvalue,           0, &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterSelect Method
// GP_CHARACTER_SELECT 참조.
SQLRETURN cSQLGameStmt::CharacterSelect(CHARACTER_SELECT* characterSelect)
{
    SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT n;

	TB_CHARACTER* character = &characterSelect->character;
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_SELECT(?, ?)}";

        retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterSelect->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterSelect->userIdx,      0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			SQLINTEGER nameSize = sizeof(character->name);

			if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			{
				SQLGetData( mHstmt, n=1, SQL_C_SLONG,   &character->idx,               0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_WCHAR,   &character->name[0],           nameSize, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_TINYINT, &character->level,             0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_TINYINT, &character->race,              0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_TINYINT, &character->gender,            0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->job,               0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->hair,              0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SSHORT,  &character->hairColor,         0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->face,              0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_TINYINT, &character->activeWeapon,      0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_SSHORT,  &character->mapNum,            0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_FLOAT,   &character->xPos,              0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_FLOAT,   &character->yPos,              0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->exp,               0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->HP,                0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->MP,                0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_TINYINT, &character->skillLevel,        0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->skillPointRemain,  0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->skillPointTotal,   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->skillExp,          0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->tarotPoint,        0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_SBIGINT, &character->money,             0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SBIGINT, &character->deposit,           0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SLONG,   &character->option1,           0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_UTINYINT,&character->mForceType,        0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mPvPPoint,         0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mFirePoint,        0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mFireFriendly,     0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mWaterPoint,       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mWaterFriendly,    0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mWindPoint,        0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mWindFriendly,     0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mEarthPoint,       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mEarthFriendly,    0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mTitleIndex,	   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,   &character->mGuildIndex,	   0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_TINYINT, &character->mGuildPosition,	   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_UTINYINT,&character->mMakeSkill1,	   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_UTINYINT,&character->mMakeSkillStep1,   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,	&character->mMakeSkillExp1,	   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_UTINYINT,&character->mMakeSkill2,	   0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_UTINYINT,&character->mMakeSkillStep2,   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,	&character->mMakeSkillExp2,	   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_USHORT,	&character->mBagEnd,	       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_USHORT,	&character->mWearEnd,	       0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_USHORT,	&character->mWareHouseEnd,	   0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_USHORT,	&character->mMakeSkillEnd,	   0,        &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_ULONG,	&character->tutorialModeIndex, 0,        &(strLenOrInd=0) );

				SQLGetData( mHstmt, ++n, SQL_C_SLONG,	       &character->mExpRecover,        0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_TYPE_TIMESTAMP, &character->mExpRecoverendDate, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			}

			SQLClose( mHstmt );
		}
		else
			return retcode;
	}

	/// 장비 SELECT
    if ( character->idx > 0 )
	{
		SQLCHAR*   statement = (SQLCHAR*)"{call GP_INVENTORY_EQUIP_SELECT(?)}";

		SQLINTEGER  itemDefineIndex = 0;
		SQLSMALLINT number          = 0;
		SQLCHAR     enhanced        = 0;

		retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterSelect->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			{
				SQLGetData( mHstmt, n=1, SQL_C_LONG,  &itemDefineIndex, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_SHORT, &number,          0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++n, SQL_C_CHAR,  &enhanced,        0, &(strLenOrInd=0) );

				if ( number >= INVENTORY_EQUIP_BEGIN && number <= INVENTORY_EQUIP_END )
				{
					character->equipItem[ number ] = itemDefineIndex;
					character->enhancedItem[ number ] = enhanced;
				}
			}
			SQLClose( mHstmt );
		}
    }

    return retcode;
}

// CharacterMoney Method
// GP_CHARACTER_MONEY 참조.
SQLRETURN cSQLGameStmt::CharacterMoney(CHARACTER_MONEY* characterMoney)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterMoney->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterMoney->money,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterMoney->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterDeposit Method
// GP_CHARACTER_DEPOSIT 참조.
SQLRETURN cSQLGameStmt::CharacterDeposit(CHARACTER_DEPOSIT* characterDeposit)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_DEPOSIT(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDeposit->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDeposit->money,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterDeposit->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterTarotPoint Method
SQLRETURN cSQLGameStmt::CharacterTarotPoint(CHARACTER_TAROT_POINT* characterTarotPoint)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_TAROT_POINT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterTarotPoint->idx,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterTarotPoint->point, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterActiveWeapon Method
// GP_CHARACTER_ACTIVE_WEAPON 참조.
SQLRETURN cSQLGameStmt::CharacterActiveWeapon(CHARACTER_ACTIVE_WEAPON* characterActiveWeapon)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_ACTIVE_WEAPON(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_ULONG,   SQL_INTEGER, 0, 0, &characterActiveWeapon->idx,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &characterActiveWeapon->activeWeapon, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterOption Method
// GP_CHARACTER_OPTION 참조.
SQLRETURN cSQLGameStmt::CharacterOption(CHARACTER_OPTION* characterOption)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_OPTION(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterOption->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterOption->option1,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterOption->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}


SQLRETURN cSQLGameStmt::CharacterForce( CHARACTER_FORCE* characterForce )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_FORCE(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &characterForce->mIdx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_UTINYINT, SQL_INTEGER, 0, 0, &characterForce->mForceType,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterForce->mRetValue, 0, &(strLenOrInd=0) );
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


SQLRETURN cSQLGameStmt::CharacterSkillReset( CHARACTER_SKILL_RESET* reset, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_RESET( ?, ?, ?, ?, ?, ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &reset->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &reset->mRace, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &reset->mJobStep, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &reset->mJob, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &reset->mInventoryIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &reset->mRetvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
        reset->mRowCount = 0;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS ; ++i )
		{
			n = 0;
			SQLGetData( mHstmt, ++n, SQL_C_ULONG, &reset->mTable[ i ].mSkillIdx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_UTINYINT, &reset->mTable[ i ].mSkillStep, 0, &(strLenOrInd=0) );

			reset->mTable[ i ].mEndCoolTime = 0;

			reset->mRowCount++;
		}
		cbLength += ( reset->mRowCount * sizeof(reset->mTable) );

		if ( SQLMoreResults( mHstmt ) == SQL_SUCCESS )
		{
			if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				SQLGetFullInv( mHstmt, &reset->mItem );
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::CharacterThemeReset( CHARACTER_THEME_RESET* reset, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_THEME_RESET( ?, ?, ?, ?, ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &reset->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_TINYINT, 0, 0, &reset->mThemeIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &reset->mThemeMode, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &reset->mInventoryIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &reset->mRetvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
        reset->mRowCount = 0;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS ; ++i )
		{
			n = 0;
			SQLGetData( mHstmt, ++n, SQL_C_SHORT,          &reset->mTable[ i ].themeIdx,  0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SHORT,          &reset->mTable[ i ].roomIdx,   0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_TINYINT,        &reset->mTable[ i ].themeMode, 0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_TYPE_TIMESTAMP, &reset->mTable[ i ].lastDate,  SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_TYPE_TIMESTAMP, &reset->mTable[ i ].validThru, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			reset->mRowCount++;
		}
		cbLength += ( reset->mRowCount * sizeof(reset->mTable) );

		if ( SQLMoreResults( mHstmt ) == SQL_SUCCESS )
		{
			if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				SQLGetFullInv( mHstmt, &reset->mItem );
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

// CharacterBlockSelect Method
SQLRETURN cSQLGameStmt::CharacterBlockSelect(CHARACTER_BLOCK_SELECT* characterBlockSelect)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_BLOCK_SELECT(?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterBlockSelect->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterBlockSelect->rowCount,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, &characterBlockSelect->validTrue, 0, &(strLenOrInd=0) );
		}
		SQLClose( mHstmt );
	}
	return retcode;
}

// CharacterBlockSelectList Method
SQLRETURN cSQLGameStmt::CharacterBlockSelectList(CHARACTER_BLOCK_SELECT_LIST* characterBlockSelectList, u_long& cbLength)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_BLOCK_SELECT_LOG(?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long       nameSize = sizeof(characterBlockSelectList->characterName);
	long       cbName   = wcslen(characterBlockSelectList->characterName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, &characterBlockSelectList->characterName, nameSize, &cbName );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,   0, 0, &characterBlockSelectList->retValue,    0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		NO_CHAT_CHARACTER* table     = characterBlockSelectList->table;
		u_long			   tableSize = sizeof(characterBlockSelectList->table);
		SQLINTEGER		   nameSize  = sizeof(characterBlockSelectList->table->characterName);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetData( mHstmt, param=2, SQL_C_WCHAR,		   table->characterName, nameSize,		    &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR,		   table->adminName,     nameSize,		    &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->registDate, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->validThru,    SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			characterBlockSelectList->rowCount++;
		}
		cbLength += (characterBlockSelectList->rowCount * tableSize);

		SQLClose( mHstmt );
	}
	return retcode;

}

// CharacterBlockInsert Method
SQLRETURN cSQLGameStmt::CharacterBlockInsert(CHARACTER_BLOCK_INSERT* characterBlockInsert)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_BLOCK_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long       nameSize = sizeof(characterBlockInsert->characterName);
	long       cbName   = wcslen(characterBlockInsert->characterName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_WCHAR,   SQL_WVARCHAR, 16, 0, characterBlockInsert->characterName,	     nameSize, &cbName   );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,   0, 0, &characterBlockInsert->adminCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,   0, 0, &characterBlockInsert->validTrue,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,   0, 0, &characterBlockInsert->blockCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG,    SQL_INTEGER,   0, 0, &characterBlockInsert->channelNum,	     0, &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterGmName Method
SQLRETURN cSQLGameStmt::CharacterGmName(CHARACTER_GM_NAME* name)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_GM_NAME(?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long       nameSize = sizeof(name->characterName);
	long       cbName   = wcslen(name->characterName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &name->characterIndex, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, name->characterName, nameSize, &cbName );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &name->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterInvenSize Method
SQLRETURN cSQLGameStmt::CharacterInvenSize(CHARACTER_INVEN_SIZE* invenSize)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_INVEN_SIZE(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,   SQL_C_SLONG,    SQL_INTEGER,  0, 0, &invenSize->idx,         0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_USHORT,   SQL_SMALLINT, 0, 0, &invenSize->bag,         0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_USHORT,   SQL_SMALLINT, 0, 0, &invenSize->wear,        0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_USHORT,   SQL_SMALLINT, 0, 0, &invenSize->wareHouse,   0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_USHORT,   SQL_SMALLINT, 0, 0, &invenSize->makeSkill,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,  SQL_C_SLONG,    SQL_INTEGER,  0, 0, &invenSize->retvalue,    0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// CharacterTTL Method.
SQLRETURN cSQLGameStmt::CharacterTTL(CHARACTER_TTL* characterTTL)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_TTL(?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterTTL->idx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterTTL->ttl, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &characterTTL->channelNum, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::CharacterChannelSelect( CHARACTER_CHANNEL* characterChannel )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_CHANNEL(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	long       nameSize = sizeof(characterChannel->name);
	long       cbName   = wcslen(characterChannel->name) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0,  0, &characterChannel->channelNum, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, characterChannel->name, nameSize, &cbName          );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::CharacterGMThemeReset(CHARACTER_GM_THEME_RESET* reset, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_GM_THEME_RESET( ?, ?, ?, ?, ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLUSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &reset->mGMCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &reset->mCharacterIdx,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_TINYINT, 0, 0, &reset->mThemeIdx,	      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &reset->mThemeMode,	  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &reset->mRetvalue,	      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS ; ++i )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SHORT,          &reset->mTable[ i ].themeIdx,  0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SHORT,          &reset->mTable[ i ].roomIdx,   0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &reset->mTable[ i ].themeMode, 0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &reset->mTable[ i ].lastDate,  SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &reset->mTable[ i ].validThru, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			reset->mRowCount++;
		}
		cbLength += ( reset->mRowCount * sizeof(reset->mTable) );
		
		SQLClose( mHstmt );
	}

	return retcode;
}



// CharacterKickSelect Method
SQLRETURN cSQLGameStmt::CharacterGMKickSelect(CHARACTER_KICK_SELECT* characterKickSelect)
{
	SQLCHAR*     statement = (SQLCHAR*)"{call GP_CHARACTER_GM_KICK_SELECT(?, ?, ?)}";
	SQLINTEGER   strLenOrInd;
	SQLRETURN    retcode;
	SQLUSMALLINT param;

	long       nameSize = sizeof(characterKickSelect->characterName);
	long       cbName   = wcslen(characterKickSelect->characterName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_WCHAR,   SQL_WVARCHAR, 16, 0, characterKickSelect->characterName,	   nameSize, &cbName   );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,   0, 0, &characterKickSelect->kickCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG,    SQL_INTEGER,   0, 0, &characterKickSelect->channelNum,	   0, &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InventorySelect Method.
SQLRETURN cSQLGameStmt::InventorySelect(TB_INVENTORY* inventory)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_INVENTORY(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventory->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			SQLGetFullInv( mHstmt, inventory );

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryInsert Method
// GP_INVENTORY_INSERT 참조.
SQLRETURN cSQLGameStmt::InventoryInsert(INVENTORY_INSERT* inventoryInsert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryNumner;
	SQLSMALLINT inventoryCount;
	SQLCHAR     inventoryEnhanced;
	SQLINTEGER  inventoryCardSlot1;
	SQLINTEGER  inventoryCardSlot2;
	SQLINTEGER  inventoryCardSlot3;
	SQLINTEGER  inventoryCardSlot4;
	SQLINTEGER  inventoryCardSlot5;
	SQLCHAR     inventorySeal; 

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryInsert->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryIdx,                  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemDefineIndex,               0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryNumner,               0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryCount,                0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &inventoryEnhanced,             0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryCardSlot1,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryCardSlot2,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryCardSlot3,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryCardSlot4,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryCardSlot5,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &inventorySeal,                 0, &(strLenOrInd=0) );

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table = inventoryInsert->table;
		for ( long i = 0; i < inventoryInsert->rowCount; i++, table++ )
		{
			itemDefineIndex    = table->itemDefineIndex;
			inventoryNumner    = table->number;
			inventoryCount     = table->count;
			inventoryEnhanced  = table->enhanced;
			inventoryCardSlot1 = table->cardSlot1;
			inventoryCardSlot2 = table->cardSlot2;
			inventoryCardSlot3 = table->cardSlot3;
			inventoryCardSlot4 = table->cardSlot4;
			inventoryCardSlot5 = table->cardSlot5;
			inventorySeal      = table->seal;
			retcode            = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				table->idx = inventoryIdx;

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventorySelect Method
// GP_INVENTORY_SELECT 참조.
//   @CHARACTER_IDX [int] --캐릭터 고유번호
SQLRETURN cSQLGameStmt::InventorySelect(INVENTORY_SELECT* inventorySelect, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_INVENTORY_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventorySelect->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table     = inventorySelect->table;
		u_long        tableSize = sizeof(inventorySelect->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetFullInv( mHstmt, table );
			inventorySelect->rowCount++;
		}

		cbLength += (inventorySelect->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryCashSelect Method
SQLRETURN cSQLGameStmt::InventoryCashSelect(INVENTORY_SELECT* inventorySelect, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_INVENTORY_CASH_SELECT( ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventorySelect->characterIdx,0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table     = inventorySelect->table;
		u_long        tableSize = sizeof(inventorySelect->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetFullInv( mHstmt, table );
			inventorySelect->rowCount++;
		}

		cbLength += (inventorySelect->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryDelete Method
SQLRETURN cSQLGameStmt::InventoryDelete(INVENTORY_DELETE* inventoryDelete)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_DELETE(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryDelete->table.idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	return SQLClose( retcode, mHstmt );
}

// InventoryCooltimeSelect Method
SQLRETURN cSQLGameStmt::InventoryCooltimeSelect(INVENTORY_COOLTIME_SELECT* inventoryCooltimeSelect, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_COOLTIME_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryCooltimeSelect->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY_COOLTIME* table     = inventoryCooltimeSelect->table;
		u_long                 tableSize = sizeof(inventoryCooltimeSelect->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &table->itemDefineIndex, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->cooltime2,       0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->cooltime,        0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &table->cooltime1,       0, &(strLenOrInd=0) );

			inventoryCooltimeSelect->rowCount++;
		}

		cbLength += (inventoryCooltimeSelect->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryCooltimeSelect Method
SQLRETURN cSQLGameStmt::InventoryCooltimeInsert(INVENTORY_COOLTIME_INSERT* inventoryCooltimeInsert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_COOLTIME_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	TB_INVENTORY_COOLTIME* table = inventoryCooltimeInsert->table;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryCooltimeInsert->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->itemDefineIndex,                0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->cooltime1,                      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->cooltime2,                      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->cooltime,                       0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InventoryRemove Method
// GP_INVENTORY_REMOVE
//   @CHARACTER_IDX    [int]       --캐릭터 고유번호
//   @RETVALUE         [int]OUTPUT --반환값
SQLRETURN cSQLGameStmt::InventoryRemove(INVENTORY_REMOVE* inventoryRemove)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_REMOVE(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryRemove->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InventoryUse Method
// GP_INVENTORY_USE
SQLRETURN cSQLGameStmt::InventoryUse(INVENTORY_USE* inventoryUse)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_USE(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	TB_INVENTORY* table = inventoryUse->table;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &table->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &table->count, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
		}
		SQLClose( mHstmt );
	}
	return retcode;
}

// InventorySwap Method.
// GP_INVENTORY_SWAP 참조.
//   @IDX1    [int]      --고유번호1
//   @IDX2    [int]      --고유번호2
SQLRETURN cSQLGameStmt::InventorySwap(INVENTORY_SWAP* inventorySwap)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_SWAP(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &inventorySwap->idx1,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &inventorySwap->number1,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &inventorySwap->idx2,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &inventorySwap->number2,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &inventorySwap->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InventoryMove Method.
// GP_INVENTORY_MOVE 참조.
//   @CHARACTER_IDX [int]      --캐릭터 고유번호
//   @IDX           [int]      --고유번호
//   @NUMBER        [smallint] --번호
SQLRETURN cSQLGameStmt::InventoryMove(INVENTORY_MOVE* inventoryMove)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_MOVE(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryMove->idx,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryMove->number, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// InventoryExcept Method.
// GP_INVENTORY_EXCEPT 참조.
//   @CHARACTER_IDX [int]      --캐릭터 고유번호
//   @IDX           [int]      --고유번호
//   @NUMBER        [smallint] --번호
SQLRETURN cSQLGameStmt::InventoryExcept(INVENTORY_EXCEPT* inventoryExcept)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_EXCEPT(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLSMALLINT inventoryNumner;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner, 0, &(strLenOrInd=0) );

		for ( long i = 0; i < inventoryExcept->rowCount; i++ )
		{
			inventoryIdx    = inventoryExcept->table[ i ].idx;
			inventoryNumner = inventoryExcept->table[ i ].number;
			retcode         = SQLExecute( mHstmt );

			if ( !SQL_SUCCEEDED( retcode ) )
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryMerge Method.
// GP_INVENTORY_MERGE 참조.
SQLRETURN cSQLGameStmt::InventoryMerge(INVENTORY_MERGE* inventoryMerge, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_MERGE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &inventoryMerge->idx1,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &inventoryMerge->idx2,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &inventoryMerge->capacity, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table     = inventoryMerge->table;
		u_long        tableSize = sizeof(inventoryMerge->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetBaseInv( mHstmt, table );
			inventoryMerge->rowCount++;
		}

		cbLength += (inventoryMerge->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// GP_INVENTORY_DIVIDE 참조.
SQLRETURN cSQLGameStmt::InventoryDivide(INVENTORY_DIVIDE* divide)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_DIVIDE(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryCount;
	SQLSMALLINT inventoryNumner;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &divide->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,         0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,      0, &(strLenOrInd=0) );

		TB_INVENTORY* table = divide->table;
		for ( long i = 0; i < ROW_INVENTORY_DIVIDE; i++, table++ )
		{
			inventoryIdx    = table->idx;
			itemDefineIndex = table->itemDefineIndex;
			inventoryCount  = table->count;
			inventoryNumner = table->number;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				table->idx = inventoryIdx;
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemSell Method.
// GP_ITEM_SELL 참조.
SQLRETURN cSQLGameStmt::ItemSell(ITEM_SELL* itemSell)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_SELL(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryCount;
	SQLSMALLINT inventoryNumner;
	SQLCHAR     inventorySeal;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemSell->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryIdx,           0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemDefineIndex,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryCount,         0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryNumner,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &inventorySeal,         0, &(strLenOrInd=0) );

		TB_INVENTORY* table = itemSell->table;
		for ( long i = 0; i < itemSell->rowCount; i++, table++ )
		{
			inventoryIdx    = table->idx; 
			itemDefineIndex = table->itemDefineIndex;
			inventoryCount  = table->count;
			inventoryNumner = table->number;
			inventorySeal   = table->seal;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				{
					SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &table->count, 0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
				}
				table->idx = inventoryIdx;

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemBuy Method.
// GP_ITEM_BUY 참조.
SQLRETURN cSQLGameStmt::ItemBuy(ITEM_BUY* itemBuy)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_BUY(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryCount;
	SQLSMALLINT inventoryNumner;
	SQLCHAR     inventorySeal;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemBuy->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryIdx,          0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemDefineIndex,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryCount,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryNumner,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &inventorySeal,         0, &(strLenOrInd=0) );

		TB_INVENTORY* table = itemBuy->table;
		for ( long i = 0; i < itemBuy->rowCount; i++, table++ )
		{
			inventoryIdx    = table->idx; 
			itemDefineIndex = table->itemDefineIndex;
			inventoryCount  = table->count;
			inventoryNumner = table->number;
			inventorySeal   = table->seal;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				{
					SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &table->count, 0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
				}
				table->idx = inventoryIdx;

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemCollect Method.
// GP_ITEM_COLLECT 참조.
SQLRETURN cSQLGameStmt::ItemCollect(ITEM_COLLECT* itemCollect)
{
	TB_INVENTORY* update = &itemCollect->table[0];
    TB_INVENTORY* insert = &itemCollect->table[1];
    TB_INVENTORY* remove = &itemCollect->table[2];

    SQLINTEGER  strLenOrInd;
    SQLRETURN   retcode = 0;
    SQLSMALLINT	param;

    // 기존 등록된 카드 삭제
    if ( remove->idx > 0 )
    {
        SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_COLLECT_REMOVE(?, ?)}";

        retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &remove->idx,   0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT,  0, 0, &remove->apply, 0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( !SQL_SUCCEEDED( retcode ) )
            return retcode;

        SQLClose( mHstmt );
    }

    if ( insert->idx == update->idx )
    {
        SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_MOVE(?, ?)}";

        retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &insert->idx,    0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &insert->number, 0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( !SQL_SUCCEEDED( retcode ) )
            return retcode;

        SQLClose( mHstmt );
    }
    else
    {
        // 기존 카드 count Update
        {
            SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_COLLECT_UPDATE(?, ?)}";

            retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &update->idx,   0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &update->count, 0, &(strLenOrInd=0) );
            retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

            if ( !SQL_SUCCEEDED( retcode ) )
                return retcode;

            SQLClose( mHstmt );
        }

        // 새 카드 등록
        {
            SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_COLLECT_INSERT(?, ?, ?, ?, ?)}";

            retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemCollect->characterIdx,  0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &insert->idx,                0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &insert->itemDefineIndex,    0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &insert->count,              0, &(strLenOrInd=0) );
            retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &insert->number,             0, &(strLenOrInd=0) );
            retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

            if ( !SQL_SUCCEEDED( retcode ) )
                return retcode;

            SQLClose( mHstmt );
        }
    }

    return retcode;
}

// ItemCollectChange Method.
// GP_ITEM_COLLECT_CHANGE 참조.
SQLRETURN cSQLGameStmt::ItemCollectChange(ITEM_COLLECT_CHANGE* itemCollect)
{
	TB_INVENTORY* changeItem  = &itemCollect->changeItem;
    TB_INVENTORY* tarotCard   = &itemCollect->table[0];

    SQLINTEGER  strLenOrInd;
    SQLRETURN   retcode = 0;
    SQLSMALLINT	param;

    SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_COLLECT_CARD_CHANGE(?, ?, ?, ?, ?, ?, ?, ?)}";

    retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemCollect->characterIdx,  0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &itemCollect->begin,         0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &itemCollect->end,           0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &changeItem->idx,            0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &changeItem->itemDefineIndex,0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &changeItem->count,          0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &changeItem->number,         0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemCollect->retvalue,      0, &(strLenOrInd=0) );
    retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

    if ( SQL_SUCCEEDED( retcode ) && itemCollect->retvalue == 0 )
    {
        SQLClose( mHstmt );

        for ( long i = 0; i < itemCollect->rowCount; i++, tarotCard++ )
        {
            statement = (SQLCHAR*)"{call GP_ITEM_COLLECT_SELECT_INVENTORY(?)}";

            retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &tarotCard->idx, 0, &(strLenOrInd=0) );
            retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

            if ( SQL_SUCCEEDED( retcode ) )
            {
                if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
                    SQLGetBaseInv( mHstmt, tarotCard );

                SQLClose( mHstmt );
            }
        }
    }

    return retcode;
}

// ItemGet Method.
// GP_ITEM_GET 참조.
SQLRETURN cSQLGameStmt::ItemGet(ITEM_GET* itemGet)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_GET(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryNumber;
	SQLSMALLINT inventoryCount;
	SQLCHAR     inventorySeal;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryIdx,          0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemGet->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemDefineIndex,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryNumber,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryCount,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &inventorySeal,         0, &(strLenOrInd=0) );

		TB_INVENTORY* table = itemGet->table;
		for ( long i = 0; i < itemGet->rowCount; i++, table++ )
		{
			inventoryIdx    = table->idx;
			itemDefineIndex = table->itemDefineIndex;
			inventoryNumber = table->number;
			inventoryCount  = table->count;
			inventorySeal   = table->seal;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				table->idx   = inventoryIdx;
				table->count = inventoryCount;
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemPartyGive Method.
// GP_ITEM_PARTY_GIVE 참조.
SQLRETURN cSQLGameStmt::ItemPartyGive(ITEM_PARTY_GIVE* itemPartyGive)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_GET(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;


	SQLINTEGER  characterIdx = itemPartyGive->takeCharacterIdx;
    SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryNumber;
	SQLSMALLINT inventoryCount;
	SQLCHAR     inventorySeal;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryIdx,      0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &characterIdx,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemDefineIndex,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryNumber,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &inventoryCount,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &inventorySeal,     0, &(strLenOrInd=0) );

		TB_INVENTORY* table = itemPartyGive->table;
		for ( long i = 0; i < itemPartyGive->rowCount; i++, table++ )
		{
			inventoryIdx    = table->idx;
			itemDefineIndex = table->itemDefineIndex;
			inventoryNumber = table->number;
			inventoryCount  = table->count;
			inventorySeal   = table->seal;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				table->idx   = inventoryIdx;
				table->count = inventoryCount;
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}

	return retcode;
}

// ItemUse Method.
SQLRETURN cSQLGameStmt::ItemUse(ITEM_USE* itemUse)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_USE(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLSMALLINT count;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &count,        0, &(strLenOrInd=0) );

		TB_INVENTORY* table = itemUse->table;
		for ( long i = 0; i < itemUse->rowCount; i++, table++ )
		{
			inventoryIdx = table->idx;
			count        = table->count;
			retcode      = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				{
					SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &table->count, 0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
				}

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemExchange Method.
SQLRETURN cSQLGameStmt::ItemExchange(ITEM_EXCHANGE* itemExchange)
{
	SQLINTEGER  strLenOrInd = 0;
	SQLRETURN   retcode     = SQL_SUCCESS;
	SQLSMALLINT	param;

	for ( int i0 = 0; i0 < itemExchange->rowCount; i0++ )
	{
		if ( itemExchange->table[ i0 ].characterMoney != 0 )
		{
			SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";

			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemExchange->table[ i0 ].characterIdx,   0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemExchange->table[ i0 ].characterMoney, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemExchange->table[ i0 ].retvalue,       0, &(strLenOrInd=0) );
			retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

			if ( !(SQL_SUCCEEDED( retcode )) )
				return retcode;

			SQLClose( mHstmt );
		}

		if ( itemExchange->table[ i0 ].retvalue == 0 && itemExchange->table[ i0 ].rowCount > 0 )
		{
			SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_EXCHANGE(?, ?, ?)}";

			SQLINTEGER  characterIdx = itemExchange->table[ i0 ].characterIdx;
			SQLINTEGER  inventoryIdx;
			SQLSMALLINT inventoryNumber;

			retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
			if ( !(SQL_SUCCEEDED( retcode )) )
				return retcode;

			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &characterIdx,    0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,    0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumber, 0, &(strLenOrInd=0) );

			TB_INVENTORY* table = itemExchange->table[ i0 ].table;
			for ( long i1 = 0; i1 < itemExchange->table[ i0 ].rowCount; i1++, table++ )
			{
				inventoryIdx    = table->idx;
				inventoryNumber = table->number;
				retcode         = SQLExecute( mHstmt );

				if ( !SQL_SUCCEEDED( retcode ) )
					return retcode;
			}

			SQLClose( mHstmt );
		}
	}

	return retcode;
}

// ItemMix Method.
SQLRETURN cSQLGameStmt::ItemMix(ITEM_MIX* itemMix)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_MIX(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryCount;
	SQLSMALLINT inventoryNumner;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemMix->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,          0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,       0, &(strLenOrInd=0) );

		TB_INVENTORY* table = itemMix->table;
		for ( long i = 0; i < itemMix->rowCount; i++, table++ )
		{
			inventoryIdx    = table->idx; 
			itemDefineIndex = table->itemDefineIndex;
			inventoryCount  = table->count;
			inventoryNumner = table->number;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				{
					SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &table->count, 0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
				}
				table->idx = inventoryIdx;

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemSummonPost Method.
SQLRETURN cSQLGameStmt::ItemSummonPost( ITEM_SUMMON_POST* summonPost, u_long& cbLength )
{
    SQLINTEGER  strLenOrInd;
    SQLSMALLINT	param;
    SQLRETURN   retcode;

    // 우편 SELECT
    {
        SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_SELECT(?, ?, ?, ?)}";

        retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &summonPost->characterIdx, 0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &summonPost->startPage,    0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &summonPost->endPage,      0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &summonPost->noneRead,     0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( SQL_SUCCEEDED( retcode ) )
        {
            POST_RESULT* post      = summonPost->post;
            u_long       tableSize = sizeof(summonPost->post);

            SQLINTEGER   nameSize    = sizeof(post->fromName);
            SQLINTEGER   titleSize   = sizeof(post->contenTtitle);
            SQLINTEGER   messageSize = sizeof(post->contentMessage);

            summonPost->postCount = 0;
            while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
            {
                SQLGetData( mHstmt, param=1, SQL_C_SLONG,          &post->idx,                   0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &post->registerDate,          SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->validThru,             0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &post->type,                  0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &post->status,                0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->fromIdx,               0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          post->fromName,               nameSize,          &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          post->contenTtitle,           titleSize,         &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          post->contentMessage,         messageSize,       &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->receiptPrice,          0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->receiptSalesCharges,   0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->receiptInventory.idx,  0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SHORT,          &post->receiptInventoryCount, 0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->inventory1.idx,        0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->inventory2.idx,        0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->inventory3.idx,        0,                 &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_LONG,           &post->money,                 0,                 &(strLenOrInd=0) );

                post++;
                summonPost->postCount++;
            }

            cbLength += (summonPost->postCount * tableSize);
            SQLClose( mHstmt );

            post = summonPost->post;
            for ( long i = 0; i < summonPost->postCount; i++, post++ )
            {
                if ( post->receiptInventory.idx )
                    InventorySelect( &post->receiptInventory );

                if ( post->inventory1.idx )
                    InventorySelect( &post->inventory1 );

                if ( post->inventory2.idx )
                    InventorySelect( &post->inventory2 );

                if ( post->inventory3.idx )
                    InventorySelect( &post->inventory3 );
            }
        }
        else
            return retcode;
    }

    // 소환 주문서 사용
    {
        SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_USE(?)}";
        SQLINTEGER  strLenOrInd;
        SQLRETURN   retcode;
        SQLSMALLINT	param;

        TB_INVENTORY* inventory = &(summonPost->inventory);

        retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventory->idx, 0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( SQL_SUCCEEDED( retcode ) )
        {
            if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
            {
                SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &inventory->count, 0, &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &inventory->apply, 0, &(strLenOrInd=0) );
            }
            SQLClose( mHstmt );
        }
    }

	return retcode;
}

// ItemSummonAgent Method.
SQLRETURN cSQLGameStmt::ItemSummonAgent( ITEM_SUMMON_AGENT* summonAgent, u_long& cbLength )
{
    SQLINTEGER  strLenOrInd;
    SQLRETURN   retcode;
    SQLSMALLINT	param;

    // 판매 대행 SELECT
    {
        SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_OWNER(?, ?, ?)}";

        retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_LONG, SQL_INTEGER, 0, 0, &summonAgent->characterIdx, 0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &summonAgent->startPage,    0, &(strLenOrInd=0) );
        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG, SQL_INTEGER, 0, 0, &summonAgent->endPage,      0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( SQL_SUCCEEDED( retcode ) )
        {
            u_long                   tableSize = sizeof(summonAgent->agent);
            ITEM_AGENT_RESULT_OWNER* result    = summonAgent->agent;

            summonAgent->rowCount = 0;
            for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
            {
                SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,       0, &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru, 0, &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,     0, &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx, 0, &(strLenOrInd=0) );
                summonAgent->rowCount++;
            }
            SQLClose( mHstmt );

            result = summonAgent->agent;
            for ( long i = 0; i < summonAgent->rowCount; i++, result++ )
            {
                InventorySelect( &result->table );
            }

            cbLength += (summonAgent->rowCount * tableSize);
        }
        else
            return retcode;
    }

    // 소환 주문서 사용
    {
        SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_USE(?)}";
        SQLINTEGER  strLenOrInd;
        SQLRETURN   retcode;
        SQLSMALLINT	param;

        TB_INVENTORY* inventory = &(summonAgent->inventory);

        retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventory->idx, 0, &(strLenOrInd=0) );
        retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

        if ( SQL_SUCCEEDED( retcode ) )
        {
            if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
            {
                SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &inventory->count, 0, &(strLenOrInd=0) );
                SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &inventory->apply, 0, &(strLenOrInd=0) );
            }
            SQLClose( mHstmt );
        }
    }

    return retcode;
}

// ItemSummon Method.
SQLRETURN cSQLGameStmt::ItemSummonWareHouse( ITEM_SUMMON_WAREHOUSE* summonWare )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_USE(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

    TB_INVENTORY* inventory = &(summonWare->inventory);

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventory->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &inventory->count, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &inventory->apply, 0, &(strLenOrInd=0) );
		}
		SQLClose( mHstmt );
	}

    return retcode;
}

// ItemNpcEvent Method.
SQLRETURN cSQLGameStmt::ItemNpcEvent(ITEM_NPC_EVENT* npcEvent, u_long& cbLength)
{
	SQLRETURN   retcode = 0;

	if( npcEvent->eventType == NPC_EVENT_PWD_CHANGE )
	{
		SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_NPC_EVENT(?, ?, ?, ?)}";
		SQLINTEGER  strLenOrInd;
		SQLSMALLINT	param;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->userIdx,		0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->characterIdx,0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->eventType,	0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->retvalue,    0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) && npcEvent->retvalue == 0 )
		{
			TB_INVENTORY* table     = npcEvent->table;
			u_long        tableSize = sizeof(npcEvent->table);

			npcEvent->rowCount = 0;
			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
			{
				SQLGetFullInv( mHstmt, table );
				npcEvent->rowCount++;
			}

			cbLength += (npcEvent->rowCount * tableSize);
			SQLClose( mHstmt );
		}
	}
	else if( npcEvent->eventType == NPC_EVENT_MAX_LEVEL )
	{
		SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_NPC_EVENT_MAX_LEVEL(?, ?, ?, ?)}";
		SQLINTEGER  strLenOrInd;
		SQLSMALLINT	param;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->userIdx,		0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->characterIdx,0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->eventType,	0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,	SQL_C_SLONG, SQL_INTEGER, 0, 0, &npcEvent->retvalue,    0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) && npcEvent->retvalue == 0 )
		{
			TB_INVENTORY* table     = npcEvent->table;
			u_long        tableSize = sizeof(npcEvent->table);

			npcEvent->rowCount = 0;
			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
			{
				SQLGetFullInv( mHstmt, table );
				npcEvent->rowCount++;
			}

			cbLength += (npcEvent->rowCount * tableSize);
			SQLClose( mHstmt );
		}
	}
	return retcode;
}

// ItemAgentCheck Method.
SQLRETURN cSQLGameStmt::ItemAgentCheck(ITEM_AGENT_CHECK* itemAgentCheck)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_CHECK(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemAgentCheck->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemAgentCheck->rowCount,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemAgentCheck->nextCheck,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemAgentCheck->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ItemAgentSelectName Method.
SQLRETURN cSQLGameStmt::ItemAgentSelectOwner(ITEM_AGENT_SELECT_OWNER* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_OWNER(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_LONG, SQL_INTEGER, 0, 0, &select->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &select->startPage,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG, SQL_INTEGER, 0, 0, &select->endPage,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long                   tableSize = sizeof(select->table);
		ITEM_AGENT_RESULT_OWNER* result;

		result = select->table;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,       0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx, 0, &(strLenOrInd=0) );
			select->rowCount++;
		}
		SQLClose( mHstmt );

		result = select->table;
		for ( long i = 0; i < select->rowCount; i++, result++ )
		{
			InventorySelect( &result->table );
		}

		cbLength += (select->rowCount * tableSize);
	}
	return retcode;
}

// ItemAgentInsert Method.
SQLRETURN cSQLGameStmt::ItemAgentInsert(ITEM_AGENT_INSERT* insert, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_AGENT_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &insert->inventory.idx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &insert->validThru,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &insert->nextCheck,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &insert->price,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &insert->retvalue,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	SQLClose( mHstmt );

	if ( SQL_SUCCEEDED( retcode ) && insert->retvalue == 0)
	{
		// 결과검색.
		InventorySelect( &insert->inventory );

		// 목록검색.
		SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_OWNER(?, ?, ?)}";
		SQLINTEGER  strLenOrInd;
		SQLRETURN   retcode;
		SQLSMALLINT	param;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_LONG, SQL_INTEGER, 0, 0, &insert->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &insert->startPage,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG, SQL_INTEGER, 0, 0, &insert->endPage,      0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			u_long                   tableSize = sizeof(insert->table);
			ITEM_AGENT_RESULT_OWNER* result;

			result = insert->table;
			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
			{
				SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,       0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,     0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx, 0, &(strLenOrInd=0) );
				insert->rowCount++;
			}
			SQLClose( mHstmt );

			result = insert->table;
			for ( long i = 0; i < insert->rowCount; i++, result++ )
			{
				InventorySelect( &result->table );
			}

			cbLength += (insert->rowCount * tableSize);
		}
	}
	return retcode;
}

// ItemAgentDelete Method.
SQLRETURN cSQLGameStmt::ItemAgentDelete(ITEM_AGENT_DELETE* del, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_AGENT_DELETE(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &del->idx,             0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &del->characterIdx,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &del->inventoryNumber, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &del->retvalue,        0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) && del->retvalue == 0 )
	{
		// 결과검색.
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			SQLGetFullInv( mHstmt, &del->result );

		SQLClose( mHstmt );

		// 목록검색.
		SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_OWNER(?, ?, ?)}";
		SQLINTEGER  strLenOrInd;
		SQLRETURN   retcode;
		SQLSMALLINT	param;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_LONG, SQL_INTEGER, 0, 0, &del->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &del->startPage,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG, SQL_INTEGER, 0, 0, &del->endPage,      0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			u_long                   tableSize = sizeof(del->table);
			ITEM_AGENT_RESULT_OWNER* result;

			result = del->table;
			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
			{
				SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,       0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,     0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx, 0, &(strLenOrInd=0) );
				del->rowCount++;
			}
			SQLClose( mHstmt );

			result = del->table;
			for ( long i = 0; i < del->rowCount; i++, result++ )
			{
				InventorySelect( &result->table );
			}

			cbLength += (del->rowCount * tableSize);
		}
	}
	return retcode;
}

// ItemAgentUpdate Method.
SQLRETURN cSQLGameStmt::ItemAgentUpdate(ITEM_AGENT_UPDATE* update, u_long& cbLength)
{
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT	param;

	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_AGENT_UPDATE(?, ?, ?, ?, ?, ?, ?, ?)}";
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->idx,          0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &update->count,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->price,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->retvalue,     0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->sccIdx,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->sciLog.idx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->bciLog.idx,   0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
		SQLClose( mHstmt );
	}
	if ( SQL_SUCCEEDED( retcode ) && update->retvalue == 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_ALL(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";

		long nameSize = sizeof(update->itemDefineName);
		long cbName   = wcslen(update->itemDefineName) ? SQL_NTS : SQL_NULL_DATA;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG,    SQL_INTEGER,  0,  0, &update->startPage,               0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG,    SQL_INTEGER,  0,  0, &update->endPage,                 0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &update->sort,                    0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_WCHAR,   SQL_WVARCHAR, 50, 0, update->itemDefineName,           nameSize, &cbName          );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &update->itemDefineSearchType,    0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SHORT,   SQL_SMALLINT, 0,  0, &update->itemDefineSearchSubType, 0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &update->itemDefineRareLevel,     0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &update->itemLimitLevelB,         0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &update->itemLimitLevelE,         0,        &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &update->inventoryEnhanced,       0,        &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			u_long             tableSize = sizeof(update->table);
			ITEM_AGENT_RESULT* result;

			result = update->table;
			for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
			{
				long characterNameSize = sizeof(result->characterName);

				SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,          0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_WCHAR, result->characterName, characterNameSize, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru,    0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,        0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx,    0,                 &(strLenOrInd=0) );
				update->rowCount++;
			}
			SQLClose( mHstmt );

			result = update->table;
			for ( long i = 0; i < update->rowCount; i++, result++ )
			{
				InventorySelect( &result->table );
			}

			cbLength += (update->rowCount * tableSize);
		}

		// 로그 처리용 검색.
		InventorySelect( &update->sciLog );
		InventorySelect( &update->bciLog );
	}
	return retcode;
}

// ItemAgentSelectName Method.
SQLRETURN cSQLGameStmt::ItemAgentSelect(ITEM_AGENT_SELECT* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG,    SQL_INTEGER,  0, 0, &select->startPage,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG,    SQL_INTEGER,  0, 0, &select->endPage,             0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->sort,                0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemDefineRareLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemLimitLevelB,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemLimitLevelE,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->inventoryEnhanced,   0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long             tableSize = sizeof(select->table);
		ITEM_AGENT_RESULT* result;

		result = select->table;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
		{
			long characterNameSize = sizeof(result->characterName);

			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,          0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR, result->characterName, characterNameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru,    0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx,    0,                 &(strLenOrInd=0) );
			select->rowCount++;
		}
		SQLClose( mHstmt );

		result = select->table;
		for ( long i = 0; i < select->rowCount; i++, result++ )
		{
			InventorySelect( &result->table );
		}

		cbLength += (select->rowCount * tableSize);
	}
	return retcode;
}

// ItemAgentSelectName Method.
SQLRETURN cSQLGameStmt::ItemAgentSelectName(ITEM_AGENT_SELECT_NAME* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_NAME(?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	long nameSize = sizeof(select->itemDefineName);
	long cbName   = wcslen(select->itemDefineName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG,    SQL_INTEGER,  0,  0, &select->startPage,           0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG,    SQL_INTEGER,  0,  0, &select->endPage,             0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->sort,                0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_WCHAR,   SQL_WVARCHAR, 50, 0, select->itemDefineName,       nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemDefineRareLevel, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemLimitLevelB,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemLimitLevelE,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->inventoryEnhanced,   0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long             tableSize = sizeof(select->table);
		ITEM_AGENT_RESULT* result;

		result = select->table;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
		{
			long characterNameSize = sizeof(result->characterName);

			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,          0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR, result->characterName, characterNameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru,    0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx,    0,                 &(strLenOrInd=0) );
			select->rowCount++;
		}
		SQLClose( mHstmt );

		result = select->table;
		for ( long i = 0; i < select->rowCount; i++, result++ )
		{
			InventorySelect( &result->table );
		}

		cbLength += (select->rowCount * tableSize);
	}
	return retcode;
}

// ItemAgentSelectType Method.
SQLRETURN cSQLGameStmt::ItemAgentSelectType(ITEM_AGENT_SELECT_TYPE* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_TYPE(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG,    SQL_INTEGER,  0, 0, &select->startPage,               0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG,    SQL_INTEGER,  0, 0, &select->endPage,                 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->sort,                    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemDefineSearchType,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &select->itemDefineSearchSubType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemDefineRareLevel,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemLimitLevelB,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->itemLimitLevelE,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0, 0, &select->inventoryEnhanced,       0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long             tableSize = sizeof(select->table);
		ITEM_AGENT_RESULT* result;

		result = select->table;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
		{
			long characterNameSize = sizeof(result->characterName);

			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,          0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR, result->characterName, characterNameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru,    0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx,    0,                 &(strLenOrInd=0) );
			select->rowCount++;
		}
		SQLClose( mHstmt );

		result = select->table;
		for ( long i = 0; i < select->rowCount; i++, result++ )
		{
			InventorySelect( &result->table );
		}

		cbLength += (select->rowCount * tableSize);
	}
	return retcode;
}

// ItemAgentSelectAll Method.
SQLRETURN cSQLGameStmt::ItemAgentSelectAll(ITEM_AGENT_SELECT_ALL* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_AGENT_SELECT_BY_ALL(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	long nameSize = sizeof(select->itemDefineName);
	long cbName   = wcslen(select->itemDefineName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG,    SQL_INTEGER,  0,  0, &select->startPage,               0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_LONG,    SQL_INTEGER,  0,  0, &select->endPage,                 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->sort,                    0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_WCHAR,   SQL_WVARCHAR, 50, 0, select->itemDefineName,           nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemDefineSearchType,    0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0,  0, &select->itemDefineSearchSubType, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemDefineRareLevel,     0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemLimitLevelB,         0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->itemLimitLevelE,         0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT, SQL_TINYINT,  0,  0, &select->inventoryEnhanced,       0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long             tableSize = sizeof(select->table);
		ITEM_AGENT_RESULT* result;

		result = select->table;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, result++ )
		{
			long characterNameSize = sizeof(result->characterName);

			SQLGetData( mHstmt, param=1, SQL_C_SLONG, &result->idx,          0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR, result->characterName, characterNameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->validThru,    0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->price,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG, &result->table.idx,    0,                 &(strLenOrInd=0) );
			select->rowCount++;
		}
		SQLClose( mHstmt );

		result = select->table;
		for ( long i = 0; i < select->rowCount; i++, result++ )
		{
			InventorySelect( &result->table );
		}

		cbLength += (select->rowCount * tableSize);
	}
	return retcode;
}

// ItemObtEvent Method.
SQLRETURN cSQLGameStmt::ItemObtEvent(ITEM_OBT_EVENT* itemEvent, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_OBT_EVENT(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,		 SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemEvent->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT,  0, 0, &itemEvent->cbtUser,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT,  0, 0, &itemEvent->obtUser,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_TINYINT, SQL_TINYINT,  0, 0, &itemEvent->fourGamer,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,		 SQL_C_SLONG,   SQL_INTEGER,  0, 0, &itemEvent->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table     = itemEvent->table;
		u_long        tableSize = sizeof(itemEvent->table);

		itemEvent->rowCount = 0;
		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetBaseInv( mHstmt, table );
			itemEvent->rowCount++;
		}

		cbLength += (itemEvent->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryMove2 Method.
// GP_INVENTORY_MOVE2 참조.
SQLRETURN cSQLGameStmt::InventoryMove2(INVENTORY_MOVE2* move2)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_MOVE2(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	SQLINTEGER  inventoryIdx;
	SQLINTEGER  itemDefineIndex;
	SQLSMALLINT inventoryCount;
	SQLSMALLINT inventoryNumner;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &move2->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,         0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,       0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,      0, &(strLenOrInd=0) );

		TB_INVENTORY* table = move2->table;
		for ( long i = 0; i < move2->rowCount; i++, table++ )
		{
			inventoryIdx    = table->idx;
			itemDefineIndex = table->itemDefineIndex;
			inventoryCount  = table->count;
			inventoryNumner = table->number;
			retcode         = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				table->idx = inventoryIdx;
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryEnhanced Method.
SQLRETURN cSQLGameStmt::InventoryEnhanced(INVENTORY_ENHANCED* inventoryEnhanced)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_ENHANCED(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	TB_INVENTORY* equip     = &inventoryEnhanced->table[ 0 ];
	TB_INVENTORY* card      = &inventoryEnhanced->table[ 1 ];
	TB_INVENTORY* safeCard  = &inventoryEnhanced->table[ 2 ];

    long safeCardIdx = 0;
    if ( safeCard ) safeCardIdx = safeCard->idx;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &equip->idx,                0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &card->idx,                 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &safeCardIdx,               0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &inventoryEnhanced->result, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 강화장비
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &equip->count,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &equip->enhanced, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &equip->apply,    0, &(strLenOrInd=0) );
		}

		SQLMoreResults( mHstmt );

		// 강화카드
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &card->count,  0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &card->apply,  0, &(strLenOrInd=0) );
		}

		SQLMoreResults( mHstmt );

		// 강화카드
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &safeCard->count,  0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &safeCard->apply,  0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryDisjoint Method.
SQLRETURN cSQLGameStmt::InventoryDisjoint(INVENTORY_DISJOINT* inventoryDisjoint)
{
	SQLINTEGER  strLenOrInd = 0;
	SQLRETURN   retcode     = 0;
	SQLSMALLINT	param;

	if ( inventoryDisjoint->characterMoney != 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryDisjoint->characterIdx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryDisjoint->characterMoney, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryDisjoint->retvalue,       0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( !(SQL_SUCCEEDED( retcode )) )
			return retcode;

		SQLClose( mHstmt );
	}

	if ( inventoryDisjoint->retvalue == 0 && inventoryDisjoint->rowCount > 0 )
	{
		SQLCHAR*      statement = (SQLCHAR*)"{call GP_INVENTORY_DISJOINT(?, ?, ?, ?, ?)}";
		TB_INVENTORY* table     = inventoryDisjoint->table;

		for ( long i = 0; i < inventoryDisjoint->rowCount && inventoryDisjoint->retvalue == 0; i++, table++ )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &inventoryDisjoint->characterIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &table->idx,                      0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,   SQL_INTEGER,  0, 0, &table->itemDefineIndex,          0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &table->number,                   0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &table->count,                    0, &(strLenOrInd=0) );
			retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
				{
					SQLGetData( mHstmt, param=1, SQL_C_SHORT,   &table->count, 0, &(strLenOrInd=0) );
					SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
				}

				while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryPutCard Method.
SQLRETURN cSQLGameStmt::InventoryPutCard(INVENTORY_PUT_CARD* inventoryPutCard)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_PUT_CARD(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &inventoryPutCard->idx1,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &inventoryPutCard->cardSlot,        0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &inventoryPutCard->idx2,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &inventoryPutCard->itemDefineIndex, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &inventoryPutCard->retvalue,        0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table = inventoryPutCard->table;
		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetFullInv( mHstmt, table );
			table++;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// InventoryChange Method.
SQLRETURN cSQLGameStmt::InventoryChange(INVENTORY_CHANGE* inventoryChange)
{
	SQLINTEGER  strLenOrInd = 0;
	SQLRETURN   retcode     = 0;
	SQLSMALLINT	param;

	if ( inventoryChange->characterMoney != 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryChange->characterIdx,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryChange->characterMoney, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &inventoryChange->retvalue,       0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( !(SQL_SUCCEEDED( retcode )) )
			return retcode;

		SQLClose( mHstmt );
	}

	if ( SQL_SUCCEEDED( retcode ) && inventoryChange->retvalue == 0 && inventoryChange->rowCount > 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_CHANGE(?, ?, ?, ?, ?, ?)}";

		SQLINTEGER  inventoryIdx;
		SQLINTEGER  itemDefineIndex;
		SQLSMALLINT inventoryCount;
		SQLSMALLINT inventoryNumner;
		SQLCHAR     inventorySeal;

		retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
		if ( SQL_SUCCEEDED( retcode ) )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryChange->characterIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,                  0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,               0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,               0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,                0, &(strLenOrInd=0) );
	        retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_TINYINT,SQL_TINYINT,  0, 0, &inventorySeal,                 0, &(strLenOrInd=0) );

			TB_INVENTORY* table = inventoryChange->table;
			for ( long i = 0; i < inventoryChange->rowCount; i++, table++ )
			{
				inventoryIdx    = table->idx; 
				itemDefineIndex = table->itemDefineIndex;
				inventoryCount  = table->count;
				inventoryNumner = table->number;
                inventorySeal   = table->seal;
				retcode         = SQLExecute( mHstmt );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
					{
						SQLGetData( mHstmt, param=1, SQL_C_SSHORT,  &table->count, 0, &(strLenOrInd=0) );
						SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply, 0, &(strLenOrInd=0) );
					}
					table->idx = inventoryIdx;

					// 저장프로시저 쿼리 마무리.
					while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
				}
				else
					return retcode;
			}
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

SQLRETURN cSQLGameStmt::InventorySeal(INVENTORY_SEAL* seal)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_INVENTORY_SEAL(?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	TB_INVENTORY* inventory = &seal->inventory;
	TB_ITEM_BILL* itemBill  = &seal->itemBill;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &inventory->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &inventory->seal,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG,    SQL_INTEGER, 0, 0, &itemBill->idx,       0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &itemBill->type,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_LONG,    SQL_INTEGER, 0, 0, &itemBill->validDate, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_LONG,    SQL_INTEGER, 0, 0, &itemBill->validTime, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_LONG,    SQL_INTEGER, 0, 0, &seal->characterIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG,    SQL_INTEGER, 0, 0, &seal->retvalue,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ItemBillSelect Method.
SQLRETURN cSQLGameStmt::ItemBillSelect(ITEM_BILL_SELECT* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_BILL_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &select->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_BILL* table     = select->table;
		u_long        tableSize = sizeof(select->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetData( mHstmt, param=1, SQL_C_LONG,    &table->idx,          0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->type,         0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,    &table->validDate,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,    &table->validTime,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,    &table->inventoryIdx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->apply,        0, &(strLenOrInd=0) );
			select->rowCount++;
		}

		cbLength += (select->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemBillRemove Method.
SQLRETURN cSQLGameStmt::ItemBillRemove(ITEM_BILL_REMOVE* remove)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_BILL_REMOVE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_LONG, SQL_INTEGER, 0, 0, &remove->idx,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_LONG, SQL_INTEGER, 0, 0, &remove->inventory.idx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &remove->retvalue,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ItemBillUpdate Method.
SQLRETURN cSQLGameStmt::ItemBillUpdate(ITEM_BILL_UPDATE* update)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_BILL_UPDATE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_LONG, SQL_INTEGER, 0, 0, &update->idx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_LONG, SQL_INTEGER, 0, 0, &update->elapsed,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &update->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PostCheck Method.
SQLRETURN cSQLGameStmt::PostCheck(POST_CHECK* check)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_CHECK(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &check->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &check->rowCount,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PostSelect Method.
SQLRETURN cSQLGameStmt::PostSelect(POST_SELECT* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_SELECT(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT	param;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &select->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &select->startPage,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &select->endPage,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &select->noneRead,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		POST_RESULT* table     = select->table;
		u_long       tableSize = sizeof(select->table);

		SQLINTEGER   nameSize    = sizeof(table->fromName);
		SQLINTEGER   titleSize   = sizeof(table->contenTtitle);
		SQLINTEGER   messageSize = sizeof(table->contentMessage);

		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG,          &table->idx,                   0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->registerDate,          SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->validThru,             0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &table->type,                  0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &table->status,                0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->fromIdx,               0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          table->fromName,               nameSize,          &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          table->contenTtitle,           titleSize,         &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          table->contentMessage,         messageSize,       &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->receiptPrice,          0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->receiptSalesCharges,   0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->receiptInventory.idx,  0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SHORT,          &table->receiptInventoryCount, 0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->inventory1.idx,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->inventory2.idx,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->inventory3.idx,        0,                 &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->money,                 0,                 &(strLenOrInd=0) );

			table++;
			select->rowCount++;
		}

		cbLength += (select->rowCount * tableSize);
		SQLClose( mHstmt );

		table = select->table;
		for ( long i = 0; i < select->rowCount; i++, table++ )
		{
			if ( table->receiptInventory.idx )
				InventorySelect( &table->receiptInventory );

			if ( table->inventory1.idx )
				InventorySelect( &table->inventory1 );

			if ( table->inventory2.idx )
				InventorySelect( &table->inventory2 );

			if ( table->inventory3.idx )
				InventorySelect( &table->inventory3 );
		}
	}
	return retcode;
}

// PostInsert Method.
SQLRETURN cSQLGameStmt::PostInsert(POST_INSERT* insert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT	param;
	SQLRETURN   retcode;

	long       toSize      = sizeof(insert->toName);
	long       cbTo        = wcslen(insert->toName) ? SQL_NTS : SQL_NULL_DATA;
	long       fromSize    = sizeof(insert->fromName);
	long       cbFrom      = wcslen(insert->fromName) ? SQL_NTS : SQL_NULL_DATA;
	long       titleSize   = sizeof(insert->contentTitle);
	long       cbTitle     = wcslen(insert->contentTitle) ? SQL_NTS : SQL_NULL_DATA;
	long       messageSize = sizeof(insert->contentMessage);
	long       cbMessage   = wcslen(insert->contentMessage) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT,  SQL_C_SLONG,   SQL_INTEGER,  0,                0, &insert->toIdx,                  0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_WCHAR,   SQL_WVARCHAR, MAX_POST_NAME,    0, insert->toName,                  toSize,      &cbTo            );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_TINYINT, SQL_TINYINT,  0,                0, &insert->validThru,              0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_TINYINT, SQL_TINYINT,  0,                0, &insert->type,                   0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->fromIdx,                0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_WCHAR,   SQL_WVARCHAR, MAX_POST_NAME,    0, insert->fromName,                fromSize,    &cbFrom          );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_WCHAR,   SQL_WVARCHAR, MAX_POST_TITLE,   0, insert->contentTitle,            titleSize,   &cbTitle         );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_WCHAR,   SQL_WVARCHAR, MAX_POST_MESSAGE, 0, insert->contentMessage,          messageSize, &cbMessage       );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->receiptPrice,           0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->receiptSalesCharges,    0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->receiptInventory.idx,   0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_SHORT,   SQL_SMALLINT, 0,                0, &insert->receiptInventory.count, 0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->inventory1.idx,         0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->inventory2.idx,         0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->inventory3.idx,         0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->money,                  0,           &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,  SQL_C_LONG,    SQL_INTEGER,  0,                0, &insert->retvalue,               0,           &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PostUpdateStatus Method.
SQLRETURN cSQLGameStmt::PostUpdateStatus(POST_UPDATE_STATUS* status)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_UPDATE_STATUS(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT	param;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &status->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &status->idx,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &status->status,       0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &status->noneRead,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PostUpdate Method.
SQLRETURN cSQLGameStmt::PostUpdate(POST_UPDATE* update, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_UPDATE(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT	param;
	SQLRETURN   retcode;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->idx,              0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->characterIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SHORT, SQL_SMALLINT, 0, 0, &update->inventoryNumber1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SHORT, SQL_SMALLINT, 0, 0, &update->inventoryNumber2, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SHORT, SQL_SMALLINT, 0, 0, &update->inventoryNumber3, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->money,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,       SQL_C_SLONG, SQL_INTEGER,  0, 0, &update->retvalue,         0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long        tableSize = sizeof(update->table);
		TB_INVENTORY* table     = update->table;

		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetFullInv( mHstmt, table );

			table++;
			update->rowCount++;
		}

		cbLength += (update->rowCount * tableSize);
		SQLClose( mHstmt );
	}

	if ( update->retvalue == 0 && update->money > 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &update->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &update->money,        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &update->retvalue,     0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( !(SQL_SUCCEEDED( retcode )) )
			return retcode;

		SQLClose( mHstmt );
	}
	return retcode;
}

// PostDelete Method.
SQLRETURN cSQLGameStmt::PostDelete(POST_DELETE* del, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_DELETE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLSMALLINT	param;
	SQLRETURN   retcode;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLINTEGER idx;
		long*      ptr = del->idx;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &del->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &idx,               0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &del->retvalue,     0, &(strLenOrInd=0) );

		while ( (*ptr) != 0 )
		{
			idx     = (*ptr);
			retcode = SQLExecute( mHstmt );

			if ( !(SQL_SUCCEEDED( retcode ) && del->retvalue == 0) )
			{
				SQLClose( mHstmt );
				return retcode;
			}

			ptr++;
		}
		SQLClose( mHstmt );
	}

	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLCHAR*    statement = (SQLCHAR*)"{call GP_POST_SELECT(?, ?, ?, ?)}";
		SQLINTEGER  strLenOrInd;
		SQLSMALLINT	param;
		SQLRETURN   retcode;

		retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &del->characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &del->startPage,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &del->endPage,      0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &del->noneRead,     0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			POST_RESULT* table     = del->table;
			u_long       tableSize = sizeof(del->table);

			SQLINTEGER   nameSize    = sizeof(table->fromName);
			SQLINTEGER   titleSize   = sizeof(table->contenTtitle);
			SQLINTEGER   messageSize = sizeof(table->contentMessage);

			while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			{
				SQLGetData( mHstmt, param=1, SQL_C_SLONG,          &table->idx,                  0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TYPE_TIMESTAMP, &table->registerDate,         SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->validThru,            0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &table->type,                 0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_TINYINT,        &table->status,               0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->fromIdx,              0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          table->fromName,              nameSize,          &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          table->contenTtitle,          titleSize,         &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_WCHAR,          table->contentMessage,        messageSize,       &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->receiptPrice,         0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->receiptSalesCharges,  0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->receiptInventory.idx, 0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_SHORT,          &table->receiptInventoryCount,0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->inventory1.idx,       0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->inventory2.idx,       0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->inventory3.idx,       0,                 &(strLenOrInd=0) );
				SQLGetData( mHstmt, ++param, SQL_C_LONG,           &table->money,                0,                 &(strLenOrInd=0) );

				table++;
				del->rowCount++;
			}

			cbLength += (del->rowCount * tableSize);
			SQLClose( mHstmt );

			table = del->table;
			for ( long i = 0; i < del->rowCount; i++, table++ )
			{
				if ( table->receiptInventory.idx )
					InventorySelect( &table->receiptInventory );

				if ( table->inventory1.idx )
					InventorySelect( &table->inventory1 );

				if ( table->inventory2.idx )
					InventorySelect( &table->inventory2 );

				if ( table->inventory3.idx )
					InventorySelect( &table->inventory3 );
			}
		}
	}
	return retcode;
}


// FriendSelectr Method.
SQLRETURN cSQLGameStmt::FriendSelect(FRIEND_SELECT* friendSelect, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_FRIEND_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendSelect->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLINTEGER nameSize  = sizeof(friendSelect->table->characterName);
		u_long     tableSize = sizeof(friendSelect->table);

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &friendSelect->table[ i ].idx,          0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG,   &friendSelect->table[ i ].characterIdx, 0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_WCHAR,   friendSelect->table[ i ].characterName, nameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_TINYINT, &friendSelect->table[ i ].race,         0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_TINYINT, &friendSelect->table[ i ].gender,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 6, SQL_C_SHORT,	  &friendSelect->table[ i ].mapNum,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 7, SQL_C_TINYINT, &friendSelect->table[ i ].level,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 8, SQL_C_SLONG,	  &friendSelect->table[ i ].job,         0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 9,  SQL_C_SHORT,   &friendSelect->table[ i ].channelNum,   0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 10, SQL_C_TINYINT, &friendSelect->table[ i ].status,       0,        &(strLenOrInd=0) );
			SQLGetData( mHstmt, 11, SQL_C_TINYINT, &friendSelect->table[ i ].block,        0,        &(strLenOrInd=0) );

			friendSelect->rowCount++;
		}

		cbLength += (friendSelect->rowCount * tableSize);
		SQLClose( mHstmt );
	}
	return retcode;
}

// FriendInsert Method.
SQLRETURN cSQLGameStmt::FriendInsert(FRIEND_INSERT* friendInsert)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_FRIEND_INSERT(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       nameSize = sizeof(friendInsert->characterName);
	long       cbName   = wcslen(friendInsert->characterName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0,  0, &friendInsert->characterIdx1, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0,  0, &friendInsert->characterIdx2, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, friendInsert->characterName,  nameSize, &cbName          );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0,  0, &friendInsert->retvalue,      0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// FriendUpdate Method.
SQLRETURN cSQLGameStmt::FriendUpdate(FRIEND_UPDATE* friendUpdate)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_FRIEND_UPDATE(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendUpdate->idx,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendUpdate->characterIdx1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendUpdate->characterIdx2, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// FriendDelete Method.
SQLRETURN cSQLGameStmt::FriendDelete(FRIEND_DELETE* friendDelete)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_FRIEND_DELETE(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendDelete->idx,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendDelete->characterIdx1, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &friendDelete->characterIdx2, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// ItemSpreadValue Method
// GP_ITEM_SPREAD_VALUE 참조.
SQLRETURN cSQLGameStmt::ItemSpreadValue(ITEM_SPREAD_VALUE* itemSpreadValue)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_SPREAD_VALUE(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER array = 1;
	SQLCHAR    dir   = 0;
	SQLINTEGER value = 0;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,        SQL_C_SLONG,    SQL_INTEGER, 0, 0, &itemSpreadValue->spreadIndex, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,        SQL_C_SLONG,    SQL_INTEGER, 0, 0, &array,                        0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,        SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &dir,                          0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG,    SQL_INTEGER, 0, 0, &value,                        0, &(strLenOrInd=0) );

		for ( long i = 0; i < 10; i++, array++ )
		{
			dir     = itemSpreadValue->values[ i ].dir;
			value   = itemSpreadValue->values[ i ].tarot;
			retcode = SQLExecute( mHstmt );

			if ( retcode == SQL_SUCCESS )
			{
				itemSpreadValue->spreadValue += value;
				itemSpreadValue->values[ i ].spread = value;
			}
			else
				return retcode;
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// ItemTarotResult Method.
SQLRETURN cSQLGameStmt::ItemTarotResult(ITEM_TAROT_RESULT* itemTarotResult)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_ITEM_TAROT_RESULT(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemTarotResult->itemDefineIndex, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemTarotResult->value,           0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, &itemTarotResult->influence_idx1, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG, &itemTarotResult->influence_idx2, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_SLONG, &itemTarotResult->influence_idx3, 0, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

// SkillSelect Method.
SQLRETURN cSQLGameStmt::SkillSelect( SKILL_SELECT* skillSelect, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &skillSelect->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long     tableSize = sizeof( skillSelect->mTable );
		SQLUSMALLINT n;

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			n = 0;
			SQLGetData( mHstmt, ++n, SQL_C_ULONG, &skillSelect->mTable[ i ].mSkillIdx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_UTINYINT, &skillSelect->mTable[ i ].mSkillStep, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_LONG, &skillSelect->mTable[ i ].mEndCoolTime, 0, &(strLenOrInd=0) );

			if( skillSelect->mTable[ i ].mEndCoolTime < 0 )
				skillSelect->mTable[ i ].mEndCoolTime = 0;
			else
				skillSelect->mTable[ i ].mEndCoolTime = skillSelect->mTable[ i ].mEndCoolTime * SECOND_THOUSAND;

			skillSelect->mRowCount++;
		}
		cbLength += ( skillSelect->mRowCount * tableSize );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


// SkillInsert Method.
SQLRETURN cSQLGameStmt::SkillInsert( SKILL_INSERT* skillInsert )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_INSERT(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT n = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillInsert->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillInsert->mSkillClassIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_USHORT, SQL_SMALLINT, 0, 0, &skillInsert->mSP, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &skillInsert->skillLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillInsert->skillPoint, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &skillInsert->mRetvalue, 0, &(strLenOrInd=0) );
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

// SkillUpdate Method.
SQLRETURN cSQLGameStmt::SkillUpdate( SKILL_UPDATE* skillUpdate )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_UPDATE(?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT n = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillUpdate->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillUpdate->mSkillClassIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &skillUpdate->mSkillStep, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_USHORT, SQL_SMALLINT, 0, 0, &skillUpdate->mSP, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &skillUpdate->skillLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillUpdate->skillPoint, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &skillUpdate->mRetvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	else
		assert(NULL);

	return retcode;
}


// SkillCheatInsert Method.
SQLRETURN cSQLGameStmt::SkillCheatInsert( SKILL_CHEAT_INSERT* skillInsert )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_CHEAT_INSERT(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT n = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillInsert->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillInsert->mSkillClassIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &skillInsert->mSkillStep, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &skillInsert->mRetvalue, 0, &(strLenOrInd=0) );
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

// SkillCheatUpdate Method.
SQLRETURN cSQLGameStmt::SkillCheatUpdate( SKILL_CHEAT_UPDATE* skillUpdate )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_CHEAT_UPDATE(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLSMALLINT n = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillUpdate->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillUpdate->mSkillClassIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &skillUpdate->mSkillStep, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &skillUpdate->mRetvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	else
		assert(NULL);

	return retcode;
}

// SkillCoolTime Method.
SQLRETURN cSQLGameStmt::SkillCoolTime( SKILL_COOLTIME* skillCoolTime )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_SKILL_COOLTIME(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER characterIdx;
	SQLINTEGER skillIdx;
	SQLINTEGER endCoolTime;
	SQLINTEGER retValue;

	SQLSMALLINT n;

	skillCoolTime->mRetvalue = 0;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		n = 0;

		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &skillIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &endCoolTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &retValue, 0, &(strLenOrInd=0) );

		for ( long i = 0; i < skillCoolTime->mRowCount; ++i )
		{
			characterIdx = skillCoolTime->mCharacterIdx;
			skillIdx = skillCoolTime->mTable[i].mSkillIdx;
			endCoolTime = (unsigned long)(skillCoolTime->mTable[i].mEndCoolTime / SECOND_THOUSAND);
			
			retcode = SQLExecute( mHstmt );

			if ( (SQL_SUCCEEDED( retcode )) )
			{
				if( retValue != 0 )
					skillCoolTime->mRetvalue = retValue;
			}
			else
			{
				assert(NULL);
				return retcode;
			}
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	else
		assert(NULL);

	return retcode;
}


// SkillInfluenceSelect Method.
SQLRETURN cSQLGameStmt::SkillInfluenceSelect( SKILL_INFLUENCE_SELECT* influenceSelect, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_INFLUENCE_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &influenceSelect->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long     tableSize = sizeof( influenceSelect->mTable );
		SQLUSMALLINT n;

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			n = 0; // 컬럼 번호
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mDbIdx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mInfluenceClassIdx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mAttacker.index, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_STINYINT, &influenceSelect->mTable[i].mAttacker.type, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_BIT, &influenceSelect->mTable[i].mIsRealTime, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mRestTime, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_TYPE_TIMESTAMP, &influenceSelect->mTable[ i ].mEndDate, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mTime1, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mTime2, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mInfluenceValue1, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mInfluenceValue2, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mAttriButeType, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mAttackerLv, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mAttackerDamageMin, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mAttackerDamageMax, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mHeal, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mLeftGuardCnt, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_SLONG, &influenceSelect->mTable[i].mLeftGuardValue, 0, &(strLenOrInd=0) );
			influenceSelect->mRowCount++;
		}
		cbLength += ( influenceSelect->mRowCount * tableSize );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


// SkillInfluenceUpdate Method.
SQLRETURN cSQLGameStmt::SkillInfluenceUpdate( SKILL_INFLUENCE_UPDATE* influenceUpdate )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_INFLUENCE_UPDATE(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER influenceIdx;
	SQLINTEGER dbIdx = 0;
	SQLINTEGER attackerIdx;
	SQLINTEGER attackerType;
	SQLCHAR isRealTime;
	SQLINTEGER restTime;
	SQL_TIMESTAMP_STRUCT endDate;

	SQLINTEGER time1;
	SQLINTEGER time2;

	SQLINTEGER influenceValue1;
	SQLINTEGER influenceValue2;

	SQLINTEGER attriButeType;

	SQLCHAR attackerLv;
	SQLINTEGER attackerDamageMin;
	SQLINTEGER attackerDamageMax;
	SQLINTEGER heal;

	SQLCHAR leftGuardCnt;
	SQLSMALLINT leftGuardValue;
	SQLCHAR delIdx;

	SQLINTEGER retValue = 0;
	SQLSMALLINT n;

	influenceUpdate->mRetvalue = 0;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		n = 0;
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &influenceUpdate->mCharacterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &influenceIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &attackerIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &attackerType, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT, SQL_TINYINT, 0, 0, &isRealTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &restTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &endDate, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &time1, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &time2, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &influenceValue1, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &influenceValue2, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &attriButeType, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT, SQL_TINYINT, 0, 0, &attackerLv, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &attackerDamageMin, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &attackerDamageMax, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &heal, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT, SQL_TINYINT, 0, 0, &leftGuardCnt, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &leftGuardValue, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &retValue, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &dbIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_STINYINT, SQL_TINYINT, 0, 0, &delIdx, 0, &(strLenOrInd=0) );

		for ( long i = 0; i < influenceUpdate->mRowCount; ++i )
		{
			influenceIdx = influenceUpdate->mTable[i].mInfluenceClassIdx;
			attackerIdx = influenceUpdate->mTable[i].mAttacker.index;
			attackerType = influenceUpdate->mTable[i].mAttacker.type;
			isRealTime = influenceUpdate->mTable[i].mIsRealTime;
			restTime = influenceUpdate->mTable[i].mRestTime;
			endDate = influenceUpdate->mTable[i].mEndDate;
			time1 = influenceUpdate->mTable[i].mTime1;
			time2 = influenceUpdate->mTable[i].mTime2;
			influenceValue1 = influenceUpdate->mTable[i].mInfluenceValue1;
			influenceValue2 = influenceUpdate->mTable[i].mInfluenceValue2;
			attriButeType = influenceUpdate->mTable[i].mAttriButeType;
			attackerLv = influenceUpdate->mTable[i].mAttackerLv;
			attackerDamageMin = influenceUpdate->mTable[i].mAttackerDamageMin;
			attackerDamageMax = influenceUpdate->mTable[i].mAttackerDamageMax;
			heal = influenceUpdate->mTable[i].mHeal;
			leftGuardCnt = influenceUpdate->mTable[i].mLeftGuardCnt;
			leftGuardValue = influenceUpdate->mTable[i].mLeftGuardValue;
			dbIdx = influenceUpdate->mTable[i].mDbIdx;
			delIdx = influenceUpdate->mTable[i].mDel;

			retcode = SQLExecute( mHstmt );

			if ( SQL_SUCCEEDED( retcode ) )
			{
				influenceUpdate->mRetvalue = retValue;
				influenceUpdate->mTable[i].mDbIdx = dbIdx;
				if( retValue != 0 )
					influenceUpdate->mRetvalue = retValue;
			}
			else
			{
		//		assert(NULL);
				return retcode;
			}
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	else
		assert(NULL);

	return retcode;
}


// TruncateItemDefine Method
SQLRETURN cSQLGameStmt::TruncateSkillListPc(void)
{
	SQLCHAR*  statement = (SQLCHAR*)"TRUNCATE TABLE TB_SKILLLIST_PC";
	SQLRETURN retcode   = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

// SkillListPcInsert Method.
SQLRETURN cSQLGameStmt::SkillListPcInsert( void* pPlayerSkillBaseInfo )
{
	sPlayerSkillBaseInfo* skillInsert = (sPlayerSkillBaseInfo*)pPlayerSkillBaseInfo;

	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SKILLLIST_PC_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLINTEGER output = 0;
	SQLSMALLINT n = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mSkillIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mNameIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mIconIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &skillInsert->mRace, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &skillInsert->mUseEquipment, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &skillInsert->mUseState, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mUseItem, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mAttributeType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mShotType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mBoundType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mApplyType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mRangeType, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &skillInsert->mIsGlobalCoolTime, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &skillInsert->mGroupCoolTimeIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &skillInsert->mStepCount, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &output, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( output != 0 )
	{
		assert(NULL);
		return (SQLRETURN)output;
	}

	return retcode;
}

// TruncateItemDefine Method
SQLRETURN cSQLGameStmt::TruncateDefalultSkill(void)
{
	SQLCHAR*  statement = (SQLCHAR*)"TRUNCATE TABLE TB_DEFAULT_SKILL";
	SQLRETURN retcode   = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


// TruncateItemDefine Method
SQLRETURN cSQLGameStmt::TruncateJobTree(void)
{
	SQLCHAR*  statement = (SQLCHAR*)"TRUNCATE TABLE TB_JOBTREE";
	SQLRETURN retcode   = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


// SkillListPcInsert Method.
SQLRETURN cSQLGameStmt::DefalultSkillInsert( unsigned long jobStep, unsigned long skillIdx, unsigned char race, unsigned long jobIdx )
{

	SQLCHAR*   statement = (SQLCHAR*)"{call GP_DEFAULT_SKILL_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLINTEGER output = 0;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &jobStep, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &skillIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &race, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &jobIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &output, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( output != 0 )
	{
		assert(NULL);
		return (SQLRETURN)output;
	}

	return retcode;
}


SQLRETURN cSQLGameStmt::JobTreeInsert( unsigned long jobIdx, unsigned long parentJobIdx )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_JOBTREE_INSERT(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;
	SQLINTEGER output = 0;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &jobIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &parentJobIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &output, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( output != 0 )
	{
		assert(NULL);
		return (SQLRETURN)output;
	}

	return retcode;
}

// TruncateQuest Method
SQLRETURN cSQLGameStmt::TruncateQuestDefine( void )
{
	SQLCHAR*  statement = (SQLCHAR*)"TRUNCATE TABLE TB_QUEST_DEFINE";
	SQLRETURN retcode   = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::TruncateQuestProgress( void )
{
	SQLCHAR*  statement = (SQLCHAR*)"TRUNCATE TABLE TB_QUEST_PROGRESS";
	SQLRETURN retcode   = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::InsertQuestDefine( void* questDefine )
{
	cQuestDefine* define = (cQuestDefine*)questDefine;

	SQLCHAR*	statement = (SQLCHAR*)"{call GP_QUEST_DEFINE_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	unsigned char timeEvent = ( define->mEventType == eQUEST_TIMEEVENT ) ? 1 : 0;

	retcode = SQLBindParameter( mHstmt, 1,   SQL_PARAM_INPUT, SQL_C_SLONG,	SQL_INTEGER,	0, 0, &define->mIndex,			0,	&(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2,   SQL_PARAM_INPUT, SQL_C_SLONG,	SQL_INTEGER,	0, 0, &define->mTitleIndex,		0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3,   SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,	0, 0, &define->mEventType,		0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4,   SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT,	0, 0, &define->mGroup,			0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5,   SQL_PARAM_INPUT, SQL_C_SLONG,	SQL_INTEGER,	0, 0, &define->mGiveNpcIndex,	0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6,   SQL_PARAM_INPUT, SQL_C_SLONG,	SQL_INTEGER,	0, 0, &define->mTakeNpcIndex,	0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 7,   SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,	0, 0, &define->mRepeatType,		0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 8,   SQL_PARAM_INPUT, SQL_C_SSHORT,	SQL_SMALLINT,	0, 0, &define->mSelectCount,	0,  &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 9,   SQL_PARAM_INPUT, SQL_C_SBIGINT,SQL_BIGINT,		0, 0, &define->mComplete,		0,  &(strLenOrInd=0) );

	//retcode = SQLBindParameter( mHstmt, 10,  SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,	0, 0, &timeEvent,				0,  &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		/// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		/// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	
	return retcode;
}

SQLRETURN cSQLGameStmt::QuestInsert(QUEST_INSERT* questInsert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_QUEST_INSERT(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";

	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->questIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SBIGINT, SQL_BIGINT,  0, 0, &questInsert->check,        0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &questInsert->timeType,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->restTime,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->takemoney,    0, &(strLenOrInd=0) );

	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->firepoint,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->waterpoint,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->windpoint,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->earthpoint,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->tutorialindex, 0, &(strLenOrInd=0) );

	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questInsert->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			TB_QUEST_PROGRESS* table = &questInsert->quest;
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &table->idx,		0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG,   &table->questIdx,	0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_SBIGINT, &table->check,	0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_SLONG,	  &table->restTime,	0, &(strLenOrInd=0) );
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( SQL_SUCCEEDED( retcode ) && questInsert->retvalue == 0 && questInsert->rowCount > 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_UPDATE(?, ?, ?, ?, ?, ?)}";

		SQLINTEGER  inventoryIdx;
		SQLINTEGER  itemDefineIndex;
		SQLSMALLINT inventoryCount;
		SQLSMALLINT inventoryNumner;
		SQLCHAR     inventorySeal; 

		retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
		if ( SQL_SUCCEEDED( retcode ) )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &questInsert->characterIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,              0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,           0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,            0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,           0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,  0, 0, &inventorySeal,             0, &(strLenOrInd=0) );

			TB_INVENTORY* table = questInsert->inventory;
			for ( long i = 0; i < questInsert->rowCount; i++, table++ )
			{
				inventoryIdx    = table->idx; 
				itemDefineIndex = table->itemDefineIndex;
				inventoryCount  = table->count;
				inventoryNumner = table->number;
				inventorySeal	= table->seal;
				retcode         = SQLExecute( mHstmt );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
						SQLGetBaseInv( mHstmt, table );

					// 저장프로시저 쿼리 마무리.
					while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
				}
				else
					return retcode;
			}

			SQLClose( mHstmt );
		}
	}
	return retcode;
}

SQLRETURN cSQLGameStmt::QuestSelect( QUEST_SELECT* questSelect, u_long& cbLength)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_QUEST_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,   0, &questSelect->characterIdx,  0,    &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		u_long tableSize = sizeof(questSelect->table);
		
		for( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &questSelect->table[i].idx,			0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG,   &questSelect->table[i].questIdx,		0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_SBIGINT, &questSelect->table[i].check,			0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_SLONG,	  &questSelect->table[i].restTime,		0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_TINYINT, &questSelect->table[i].status,		0, &(strLenOrInd=0) );

			questSelect->rowCount++;
		}
		cbLength += (questSelect->rowCount * tableSize);

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::QuestRemove( QUEST_REMOVE* questRemove )
{
	SQLINTEGER strLenOrInd;
	SQLRETURN retcode;

	/// 퀘스트 강제삭제
	SQLCHAR* statement = (SQLCHAR*)"{call GP_QUEST_REMOVE(?)}";

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &questRemove->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
		SQLCloseCursor( mHstmt );
	}
	return retcode;
}

SQLRETURN cSQLGameStmt::QuestDelete( QUEST_DELETE* questDelete)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_QUEST_DELETE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &questDelete->idx,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &questDelete->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &questDelete->retvalue,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( SQL_SUCCEEDED( retcode ) && questDelete->retvalue == 0 && questDelete->rowCount > 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_UPDATE(?, ?, ?, ?, ?, ?)}";

		SQLINTEGER  inventoryIdx;
		SQLINTEGER  itemDefineIndex;
		SQLSMALLINT inventoryCount;
		SQLSMALLINT inventoryNumner;
		SQLCHAR     inventorySeal; 

		retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
		if ( SQL_SUCCEEDED( retcode ) )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &questDelete->characterIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,              0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,           0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,            0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,           0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,  0, 0, &inventorySeal,             0, &(strLenOrInd=0) );

			TB_INVENTORY* table = questDelete->table;
			for ( long i = 0; i < questDelete->rowCount; i++, table++ )
			{
				inventoryIdx    = table->idx; 
				itemDefineIndex = table->itemDefineIndex;
				inventoryCount  = table->count;
				inventoryNumner = table->number;
				inventorySeal	= table->seal;
				retcode         = SQLExecute( mHstmt );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
						SQLGetBaseInv( mHstmt, table );

					// 저장프로시저 쿼리 마무리.
					while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
				}
				else
					return retcode;
			}

			SQLClose( mHstmt );
		}
	}

	return retcode;
}

// QuestComplete Method
SQLRETURN cSQLGameStmt::QuestComplete(QUEST_COMPLETE* questComplete)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_QUEST_COMPLETE(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0,   0, &questComplete->idx,          0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0,   0, &questComplete->characterIdx, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0,   0, &questComplete->repeatType,   0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0,   0, &questComplete->tutorialIndex,0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER, 0,   0, &questComplete->retvalue,     0,        &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SLONG,   &questComplete->idx,    0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &questComplete->status, 0, &(strLenOrInd=0) );
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( SQL_SUCCEEDED( retcode ) && questComplete->retvalue == 0 && questComplete->rowCount > 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_UPDATE(?, ?, ?, ?, ?, ?)}";

		SQLINTEGER  inventoryIdx;
		SQLINTEGER  itemDefineIndex;
		SQLSMALLINT inventoryCount;
		SQLSMALLINT inventoryNumner;
		SQLCHAR     inventorySeal; 

		retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
		if ( SQL_SUCCEEDED( retcode ) )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &questComplete->characterIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,                0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,             0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,              0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,             0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,  0, 0, &inventorySeal,             0, &(strLenOrInd=0) );

			TB_INVENTORY* table = questComplete->table;
			for ( long i = 0; i < questComplete->rowCount; i++, table++ )
			{
				inventoryIdx    = table->idx; 
				itemDefineIndex = table->itemDefineIndex;
				inventoryCount  = table->count;
				inventoryNumner = table->number;
				inventorySeal	= table->seal;
				retcode         = SQLExecute( mHstmt );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
						SQLGetBaseInv( mHstmt, table );

					// 저장프로시저 쿼리 마무리.
					while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
				}
				else
					return retcode;
			}

			SQLClose( mHstmt );
		}
	}

	return retcode;
}

// QuestReward Method
SQLRETURN cSQLGameStmt::QuestReward(QUEST_REWARD* questReward, u_long& cbLength)
{
	SQLCHAR* statement = (SQLCHAR*)"{call GP_QUEST_REWARD(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)}";

	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questReward->idx,            0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questReward->characterIdx,   0, &(strLenOrInd=0) );

	if( questReward->thru.year == 0 )
	{
		/// 프로시져 상에서, null 인경우는 GP_QUESTVALID_INSERT 를 호출하지 않는다
		SQLINTEGER	data = SQL_NULL_DATA;
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 1, 0, 0, 0, &data );
	}
	else
	{
		retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &questReward->thru, 0, &(strLenOrInd=0) );
	}

	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questReward->characterMoney, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questReward->jobIdx,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questReward->jobStep,        0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &questReward->race,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &questReward->forceType,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &questReward->titleIdx,       0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_ULONG,   SQL_INTEGER, 0, 0, &questReward->skillPoint,     0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	/// select  결과물이 2개
	if ( SQL_SUCCEEDED( retcode ) )
	{
		/// 습득 스킬 목록은 가변길이
		unsigned long* table = questReward->skillClassIdx;
		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, table, 0, &(strLenOrInd=0) );

			++questReward->skillRowCount;
			++table;
		}

		cbLength += (questReward->skillRowCount * sizeof(questReward->skillClassIdx));

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( SQL_SUCCEEDED( retcode ) && questReward->itemRowCount > 0 )
	{
		SQLCHAR* statement = (SQLCHAR*)"{call GP_INVENTORY_UPDATE(?, ?, ?, ?, ?, ?)}";

		SQLINTEGER  inventoryIdx;
		SQLINTEGER  itemDefineIndex;
		SQLSMALLINT inventoryCount;
		SQLSMALLINT inventoryNumner;
		SQLCHAR     inventorySeal; 

		retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
		if ( SQL_SUCCEEDED( retcode ) )
		{
			retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &questReward->characterIdx, 0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &inventoryIdx,              0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER,  0, 0, &itemDefineIndex,           0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryCount,            0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SSHORT, SQL_SMALLINT, 0, 0, &inventoryNumner,           0, &(strLenOrInd=0) );
			retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT,SQL_TINYINT,  0, 0, &inventorySeal,             0, &(strLenOrInd=0) );

			TB_INVENTORY* table = questReward->table;
			for ( long i = 0; i < questReward->itemRowCount; i++, table++ )
			{
				inventoryIdx    = table->idx; 
				itemDefineIndex = table->itemDefineIndex;
				inventoryCount  = table->count;
				inventoryNumner = table->number;
				inventorySeal	= table->seal;
				retcode         = SQLExecute( mHstmt );

				if ( SQL_SUCCEEDED( retcode ) )
				{
					if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
						SQLGetBaseInv( mHstmt, table );

					// 저장프로시저 쿼리 마무리.
					while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
				}
				else
					return retcode;
			}

			SQLClose( mHstmt );
		}
	}

	return retcode;
}


SQLRETURN cSQLGameStmt::QuestUpdate( QUEST_UPDATE* questUpdate )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_QUEST_UPDATE(?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER idx;
	SQLBIGINT check;
	SQLINTEGER resttime;
	SQLINTEGER  retvalue;
	
	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,   0, &idx,			0,    &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SBIGINT,SQL_BIGINT,	 0,   0, &check,		0,    &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,	 0,   0, &resttime,		0,    &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_OUTPUT, SQL_C_SLONG,  SQL_INTEGER,  0,   0, &retvalue,		0,    &(strLenOrInd=0) );

		for( long i = 0; i < questUpdate->rowCount; ++i )
		{
			idx			= questUpdate->table[i].idx;
			check		= questUpdate->table[i].check;
			resttime	= questUpdate->table[i].restTime;
			retvalue	= questUpdate->table[i].retvalue;
			retcode     = SQLExecute( mHstmt );

			if( SQL_SUCCEEDED( retcode ) )
			{
				questUpdate->table[i].retvalue = retvalue;

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				return retcode;
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::QuestEndListSelect( QUEST_ENDLIST* questEndList, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_QUEST_ENDLISTSELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,   0, &questEndList->characterIdx,  0,    &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		u_long tableSize = sizeof(questEndList->questIdx);

		for( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &questEndList->questIdx[i],			0, &(strLenOrInd=0) );
		
			questEndList->rowCount++;
		}
		cbLength += (questEndList->rowCount * tableSize);

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::QuestValidListSelect( QUEST_VALIDLIST* questValidList, u_long& cbLength )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_QUESTVALID_LISTSELECT(?)}";
	SQLINTEGER	strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,  SQL_INTEGER,  0,   0, &questValidList->characterIdx,  0,    &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		u_long tableSize = sizeof(questValidList->table);

		for( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,				&questValidList->table[i].questIdx,	0,					&(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_TYPE_TIMESTAMP,	&questValidList->table[i].valid,	SQL_TIMESTAMP_LEN,	&(strLenOrInd=0) );

			questValidList->rowCount++;
		}
		cbLength += (questValidList->rowCount * tableSize);

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

// StallSellGet Method
SQLRETURN cSQLGameStmt::StallSellGet(STALL_SELL_GET* stallSellGet)
{
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	// 아이템 판매 금액 입금
	{
		SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";

		retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &stallSellGet->fObject.index,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &stallSellGet->fCharacterMoney, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &stallSellGet->retvalue,        0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( !(SQL_SUCCEEDED( retcode )) )
			return retcode;

		SQLClose( mHstmt );
	}
	// 아이템 구입 금액 출금
	{
		SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_MONEY(?, ?, ?)}";

		retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &stallSellGet->tObject.index,   0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER, 0, 0, &stallSellGet->tCharacterMoney, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &stallSellGet->retvalue,        0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( !(SQL_SUCCEEDED( retcode )) )
			return retcode;

		SQLClose( mHstmt );
	}

	// 노점 판매&구입.
	{
		SQLCHAR*   statement = (SQLCHAR*)"{call GP_STALL_SELL_GET(?, ?, ?, ?, ?, ?)}";
		SQLINTEGER strLenOrInd;
		SQLRETURN  retcode;

		retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &stallSellGet->fInventory.idx,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER,  0, 0, &stallSellGet->tObject.index,     0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &stallSellGet->tInventory.idx,    0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &stallSellGet->tInventory.number, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT,  SQL_C_SSHORT,  SQL_SMALLINT, 0, 0, &stallSellGet->tInventory.count,  0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_OUTPUT, SQL_C_SLONG,   SQL_INTEGER,  0, 0, &stallSellGet->retvalue,          0, &(strLenOrInd=0) );
		retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

		if ( SQL_SUCCEEDED( retcode ) )
		{
			if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			{
				SQLGetData( mHstmt, 1, SQL_C_SSHORT,  &stallSellGet->fInventory.count, 0, &(strLenOrInd=0) );
				SQLGetData( mHstmt, 2, SQL_C_TINYINT, &stallSellGet->fInventory.apply, 0, &(strLenOrInd=0) );
			}

			SQLClose( mHstmt );
		}
	}
	return retcode;
}

// ShortcutSelect Method
SQLRETURN cSQLGameStmt::ShortcutSelect(SHORTCUT_SELECT* shortcutSelect)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SHORTCUT_SELECT(?,?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       dataLen   = sizeof(shortcutSelect->table.binary);

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &shortcutSelect->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &shortcutSelect->rowCount,    0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_BINARY, shortcutSelect->table.binary, dataLen, &(strLenOrInd=0) );
		}

		SQLClose( mHstmt );
	}

	return retcode;
}

// ShortcutUpdate Method
SQLRETURN cSQLGameStmt::ShortcutUpdate(SHORTCUT_UPDATE* shortcutUpdate)
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_SHORTCUT_UPDATE(?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	long       dataLen   = sizeof(shortcutUpdate->table.binary);
	long       cbData    = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch   = sizeof(shortcutUpdate->table.binary);

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG,  SQL_INTEGER, 0,       0, &shortcutUpdate->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY,  dataLen, 0, (VOID*)1,                      0, &cbData          );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, shortcutUpdate->table.binary, cbBatch );
				dataLen -= cbBatch;
			} while ( dataLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, shortcutUpdate->table.binary, dataLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}

		SQLClose( mHstmt );
	}
	return retcode;
}

SQLRETURN cSQLGameStmt::TitleInsert(TITLE_INSERT* titleInsert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_TITLE_INSERT(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &titleInsert->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_SLONG,   SQL_INTEGER, 0, 0, &titleInsert->titleIdx,	    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &titleInsert->retvalue,	    0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::TitleSelect( TITLE_SELECT* titleSelect, u_long& cbLength )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_TITLE_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1,   SQL_PARAM_INPUT, SQL_C_SLONG,	SQL_INTEGER,	0, 0, &titleSelect->characterIdx,0,	&(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		u_long tableSize = sizeof(titleSelect->table);

		for( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,   &titleSelect->table[i].idx,			0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_SLONG,   &titleSelect->table[i].titleIdx,		0, &(strLenOrInd=0) );

			titleSelect->rowCount++;
		}
		cbLength += (titleSelect->rowCount * tableSize);

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::VerifyGuild( VERIFY_GUILD* verifyGuild )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_VERIFY_GUILD(?, ?, ?)}";
	SQLRETURN	retcode;
	SQLINTEGER	strLenOrInd;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyGuild->count, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyGuild->total, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &verifyGuild->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::GuildSelect( GUILD_SELECT* guildSelect )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILD_SELECT(?)}";
	SQLRETURN	retcode;
	SQLINTEGER	strLenOrInd;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &guildSelect->idx,		0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		TB_GUILD* guild = guildSelect->table;
		SQLINTEGER nameSize = sizeof(guild->name);
		SQLINTEGER masterSize = sizeof(guild->masterName);
		SQLINTEGER noticeSize = sizeof(guild->notice);
		long       dataLen   = sizeof(guild->mark);

		if( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,  &guild->idx,			0,			&(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_WCHAR,  &guild->name,			nameSize,	&(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_SLONG,  &guild->masterIndex,	0,			&(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_WCHAR,  &guild->masterName,	masterSize,	&(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_WCHAR,  &guild->notice,		noticeSize,	&(strLenOrInd=0) );
			SQLGetData( mHstmt, 6, SQL_C_TYPE_TIMESTAMP,  &guild->markDate,		SQL_TIMESTAMP_LEN,	&(strLenOrInd=0) );
			SQLGetData( mHstmt, 7, SQL_C_BINARY, &guild->mark,			dataLen,	&(strLenOrInd=0) );
			SQLGetData( mHstmt, 8, SQL_C_SSHORT, &guild->apply,			0,			&(strLenOrInd=0) );
		}

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}
	return retcode;
}

SQLRETURN cSQLGameStmt::GuildUserSelect( GUILDUSER_SELECT* guildUserSelect, u_long& cbLength )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILDUSER_SELECT(?)}";
	SQLRETURN	retcode;
	SQLINTEGER	strLenOrInd;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &guildUserSelect->idx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if( SQL_SUCCEEDED( retcode ) )
	{
		u_long tableSize = sizeof(guildUserSelect->table);
		SQLINTEGER nameSize = sizeof(guildUserSelect->table->name);

		for( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG,		&guildUserSelect->table[i].characterIdx,	0,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_WCHAR,		&guildUserSelect->table[i].name,			nameSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_TINYINT,	&guildUserSelect->table[i].level,			0,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_SLONG,		&guildUserSelect->table[i].job,				0,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_SSHORT,	&guildUserSelect->table[i].mapnum,			0,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 6, SQL_C_TINYINT,	&guildUserSelect->table[i].position,		0,		  &(strLenOrInd=0) );

			guildUserSelect->rowCount++;
		}
		cbLength += (guildUserSelect->rowCount * tableSize);

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::GuildCreate( GUILD_CREATE* guildCreate )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILD_CREATE(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	long       nameSize = sizeof(guildCreate->name);
	long       masterSize = sizeof(guildCreate->masterName);
	long       cbName		= wcslen(guildCreate->name) ? SQL_NTS : SQL_NULL_DATA;
	long       cbMasterName = wcslen(guildCreate->masterName) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildCreate->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR,	50, 0, &guildCreate->name,			nameSize, &cbName );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildCreate->characterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR,	50, 0, &guildCreate->masterName,		masterSize, &cbMasterName );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildCreate->money,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildCreate->retvalue,		0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::GuildUpdate( GUILD_UPDATE* guildUpdate )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILD_UPDATE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	long		noticeSize = sizeof(guildUpdate->notice);
	long		cbNotice   = wcslen(guildUpdate->notice) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUpdate->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR,	40, 0, &guildUpdate->notice,		noticeSize, &cbNotice );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUpdate->retvalue,		0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::GuildDelete( GUILD_DELETE* guildDelete )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILD_DELETE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildDelete->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildDelete->charaterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildDelete->retvalue,		0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::GuildUserAdd( GUILDUSER_ADD* guildUserAdd )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILDUSER_ADD(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUserAdd->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUserAdd->charaterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUserAdd->retvalue,		0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::GuildUserOut( GUILDUSER_OUT* guildUserOut )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILDUSER_OUT(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUserOut->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUserOut->charaterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &guildUserOut->retvalue,		0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::GuildUserGivePosition( GUILDUSER_GIVEPOSITION* guildUserGivePosition )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILDUSER_GIVEPOSITION(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,	SQL_INTEGER,	0, 0, &guildUserGivePosition->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_SLONG,	SQL_INTEGER,	0, 0, &guildUserGivePosition->characterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,  SQL_C_TINYINT,	SQL_TINYINT,	0, 0, &guildUserGivePosition->position,		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_OUTPUT, SQL_C_SLONG,	SQL_INTEGER,	0, 0, &guildUserGivePosition->retvalue,		0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::GuildMarkUpdate( GUILDMARK_UPDATE* guildMarkUpdate )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_GUILDMARK_UPDATE(?, ?, ?, ?)}";
	SQLINTEGER	strLenOrInd;
	SQLRETURN	retcode;

	long       dataLen   = sizeof(guildMarkUpdate->mark.binary);
	long       cbData    = SQL_DATA_AT_EXEC;

	void*      parmID;
	long       cbBatch   = sizeof(guildMarkUpdate->mark.binary);

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,		  SQL_INTEGER,			0,				   0, &guildMarkUpdate->idx,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP,SQL_TYPE_TIMESTAMP,	SQL_TIMESTAMP_LEN, 0, &guildMarkUpdate->markRegistDate,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,  SQL_C_BINARY,		  SQL_BINARY,			dataLen,		   0, (VOID*)1,							0, &cbData );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_OUTPUT, SQL_C_SLONG,		  SQL_INTEGER,			0,				   0, &guildMarkUpdate->retvalue,		0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );
	if ( retcode == SQL_NEED_DATA )
	{
		retcode = SQLParamData( mHstmt, &parmID );
		while ( retcode == SQL_NEED_DATA )
		{
			do {
				SQLPutData( mHstmt, guildMarkUpdate->mark.binary, cbBatch );
				dataLen -= cbBatch;
			} while ( dataLen > cbBatch );

			// Put final batch.
			SQLPutData( mHstmt, guildMarkUpdate->mark.binary, dataLen );

			// Make final SQLParamData call.
			retcode = SQLParamData( mHstmt, &parmID );
		}

		SQLClose( mHstmt );
	}
	return SQLClose( retcode, mHstmt );
}


SQLRETURN cSQLGameStmt::MakeSkillInsert( MAKESKILL_INSERT* pInsert )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_CHARACTER_MAKESKILL_INSERT(?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,		SQL_INTEGER, 0, 0, &pInsert->mCharacterIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_UTINYINT,	SQL_TINYINT, 0, 0, &pInsert->mIsFirstSkill,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_UTINYINT,	SQL_TINYINT, 0, 0, &pInsert->mMakeSkillIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_UTINYINT,	SQL_TINYINT, 0, 0, &pInsert->mMakeSkillStep, 0, &(strLenOrInd=0) );
    retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,		SQL_INTEGER, 0, 0, &pInsert->mInventoryIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &pInsert->mRetvalue,      0, &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table = &pInsert->mInventory;

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
			SQLGetFullInv( mHstmt, table );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


/// RecipeCoolTime Method.
SQLRETURN cSQLGameStmt::RecipeCoolTime( RECIPE_COOLTIME* recipeCoolTime )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_RECIPE_COOLTIME(?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER characterIdx;
	SQLINTEGER makeSkill;
	SQLINTEGER recipeIdx;
	SQLINTEGER coolTime;
	SQLINTEGER retValue;

	recipeCoolTime->mRetvalue = 0;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLSMALLINT n = 0;

		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &characterIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_UTINYINT, SQL_TINYINT, 0, 0, &makeSkill, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &recipeIdx, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &coolTime, 0, &(strLenOrInd=0) );
		retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &retValue, 0, &(strLenOrInd=0) );

		for ( long i = 0; i < recipeCoolTime->mRowCount; ++i )
		{
			characterIdx = recipeCoolTime->mCharacterIdx;
			makeSkill = recipeCoolTime->mTable[i].mMakeSkill;
			recipeIdx = recipeCoolTime->mTable[i].mRecipeIdx;
			coolTime = (unsigned long)(recipeCoolTime->mTable[i].mLeftCoolTime / SECOND_THOUSAND);

			retcode = SQLExecute( mHstmt );

			if ( (SQL_SUCCEEDED( retcode )) )
			{
				if( retValue != 0 )
					recipeCoolTime->mRetvalue = retValue;
			}
			else
			{
				assert(NULL);
				return retcode;
			}
		}

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}
	else
		assert(NULL);

	return retcode;
}

// MakeSkillSelect Method.
SQLRETURN cSQLGameStmt::MakeSkillSelect( MAKESKILL_SELECT* pSelect, u_long& cbLength )
{
	SQLCHAR*   statement = (SQLCHAR*)"{call GP_CHARACTER_MAKESKILL_SELECT(?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 0, 0, &pSelect->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		u_long tableSize = sizeof( pSelect->mTable );
		SQLUSMALLINT n;

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++ )
		{
			n = 0;
			SQLGetData( mHstmt, ++n, SQL_C_UTINYINT, &pSelect->mTable[ i ].mMakeSkill, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_ULONG, &pSelect->mTable[ i ].mRecipeIdx, 0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++n, SQL_C_ULONG, &pSelect->mTable[ i ].mCoolTime, 0, &(strLenOrInd=0) );

			pSelect->mRowCount++;
		}
		cbLength += ( pSelect->mRowCount * tableSize );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


SQLRETURN cSQLGameStmt::MakeSkillDelete( MAKESKILL_DELETE* pDelete )
{
	SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_MAKESKILL_DELETE(?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN retcode;
	SQLUSMALLINT n = 0;	

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,     SQL_INTEGER, 0, 0, &pDelete->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_UTINYINT,  SQL_TINYINT, 0, 0, &pDelete->mMakeSkillIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,     SQL_INTEGER, 0, 0, &pDelete->mRetvalue,     0, &(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::RecipeInsert( RECIPE_INSERT* pInsert )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_CHARACTER_RECIPE_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,     SQL_INTEGER, 0, 0, &pInsert->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_UTINYINT,  SQL_TINYINT, 0, 0, &pInsert->mMakeSkillIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,     SQL_INTEGER, 0, 0, &pInsert->mRecipeIdx,    0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,     SQL_INTEGER, 0, 0, &pInsert->mInventoryIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,     SQL_INTEGER, 0, 0, &pInsert->retvalue,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_INVENTORY* table = &pInsert->mInventory;

		if( SQLFetch( mHstmt ) == SQL_SUCCESS )
			SQLGetFullInv( mHstmt, table );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return retcode;
}


/// RecipeCoolTime Method.
SQLRETURN cSQLGameStmt::RecipeGroupInsert( RECIPE_GROUP_INSERT* pRecipeGroup )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_CHARACTER_RECIPE_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	SQLINTEGER characterIdx;
	SQLINTEGER makeSkill;
	SQLINTEGER recipeIdx;
	SQLINTEGER inventoryIdx;
	SQLINTEGER retValue = 0;

	retcode = SQLPrepare( mHstmt, statement, SQL_NTS );
	if ( !SQL_SUCCEEDED( retcode ) )
		return retcode;

	SQLSMALLINT n = 0;

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,     SQL_INTEGER, 0, 0, &characterIdx,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_UTINYINT,  SQL_TINYINT, 0, 0, &makeSkill,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG,     SQL_INTEGER, 0, 0, &recipeIdx,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,     SQL_INTEGER, 0, 0, &inventoryIdx,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,     SQL_INTEGER, 0, 0, &retValue,       0, &(strLenOrInd=0) );

	pRecipeGroup->mRetvalue = 0;
	for ( unsigned long i = 0; i < pRecipeGroup->mRowCnt; ++i )
	{
		characterIdx    = pRecipeGroup->mCharacterIdx;
		makeSkill       = pRecipeGroup->mRecipeGroup[i].mMakeSkill;
		recipeIdx       = pRecipeGroup->mRecipeGroup[i].mRecipeIdx;

		if( pRecipeGroup->mRowCnt == i+1 )
			inventoryIdx = pRecipeGroup->mInventoryIdx;
		else
			inventoryIdx = 0;

		retcode = SQLExecute( mHstmt );

		if ( (SQL_SUCCEEDED( retcode )) )
		{
			if( retValue == 0 )
			{				
				if ( SQLFetch( mHstmt ) == SQL_SUCCESS && inventoryIdx != 0 )
				{
					TB_INVENTORY* table = &pRecipeGroup->mInventory;
					SQLGetFullInv( mHstmt, table );
				}

				// 저장프로시저 쿼리 마무리.
				while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );
			}
			else
				pRecipeGroup->mRetvalue = retValue;
		}
		else
			return retcode;
	}

	// 열려있는 결과 닫기. (Close the open result set.)
	SQLCloseCursor( mHstmt );

	return retcode;
}

SQLRETURN cSQLGameStmt::FortuneSelect( FORTUNE_SELECT* fortuneSelect )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_FORTUNE_SELECT(?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER,	0, 0, &fortuneSelect->characterIdx,			0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		SQLINTEGER wordSize = sizeof(fortuneSelect->word);

		if ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_TYPE_TIMESTAMP,		&fortuneSelect->thru[0],	SQL_TIMESTAMP_LEN,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 2, SQL_C_TYPE_TIMESTAMP,		&fortuneSelect->thru[1],	SQL_TIMESTAMP_LEN,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 3, SQL_C_TYPE_TIMESTAMP,		&fortuneSelect->thru[2],	SQL_TIMESTAMP_LEN,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 4, SQL_C_TYPE_TIMESTAMP,		&fortuneSelect->thru[3],	SQL_TIMESTAMP_LEN,		  &(strLenOrInd=0) );
			SQLGetData( mHstmt, 5, SQL_C_TYPE_TIMESTAMP,		&fortuneSelect->thru[4],	SQL_TIMESTAMP_LEN,		  &(strLenOrInd=0) );

			SQLGetData( mHstmt, 6, SQL_C_WCHAR,		&fortuneSelect->word,	wordSize, &(strLenOrInd=0) );
			SQLGetData( mHstmt, 7, SQL_C_SLONG,		&fortuneSelect->color,	0,		  &(strLenOrInd=0) );
		}

		while( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		SQLCloseCursor( mHstmt );
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::FortuneUpdate( FORTUNE_UPDATE* fortuneUpdate )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_FORTUNE_UPDATE(?, ?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	long       wordSize = sizeof(fortuneUpdate->word);
	long       cbWord= wcslen(fortuneUpdate->word) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, 1, SQL_PARAM_INPUT,  SQL_C_SLONG,		   SQL_INTEGER,	       0,                 0, &fortuneUpdate->characterIdx,	0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 2, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &fortuneUpdate->thru[0],		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 3, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &fortuneUpdate->thru[1],		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 4, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &fortuneUpdate->thru[2],		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 5, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &fortuneUpdate->thru[3],		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 6, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &fortuneUpdate->thru[4],		0, &(strLenOrInd=0) );

	retcode = SQLBindParameter( mHstmt, 7, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 50, 0, &fortuneUpdate->word,	wordSize,	&cbWord );
	retcode = SQLBindParameter( mHstmt, 8, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,   0, 0, &fortuneUpdate->color,	0,			&(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, 9, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,	 0, 0, &fortuneUpdate->retvalue,0,			&(strLenOrInd=0) );
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

SQLRETURN cSQLGameStmt::ForcePointUpdate( FORCE_POINT_UPDATE* update )
{
	SQLCHAR* statement = (SQLCHAR*)"{call GP_CHARACTER_FORCEPOINT_UPDATE( ?, ?, ?, ?, ?, ?, ?, ?, ?, ? )}";
	SQLINTEGER strLenOrInd;
	SQLRETURN retcode;
	SQLUSMALLINT n = 0;	

	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mCharacterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mFirePoint, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mFireFriendly, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mWaterPoint, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mWaterFriendly, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mWindPoint, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mWindFriendly, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mEarthPoint, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_ULONG, SQL_INTEGER, 0, 0, &update->mEarthFriendly, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &update->mRetValue, 0, &(strLenOrInd=0) );
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

// PVPUserInsert Method.
SQLRETURN cSQLGameStmt::PVPUserInsert(PVP_USER_INSERT* insert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_PVP_USER_INSERT(?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &insert->pvpIdx,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &insert->characterChannel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &insert->characterIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT,  0, 0, &insert->characterLevel,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &insert->retvalue,         0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PVPUserSelect Method.
SQLRETURN cSQLGameStmt::PVPUserSelect(PVP_USER_SELECT* select, u_long& cbLength)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_PVP_USER_SELECT(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &select->pvpIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &select->minLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &select->maxLevel, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		PVP_USER* table = select->table;

		for ( int i = 0; SQLFetch( mHstmt ) == SQL_SUCCESS; i++, table++ )
		{
			SQLGetData( mHstmt, param=1, SQL_C_SHORT,   &table->characterChannel,   0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_SLONG,   &table->characterIdx,       0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->characterLevel,     0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++param, SQL_C_TINYINT, &table->characterForceType, 0, &(strLenOrInd=0) );
			++select->rowCount;
		}

		cbLength += (select->rowCount * sizeof(select->table));
		SQLClose( mHstmt );
	}

	return SQLClose( retcode, mHstmt );
}

// PVPUserRemove Method.
SQLRETURN cSQLGameStmt::PVPUserRemove(PVP_USER_REMOVE* remove)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_PVP_USER_DELETE(?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &remove->pvpIdx,           0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG, SQL_INTEGER,  0, 0, &remove->characterIdx,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &remove->retvalue,         0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PVPUserClear Method.
SQLRETURN cSQLGameStmt::PVPUserClear(PVP_USER_CLEAR* clear)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_PVP_USER_CLEAR(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SHORT, SQL_SMALLINT, 0, 0, &clear->pvpIdx,   0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_STINYINT, SQL_TINYINT,  0, 0, &clear->minLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT, SQL_C_STINYINT, SQL_TINYINT,  0, 0, &clear->maxLevel, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0, 0, &clear->retvalue, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// PartyUserInsert Method.
SQLRETURN cSQLGameStmt::PartyInsert(PARTY_INSERT* insert)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_PARTY_INSERT(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT_OUTPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &insert->partyIdx,         0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,        SQL_C_SLONG, SQL_INTEGER, 0, 0, &insert->leader.userIndex, 0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

// WhisperSelect Method.
SQLRETURN cSQLGameStmt::WhisperSelect(WHISPER_SELECT* select)
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_LOGIN(?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	long       nameSize = sizeof(select->name);
	long       cbName   = wcslen(select->name) ? SQL_NTS : SQL_NULL_DATA;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT, SQL_C_SLONG, SQL_INTEGER,  0,  0, &select->idx, 0,        &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_WCHAR, SQL_WVARCHAR, 16, 0, select->name, nameSize, &cbName          );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::GMJobUpdate( GM_JOB_UPDATE* update, u_long& cbLength ) 
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_CHARACTER_GM_JOB_UPDATE(?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;
	
	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &update->characterIdx, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &update->jobIndex,     0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_SLONG,   SQL_INTEGER, 0, 0, &update->jobStep,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,  SQL_C_TINYINT, SQL_TINYINT, 0, 0, &update->race,         0, &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	/// select  결과물 처리
	if ( SQL_SUCCEEDED( retcode ) )
	{
		/// 습득 스킬 목록은 가변길이
		unsigned long* table = update->skillClassIdx;
		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLGetData( mHstmt, 1, SQL_C_SLONG, table, 0, &(strLenOrInd=0) );

			++update->skillRowCount;
			++table;
		}

		cbLength += (update->skillRowCount * sizeof(update->skillClassIdx));

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::GMEventSelect( VERIFY_GM_EVENT* select, u_long& cbLength )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_VERIFY_GM_EVENT}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	/// select  결과물 처리
	if ( SQL_SUCCEEDED( retcode ) )
	{
		GM_INFLUENCE_LIST* table = select->table;

		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLSMALLINT	cnt;
			SQLGetData( mHstmt, cnt=1, SQL_C_SLONG, &table->index,  0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++cnt, SQL_C_SLONG, &table->infIndex,  0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++cnt, SQL_C_TYPE_TIMESTAMP, &table->startTime, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++cnt, SQL_C_TYPE_TIMESTAMP, &table->validThru, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			++select->rowcount;
			++table;
		}

		cbLength += ( select->rowcount * sizeof(select->table) );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::GMEventInsert( GM_EVENT_INSERT* insert )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_GM_EVENT_INSERT(?, ?, ?, ?, ?) }";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;
	
	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_OUTPUT,  SQL_C_SLONG,		  SQL_INTEGER,   0,					0, &insert->index,          0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,   SQL_C_SLONG,		  SQL_INTEGER,   0,					0, &insert->influenceIndex, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,	SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &insert->startTime,		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_INPUT,	SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &insert->validThru,		0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT,  SQL_C_SLONG,		  SQL_INTEGER,   0,				    0, &insert->retvalue,		0, &(strLenOrInd=0) );
	
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::GMEventUpdate( GM_EVENT_UPDATE* update )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_GM_EVENT_UPDATE(?, ?, ?) }";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;
	SQLSMALLINT	param;

	retcode = SQLBindParameter( mHstmt, param=1, SQL_PARAM_INPUT,  SQL_C_SLONG,	SQL_INTEGER, 0, 0, &update->index,			0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,	SQL_INTEGER, 0,	0, &update->influenceIndex, 0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++param, SQL_PARAM_OUTPUT, SQL_C_SLONG,	SQL_INTEGER, 0,	0, &update->retvalue,	    0, &(strLenOrInd=0) );

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	return SQLClose( retcode, mHstmt );
}

SQLRETURN cSQLGameStmt::ClearItemDropLimit( )
{
	SQLCHAR*  statement = (SQLCHAR*)"{call GP_ITEM_DROPLIMIT_CLEAR( ? ) }";
	SQLINTEGER  output = 0;
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output,      0, &(strLenOrInd=0) );
	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( output != 0 )
	{
		assert(NULL);
		return (SQLRETURN)output;
	}

	return retcode;
	
}

SQLRETURN cSQLGameStmt::InsertItemDropLimit( unsigned long itemIdx, unsigned long limitTime )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_ITEM_DROPLIMIT_INSERT(?, ?, ?, ?, ?, ?, ?, ?)}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;
	SQLINTEGER  output = 0;
	SQLINTEGER  output1 = 1;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,		SQL_INTEGER, 0, 0, &itemIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,		SQL_INTEGER, 0, 0, &limitTime,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output1,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output1,      0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &output,      0, &(strLenOrInd=0) );



	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	if ( SQL_SUCCEEDED( retcode ) )
	{
		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	if ( output != 0 )
	{
		assert(NULL);
		return (SQLRETURN)output;
	}

	return retcode;
}

SQLRETURN cSQLGameStmt::ItemDropLimitSelect( ITEM_DROPLIMIT_TABLE* select, u_long& cbLength )
{
	SQLCHAR*    statement = (SQLCHAR*)"{call GP_ITEM_DROPLIMIT_SELECT}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN   retcode;

	retcode = SQLExecDirect( mHstmt, statement, SQL_NTS );

	/// select  결과물 처리
	if ( SQL_SUCCEEDED( retcode ) )
	{
		TB_ITEM_DROPLIMIT* table = select->mTable;
		select->mCount = 0;

		while ( SQLFetch( mHstmt ) == SQL_SUCCESS )
		{
			SQLSMALLINT	cnt = 0;
			SQLGetData( mHstmt, ++cnt, SQL_C_SLONG, &table->mItemIdx,  0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++cnt, SQL_C_SLONG, &table->mDropTerm,  0, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++cnt, SQL_C_TYPE_TIMESTAMP, &table->mDropEndTime, SQL_TIMESTAMP_LEN, &(strLenOrInd=0) );
			SQLGetData( mHstmt, ++cnt, SQL_C_STINYINT, &table->mIsDrop,  0, &(strLenOrInd=0) );

			++select->mCount;
			++table;
		}

		cbLength += ( select->mCount * sizeof(select->mTable) );

		// 저장프로시저 쿼리 마무리.
		while ( SQLMoreResults( mHstmt ) == SQL_SUCCESS );

		// 열려있는 결과 닫기. (Close the open result set.)
		SQLCloseCursor( mHstmt );
	}

	return SQLClose( retcode, mHstmt );
}


SQLRETURN cSQLGameStmt::ItemDropLimitUpdate( ITEM_DROPLIMIT_UPDATE* pUpdate )
{
	SQLCHAR*	statement = (SQLCHAR*)"{call GP_ITEM_DROPLIMIT_UPDATE( ?, ?, ? )}";
	SQLINTEGER  strLenOrInd;
	SQLRETURN	retcode;

	SQLUSMALLINT n = 0;
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_SLONG,		SQL_INTEGER, 0, 0, &pUpdate->mItemIdx,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_INPUT,  SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, SQL_TIMESTAMP_LEN, 0, &pUpdate->mEndTime,  0, &(strLenOrInd=0) );
	retcode = SQLBindParameter( mHstmt, ++n, SQL_PARAM_OUTPUT, SQL_C_SLONG,		SQL_INTEGER, 0, 0, &pUpdate->mRetValue, 0, &(strLenOrInd=0) );

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
