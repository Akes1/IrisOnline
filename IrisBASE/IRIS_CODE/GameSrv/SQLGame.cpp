// Include
#include "gamesrv.h"
#include "stdafx.h"

#include "DramaturgyManager.h"
#include "Skillscript.h"
#include "Skill_Common.h"
#include "Player_Common.h"

// Local definitions
#pragma warning( disable: 4127 )

// memory를 안전하게 해제
#define SafeDelete(P) if (P!=NULL) { delete(P); (P)=NULL; }

/* macro for strtok checking */
#define TOKEN_CHECK(token, buffer, seps)					    \
	token = strtok( buffer, seps );								\
	if ( token == NULL ) throw false;

#define MAX_QUERY_LOG	30000

// Global data

// cSQLGame Constructor.
cSQLGame::cSQLGame(void) : mIoContextPool(NULL)
{
}

// ~cSQLGame Destructor.
cSQLGame::~cSQLGame(void)
{
}

// call_stored_procedure Method
bool cSQLGame::call_stored_procedure(char* dsn, char* uid, char* pwd, char* sp_name)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	if ( success == true )
	{
		SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
		SQLCHAR    statement[260];
		SQLRETURN  retcode;

		sprintf( (char*)statement, "{call %s}", sp_name );

		retcode = SQLExecDirect( hstmt, statement, SQL_NTS );
		success = SQL_SUCCEEDED( retcode );
		SQLCloseCursor( hstmt );
	}

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_inventory Method
bool cSQLGame::sp_inventory(char* dsn, char* uid, char* pwd, int& total, int& remove)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_INVENTORY", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &total, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;
	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_INVENTORY WHERE APPLY=1", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &remove, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_remove_inventory Method
bool cSQLGame::sp_remove_inventory(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_remove_inventory" );
}

// sp_delete_inventory Method
bool cSQLGame::sp_delete_inventory(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_delete_inventory" );
}

// sp_db_reindex_inventory Method
bool cSQLGame::sp_db_reindex_inventory(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_db_reindex_inventory" );
}

// sp_influence Method
bool cSQLGame::sp_influence(char* dsn, char* uid, char* pwd, int& total, int& remove)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_CHARACTER_INFLUENCE", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &total, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;
	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_CHARACTER_INFLUENCE WHERE IS_DEL=1", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &remove, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_remove_inventory Method
bool cSQLGame::sp_remove_influence(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_remove_influence" );
}

// sp_delete_influence Method
bool cSQLGame::sp_delete_influence(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_delete_influence" );
}

// sp_db_reindex_influence Method
bool cSQLGame::sp_db_reindex_influence(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_db_reindex_influence" );
}

// sp_post Method
bool cSQLGame::sp_post(char* dsn, char* uid, char* pwd, int& total, int& remove)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_POST", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &total, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;
	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_POST WHERE STATUS=2", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &remove, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_remove_post Method
bool cSQLGame::sp_remove_post(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_remove_post" );
}

// sp_delete_post Method
bool cSQLGame::sp_delete_post(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_delete_post" );
}

// sp_db_reindex_post Method
bool cSQLGame::sp_db_reindex_post(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_db_reindex_post" );
}

// sp_agent Method
bool cSQLGame::sp_agent(char* dsn, char* uid, char* pwd, int& total, int& remove)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_ITEM_AGENT", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &total, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;
	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_ITEM_AGENT WHERE APPLY=1", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &remove, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_remove_agent Method
bool cSQLGame::sp_remove_agent(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_remove_agent" );
}

// sp_delete_agent Method
bool cSQLGame::sp_delete_agent(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_delete_agent" );
}

// sp_db_reindex_agent Method
bool cSQLGame::sp_db_reindex_agent(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_db_reindex_agent" );
}


// sp_quest_progress Method
bool cSQLGame::sp_quest_progress(char* dsn, char* uid, char* pwd, int& total, int& remove)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_QUEST_PROGRESS", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &total, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;
	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_QUEST_PROGRESS WHERE DELETEWAIT=1", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &remove, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_remove_quest_progress Method
bool cSQLGame::sp_remove_quest_progress(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_remove_quest_progress" );
}

// sp_delete_quest_progress Method
bool cSQLGame::sp_delete_quest_progress(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_delete_quest_progress" );
}

// sp_db_reindex_quest_progress Method
bool cSQLGame::sp_db_reindex_quest_progress(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_db_reindex_quest_progress" );
}


// sp_quest_progress Method
bool cSQLGame::sp_quest_valid(char* dsn, char* uid, char* pwd, int& total, int& remove)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	bool             success       = false;

	if ( sqlEnv->AllocEnv( ) == false );
	else if ( sqlEnv->SetEnvAttr( ) == false );
	else if ( sqlConnection->AllocDbc( sqlEnv ) == false );
	else if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false );
	else if ( sqlStatement->AllocStmt( sqlConnection ) == false );
	else success = true;

	SQLHSTMT   hstmt = sqlStatement->GetSqlStmt( );
	SQLINTEGER strLenOrInd;
	SQLRETURN  retcode;

	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_QUEST_VALID", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &total, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;
	retcode = SQLExecDirect( hstmt, (SQLCHAR*)"SELECT COUNT(*) FROM TB_QUEST_VALID WHERE DELETEWAIT=1", SQL_NTS );
	if ( SQL_SUCCEEDED( retcode ) && SQLFetch( hstmt ) == SQL_SUCCESS )
	{
		SQLGetData( hstmt, 0x01, SQL_C_LONG, &remove, 0, &(strLenOrInd=0) );
		SQLCloseCursor( hstmt );
	} else success = false;

	SafeDelete( sqlStatement );
	SafeDelete( sqlConnection );
	SafeDelete( sqlEnv );
	return success;
}

// sp_remove_quest_valid Method
bool cSQLGame::sp_remove_quest_valid(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_remove_quest_valid" );
}

// sp_delete_quest_valid Method
bool cSQLGame::sp_delete_quest_valid(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_delete_quest_valid" );
}

// sp_db_reindex_quest_valid Method
bool cSQLGame::sp_db_reindex_quest_valid(char* dsn, char* uid, char* pwd)
{
	return call_stored_procedure( dsn, uid, pwd, "sp_db_reindex_quest_valid" );
}

// ConsoleInsertInit Method
bool cSQLGame::ConsoleInsertInit()
{
	mpSkillScript = new cSkillScript;
	if ( !mpSkillScript->Init( ) )
		return false;

	mpTriggerManager = new cTriggerManager;
	if ( !mpTriggerManager->Init( ) )
		return false;

	mpQuestManager = new cQuestManager;
	if ( !mpQuestManager->Init( ) )
		return false;

	mpTitleManager = new cTitleManager;
	if( !mpTitleManager->Init() )
		return false;
	
	mpStatusScript = new cStatusScript;
	if( !mpStatusScript->Init() )
		return false;

	mpDropScript = new cDropScript;
	if( !mpDropScript->Init( true ) )
		return false;

	return true;
}

// ConsoleInsertRelease Method
void cSQLGame::ConsoleInsertRelease()
{
	mpDropScript->Release();
	delete mpDropScript;

	mpSkillScript->Release();
	delete mpSkillScript;

	mpStatusScript->Release();
	delete mpStatusScript;
}

// InsertItemDefine Method
bool cSQLGame::InsertItemDefine(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Define.txt", "rt" );

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

		if ( sqlStatement->TruncateItemDefine( ) != SQL_SUCCESS )
			throw false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char           buffer[4096];
		int            bufferLen = sizeof(buffer)/sizeof(char);

		char           seps[] = "\t";           // 파싱구문 Parsing.
		char*          token;                   // 파싱토근 Parsing Token.

		TB_ITEM_DEFINE tbItemDefine;
		SQLRETURN      sqlReturn;
		long           retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemDefine, 0, sizeof(TB_ITEM_DEFINE) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// 인덱스(INDEX)
			TOKEN_CHECK( token, buffer, seps );
			tbItemDefine.index = atoi( token );

			// 이름(NAME)
			TOKEN_CHECK( token, NULL, seps );
			MultiByteToWideChar( CP_ACP, 0, token, (int)strlen(token)+1, tbItemDefine.name, (int)(sizeof(tbItemDefine.name)/sizeof(wchar_t)) );

			// 이름 인덱스(NAME INDEX)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// 툴팁 인덱스(TOOL TIP INDEX)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// 종류(TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.type = (BYTE)atoi( token );

			// 세부종류(SUBTYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.subType = (BYTE)atoi( token );

			// 검색종류-대분류(SEARCH TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.searchType = (BYTE)atoi( token );

			// 검색종류-중분류(SEARCH SUBTYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.searchSubType = (short)atoi( token );

			// 검색종류-소분류
			TOKEN_CHECK( token, NULL, seps );
			// SKIP

			// 무게(WEIGHT)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.weight = (short)atoi( token );

			// 겹침개수(CAPACITY)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.capacity = (short)atoi( token );

			// 판매방법(SELL TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.sellType = atoi( token );

			// 판매가(SELL PRICE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.sellPrice = atoi( token );

			// 구입방법(BUY TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.buyType = atoi( token );

			// 구입가(BUY PRICE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.buyPrice = atoi( token );

			// 최대 출현갯수(MAX PRESENT)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.maxPresent = atoi( token );

			// 보유가능개수(MAX INVENTORY)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.maxInventory = atoi( token );

			// 기간제설정(LICENSE CLASS)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.licenseClass = (BYTE)atoi( token );

			// 사용기한(LICENSE VALID-THRU)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.licenseValidThru = atoi( token );

			// 인증여부
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.licenseType = (BYTE)atoi( token );

			// 사용자거래여부(EXCHANGE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.exchange = (BYTE)atoi( token );

			// 봉인설정 
			//TOKEN_CHECK( token, NULL, seps );
			//tbItemDefine.sealcount = (BYTE)atoi( token );


			// 속성 인덱스(ABILITY INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.abilityIndex = atoi( token );

			// 제한 인덱스(LIMIT INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.limitIndex = atoi( token );

			// 카드 인덱스(CARD INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.cardIndex = atoi( token );

			// 타로 인덱스(TAROT INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.tarotIndex = atoi( token );

			// 스프레드 인덱스(SPREAD INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.spreadIndex = atoi( token );

			// 분해 인덱스(DISJOINT INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.disjointIndex = atoi( token );

			// 카드슬롯 인덱스(CARD SLOT INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.cardSlotIndex = atoi( token );

			// 강화확률 인덱스(ENHANCED RATE INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.enhancedRateIndex = atoi( token );

			// 강화 인덱스(ENHANCED INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.enhancedIndex = atoi( token );

			// 교환 인덱스(CHANGE INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.changeIndex = atoi( token );

			// 체인지_인덱스
			//TOKEN_CHECK( token, NULL, seps );
			// skip
			
			// 호칭 인덱스(TITLE INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.titleIndex = atoi( token );

			// 세트 인덱스(SET INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.setIndex = atoi( token );

			// 아이콘 인덱스(ICON INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.iconIndex = atoi( token );

			// 모델 인덱스0-오른손(MODEL INDEX)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// 모델 인덱스0-왼  손(MODEL INDEX)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// 투구 여부 ON(1)/OFF(0)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// 레어수준(RARE LEVEL)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.rareLevel = (BYTE)atoi( token );

			// 웹 페이지 보이기(WEB PAGE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemDefine.webPage = (BYTE)atoi( token );

			sqlReturn = sqlStatement->InsertItemDefine( &tbItemDefine, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemDefine, 0, sizeof(TB_ITEM_DEFINE) );
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

// InsertItemAbility Method
bool cSQLGame::InsertItemAbility(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Ability.txt", "rt" );

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

		if ( sqlStatement->TruncateItemAbility( ) != SQL_SUCCESS )
			throw false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_ABILITY tbItemAbility;
		SQLRETURN       sqlReturn;
		long            retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemAbility, 0, sizeof(TB_ITEM_ABILITY) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// 이름(NAME)
			// TOKEN_CHECK( token, buffer, seps );
			// Skip

			// 인덱스(INDEX)
			TOKEN_CHECK( token, buffer, seps );
			tbItemAbility.index = atoi( token );

			// 사용효과설명 인덱스(DESCRIPTION INDEX)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// 방어구재질(DEFENSE TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.def_class = (short)atoi( token );

			// 범위방식(RANGE_TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.range_type = (BYTE)atoi( token );

			// 효과적용타겟(TARGET_TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.target_type = (BYTE)atoi( token );

			// 사용거리(USE_RANGE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.use_range = (short)atoi( token );

			// 물리방어력(DEFENSE P)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.p_def = (short)atoi( token );

			// 마법방어력(DEFENSE M)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.m_def = (short)atoi( token );

			// 사거리(ATTACK RANGE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.atk_range = (short)atoi( token );

			// 공격애니속도(ATTACK SPEED)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.atk_spd = (short)atoi( token );

			// 재사용시간(COOL TIME1)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.cool_time1 = atoi( token );

			// 그룹쿨타임(COOL TIME2)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.cool_time2 = atoi( token );

			// 물리공격력-소(ATTACK P MIN)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.min_p_atk = (short)atoi( token );

			// 물리공격력-대(ATTACK P MAX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.max_p_atk = (short)atoi( token );

			// 마법공격력-소(ATTACK M MIN)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.min_m_atk = (short)atoi( token );

			// 마법공격력-대(ATTACK_M_MAX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.max_m_atk = (short)atoi( token );

			// 힐 회목량(HEAL)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.heal = (short)atoi( token );

			// 옵션01인덱스(OPT01_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt01_index = (short)atoi( token );
			// 옵션01효과값(OPT01_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt01_value = (short)atoi( token );

			// 옵션02인덱스(OPT02_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt02_index = (short)atoi( token );
			// 옵션02효과값(OPT02_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt02_value = (short)atoi( token );

			// 옵션03인덱스(OPT03_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt03_index = (short)atoi( token );
			// 옵션03효과값(OPT03_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt03_value = (short)atoi( token );

			// 옵션04인덱스(OPT04_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt04_index = (short)atoi( token );
			// 옵션04효과값(OPT04_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt04_value = (short)atoi( token );

			// 옵션05인덱스(OPT05_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt05_index = (short)atoi( token );
			// 옵션05효과값(OPT05_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.opt05_value = (short)atoi( token );

			// 소모효과(INFLUENCE_IDX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.influence_idx = atoi( token );

			// 소모효과(INFLUENCE_IDX)
			TOKEN_CHECK( token, NULL, seps );
			tbItemAbility.active_index = atoi( token );

			sqlReturn = sqlStatement->InsertItemAbility( &tbItemAbility, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemAbility, 0, sizeof(TB_ITEM_ABILITY) );
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

// InsertItemLimit Method
bool cSQLGame::InsertItemLimit(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Limit.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemLimit( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_LIMIT   tbItemLimit;
		SQLRETURN       sqlReturn;
		long            retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemLimit, 0, sizeof(TB_ITEM_LIMIT) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX
			TOKEN_CHECK( token, buffer, seps );
			tbItemLimit.index = atoi( token );
			// CHAR_RACE
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.charRace = (BYTE)atoi( token );
			// CHAR_GENDER
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.charGender = (BYTE)atoi( token );
			// CHAR_JOB1
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.charJob1 = atoi( token );
			// CHAR_JOB2
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.charJob2 = atoi( token );
			// CHAR_LEVEL_MIN
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.charLevelMin = (BYTE)atoi( token );
			// CHAR_LEVEL_MAX
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.charLevelMax = (BYTE)atoi( token );
			// SERVER_TYPE
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.serverType = (BYTE)atoi( token );
			// FORCE_TYPE
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.forceType = (BYTE)atoi( token );
			// MAKE_SKILL
			TOKEN_CHECK( token, NULL, seps );
			tbItemLimit.makeSkill = (BYTE)atoi( token );
			// MAKE_SKILL_EXP
			TOKEN_CHECK( token, NULL, seps );
            tbItemLimit.makeSkillExp = atoi( token );

			sqlReturn = sqlStatement->InsertItemLimit( &tbItemLimit, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemLimit, 0, sizeof(TB_ITEM_LIMIT) );
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

// InsertItemCard Method
bool cSQLGame::InsertItemCard(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Card.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemCard( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_CARD    tbItemCard;
		SQLRETURN       sqlReturn;
		long            retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemCard, 0, sizeof(TB_ITEM_CARD) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// 카드명(NAME)
			// TOKEN_CHECK( token, buffer, seps );
			// Skip

			// 인덱스(INDEX)
			TOKEN_CHECK( token, buffer, seps );
			tbItemCard.index = (long)atoi( token );

			// 판(EPISODE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemCard.episode = (long)atoi( token );

			// 카드넘버(NUMBER)
			TOKEN_CHECK( token, NULL, seps );
			tbItemCard.number = (long)atoi( token );

			// 희귀도(LEVEL)
			TOKEN_CHECK( token, NULL, seps );
			tbItemCard.level = (BYTE)atoi( token );

			// 카드분류(TYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemCard.type = (BYTE)atoi( token );

			// 장착위치(SUBTYPE)
			TOKEN_CHECK( token, NULL, seps );
			tbItemCard.subType = (BYTE)atoi( token );

			// 이미지인덱스(IMAGE_INDEX)
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			sqlReturn = sqlStatement->InsertItemCard( &tbItemCard, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemCard, 0, sizeof(TB_ITEM_CARD) );
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

// InsertItemCardSlot Method
bool cSQLGame::InsertItemCardSlot(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Card_Slot.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemCardSlot( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char              buffer[4096];
		int               bufferLen = sizeof(buffer)/sizeof(char);

		char              seps[] = "\t";           // 파싱구문 Parsing.
		char*             token;                   // 파싱토근 Parsing Token.

		TB_ITEM_CARD_SLOT itemCardSlot;
		SQLRETURN         sqlReturn;
		long              retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &itemCardSlot, 0, sizeof(itemCardSlot) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// 인덱스(INDEX)
			TOKEN_CHECK( token, buffer, seps );
			itemCardSlot.index = atoi( token );

			// 카드슬롯1(0없음/1A형/2B형/3AB형/4O형)
			TOKEN_CHECK( token, NULL, seps );
			itemCardSlot.slot1 = (BYTE)atoi( token );

			// 카드슬롯2(0없음/1A형/2B형/3AB형/4O형)
			TOKEN_CHECK( token, NULL, seps );
			itemCardSlot.slot2 = (BYTE)atoi( token );

			// 카드슬롯3(0없음/1A형/2B형/3AB형/4O형)
			TOKEN_CHECK( token, NULL, seps );
			itemCardSlot.slot3 = (BYTE)atoi( token );

			// 카드슬롯4(0없음/1A형/2B형/3AB형/4O형)
			TOKEN_CHECK( token, NULL, seps );
			itemCardSlot.slot4 = (BYTE)atoi( token );

			// 카드슬롯5(0없음/1A형/2B형/3AB형/4O형)
			TOKEN_CHECK( token, NULL, seps );
			itemCardSlot.slot5 = (BYTE)atoi( token );

			sqlReturn = sqlStatement->InsertItemCardSlot( &itemCardSlot, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &itemCardSlot, 0, sizeof(itemCardSlot) );
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

// InsertItemTarot Method
bool cSQLGame::InsertItemTarot(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Tarot.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemTarot( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_TAROT   tbItemTarot;
		SQLRETURN       sqlReturn;
		long            retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemTarot, 0, sizeof(TB_ITEM_TAROT) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX
			TOKEN_CHECK( token, buffer, seps );
			tbItemTarot.index = atoi( token );

			// IMG_INDEX
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// NUMBER
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarot.number = atoi( token );

			// LEVEL
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarot.level = (BYTE)atoi( token );

			// STR_VALUE
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarot.strValue = atoi( token );

			// INV_VALUE
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarot.invValue = atoi( token );

			// PROP
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarot.prop = atoi( token );

			// EPISODE
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarot.episode = atoi( token );

			// STR_DESCRIPTION
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// INV_DESCRIPTION
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			sqlReturn = sqlStatement->InsertItemTarot( &tbItemTarot, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemTarot, 0, sizeof(TB_ITEM_TAROT) );
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

// InsertItemTarotResult Method
bool cSQLGame::InsertItemTarotResult(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Tarot_Result.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemTarotResult( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char                 buffer[4096];
		int                  bufferLen = sizeof(buffer)/sizeof(char);

		char                 seps[] = "\t";           // 파싱구문 Parsing.
		char*                token;                   // 파싱토근 Parsing Token.

		TB_ITEM_TAROT_RESULT tbItemTarotResult;
		SQLRETURN            sqlReturn;
		long                 retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemTarotResult, 0, sizeof(TB_ITEM_TAROT_RESULT) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX
			TOKEN_CHECK( token, buffer, seps );
			tbItemTarotResult.index = (long)atoi( token );

			// MIN
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarotResult.min_value = (long)atoi( token );

			// MAX
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarotResult.max_value = (long)atoi( token );

			// INFLUENCE_IDX1
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarotResult.influence_idx1 = (long)atoi( token );

			// INFLUENCE_IDX2
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarotResult.influence_idx2 = (long)atoi( token );

			// INFLUENCE_IDX3
			TOKEN_CHECK( token, NULL, seps );
			tbItemTarotResult.influence_idx3 = (long)atoi( token );

			sqlReturn = sqlStatement->InsertItemTarotResult( &tbItemTarotResult, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemTarotResult, 0, sizeof(TB_ITEM_TAROT_RESULT) );
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

// InsertItemSpread Method
bool cSQLGame::InsertItemSpread(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Spread.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemSpread( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_SPREAD   tbItemSpread;
		SQLRETURN        sqlReturn;
		long             retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemSpread, 0, sizeof(TB_ITEM_SPREAD) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX
			TOKEN_CHECK( token, buffer, seps );
			tbItemSpread.index = atoi( token );

			// IMG_INDEX
			TOKEN_CHECK( token, NULL, seps );
			// Skip

			// AXIS_ARRAY
			TOKEN_CHECK( token, NULL, seps );
			tbItemSpread.axisArray = (BYTE)atoi( token );

			// XPOS
			TOKEN_CHECK( token, NULL, seps );
			tbItemSpread.xPos = (short)atoi( token );

			// YPOS
			TOKEN_CHECK( token, NULL, seps );
			tbItemSpread.yPos = (short)atoi( token );

			// DIR
			TOKEN_CHECK( token, NULL, seps );
			tbItemSpread.dir = (BYTE)atoi( token );

			// CALC
			TOKEN_CHECK( token, NULL, seps );
			strncpy( tbItemSpread.calc, token, 1 );

			// VALUE
			TOKEN_CHECK( token, NULL, seps );
			tbItemSpread.value = (short)atoi( token );

			sqlReturn = sqlStatement->InsertItemSpread( &tbItemSpread, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemSpread, 0, sizeof(TB_ITEM_SPREAD) );
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

// InsertItemDisjoint Method
bool cSQLGame::InsertItemDisjoint(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Disjoint.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemDisjoint( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_DISJOINT itemDisjoint;
		SQLRETURN        sqlReturn;
		long             retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &itemDisjoint, 0, sizeof(itemDisjoint) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX
			TOKEN_CHECK( token, buffer, seps );
			itemDisjoint.index = atoi( token );

			// PRICE
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.price = atoi( token );

			// ITEM_DEFINE_INDEX1
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.itemDefineIndex1 = atoi( token );

			// INVENTORY_COUNT1
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.inventoryCount1 = (short)atoi( token );

			// ITEM_DEFINE_INDEX2
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.itemDefineIndex2 = atoi( token );

			// INVENTORY_COUNT2
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.inventoryCount2 = (short)atoi( token );

			// ITEM_DEFINE_INDEX3
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.itemDefineIndex3 = atoi( token );

			// INVENTORY_COUNT3
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.inventoryCount3 = (short)atoi( token );

			// ITEM_DEFINE_INDEX4
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.itemDefineIndex4 = atoi( token );

			// INVENTORY_COUNT4
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.inventoryCount4 = (short)atoi( token );

			// ITEM_DEFINE_INDEX5
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.itemDefineIndex5 = atoi( token );

			// INVENTORY_COUNT5
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.inventoryCount5 = (short)atoi( token );

			// ITEM_DEFINE_INDEX6
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.itemDefineIndex6 = atoi( token );

			// INVENTORY_COUNT6
			TOKEN_CHECK( token, NULL, seps );
			itemDisjoint.inventoryCount6 = (short)atoi( token );

			sqlReturn = sqlStatement->InsertItemDisjoint( &itemDisjoint, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &itemDisjoint, 0, sizeof(itemDisjoint) );
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

// InsertItemEnhanced Method
bool cSQLGame::InsertItemEnhanced(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Enhanced.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemEnhanced( ) != SQL_SUCCESS )
			return false;

		if ( sqlStatement->TruncateItemEnhancedValue( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_ENHANCED tbItemEnhanced;
		SQLRETURN        sqlReturn;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemEnhanced, 0, sizeof(TB_ITEM_ENHANCED) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}
			else if ( !strnicmp( buffer, "\t", 1 ) )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX - 구문 문자열
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "index" ) )
				throw false;

			// INDEX - 값
			TOKEN_CHECK( token, NULL, seps );
			tbItemEnhanced.index = atoi( token );

			for ( int i0 = 0; i0 < 3; i0++ )
			{
				// 다음 줄
				if ( fgets( buffer, bufferLen, stream ) == NULL )
					throw false;

				// VALUE - 속성 및 강화 값
				TOKEN_CHECK( token, buffer, seps );

				if ( stricmp( token, "end" ) )
				{
					
					//tbItemEnhanced.value[ i0 ].equip= (BYTE)atoi( token );
					//TOKEN_CHECK( token, NULL, seps );
					tbItemEnhanced.value[ i0 ].type = (BYTE)atoi( token );
					
				}
				else
					break;

				for ( int i1 = 0; i1 < MAX_ITEM_ENHANCED; i1++ )
				{
					TOKEN_CHECK( token, NULL, seps );
					tbItemEnhanced.value[ i0 ].value[ i1 ] = (short)atoi( token );
				}

				tbItemEnhanced.row++;
			}

			sqlReturn = sqlStatement->InsertItemEnhanced( &tbItemEnhanced );
			if ( sqlReturn != SQL_SUCCESS )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemEnhanced, 0, sizeof(TB_ITEM_ENHANCED) );
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

// InsertItemEnhancedRate Method
bool cSQLGame::InsertItemEnhancedRate(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_EnhancedRate.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemEnhancedRate( ) != SQL_SUCCESS )
			return false;

		if ( sqlStatement->TruncateItemEnhancedSuccess( ) != SQL_SUCCESS )
			return false;

		if ( sqlStatement->TruncateItemEnhancedDown( ) != SQL_SUCCESS )
			return false;

		if ( sqlStatement->TruncateItemEnhancedDestruct( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_ENHANCED_RATE tbItemEnhancedRate;
		SQLRETURN             sqlReturn;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbItemEnhancedRate, 0, sizeof(TB_ITEM_ENHANCED_RATE) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX - 구문 문자열
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "index" ) )
				throw false;

			// INDEX - 값
			TOKEN_CHECK( token, NULL, seps );
			tbItemEnhancedRate.index = atoi( token );

			for ( int i0 = 0; i0 < 2; i0++ )
			{
				// TYPE 1
				if ( fgets( buffer, bufferLen, stream ) == NULL )
					throw false;

				TOKEN_CHECK( token, buffer, seps );
				tbItemEnhancedRate.value[ i0 ].type = (BYTE)atoi( token );

				// SUCCESS VALUES
				if ( fgets( buffer, bufferLen, stream ) == NULL )
					throw false;

				TOKEN_CHECK( token, buffer, seps );
				tbItemEnhancedRate.value[ i0 ].success[ 0 ] = (long)(atof( token ) * 1000000);

				for ( int i = 1; i < MAX_ITEM_ENHANCED; i++ )
				{
					TOKEN_CHECK( token, NULL, seps );
					tbItemEnhancedRate.value[ i0 ].success[ i ] = (long)(atof( token ) * 1000000);
				}

				// DOWN VALUES
				if ( fgets( buffer, bufferLen, stream ) == NULL )
					throw false;

				TOKEN_CHECK( token, buffer, seps );
				tbItemEnhancedRate.value[ i0 ].down[ 0 ] = (long)(atof( token ) * 1000000);

				for ( int i = 1; i < MAX_ITEM_ENHANCED; i++ )
				{
					TOKEN_CHECK( token, NULL, seps );
					tbItemEnhancedRate.value[ i0 ].down[ i ] = (long)(atof( token ) * 1000000);
				}

				// DESTRUCT VALUES
				if ( fgets( buffer, bufferLen, stream ) == NULL )
					throw false;

				TOKEN_CHECK( token, buffer, seps );
				tbItemEnhancedRate.value[ i0 ].destruct[ 0 ] = (long)(atof( token ) * 1000000);

				for ( int i = 1; i < MAX_ITEM_ENHANCED; i++ )
				{
					TOKEN_CHECK( token, NULL, seps );
					tbItemEnhancedRate.value[ i0 ].destruct[ i ] = (long)(atof( token ) * 1000000);
				}
			}

			if ( fgets( buffer, bufferLen, stream ) == NULL )
				throw false;

			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "end" ) )
				throw false;

			sqlReturn = sqlStatement->InsertItemEnhancedRate( &tbItemEnhancedRate );
			if ( sqlReturn != SQL_SUCCESS )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbItemEnhancedRate, 0, sizeof(TB_ITEM_ENHANCED_RATE) );
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

// InsertItemChange Method
bool cSQLGame::InsertItemChange(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Item_Change.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateItemChange( ) != SQL_SUCCESS )
			return false;

		if ( sqlStatement->TruncateItemChangeItems( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t\n";         // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_ITEM_CHANGE  itemChange;
		SQLRETURN       sqlReturn;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &itemChange, 0, sizeof(TB_ITEM_CHANGE) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// INDEX - 구문 문자열
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "index" ) )
				throw false;

			// INDEX - 값
			TOKEN_CHECK( token, NULL, seps );
			itemChange.index = atoi( token );

			if ( fgets( buffer, bufferLen, stream ) == NULL )
				throw false;

			// MONEY - 구문 문자열
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "money" ) )
				throw false;

			// Min Money
			TOKEN_CHECK( token, NULL, seps );
			itemChange.minMoney = atoi( token );

			// Max Money
			TOKEN_CHECK( token, NULL, seps );
			itemChange.maxMoney = atoi( token );

			if ( fgets( buffer, bufferLen, stream ) == NULL )
				throw false;

			// RATE
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "rate" ) )
				throw false;

			TOKEN_CHECK( token, NULL, seps );
			itemChange.rate = atoi( token );

			// TB_ITEM_DEFINE::INDEX.
			if ( fgets( buffer, bufferLen, stream ) == NULL )
				throw false;
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "item" ) )
				throw false;

			token = strtok( NULL, seps );
			for ( int i = 0; i < MAX_ITEM_CHANGE_DATA && token != NULL; i++ )
			{
				itemChange.itemDefineIndex[ i ] = atoi( token );
				token = strtok( NULL, seps );
				itemChange.row++;
			}

			// TB_INVENTORY::COUNT.
			if ( fgets( buffer, bufferLen, stream ) == NULL )
				throw false;
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "count" ) )
				throw false;

			token = strtok( NULL, seps );
			for ( int i = 0; i < itemChange.row && token != NULL; i++ )
			{
				itemChange.itemCount[ i ] = (short)atoi( token );
				token = strtok( NULL, seps );
			}

			// enhanced.
			//if ( fgets( buffer, bufferLen, stream ) == NULL )
			//	throw false;
			//TOKEN_CHECK( token, buffer, seps );
			//if ( stricmp( token, "enhanced" ) )
			//	throw false;

			token = strtok( NULL, seps );
			for ( int i = 0; i < itemChange.row && token != NULL; i++ )
			{
				itemChange.enhanced[ i ] = (short)atoi( token );
				token = strtok( NULL, seps );
			}

			// CHANGE RATE
			if ( fgets( buffer, bufferLen, stream ) == NULL )
				throw false;
			TOKEN_CHECK( token, buffer, seps );
			if ( stricmp( token, "changerate" ) )
				throw false;

			token = strtok( NULL, seps );
			for ( int i = 0; i < itemChange.row && token != NULL; i++ )
			{
				itemChange.changeRate[ i ] = atoi( token );
				token = strtok( NULL, seps );
			}

			sqlReturn = sqlStatement->InsertItemChange( &itemChange );
			if ( sqlReturn != SQL_SUCCESS )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &itemChange, 0, sizeof(TB_ITEM_CHANGE) );
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

// InsertDefaultItems Method
bool cSQLGame::InsertDefaultItems(char* dsn, char* uid, char* pwd)
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );
	FILE*            stream        = fopen( "Script\\Resource\\Default_Items.txt", "rt" );

	try
	{
		//ODBC, SQL 사용을 초기화.
		if ( sqlEnv->AllocEnv( ) == false )
			return false;

		//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
		if ( sqlEnv->SetEnvAttr( ) == false )
			return false;

		if ( sqlConnection->AllocDbc( sqlEnv ) == false )
			return false;

		if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
			return false;

		if ( sqlStatement->AllocStmt( sqlConnection ) == false )
			return false;

		if ( sqlStatement->TruncateDefaultItems( ) != SQL_SUCCESS )
			return false;

		if ( stream == NULL )
			throw false;

		// Parsing.
		char            buffer[4096];
		int             bufferLen = sizeof(buffer)/sizeof(char);

		char            seps[] = "\t";           // 파싱구문 Parsing.
		char*           token;                   // 파싱토근 Parsing Token.

		TB_DEFAULT_ITEMS tbDefaultItems;
		SQLRETURN        sqlReturn;
		long             retvalue;

		fseek( stream, 0, SEEK_END );
		fseek( stream, 0, SEEK_SET );

		memset( buffer, 0, bufferLen );
		memset( &tbDefaultItems, 0, sizeof(TB_DEFAULT_ITEMS) );

		// fgets 에서 문자열로 읽어 올 경우, 0x0A(\r,CR) 값은 생략된다.
		while ( fgets( buffer, bufferLen, stream ) != NULL )
		{
			if ( !strnicmp( buffer, "//", 2 ) || !strnicmp( buffer, "\r\n", 2 ) || (*buffer) == 0x0A )
			{
				memset( buffer, 0, bufferLen );
				continue;
			}

			// CHARACTER_JOB
			TOKEN_CHECK( token, buffer, seps );
			tbDefaultItems.characterJob = (long)atoi( token );

			// ITEM_DEFINE_INDEX
			TOKEN_CHECK( token, NULL, seps );
			tbDefaultItems.itemDefineIndex = (long)atoi( token );

			// INVENTORY_NUMBER
			TOKEN_CHECK( token, NULL, seps );
			tbDefaultItems.inventoryNumber = (short)atoi( token );

			// INVENTORY_COUNT
			TOKEN_CHECK( token, NULL, seps );
			tbDefaultItems.inventoryCount = (short)atoi( token );

			sqlReturn = sqlStatement->InsertDefaultItems( &tbDefaultItems, retvalue );
			if ( sqlReturn != SQL_SUCCESS || retvalue != 0 )
				throw false;

			memset( buffer, 0, bufferLen );
			memset( &tbDefaultItems, 0, sizeof(TB_DEFAULT_ITEMS) );
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

bool cSQLGame::InsertSkillPC( char* dsn, char* uid, char* pwd )
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );

	//ODBC, SQL 사용을 초기화.
	if ( sqlEnv->AllocEnv( ) == false )
		return false;

	//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
	if ( sqlEnv->SetEnvAttr( ) == false )
		return false;

	if ( sqlConnection->AllocDbc( sqlEnv ) == false )
		return false;

	if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
		return false;

	if ( sqlStatement->AllocStmt( sqlConnection ) == false )
		return false;

	if ( sqlStatement->TruncateSkillListPc( ) != SQL_SUCCESS )
		return false;

	typedef tPointerHashMap<unsigned long, void*> cSkillHashMap;

	cSkillHashMap::cIterator start = mpSkillScript->GetPlayerSkillInfoMap()->Begin();
	cSkillHashMap::cIterator end = mpSkillScript->GetPlayerSkillInfoMap()->End();

	for( ; start != end ; ++start )
	{
		if ( sqlStatement->SkillListPcInsert( (*start).mSecond ) != SQL_SUCCESS )
			return false;
	}
	return true;
}

bool cSQLGame::InsertDefaultSkill( char* dsn, char* uid, char* pwd )
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );

	//ODBC, SQL 사용을 초기화.
	if ( sqlEnv->AllocEnv( ) == false )
		return false;

	//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
	if ( sqlEnv->SetEnvAttr( ) == false )
		return false;

	if ( sqlConnection->AllocDbc( sqlEnv ) == false )
		return false;

	if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
		return false;

	if ( sqlStatement->AllocStmt( sqlConnection ) == false )
		return false;

	if ( sqlStatement->TruncateDefalultSkill( ) != SQL_SUCCESS )
		return false;

	typedef tArray<unsigned long> cSkillAry;

	for( int i=0; i<10; i++ )
	{
		cSkillAry* pSkillAry = SKILLSCRIPT->GetFreeSkill( i );
		if( pSkillAry )
		{
			cSkillAry::cIterator begin = pSkillAry->Begin();
			cSkillAry::cIterator end = pSkillAry->End();
			for( ; begin != end; ++begin )
			{
				sPlayerSkillBaseInfo* pBase = mpSkillScript->GetPlayerSkillInfo( *begin );
				if( pBase == NULL )
					return false;

				if( pBase->mStepCount == 0 )
					continue;

				sPlayerSkillStepInfo* pStep = &pBase->mpSetpInfoArray[0];
				if( pStep == NULL )
					return false;

				unsigned char skillRace = pBase->mRace;
				cSkillAry* pJobAry = SKILLSCRIPT->GetPlayerSkillJob( pStep->mJobType );
				if( pJobAry == NULL || pJobAry->GetSize() == 0)
				{
					unsigned long jobIdx = 0;

					for( char playerRace = eRACE_HUMAN ; playerRace < eRACE_MAX ; ++playerRace )
					{
						if( skillRace == eSKILLRACE_ALL )
						{
							if ( sqlStatement->DefalultSkillInsert( i, *begin, playerRace, jobIdx ) != SQL_SUCCESS )
								return false;

							continue;
						}

						switch( playerRace )
						{
						case eRACE_HUMAN:
							{
								if( skillRace == eSKILLRACE_HUMAN ||
									skillRace == eSKILLRACE_HUMAN_BEAST || skillRace == eSKILLRACE_HUMAN_ELF )
								{
									if ( sqlStatement->DefalultSkillInsert( i, *begin, eRACE_HUMAN, jobIdx ) != SQL_SUCCESS )
										return false;
								}						
							}break;
						case eRACE_BEAST:
							{
								if( skillRace == eSKILLRACE_BEAST ||
									skillRace == eSKILLRACE_HUMAN_BEAST || skillRace == eSKILLRACE_BEAST_ELF )
								{
									if ( sqlStatement->DefalultSkillInsert( i, *begin, eRACE_BEAST, jobIdx ) != SQL_SUCCESS )
										return false;
								}
							}break;
						case eRACE_ELF:
							{
								if( skillRace == eSKILLRACE_ELF ||
									skillRace == eSKILLRACE_HUMAN_ELF || skillRace == eSKILLRACE_BEAST_ELF )
								{
									if ( sqlStatement->DefalultSkillInsert( i, *begin, eRACE_ELF, jobIdx ) != SQL_SUCCESS )
										return false;
								}
							}break;
						}
					}
				}
				else
				{
					for( unsigned long j = 0 ; j < pJobAry->GetSize() ; ++j )
					{
						unsigned long jobIdx = (*pJobAry)[j];

						for( char playerRace = eRACE_HUMAN ; playerRace < eRACE_MAX ; ++playerRace )
						{
							if( skillRace == eSKILLRACE_ALL )
							{
								if ( sqlStatement->DefalultSkillInsert( i, *begin, playerRace, jobIdx ) != SQL_SUCCESS )
									return false;

								continue;
							}

							switch( playerRace )
							{
							case eRACE_HUMAN:
								{
									if( skillRace == eSKILLRACE_HUMAN ||
										skillRace == eSKILLRACE_HUMAN_BEAST || skillRace == eSKILLRACE_HUMAN_ELF )
									{
										if ( sqlStatement->DefalultSkillInsert( i, *begin, eRACE_HUMAN, jobIdx ) != SQL_SUCCESS )
											return false;
									}						
								}break;
							case eRACE_BEAST:
								{
									if( skillRace == eSKILLRACE_BEAST ||
										skillRace == eSKILLRACE_HUMAN_BEAST || skillRace == eSKILLRACE_BEAST_ELF )
									{
										if ( sqlStatement->DefalultSkillInsert( i, *begin, eRACE_BEAST, jobIdx ) != SQL_SUCCESS )
											return false;
									}
								}break;
							case eRACE_ELF:
								{
									if( skillRace == eSKILLRACE_ELF ||
										skillRace == eSKILLRACE_HUMAN_ELF || skillRace == eSKILLRACE_BEAST_ELF )
									{
										if ( sqlStatement->DefalultSkillInsert( i, *begin, eRACE_ELF, jobIdx ) != SQL_SUCCESS )
											return false;
									}
								}break;
							}
						}
					}
				}
			}

		}
	}

	return true;
}


bool cSQLGame::InsertJobTree( char* dsn, char* uid, char* pwd )
{
	cSQLEnvironment* sqlEnv        = new cSQLEnvironment( );
	cSQLConnection*  sqlConnection = new cSQLConnection( );
	cSQLGameStmt*    sqlStatement  = new cSQLGameStmt( );

	//ODBC, SQL 사용을 초기화.
	if ( sqlEnv->AllocEnv( ) == false )
		return false;

	//SQL_ATTR_ODBC_VERSION 를 SQL_OV_ODBC3 로 설정.
	if ( sqlEnv->SetEnvAttr( ) == false )
		return false;

	if ( sqlConnection->AllocDbc( sqlEnv ) == false )
		return false;

	if ( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
		return false;

	if ( sqlStatement->AllocStmt( sqlConnection ) == false )
		return false;

	if ( sqlStatement->TruncateJobTree( ) != SQL_SUCCESS )
		return false;

	typedef tArray<unsigned long> cAry;
	typedef tPointerHashMap<unsigned long, void*> cPHashMap;

	cPHashMap* pJobTree = mpStatusScript->GetJobTree();
	if( pJobTree == NULL )
		return false;

	cPHashMap::cIterator b = pJobTree->Begin();
	cPHashMap::cIterator e = pJobTree->End();
	for( ; b != e ; ++b )
	{
		unsigned long jobIdx = (*b).mFirst;
		cAry* pParentAry = (cAry*)(*b).mSecond;
		if( pParentAry != NULL )
		{
			for( unsigned long i = 0 ; i < pParentAry->GetSize() ; ++i )
				sqlStatement->JobTreeInsert( jobIdx, (*pParentAry)[i] );
		}
	}	

	return true;
}


bool cSQLGame::InsertQuestData( char* dsn, char* uid, char* pwd )
{
	cSQLEnvironment* sqlEnv = new cSQLEnvironment();
	cSQLConnection* sqlConnection = new cSQLConnection();
	cSQLGameStmt* sqlGameStmt = new cSQLGameStmt();

	if( sqlEnv->AllocEnv() == false )
		return false;

	if( sqlEnv->SetEnvAttr() == false )
		return false;

	if( sqlConnection->AllocDbc( sqlEnv ) == false )
		return false;

	if( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
		return false;

	if( sqlGameStmt->AllocStmt( sqlConnection ) == false )
		return false;

	/// 각 퀘스트 테이블 초기화 
	if( sqlGameStmt->TruncateQuestDefine() != SQL_SUCCESS )
		return false;

	/// 로딩한 데이터를 DB 에 넣기.
	typedef tPointerHashMap<unsigned long, void*> cQuestDefineMap;
	cQuestDefineMap::cIterator i = mpQuestManager->GetQuestDefineMap()->Begin();
	cQuestDefineMap::cIterator end = mpQuestManager->GetQuestDefineMap()->End();

	for( ; i != end; ++i )
	{
		cQuestDefine* define = (cQuestDefine*)(*i).mSecond;
		if( !define )
			return false;
		
		/// 각 테이블간 연결 인덱스 정의
		if( sqlGameStmt->InsertQuestDefine( define ) != SQL_SUCCESS )
			return false;
	}
	return true;
}

bool cSQLGame::InsertItemDropLimitData( char* dsn, char* uid, char* pwd )
{
	cSQLEnvironment* sqlEnv = new cSQLEnvironment();
	cSQLConnection* sqlConnection = new cSQLConnection();
	cSQLGameStmt* sqlGameStmt = new cSQLGameStmt();

	if( sqlEnv->AllocEnv() == false )
		return false;

	if( sqlEnv->SetEnvAttr() == false )
		return false;

	if( sqlConnection->AllocDbc( sqlEnv ) == false )
		return false;

	if( sqlConnection->Connect( (SQLCHAR*)dsn, (SQLCHAR*)uid, (SQLCHAR*)pwd ) == false )
		return false;

	if( sqlGameStmt->AllocStmt( sqlConnection ) == false )
		return false;

	/// 전체를 사용 안함으로 바꾸고 밑에서 있는것들을 사용함으로 바꾼다
	if( sqlGameStmt->ClearItemDropLimit() != SQL_SUCCESS )
		return false;

	typedef tHashMap<unsigned long, unsigned long> cDropLimitMap;

	cDropLimitMap* pDropLimit = (cDropLimitMap*)mpDropScript->GetItemDropLimit();
	if( pDropLimit == NULL )
		return false;
	
	/// 로딩한 데이터를 DB 에 넣기.
	cDropLimitMap::cIterator i = pDropLimit->Begin();
	cDropLimitMap::cIterator end = pDropLimit->End();
	for( ; i != end; ++i )
	{
		unsigned long itemIdx = (*i).mFirst;
		unsigned long timeLimit = (*i).mSecond;

		/// 변경사항 없는 데이터 사용플래그 킴 외 신규 데이터 추가, 변경 사항있는 데이터 날짜 초기화
		if( sqlGameStmt->InsertItemDropLimit( itemIdx, timeLimit ) != SQL_SUCCESS )
			return false;
	}
	return true;
}

// Initialize Method
bool cSQLGame::Initialize(char* dsn, char* uid, char* pwd, unsigned int numWorkerThreads, unsigned int bufferLength)
{
	mIoContextPool = new cIoContextPool( bufferLength );
	return cSQLPool::Initialize( dsn, uid, pwd, numWorkerThreads );
}

// Shutdown Method
void cSQLGame::Shutdown(void)
{
	// SQL 종료.
	PostServerEvent( "cSQLGame - Have received a request from the manager to stop the game server is shutting down." );

	// 메모리 사용량
	SIZE_T quotaPagedPoolUsage;
	SIZE_T quotaNonePagedPoolUsage;
	SIZE_T workingSetSize;

	mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
	PostServerEvent( "cSQLGame - IoContextPool Working Set Size (=%d), Quota Paged Pool Usage (=%d), Quota None Paged Pool Usage (=%d)."
					,workingSetSize
					,quotaPagedPoolUsage
					,quotaNonePagedPoolUsage );

	cSQLPool::Shutdown( );
	SafeDelete( mIoContextPool );

	Sleep( 100 );
	PostServerEvent( "cSQLGame - Shutdown has completed." );
}

// AllocSQLConnection Method
PerSQLConnection* cSQLGame::AllocSQLConnection(void)
{
	PerSQLConnection* perSQLConnection = (PerSQLConnection*)GlobalAlloc( GPTR, sizeof(PerSQLConnection) );

	if ( perSQLConnection != NULL )
	{
		// PerSQLConnection - 초기화
		perSQLConnection->sqlConnection = new cSQLConnection( );  // 1. SQLConnection 생성.
		perSQLConnection->sqlStatement  = new cSQLGameStmt( );    // 2. cSQLGameStmt   생성.
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
bool cSQLGame::PostServerEvent(LPCTSTR format, ...)
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
void cSQLGame::GetIoContextPool(SIZE_T& quotaPagedPoolUsage, SIZE_T& quotaNonPagedPoolUsage, SIZE_T& workingSetSize)
{
	mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );
}

// Get Method
PerIoContext* cSQLGame::Get( )
{
	return mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
}

// Release Method
void cSQLGame::Release(PerIoContext* perIoContext)
{
	DWORD error = 0;
	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cSQLGame::Release:Error(=0x%08xh)", error );
}

// WorkerThread Method
DWORD cSQLGame::WorkerThread(void)
{
	PerSQLConnection* perSQLConnection = NULL;
	cSQLGameStmt*     sqlGameStmt      = NULL;
	SQLRETURN         sqlReturn        = 0;

	PerIoContext*     perIoContext     = NULL;
	DWORD             delayTick;
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

		// SQLStatement를 cSQLGameStmt로 캐스팅.
		sqlGameStmt = (cSQLGameStmt*)perSQLConnection->sqlStatement;

		// OVERLAPPED를 PerIoContext로 케스팅.
		perIoContext = (PerIoContext*)overlapped;
		perIoContext->lParam = GetTickCount( );

		// 1. SQL를 실행
		switch ( perIoContext->iParam )
		{
		/*-- Init & Shutdown Game Database --*/
		case SQL_REQUEST_GAME_INIT:
			sqlReturn = sqlGameStmt->Init( (INIT_GAME*)perIoContext->buffer );
			break;
		case SQL_REQUEST_GAME_SHUTDOWN:
			sqlReturn = sqlGameStmt->Shutdown( (SHUTDOWN_GAME*)perIoContext->buffer );
			break;

		/*-- Theme Database --*/
		case SQL_GAME_PROCESS_THEME_CREATE:
			sqlReturn = sqlGameStmt->ThemeCreate( (THEME_CREATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_ROOM_CHECK:
			sqlReturn = sqlGameStmt->ThemeRoomCheck( (THEME_ROOM_CHECK*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_USER_CHECK:
			sqlReturn = sqlGameStmt->ThemeUserCheck( (THEME_USER_CHECK*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_USER_INSERT:
			sqlReturn = sqlGameStmt->ThemeUserInsert( (THEME_USER_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_USER_SELECT:
			sqlReturn = sqlGameStmt->ThemeUserSelect( (THEME_USER_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_THEME_USER_UPDATE:
			sqlReturn = sqlGameStmt->ThemeUserUpdate( (THEME_USER_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_USER_REMOVE:
			sqlReturn = sqlGameStmt->ThemeUserRemove( (THEME_USER_REMOVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_USER_SAVE:
			sqlReturn = sqlGameStmt->ThemeUserSave( (THEME_USER_SAVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_THEME_PARTY_SELECT:
			sqlReturn = sqlGameStmt->ThemePartySelect( (THEME_PARTYINFO_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_THEME_PARTYROOM_SELECT:
			sqlReturn = sqlGameStmt->ThemePartyRoomSelect( (THEME_PARTYROOM_SELECT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GM_THEME_USER_SELECT:
			sqlReturn = sqlGameStmt->ThemeGMUserSelect( (THEME_GM_USER_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;

		/*-- Item Database --*/
		case SQL_GAME_PROCESS_VERIFY_ITEM_DEFINE:
			sqlReturn = sqlGameStmt->VerifyItemDefine( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_ABILITY:
			sqlReturn = sqlGameStmt->VerifyItemAbility( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_COOLTIME2:
			sqlReturn = sqlGameStmt->VerifyItemCooltime2( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_LIMIT:
			sqlReturn = sqlGameStmt->VerifyItemLimit( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_CARD:
			sqlReturn = sqlGameStmt->VerifyItemCard( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_CARD_SLOT:
			sqlReturn = sqlGameStmt->VerifyItemCardSlot( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_TAROT:
			sqlReturn = sqlGameStmt->VerifyItemTarot( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCED:
			sqlReturn = sqlGameStmt->VerifyItemEnhanced( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCEDRATE:
			sqlReturn = sqlGameStmt->VerifyItemEnhancedRate( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_DISJOINT:
			sqlReturn = sqlGameStmt->VerifyItemDisjoint( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_CHANGE:
			sqlReturn = sqlGameStmt->VerifyItemChange( (VERIFY_ITEM_TABLE*)perIoContext->buffer );
			break;

		case SQL_GAME_PROCESS_ITEM_DEFINE:
			sqlReturn = sqlGameStmt->ItemDefine( (ITEM_DEFINE_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_ABILITY:
			sqlReturn = sqlGameStmt->ItemAbility( (ITEM_ABILITY_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_COOLTIME2:
			sqlReturn = sqlGameStmt->ItemCooltime2( (ITEM_COOLTIME2_TABLE*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_LIMIT:
			sqlReturn = sqlGameStmt->ItemLimit( (ITEM_LIMIT_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_CARD:
			sqlReturn = sqlGameStmt->ItemCard( (ITEM_CARD_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_CARD_SLOT:
			sqlReturn = sqlGameStmt->ItemCardSlot( (ITEM_CARD_SLOT_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_TAROT:
			sqlReturn = sqlGameStmt->ItemTarot( (ITEM_TAROT_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_ENHANCED:
			sqlReturn = sqlGameStmt->ItemEnhanced( (ITEM_ENHANCED_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_ENHANCEDRATE:
			sqlReturn = sqlGameStmt->ItemEnhancedRate( (ITEM_ENHANCED_RATE_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_DISJOINT:
			sqlReturn = sqlGameStmt->ItemDisjoint( (ITEM_DISJOINT_TABLE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_CHANGE:
			sqlReturn = sqlGameStmt->ItemChange( (ITEM_CHANGE_TABLE*)perIoContext->buffer );
			break;

		case SQL_GAME_PROCESS_VERIFY_DEFAULT_ITEMS:
			sqlReturn = sqlGameStmt->VerifyDefaultItems( (VERIFY_DEFAULT_ITEMS*)perIoContext->buffer );
			break;

		case SQL_GAME_PROCESS_VERIFY_GUILD:
			sqlReturn = sqlGameStmt->VerifyGuild( (VERIFY_GUILD*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILD_SELECT:
			sqlReturn = sqlGameStmt->GuildSelect( (GUILD_SELECT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILDUSER_SELECT:
			sqlReturn = sqlGameStmt->GuildUserSelect( (GUILDUSER_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;

		/*-- TB_CHARACTER Table --*/
		case SQL_GAME_PROCESS_CHARACTER_LIST:
			sqlReturn = sqlGameStmt->CharacterList( (CHARACTER_LIST*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_CHARACTER_NAME:
			sqlReturn = sqlGameStmt->CharacterName( (CHARACTER_NAME*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_INSERT:
			sqlReturn = sqlGameStmt->CharacterInsert( (CHARACTER_INSERT*)perIoContext->buffer, perIoContext->offset );
			break;
        case SQL_GAME_PROCESS_CHARACTER_DELETE_WAIT:
			sqlReturn = sqlGameStmt->CharacterDeleteWait( (CHARACTER_DELETE_WAIT*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_CHARACTER_DELETE_CANCEL:
			sqlReturn = sqlGameStmt->CharacterDeleteCancel( (CHARACTER_DELETE_CANCEL*)perIoContext->buffer );
            break;
		case SQL_GAME_PROCESS_CHARACTER_DELETE:
			sqlReturn = sqlGameStmt->CharacterDelete( (CHARACTER_DELETE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_UPDATE:
			sqlReturn = sqlGameStmt->CharacterUpdate( (CHARACTER_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_SELECT:
			sqlReturn = sqlGameStmt->CharacterSelect( (CHARACTER_SELECT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_MONEY:
			sqlReturn = sqlGameStmt->CharacterMoney( (CHARACTER_MONEY*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_DEPOSIT:
			sqlReturn = sqlGameStmt->CharacterDeposit( (CHARACTER_DEPOSIT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_TAROT_POINT:
			sqlReturn = sqlGameStmt->CharacterTarotPoint( (CHARACTER_TAROT_POINT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_ACTIVE_WEPON:
			sqlReturn = sqlGameStmt->CharacterActiveWeapon( (CHARACTER_ACTIVE_WEAPON*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_OPTION:
			sqlReturn = sqlGameStmt->CharacterOption( (CHARACTER_OPTION*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_FORCETYPE:
			sqlReturn = sqlGameStmt->CharacterForce( (CHARACTER_FORCE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_SKILLRESET:
			sqlReturn = sqlGameStmt->CharacterSkillReset( (CHARACTER_SKILL_RESET*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT:
			sqlReturn = sqlGameStmt->CharacterBlockSelect( (CHARACTER_BLOCK_SELECT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT_LOG:
			sqlReturn = sqlGameStmt->CharacterBlockSelectList( (CHARACTER_BLOCK_SELECT_LIST*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_CHARACTER_BLOCK_INSERT:
			sqlReturn = sqlGameStmt->CharacterBlockInsert( (CHARACTER_BLOCK_INSERT*)perIoContext->buffer );
			break;	
		case SQL_GAME_PROCESS_CHARACTER_GM_NAME:
			sqlReturn = sqlGameStmt->CharacterGmName( (CHARACTER_GM_NAME*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_INVEN_SIZE:
			sqlReturn = sqlGameStmt->CharacterInvenSize( (CHARACTER_INVEN_SIZE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_TTL:
			sqlReturn = sqlGameStmt->CharacterTTL( (CHARACTER_TTL*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_THEMERESET:
			sqlReturn = sqlGameStmt->CharacterThemeReset( (CHARACTER_THEME_RESET*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_CHARACTER_CHANNEL:
			sqlReturn = sqlGameStmt->CharacterChannelSelect( (CHARACTER_CHANNEL*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_CHARACTER_GM_THEMERESET:
			sqlReturn = sqlGameStmt->CharacterGMThemeReset( (CHARACTER_GM_THEME_RESET*)perIoContext->buffer, perIoContext->offset );
			break;

		case SQL_GAME_PROCESS_CHARACTER_KICK_SELECT:
			sqlReturn = sqlGameStmt->CharacterGMKickSelect( (CHARACTER_KICK_SELECT*)perIoContext->buffer );
	        break;
		/*-- TB_INVENTORY Table
		*/
		case SQL_GAME_PROCESS_INVENTORY_INSERT:
			sqlReturn = sqlGameStmt->InventoryInsert( (INVENTORY_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_SELECT:
			sqlReturn = sqlGameStmt->InventorySelect( (INVENTORY_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
        case SQL_GAME_PROCESS_INVENTORY_CASH_SELECT:
			sqlReturn = sqlGameStmt->InventoryCashSelect( (INVENTORY_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_INVENTORY_DELETE:
			sqlReturn = sqlGameStmt->InventoryDelete( (INVENTORY_DELETE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_COOLTIME_SELECT:
			sqlReturn = sqlGameStmt->InventoryCooltimeSelect( (INVENTORY_COOLTIME_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_INVENTORY_COOLTIME_INSERT:
			sqlReturn = sqlGameStmt->InventoryCooltimeInsert( (INVENTORY_COOLTIME_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_REMOVE:
			sqlReturn = sqlGameStmt->InventoryRemove( (INVENTORY_REMOVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_USE:
			sqlReturn = sqlGameStmt->InventoryUse( (INVENTORY_USE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_SWAP:
			sqlReturn = sqlGameStmt->InventorySwap( (INVENTORY_SWAP*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_MOVE:
			sqlReturn = sqlGameStmt->InventoryMove( (INVENTORY_MOVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_EXCEPT:
			sqlReturn = sqlGameStmt->InventoryExcept( (INVENTORY_EXCEPT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_MERGE:
			sqlReturn = sqlGameStmt->InventoryMerge( (INVENTORY_MERGE*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_INVENTORY_DIVIDE:
			sqlReturn = sqlGameStmt->InventoryDivide( (INVENTORY_DIVIDE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_MOVE2:
			sqlReturn = sqlGameStmt->InventoryMove2( (INVENTORY_MOVE2*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_ENHANCED:
			sqlReturn = sqlGameStmt->InventoryEnhanced( (INVENTORY_ENHANCED*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_DISJOINT:
			sqlReturn = sqlGameStmt->InventoryDisjoint( (INVENTORY_DISJOINT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_PUT_CARD:
			sqlReturn = sqlGameStmt->InventoryPutCard( (INVENTORY_PUT_CARD*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_CHANGE:
			sqlReturn = sqlGameStmt->InventoryChange( (INVENTORY_CHANGE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_INVENTORY_SEAL:
			sqlReturn = sqlGameStmt->InventorySeal( (INVENTORY_SEAL*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_BILL_SELECT:
			sqlReturn = sqlGameStmt->ItemBillSelect( (ITEM_BILL_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_BILL_REMOVE:
			sqlReturn = sqlGameStmt->ItemBillRemove( (ITEM_BILL_REMOVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_BILL_UPDATE:
			sqlReturn = sqlGameStmt->ItemBillUpdate( (ITEM_BILL_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_SELL:
			sqlReturn = sqlGameStmt->ItemSell( (ITEM_SELL*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_BUY:
			sqlReturn = sqlGameStmt->ItemBuy( (ITEM_BUY*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_COLLECT:
			sqlReturn = sqlGameStmt->ItemCollect( (ITEM_COLLECT*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_ITEM_COLLECT_CHANGE:
			sqlReturn = sqlGameStmt->ItemCollectChange( (ITEM_COLLECT_CHANGE*)perIoContext->buffer );
            break;
		case SQL_GAME_PROCESS_ITEM_GET:
		case SQL_GAME_PROCESS_ITEM_GET_QUEST:
			sqlReturn = sqlGameStmt->ItemGet( (ITEM_GET*)perIoContext->buffer );
			break;
        case SQL_GAME_PROCESS_ITEM_GET_PARTY_GIVE:
			sqlReturn = sqlGameStmt->ItemPartyGive( (ITEM_PARTY_GIVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_USE:
			sqlReturn = sqlGameStmt->ItemUse( (ITEM_USE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_EXCHANGE:
			sqlReturn = sqlGameStmt->ItemExchange( (ITEM_EXCHANGE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_MIX:
			sqlReturn = sqlGameStmt->ItemMix( (ITEM_MIX*)perIoContext->buffer );
			break;
		// 아이템 판매대행
		case SQL_GAME_PROCESS_ITEM_AGENT_CHECK:
			sqlReturn = sqlGameStmt->ItemAgentCheck( (ITEM_AGENT_CHECK*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_OWNER:
			sqlReturn = sqlGameStmt->ItemAgentSelectOwner( (ITEM_AGENT_SELECT_OWNER*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_INSERT:
			sqlReturn = sqlGameStmt->ItemAgentInsert( (ITEM_AGENT_INSERT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_DELETE:
			sqlReturn = sqlGameStmt->ItemAgentDelete( (ITEM_AGENT_DELETE*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_UPDATE:
			sqlReturn = sqlGameStmt->ItemAgentUpdate( (ITEM_AGENT_UPDATE*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT:
			sqlReturn = sqlGameStmt->ItemAgentSelect( (ITEM_AGENT_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_NAME:
			sqlReturn = sqlGameStmt->ItemAgentSelectName( (ITEM_AGENT_SELECT_NAME*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_TYPE:
			sqlReturn = sqlGameStmt->ItemAgentSelectType( (ITEM_AGENT_SELECT_TYPE*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_ALL:
			sqlReturn = sqlGameStmt->ItemAgentSelectAll( (ITEM_AGENT_SELECT_ALL*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_OBT_EVENT:
			sqlReturn = sqlGameStmt->ItemObtEvent( (ITEM_OBT_EVENT*)perIoContext->buffer, perIoContext->offset );

        // 소환 주문서
        case SQL_GAME_PROCESS_ITEM_SUMMON_POST:
			sqlReturn = sqlGameStmt->ItemSummonPost( (ITEM_SUMMON_POST*)perIoContext->buffer, perIoContext->offset );
            break;
        case SQL_GAME_PROCESS_ITEM_SUMMON_AGENT:
            sqlReturn = sqlGameStmt->ItemSummonAgent( (ITEM_SUMMON_AGENT*)perIoContext->buffer, perIoContext->offset );
            break;
        case SQL_GAME_PROCESS_ITEM_SUMMON_WAREHOUSE:
			sqlReturn = sqlGameStmt->ItemSummonWareHouse( (ITEM_SUMMON_WAREHOUSE*)perIoContext->buffer );
			break;

		// NPC이벤트 아이템 지급
		case SQL_GAME_PROCESS_ITEM_NPC_EVENT:
			sqlReturn = sqlGameStmt->ItemNpcEvent( (ITEM_NPC_EVENT*)perIoContext->buffer, perIoContext->offset );
			break;

		// TODO - 우편 신규 작업.
		/*-- 우편 시스템 --*/
		case SQL_GAME_PROCESS_POST_CHECK:
			sqlReturn = sqlGameStmt->PostCheck( (POST_CHECK*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_POST_SELECT:
			sqlReturn = sqlGameStmt->PostSelect( (POST_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;		
		case SQL_GAME_PROCESS_POST_INSERT:
			sqlReturn = sqlGameStmt->PostInsert( (POST_INSERT*)perIoContext->buffer );
			break;		
		case SQL_GAME_PROCESS_POST_UPDATE_STATUS:
			sqlReturn = sqlGameStmt->PostUpdateStatus( (POST_UPDATE_STATUS*)perIoContext->buffer );
			break;		
		case SQL_GAME_PROCESS_POST_UPDATE:
			sqlReturn = sqlGameStmt->PostUpdate( (POST_UPDATE*)perIoContext->buffer, perIoContext->offset );
			break;		
		case SQL_GAME_PROCESS_POST_DELETE:
			sqlReturn = sqlGameStmt->PostDelete( (POST_DELETE*)perIoContext->buffer, perIoContext->offset );
			break;		

		/*-- TB_FRIEND Table
		*/
		case SQL_GAME_PROCESS_FRIEND_SELECT:
			sqlReturn = sqlGameStmt->FriendSelect( (FRIEND_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_FRIEND_INSERT:
			sqlReturn = sqlGameStmt->FriendInsert( (FRIEND_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_FRIEND_UPDATE:
			sqlReturn = sqlGameStmt->FriendUpdate( (FRIEND_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_FRIEND_DELETE:
			sqlReturn = sqlGameStmt->FriendDelete( (FRIEND_DELETE*)perIoContext->buffer );
			break;

		/*-- TB_ITEM_SPREAD Table
		*/
		case SQL_GAME_PROCESS_ITEM_SPREAD_VALUE:
		case SQL_GAME_PROCESS_ITEM_TUTORIALSPREAD_VALUE:
        case SQL_GAME_PROCESS_ITEM_SELF_SPREAD_VALUE:
			sqlReturn = sqlGameStmt->ItemSpreadValue( (ITEM_SPREAD_VALUE*)perIoContext->buffer );
			break;
		/*-- TB_ITEM_TAROT_RESULT Table
		*/
		case SQL_GAME_PROCESS_ITEM_TAROT_RESULT:
		case SQL_GAME_PROCESS_ITEM_TUTORIALTAROT_RESULT:
        case SQL_GAME_PROCESS_ITEM_SELF_TAROT_RESULT:
			sqlReturn = sqlGameStmt->ItemTarotResult( (ITEM_TAROT_RESULT*)perIoContext->buffer );
			break;

		/*-- TB_CHARACTER_SKILL Table
		*/
		case SQL_GAME_PROCESS_SKILL_SELECT:
			sqlReturn = sqlGameStmt->SkillSelect( (SKILL_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_SKILL_INSERT:
			sqlReturn = sqlGameStmt->SkillInsert( (SKILL_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_SKILL_UPDATE:
			sqlReturn = sqlGameStmt->SkillUpdate( (SKILL_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_SKILL_CHEAT_INSERT:
			sqlReturn = sqlGameStmt->SkillCheatInsert( (SKILL_CHEAT_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_SKILL_CHEAT_UPDATE:
			sqlReturn = sqlGameStmt->SkillCheatUpdate( (SKILL_CHEAT_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_SKILL_COOLTIME:
			sqlReturn = sqlGameStmt->SkillCoolTime( (SKILL_COOLTIME*)perIoContext->buffer );
			break;

		case SQL_GAME_PROCESS_INFLUENCE_SELECT:
			sqlReturn = sqlGameStmt->SkillInfluenceSelect( (SKILL_INFLUENCE_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_INFLUENCE_UPDATE:
			sqlReturn = sqlGameStmt->SkillInfluenceUpdate( (SKILL_INFLUENCE_UPDATE*)perIoContext->buffer );
			break;

		/*-- TB_STALL_SELL Table
		*/
		case SQL_GAME_PROCESS_STALL_SELL_GET:
			sqlReturn = sqlGameStmt->StallSellGet( (STALL_SELL_GET*)perIoContext->buffer );
			break;

		/*-- TB_SHORTCUT Table
		*/
		case SQL_GAME_PROCESS_SHORTCUT_SELECT:
			sqlReturn = sqlGameStmt->ShortcutSelect( (SHORTCUT_SELECT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_SHORTCUT_UPDATE:
			sqlReturn = sqlGameStmt->ShortcutUpdate( (SHORTCUT_UPDATE*)perIoContext->buffer );
			break;

		/*-- TB_QUEST_PROGRESS Table
		*/
		case SQL_GAME_PROCESS_QUEST_INSERT:
		case SQL_GAME_PROCESS_QUEST_INSERTBYITEM:
			sqlReturn = sqlGameStmt->QuestInsert( (QUEST_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_QUEST_SELECT:
			sqlReturn = sqlGameStmt->QuestSelect( (QUEST_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_QUEST_UPDATE:
			sqlReturn = sqlGameStmt->QuestUpdate( (QUEST_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_QUEST_DELETE:
		case SQL_GAME_PROCESS_QUEST_DELETEAUTO:
			sqlReturn = sqlGameStmt->QuestDelete( (QUEST_DELETE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_QUEST_COMPLETE:
			sqlReturn = sqlGameStmt->QuestComplete( (QUEST_COMPLETE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_QUEST_REWARD:
			sqlReturn = sqlGameStmt->QuestReward( (QUEST_REWARD*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_QUEST_ENDLISTSELECT:
			sqlReturn = sqlGameStmt->QuestEndListSelect( (QUEST_ENDLIST*)perIoContext->buffer, perIoContext->offset );
			break;		
		case SQL_GAME_PROCESS_QUEST_VALIDLISTSELECT:
			sqlReturn = sqlGameStmt->QuestValidListSelect( (QUEST_VALIDLIST*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_QUEST_REMOVE:
			sqlReturn = sqlGameStmt->QuestRemove( (QUEST_REMOVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_TITLE_INSERT:
			sqlReturn = sqlGameStmt->TitleInsert( (TITLE_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_TITLE_SELECT:
			sqlReturn = sqlGameStmt->TitleSelect( (TITLE_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_GUILD_CREATE:
			sqlReturn = sqlGameStmt->GuildCreate( (GUILD_CREATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILDUSER_ADD:
			sqlReturn = sqlGameStmt->GuildUserAdd( (GUILDUSER_ADD*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILDUSER_OUT:
			sqlReturn = sqlGameStmt->GuildUserOut( (GUILDUSER_OUT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILDUSER_CUT:
			sqlReturn = sqlGameStmt->GuildUserOut( (GUILDUSER_OUT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILD_DELETE:
			sqlReturn = sqlGameStmt->GuildDelete( (GUILD_DELETE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILD_UPDATE:
			sqlReturn = sqlGameStmt->GuildUpdate( (GUILD_UPDATE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION:
			sqlReturn = sqlGameStmt->GuildUserGivePosition( (GUILDUSER_GIVEPOSITION*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GUILDMARK_UPDATE:
			sqlReturn = sqlGameStmt->GuildMarkUpdate( (GUILDMARK_UPDATE*)perIoContext->buffer );
			break;

			/*-- TB_CHARACTER_MAKESKILL Table
			*/
		case SQL_GAME_PROCESS_MAKESKILL_INSERT:
			sqlReturn = sqlGameStmt->MakeSkillInsert( (MAKESKILL_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_MAKESKILL_SELECT:
			sqlReturn = sqlGameStmt->MakeSkillSelect( (MAKESKILL_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_MAKESKILL_DELETE:
			sqlReturn = sqlGameStmt->MakeSkillDelete( (MAKESKILL_DELETE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_RECIPE_INSERT:
			sqlReturn = sqlGameStmt->RecipeInsert( (RECIPE_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_RECIPE_GROUP_INSERT:
			sqlReturn = sqlGameStmt->RecipeGroupInsert( (RECIPE_GROUP_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_RECIPE_COOLTIME:
			sqlReturn = sqlGameStmt->RecipeCoolTime( (RECIPE_COOLTIME*)perIoContext->buffer );
			break;

		case SQL_GAME_PROCESS_FORTUNE_SELECT:
			sqlReturn = sqlGameStmt->FortuneSelect( (FORTUNE_SELECT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_FORTUNE_UPDATE:
			sqlReturn = sqlGameStmt->FortuneUpdate( (FORTUNE_UPDATE*)perIoContext->buffer );
			break;

		case SQL_GAME_PROCESS_FORCEPOINT_UPDATE:
			sqlReturn = sqlGameStmt->ForcePointUpdate( (FORCE_POINT_UPDATE*)perIoContext->buffer );
			break;

			/*-- TB_PVP Tables --*/
		case SQL_GAME_PROCESS_PVP_USER_INSERT:
			sqlReturn = sqlGameStmt->PVPUserInsert( (PVP_USER_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_PVP_USER_SELECT:
			sqlReturn = sqlGameStmt->PVPUserSelect( (PVP_USER_SELECT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_PVP_USER_REMOVE:
			sqlReturn = sqlGameStmt->PVPUserRemove( (PVP_USER_REMOVE*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_PVP_USER_CLEAR:
			sqlReturn = sqlGameStmt->PVPUserClear( (PVP_USER_CLEAR*)perIoContext->buffer );
			break;

			/*-- TB_PARTY & TB_PARTY_USER Tables --*/
		case SQL_GAME_PROCESS_PARTY_INSERT:
			sqlReturn = sqlGameStmt->PartyInsert( (PARTY_INSERT*)perIoContext->buffer );
			break;

			/*-- ETC --*/
		case SQL_GAME_PROCESS_WHISPER:
			sqlReturn = sqlGameStmt->WhisperSelect( (WHISPER_SELECT*)perIoContext->buffer );
			break;

			/*-- CHEAT --*/
		case SQL_GAME_PROCESS_JOB_CHEAT_UPDATE:
			sqlReturn = sqlGameStmt->GMJobUpdate( (GM_JOB_UPDATE*)perIoContext->buffer, perIoContext->offset );
			break;
			/*-- CHEAT :: TB_GM_EVENT --*/
		case SQL_GAME_PROCESS_VERIFY_GM_EVENT:
			sqlReturn = sqlGameStmt->GMEventSelect( (VERIFY_GM_EVENT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_GM_EVENT_SEARCH:
			sqlReturn = sqlGameStmt->GMEventSelect( (VERIFY_GM_EVENT*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_GM_EVENT_INSERT:
			sqlReturn = sqlGameStmt->GMEventInsert( (GM_EVENT_INSERT*)perIoContext->buffer );
			break;
		case SQL_GAME_PROCESS_GM_EVENT_UPDATE:
			sqlReturn = sqlGameStmt->GMEventUpdate( (GM_EVENT_UPDATE*)perIoContext->buffer );
			break;

			/*-- TB_ITEM_DROPLIMIT --*/
		case SQL_GAME_PROCESS_ITEM_DROPLIMIT_SELECT:
			sqlReturn = sqlGameStmt->ItemDropLimitSelect( (ITEM_DROPLIMIT_TABLE*)perIoContext->buffer, perIoContext->offset );
			break;
		case SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE:
			sqlReturn = sqlGameStmt->ItemDropLimitUpdate( (ITEM_DROPLIMIT_UPDATE*)perIoContext->buffer );
			break;

		/*-- Error
		*/
		default:
			sqlReturn = SQL_ERROR;
			break;
		}

		// Query 지연시간 기록.
		delayTick = GetTickCount( ) - (DWORD)perIoContext->lParam;
		if ( delayTick > MAX_QUERY_LOG )
			PostServerEvent( "QUERY - cSQLGame::iParam(=%d) Delay Time(=%d)", perIoContext->iParam, delayTick );

		// 2. 이벤트 - 오류만 기록.
		if ( !SQL_SUCCEEDED( sqlReturn ) )
		{
			if ( sqlReturn != SQL_NO_DATA )
			{
				SQLHSTMT    hstmt = sqlGameStmt->GetSqlStmt( );
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

				result = SQLGetDiagRec( SQL_HANDLE_STMT, hstmt, 1, sqlstate, &nativeErrorPtr, messageText, bufferLength, &textLength );

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
				PostServerEvent( "cSQLGame::IParam[%d]:Returns[%d] - SQL Error State string[%s] Native Error code[%d] SQL Error Text string[%s]",
								 perIoContext->iParam,
								 sqlReturn,
								 sqlstate,
								 nativeErrorPtr,
								 messageText );

				PostServerEvent( "cSQLGame::IParam[%d]:Returns[%d] - ODBCRowNumber[%d] SSrvrLine[%d] SSrvrMsgState[%d] SSrvrSeverity[%d] SSrvrProcname[%s] SSrvrSrvname[%s]",
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
				PostServerEvent( "cSQLGame::IParam[%d]:Returns[%d]", perIoContext->iParam, sqlReturn );
			}
		}

		// 3. PerIoContext 사용완료 - 결과는 콜백으로 되돌려준다.
		// perIoContext->requestType   = IOCP_REQUEST_CALLBACK;
		// perIoContext->requestResult = IOCP_REQUEST_SUCCESS;
		cGameProcess* gameProcess = g_gameSrv->GetGameProcess( );
		if ( gameProcess != NULL )
		{
			perIoContext->requestResult = SQL_SUCCEEDED( sqlReturn ) ? IOCP_REQUEST_SUCCESS : IOCP_REQUEST_ERROR;
			gameProcess->QueueRequest( completionKey, (OVERLAPPED*)perIoContext, perIoContext->offset );
		}

		// SQLConnection - 내보내기.
		ReleasePool( perSQLConnection );
	}
	return 0L;
}
