// Include
#include "gamesrv.h"
#include "packet.h"

#include "protocol.h"
#include "protocol_Cheat.h"
#include "commonnetworkmsgstruct.h"

#include "item_common.h"
#include "trade_common.h"
#include "tarot_common.h"
#include "party_common.h"
#include "chat_common.h"
#include "Cheat_Common.h"
#include "Guild_Common.h"
#include "MakeSkill_Common.h"
#include "Notice_Common.h"
#include "Tutorial_Common.h"

#include "player.h"
#include "npc.h"
#include "item.h"
#include "monster.h"
#include "Guild.h"
#include "Gathering.h"
#include "statuscalc_server.h"
#include "rangecheck.h"
#include "pointerhashmap.h"
#include "Duel_Common.h"
#include "PvPScript.h"
#include "NpcScript.h"
#include "ShortcutScript.h"
#include "Drop.h"
#include "PVPManager.h"
#include "DuelManager.h"
#include "DamageCalc.h"
#include "SkillScript.h"
#include "SkillManager.h"
#include "ObjectManager.h"
#include "AIManager.h"
#include "StageScript.h"
#include "ItemManager.h"
#include "TitleManager.h"
#include "GuildManager.h"
#include "GatheringScript.h"
#include "PartyManager.h"
#include "TriggerManager.h"
#include "QuestManager.h"
#include "GridManager.h"
#include "AppTimer.h"
#include "memorypool.h"
#include "CooltimePool.h"
#include "TakeDamagePool.h"
#include "ItemBillPool.h"
#include "ItemGetPool.h"
#include "ItemCountPool.h"
#include "ThemeUserPool.h"
#include "RandomTable.h"
#include "FileSystem.h"
#include "ThemeManager.h"
#include "Theme_Common.h"
#include "PartyUnionManager.h"
#include "ThemeObject.h"
#include "StatusScript.h"
#include "UserPortal_Common.h"
#include "TeleportPortal_Common.h"
#include "SummonPatternPool.h"
#include "SkillTargetPool.h"
#include "TwitterScript.h"

#include "ClientInfo.h"
//#include "HackShield_Common.h"
#include <io.h>

// Local definitions
#pragma warning( disable: 4127 )

#define MAX_LOGIN_UPDATE		90000
#define MAX_CHARACTER_TTL		90000
#define MAX_CASH_SELECT			10000

#define MAX_RELEASE_COUNT       10000
#define MAX_CHARACTER_SELECT	16
#define MAX_GAME_IN_DB			16
#define MAX_GAME_OUT_DB			16

/* macro for SQL Class checking */
#define SQL_ACCOUNT_CHECK(sqlAccount)       \
sqlAccount = g_gameSrv->GetSQLAccount( );   \
if ( sqlAccount == NULL )                   \
return false;

#define SQL_GAME_CHECK(sqlGame)             \
sqlGame = g_gameSrv->GetSQLGame( );         \
if ( sqlGame == NULL )                      \
return false;

/* macro for TCP Class checking */
#define LOGIN_CLIENT_CHECK(loginClient)     \
loginClient = g_gameSrv->GetLoginClient( ); \
if ( loginClient == NULL )                  \
return false;

#pragma pack(1) // 1Byte 정렬 설정

struct Game2Game : Packet
{
	unsigned long  ipv4;
	unsigned short port;
};

#pragma pack( ) // 1Byte 정렬 해제


// Global data
cGameProcess* cGameProcess::mGameProcess = NULL;


BYTE SY_RKEY_TABLE[16][4]=
{
	{ 0x0F,0xAD,0x37,0x6C },	//6
	{ 0x5C,0x55,0x6A,0xD8 },	//10
	{ 0x6C,0xA9,0xDB,0x64 },	//11
	{ 0x5A,0xBE,0xF2,0xF6 },	//12
	{ 0xB3,0xB7,0xEC,0x7B },	//8
	{ 0xE8,0x9B,0x3A,0x8A },	//5
	{ 0xD9,0x64,0x28,0x5C },	//7
	{ 0xA6,0x4D,0x8F,0xB9 },	//13
	{ 0x17,0xb3,0x67,0xaa },	//0
	{ 0x4B,0x1C,0x15,0x11 },	//4
	{ 0x9C,0x13,0xFE,0xF5 },	//9
	{ 0x53,0xf2,0x74,0xb1 },	//1
	{ 0x67,0x9F,0x7D,0xF6 },	//14
	{ 0x98,0xea,0x55,0x29 },	//3
	{ 0x8c,0x72,0x59,0x32 },	//2
	{ 0xCA,0x6A,0xA9,0xEF },	//15
};

BYTE SY_KEY_TABLE[16][4]=
{
	{ 0xC0,0x3C,0x06,0xEA },	//0
	{ 0x68,0xCC,0x56,0x3A },	//10
	{ 0xB9,0x78,0xB3,0x45 },	//1
	{ 0xA1,0x8A,0x0B,0x70 },	//12
	{ 0x3B,0xB1,0x60,0x0B },	//8
	{ 0x88,0x48,0xC0,0x71 },	//2
	{ 0x43,0xC8,0x53,0x63 },	//7
	{ 0x5C,0xF8,0x90,0x4A },	//11
	{ 0x54,0x1B,0x0F,0x9C },	//15
	{ 0x44,0x95,0xAF,0x99 },	//6
	{ 0x85,0x4C,0xB6,0xE7 },	//3
	{ 0x28,0x45,0x80,0xB1 },	//13
	{ 0x2D,0x83,0x06,0x55 },	//5
	{ 0x98,0xAE,0xD9,0xBF },	//4
	{ 0x71,0x9D,0x4A,0x80 },	//14
	{ 0x19,0x2D,0x57,0xC6 },	//9
};

time_t timestamp2time_t(TIMESTAMP_STRUCT& ts)
{
	struct tm when;

	when.tm_year = ts.year  - 1900;
	when.tm_mon  = ts.month - 1;
	when.tm_mday = ts.day;
	when.tm_hour = ts.hour;
	when.tm_min  = ts.minute;
	when.tm_sec  = ts.second;

	return mktime( &when );
}

time_t plusSecTime_t( long sec )
{
    time_t      ltime;
    struct tm   when;

    time( &ltime );
    when = *localtime( &ltime );
    when.tm_sec += (int)sec;

    return mktime( &when );
}

// cGameProcess Constructor.
cGameProcess::cGameProcess(DWORD serverNum, DWORD channelNum, TCHAR* inetnum/*, int ahnhsEnable, DWORD ahnhsCheckTime, DWORD ahnhsWaitTime*/)
	:mServerNum(serverNum)
	,mChannelNum(channelNum)
	,mServerInetnum(inetnum)
//	,mHackShieldEnable(ahnhsEnable)
//	,mHackShieldCheckTime(ahnhsCheckTime)
//	,mHackShieldWaitTime(ahnhsWaitTime)
{
	mGameProcess         = this;

	mSystemManagerData   = 0;
	mRequestImportData   = 0;
	mCompleteImportData  = 0;

	mRequestData         = 0;
	mCompleteData        = 0;

	for( unsigned long i = 0 ; i < PVP_DM_LEVEL_SECTION_MAX ; ++i )
	{
		mPVPData[i].cid         = 0;
		mPVPData[i].status      = _E_PVP_CLOSED_;
		mPVPData[i].timer       = 0;
		mPVPData[i].pvpType     = MAP_NONE;
		mPVPData[i].pvpEndTime  = 0;
	}

	for( int i = 0 ; i < MAX_THEMEDATA_MAX ; ++i )
	{
		mThemeData[i].cid       = 0;
		mThemeData[i].status    = _E_THEME_CLOSED_;

		mThemeData[i].themeIdx  = 0;
		mThemeData[i].fixTime   = 0;
		mThemeData[i].varTime   = 0;
		mThemeData[i].maxRoom   = 0;
		mThemeData[i].lastDate  = 0;
		mThemeData[i].validThru = 0;
	}
	
	mLastCategory        = 0;
	mLastProtocol        = 0;
	mLastPacketSize      = 0;

	mFrame               = 0;
	mFpsTick             = 0;
	mThreadPriority      = false;
	mThreadPriorityTick  = 0;

	mElapsedTick         = 0;
	mMinElapsedTick      = 0;
	mMaxElapsedTick      = 0;

	// 채널 정보 초기화
	mType                = _E_ST_NORMAL_MAP_;
	mStatus              = _E_STATUS_CLOSED_;

	// 채널 리스트 초기화
	ZeroMemory( mChannelList, sizeof(mChannelList) );

	mTTLTick             = 0;
	mStatusSyncTick      = 0;

	// MapServer 멤버 변수 초기화
	mpFileSystem         = NULL;
	mpAppTimer           = NULL;
	mpMemoryPool         = NULL;
	mpRandomTable        = NULL;
	mpSummonPatternPool  = NULL;
	mpSkillTargetPool    = NULL;
	mpGridManager        = NULL;
	mpObjectManager      = NULL;
	mpAIManager          = NULL;
	mpDrop               = NULL;
	mpSkillManager       = NULL;
	mpStageScript		 = NULL;
	mpItemManager        = NULL;
	mpCooltimePool       = NULL;
	mpTakeDamagePool	 = NULL;
	mpItemBillPool       = NULL;
	mpItemGetPool        = NULL;
	mpItemCountPool      = NULL;
	mpPvPObjectPool      = NULL;
	mpPvPPlayerListPool  = NULL;
	mpThemeUserPool      = NULL;
	mpPartyManager		 = NULL;
	mpTriggerManager	 = NULL;
	mpQuestManager		 = NULL;
	mpDuelManager        = NULL;
	mpPVPManager         = NULL;
	mpThemeManager       = NULL;
	mpTitleManager		 = NULL;
	mpGuildManager		 = NULL;
	mpPartyUnionManager	 = NULL;
	mpTwitterScript      = NULL;

	mAccumTime           = 0;

//	mHackShieldPause     = false;

//	if ( mHackShieldEnable != 0 )
//	{
		// --------------------------------------------------------------------------
		// [_AhnHS_CreateServerObject]
		// --------------------------------------------------------------------------
		// - DESCRIPTION:
		//   :HSBGen.exe를 통해 생성한 .hsb 파일을 로드하여 서버 핸들(Server Handle)을 
		//    생성합니다. 보통 하나의 게임을 서비스하는 서버 프로세스에서 서버 핸들은 
		//	  한 개를 생성하여 게임 서버 프로세스가 종료할 때까지 유지합니다.
		//
		// - SYNTAX:
		//    AHNHS_SERVER_HANDLE __stdcall _AhnHS_CreateServerObject (IN const char *pszFilePath);
		//
		// - PARAMETERS:
		//    .pszFilePath	:const char *	HackShield Briefcase (.hsb) 파일 전체 경로
		//
		// - RETURN VALUE:
		//   서버 핸들을 올바르게 생성하지 못했을 경우 NULL(ANTICPX_INVALID_HANDLE_VALUE) 값을 리턴합니다. 
		//   보통 HackShield Briefcase (.hsb) 파일 경로가 올바르지 않을 경우나 시스템 리소스(메모리)가 
		//   부족할 경우에 발생합니다.
//		mHackShieldHandle = _AhnHS_CreateServerObject ( ".\\HackShield\\irisclient.hsb" ); // .hsb 파일 경로
//	}
//	else
//		mHackShieldHandle = NULL;

    mCaptchaOffset = 0;
    mCaptchaMax    = 0;

	mReleaseCountTick     = 0;
	mCharacterSelectCount = 0;
	mGameInDbCount        = 0;
	mGameOutDbCount       = 0;
}

// ~cGameProcess Destructor.
cGameProcess::~cGameProcess(void)
{
	mGameProcess = NULL;
}

// Initialize Method - 순서에 주의!
bool cGameProcess::Initialize(char* ipAddr, unsigned short port, unsigned short numWorkerThreads, u_long bufferLength)
{
	// HackShield 초기화 검사
//	if ( mHackShieldEnable != 0 && mHackShieldHandle == ANTICPX_INVALID_HANDLE_VALUE )
//		return false;

 	mInDunMapNumber = MAP_NONE;
	if( MAP_PVP_MIN <= mChannelNum && mChannelNum <= MAP_PVP_MAX )
	{
		mType = _E_ST_ID_PVP_;
		mInDunMapNumber = (unsigned short)mChannelNum;
	}
	else if( MAP_THEME_MIN <= mChannelNum && mChannelNum <= MAP_THEME_MAX )
	{
		mType = _E_ST_ID_THEME_;
		mInDunMapNumber = (unsigned short)mChannelNum;
	}
	else if( mChannelNum == MAP_TUTORIAL )
	{
		mType = _E_ST_ID_TUTORIAL_;
		mInDunMapNumber = (unsigned short)mChannelNum;
	}

	// cFileSystem 클래스 생성
	mpFileSystem = new cFileSystem( );

	// cAppTimer 클래스 생성
	mpAppTimer = new cAppTimer( );

	// cMemoryPool 클래스 생성
	mpMemoryPool = new cMemoryPool( );

	// cRandomTable 클래스 생성 및 초기화
	mpRandomTable = new cRandomTable( );
	if( !mpRandomTable->Initialize( ) )
		return false;

	mpSummonPatternPool = new cSummonPatternPool();
	if ( mpSummonPatternPool == NULL )
		return false;

	mpSkillTargetPool = new cSkillTargetPool();
	if ( mpSkillTargetPool == NULL )
		return false;

	// cStageScript 클래스 생성 및 초기화
	mpStageScript = new cStageScript( );
	if( !mpStageScript->Init( ) )
		return false;

	// cStageScript 클래스 생성 및 초기화
	mpTwitterScript = new cTwitterScript( );
	if( !mpTwitterScript->Init( ) )
		return false;

	// cGridManager 클래스 생성 및 초기화
	mpGridManager = new cGridManager( );
	switch ( mType )
	{
	case _E_ST_NORMAL_MAP_:
		if( !mpGridManager->Init( _E_GMT_3X3_, mType ) ) return false;
		break;
	case _E_ST_ID_PVP_:
		if( !mpGridManager->Init( _E_GMT_5X5_, mType ) ) return false;
		break;
	case _E_ST_ID_THEME_:
		if( !mpGridManager->Init( _E_GMT_5X5_, mType ) ) return false;
		break;
	case _E_ST_ID_TUTORIAL_:
		if( !mpGridManager->Init( _E_GMT_5X5_, mType ) ) return false;
		break;
	default:
		return false;
	}

	// cObjectManager 클래스 생성 및 초기화
	mpObjectManager = new cObjectManager( );
	if( !mpObjectManager->Init( mType ) )
		return false;		

	// cSkillManager 클래스 생성 및 초기화
	mpSkillManager = new cSkillManager( );
	if( !mpSkillManager->Init( ) )
		return false;

	// cAIManager 클래스 생성 및 초기화
	mpAIManager = new cAIManager( );
	if( !mpAIManager->Init( mType ) )
		return false;

	// cDrop 클래스 생성 및 초기화
	mpDrop = new cDrop( );
	if( !mpDrop->Init() )
		return false;

	// cItemManager 클래스 생성 및 초기화
	mpItemManager = new cItemManager( );
	if ( !mpItemManager->Init( ) )
		return false;

	mpCooltimePool = new cCooltimePool( );
	if ( mpCooltimePool == NULL )
		return false;

	mpTakeDamagePool = new cTakeDamagePool( );
	if ( mpTakeDamagePool == NULL )
		return false;

	mpItemBillPool = new cItemBillPool( );
	if ( mpItemBillPool == NULL )
		return false;

	mpItemGetPool = new cItemGetPool( );
	if ( mpItemGetPool == NULL )
		return false;

	mpItemCountPool = new cItemCountPool( );
	if ( mpItemCountPool == NULL )
		return false;

	mpPvPObjectPool = new cPvPObjectPool( );
	if ( mpPvPObjectPool == NULL )
		return false;
	
	mpPvPPlayerListPool = new cPvPPlayerListPool();
	if ( mpPvPPlayerListPool == NULL )
		return false;

	mpThemeUserPool = new cThemeUserPool( );
	if ( mpThemeUserPool == NULL )
		return false;

	// cPartyManager 클래스 생성 및 초기화
	mpPartyManager = new cPartyManager( );
	if( !mpPartyManager->Init() )
		return false;

	// cTriggerManager 클래스 생성 및 초기화
	mpTriggerManager = new cTriggerManager( );
	if( !mpTriggerManager->Init() )
		return false;

	// cQuestManager 클래스 생성 및 초기화
	mpQuestManager = new cQuestManager( );
	if( !mpQuestManager->Init() )
		return false;

	// cDuelManager 클래스 생성 및 초기화
	mpDuelManager = new cDuelManager( );
	if( !mpDuelManager->Init() )
		return false;

	// cPvPManager 클래스 생성 및 초기화
	mpPVPManager = new cPVPManager( );
	if( !mpPVPManager->Init( mType, mInDunMapNumber ) )
		return false;

	// cThemeManager 클래스 생성 및 초기화
	mpThemeManager = new cThemeManager( );
	if( !mpThemeManager->Init( mType ) )
		return false;

	// cTitleManager 클래스 생성 및 초기화
	mpTitleManager = new cTitleManager( );
	if( !mpTitleManager->Init() )
		return false;

	// cGuildManager 클래스 생성
	mpGuildManager = new cGuildManager( );
	if( mpGuildManager == NULL )
		return false;

	// cPartyUnionManager 클래스 생성
	mpPartyUnionManager = new cPartyUnionManager( );
	if( !mpPartyUnionManager->Init() )
		return false;

	// 타이머 시작
	if ( mpAppTimer != NULL )
		mpAppTimer->Start( );

	// IocpServer 초기화.
	return cIocpServer::Initialize( ipAddr, port, numWorkerThreads, bufferLength );
}

// Shutdown Method - 순서에 주위!
void cGameProcess::Shutdown(DWORD maxWait)
{
	// IocpServer 종료.
	PostServerEvent( "cGameProcess('%s:%d') - Have received a request from the manager to stop the game server is shutting down."
					 ,inet_ntoa( mAddr.sin_addr )
					 ,mPort );

	cIocpServer::Shutdown( maxWait );

	if ( mpAppTimer != NULL )
		mpAppTimer->Stop();

	//--------------------------------------------------------------------
	//[_AhnHS_CloseServerHandle]
	//--------------------------------------------------------------------
	// - DESCRIPTION:
	//   서버 핸들(Server Handle)을 닫습니다.
	//
	// - SYNTAX:
	//   void __stdcall _AhnHS_CloseServerHandle (IN AHNHS_SERVER_HANDLE hServer);
	//
	// - PARAMETERS:
	//   .pszFilePath	:AHNHS_SERVER_HANDLE	_AhnHS_CreateServerObject함수를 통해 생성한 핸들
	//
	// - RETURN VALUE:
	//   없음.
//	if ( mHackShieldHandle != ANTICPX_INVALID_HANDLE_VALUE )
//	{
//		_AhnHS_CloseServerHandle( mHackShieldHandle );
//		mHackShieldHandle = ANTICPX_INVALID_HANDLE_VALUE;
//	}

	SAFE_DELETE( mpTwitterScript );
	SAFE_DELETE( mpPartyUnionManager );
	SAFE_RELEASE_DELETE( mpGuildManager );
	SAFE_RELEASE_DELETE( mpTitleManager );
	SAFE_RELEASE_DELETE( mpThemeManager );
	SAFE_RELEASE_DELETE( mpPVPManager );
	SAFE_RELEASE_DELETE( mpDuelManager );
	SAFE_DELETE( mpQuestManager );
	SAFE_DELETE( mpTriggerManager );
	SAFE_DELETE( mpPartyManager );
	SAFE_DELETE( mpThemeUserPool );
	SAFE_DELETE( mpPvPPlayerListPool );
	SAFE_DELETE( mpPvPObjectPool );
	SAFE_DELETE( mpItemCountPool );
	SAFE_DELETE( mpItemGetPool );
	SAFE_DELETE( mpItemBillPool );
	SAFE_DELETE( mpTakeDamagePool );
	SAFE_DELETE( mpCooltimePool );
	SAFE_DELETE( mpItemManager );
	SAFE_RELEASE_DELETE( mpStageScript );
	SAFE_RELEASE_DELETE( mpDrop );
	SAFE_RELEASE_DELETE( mpAIManager );
	SAFE_DELETE( mpSkillManager );
	SAFE_RELEASE_DELETE( mpObjectManager );
	SAFE_RELEASE_DELETE( mpGridManager );
	SAFE_DELETE( mpSkillTargetPool );
	SAFE_DELETE( mpSummonPatternPool );
	SAFE_DELETE( mpRandomTable );
	SAFE_DELETE( mpMemoryPool );
	SAFE_DELETE( mpAppTimer );
	SAFE_DELETE( mpFileSystem );
}

// GetItemTimeToLive Method
unsigned long cGameProcess::GetItemTimeToLive( )
{
	switch ( mType )
	{
	default:
		return mAccumTime + 90000;
	case _E_ST_ID_THEME_:
		return mpThemeManager->NextResetTime( );
	}
}

// GetItemTimeToOwner Method
unsigned long cGameProcess::GetItemTimeToOwner( )
{
	switch ( mType )
	{
	default:
		return mAccumTime + 30000;
	case _E_ST_ID_THEME_:
		return mpThemeManager->NextResetTime( );
	}
}

// GetItemTimeToParty Method
unsigned long cGameProcess::GetItemTimeToParty( )
{
	switch ( mType )
	{
	default:
		return mAccumTime + 60000;
	case _E_ST_ID_THEME_:
		return mpThemeManager->NextResetTime( );
	}
}

// CloseCID Method
bool cGameProcess::CloseCID(DWORD cid)
{
	cCSLock           lock( &mCs );
	PerSocketContext* perSocketContext = mSocketContextPool->GetCID( cid );
	if ( perSocketContext != NULL )
	{
		Close( perSocketContext );
		return true;
	}
	return false;
}

// GetIoContext Method
PerIoContext* cGameProcess::GetIoContext(u_long request)
{
	return mIoContextPool->GetIoContext( NULL, request );
}

// ReleaseIoContext Method
void cGameProcess::ReleaseIoContext(PerIoContext* perIoContext, char* callStack)
{
	DWORD error = 0;
	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
	{
		if ( callStack != NULL )
		{
			PostServerEvent( "WARNING - cGameProcess::ReleaseIoContext:Error(=0x%08xh) / CallStack(=%s).", error, callStack );
		}
		else
		{
			PostServerEvent( "WARNING - cGameProcess::ReleaseIoContext:Error(=0x%08xh)", error );
		}
	}
}

sThemeData* cGameProcess::GetThemeData( unsigned short mapType )
{
	if( MAP_THEME_MIN <= mapType && mapType <= MAP_THEME_MAX )
	{
		unsigned char row = (unsigned char)( mapType - MAP_THEME_MIN );
		return &mThemeData[row];	
	}
	else if( MAP_ETC_MIN <= mapType && mapType <= MAP_ETC_MAX )
	{
		unsigned char row = (unsigned char)( mapType - MAP_ETC_MIN + MAP_THEME_CNT );
		return &mThemeData[row];
	}
	return NULL;
	
	/*if( MAP_THEME_MIN > mapType || mapType > MAP_THEME_MAX )
		return NULL;
	
	unsigned char row = (unsigned char)( mapType - MAP_THEME_MIN );

	return &mThemeData[row];*/
}

// GetSQL Method
char* cGameProcess::GetSQL(void** handle, UINT_PTR iParam)
{
	PerIoContext* perIoContext = NULL;

	if ( iParam >= DEFAULT_SQL_ACCOUNT && iParam < MAX_SQL_ACCOUNT )
	{
		perIoContext = g_gameSrv->GetSQLAccount( )->Get( );
	}
	else if ( iParam >= DEFAULT_SQL_GAME && iParam < MAX_SQL_GAME )
	{
		perIoContext = g_gameSrv->GetSQLGame( )->Get( );
	}
	else if ( iParam >= DEFAULT_GAME_PROCESS && iParam < MAX_GAME_PROCESS )
	{
		perIoContext = mIoContextPool->GetIoContext( NULL, IOCP_REQUEST_CALLBACK );
	}
	else
		PostServerEvent( "WARNING - cGameProcess::GetSQL:iParam(=0x%04dxh)", iParam );

	if ( perIoContext != NULL )
	{
		(*handle) = perIoContext;
		perIoContext->iParam = iParam;
		return perIoContext->buffer;
	}
	return NULL;
}

// SendSQL Method
bool cGameProcess::SendSQL(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	BOOL          retvalue     = FALSE;

	perIoContext->offset = length;

	if ( perIoContext->iParam >= DEFAULT_SQL_ACCOUNT && perIoContext->iParam < MAX_SQL_ACCOUNT )
	{
		// SQLAccount를 통해 SQLServer로 요청.
		retvalue = g_gameSrv->GetSQLAccount( )->QueueRequest( NULL, (LPOVERLAPPED)perIoContext, perIoContext->offset );
	}
	else if ( perIoContext->iParam >= DEFAULT_SQL_GAME && perIoContext->iParam < MAX_SQL_GAME )
	{
		// cSQLGame를 통해 SQLServer로 요청.
		retvalue = g_gameSrv->GetSQLGame( )->QueueRequest( NULL, (LPOVERLAPPED)perIoContext, perIoContext->offset );
	}
	else
		PostServerEvent( "WARNING - cGameProcess::SendSQL:iParam(=0x%04dxh)", perIoContext->iParam );

	return (retvalue == TRUE) ? true : false;
}
bool cGameProcess::SendSQL(PerSocketContext* perSocketContext, void* handle, unsigned long length, long commonDatabase)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	BOOL          retvalue     = FALSE;

	perIoContext->socket = perSocketContext->socket;
	perIoContext->offset = length;

	if ( perIoContext->iParam >= DEFAULT_SQL_ACCOUNT && perIoContext->iParam < MAX_SQL_ACCOUNT )
	{
		// SQLAccount를 통해 SQLServer로 요청.
		retvalue = g_gameSrv->GetSQLAccount( )->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)perIoContext, perIoContext->offset );
	}
	else if ( perIoContext->iParam >= DEFAULT_SQL_GAME && perIoContext->iParam < MAX_SQL_GAME )
	{
		// cSQLGame를 통해 SQLServer로 요청.
		retvalue = g_gameSrv->GetSQLGame( )->QueueRequest( (ULONG_PTR)perSocketContext, (LPOVERLAPPED)perIoContext, perIoContext->offset );
	}
	else
		PostServerEvent( "WARNING - cGameProcess::SendSQL:iParam(=0x%04dxh)", perIoContext->iParam );

	if ( retvalue == TRUE )
	{
		ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
		clientInfo->commonDatabase |= commonDatabase;
		return true;
	}
	else
		return false;
}
bool cGameProcess::SendSQL(unsigned long connectionIdx, void* handle, unsigned long length, long commonDatabase)
{
	PerSocketContext* perSocketContext = mSocketContextPool->GetCID( connectionIdx );
	if ( perSocketContext != NULL )
	{
		return SendSQL( perSocketContext, handle, length, commonDatabase );
	}
	else
		return false;
}

// ReleaseSQL Method
void cGameProcess::ReleaseSQL(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;

	perIoContext->offset = length;
	if ( perIoContext->iParam >= DEFAULT_SQL_ACCOUNT && perIoContext->iParam < MAX_SQL_ACCOUNT )
	{
		g_gameSrv->GetSQLAccount( )->Release( perIoContext );
	}
	else if ( perIoContext->iParam >= DEFAULT_SQL_GAME && perIoContext->iParam < MAX_SQL_GAME )
	{
		g_gameSrv->GetSQLGame( )->Release( perIoContext );
	}
	else if ( perIoContext->iParam >= DEFAULT_GAME_PROCESS && perIoContext->iParam < MAX_GAME_PROCESS )
	{
		DWORD error = 0;
		mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
		if ( error != 0 )
			PostServerEvent( "WARNING - cGameProcess::ReleaseSQL:Error(=0x%08xh)", error );
	}
	else
		PostServerEvent( "WARNING - cGameProcess::ReleaseSQL:iParam(=0x%04dxh)", perIoContext->iParam );
}

// SendGame2Game Method
bool cGameProcess::SendGame2Game(PerSocketContext* perSocketContext, u_long ipv4, u_short port)
{
	PerIoContext* sendIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	Game2Game*    packet        = (Game2Game*)sendIoContext->buffer;

	packet->ver  = PHVer;
	packet->hlen = PHLen;
	packet->tos  = TOS_GAME_2_GAME;
	packet->tlen = sizeof(Game2Game);
	packet->seq  = (++perSocketContext->seq);
	packet->ipv4 = ipv4;
	packet->port = port;

	sendIoContext->offset = packet->tlen;
	return SendPost( perSocketContext, sendIoContext );
}

// GotoInstantDungeon Method
bool cGameProcess::GotoInstantDungeon(PerSocketContext* perSocketContext, long cid)
{
	ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

	clientInfo->request.gotoInstantDungeon = true;
	clientInfo->gotoCID = cid;

	return true;
}

bool cGameProcess::GotoInstantDungeon(unsigned long connectionIdx, long cid)
{
	PerSocketContext* perSocketContext = mSocketContextPool->GetCID( connectionIdx );
	if ( perSocketContext != NULL )
	{
		return GotoInstantDungeon( perSocketContext, cid );
	}
	return false;
}


bool cGameProcess::ReturnGameServer( unsigned long connectionIdx )
{
	PerSocketContext* perSocketContext = mSocketContextPool->GetCID( connectionIdx );
	if ( perSocketContext != NULL )
	{
		ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

		if( clientInfo->request.returnToMap == true )
			return false;

		WORD lastChannel = (WORD)clientInfo->lastChannel;		 
		if( clientInfo->lastChannel > MAP_MAX )		/// 이전 채널이 게임서버가 아니면 강제로 1번채널로 이동 - 나오면 안됨
			lastChannel = 1;						/// 강제로 1번으로이동햇는데 채널이없으며 짤림

		clientInfo->request.returnToMap = true;
		clientInfo->gotoCID = MAKECID( (WORD)mServerNum, lastChannel );

		return true;
	}

	return false;
}


// SendToUser Method
bool cGameProcess::SendToUser(unsigned long connectionIdx, char* msg, unsigned long length)
{
	cCSLock  lock( &mCs );

	HANDLE   handle  = NULL;
	MSGROOT* msgRoot = (MSGROOT*)GetMsgRoot( &handle, connectionIdx );

	if ( msgRoot != NULL )
	{
		memcpy( msgRoot, msg, length );
		return SendMsgRoot( handle, length );
	}
	else
		return false;
}

// SendMsgRoot Method
bool cGameProcess::SendMsgRoot(PerSocketContext* perSocketContext, char category, char protocol)
{
	HANDLE handle = NULL;
	return ( GetMsgRoot( &handle, perSocketContext, category, protocol ) != NULL ) ? SendMsgRoot( handle, sizeof(MSGROOT) ) : false;
}
bool cGameProcess::SendMsgRoot(unsigned long connectionIdx, char category, char protocol)
{
	HANDLE handle = NULL;
	return ( GetMsgRoot( &handle, connectionIdx, category, protocol ) != NULL ) ? SendMsgRoot( handle, sizeof(MSGROOT) ) : false;
}

// SendMsgError Method
bool cGameProcess::SendMsgError(PerSocketContext* perSocketContext, char category, char protocol, long error)
{
	HANDLE     handle  = NULL;
	MSG_ERROR* sendMsg = (MSG_ERROR*)GetMsgRoot( &handle, perSocketContext, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = error;
		return SendMsgRoot( handle, sizeof(MSG_ERROR) );
	}
	return false;
}
bool cGameProcess::SendMsgError(unsigned long connectionIdx, char category, char protocol, long error)
{
	HANDLE     handle  = NULL;
	MSG_ERROR* sendMsg = (MSG_ERROR*)GetMsgRoot( &handle, connectionIdx, category, protocol );
	if ( sendMsg != NULL )
	{
		sendMsg->ErrorCode = error;
		return SendMsgRoot( handle, sizeof(MSG_ERROR) );
	}
	return false;
}

// GetMsgRoot Method
MSGROOT* cGameProcess::GetMsgRoot(void** handle, PerSocketContext* perSocketContext, char category, char protocol)
{
	PerIoContext* perIoContext = NULL;
	Packet*       packet       = NULL;
	MSGROOT*      msgRoot      = NULL;

	perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	if ( perIoContext != NULL )
	{
		packet = (Packet*)perIoContext->buffer;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_GAME;
		packet->tlen = packet->hlen;
		packet->seq  = (++perSocketContext->seq);

		(*handle) = perIoContext;
		msgRoot   = (MSGROOT*)(perIoContext->buffer + packet->hlen);
		msgRoot->Category = category;
		msgRoot->Protocol = protocol;

		perIoContext->lParam = (ULONG_PTR)perSocketContext;
	}
	return msgRoot;
}
MSGROOT* cGameProcess::GetMsgRoot(void** handle, unsigned long connectionIdx, char category, char protocol)
{
	cCSLock           lock( &mCs );
	PerSocketContext* perSocketContext = mSocketContextPool->GetCID( connectionIdx );
	return ( perSocketContext != NULL ) ? GetMsgRoot( handle, perSocketContext, category, protocol ) : NULL;
}

// SendMsgRoot Method
bool cGameProcess::SendMsgRoot(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	Packet*       packet       = (Packet*)perIoContext->buffer;

	packet->tlen = packet->hlen + (u_short)length;

	perIoContext->offset = packet->tlen;
	return SendPost( (PerSocketContext*)perIoContext->lParam, perIoContext );
}

// SendMsgRoot Method
void cGameProcess::SendMsgRoot(char* buffer, unsigned long& internal, unsigned long& internalHigh, char* msg, int msgLen)
{
	DWORD             cid;
	PerSocketContext* perSocketContext;

	while ( internal < internalHigh )
	{
		cid = *(DWORD*)(buffer + internal);
		perSocketContext = mSocketContextPool->GetCID( cid );
		internal += sizeof(DWORD);

		if ( perSocketContext != NULL )
		{
			HANDLE   handle  = NULL;
			MSGROOT* msgRoot = GetMsgRoot( &handle, perSocketContext );

			if ( msgRoot != NULL )
			{
				memcpy( msgRoot, msg, msgLen );
				SendMsgRoot( handle, msgLen );
			}
		}
	}
}

// ReleaseMsgRoot Method
void cGameProcess::ReleaseMsgRoot(void* handle, unsigned long length)
{
	PerIoContext* perIoContext = (PerIoContext*)handle;
	Packet*       packet       = (Packet*)perIoContext->buffer;
	DWORD         error        = 0;

	packet->tlen = packet->hlen + (u_short)length;

	perIoContext->offset = packet->tlen;
	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::ReleaseMsgRoot:Error(=0x%08xh)", error );
}

// GetAhnhsMsgRoot Method
MSGROOT* cGameProcess::GetHackShieldMsgRoot(void** handle, PerSocketContext* perSocketContext, char category, char protocol)
{
	PerIoContext*     perIoContext     = NULL;
	Packet*           packet           = NULL;
	MSGROOT*          msgRoot          = NULL;

	perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
	if ( perIoContext != NULL )
	{
		packet = (Packet*)perIoContext->buffer;

		packet->ver  = PHVer;
		packet->hlen = PHLen;
		packet->tos  = TOS_HACKSHIELD;
		packet->tlen = packet->hlen;
		packet->seq  = (++perSocketContext->seq);

		(*handle) = perIoContext;
		msgRoot   = (MSGROOT*)(perIoContext->buffer + packet->hlen);
		msgRoot->Category = category;
		msgRoot->Protocol = protocol;

		perIoContext->lParam = (ULONG_PTR)perSocketContext;
	}
	return msgRoot;
}

// QuickSend Method
bool cGameProcess::QuickSend(cBaseObject* object, char* msg, int length)
{
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( );
	DWORD         error        = 0;

	mpGridManager->MakeCID( object, sizeof(DWORD), perIoContext->buffer, perIoContext->length, perIoContext->InternalHigh );
	SendMsgRoot( perIoContext->buffer, perIoContext->Internal, perIoContext->InternalHigh, msg, length );

	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::QuickSend:Error(=0x%08xh)", error );
	return true;
}

// QuickSendExcept Method
bool cGameProcess::QuickSendExcept(cBaseObject* object, char* msg, int length)
{
	PerIoContext* perIoContext = mIoContextPool->GetIoContext();
	DWORD         error        = 0;

	mpGridManager->MakeCIDExcept( object, sizeof(DWORD), perIoContext->buffer, perIoContext->length, perIoContext->InternalHigh );
	SendMsgRoot( perIoContext->buffer, perIoContext->Internal, perIoContext->InternalHigh, msg, length );

	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::QuickSendExcept:Error(=0x%08xh)", error );
	return true;
}

// QuickSendMap Method
bool cGameProcess::QuickSendMap(cBaseObject* object, char* msg, int length)
{
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( );
	DWORD         error        = 0;

	mpGridManager->MakeCIDMap( object, sizeof(DWORD), perIoContext->buffer, perIoContext->length, perIoContext->InternalHigh );
	SendMsgRoot( perIoContext->buffer, perIoContext->Internal, perIoContext->InternalHigh, msg, length );

	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::QuickSendMap:Error(=0x%08xh)", error );
	return true;
}

// QuickSendMap Method
bool cGameProcess::QuickSendMap(unsigned short mapNumber, char* msg, int length)
{
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( );
	DWORD         error        = 0;

	mpGridManager->MakeCIDMap( mapNumber, sizeof(DWORD), perIoContext->buffer, perIoContext->length, perIoContext->InternalHigh );
	SendMsgRoot( perIoContext->buffer, perIoContext->Internal, perIoContext->InternalHigh, msg, length );

	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::QuickSendMap:Error(=0x%08xh)", error );
	return true;
}

// QuickSendMap Method
bool cGameProcess::QuickSendExceptMap(cPlayer* object1, cPlayer* object2, char* msg, int length)
{
	PerIoContext* perIoContext = mIoContextPool->GetIoContext( );
	DWORD         error        = 0;

	mpGridManager->MakeCIDExceptMap( object1, object2, sizeof(DWORD), perIoContext->buffer, perIoContext->length, perIoContext->InternalHigh );
	SendMsgRoot( perIoContext->buffer, perIoContext->Internal, perIoContext->InternalHigh, msg, length );

	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::QuickSendExceptMap:Error(=0x%08xh)", error );
	return true;
}

// PostServerEvent Method
bool cGameProcess::PostServerEvent(LPCTSTR format, ...)
{
	cSender* sender = g_gameSrv->GetSender( );

	LPVOID msgBuf = NULL;
	DWORD  bufferLength;
	bool   retvalue = false;

	va_list args;

	va_start( args, format );
		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );
		vsprintf( (char*)msgBuf, format, args );
	va_end( args );

	if ( msgBuf != NULL )
	{
		/// 에러 로그를 console 화면에 출력
		if( g_eventPrint == true )
			printf("\n%s",msgBuf);

		retvalue = sender->PostServerEvent( (char*)msgBuf );
		free( msgBuf );
	}
	return retvalue;
}

// PostMemberEvent Method
bool cGameProcess::PostMemberEvent(char type, char category, long memberIdx, LPCTSTR format, ...)
{
	cSender* sender = g_gameSrv->GetSender( );

	LPVOID msgBuf = NULL;
	DWORD  bufferLength;
	bool   retvalue = false;

	va_list args;

	va_start( args, format );
		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );
		vsprintf( (char*)msgBuf, format, args );
	va_end( args );

	if ( msgBuf != NULL )
	{
		retvalue = sender->PostMemberEvent( type, category, memberIdx, (char*)msgBuf );
		free( msgBuf );
	}
	return retvalue;
}

// PostCharacterEvent Method
bool cGameProcess::PostCharacterEvent(char category, long memberIdx, long characterIdx, char _level, long _exp, char _skillLevel, long _skillExp, long _skillPoint, LPCTSTR format, ...)
{
	cSender* sender = g_gameSrv->GetSender( );

	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;
	bool    retvalue = false;

	va_list args;

	va_start( args, format );
		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );
		vsprintf( (char*)msgBuf, format, args );
	va_end( args );

	if ( msgBuf != NULL )
	{
		retvalue = sender->PostCharacterEvent( category, memberIdx, mServerNum, characterIdx, _level, _exp, _skillLevel, _skillExp, _skillPoint, (char*)msgBuf );
		free( msgBuf );
	}
	return retvalue;
}

// PostMoneyEvent Method
bool cGameProcess::PostMoneyEvent(long characterIdx, long before, long after, LPCTSTR format, ...)
{
	cSender* sender = g_gameSrv->GetSender( );

	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;
	bool    retvalue = false;

	va_list args;

	va_start( args, format );
		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );
		vsprintf( (char*)msgBuf, format, args );
	va_end( args );

	if ( msgBuf != NULL )
	{
		retvalue = sender->PostMoneyEvent( mServerNum, characterIdx, before, after, (char*)msgBuf );
		free( msgBuf );
	}
	return retvalue;
}

// PostDepositEvent Method
bool cGameProcess::PostDepositEvent(long characterIdx, long before, long after, char* message)
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostDepositEvent( mServerNum, characterIdx, before, after, message );
}

// PostInventoryEvent Method
bool cGameProcess::PostInventoryEvent(char category, long characterIdx, TB_INVENTORY* inventory, LPCTSTR format, ...)
{
    // 인벤토리 로그 제외(캐시 아이템 제외)
    if ( category == EVENT_INVENTORY_UPDATE && inventory->cash == 0 )
    {
        TB_ITEM_DEFINE* itemDefine = ITEMMANAGER->GetItemDefine( inventory->itemDefineIdx );
        if ( itemDefine )
        {
            if ( (itemDefine->type == ITEM_ETC1     && itemDefine->subType == ITEM_ETC1_POTION  ) ||
                 (itemDefine->type == ITEM_MATERIAL && itemDefine->subType == ITEM_MATERIAL_MISC) )
            {
                return true;
            }
        }
    }

	cSender* sender = g_gameSrv->GetSender( );

	LPVOID  msgBuf = NULL;
	DWORD   bufferLength;
	bool    retvalue = false;

	va_list args;

	va_start( args, format );
		bufferLength = _vscprintf( format, args ) + 1;
		msgBuf       = malloc( bufferLength );
		vsprintf( (char*)msgBuf, format, args );
	va_end( args );

	if ( msgBuf != NULL )
	{
		retvalue = sender->PostInventoryEvent( mServerNum, category, inventory->idx, characterIdx, inventory->itemDefineIndex, inventory->number, inventory->count, inventory->enhanced, inventory->seal, (char*)msgBuf );
		free( msgBuf );
	}
	return retvalue;
}

// PostQuestEvent Method
bool cGameProcess::PostQuestEvent( char category, long characterIdx, long questIdx, char* message )
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostQuestEvent( mServerNum, category, characterIdx, questIdx, message );
}

// PostGuildEvent Method
bool cGameProcess::PostGuildEvent( long characterIdx, long guildIdx, char guildPosition, char* message )
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostGuildEvent( mServerNum, characterIdx, guildIdx, guildPosition, message );
}

// PostDropItemEvent Method
bool cGameProcess::PostDropItemEvent( sDropItem* pDropItem, unsigned long count )
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostDropItemEvent( mServerNum, mChannelNum, pDropItem, count );
}

// PostSkillEvent Method
bool cGameProcess::PostSkillEvent( long characterIdx, bool isReset, long skillIdx, char skillStep )
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostSkillEvent( mServerNum, characterIdx, isReset, skillIdx, skillStep );
}

bool cGameProcess::PostItemBillEvent(long characterIdx, long inventory, long before, long after)
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostItemBillEvent( mServerNum, characterIdx, inventory, before, after );
}

bool cGameProcess::PostInfCashEvent( long characterIdx, char category, SKILL_INFLUENCE_UPDATE* pInf )
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostInfCashEvent( mServerNum, characterIdx, category, pInf->mTable, pInf->mRowCount );
}

// PostReservedEvent Method
bool cGameProcess::PostReservedEvent(short type, long characterIdx, long memberIdx)
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostReservedEvent( mServerNum, type, characterIdx, memberIdx );
}

// PostPostEvent Method
bool cGameProcess::PostPostEvent(long postIdx, long characterIdx, long inventoryIdx1, long inventoryIdx2, long inventoryIdx3, long money)
{
	cSender* sender = g_gameSrv->GetSender( );
	return sender->PostPostEvent( mServerNum, postIdx, characterIdx, inventoryIdx1, inventoryIdx2, inventoryIdx3, money );
}

// PostPVPStatus Method
bool cGameProcess::PostPVPStatus(BYTE status, unsigned short pvpType, unsigned long aryPos )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostPVPStatus( cid, status, pvpType, aryPos );
}

// PostPvPNotice Method
bool cGameProcess::PostPvPNotice( unsigned char leftMinute, unsigned short pvpType, unsigned long aryPos )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostPVPNotice( cid, leftMinute, pvpType, aryPos );
}

bool cGameProcess::PostPVPPlayerList( char* msg, unsigned long len )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostPVPPlayerList( cid, msg, len );
}

// PostPVPStatus Method
bool cGameProcess::PostThemeStatus(BYTE status)
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	unsigned short mapType = GetInDunMapNum();
	return sender->PostThemeStatus( cid, status, mapType );
}

bool cGameProcess::PostPvPGmChannelMove( unsigned long playerIdx, unsigned short mapType )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostPvPGmChannelMove( cid, playerIdx, mapType );
}

bool cGameProcess::PostNoChatEvent( long characterIdx, long validThru )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostNoChatSync( cid, characterIdx, validThru );
}

bool cGameProcess::PostMonRegenSync( unsigned short levelMode, unsigned long switchGroup )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );

	MSG_SYN_POSTMONSTER_SWITCHREGEN msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTMONSTER_SWITCHREGEN) );
	msg.Category = NM_MONSTER;
	msg.Protocol = NM_MONSTERPOST_SWITCHREGEN_SYN;
	msg.mLevelMode = levelMode;
	msg.mSwitchRegen = switchGroup;

	return sender->PostMonsterSync( cid, (char*)&msg, sizeof(MSG_SYN_POSTMONSTER_SWITCHREGEN) );
}

/// 다른채널로부터 귓속말 받음
bool cGameProcess::PostChatWhisper(unsigned long senderIdx, unsigned short senderJob, wchar_t* senderName, wchar_t* recvName, wchar_t* message, sInventory* inven)
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );

	MSG_SYN_POSTCHAT_WHISPER msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTCHAT_WHISPER) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHATPOST_WHISPER_SYN;

	msg.senderIndex = senderIdx;
    msg.senderJob   = senderJob;
	wcscpy( msg.senderName, senderName );
	wcscpy( msg.recvName, recvName );
	wcscpy( msg.message, message );

	msg.inventory = *inven;

	return sender->PostChatSync( cid, (char*)&msg, msg.GetMsgLength() );
}

/// 받은 귓속말에대한 결과를 다시 보내줌
bool cGameProcess::PostChatWhisperResult(int errorCode, unsigned long senderIdx, wchar_t* recvName, ePLAYER_JOB recvJob, wchar_t* message, sInventory* inven)
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );

	MSG_SYN_POSTCHAT_WHISPERRESULT msg;
	::memset( &msg, 0, sizeof(MSG_SYN_POSTCHAT_WHISPERRESULT) );

	msg.Category = NM_CHAT;
	msg.Protocol = NM_CHATPOST_WHISPER_RESULTSYN;
	msg.error          = errorCode;

	msg.senderIndex = senderIdx;
    msg.recvJob     = (unsigned short)recvJob;
	wcscpy( msg.recvName, recvName );
	wcscpy( msg.message,  message  );
	msg.inventory = *inven;

	return sender->PostChatSync( cid, (char*)&msg, msg.GetMsgLength() );
}

// PostPostReceived Method
bool cGameProcess::PostPostReceived(long characterIdx)
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostPostReceived( cid, characterIdx );
}

bool cGameProcess::PostItemDropLimitUpdate( unsigned long itemIdx, TIMESTAMP_STRUCT endTime )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostItemDropLimitUpdate( cid, itemIdx, endTime );
}

// GM 추가 이벤트 동기화 
bool cGameProcess::PostAddGMEvent( sGMEventInfo eventInfo )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostAddGMEvent( cid, eventInfo.index, eventInfo.influenceIdx, eventInfo.startTime, eventInfo.validThru );
}

bool cGameProcess::PostDelGMEvent( unsigned long dbIndex, unsigned long influenceIdx )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostDelGMEvent( cid, dbIndex, influenceIdx );
}


bool cGameProcess::PostGMKickEvent( long characterIdx )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID( (WORD)mServerNum, (WORD)mChannelNum );
	return sender->PostGMKickEvent( cid, characterIdx );
}

// VerifyItemDefine Method
bool cGameProcess::VerifyItemDefine(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_DEFINE ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemAbility Method
bool cGameProcess::VerifyItemAbility(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_ABILITY ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemCooltime2 Method
bool cGameProcess::VerifyItemCooltime2(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_COOLTIME2 ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemLimit Method
bool cGameProcess::VerifyItemLimit(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_LIMIT ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemCard Method
bool cGameProcess::VerifyItemCard(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_CARD ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemCard Method
bool cGameProcess::VerifyItemCardSlot(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_CARD_SLOT ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemTarot Method
bool cGameProcess::VerifyItemTarot(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_TAROT ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemEnhanced Method
bool cGameProcess::VerifyItemEnhanced(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCED ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemEnhancedRate Method
bool cGameProcess::VerifyItemEnhancedRate(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCEDRATE ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemDisjoint Method
bool cGameProcess::VerifyItemDisjoint(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_DISJOINT ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// VerifyItemChange Method
bool cGameProcess::VerifyItemChange(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_ITEM_CHANGE ) != NULL) ? SendSQL( handle, sizeof(VERIFY_ITEM_TABLE) ) : false;
}

// ItemDefine Method
bool cGameProcess::ItemDefine(long idx)
{
	HANDLE             handle          = NULL;
	ITEM_DEFINE_TABLE* itemDefineTable = (ITEM_DEFINE_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_DEFINE );

	itemDefineTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_DEFINE_TABLE) );
}

// ItemAbility Method
bool cGameProcess::ItemAbility(long idx)
{
	HANDLE              handle           = NULL;
	ITEM_ABILITY_TABLE* itemAbilityTable = (ITEM_ABILITY_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_ABILITY );

	itemAbilityTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_ABILITY_TABLE) );
}

// ItemCooltime2 Method
bool cGameProcess::ItemCooltime2(long idx)
{
	HANDLE                handle             = NULL;
	ITEM_COOLTIME2_TABLE* itemCooltime2Table = (ITEM_COOLTIME2_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_COOLTIME2 );
	long                  length             = sizeof(ITEM_COOLTIME2_TABLE) - sizeof(itemCooltime2Table->table);

	itemCooltime2Table->idx = idx;

	return SendSQL( handle, length );
}

// ItemLimit Method
bool cGameProcess::ItemLimit(long idx)
{
	HANDLE            handle         = NULL;
	ITEM_LIMIT_TABLE* itemLimitTable = (ITEM_LIMIT_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_LIMIT );

	itemLimitTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_LIMIT_TABLE) );
}

// ItemCard Method
bool cGameProcess::ItemCard(long idx)
{
	HANDLE           handle        = NULL;
	ITEM_CARD_TABLE* itemCardTable = (ITEM_CARD_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_CARD );

	itemCardTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_CARD_TABLE) );
}

// ItemCardSlot Method
bool cGameProcess::ItemCardSlot(long idx)
{
	HANDLE                handle            = NULL;
	ITEM_CARD_SLOT_TABLE* itemCardSlotTable = (ITEM_CARD_SLOT_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_CARD_SLOT );

	itemCardSlotTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_CARD_SLOT_TABLE) );
}

// ItemTarot Method
bool cGameProcess::ItemTarot(long idx)
{
	HANDLE            handle         = NULL;
	ITEM_TAROT_TABLE* itemTarotTable = (ITEM_TAROT_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_TAROT );

	itemTarotTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_TAROT_TABLE) );
}

// ItemEnhanced Method
bool cGameProcess::ItemEnhanced(long idx)
{
	HANDLE               handle            = NULL;
	ITEM_ENHANCED_TABLE* itemEnhancedTable = (ITEM_ENHANCED_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_ENHANCED );

	itemEnhancedTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_ENHANCED_TABLE) );
}

// ItemEnhancedRate Method
bool cGameProcess::ItemEnhancedRate(long idx)
{
	HANDLE                    handle                = NULL;
	ITEM_ENHANCED_RATE_TABLE* itemEnhancedRateTable = (ITEM_ENHANCED_RATE_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_ENHANCEDRATE );

	itemEnhancedRateTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_ENHANCED_RATE_TABLE) );
}

// ItemDisjoint Method
bool cGameProcess::ItemDisjoint(long idx)
{
	HANDLE               handle            = NULL;
	ITEM_DISJOINT_TABLE* itemDisjointTable = (ITEM_DISJOINT_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_DISJOINT );

	itemDisjointTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_DISJOINT_TABLE) );
}

// ItemChange Method
bool cGameProcess::ItemChange(long idx)
{
	HANDLE             handle          = NULL;
	ITEM_CHANGE_TABLE* itemChangeTable = (ITEM_CHANGE_TABLE*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_CHANGE );

	itemChangeTable->idx = idx;

	return SendSQL( handle, sizeof(ITEM_CHANGE_TABLE) );
}

// VerifyDefaultItems Method
bool cGameProcess::VerifyDefaultItems(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_DEFAULT_ITEMS ) != NULL) ? SendSQL( handle, sizeof(VERIFY_DEFAULT_ITEMS) ) : false;
}

bool cGameProcess::VerifyGuild(void)
{
	HANDLE handle = NULL;
	return (GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_GUILD ) != NULL ) ? SendSQL( handle, sizeof(VERIFY_GUILD) ) : false;
}

bool cGameProcess::VerifyGmEvent(void)
{
	HANDLE			  handle  = NULL;
	VERIFY_GM_EVENT*  gmEvent = (VERIFY_GM_EVENT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_VERIFY_GM_EVENT );
	unsigned long	  length  = sizeof(VERIFY_GM_EVENT) - sizeof(gmEvent->table);

	return NETWORK2->SendSQL( handle, length );
}

bool cGameProcess::VerifyItemDropLimit(void)
{
	HANDLE			  handle  = NULL;
	ITEM_DROPLIMIT_TABLE*  pDropLimit = (ITEM_DROPLIMIT_TABLE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_ITEM_DROPLIMIT_SELECT );
	unsigned long	  length  = sizeof(ITEM_DROPLIMIT_TABLE) - sizeof(pDropLimit->mTable);

	return NETWORK2->SendSQL( handle, length );
}

bool cGameProcess::GuildSelect( long idx, bool restore )
{
	HANDLE handle = NULL;
	GUILD_SELECT* guildSelect = (GUILD_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_GUILD_SELECT );

	guildSelect->idx = idx;
	guildSelect->restore = restore;

	return SendSQL( handle, sizeof(GUILD_SELECT) );
}

bool cGameProcess::GuildUserSelect( long idx, bool restore )
{
	HANDLE			  handle          = NULL;
	GUILDUSER_SELECT* guildUserSelect = (GUILDUSER_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_GUILDUSER_SELECT );
	unsigned long	  length		  = sizeof(GUILDUSER_SELECT)-sizeof(guildUserSelect->table);

	guildUserSelect->idx = idx;
	guildUserSelect->restore = restore;

	return SendSQL( handle, length );
}

// ChannelCheck Method
bool cGameProcess::ChannelCheck(short serverNum, short channelNum, char* inetnum, unsigned long ipv4, unsigned short port)
{
	HANDLE         handle = NULL;
	CHANNEL_CHECK* channelCheck = (CHANNEL_CHECK*)GetSQL( &handle, SQL_GAME_PROCESS_CHANNEL_CHECK );

	channelCheck->serverNum  = serverNum;
	channelCheck->channelNum = channelNum;
	strcpy( channelCheck->inetnum, inetnum );
	channelCheck->ipv4       = ipv4;
	channelCheck->port       = port;
	channelCheck->retvalue   = 0;

	return SendSQL( handle, sizeof(CHANNEL_CHECK) );
}

// CaptChaOffset Method
bool cGameProcess::CaptChaOffset( )
{
	HANDLE         handle = NULL;
	CAPTCHA_OFFSET* captcha = (CAPTCHA_OFFSET*)GetSQL( &handle, SQL_GAME_PROCESS_CAPTCHA_OFFSET );
    captcha->offset = 0;

	return SendSQL( handle, sizeof(CAPTCHA_OFFSET) );
}

// InitGameDb Method
bool cGameProcess::InitGameDb( )
{
	HANDLE     handle   = NULL;
	INIT_GAME* initGame = (INIT_GAME*)GetSQL( &handle, SQL_REQUEST_GAME_INIT );

	initGame->channelNum = mChannelNum;

	return SendSQL( handle, sizeof(INIT_GAME) );
}

// ShutdownGameDb Method
bool cGameProcess::ShutdownGameDb( )
{
	HANDLE         handle       = NULL;
	SHUTDOWN_GAME* shutdownGame = (SHUTDOWN_GAME*)GetSQL( &handle, SQL_REQUEST_GAME_SHUTDOWN );

	shutdownGame->channelNum = mChannelNum;

	return SendSQL( handle, sizeof(SHUTDOWN_GAME) );
}

// ShutdownAccountDb Method
bool cGameProcess::ShutdownAccountDb( )
{
	HANDLE            handle          = NULL;
	SHUTDOWN_ACCOUNT* shutdownAccount = (SHUTDOWN_ACCOUNT*)GetSQL( &handle, SQL_REQUEST_ACCOUNT_SHUTDOWN );

	shutdownAccount->serverNum  = (short)mServerNum;
	shutdownAccount->channelNum = (short)mChannelNum;

	return SendSQL( handle, sizeof(SHUTDOWN_ACCOUNT) );
}

// CharacterList Method
bool cGameProcess::CharacterList(PerSocketContext* perSocketContext, long memberIdx)
{
	HANDLE          handle        = NULL;
	CHARACTER_LIST* characterList = (CHARACTER_LIST*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_LIST );
	unsigned long   length        = sizeof(CHARACTER_LIST) - sizeof(characterList->table);

	characterList->userIdx = memberIdx;

	return SendSQL( perSocketContext, handle, length );
}

// CharacterDelete Method
bool cGameProcess::CharacterDelete(PerSocketContext* perSocketContext, long memberIdx, long characterIdx, bool reuest)
{
    HANDLE            handle          = NULL;
    CHARACTER_DELETE* characterDelete = (CHARACTER_DELETE*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_DELETE );

    characterDelete->idx     = characterIdx;
    characterDelete->userIdx = memberIdx;
    characterDelete->request = ( reuest == true ) ? 1 : 0;

    return SendSQL( perSocketContext, handle, sizeof(CHARACTER_DELETE) );
}

// CharacterTTL Method
bool cGameProcess::CharacterTTL(long characterIdx, long ttl, DWORD channelNum )
{
	HANDLE         handle       = NULL;
	CHARACTER_TTL* characterTTL = (CHARACTER_TTL*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_TTL );

	characterTTL->idx = characterIdx;
	characterTTL->ttl = ttl;
	characterTTL->channelNum = channelNum;

	return SendSQL( handle, sizeof(CHARACTER_TTL) );
}

// InventorySelect Method
bool cGameProcess::InventorySelect(PerSocketContext* perSocketContext, long characterIdx)
{
	HANDLE            handle          = NULL;
	INVENTORY_SELECT* inventorySelect = (INVENTORY_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_SELECT );
	unsigned long     length          = sizeof(INVENTORY_SELECT) - sizeof(inventorySelect->table);

	inventorySelect->characterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// InventoryDelete Method
bool cGameProcess::InventoryDelete(PerSocketContext* perSocketContext, INVENTORY_WARNING warning, long characterIdx, TB_INVENTORY* table)
{
	HANDLE            handle          = NULL;
	INVENTORY_DELETE* inventoryDelete = (INVENTORY_DELETE*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_DELETE );

	inventoryDelete->warning      = warning;
	inventoryDelete->characterIdx = characterIdx;
	inventoryDelete->table        = (*table);

	return SendSQL( perSocketContext, handle, sizeof(INVENTORY_DELETE) );
}

// InventoryCooltimeSelect Method
bool cGameProcess::InventoryCooltimeSelect(PerSocketContext* perSocketContext, long characterIdx)
{
	HANDLE                     handle                  = NULL;
	INVENTORY_COOLTIME_SELECT* inventoryCooltimeSelect = (INVENTORY_COOLTIME_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_COOLTIME_SELECT );
	unsigned long              length                  = sizeof(INVENTORY_COOLTIME_SELECT) - sizeof(inventoryCooltimeSelect->table);

	inventoryCooltimeSelect->characterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// InventorySwap Method
bool cGameProcess::InventorySwap(PerSocketContext* perSocketContext, long idx1, short number1, long idx2, short number2)
{
	HANDLE          handle        = NULL;
	INVENTORY_SWAP* inventorySwap = (INVENTORY_SWAP*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_SWAP );

	inventorySwap->idx1    = idx1;
	inventorySwap->number1 = number1;
	inventorySwap->idx2    = idx2;
	inventorySwap->number2 = number2;

	return SendSQL( perSocketContext, handle, sizeof(INVENTORY_SWAP) );
}

// InventoryMove Method
bool cGameProcess::InventoryMove(PerSocketContext* perSocketContext, long idx, short number, bool cashBox)
{
	HANDLE          handle        = NULL;
	INVENTORY_MOVE* inventoryMove = (INVENTORY_MOVE*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_MOVE );

	inventoryMove->idx     = idx;
	inventoryMove->number  = number;
	inventoryMove->cashBox = cashBox;

	return SendSQL( perSocketContext, handle, sizeof(INVENTORY_MOVE) );
}

// InventoryExcept Method
bool cGameProcess::InventoryExcept(PerSocketContext* perSocketContext, long* idx, short* number)
{
	HANDLE            handle          = NULL;
	INVENTORY_EXCEPT* inventoryExcept = (INVENTORY_EXCEPT*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_EXCEPT );
	long              length          = sizeof(INVENTORY_EXCEPT) - sizeof(inventoryExcept->table);

	for ( int i = 0; i < 3 && (*idx) != 0; i++, idx++, number++ )
	{
		inventoryExcept->rowCount++;
		inventoryExcept->table[ i ].idx = (*idx);
		inventoryExcept->table[ i ].number = (*number);
	}

	length += inventoryExcept->rowCount * sizeof(inventoryExcept->table);
	return SendSQL( perSocketContext, handle, length );
}

// InventoryExcept Method
bool cGameProcess::InventoryMerge(PerSocketContext* perSocketContext, long idx1, long idx2, short capacity)
{
    HANDLE           handle         = NULL;
    INVENTORY_MERGE* inventoryMerge = (INVENTORY_MERGE*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_MERGE );
    unsigned long    length         = sizeof(INVENTORY_MERGE) - sizeof(inventoryMerge->table);

    inventoryMerge->idx1     = idx1;
    inventoryMerge->idx2     = idx2;
    inventoryMerge->capacity = capacity;

    return SendSQL( perSocketContext, handle, length );
}

// ItemBillSelect Method
bool cGameProcess::ItemBillSelect(PerSocketContext* perSocketContext, long characterIdx)
{
    HANDLE            handle = NULL;
	ITEM_BILL_SELECT* select = (ITEM_BILL_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_BILL_SELECT );
    unsigned long     length = sizeof(ITEM_BILL_SELECT) - sizeof(select->table);

	select->characterIdx = characterIdx;

    return SendSQL( perSocketContext, handle, length );
}

// FriendSelect Method
bool cGameProcess::FriendSelect(long characterIdx)
{
	HANDLE         handle       = NULL;
	FRIEND_SELECT* friendSelect = (FRIEND_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_FRIEND_SELECT );
	unsigned long  length       = sizeof(FRIEND_SELECT) - sizeof(friendSelect->table);

	friendSelect->characterIdx = characterIdx;

	return SendSQL( handle, length );
}

// ItemTarotResult Method
bool cGameProcess::ItemTarotResult(PerSocketContext* perSocketContext, long itemDefineIndex, long value)
{
	HANDLE             handle          = NULL;
	ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)GetSQL( &handle, SQL_GAME_PROCESS_ITEM_TAROT_RESULT );

	itemTarotResult->itemDefineIndex = itemDefineIndex;
	itemTarotResult->value           = value;

	return SendSQL( perSocketContext, handle, sizeof(ITEM_TAROT_RESULT) );
}

// SkillSelect Method
bool cGameProcess::SkillSelect( PerSocketContext* perSocketContext, unsigned long characterIdx )
{
	HANDLE        handle      = NULL;
	SKILL_SELECT* skillSelect = (SKILL_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_SKILL_SELECT );
	unsigned long length      = sizeof(SKILL_SELECT) - sizeof(skillSelect->mTable);

	skillSelect->mCharacterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// SkillInsert Method
bool cGameProcess::SkillInsert( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, /*unsigned long money,*/ unsigned short sp, unsigned char skillLevel, unsigned long skillPoint )
{
	HANDLE        handle      = NULL;
	SKILL_INSERT* skillInsert = (SKILL_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_SKILL_INSERT );

	skillInsert->mCharacterIdx  = characterIdx;
	skillInsert->mSkillClassIdx = skillClassIdx;
	/*skillInsert->mMoney         = money;*/
	skillInsert->mSP            = sp;
	skillInsert->skillLevel		= skillLevel;
	skillInsert->skillPoint		= skillPoint;
	skillInsert->mRetvalue		= -1;

	return SendSQL( perSocketContext, handle, sizeof(SKILL_INSERT) );
}

// SkillUpdate Method
bool cGameProcess::SkillUpdate( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, unsigned char skillStep, /*unsigned long money,*/ unsigned short sp, unsigned char skillLevel, unsigned long skillPoint )
{
	HANDLE        handle      = NULL;
	SKILL_UPDATE* skillUpdate = (SKILL_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_SKILL_UPDATE );

	skillUpdate->mCharacterIdx  = characterIdx;
	skillUpdate->mSkillClassIdx = skillClassIdx;
	skillUpdate->mSkillStep     = skillStep;
 	/*skillInsert->mMoney         = money;*/
 	skillUpdate->mSP            = sp;
 	skillUpdate->skillLevel		= skillLevel;
 	skillUpdate->skillPoint		= skillPoint;
	skillUpdate->mRetvalue		= -1;

	return SendSQL( perSocketContext, handle, sizeof(SKILL_UPDATE) );
}

// SkillCheatInsert Method
bool cGameProcess::SkillCheatInsert( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, unsigned char skillStep )
{
	HANDLE				handle      = NULL;
	SKILL_CHEAT_INSERT* skillInsert = (SKILL_CHEAT_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_SKILL_CHEAT_INSERT );

	skillInsert->mCharacterIdx  = characterIdx;
	skillInsert->mSkillClassIdx = skillClassIdx;
	skillInsert->mSkillStep		= skillStep;
	skillInsert->mRetvalue		= -1;

	return SendSQL( perSocketContext, handle, sizeof(SKILL_CHEAT_INSERT) );
}

// SkillCheatUpdate Method
bool cGameProcess::SkillCheatUpdate( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, unsigned char skillStep )
{
	HANDLE				handle      = NULL;
	SKILL_CHEAT_UPDATE* skillUpdate = (SKILL_CHEAT_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_SKILL_CHEAT_UPDATE );

	skillUpdate->mCharacterIdx  = characterIdx;
	skillUpdate->mSkillClassIdx = skillClassIdx;
	skillUpdate->mSkillStep     = skillStep;
 	skillUpdate->mRetvalue		= -1;

	return SendSQL( perSocketContext, handle, sizeof(SKILL_CHEAT_UPDATE) );
}

// SkillCoolTime Method
bool cGameProcess::SkillCoolTime( PerSocketContext* perSocketContext, unsigned long characterIdx )
{
	/// 튜토리얼에서 사용하던 스킬은 저장하지 않는다.
	if( mType == _E_ST_ID_TUTORIAL_ )
		return true;

	HANDLE          handle        = NULL;
	SKILL_COOLTIME* skillCoolTime = (SKILL_COOLTIME*)GetSQL( &handle, SQL_GAME_PROCESS_SKILL_COOLTIME );
	unsigned long   length        = sizeof(SKILL_COOLTIME) - sizeof(skillCoolTime->mTable);

	if ( !SKILLMANAGER->SaveSkillCoolTime( characterIdx, skillCoolTime ) )
	{
		length += skillCoolTime->mRowCount * sizeof(TB_CHARACTER_SKILL);
		// 저장 할 쿨타임이 없는 경우	
		ReleaseSQL( handle, length );
		return true;
	}

	length += skillCoolTime->mRowCount * sizeof(TB_CHARACTER_SKILL);
	return SendSQL( perSocketContext, handle, length );
}

// RecipeTime Method
bool cGameProcess::RecipeCoolTime( PerSocketContext* perSocketContext, cPlayer* hero )
{
	HANDLE          handle        = NULL;
	RECIPE_COOLTIME* pCoolTime = (RECIPE_COOLTIME*)GetSQL( &handle, SQL_GAME_PROCESS_RECIPE_COOLTIME );
	unsigned long   length        = sizeof(RECIPE_COOLTIME) - sizeof(pCoolTime->mTable);
	
	if ( !hero->SaveRecipeCoolTime( pCoolTime ) )
	{
		length += pCoolTime->mRowCount * sizeof(TB_RECIPE_COOLTIME);
		// 저장 할 쿨타임이 없는 경우	
		ReleaseSQL( handle, length );
		return true;
	}

	length += pCoolTime->mRowCount * sizeof(TB_RECIPE_COOLTIME);
	return SendSQL( perSocketContext, handle, length );
}

// SkillInfluenceSelect Method
bool cGameProcess::SkillInfluenceSelect( PerSocketContext* perSocketContext, unsigned long characterIdx )
{
	HANDLE                  handle          = NULL;
	SKILL_INFLUENCE_SELECT* influenceSelect = (SKILL_INFLUENCE_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_INFLUENCE_SELECT );
	unsigned long           length          = sizeof(SKILL_INFLUENCE_SELECT) - sizeof(influenceSelect->mTable);

	influenceSelect->mCharacterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// MakeSkillSelect Method
bool cGameProcess::MakeSkillSelect( PerSocketContext* perSocketContext, unsigned long characterIdx )
{
	HANDLE handle = NULL;
	MAKESKILL_SELECT* pMakeSkill= (MAKESKILL_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_MAKESKILL_SELECT );
	unsigned long length = sizeof(MAKESKILL_SELECT) - sizeof(pMakeSkill->mTable);

	pMakeSkill->mCharacterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

bool cGameProcess::FortuneSelect( PerSocketContext* perSocketContext, unsigned long characterIdx )
{
	HANDLE				handle = NULL;
	FORTUNE_SELECT*		fortuneSelect = (FORTUNE_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_FORTUNE_SELECT );

	fortuneSelect->characterIdx = characterIdx;

	/// 초기화
	TIMESTAMP_STRUCT* thru = fortuneSelect->thru;
	for( int i = 0; i < 5; ++i, ++thru )
	{
		thru->year		= 1900;
		thru->month		= 1;
		thru->day		= 1;
		thru->hour		= 0;
		thru->minute	= 0;
		thru->second	= 0;
		thru->fraction  = 0;
	}

	return SendSQL( perSocketContext, handle, sizeof(FORTUNE_SELECT) );
}

// WhisperMemberSelect Method
bool cGameProcess::WhisperSelect( PerSocketContext* perSocketContext, wchar_t* recvName, wchar_t* message, short slotIndex )
{
	HANDLE          handle = NULL;
	WHISPER_SELECT*	select = (WHISPER_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_WHISPER );
	
	wcscpy( select->name, recvName );
	wcscpy( select->message, message );
	select->slotIndex = slotIndex;

	return SendSQL( perSocketContext, handle, sizeof(WHISPER_SELECT) );
}

// ThemeUserSelect Method
bool cGameProcess::ThemeUserSelect(PerSocketContext* perSocketContext, long characterIdx)
{
	HANDLE             handle          = NULL;
	THEME_USER_SELECT* themeUserSelect = (THEME_USER_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_SELECT );
	unsigned long      length          = sizeof(THEME_USER_SELECT) - sizeof(themeUserSelect->table);

	themeUserSelect->characterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// StallSellDelete Method
bool cGameProcess::ShortcutSelect(PerSocketContext* perSocketContext, unsigned long characterIdx)
{
	HANDLE           handle         = NULL;
	SHORTCUT_SELECT* shortcutSelect = (SHORTCUT_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_SHORTCUT_SELECT );

	shortcutSelect->characterIdx = characterIdx;
	shortcutSelect->rowCount	 = 0;

	return SendSQL( perSocketContext, handle, sizeof(SHORTCUT_SELECT) );
}

// QuestSelect Method
bool cGameProcess::QuestSelect( PerSocketContext* perSocketContext, long characterIdx )
{
	HANDLE        handle      = NULL;
	QUEST_SELECT* questSelect = (QUEST_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_QUEST_SELECT );
	unsigned long length      = sizeof(QUEST_SELECT)-sizeof(questSelect->table);

	questSelect->characterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// QuestCompleteSelect Method
bool cGameProcess::QuestCompleteSelect( PerSocketContext* perSocketContext, long characterIdx )
{
	HANDLE			handle		  = NULL;
	QUEST_ENDLIST*  questEndList = (QUEST_ENDLIST*)GetSQL( &handle, SQL_GAME_PROCESS_QUEST_ENDLISTSELECT );
	unsigned long	length      = sizeof(QUEST_ENDLIST)-sizeof(questEndList->questIdx);

	questEndList->characterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// QuestValidSelect Method
bool cGameProcess::QuestValidSelect( PerSocketContext* perSocketContext, long characterIdx )
{
	HANDLE				handle		  = NULL;
	QUEST_VALIDLIST*	questValidList = (QUEST_VALIDLIST*)GetSQL( &handle, SQL_GAME_PROCESS_QUEST_VALIDLISTSELECT );
	unsigned long		length      = sizeof(QUEST_VALIDLIST)-sizeof(questValidList->table);

	questValidList->characterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, length );
}

// QuestRemove Method
bool cGameProcess::QuestRemove(PerSocketContext* perSocketContext, long dbIdx )
{
	HANDLE            handle      = NULL;
	QUEST_REMOVE*	  questRemove = (QUEST_REMOVE*)GetSQL( &handle, SQL_GAME_PROCESS_QUEST_REMOVE );

	questRemove->idx = dbIdx;

	return SendSQL( perSocketContext, handle, sizeof(QUEST_REMOVE) );
}

// TitleSelect Method
bool cGameProcess::TitleSelect( PerSocketContext* perSocketContext, long characterIdx )
{
	HANDLE			handle		= NULL;
	TITLE_SELECT*	titleSelect = (TITLE_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_TITLE_SELECT );

	titleSelect->characterIdx	= characterIdx;

	return SendSQL( perSocketContext, handle, sizeof(TITLE_SELECT) );
}

// TrialTimeSelect Method
bool cGameProcess::TrialTimeSelect( PerSocketContext* perSocketContext, long memberIdx )
{
	HANDLE			handle		= NULL;
	TRIAL_SELECT*	trialSelect = (TRIAL_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_TRIAL_SELECT );

    trialSelect->memberIdx = memberIdx;

	return SendSQL( perSocketContext, handle, sizeof(TRIAL_SELECT) );
}

// GuildCreate Method
bool cGameProcess::GuildCreate( PerSocketContext* perSocketContext, long characterIdx, wchar_t* guildName, wchar_t* masterName )
{
	HANDLE			handle		= NULL;
	GUILD_CREATE*	guildCreate = (GUILD_CREATE*)GetSQL( &handle, SQL_GAME_PROCESS_GUILD_CREATE );

	guildCreate->characterIdx = characterIdx;
	wcscpy( guildCreate->name, guildName );
	wcscpy( guildCreate->masterName, masterName );
	guildCreate->money = -GUILDLIMIT_MONEY;

	return SendSQL( perSocketContext, handle, sizeof(GUILD_CREATE) );
}

// GuildDelete Method
bool cGameProcess::GuildDelete(PerSocketContext* perSocketContext, long idx, long characterIdx )
{
	HANDLE			handle		= NULL;
	GUILD_DELETE*	guildDelete = (GUILD_DELETE*)GetSQL( &handle, SQL_GAME_PROCESS_GUILD_DELETE );

	guildDelete->idx = idx;
	guildDelete->charaterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, sizeof(GUILD_DELETE) );
}

// GuildUserAdd Method
bool cGameProcess::GuildUserAdd( PerSocketContext* perSocketContext, long idx, long characterIdx )
{
	HANDLE			handle		 = NULL;
	GUILDUSER_ADD*	guildUserAdd = (GUILDUSER_ADD*)GetSQL( &handle, SQL_GAME_PROCESS_GUILDUSER_ADD );

	guildUserAdd->idx = idx;
	guildUserAdd->charaterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, sizeof(GUILDUSER_ADD) );
}

// GuildUserOut Method
bool cGameProcess::GuildUserOut( PerSocketContext* perSocketContext, long idx, long characterIdx )
{
	HANDLE			handle		 = NULL;
	GUILDUSER_OUT*	guildUserOut = (GUILDUSER_OUT*)GetSQL( &handle, SQL_GAME_PROCESS_GUILDUSER_OUT );

	guildUserOut->idx = idx;
	guildUserOut->charaterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, sizeof(GUILDUSER_OUT) );
}

// GuildUserCut Method
bool cGameProcess::GuildUserCut( PerSocketContext* perSocketContext, long idx, long characterIdx )
{
	HANDLE			handle		 = NULL;
	GUILDUSER_OUT*	guildUserOut = (GUILDUSER_OUT*)GetSQL( &handle, SQL_GAME_PROCESS_GUILDUSER_CUT );

	guildUserOut->idx = idx;
	guildUserOut->charaterIdx = characterIdx;

	return SendSQL( perSocketContext, handle, sizeof(GUILDUSER_OUT) );
}

// GuildUserGivePosition Method
bool cGameProcess::GuildUserGivePosition( PerSocketContext* perSocketContext, long idx, long characterIdx, char position )
{
	HANDLE			handle		= NULL;
	GUILDUSER_GIVEPOSITION*		guildUserGivePosition = (GUILDUSER_GIVEPOSITION*)GetSQL( &handle, SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION );

	guildUserGivePosition->idx			= idx;
	guildUserGivePosition->characterIdx = characterIdx;
	guildUserGivePosition->position		= position;

	return SendSQL( perSocketContext, handle, sizeof(GUILDUSER_GIVEPOSITION) );
}

// GuildMarkUpdate Method
bool cGameProcess::GuildMarkUpdate( PerSocketContext* perSocketContext, long idx, TIMESTAMP_STRUCT registDate, BYTE* mark )
{
	HANDLE						handle = NULL;
	GUILDMARK_UPDATE*  guildMarkUpdate = (GUILDMARK_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_GUILDMARK_UPDATE );
	
	guildMarkUpdate->idx			= idx;
	guildMarkUpdate->markRegistDate = registDate;
	memcpy( &guildMarkUpdate->mark, mark, sizeof(TB_GUILDMARK));

	return SendSQL( perSocketContext, handle, sizeof(GUILDMARK_UPDATE) );
}

// GM JobUpdate Methos
bool cGameProcess::CheatJobUpdate( PerSocketContext* perSocketContext, long characterIdx, long updateJobIndex, unsigned char race, long jobStep )
{
	HANDLE						handle = NULL;
	GM_JOB_UPDATE*				jobUpdate = (GM_JOB_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_JOB_CHEAT_UPDATE );
	unsigned long				length    = sizeof(GM_JOB_UPDATE) - sizeof(jobUpdate->skillClassIdx);

	jobUpdate->characterIdx = characterIdx;
	jobUpdate->jobIndex		= updateJobIndex;

	jobUpdate->jobStep = jobStep;
	jobUpdate->race    = race;
	jobUpdate->skillClassIdx;
	jobUpdate->skillRowCount;

	return SendSQL( perSocketContext, handle, length );
}

// ChannelSync Method
void cGameProcess::ChannelSync( )
{
	cSender* sender = g_gameSrv->GetSender( );
	long     cid    = MAKECID((WORD)mServerNum, (WORD)mChannelNum);
	sender->PostChStatus( cid, (BYTE)mStatus );
}

// ChannelStatus Method
void cGameProcess::ChannelStatus(long cid, BYTE status)
{
	if ( mServerNum == HIWORD( cid ) )
	{
		cCSLock lock( &mCs );
		WORD    id = LOWORD( cid ) - 1;

		if ( mChannelNum == LOWORD( cid ) )
		{
			switch ( status )
			{
			case _E_STATUS_RUNABLE_:
				if ( mStatus == _E_STATUS_RUNABLE_ )
				{
					mComplete.channelSyn = true;
					mStatus = _E_STATUS_RUNNING_S1_;
					g_gameSrv->GetSender( )->PostChStatus( cid, (BYTE)mStatus );
				}
				break;
			case _E_STATUS_RUNNING_S1_:
			case _E_STATUS_RUNNING_S2_:
			case _E_STATUS_RUNNING_S3_:
			case _E_STATUS_RUNNING_S4_:
				break;
			case _E_STATUS_FIN_WAIT1_:
			case _E_STATUS_FIN_WAIT2_:
			case _E_STATUS_LAST_ACK_:
				g_gameSrv->GetSender( )->PostChStatus( cid, (BYTE)mStatus );
				break;
			}
		}
		if ( id < MAX_CHANNEL )
		{
			ChannelList* channel = (mChannelList + id);

			switch ( status )
			{
			case _E_STATUS_RUNABLE_:
			case _E_STATUS_RUNNING_S1_:
			case _E_STATUS_RUNNING_S2_:
			case _E_STATUS_RUNNING_S3_:
			case _E_STATUS_RUNNING_S4_:
				channel->used = true;
				channel->status = status;
				break;
			case _E_STATUS_CLOSED_:		// 종료완료
			case _E_STATUS_CLOSING_:	// 정상종료
			case _E_STATUS_FIN_WAIT1_:	// 마지막 대기 1
			case _E_STATUS_FIN_WAIT2_:	// 마지막 대기 2
			case _E_STATUS_LAST_ACK_:	// 최종확인
			case _E_STATUS_DELETE_:		// 강제삭제
			case _E_STATUS_ERROR_:		// 오류종료
				channel->used = false;
				channel->status = status;
				break;
			}
		}
	}
}

// ServerNotice Method
void cGameProcess::ServerNotice(long cid, wchar_t* message)
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && mChannelNum != (DWORD)LOWORD( cid ) )
	{
		cCSLock lock( &mCs );

		MSG_SYN_NOTICE_SERVER syn;
			syn.Category = NM_NOTICE;
			syn.Protocol = NM_NOTICE_NOTICE_SERVER_SYN;
			wcscpy( syn.mMessage, message );
			syn.mMessage[MAX_CHAT_SIZE] = 0;
		OBJECTMANAGER->SendNotice( (char*)&syn, syn.GetMsgLength( ) );
	}
}

// Server Down Count Notice Method
void cGameProcess::ServerDownCountNotice(long cid, unsigned short count )
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && mChannelNum != (DWORD)LOWORD( cid ) )
	{
		cCSLock lock( &mCs );

		MSG_SYN_NOTICE_DOWN_COUNT_SERVER  syn;
		syn.Category = NM_NOTICE;
		syn.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_SERVER_SYN;
		syn.mCount    = count;
		OBJECTMANAGER->SendNotice( (char*)&syn, sizeof(syn) );
	}
}

// ChatMegaphone Method
void cGameProcess::ChatMegaphone(long cid, char* msg, int len)
{
	if ( mServerNum == (DWORD)HIWORD( cid ) )
	{
		cCSLock lock( &mCs );
		MSG_SYN_CHAT_MEGAPHONE* recvMsg = (MSG_SYN_CHAT_MEGAPHONE*)msg;

		PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
		ClientInfo*       clientInfo;

		while ( socketContext != NULL )
		{
			clientInfo = (ClientInfo*)socketContext->buffer;

			if ( clientInfo->complete.gameIn )
			{
				HANDLE                  handle  = NULL;
				MSG_SYN_CHAT_MEGAPHONE* sendSyn = (MSG_SYN_CHAT_MEGAPHONE*)GetMsgRoot( &handle, socketContext );
				memcpy( sendSyn, recvMsg, len );
				SendMsgRoot( handle, len );
			}

			socketContext = socketContext->next;
		}
	}
}

// PVPStatus Method
void cGameProcess::PVPStatus(long cid, BYTE status, unsigned short pvpType, unsigned long aryPos )
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && 
		MAP_PVP_MIN <= (DWORD)LOWORD( cid ) && (DWORD)LOWORD( cid ) <= MAP_PVP_MAX && 
		MAP_MIN <= mChannelNum && mChannelNum <= MAP_MAX )
	{
		cCSLock lock( &mCs );

		if( aryPos >= PVP_DM_LEVEL_SECTION_MAX )
			return;

		if( pvpType != mPVPData[aryPos].pvpType )
		{
			PostServerEvent("cGameProcess::PVPStatus Duplicate[%d,%d][%d,%d,%d][%d]", mPVPData[aryPos].pvpType, status, cid, status, pvpType, aryPos );
			return;
		}

		switch ( status )
		{
		case _E_PVP_CLOSED_:
			mPVPData[aryPos].timer = 0;
			mPVPData[aryPos].pvpType = MAP_NONE;
			break;
		case _E_PVP_READY_:
			mpObjectManager->SendMsgSynPVPReadyQuestion( pvpType, aryPos );
			mPVPData[aryPos].timer = GetAccumTime() + PVP_DM_JOIN_TIME;
			break;
		case _E_PVP_PLAYERLISTWAIT_:
			//mpObjectManager->SendMsgSynPVPReadyClose( aryPos );
			mpPVPManager->SendMsgSynPvPReadyClose( aryPos );
			mPVPData[aryPos].timer = 0;
			break;
		case _E_PVP_RUNNING_:
			mPVPData[aryPos].timer = 0;
			break;
		case _E_PVP_CLOSING_:
			mPVPData[aryPos].timer = 0;
			mPVPData[aryPos].pvpType = MAP_NONE;
			break;
		}
		mPVPData[aryPos].cid    = cid;
		mPVPData[aryPos].status = (PVP_STATUS)status;
	}
}

// PVPStatus Method
void cGameProcess::PVPNotice( long cid, unsigned char leftMinute, unsigned short pvpType, unsigned long aryPos )
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && 
		MAP_PVP_MIN <= (DWORD)LOWORD( cid ) && (DWORD)LOWORD( cid ) <= MAP_PVP_MAX && 
		MAP_MIN <= mChannelNum && mChannelNum <= MAP_MAX )
	{
		cCSLock lock( &mCs );

		if( aryPos >= PVP_DM_LEVEL_SECTION_MAX )
			return;

		/// 앞에서 진행된 pvp가 끝났는지 체크
		if( MAP_NONE != mPVPData[aryPos].pvpType )
		{
			/// 무조건 끝아야 할 시간이 지났는지 체크
			if( mPVPData[aryPos].pvpEndTime < GetAccumTime() )
			{
				/// 시간이 지난 경우는 앞단 pvp 서버가 다운되서 종료신호를 안준경우라 로그만 찍어놓고 다음 pvp를 진행
				PostServerEvent("cGameProcess::PVPNotice new[%d,%d,%d][%d,%d][%d]", 
					mPVPData[aryPos].pvpType, mPVPData[aryPos].status, mPVPData[aryPos].pvpEndTime, pvpType, GetAccumTime(), aryPos );
			}
			else
			{
				if( mPVPData[aryPos].pvpType != pvpType )
				{
					/// 아직 앞단 서버에서 끝날 시간이 안된 경우이므로 스크립트 작성이 중복 됐을가능성이큼 - 진행불가
					PostServerEvent("cGameProcess::PVPNotice Duplicate[%d,%d,%d][%d,%d][%d]", 
						mPVPData[aryPos].pvpType, mPVPData[aryPos].status, mPVPData[aryPos].pvpEndTime, pvpType, GetAccumTime(), aryPos );
					return;
				}
			}
		}

		sDMInfo* pInfo = PVPSCRIPT->GetDmInfo( pvpType );
		if( pInfo == NULL )
		{
			PostServerEvent("cGameProcess::PVPNotice pInfo(%d) == NULL", pvpType );
			return;
		}
		mPVPData[aryPos].cid = cid;
		mPVPData[aryPos].status = _E_PVP_NOTICE_;
		mPVPData[aryPos].pvpType = pvpType;

		/// 공지시간 + 입장대기시간 + 채널이동대기시간
		/// + 게임총플레이시간 + 게임아웃대기시간 + 지연여유대기시간1분
		mPVPData[aryPos].pvpEndTime = leftMinute * MINUTE + PVP_DM_JOIN_TIME + PVP_DM_READY_TIME 
			+ pInfo->mTotalDMTime * MINUTE + PVP_DM_OUTWAIT_TIME + 1 * MINUTE
			+ GetAccumTime();

		OBJECTMANAGER->SendPVPNotice( leftMinute, pvpType, aryPos );
	}
}

// ThemeStatus Method
void cGameProcess::PVPPlayerList( MB_SYN_PVP_PLAYERLIST* pMBMsg )
{
	if ( mServerNum == (DWORD)HIWORD( pMBMsg->cid ) && 
		MAP_PVP_MIN <= (DWORD)LOWORD( pMBMsg->cid ) && (DWORD)LOWORD( pMBMsg->cid ) <= MAP_PVP_MAX && 
		MAP_MIN <= mChannelNum && mChannelNum <= MAP_MAX )
	{
		cCSLock lock( &mCs );
		MSG_SYN_POSTPVP_PLAYERLIST* pList = (MSG_SYN_POSTPVP_PLAYERLIST*)pMBMsg->msg;

		unsigned long aryPos = pList->mAryPos;
		if( PVP_DM_LEVEL_SECTION_MAX <= aryPos )
			return;

		if( mPVPData[aryPos].status != _E_PVP_PLAYERLISTWAIT_ )
			return;

		mPVPData[aryPos].pvpType = MAP_NONE;
		mPVPData[aryPos].status = _E_PVP_CLOSING_;
		mPVPData[aryPos].timer = 0;

		sSecionInfo* pSection = PVPMANAGER->GetSectionInfo( aryPos );
		if( pSection == NULL )
			return;
		
		/// 입장 가능한 유저들에게 전장입장 시킴
		for( unsigned long i = 0 ; i < pList->mCount ; ++i )
		{
			if( pList->mList[i].mCharacterChannel != (short)mChannelNum )
				continue;

			if( pList->mList[i].mSelection == false )
				continue;

			cPlayer* pPlayer = OBJECTMANAGER->GetPlayer( pList->mList[i].mPlayerIdx );
			if( pPlayer == NULL )
				continue;

			if( pPlayer->GetGameIn() == false )
				continue;

			if( pPlayer->GetPvPJoinAryPos() != aryPos )
				continue;

			pPlayer->ChannelMovePvPWait( pMBMsg->cid, i * 10 );

			/// ready 목록에서 지움
			PVPMANAGER->DelReadyPlayer( pPlayer );

			SKILLMANAGER->AddInfluence( pPlayer->GetObject(), pPlayer->GetObject(), PVP_DM_WAIT_NODAMAGE_INFLUENCE,
				0, true );
		}

		/// 입장 불가능인 유저들에게 알림
		PVPMANAGER->SendNotPvPIn( aryPos );
	}
}

void cGameProcess::PvPGMChannelRes( MB_SYN_PVPGM_CHANNELMOVE* pMBMsg )
{
	if ( mServerNum == (DWORD)HIWORD( pMBMsg->cid ) && pMBMsg->mapType == mChannelNum &&
		MAP_MIN <= (DWORD)LOWORD( pMBMsg->cid ) && (DWORD)LOWORD( pMBMsg->cid ) <=  MAP_MIN )
	{
		cCSLock lock( &mCs );

		PVPMANAGER->AddGmPlayer( pMBMsg->playerIdx );
	}
}

// ThemeStatus Method
void cGameProcess::ThemeStatus( long cid, BYTE status, unsigned short mapNum )
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && 
		( ( MAP_THEME_MIN <= (DWORD)LOWORD( cid ) && (DWORD)LOWORD( cid ) <= MAP_THEME_MAX ) ||
		( MAP_ETC_MIN <= (DWORD)LOWORD( cid ) && (DWORD)LOWORD( cid ) <= MAP_ETC_MAX ) ) && 
		MAP_MIN <= mChannelNum && mChannelNum <= MAP_MAX )
	{
		sThemeData* pData = GetThemeData( mapNum );
		if( pData != NULL )
		{
			cCSLock lock( &mCs );
			pData->cid    = cid;
			pData->status = (THEME_STATUS)status;
		}
	}
}

void cGameProcess::FriendSync( long cid, char* msg, int /*len*/ )
{
	if( mServerNum == (DWORD)HIWORD( cid ) && mChannelNum != (DWORD)LOWORD( cid ) )
	{
		cCSLock		lock( &mCs );
		MSGROOT*	tempMsg = (MSGROOT*)msg;

		switch( tempMsg->Protocol )
		{
		case NM_FRIENDPOST_CONNECT_SYN:
			{
				MSG_SYN_POSTFRIEND_CONNECT* pmsg = (MSG_SYN_POSTFRIEND_CONNECT*)msg;

				long* friendList = pmsg->friendlist;

				for( long i = 0; i < 50; ++i, ++friendList )
				{
					if( *friendList > 0 )
					{
						cPlayer* player = OBJECTMANAGER->GetPlayer( *friendList );
						if( player != NULL )
						{
							player->SyncFriendConnect( pmsg->characterIdx, pmsg->level, pmsg->job, pmsg->mapNum, pmsg->connect, (DWORD)LOWORD( cid ) );
						}
					}
				}
			}
			break;
		}
	}
}

// GuildSync Method
void cGameProcess::GuildSync(long cid, char* msg, int /*len*/)
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && mChannelNum != (DWORD)LOWORD( cid ) )
	{
		cCSLock  lock( &mCs );
		MSGROOT* tempMsg = (MSGROOT*)msg;
		cGuild*	   guild = NULL;
		DWORD  tickCount = GetTickCount( );
		
		switch( tempMsg->Protocol )
		{
		case NM_GUILDPOST_CREATE_SYN:
			{
				MSG_SYN_POSTGUILD_CREATE* pmsg = (MSG_SYN_POSTGUILD_CREATE*)msg;
				mpGuildManager->SynAddGuild( (void*)pmsg, (DWORD)LOWORD( cid ) );
			}
			break;
		case NM_GUILDPOST_DELETE_SYN:
			{
				MSG_SYN_POSTGUILD_DELETE* pmsg = (MSG_SYN_POSTGUILD_DELETE*)msg;
				mpGuildManager->DeleteGuild( pmsg->idx );
			}
			break;
		case NM_GUILDPOST_USERADD_SYN:
			{
				MSG_SYN_POSTGUILD_USERADD* pmsg = (MSG_SYN_POSTGUILD_USERADD*)msg;
				mpGuildManager->SynAddGuildUser( (void*)pmsg, (DWORD)LOWORD( cid ) );
			}
			break;
		case NM_GUILDPOST_USERCUT_SYN:
			{
				MSG_SYN_POSTGUILD_USERCUT* pmsg = (MSG_SYN_POSTGUILD_USERCUT*)msg;
				mpGuildManager->SynCutGuildUser( (void*)pmsg );
			}
			break;
		case NM_GUILDPOST_USEROUT_SYN:
			{
				MSG_SYN_POSTGUILD_USEROUT* pmsg = (MSG_SYN_POSTGUILD_USEROUT*)msg;
				mpGuildManager->SynOutGuildUser( (void*)pmsg );
			}
			break;
		case NM_GUILDPOST_GIVEPOSITION_SYN:
			{
				MSG_SYN_POSTGUILD_GIVEPOSITION* pmsg = (MSG_SYN_POSTGUILD_GIVEPOSITION*)msg;
				cGuild* guild = mpGuildManager->GetGuild( pmsg->idx );
				mpGuildManager->SendGuildChangePosition( guild, pmsg->characterIdx, pmsg->position );
			}
			break;
		case NM_GUILDPOST_UPDATEMARK_SYN:
			{
				MSG_SYN_POSTGUILD_UPDATEMARK* pmsg = (MSG_SYN_POSTGUILD_UPDATEMARK*)msg;
				mpGuildManager->SynUpdateMark( (void*)pmsg );
			}
			break;
		case NM_GUILDPOST_NOTICE_SYN:
			{
				MSG_SYN_POSTGUILD_NOTICE* pmsg = (MSG_SYN_POSTGUILD_NOTICE*)msg;
				mpGuildManager->SendGuildNotice( pmsg->idx, pmsg->notice );
			}
			break;
		case NM_GUILDPOST_CHAT_SYN:
			if ( mType != _E_ST_ID_PVP_ )
			{
				MSG_SYN_POSTGUILD_CHAT* pmsg = (MSG_SYN_POSTGUILD_CHAT*)msg;
				mpGuildManager->SynGuildChat( (void*)pmsg );
			}
			break;
		case NM_GUILDPOST_CONNECT_SYN:
			{
				MSG_SYN_POSTGUILD_CONNECT* pmsg = (MSG_SYN_POSTGUILD_CONNECT*)msg;

				// 자신의 서버에 접속 / 비접속 알림
				mpGuildManager->SendGuildUserConnect( pmsg->idx, pmsg->characterIdx, pmsg->connect, pmsg->channelNum, pmsg->sendConnect );
			}
			break;
		case NM_GUILDPOST_TTL_SYN:
			{
				MSG_SYN_POSTGUILD_TTL* pmsg = (MSG_SYN_POSTGUILD_TTL*)msg;

				// 길드원의 접속 유지 정보 받음
				guild = mpGuildManager->GetGuild( pmsg->idx );
				if( guild != NULL )
					guild->UpdateInfo( &pmsg->data, tickCount );
			}
			break;
		}
	}
}

void cGameProcess::MonsterSync( long cid, char* msg, int /*len*/ )
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && mChannelNum != (DWORD)LOWORD( cid ) )
	{
		cCSLock lock( &mCs );
		MSGROOT* tempMsg = (MSGROOT*)msg;
		switch( tempMsg->Protocol )
		{
		case NM_MONSTERPOST_SWITCHREGEN_SYN:
			{
				MSG_SYN_POSTMONSTER_SWITCHREGEN* pStart = (MSG_SYN_POSTMONSTER_SWITCHREGEN*)msg;
				AIMANAGER->SwitchRegen( pStart->mLevelMode, (unsigned short)LOWORD( cid ), pStart->mSwitchRegen );
			}
			break;
		}
	}
}

void cGameProcess::ChatSync( long cid, char* msg, int /*len*/ )
{
	if ( mServerNum == (DWORD)HIWORD( cid ) && mChannelNum != (DWORD)LOWORD( cid ) )
	{
		cCSLock  lock( &mCs );
		MSGROOT* tempMsg = (MSGROOT*)msg;

		switch( tempMsg->Protocol )
		{
		case NM_CHATPOST_WHISPER_SYN:	/// 다른 채널로부터 귓말 옴
			{
				/// 내 채널에 온 정보면 처리
				MSG_SYN_POSTCHAT_WHISPER* pmsg = (MSG_SYN_POSTCHAT_WHISPER*)msg;

				cPlayer* findPlayer = mpObjectManager->GetPlayer( pmsg->recvName );
				if ( findPlayer != NULL  )
				{
					cPlayer* gameInUser = mpGridManager->GetPlayer( findPlayer->GetObjectID() );
					if ( gameInUser != NULL )
					{
						sGameOption* toOption1 = gameInUser->GetOptionData();
						if( toOption1->option1.rejectionWhisper == false )	
						{
							PostChatWhisperResult( ERROR_CHAT_WHISPER_SUCCESS, 
                                                   pmsg->senderIndex, 
                                                   pmsg->recvName, 
                                                   gameInUser->GetJob(), 
                                                   pmsg->message, 
                                                   &pmsg->inventory );

							/// 해당유저에게 귓말 전송
							HANDLE                handle  = NULL;
							MSG_SYN_CHAT_WHISPER* sendMsg = (MSG_SYN_CHAT_WHISPER*)GetMsgRoot( &handle
																							  ,gameInUser->GetConnectionIdx( )
																							  ,NM_CHAT
																							  ,NM_CHAT_WHISPER_SYN );
							if ( sendMsg != NULL )
							{
								sendMsg->inventory  = pmsg->inventory;				// 보낸 인벤토리 정보
                                sendMsg->Job        = pmsg->senderJob;              // 보낸 캐릭터 직업
                                wcscpy( sendMsg->Name,    pmsg->senderName );		// 보낸 캐릭터 이름
								wcscpy( sendMsg->Message, pmsg->message );			// 보낸 메시지

								SendMsgRoot( handle, sendMsg->GetMsgLength( ) );	// 발송.
							}
						}
						else
						{
							/// 거부로인한 귓말전송 불가
							PostChatWhisperResult( ERROR_CHAT_WHISPER_OPTIONREFUSE, 
                                                   pmsg->senderIndex, 
                                                   pmsg->recvName, 
                                                   gameInUser->GetJob(), 
                                                   pmsg->message, 
                                                   &pmsg->inventory );
						}
					}
				}
			}
			break;
		case NM_CHATPOST_WHISPER_RESULTSYN:		/// 보낸 귓말에 대한 결과를 받음
			{
				MSG_SYN_POSTCHAT_WHISPERRESULT* pmsg = (MSG_SYN_POSTCHAT_WHISPERRESULT*)msg;
				/// 귓말을 보냈었던 유저에게 결과를 알려줌
				cPlayer* sendPlayer = mpGridManager->GetPlayer( pmsg->senderIndex );
				if ( sendPlayer != NULL )
				{
					HANDLE                handle  = NULL;
					MSG_RES_CHAT_WHISPER* sendMsg = (MSG_RES_CHAT_WHISPER*)GetMsgRoot( &handle
																					  ,sendPlayer->GetConnectionIdx()
																					  ,NM_CHAT
																					  ,NM_CHAT_WHISPER_RES );
					if ( sendMsg != NULL )
					{
						sendMsg->ErrorCode = pmsg->error;

						if ( pmsg->error == ERROR_CHAT_WHISPER_SUCCESS )
						{
							sendMsg->inventory  = pmsg->inventory;					// 보낸 인벤토리.
                            sendMsg->Job        = pmsg->recvJob;                    // 받는 캐릭터직업.

							wcscpy( sendMsg->Name,    pmsg->recvName );				// 받는 캐릭터이름.
							wcscpy( sendMsg->Message, pmsg->message  );				// 보낸 메시지.
						}

						SendMsgRoot( handle, sendMsg->GetMsgLength( ) );			// 발송.
					}
				}
			}
			break;
		}
	}
}

// PostReceived Method
void cGameProcess::PostReceived(MB_SYN_POST_RECEIVED* msgBuf)
{
	if ( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
	{
		cCSLock  lock( &mCs );
		cPlayer* player = mpGridManager->GetPlayer( msgBuf->characterIdx );

		if ( player != NULL )
		{
			HANDLE                      handle  = NULL;
			MSG_SYN_CHAT_POST_RECEIVED* sendMsg = (MSG_SYN_CHAT_POST_RECEIVED*)GetMsgRoot( &handle
																						  ,player->GetConnectionIdx()
																						  ,NM_CHAT
																						  ,NM_CHAT_POST_RECEIVED_SYN );
			if ( sendMsg != NULL )
			{
				SendMsgRoot( handle, sizeof(MSG_SYN_CHAT_POST_RECEIVED) ); // 발송.
			}
		}
	}
}

// PartySync Method
void cGameProcess::PartySync(MB_SYN_PARTY* msgBuf)
{
	if ( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
	{
		cCSLock lock( &mCs );
		DWORD   tickCount = GetTickCount( );

		PARTY_DATA_SYNC* sync = (PARTY_DATA_SYNC*)msgBuf->msg;
		cParty*          party;

		switch ( sync->protocol )
		{
		case PSP_SYN_PARTY:
			party = mpPartyManager->GetParty( sync->synParty.partyIdx );
			if ( party == NULL )
			{
				party = mpPartyManager->AddParty( sync->synParty.partyIdx, sync->synParty.player[ 0 ].userIndex, sync->synParty.player[ 1 ].userIndex );
				if ( party != NULL )
				{
					party->SetUserData( &sync->synParty.player[ 0 ], tickCount );
					party->SetUserData( &sync->synParty.player[ 1 ], tickCount );

					for ( int i = 2; (sync->synParty.player[ i ].userIndex > 0) && (i < MAX_PARTYUSER); i++ )
					{
						party->AddUser( sync->synParty.player[ i ].userIndex );
						party->SetUserData( &sync->synParty.player[ i ], tickCount );
					}

					party->SetLeader( sync->synParty.leaderIdx );
				}
			}
			else
			{
				for ( int i = 0; (sync->synParty.player[ i ].userIndex > 0) && (i < MAX_PARTYUSER); i++ )
				{
					cPlayer* player = mpObjectManager->GetPlayer( sync->synParty.player[ i ].userIndex );
					if ( player == NULL )
					{
						party->SetUserData( &sync->synParty.player[ i ], tickCount );
					}
				}
			}
			break;
		case PSP_ASK_PARTY:
			{
				cPlayer* player = mpObjectManager->GetPlayer( sync->askParty.playerName );
				if ( player != NULL )
				{
					int retvalue = mpPartyManager->CheckPlayer( player );

					if ( retvalue == ERROR_PARTY_ADD_SUCCESS )
					{
						player->SetPartyState( ePARTY_RECVJOIN );
						player->StartRequestRejection( eREQREJCT_PARTY );

						HANDLE		        handle = NULL;
						MSG_SYN_PARTY_ADD*	synMsg = (MSG_SYN_PARTY_ADD*)GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_SYN );
						if ( synMsg != NULL )
						{
							wcscpy( synMsg->Name, sync->askParty.leader.name );
							SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_ADD) );
						}

						// 신청등록.
						PartyJoin* join = mpPartyManager->GetJoin( sync->askParty.playerName );
						if ( join != NULL )
						{
							join->partyIdx  = sync->askParty.partyIdx;
							join->playerDat = sync->askParty.leader;
						}
					}
					else
					{
						mpPartyManager->PostRepParty( sync->askParty.leader.userIndex, sync->askParty.playerName, retvalue, 0 );
					}
				}
			}
			break;
		case PSP_REP_PARTY:
			{
				cPlayer* player = mpObjectManager->GetPlayer( sync->repParty.leaderIdx );
				if ( player != NULL )
				{
					ePLAYER_PARTYSTATE state = (player->GetPartyIndex( ) > 0) ? ePARTY_COMPLETE : ePARTY_NONE;
					player->SetPartyState( state );

					if ( sync->repParty.error != 0 )
					{
						SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, sync->repParty.error );
					}
					else switch ( sync->repParty.replay )
					{
					case PARTY_REP_REJECT:
						SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_REFUSE );
						break;
					case PARTY_REP_REJECTTIME:
						SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_REFUSETIME );
						break;
					case PARTY_REP_ACCEPT:
						break;
					}

					PartyJoin* join = mpPartyManager->SearchJoin( sync->repParty.playerName );
					if ( join != NULL )
					{
						mpPartyManager->ReleaseJoin( join );	// 등록해지.
					}
				}
			}
			break;
		case PSP_ADD_PARTY:
			party = mpPartyManager->GetParty( sync->addParty.partyIdx );
			if ( party == NULL )
			{
				party = mpPartyManager->AddParty( sync->addParty.partyIdx, sync->addParty.leader.userIndex, sync->addParty.player.userIndex );
				if ( party != NULL )
				{
					cPlayer* leader = mpObjectManager->GetPlayer( sync->addParty.leader.userIndex );
					if ( leader != NULL )
					{
						leader->SetPartyIndex( party->GetIndex( ) );
						leader->SetPartyState( ePARTY_COMPLETE );
						party->SetUserData( leader, 0 );

						SendMsgError( leader->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_CREATE_SUCCESS );
					}
					else
						party->SetUserData( &sync->addParty.leader, tickCount );

					cPlayer* player = mpObjectManager->GetPlayer( sync->addParty.player.userIndex );
					if ( player != NULL )
					{
						player->SetPartyIndex( party->GetIndex( ) );
						player->SetPartyState( ePARTY_COMPLETE );
						party->SetUserData( player, 0 );

						SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_CREATE_SUCCESS );
					}
					else
						party->SetUserData( &sync->addParty.player, tickCount );

					if ( leader != NULL ) mpPartyManager->ResPartyList( leader, party );
					if ( player != NULL ) mpPartyManager->ResPartyList( player, party );
				}
			}
			break;
		case PSP_DEL_PARTY:
			party = mpPartyManager->GetParty( sync->delParty.partyIdx );
			if ( party != NULL )
			{
				party->SendPartyDeleteSyn( );
				mpPartyManager->DelParty( &party );
			}
			break;
		case PSP_CHT_PARTY:
			party = mpPartyManager->GetParty( sync->chtParty.partyIdx );
			if ( party != NULL )
			{
				party->SendChat( &sync->chtParty.inventory, 
                                  sync->chtParty.Name, 
                                  sync->chtParty.Message, 
                                  sync->chtParty.playerIdx,
                                  sync->chtParty.job );
			}
			break;
		case PSP_ADD_USER:
			party = mpPartyManager->GetParty( sync->addUser.partyIdx );
			if ( party != NULL )
			{
				if ( party->IsUser( sync->addUser.player.userIndex ) == false )
				{
					party->AddUser( sync->addUser.player.userIndex );
					party->SetUserData( &sync->addUser.player, tickCount );
					party->SendPartyAddoneSyn( &sync->addUser.player );
				}
			}
			break;
		case PSP_DEL_USER:
			party = mpPartyManager->GetParty( sync->delUser.partyIdx );
			if ( party != NULL )
			{
				if ( party->IsUser( sync->delUser.playerIdx ) )
				{
					if ( party->GetLeader( ) != sync->delUser.leaderIdx )
					{
						party->SetLeader( sync->delUser.leaderIdx );
						party->SendPartyLeaderSyn( sync->delUser.leaderIdx );
					}

					cPlayer* player = mpObjectManager->GetPlayer( sync->delUser.playerIdx );

					if ( player != NULL )
					{
						player->SetPartyIndex( 0 );
						player->SetPartyState( ePARTY_NONE );

						// 추방된 유저에게 보냄
						SendMsgRoot( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_CUT_SYN );
					}

					party->DeleteUser( sync->delUser.playerIdx );
					party->SendPartyOutSyn( sync->delUser.playerIdx );
				}
			}
			break;
		case PSP_OUT_USER:
			party = mpPartyManager->GetParty( sync->outUser.partyIdx );
			if ( party != NULL )
			{
				if ( party->IsUser( sync->outUser.playerIdx ) )
				{
					cPlayer* player = mpObjectManager->GetPlayer( sync->outUser.playerIdx );

					if ( player != NULL )
					{
						player->SetPartyIndex( 0 );
						player->SetPartyState( ePARTY_NONE );

						// 추방된 유저에게 보냄
						SendMsgRoot( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_CUT_SYN );
					}

					party->DeleteUser( sync->outUser.playerIdx );
					party->SendPartyThrowSyn( sync->outUser.playerIdx );
				}
			}
			break;
		case PSP_SYN_LEADER:
			party = mpPartyManager->GetParty( sync->synLeader.partyIdx );
			if ( party != NULL )
			{
				if ( party->GetLeader( ) != sync->synLeader.leaderIdx )
				{
					party->SetLeader( sync->synLeader.leaderIdx );
					party->SendPartyLeaderSyn( sync->synLeader.leaderIdx );
				}
			}
			break;
		case PSP_SYN_DIVIDE_TYPE:
			party = mpPartyManager->GetParty( sync->synDivideType.partyIdx );
			if ( party != NULL )
			{
				if ( party->GetDivideType( ) != (PARTY_DIVIDE_TYPE)sync->synDivideType.divideType )
				{
					party->SetDivideType( (PARTY_DIVIDE_TYPE)sync->synDivideType.divideType );
					party->SendPartyDivideType( sync->synDivideType.divideType );
				}
			}
			break;
		case PSP_SYN_LEVEL_DATA:
			party = mpPartyManager->GetParty( sync->synLevelData.partyIdx );
			if ( party != NULL )
			{
				if ( party->IsUser( sync->synLevelData.player.userIndex ) )
				{
					party->SetUserData( &sync->synLevelData.player, tickCount );
					party->SendPartyDataLevelSyn( sync->synLevelData.player.userIndex, sync->synLevelData.player.level );
				}
			}
			break;
		case PSP_SYN_MAP_DATA:
			party = mpPartyManager->GetParty( sync->synMapData.partyIdx );
			if ( party != NULL )
			{
				if ( party->IsUser( sync->synMapData.player.userIndex ) )
				{
					party->SetUserData( &sync->synMapData.player, tickCount );
					party->SendPartyMapchangeSyn( sync->synMapData.player.userIndex, sync->synMapData.player.mapNum, sync->synMapData.player.roomNum, sync->synMapData.player.channelNum );
				}
			}
			break;
		case PSP_SYN_USER:
			if ( sync->synUser.gotoCID == GetCID( ) )
			{
				mpPartyManager->DelRestore( sync->synUser.playerIdx );
				mpPartyManager->AddRestore( sync->synUser.playerIdx, sync->synUser.partyIdx );
			}
			break;
		case PSP_SYN_DICE_START:
			party = mpPartyManager->GetParty( sync->synDiceStart.partyIdx );
			if ( party != NULL )
			{
				party->DiceStart( );
			}
			break;
		case PSP_SYN_DICE_RESULT:
			party = mpPartyManager->GetParty( sync->synDiceResult.partyIdx );
			if ( party != NULL )
			{
				party->DiceRollReq( sync->synDiceResult.playerIdx, sync->synDiceResult.value );
			}
			break;
		}
	}
}

void cGameProcess::EnhancedSync( MB_SYN_ENHANCED_RESULT* msgBuf )
{
    if ( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
    {
		cCSLock           lock( &mCs );

        MSG_SYN_CHAT_SYSTEM sync;
        memset( &sync, 0, sizeof(sync) );

        sync.Category   = NM_CHAT;
        sync.Protocol   = NM_CHAT_ENHANCED_SYN;

        memcpy( &sync.inventory, msgBuf->msg, sizeof(sInventory) );

        if ( wcslen(msgBuf->name) > 0 )
            wcscpy( sync.characterName, msgBuf->name );

        OBJECTMANAGER->SendNotice( (char*)&sync, sizeof(MSG_SYN_CHAT_SYSTEM) );
    }
}

void cGameProcess::UniqueItemSync( MB_SYN_UNIQUEITEM_GET* msgBuf )
{
	// 동일 서버군
	if( mServerNum == (DWORD)HIWORD( msgBuf->cid ) )
	{
		cCSLock           lock( &mCs );

		MSG_SYN_CHAT_SYSTEM sync;
		memset( &sync, 0, sizeof(sync) );

		sync.Category   = NM_CHAT;
		sync.Protocol   = NM_CHAT_UNIQUEITEM_SYN;

		memcpy( &sync.inventory, msgBuf->msg, sizeof(sInventory) );

		if ( wcslen(msgBuf->name) > 0 )
			wcscpy( sync.characterName, msgBuf->name );

		OBJECTMANAGER->SendNotice( (char*)&sync, sizeof(MSG_SYN_CHAT_SYSTEM) );
	}
}

void cGameProcess::NoChatSync( MB_SYN_NO_CHAT* msgBuf )
{
	// 동일 서버군
	if( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
	{
		cCSLock           lock( &mCs );

		cPlayer* player = OBJECTMANAGER->GetPlayer( msgBuf->characterIdx );
		if ( player != NULL )
		{
			if ( msgBuf->validThru != 0 )
			{
				if ( player->SetBlock( true, msgBuf->validThru ) == true )
					SendMsgRoot( player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_NO_CHAT_SYN );
			}
			else
			{
				if ( player->SetBlock( false, 0 ) == true )
					SendMsgRoot( player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_ALLOW_CHAT_SYN );
			}
		}
	}
}

void cGameProcess::AddGMEventSync( MB_SYN_ADD_GM_EVENT* msgBuf )
{
	// 동일 서버군 필터링
	if( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
	{
		cCSLock           lock( &mCs );
		
		///적용된 효과를 추가
		sGMEventInfo eventInfo;
		eventInfo.index		   = msgBuf->index;
		eventInfo.influenceIdx = msgBuf->influenceIdx;
		eventInfo.startTime    = msgBuf->startTime;
		eventInfo.validThru    = msgBuf->validThru;

		mGMEventList.PushBack( eventInfo );

		OBJECTMANAGER->AddGMEvent( eventInfo );
	}
}

void cGameProcess::DelGMEventSync( MB_SYN_DEL_GM_EVENT* msgBuf )
{
	// 동일 서버군 필터링
	if( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
	{
		cCSLock           lock( &mCs );

		///적용된 효과를 삭제
		cGMEventList::cIterator pos = mGMEventList.Begin();
		while( pos != mGMEventList.End() )
		{
			sGMEventInfo& eventInfo = (*pos);
			if( eventInfo.index == msgBuf->index && eventInfo.influenceIdx == msgBuf->influenceIdx )
			{
				pos = mGMEventList.Erase( pos );
				continue;
			}
			++pos;
		}

		///해당 효과를 삭제
		OBJECTMANAGER->DelGMEvent( msgBuf->index, msgBuf->influenceIdx );
	}
}

void cGameProcess::GMKickEventSync( MB_SYN_GM_KICK_EVENT* msgBuf )
{
	// 동일 서버군 필터링
	if( mServerNum == (DWORD)HIWORD( msgBuf->cid ) && mChannelNum != (DWORD)LOWORD( msgBuf->cid ) )
	{
		cCSLock	 lock( &mCs );
		
		cPlayer* player = OBJECTMANAGER->GetPlayer( msgBuf->characterIdx );
		if ( player != NULL )
		{
			SendMsgRoot( player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_KICK_RES );
			CloseCID( player->GetConnectionIdx() );
		}
	}
}

void cGameProcess::ItemDropLimitUpdate( MB_SYN_DROPITEM_UPDATE* msgBuf )
{
	// 동일 서버군 필터링
	if( mServerNum == (DWORD)HIWORD( msgBuf->mCid ) && mChannelNum != (DWORD)LOWORD( msgBuf->mCid ) )
	{
		cCSLock lock( &mCs );

		if( mpDrop != NULL )
		{
			sDropLimit* pDropLimit = mpDrop->GetDropMonCantTime( msgBuf->mItemIdx );
			if( pDropLimit != NULL )
			{
				pDropLimit->mEndTime = msgBuf->mDropEndTime;
				pDropLimit->mIsDrop = true;
			}
		}
	}
}

void cGameProcess::Pg_Game_Start(char* Session, char* User_ID, char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain)
{
	cCSLock           lock( &mCs );
	long              cid           = atoi( User_ID );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( cid );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		clientInfo = (ClientInfo*)socketContext->buffer;
		if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			clientInfo->complete.pgBilling = true;
			memcpy( clientInfo->status, User_Status, sizeof(clientInfo->status) );

			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
			{
				hero->Pg_Game_Start( User_Status, Bill_Method, Bill_Expire, Bill_Remain );
			}
		}
	}
}

void cGameProcess::Pg_User_Alert(long Packet_Result, char* Session, char* User_ID, char* Bill_Method, long Bill_Remain)
{
	cCSLock           lock( &mCs );
	long              cid           = atoi( User_ID );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( cid );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		clientInfo = (ClientInfo*)socketContext->buffer;
		clientInfo->complete.pgBilling = (Packet_Result == -3 || Packet_Result == 1);

		if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
			{
				hero->Pg_User_Alert( Packet_Result, Bill_Method, Bill_Remain );
			}
		}
	}
}

void cGameProcess::Pg_User_Sync(char* Session, char* User_ID, char* User_Status)
{
	cCSLock           lock( &mCs );
	long              cid           = atoi( User_ID );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( cid );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		clientInfo = (ClientInfo*)socketContext->buffer;
		if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			memcpy( clientInfo->status, User_Status, sizeof(clientInfo->status) );

			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );

			char user_id[17];
			char user_ip[24];
			sprintf( user_id, "%d", clientInfo->memberIdx );
			sprintf( user_ip
					,"%d.%d.%d.%d"
					,socketContext->addr.sin_addr.S_un.S_un_b.s_b1
					,socketContext->addr.sin_addr.S_un.S_un_b.s_b2
					,socketContext->addr.sin_addr.S_un.S_un_b.s_b3
					,socketContext->addr.sin_addr.S_un.S_un_b.s_b4 );
			PGBILLING->UserSync( (hero != NULL), clientInfo->session, user_id, user_ip, clientInfo->status );
			//inet_ntoa( )
		}
	}
}

// Pg_Billing_Auth Method
void cGameProcess::Pg_Billing_Auth(char* Session, char* User_ID, char* User_IP, char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain)
{
	cCSLock           lock( &mCs );
	long              cid           = atoi( User_ID );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( cid );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		User_IP;
		clientInfo = (ClientInfo*)socketContext->buffer;
		if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			memcpy( clientInfo->status, User_Status, sizeof(clientInfo->status) );

			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
			{
				hero->Pg_Billing_Auth( User_Status, Bill_Method, Bill_Expire, Bill_Remain );
			}
		}
	}
}

// Pg_Client_Move Method
void cGameProcess::Pg_Client_Move(char* Game_Server, char* Session, char* User_ID, char* User_IP, char* User_Status, long Game_No)
{
	cCSLock           lock( &mCs );
	long              cid           = atoi( User_ID );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( cid );
	ClientInfo*       clientInfo;

	Game_Server;
	if ( socketContext != NULL )
	{
		User_IP;
		Game_No;
		clientInfo = (ClientInfo*)socketContext->buffer;
		if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
			{
				hero->Pg_Client_Move(User_Status);
			}
		}
	}
}

// Pg_Close Method
void cGameProcess::Pg_Close( )
{
	if ( mSocketContextPool != NULL )
	{
		cCSLock           lock( &mCs );
		PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
		ClientInfo*       clientInfo;

		while ( socketContext != NULL )
		{
			clientInfo = (ClientInfo*)socketContext->buffer;

			if ( clientInfo->complete.pgBilling )
			{
				clientInfo->complete.pgBilling = false;

				cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				if ( hero != NULL )
				{
					hero->Pg_Close( );
				}
			}

			socketContext = socketContext->next;
		}
	}
}


void cGameProcess::BP_User_Auth( unsigned long memberIdx )
{
	cCSLock           lock( &mCs );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( memberIdx );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		clientInfo = (ClientInfo*)socketContext->buffer;
//		if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			clientInfo->complete.pgBilling = true;
//			memcpy( clientInfo->status, User_Status, sizeof(clientInfo->status) );

			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
			{
				clientInfo->mBPNextCheckTime = GetAccumTime() + 10 * SECOND;
				hero->BP_Start();

				PostServerEvent("cGameProcess::BP_User_Auth [%d,%d][%d.%d.%d.%d]", memberIdx, clientInfo->characterIdx, 
					socketContext->addr.sin_addr.S_un.S_un_b.s_b1,
					socketContext->addr.sin_addr.S_un.S_un_b.s_b2,
					socketContext->addr.sin_addr.S_un.S_un_b.s_b3,
					socketContext->addr.sin_addr.S_un.S_un_b.s_b4 );
			}
		}
	}
}


void cGameProcess::BP_User_End( unsigned long memberIdx )
{
	cCSLock           lock( &mCs );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( memberIdx );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		clientInfo = (ClientInfo*)socketContext->buffer;
		/// clientInfo->complete.pgBilling = (Packet_Result == -3 || Packet_Result == 1);
		clientInfo->complete.pgBilling = false;

		//if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{
			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
			{
				PostServerEvent("cGameProcess::BP_User_End [%d,%d][%d.%d.%d.%d]", memberIdx, clientInfo->characterIdx, 
					socketContext->addr.sin_addr.S_un.S_un_b.s_b1,
					socketContext->addr.sin_addr.S_un.S_un_b.s_b2,
					socketContext->addr.sin_addr.S_un.S_un_b.s_b3,
					socketContext->addr.sin_addr.S_un.S_un_b.s_b4 );
				hero->BP_End();
			}
		}
	}
}


void cGameProcess::BP_Item_Give( unsigned long memberIdx )
{
	cCSLock           lock( &mCs );
	PerSocketContext* socketContext = mSocketContextPool->GetCID( memberIdx );
	ClientInfo*       clientInfo;

	if ( socketContext != NULL )
	{
		clientInfo = (ClientInfo*)socketContext->buffer;

		PostServerEvent("cGameProcess::BP_Item_Give [%d,%d]", memberIdx, clientInfo->characterIdx );
		//if ( memcmp( Session, clientInfo->session, sizeof(clientInfo->session) ) == 0 )
		{			
			cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
			if ( hero != NULL )
				hero->BP_ItemGive();
		}
	}
}


// Pg_Close Method
void cGameProcess::BP_Close( )
{
	if ( mSocketContextPool != NULL )
	{
		cCSLock           lock( &mCs );
		PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
		ClientInfo*       clientInfo;

		while ( socketContext != NULL )
		{
			clientInfo = (ClientInfo*)socketContext->buffer;

			if ( clientInfo->complete.pgBilling )
			{
				clientInfo->complete.pgBilling = false;

				cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				if ( hero != NULL )
				{
					hero->BP_End( );
				}
			}

			socketContext = socketContext->next;
		}
	}
}

// IsCharacterName Method
bool cGameProcess::IsCharacterName(wchar_t* name, int len)
{
	wchar_t code     = 0;
	bool    retvalue = true;

	// MIN_NAME_SIZE
	if ( wcslen( name ) < 2 )
		return false;

	for ( int i = 0; i < len && (*name) != 0 && retvalue == true; i++, name++ )
	{
		code = (*name);

		// 숫자
		if (code >= 0x0030 && code <= 0x0039)
			retvalue = true;
		// 영문
		else if ( (code >= 0x0041 && code <= 0x005A) || (code >= 0x0061 && code <= 0x007A) )
			retvalue = true;
		// 한글
		//else if (code >= 0XAC00 && code <= 0XD7A3)
		//	retvalue = true;
		// 일어-HIRAGANA
		else if (code >= 0X3040 && code <= 0X309F)
			return true;
		// 일어-KATAKANA
		else if (code >= 0X30A0 && code <= 0X30FF)
			return true;
		// CJK Unified Ideographs (한중일통합)
		else if (code >= 0X4E00 && code <= 0X9FCF)
			return true;
		/*-- 중문
		else if (code >= 0X4E00 && code <= 0X9FBB)
			retvalue = true;
		*/
		// 외-오류
		else
			retvalue = false;
	}

	return retvalue;
}

// SendChannelList Method
bool cGameProcess::SendChannelList(PerSocketContext* perSocketContext)
{
	HANDLE               handle  = NULL;
	MSG_RES_CHANNELLIST* sendMsg = (MSG_RES_CHANNELLIST*)GetMsgRoot( &handle, perSocketContext, NM_USER, NM_USER_CHANNEL_LIST_RES );
	if ( sendMsg != NULL )
	{
		unsigned long  length   = sizeof(MSG_RES_CHANNELLIST) - sizeof(sendMsg->Channels);
		unsigned long& rowCount = sendMsg->RowCount;
		ChannelList*   channels = mChannelList;

		sendMsg->ChannelNo = mChannelNum;
		for ( u_long i = MAP_MIN; i <= MAP_MAX && i < MAX_CHANNEL; i++, channels++ )
		{
			if ( channels->used )
			{
				sendMsg->Channels[ rowCount ].NumOfChannel = i;
				sendMsg->Channels[ rowCount ].Status       = channels->status;
				rowCount += 1;
			}
		}
		length += (rowCount * sizeof(sendMsg->Channels));
		return SendMsgRoot( handle, length );
	}
	return false;
}

// BatchComplete Method
bool cGameProcess::BatchComplete(DWORD currentTick, PerSocketContext* perSocketContext, void* ptr)
{
	ClientInfo*   clientInfo = (ClientInfo*)perSocketContext->buffer;
	unsigned long heroIdx    = clientInfo->characterIdx;
	cPlayer*      hero       = mpObjectManager->GetPlayer( heroIdx );

	Packet*       packet     = (Packet*)ptr;
	u_long        packetLen  = packet->tlen - packet->hlen;
	MSGROOT*      msgRoot    = (MSGROOT*)((char*)ptr + packet->hlen);

	mLastCategory   = msgRoot->Category;
	mLastProtocol   = msgRoot->Protocol;
	mLastPacketSize = packetLen;

	Verbose->BatchComplete( &mCs, ptr );

	/*-- 닫기중 통신 무시 --*/
	if ( perSocketContext->status.closeWait )
		return true;

	/*-- 채널간 이동시 무시 --*/
	if ( clientInfo->request.gotoInstantDungeon || clientInfo->request.returnToMap || clientInfo->complete.gameToGame )
		return true;

	switch ( msgRoot->Category )
	{
	case NM_USER:
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_USER_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_USER::PROTOCOL(=%d):INETNUM(=%s).", msgRoot->Protocol, inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}

		switch ( msgRoot->Protocol )
		{
		case NM_USER_GAMESRV_REQ:
			if ( packetLen != sizeof(MSG_REQ_GAMESRV) )
				return false;
			if ( !clientInfo->commonDb.check && !clientInfo->requestData && !clientInfo->completeData )
			{
				MSG_REQ_GAMESRV* msg = (MSG_REQ_GAMESRV*)msgRoot;

				HANDLE        handle      = NULL;
				MEMBER_CHECK* memberCheck = (MEMBER_CHECK*)GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_CHECK );

				memberCheck->loginIdx   = msg->LoginIdx;
				memberCheck->memberIdx  = msg->MemberIdx;
				memberCheck->serverNum  = (short)mServerNum;
				memberCheck->channelNum = (short)mChannelNum;
				memberCheck->retvalue   = 0;

				clientInfo->commonDb.check = SendSQL( perSocketContext, handle, sizeof( MEMBER_CHECK ) );
			}
			break;
		case NM_USER_CHANNEL_LIST_REQ:
			if ( packetLen != sizeof(MSGROOT) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->request.channelList )
			{
				clientInfo->request.channelList = true;
				clientInfo->delay.channelList = GetTickCount( ) + 500;
			}
			break;
		case NM_USER_CHARACTER_LIST_REQ:
			if ( packetLen != sizeof(MSGROOT) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->request.characterList && !clientInfo->commonDb.characterList )
			{
				clientInfo->request.characterList = true;
				clientInfo->delay.characterList = GetTickCount( ) + 500;
			}
			break;
		case NM_USER_CHARACTER_NAME_REQ:
			if ( packetLen != sizeof(MSG_REQ_CHARACTER_NAME) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->commonDb.characterName )
			{
				MSG_REQ_CHARACTER_NAME* msg = (MSG_REQ_CHARACTER_NAME*)msgRoot;

				if ( IsCharacterName( msg->Name, MAX_NAME_BUFFER_SIZE ) )
				{
					HANDLE          handle        = NULL;
					CHARACTER_NAME* characterName = (CHARACTER_NAME*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_NAME );

					wcscpy( characterName->name, msg->Name );

					clientInfo->commonDb.characterName = SendSQL( perSocketContext, handle, sizeof(CHARACTER_NAME) );
				}
				else
				{
					SendMsgError( perSocketContext, NM_USER, (char)NM_USER_CHARACTER_NAME_RES, ERROR_CHARACTER_NAME_RULE );
				}
			}
			break;
		case NM_USER_CHARACTER_CREATE_REQ:
			if ( packetLen != sizeof(MSG_REQ_CHARACTER_CREATE) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->commonDb.characterInsert )
			{
				try
				{
					MSG_REQ_CHARACTER_CREATE* msg = (MSG_REQ_CHARACTER_CREATE*)msgRoot;
	
					// 시스템 관리자 설정.
					if ( mSystemManager.characterCreationLimited )
						throw ERROR_CHARACTER_CREATE_SYSTEM;

					// Name 검사
					msg->Name[MAX_NAME_SIZE] = 0;
					if ( IsCharacterName( msg->Name, MAX_NAME_BUFFER_SIZE ) == false )
						throw ERROR_CHARACTER_CREATE_RULE;

					// Race   검사
					if ( (unsigned char)msg->Race >= eRACE_MAX )
						throw ERROR_CHARACTER_CREATE_RULE;

					// Gender 검사
					if ( (unsigned char)msg->Gender >= eGENDER_MAX )
						throw ERROR_CHARACTER_CREATE_RULE;

					// Stand Idx 검사
					if ( msg->StandIdx >= 5 )
						throw ERROR_CHARACTER_CREATE_RULE;

					switch ( msg->Race )
					{
					case eRACE_HUMAN:
						switch ( msg->Job )
						{
						case ePLAYER_FIGHTER:
						case ePLAYER_MAGE:
							break;
						default:
							throw ERROR_CHARACTER_CREATE_RULE;
						}
						break;
					case eRACE_ELF:
						switch ( msg->Job )
						{
						case ePLAYER_WANDERER:
						case ePLAYER_WARLOCK:
							break;
						default:
							throw ERROR_CHARACTER_CREATE_RULE;
						}
						break;
					case eRACE_BEAST:
						switch ( msg->Job )
						{
						case ePLAYER_ARMSMAN:
						case ePLAYER_RANGER:
							break;
						default:
							throw ERROR_CHARACTER_CREATE_RULE;
						}
						break;
					default:
						throw ERROR_CHARACTER_CREATE_RULE;
					}

					sTargetPos pos = STAGESCRIPT->GetStageChangePos( CHARACTER_CREATE_POS_IDX );
					if ( pos.mMapNumber == 0 )
						throw ERROR_CHARACTER_CREATE_RULE;

					HANDLE            handle          = NULL;
					CHARACTER_INSERT* characterInsert = (CHARACTER_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_INSERT );
					unsigned long     length          = sizeof(CHARACTER_INSERT) - sizeof(characterInsert->table);

					wcscpy( characterInsert->name, msg->Name );

					characterInsert->race         = msg->Race;
					characterInsert->gender       = msg->Gender;
					characterInsert->job          = msg->Job;

					characterInsert->hair         = msg->Hair;
					characterInsert->hairColor    = msg->HairColor;
					characterInsert->face         = msg->Face;

					characterInsert->xPos         = pos.mPosX;
					characterInsert->yPos         = pos.mPosY;

					characterInsert->hp           = 10000;
					characterInsert->mp           = 10000;

					characterInsert->bagEnd       = INVENTORY_BAG0_END;
					characterInsert->wearEnd      = INVENTORY_WEAR_CARD1;
					characterInsert->wareHouseEnd = INVENTORY_WAREHOUSE0_END;
					characterInsert->makeSkillEnd = MIN_MAKESKILL;

					characterInsert->standIdx     = msg->StandIdx;
					characterInsert->userIdx      = clientInfo->memberIdx;

					clientInfo->commonDb.characterInsert = SendSQL( perSocketContext, handle, length );
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_USER, NM_USER_CHARACTER_CREATE_RES, error );
				}
			}
            break;
		case NM_USER_CHARACTER_DELETE_REQ:
			if ( packetLen != sizeof(MSG_REQ_CHARACTER_DELETE) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->commonDb.characterDelete )
			{
				MSG_REQ_CHARACTER_DELETE* msg = (MSG_REQ_CHARACTER_DELETE*)msgRoot;
                CharacterDelete( perSocketContext, clientInfo->memberIdx, msg->CharacterIdx, true );
			}
			break;
        case NM_USER_CHARACTER_DELETE_WAIT_REQ:
			if ( packetLen != sizeof(MSG_REQ_CHARACTER_DELETE_WAIT) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->commonDb.characterDelete )
			{
				MSG_REQ_CHARACTER_DELETE_WAIT* msg = (MSG_REQ_CHARACTER_DELETE_WAIT*)msgRoot;

				HANDLE                 handle          = NULL;
				CHARACTER_DELETE_WAIT* characterDelWait = (CHARACTER_DELETE_WAIT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_DELETE_WAIT );

				characterDelWait->idx     = msg->CharacterIdx;
				characterDelWait->userIdx = clientInfo->memberIdx;

				clientInfo->commonDb.characterDelete = SendSQL( perSocketContext, handle, sizeof(CHARACTER_DELETE_WAIT) );
			}
			break;
        case NM_USER_CHARACTER_DELETE_CANCEL_REQ:
			if ( packetLen != sizeof(MSG_REQ_CHARACTER_DELETE_CANCEL) )
				return false;
			if ( clientInfo->complete.check && !clientInfo->complete.characterSelect && !clientInfo->commonDb.characterDelete )
			{
				MSG_REQ_CHARACTER_DELETE_CANCEL* msg = (MSG_REQ_CHARACTER_DELETE_CANCEL*)msgRoot;

				HANDLE                   handle             = NULL;
				CHARACTER_DELETE_CANCEL* characterDelCancel = (CHARACTER_DELETE_CANCEL*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_DELETE_CANCEL );

				characterDelCancel->idx     = msg->CharacterIdx;
				characterDelCancel->userIdx = clientInfo->memberIdx;

				clientInfo->commonDb.characterDelete = SendSQL( perSocketContext, handle, sizeof(CHARACTER_DELETE_CANCEL) );
			}
            break;
		case NM_USER_COME_IN_GAME_REQ:
			if ( packetLen != sizeof(MSG_REQ_COMEIN_GAME) )
				return false;
			if ( clientInfo->complete.check           == true
			  && clientInfo->request.characterSelect  == false
			  && clientInfo->commonDb.characterSelect == false
			  && clientInfo->complete.characterSelect == false
			  && clientInfo->complete.gameToGame      == false )
			{
				MSG_REQ_COMEIN_GAME* msg = (MSG_REQ_COMEIN_GAME*)msgRoot;

				HANDLE        handle     = NULL;
				GAME_TO_GAME* gameToGame = (GAME_TO_GAME*)GetSQL( &handle, SQL_GAME_PROCESS_GAME_TO_GAME );

				gameToGame->loginIdx       = clientInfo->loginIdx;
				gameToGame->serverNum      = (short)mServerNum;
				gameToGame->channelNum     = (short)(msg->Tutorial ? MAP_TUTORIAL : msg->NumOfChannel);
				gameToGame->lastChannel    = (short)(msg->Tutorial ? msg->NumOfChannel : mChannelNum);
				gameToGame->characterIdx   = msg->CharacterIdx;
				gameToGame->instantDungeon = msg->Tutorial;

				clientInfo->commonDb.gameToGame = SendSQL( perSocketContext, handle, sizeof(GAME_TO_GAME) );
				clientInfo->complete.gameToGame = clientInfo->commonDb.gameToGame;
			}
			break;
		case NM_USER_LOADDATA_REQ:
			if ( packetLen != sizeof(MSGROOT) )
				return false;
			if ( clientInfo->complete.check && clientInfo->complete.characterSelect )
			{
				// 070622 PKH  몬스터 모델 정보 & NPC 모델 정보 전송
				mpObjectManager->SendMsgMonsterModelNumberToPlayer( (ULONG_PTR)perSocketContext, hero->GetMapNumber( ) );
				mpObjectManager->SendMsgNpcModelNumberToPlayer( (ULONG_PTR)perSocketContext, hero->GetMapNumber( ) );
			}
			break;
		case NM_USER_GAMEIN_REQ:
			if ( packetLen != sizeof(MSGROOT) )
				return false;
			if ( clientInfo->complete.check && clientInfo->complete.characterSelect && !clientInfo->request.gameIn )
			{
				clientInfo->request.gameIn = true;
			}
			break;
		case NM_USER_TWITTER_REQ:
			if ( packetLen != sizeof(MSG_REQ_TWITTER) )
				return false;
			if ( clientInfo->complete.check && clientInfo->complete.characterSelect && clientInfo->complete.gameIn )
			{
				MSG_REQ_TWITTER* msg = (MSG_REQ_TWITTER*)msgRoot;

				msg->uid[MAX_TWITTER_UID]=0;
				msg->pwd[MAX_TWITTER_PWD]=0;

				HANDLE          handle  = NULL;
				MEMBER_TWITTER* twitter = (MEMBER_TWITTER*)GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_TWITTER );

				twitter->memberIdx = clientInfo->memberIdx;
				strncpy( twitter->twitterUid, msg->uid, MAX_TWITTER_UID );
				strncpy( twitter->twitterPwd, msg->pwd, MAX_TWITTER_PWD );

				long error = SendSQL( handle, sizeof(MEMBER_TWITTER) ) ? ERROR_TWITTER_SUCCESS : ERROR_TWITTER_FAIL;
				SendMsgError( perSocketContext, NM_USER, NM_USER_TWITTER_RES, error );
			}
			break;
		}
		break; // case NM_USER:

	case NM_PLAYER:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn. --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_PLAYER::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_PLAYER_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_PLAYER::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_PLAYER::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_PLAYER_GAMEFINISH_REQ:
			if ( hero->IsRequestRejection() == true || 
                 hero->IsAutoTrial()        == true ||
                ( hero->GetState() == eOBJECT_STATE_IDLE || hero->GetState() == ePLAYER_STATE_ITEMPICK ) == false )
			{
				SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_GAMEFINISH_RES, ERROR_GAMEFINISH_FAIL );
			}
			else
			{
				hero->ChangeState( eOBJECT_STATE_STOP );
				hero->SetStateStop( eSTOP_GAMEFINISH );	
				hero->StartRequestRejection( eREQREJCT_GAMEFINISH );

				SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_GAMEFINISH_RES, ERROR_GAMEFINISH_SUCCESS );
			}
			break;
		case NM_PLAYER_GAMEFINISH_CANCEL_REQ:
			hero->EndRequestRejection( eREQREJCT_GAMEFINISH );
			break;
		case NM_PLAYER_DISCONNECT_REQ:
			if ( hero->GetRequestRejection() != eREQREJCT_GAMEFINISH )
			{
				SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_DISCONNECT_RES, ERROR_GAMEFINISH_FAIL );
			}
			else
			{
				SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_DISCONNECT_RES, ERROR_GAMEFINISH_SUCCESS );
			}
			break;
		case NM_PLAYER_GAMEOUT_REQ:
			if ( clientInfo->complete.gameIn )
			{
				if( hero->GetRequestRejection() == eREQREJCT_GAMEFINISH )
					clientInfo->request.gameOut = true;
				else
					SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_DISCONNECT_RES, ERROR_GAMEFINISH_FAIL );
			}
			break;
 
		case NM_PLAYER_MAPCHANGE_REQ:
			if ( clientInfo->complete.gameIn )
			{
				MSG_REQ_PLAYER_MAPCHANGE* reqMsg  = (MSG_REQ_PLAYER_MAPCHANGE*)msgRoot;

				if( hero->MapChange( reqMsg->mMapChangePosIndex ) == false )
				{
					// 맵이동 응답 메시지 - 실패
					// 이동 성공메세지는 여기서 보내지 않고 후에 맵이동에 필요한 데이타와 함게 보낸다.
					SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, 1 );

					// 상태 풀기
					hero->ChangeState( eOBJECT_STATE_IDLE );
					hero->SetStateStop( eSTOP_NONE );
				}
                else
                {
                    if( mType != _E_ST_NORMAL_MAP_ )
                    {
                        clientInfo->request.returnToMap = true;
                        clientInfo->gotoCID = MAKECID( (WORD)mServerNum, (WORD)clientInfo->lastChannel );
                    }
                }
			}
			break;
		case NM_PLAYER_MOVE_REQ:
			{
				MSG_REQ_MOVE* pmsg = (MSG_REQ_MOVE*)msgRoot;

				NiPoint2 start( pmsg->startX, pmsg->startY );
				NiPoint2 goal( pmsg->destX, pmsg->destY );

				sObject target;
				target.index = 0;
				target.type = eOBJECTTYPE_NONE;
				hero->SetMoveReq( msgRoot->Protocol, start, goal, pmsg->mClientTime, 0, target, NiPoint2::ZERO );

				//try
				//{
				//	if( hero->GetState() == eOBJECT_STATE_PUSHPULL )
				//		return true;

				//	if( hero->IsChangeState( eOBJECT_STATE_MOVE ) == false )
				//		throw false;

				//	char errorCode = hero->IsMovePossible( start, goal );
				//	if( errorCode == 1 )
				//	{
				//		/// 이동 불가능 지역이면 클라이언트가 서있던 장소로 이동시킨다.
				//		if( hero->IsClientMovePossible( start, goal ) == false )
				//		{
				//			throw false;
				//		}
				//		goal = start;
				//	}
				//	else if( errorCode == 2 )
				//		throw false;

				//	// 061208 PKH 이동 목적지를 설정
				//	hero->SetMoveTargetPos( pmsg->destX, pmsg->destY );
				//	hero->ChangeState( eOBJECT_STATE_MOVE );
				//	hero->CalcClientMoveDeff( pmsg->mClientTime );
				//	hero->SetClientMoveReq( goal, start, false );

				//	if( hero->GetCheatHideMode() == false )
				//	{
				//		// 다른 플레이어들에게도 알림
				//		MSG_SYN_MOVE Msg;
				//		Msg.Category     = NM_PLAYER;
				//		Msg.Protocol     = NM_PLAYER_MOVE_SYN;
				//		Msg.characterIdx = heroIdx;
				//		Msg.destX        = pmsg->destX;
				//		Msg.destY        = pmsg->destY;
				//		Msg.moveSpeed    = hero->GetMoveSpeed();
				//		QuickSendExcept( hero, (char*)&Msg, sizeof(Msg) );
				//	}
				//}
				//catch ( bool )
				//{
				//	float posX = hero->GetXPos();
				//	float posY = hero->GetYPos();
				//	bool possible = AIMANAGER->IsPossible( hero->GetMapNumber(), hero->GetXPos(), hero->GetYPos(), hero->GetObject() );
				//	if( possible == false )
				//	{
				//		float tempX = posX;
				//		float tempY = posY;
				//		/// 랜덤범위 50, 100, 150 3단계로 5번씩 갈수있는 지역 체크해서 가능하면 이동시킨다
				//		for( int i = 0 ; i < 15 ; ++i )
				//		{
				//			/// 랜덤폭 - 0~4=50, 5~9=100, 10~14=150
				//			unsigned long tempRange = 150;
				//			if( i <= 4 )
				//				tempRange = 50;
				//			else if( 5 <= i && i <= 9 )
				//				tempRange = 100;
				//			else
				//				tempRange = 150;

				//			tempX = hero->GetXPos() + rand() % ( tempRange * 2 ) - tempRange;
				//			tempY = hero->GetYPos() + rand() % ( tempRange * 2 ) - tempRange;
				//			possible = AIMANAGER->IsPossible( hero->GetMapNumber(), tempX, tempY, hero->GetObject() );
				//			if( possible == true )
				//			{
				//				posX = tempX;
				//				posY = tempY;
				//				break;
				//			}
				//		}
				//	}

				//	hero->SetPos( posX, posY );
				//	hero->ClearMoveSyncCalc();
				//	hero->SetMoveTargetPos( posX, posY );

				//	// 070625 PKH 실패 메세지 발송 - 서버상의 좌표로 클라이언트를 옮기게 함
				//	HANDLE            handle = NULL;
				//	MSG_RES_ERR_MOVE* msg    = (MSG_RES_ERR_MOVE*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_MOVE_RES );
				//	msg->destX = posX;
				//	msg->destY = posY;
				//	SendMsgRoot( handle, sizeof(MSG_RES_ERR_MOVE) );
				//}
			}
			break;
		case NM_PLAYER_ACTIONMOVE_REQ:
			{
				MSG_REQ_ACTIONMOVE* pmsg = (MSG_REQ_ACTIONMOVE*)msgRoot;
				NiPoint2 start( pmsg->startX, pmsg->startY );
				NiPoint2 goal( pmsg->destX, pmsg->destY );
				NiPoint2 targetPos( pmsg->targetX, pmsg->targetY );

				hero->SetMoveReq( msgRoot->Protocol, start, goal, pmsg->mClientTime, pmsg->range, pmsg->targetInfo, targetPos );

				//try
				//{
				//	if( hero->GetState() == eOBJECT_STATE_PUSHPULL )
				//		return true;

				//	if( hero->IsChangeState( eOBJECT_STATE_MOVE ) == false )
				//		throw false;

				//	char errorCode = hero->IsMovePossible( start, goal );
				//	if( errorCode == 1 )
				//	{
				//		/// 이동 불가능 지역이면 클라이언트가 서있던 장소로 이동시킨다.
				//		if( hero->IsClientMovePossible( start, goal ) == false )
				//			throw false;
				//		goal = start;
				//	}
				//	else if( errorCode == 2 )
				//		throw false;

				//	hero->SetMoveTargetPos( pmsg->destX, pmsg->destY );	
				//	hero->SetRangeTarget( pmsg->targetX, pmsg->targetY, pmsg->targetInfo, pmsg->range );
				//	hero->ChangeState( eOBJECT_STATE_MOVE );
				//	hero->CalcClientMoveDeff( pmsg->mClientTime );
				//	hero->SetClientMoveReq( goal, start, false );

				//	if( hero->GetCheatHideMode() == false )
				//	{
				//		// 다른 플레이어들에게도 알림
				//		MSG_SYN_ACTIONMOVE Msg;
				//		Msg.Category = NM_PLAYER;
				//		Msg.Protocol = NM_PLAYER_ACTIONMOVE_SYN;
				//		Msg.characterIdx = heroIdx;
				//		Msg.destX = pmsg->destX;
				//		Msg.destY = pmsg->destY;
				//		Msg.targetX = pmsg->targetX;
				//		Msg.targetY = pmsg->targetY;
				//		Msg.range = pmsg->range;
				//		Msg.targetInfo = pmsg->targetInfo;
				//		Msg.moveSpeed = hero->GetMoveSpeed();
				//		QuickSendExcept( hero, (char*)&Msg, sizeof(Msg) );
				//	}
				//}
				//catch ( bool )
				//{
				//	float posX = hero->GetXPos();
				//	float posY = hero->GetYPos();
				//	bool possible = AIMANAGER->IsPossible( hero->GetMapNumber(), hero->GetXPos(), hero->GetYPos(), hero->GetObject() );
				//	if( possible == false )
				//	{
				//		float tempX = posX;
				//		float tempY = posY;
				//		/// 랜덤범위 50, 100, 150 3단계로 5번씩 갈수있는 지역 체크해서 가능하면 이동시킨다
				//		for( int i = 0 ; i < 15 ; ++i )
				//		{
				//			/// 랜덤폭 - 0~4=50, 5~9=100, 10~14=150
				//			unsigned long tempRange = 150;
				//			if( i <= 4 )
				//				tempRange = 50;
				//			else if( 5 <= i && i <= 9 )
				//				tempRange = 100;
				//			else
				//				tempRange = 150;

				//			tempX = hero->GetXPos() + rand() % ( tempRange * 2 ) - tempRange;
				//			tempY = hero->GetYPos() + rand() % ( tempRange * 2 ) - tempRange;
				//			possible = AIMANAGER->IsPossible( hero->GetMapNumber(), tempX, tempY, hero->GetObject() );
				//			if( possible == true )
				//			{
				//				posX = tempX;
				//				posY = tempY;
				//				break;
				//			}
				//		}
				//	}

				//	hero->SetPos( posX, posY );
				//	hero->ClearMoveSyncCalc();
				//	hero->SetMoveTargetPos( posX, posY );

				//	// 070625 PKH 실패 메세지 발송 - 서버상의 좌표로 클라이언트를 옮기게 함
				//	HANDLE            handle  = NULL;
				//	MSG_RES_ERR_MOVE* sendMsg = (MSG_RES_ERR_MOVE*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_ACTIONMOVE_RES );
				//	sendMsg->destX = posX;
				//	sendMsg->destY = posY;
				//	SendMsgRoot( handle, sizeof(MSG_RES_ERR_MOVE) );
				//}
			}
			break;
		case NM_PLAYER_KEYMOVE_REQ:
			{
				MSG_REQ_KEYMOVE* pmsg = (MSG_REQ_KEYMOVE*)msgRoot;
				NiPoint2 start( pmsg->mStartX, pmsg->mStartY );
				NiPoint2 goal( pmsg->mDestX, pmsg->mDestY );

				sObject target;
				target.index = 0;
				target.type = eOBJECTTYPE_NONE;
				if( hero->GetState() != ePLAYER_STATE_RUSH )
					hero->SetMoveReq( msgRoot->Protocol, start, goal, pmsg->mClientTime, 0, target, NiPoint2::ZERO );

				//try
				//{
				//	if( hero->GetState() == eOBJECT_STATE_PUSHPULL )
				//		return true;

				//	if( hero->IsChangeState( eOBJECT_STATE_MOVE ) == false )
				//		throw false;

				//	char errorCode = hero->IsMovePossible( start, goal );
				//	if( errorCode == 1 )
				//	{
				//		/// 이동 불가능 지역이면 클라이언트가 서있던 장소로 이동시킨다.
				//		if( hero->IsClientMovePossible( start, goal ) == false )
				//			throw false;
				//		goal = start;
				//	}
				//	else if( errorCode == 2 )
				//		throw false;

				//	// 이동 목적지를 설정
				//	hero->SetMoveTargetPos( goal.x, goal.y );
				//	hero->ChangeState( eOBJECT_STATE_MOVE );
				//	hero->CalcClientMoveDeff( pmsg->mClientTime );
				//	hero->SetClientMoveReq( goal, start, true );

				//	if( hero->GetCheatHideMode() == false )
				//	{
				//		// 다른 플레이어들에게도 알림
				//		MSG_SYN_MOVE Msg;
				//		Msg.Category     = NM_PLAYER;
				//		Msg.Protocol     = NM_PLAYER_MOVE_SYN;
				//		Msg.characterIdx = heroIdx;
				//		Msg.destX        = goal.x;
				//		Msg.destY        = goal.y;
				//		Msg.moveSpeed    = hero->GetMoveSpeed();
				//		QuickSendExcept( hero, (char*)&Msg, sizeof(Msg) );
				//	}
				//}
				//catch ( bool )
				//{
				//	float posX = hero->GetXPos();
				//	float posY = hero->GetYPos();
				//	bool possible = AIMANAGER->IsPossible( hero->GetMapNumber(), hero->GetXPos(), hero->GetYPos(), hero->GetObject() );
				//	if( possible == false )
				//	{
				//		float tempX = posX;
				//		float tempY = posY;
				//		/// 랜덤범위 50, 100, 150 3단계로 5번씩 갈수있는 지역 체크해서 가능하면 이동시킨다
				//		for( int i = 0 ; i < 15 ; ++i )
				//		{
				//			/// 랜덤폭 - 0~4=50, 5~9=100, 10~14=150
				//			unsigned long tempRange = 150;
				//			if( i <= 4 )
				//				tempRange = 50;
				//			else if( 5 <= i && i <= 9 )
				//				tempRange = 100;
				//			else
				//				tempRange = 150;

				//			tempX = hero->GetXPos() + rand() % ( tempRange * 2 ) - tempRange;
				//			tempY = hero->GetYPos() + rand() % ( tempRange * 2 ) - tempRange;
				//			possible = AIMANAGER->IsPossible( hero->GetMapNumber(), tempX, tempY, hero->GetObject() );
				//			if( possible == true )
				//			{
				//				posX = tempX;
				//				posY = tempY;
				//				break;
				//			}
				//		}
				//	}

				//	hero->SetPos( posX, posY );
				//	hero->ClearMoveSyncCalc();
				//	hero->SetMoveTargetPos( posX, posY );

				//	// 실패 메세지 발송 - 서버상의 좌표로 클라이언트를 옮기게 함
				//	HANDLE handle = NULL;
				//	MSG_RES_KEYMOVE* msg = (MSG_RES_KEYMOVE*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_KEYMOVE_RES );
				//	msg->mCharacterPosX = posX;
				//	msg->mCharacterPosY = posY;
				//	SendMsgRoot( handle, sizeof(MSG_RES_KEYMOVE) );
				//}
			}
			break;
		case NM_PLAYER_ACTIONMOVE_STOP_REQ:
			{
				MSG_REQ_MOVE_STOP* pmsg = (MSG_REQ_MOVE_STOP*)msgRoot;
				hero->MoveStopReq( pmsg, msgRoot->Protocol, NM_PLAYER_ACTIONMOVE_STOP_RES );
			}
			break;
		case NM_PLAYER_KEYMOVE_STOP_REQ:
			{
				MSG_REQ_MOVE_STOP* pmsg = (MSG_REQ_MOVE_STOP*)msgRoot;
				hero->MoveStopReq( pmsg, msgRoot->Protocol, NM_PLAYER_KEYMOVE_STOP_RES );
			}
			break;
		case NM_PLAYER_FOLLOWMOVE_STOP_REQ:
			{
				MSG_REQ_MOVE_STOP* pmsg = (MSG_REQ_MOVE_STOP*)msgRoot;
				hero->MoveStopReq( pmsg, msgRoot->Protocol, NM_PLAYER_FOLLOWMOVE_STOP_RES );
			}
			break;
		case NM_PLAYER_FINDPOSITIONMOVE_STOP_REQ:
			{
				MSG_REQ_MOVE_STOP* pmsg = (MSG_REQ_MOVE_STOP*)msgRoot;
				hero->MoveStopReq( pmsg, msgRoot->Protocol, NM_PLAYER_FINDPOSITIONMOVE_STOP_RES );
			}
			break;
		case NM_PLAYER_RESURRECTION_REQ:
			{
				MSG_REQ_PLAYER_RESURRECTION* reqMsg = (MSG_REQ_PLAYER_RESURRECTION*)msgRoot;
				hero->Resurrection( reqMsg->mFlag );
			}
			break;
		case NM_PLAYER_SHORTCUT_LIST_REQ:
			if ( clientInfo->complete.shortcut )
			{
				clientInfo->request.shortcutSend = true;
			}
			break;
		case NM_PLAYER_SHORTCUT_UPDATE_REQ:
			if ( clientInfo->complete.shortcut )
			{
				// 튜토리얼 모드가 아닐때만 셋팅
				if( hero->IsTutorialMode() == false )
				{
					MSG_REQ_PLAYER_SHORTCUT_UPDATE* msg      = (MSG_REQ_PLAYER_SHORTCUT_UPDATE*)msgRoot;
					sShortcut*                      shortcut = msg->mShortcut;

					for ( int i = 0; i < MAX_SHORTCUT_SIZE; i++, shortcut++ )
					{
						if ( shortcut->mType == eSHORTCUT_ITEM && shortcut->mExData == 0 )
						{
							memset( shortcut, 0, sizeof(sShortcut) );
						}
						else if ( shortcut->mType >= eSHORTCUT_UNKNOWN )
						{
							memset( shortcut, 0, sizeof(sShortcut) );
						}
					}

					memcpy( clientInfo->shortcut.binary, msg->mShortcut, sizeof(msg->mShortcut) );
				}
			}
			break;
		case NM_PLAYER_OPTION_UPDATE_REQ:
			if ( clientInfo->complete.characterOption )
			{
				MSG_REQ_PLAYER_OPTION_UPDATE* msg = (MSG_REQ_PLAYER_OPTION_UPDATE*)msgRoot;

				if ( clientInfo->optionData1 != msg->option1 )
				{
					clientInfo->optionData1 = msg->option1;
					hero->SetOptionData( clientInfo->optionData1 );

					HANDLE            handle          = NULL;
					CHARACTER_OPTION* characterOption = (CHARACTER_OPTION*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_OPTION );

					characterOption->idx     = heroIdx;
					characterOption->option1 = clientInfo->optionData1;

					SendSQL( perSocketContext, handle, sizeof(CHARACTER_OPTION) );
				}

				SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_OPTION_UPDATE_RES, ERROR_PLAYER_OPTION_UPDATE_SUCCESS );
			}
			break;
		case NM_PLAYER_STATE_IDLE_REQ:
			{
				MSG_REQ_PLAYER_STATE_IDLE* pReqMsg = (MSG_REQ_PLAYER_STATE_IDLE*)msgRoot;

				hero->SetStateIdle( pReqMsg->mPlayerStateIdle );

				// 다른 플레이어들에게도 알림
				MSG_SYN_PLAYER_STATE_IDLE synMsg;

				synMsg.Category         = NM_PLAYER;
				synMsg.Protocol         = NM_PLAYER_STATE_IDLE_SYN;
				synMsg.mCharacterIdx    = heroIdx;
				synMsg.mPlayerStateIdle = pReqMsg->mPlayerStateIdle;

				QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
			}
			break;
		case NM_PLAYER_STATE_SITDOWN_REQ:
			{
				MSG_REQ_PLAYER_STATE_SITDWON* pReqMsg = (MSG_REQ_PLAYER_STATE_SITDWON*)msgRoot;

				unsigned char errCode = 0;

				// 앉기
				if( pReqMsg->mSitDown == true )
				{
					// 상태 변경 실패
					if( hero->ChangeState( ePLAYER_STATE_SITDOWN ) == false )
					{
						errCode = 1;
					}
					else
					{
						if( hero->GetStateOddity( eODDITYTYPE_SLEEP ) != 0 || hero->GetStateOddity( eODDITYTYPE_STUN ) != 0 )
							errCode = 1;
					}
				}
				else	// 일어서기
				{
					// 상태 변경 실패
					if( hero->GetState() != ePLAYER_STATE_SITDOWN || hero->ChangeState( eOBJECT_STATE_IDLE ) == false )
						errCode = 1;
				}

				HANDLE                        handle = NULL;
				MSG_RES_PLAYER_STATE_SITDWON* msg    = (MSG_RES_PLAYER_STATE_SITDWON*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_STATE_SITDOWN_RES );
				msg->ErrorCode = errCode;
				msg->mSitDown  = (hero->GetState() == ePLAYER_STATE_SITDOWN) ? true : false;
				SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_STATE_SITDWON) );

				if( errCode == 0 )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_PLAYER_STATE_SITDWON synMsg;

					synMsg.Category      = NM_PLAYER;
					synMsg.Protocol      = NM_PLAYER_STATE_SITDOWN_SYN;
					synMsg.mCharacterIdx = heroIdx;
					synMsg.mSitDown      = hero->GetState() == ePLAYER_STATE_SITDOWN ? true : false;

					QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
				}
			}
			break;
		case NM_PLAYER_COMMUNITYSKILL_REQ:
			{
				MSG_REQ_PLAYER_COMMUNITYSKILL* pReqMsg = (MSG_REQ_PLAYER_COMMUNITYSKILL*)msgRoot;

				try
				{
					/// 상태 체크
					if( hero->GetState() != eOBJECT_STATE_IDLE )
						throw 1;

					/// 몬스터 변신중엔 생활스킬 불가
					if( hero->GetChgMonsterIdx() != 0 )
						throw 1;

					throw 0;
				}
				catch ( int error )
				{
					HANDLE                         handle  = NULL;
					MSG_RES_PLAYER_COMMUNITYSKILL* sendMsg = (MSG_RES_PLAYER_COMMUNITYSKILL*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_COMMUNITYSKILL_RES );
						sendMsg->ErrorCode          = error;
						sendMsg->mCommunitySkillIdx = pReqMsg->mCommunitySkillIdx;
					SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_COMMUNITYSKILL) );

					if ( error == 0 )
					{
						hero->SetCommunitySkill( pReqMsg->mCommunitySkillIdx );

						// 다른 플레이어들에게도 알림
						MSG_SYN_PLAYER_COMMUNITYSKILL synMsg;
							synMsg.Category           = NM_PLAYER;
							synMsg.Protocol           = NM_PLAYER_COMMUNITYSKILL_SYN;
							synMsg.mCharacterIdx      = heroIdx;
							synMsg.mCommunitySkillIdx = pReqMsg->mCommunitySkillIdx;
						QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );		
					}
				}
			}
			break;
		//case NM_PLAYER_MONSTER_ANI_REQ:
		//	{
		//		MSG_REQ_PLAYER_MONSTER_ANI* pReqMsg = (MSG_REQ_PLAYER_MONSTER_ANI*)msgRoot;

		//		try
		//		{
		//			/// 상태 체크
		//			if( hero->GetState() != eOBJECT_STATE_IDLE )
		//				throw 1;	

		//			/// 몬스터 변신체크
		//			if( hero->GetChgMonsterIdx() == 0 )
		//				throw 1;

		//			throw 0;
		//		}
		//		catch ( int error )
		//		{
		//			/// 실패 인경우만 hero에게 전송
		//			SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_MONSTER_ANI_RES, error );

		//			if( error != 0 )
		//			{
		//				// 성공 인경우 다른 플레이어들에게도 알림
		//				MSG_SYN_PLAYER_MONSTER_ANI synMsg;
		//				synMsg.Category = NM_PLAYER;
		//				synMsg.Protocol = NM_PLAYER_MONSTER_ANI_SYN;
		//				synMsg.mCharacterIdx = heroIdx;
		//				synMsg.mMonsterAni = pReqMsg->mMonsterAni;

		//				QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );		
		//			}
		//		}
		//	}
		//	break;
		case NM_PLAYER_STATICFACE_REQ:
			{
				MSG_REQ_PLAYER_STATICFACE* pReqMsg = (MSG_REQ_PLAYER_STATICFACE*)msgRoot;

				hero->SetStaticFaceIndex( pReqMsg->mStaticFaceIdx );

				MSG_SYN_PLAYER_STATICFACE synMsg;
				synMsg.Category      = NM_PLAYER;
				synMsg.Protocol      = NM_PLAYER_STATICFACE_SYN;
				synMsg.mCharacterIdx = heroIdx;
				synMsg.mStaticFaceIdx = pReqMsg->mStaticFaceIdx;

				QuickSend( hero, (char*)&synMsg, sizeof(synMsg) );
			}
			break;
		case NM_PLAYER_TITLE_SET_REQ:
			{
				/// 호칭변경 요청
				MSG_REQ_PLAYER_TITLE_SET* pmsg = (MSG_REQ_PLAYER_TITLE_SET*)msgRoot;

				try
				{
					if( hero->GetStateDie() == true )
						throw ERROR_PLAYER_TITLE_SET_FAIL;

					bool ret = hero->ChangeTitle( pmsg->mTitleIndex );
					throw (ret == true) ? ERROR_PLAYER_TITLE_SET_SUCCESS : ERROR_PLAYER_TITLE_SET_FAIL;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_TITLE_SET_RES, error );
				}
			}
			break;
		case NM_PLAYER_TODAYWORD_CHANGE_REQ:
			{
				MSG_REQ_PLAYER_TODAYWORD_CHANGE* pmsg = (MSG_REQ_PLAYER_TODAYWORD_CHANGE*)msgRoot;

				try
				{
					if( hero->GetStateDie() == true )
						throw ERROR_PLAYER_TODAYWORD_CHANGE_FAIL;
	
					hero->ChangeTodayWord( (ULONG_PTR)perSocketContext, pmsg->mWord, pmsg->mWordColor );
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_TODAYWORD_CHANGE_RES, error );
				}
			}
			break;
		case NM_PLAYER_VEHICLE_END_REQ:
			/// 탈것 해제
			hero->EndVehicleClient( true );
			break;
		case NM_PLAYER_SKILLRESET_REQ:
			try
			{
				MSG_REQ_PLAYER_SKILLRESET* pmsg = (MSG_REQ_PLAYER_SKILLRESET*)msgRoot;
				/// 스킬 초기화 가능 체크 & db처리 요청
				throw hero->SkillResetReq( pmsg->mSlotNum );
			}
			catch( int error )
			{
				/// 실패시 메세지 발송
				if ( error != ERROR_SKILLRESET_SUCCESS )
					SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_SKILLRESET_RES, error );
			}
			break;
		case NM_PLAYER_JUMP_REQ:
			hero->Jump();
            break;
        case NM_PLAYER_TRIAL_OPEN_REQ:
            {
                MSG_REQ_PLAYER_TRIAL_OPEN* pMsg = (MSG_REQ_PLAYER_TRIAL_OPEN*)msgRoot;
                cPlayer* pTarget = mpObjectManager->GetPlayer( pMsg->characterIdx );
                try
                {
                    if ( pTarget == NULL )
                        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;

                    if ( clientInfo->commonDb.trial )
                        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;

                    if ( pTarget )
                    {
 						PerSocketContext* pTargetContext = mSocketContextPool->GetCID( pTarget->GetConnectionIdx() );
 						if ( pTargetContext != NULL )
 						{
 							ClientInfo* targetInfo = (ClientInfo*)pTargetContext->buffer;
                             if( targetInfo && targetInfo->slevel == 1 )
                                 throw ERROR_PLAYER_TRIAL_OPEN_GM;
                         }
                    }
                    else
                        throw ERROR_PLAYER_TRIAL_OPEN_FAIL;

                    throw hero->TrialOpenReq( (ULONG_PTR)perSocketContext, clientInfo->slevel, pTarget );
                }
                catch ( int error )
                {
                    if ( error != ERROR_PLAYER_TRIAL_OPEN_SUCCESS )
                        hero->SendTrialReqResult( (ULONG_PTR)perSocketContext, error, pTarget );
                }
            }
            break;
        case NM_PLAYER_TRIAL_ANSWER_REQ:
			try
			{
                MSG_REQ_PLAYER_TRIAL_ANSWER* pMsg = (MSG_REQ_PLAYER_TRIAL_ANSWER*)msgRoot;

                int retval = hero->TrialAnswer( (ULONG_PTR)perSocketContext, pMsg->answer );
                if ( retval == ERROR_PLAYER_TRIAL_ANSWER_SUCCESS )
                {
                    PerSocketContext* pOtherContext = mSocketContextPool->GetCID( hero->GetTrialMemberIdx() );
                    if ( pOtherContext != NULL )
                    {
                        ClientInfo* otherInfo = (ClientInfo*)pOtherContext->buffer;
                        if ( otherInfo->slevel != 1 )
                        {
                            /// 신고자 재사용 시간 1시간 연장
                            HANDLE        handle      = NULL;
                            MEMBER_TRIAL* trialUpdate = (MEMBER_TRIAL*)GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_TRIAL_UPDATE );
                            if ( trialUpdate != NULL )
                            {
                                trialUpdate->memberIdx   = otherInfo->memberIdx;
                                trialUpdate->trialTime   = 3600;        // 재사용시간 1시간(초단위)
                                trialUpdate->retvalue    = 0;

                                SendSQL( (PerSocketContext*)pOtherContext, handle, sizeof(MEMBER_TRIAL), COMMON_DB_MEMBER_TRIAL);
                            }
                        }
                    }
                    // 심판 결과 처리
                    hero->TrialEnd( false );
                }
                throw retval;
            }
			catch( int error )
            {
                HANDLE        handle  = NULL;
                MSG_RES_PLAYER_TRIAL_ANSWER* sendMsg = (MSG_RES_PLAYER_TRIAL_ANSWER*)NETWORK2->GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_TRIAL_ANSWER_RES );
                if ( sendMsg != NULL )
                {
                    sendMsg->trialCount = hero->GetTrialAnswerCount();
                    sendMsg->ErrorCode  = error;
                    SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_TRIAL_ANSWER) );
                }
            }
			break;
        case NM_PLAYER_TRIAL_CAPTCHA_REQ:
            try
            {
                if ( hero->IsAutoTrial() == false )
                    throw ERROR_PLAYER_TRIAL_CAPTCHA_TRIAL;

                if ( hero->GetCaptChaCount() <= 0 )
                    throw ERROR_PLAYER_TRIAL_CAPTCHA_COUNT;

                {
                    HANDLE          handle        = NULL;
                    CAPTCHA_RELOAD* captchaReload = (CAPTCHA_RELOAD*)GetSQL( &handle, SQL_GAME_PROCESS_CAPTCHA_RELOAD );
                    captchaReload->captchaOffset  = mCaptchaOffset;
                    captchaReload->captchaSize    = 0;

                    SendSQL( perSocketContext, handle, sizeof(CAPTCHA_RELOAD) );
                }
            }
            catch( int error )
            {
                HANDLE  handle  = NULL;
                MSG_RES_PLAYER_TRIAL_CAPTCHA* sendMsg = (MSG_RES_PLAYER_TRIAL_CAPTCHA*)GetMsgRoot( &handle, 
                                                                                                   perSocketContext, 
                                                                                                   NM_PLAYER, 
                                                                                                   NM_PLAYER_TRIAL_CAPTCHA_RES );
                if ( sendMsg != NULL )
                {
                    sendMsg->count      = hero->GetCaptChaCount();
                    sendMsg->ErrorCode  = error;
                    SendMsgRoot( handle, sendMsg->GetMsgLength() );
                }
            }
            break;
        case NM_PLAYER_LOOK_OTHER_EQUIP_REQ:
            try
            {
                MSG_REQ_PLAYER_LOOK_OTHER_EQUIP* pMsg = (MSG_REQ_PLAYER_LOOK_OTHER_EQUIP*)msgRoot;

                cPlayer* pTarget = OBJECTMANAGER->GetPlayer( pMsg->characterIdx );
                if ( pTarget )
                {
                    HANDLE  handle  = NULL;
                    MSG_RES_PLAYER_LOOK_OTHER_EQUIP* sendMsg = (MSG_RES_PLAYER_LOOK_OTHER_EQUIP*)GetMsgRoot( &handle, 
                                                                                                             perSocketContext, 
                                                                                                             NM_PLAYER, 
                                                                                                             NM_PLAYER_LOOK_OTHER_EQUIP_RES );
                    if( sendMsg != NULL )
                    {
                        sendMsg->ErrorCode   = ERROR_PLAYER_LOOK_OTHER_EQUIP_SUCCESS;

                        wcscpy( sendMsg->name, pTarget->GetName() );
                        sendMsg->job           = (unsigned short)pTarget->GetJob();
                        sendMsg->level         = pTarget->GetLevel();
                        sendMsg->wearEnd       = pTarget->GetWearEnd();
                        sendMsg->activeWeapon  = (char)pTarget->GetActiveWeapon();

                        pTarget->SetInventoryEquip( sendMsg->Inventory, &(sendMsg->RowCount) );
                        SendMsgRoot( handle, sendMsg->GetMsgLength() );
                    }
                }
                else
                    throw ERROR_PLAYER_LOOK_OTHER_EQUIP_FAIL;
            }
            catch( int error )
            {
                SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_LOOK_OTHER_EQUIP_RES, error );
            }
            break;
		}
		break; // NM_PLAYER

	case NM_ITEM:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn / Inventory --*/
		if ( (clientInfo->completeData & 0x000F) != 0x000F )
		{
			PostServerEvent( "WRONG NM_ITEM::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_ITEM_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_ITEM::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_ITEM::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		if ( !clientInfo->complete.inventory )
			return false;

		switch ( msgRoot->Protocol )
		{
		case NM_ITEM_MOV_INVENTORY_REQ:
			{
				MSG_REQ_ITEM_MOV_INVENTORY* msg = (MSG_REQ_ITEM_MOV_INVENTORY*)msgRoot;
				try
				{
					hero->ItemMovInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_MOV_INVENTORY*)msgRoot );
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_MOV_INVENTORY_RES, error );
				}

                // 착용정보 변경 요청.
				if ( hero->IsInventoryEquip( msg->number1 ) || hero->IsInventoryEquip( msg->number2 ) )
					clientInfo->request.characterEquip  = true;
			}
			break;	// switch ( msgRoot->Protocol )
		case NM_ITEM_USE_INVENTORY_REQ:
			try 
			{
				hero->ItemUseInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_USE_INVENTORY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_USE_INVENTORY_RES, error );
			}
			break;	// switch ( msgRoot->Protocol )
		case NM_ITEM_DEL_INVENTORY_REQ:
			try
			{
				hero->ItemDelInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_DEL_INVENTORY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_DEL_INVENTORY_RES, error );
			}
			break;
		case NM_ITEM_SWI_INVENTORY_REQ:
			try
			{
				hero->ItemSwiInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_SWI_INVENTORY*)msgRoot );

				// 착용정보 변경 요청.
				clientInfo->request.characterEquip = true;
				clientInfo->complete.characterActiveWeapon = true;
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_SWI_INVENTORY_RES, error );
			}
			break;
		case NM_ITEM_DIVIDE_INVENTORY_REQ:
			try
			{
				hero->ItemDivideInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_DIVIDE_INVENTORY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_DIVIDE_INVENTORY_RES, error );
			}
			break;
		case NM_ITEM_ENHANCED_START_REQ:
			try
			{
				hero->ItemEnhancedStart( (MSG_REQ_ITEM_ENHANCED_START*)msgRoot );
			}
			catch ( int error )
			{
				/// 성공 / 실패 메세지 발송
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_ENHANCED_START_RES, error );
			}
			break;
		case NM_ITEM_ENHANCED_CANCEL_REQ:
			hero->ItemEnhancedCancel( );
			break;
		case NM_ITEM_DISJOINT_INVENTORY_REQ:
			try
			{
				hero->ItemDisjointInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_DISJOINT_INVENTORY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_DISJOINT_INVENTORY_RES, error );
			}
			break;
		case NM_ITEM_PUT_CARD_INVENTORY_REQ:
			try
			{
				hero->ItemPutCardInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_PUT_CARD_INVENTORY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_PUT_CARD_INVENTORY_RES, error );
			}
			break;
		case NM_ITEM_CHANGE_INVENTORY_REQ:
			try
			{
				hero->ItemChangeInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_CHANGE_INVENTORY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_CHANGE_INVENTORY_RES, error );
			}
			break;

		/*-- 거래-사용자간 [요청 / 응답 / 등록 / 삭제 / 수정 / 완료 / 재수정 / 취소 / 종료]
		*/
		case NM_ITEM_EXCHANGE_ASK_REQ:
			try
			{
				if ( mType == _E_ST_ID_PVP_ )
					throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

				// KKR 튜토리얼맵에서 노점막기
				if ( mType == _E_ST_ID_TUTORIAL_ )
					throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

				hero->ExchangeAsk( (MSG_REQ_ITEM_EXCHANGE_ASK*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_ASK_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_REP_REQ:
			try
			{
				hero->ExchangeReply( (MSG_REQ_ITEM_EXCHANGE_REP*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_REP_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_ADD_REQ:
			try
			{
				hero->ExchangeAdd( (MSG_REQ_ITEM_EXCHANGE_ADD*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_ADD_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_DEL_REQ:
			try
			{
				hero->ExchangeDel( (MSG_REQ_ITEM_EXCHANGE_DEL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_DEL_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_OK_REQ:
			try
			{
				hero->ExchangeOk( (MSG_REQ_ITEM_EXCHANGE_OK*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_OK_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_RETRY_REQ:
			try
			{
				hero->ExchangeRetry( (MSG_REQ_ITEM_EXCHANGE_RETRY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_RETRY_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_CANCEL_REQ:
			try
			{
				hero->ExchangeCancel( (MSG_REQ_ITEM_EXCHANGE_CANCEL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_CANCEL_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_END_REQ:
			try
			{
				hero->ExchangeEnd( (MSG_REQ_ITEM_EXCHANGE_END*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_END_RES, error );
			}
			break;

		/*-- 노점-판매 관련 [열기 / 닫기 / 등록 / 삭제 / 수정 / 참여 / 닫기 / 구입]
		*/
		case NM_ITEM_STALL_SELL_OPEN_REQ:
			try
			{
				if ( mType == _E_ST_ID_PVP_ )
					throw ERROR_ITEM_STALL_SELL_OPEN_FAIL;

				// KKR 튜토리얼맵에서 노점막기
				if ( mType == _E_ST_ID_TUTORIAL_ )
					throw ERROR_ITEM_STALL_SELL_OPEN_FAIL;
					
				hero->StallSellOpen( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_OPEN*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_OPEN_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_CLOSE_REQ:
			try
			{
				hero->StallSellClose( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_CLOSE*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_CLOSE_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_RENAME_REQ:
			try
			{
				hero->StallSellRename( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_RENAME*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_RENAME_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_ADD_REQ:
			try
			{
				hero->StallSellAdd( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_ADD*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_ADD_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_DEL_REQ:
			try
			{
				hero->StallSellDel( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_DEL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_DEL_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_MOD_REQ:
			try
			{
				hero->StallSellMod( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_MOD*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_MOD_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_JOIN_REQ:
			try
			{
				if ( clientInfo->complete.pvpJoin )
					throw ERROR_ITEM_STALL_SELL_JOIN_FAIL;

				// KKR 튜토리얼맵에서 노점막기
				if ( mType == _E_ST_ID_TUTORIAL_ )
					throw ERROR_ITEM_STALL_SELL_JOIN_FAIL;

				hero->StallSellJoin( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_JOIN*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_JOIN_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_LEAVE_REQ:
			try
			{
				hero->StallSellLeave( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_LEAVE*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_LEAVE_RES, error );
			}
			break;
		case NM_ITEM_STALL_SELL_GET_REQ:
			try
			{
				hero->StallSellGet( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_STALL_SELL_GET*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_STALL_SELL_GET_RES, error );
			}
			break;

		/*-- 줍기 관련 [열기 / 닫기 / 줍기]
		*/
		case NM_ITEM_GET_OPEN_REQ:
			try
			{
				hero->ItemGetOpen( (MSG_REQ_ITEM_GET_OPEN*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_GET_OPEN_RES, error );
			}
			break;
		case NM_ITEM_GET_CLOSE_REQ:
			try
			{
				hero->ItemGetClose( (MSG_REQ_ITEM_GET_CLOSE*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_GET_CLOSE_RES, error );
			}
			break;
		case NM_ITEM_GET_REQ:
			try
			{
				hero->ItemGet( (MSG_REQ_ITEM_GET*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_GET_RES, error );
			}
			break;
        case NM_ITEM_PARTY_GIVE_REQ:
			try
			{
                hero->ItemPartyGive( (MSG_REQ_ITEM_PARTY_GIVE*)msgRoot );
			}
			catch ( int error )
			{
                if ( error != ERROR_ITEM_PARTY_GIVE_SUCCESS )
				    SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_PARTY_GIVE_RES, error );
			}
            break;

		/*-- 컬렉션북 관련 [ 스프레드카드 등록/카드변환 ]
		*/
		case NM_ITEM_COLLECT_REQ:
			try
			{
				hero->ItemCollect( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_COLLECT*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_COLLECT_RES, error );
			}
			break;
        case NM_ITEM_COLLECT_CARD_CHANGE_REQ:
			try
			{
				hero->ItemCollectChange( (ULONG_PTR)perSocketContext, (MSG_REQ_COLLECT_CARD_CHANGE*)msgRoot );
			}
			catch ( int error )
			{
                if ( error !=  ERROR_ITEM_COLLECT_CARD_CHANGE_SUCCESS )
				    SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_COLLECT_CARD_CHANGE_RES, error );
			}
			break;

        case NM_ITEM_LICENSE_REQ:
			try
			{
				hero->ItemLicense( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_LICENSE*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_LICENSE_RES, error );
			}
			break;
        case NM_ITEM_CASH_LIST_REQ:
			try
			{
                if ( clientInfo->delay.cashList < currentTick )
                {
					clientInfo->delay.cashList = currentTick + MAX_CASH_SELECT;
                    hero->ItemCashInventory( (ULONG_PTR)perSocketContext );
                }
                else
					hero->SendInventoryCash( (ULONG_PTR)perSocketContext );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_CASH_LIST_RES, error );
			}
			break;
		case NM_ITEM_THEMERESET_REQ:
			try
			{
				MSG_REQ_ITEM_THEMERESET* pReset = (MSG_REQ_ITEM_THEMERESET*)msgRoot;

				int error = hero->ThemeResetReq( pReset->mSlotNum, pReset->mThemeNum, pReset->mThemeMode );
				if( error != ERROR_SKILLRESET_SUCCESS )
					throw error;
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_THEMERESET_RES, error );
			}
			break;

        case NM_ITEM_WAREHOUSE_SUMMON_OPEN_REQ:
			try
			{
                hero->ItemSummonWareHouse( (ULONG_PTR)perSocketContext, (MSG_REQ_ITEM_WAREHOUSE_SUMMON_OPEN*)msgRoot );
			}
			catch ( int error )
			{
                if ( error !=  ERROR_ITEM_WAREHOUSE_SUMMON_OPEN_SUCCESS )
				    SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_WAREHOUSE_SUMMON_OPEN_RES, error );
			}
			break;
        case NM_ITEM_WAREHOUSE_SUMMON_CLOSE_REQ:
            {
                if( hero->ChangeState( eOBJECT_STATE_IDLE ) == true )
                {
                    hero->SetStateStop( eSTOP_NONE );
                }
                else
                {
                    if ( hero->GetStateDie() == false )
                        hero->SetStateStop( eSTOP_NONE );

                    PostServerEvent( "ERROR - NM_ITEM_WAREHOUSE_SUMMON_CLOSE_REQ - ChangeState( eOBJECT_STATE_IDLE ) == false heroIdx(%d) ", 
                                     clientInfo->characterIdx );
                }

                // 다른 플레이어들에게도 알림
                MSG_SYN_ITEM_WAREHOUSE_SUMMON_CLOSE synMsg;
                synMsg.Category     = NM_TRADE;
                synMsg.Protocol     = NM_ITEM_WAREHOUSE_SUMMON_CLOSE_SYN;
                synMsg.characterIdx = hero->GetObjectID();
                QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
            }
            break;
		}
		break;	// switch ( msgRoot->Category )
	case NM_TRADE:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn / Inventory --*/
		if ( (clientInfo->completeData & 0x000F) != 0x000F )
		{
			PostServerEvent( "WRONG NM_TRADE::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_TRADE_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_TRADE::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_TRADE::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		if ( !clientInfo->complete.inventory )
			return false;

		switch ( msgRoot->Protocol )
		{
        case 1: break;
        default:break;
		/*-- 거래-사용자간 [요청 / 응답 / 등록 / 삭제 / 수정 / 완료 / 재수정 / 취소 / 종료]
		*/
/*
		case NM_ITEM_EXCHANGE_ASK_REQ:
			try
			{
				if ( mType == _E_ST_ID_PVP_ )
					throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

				// KKR 튜토리얼맵에서 노점막기
				if ( mType == _E_ST_ID_TUTORIAL_ )
					throw ERROR_ITEM_EXCHANGE_ASK_FAIL;

				hero->ExchangeAsk( (MSG_REQ_ITEM_EXCHANGE_ASK*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_ASK_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_REP_REQ:
			try
			{
				hero->ExchangeReply( (MSG_REQ_ITEM_EXCHANGE_REP*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_REP_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_ADD_REQ:
			try
			{
				hero->ExchangeAdd( (MSG_REQ_ITEM_EXCHANGE_ADD*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_ADD_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_DEL_REQ:
			try
			{
				hero->ExchangeDel( (MSG_REQ_ITEM_EXCHANGE_DEL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_DEL_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_OK_REQ:
			try
			{
				hero->ExchangeOk( (MSG_REQ_ITEM_EXCHANGE_OK*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_OK_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_RETRY_REQ:
			try
			{
				hero->ExchangeRetry( (MSG_REQ_ITEM_EXCHANGE_RETRY*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_RETRY_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_CANCEL_REQ:
			try
			{
				hero->ExchangeCancel( (MSG_REQ_ITEM_EXCHANGE_CANCEL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_CANCEL_RES, error );
			}
			break;
		case NM_ITEM_EXCHANGE_END_REQ:
			try
			{
				hero->ExchangeEnd( (MSG_REQ_ITEM_EXCHANGE_END*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_ITEM, NM_ITEM_EXCHANGE_END_RES, error );
			}
			break;
*/
		/*-- 소환주문서 관련 [ 우편함/판매대행 ]
		*/

        case NM_TRADE_POST_SUMMON_OPEN_REQ:
			try
			{
                hero->ItemSummonPost( (ULONG_PTR)perSocketContext, (MSG_REQ_TRADE_POST_SUMMON_OPEN*)msgRoot );
			}
			catch ( int error )
			{
                if ( error !=  ERROR_TRADE_POST_SUMMON_OPEN_SUCCESS )
				    SendMsgError( perSocketContext, NM_TRADE, NM_TRADE_POST_SUMMON_OPEN_RES, error );
			}
			break;
        case NM_TRADE_POST_SUMMON_CLOSE_REQ:
            {
                if( hero->ChangeState( eOBJECT_STATE_IDLE ) == true )
                {
                    hero->SetStateStop( eSTOP_NONE );
                }
                else
                {
                    if ( hero->GetStateDie() == false )
                        hero->SetStateStop( eSTOP_NONE );

                    PostServerEvent( "ERROR - NM_TRADE_POST_SUMMON_OPEN_REQ - ChangeState( eOBJECT_STATE_IDLE ) == false heroIdx(%d) ", 
                                     clientInfo->characterIdx );
                }

                // 다른 플레이어들에게도 알림
                MSG_SYN_TRADE_POST_SUMMON_CLOSE synMsg;
                synMsg.Category     = NM_TRADE;
                synMsg.Protocol     = NM_TRADE_POST_SUMMON_CLOSE_SYN;
                synMsg.characterIdx = hero->GetObjectID();
                QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
            }
			break;
        case NM_TRADE_AGENT_SUMMON_OPEN_REQ:
			try
			{
                hero->ItemSummonAgent( (ULONG_PTR)perSocketContext, (MSG_REQ_TRADE_AGENT_SUMMON_OPEN*)msgRoot );
			}
			catch ( int error )
			{
                if ( error !=  ERROR_TRADE_AGENT_SUMMON_OPEN_SUCCESS )
				    SendMsgError( perSocketContext, NM_TRADE, NM_TRADE_AGENT_SUMMON_OPEN_RES, error );
			}
			break;
        case NM_TRADE_AGENT_SUMMON_CLOSE_REQ:
            {
                if( hero->ChangeState( eOBJECT_STATE_IDLE ) == true )
                {
                    hero->SetStateStop( eSTOP_NONE );
                }
                else
                {
                    if ( hero->GetStateDie() == false )
                        hero->SetStateStop( eSTOP_NONE );

                    PostServerEvent( "ERROR - NM_TRADE_AGENT_SUMMON_CLOSE_REQ - ChangeState( eOBJECT_STATE_IDLE ) == false heroIdx(%d) ", 
                                     clientInfo->characterIdx );
                }

                // 다른 플레이어들에게도 알림
                MSG_SYN_TRADE_AGENT_SUMMON_CLOSE synMsg;
                synMsg.Category     = NM_TRADE;
                synMsg.Protocol     = NM_TRADE_AGENT_SUMMON_CLOSE_SYN;
                synMsg.characterIdx = hero->GetObjectID();
                QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
            }
            break;

		}
		break;		// switch ( msgRoot->Category )
	case NM_CHAT:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_CHAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_CHAT_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_CHAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_CHAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		// 채팅지연 - SKIP - 버퍼링 사용중.
		//if ( clientInfo->delay.chatting > currentTick )
		//	return true;
		//else
		//	clientInfo->delay.chatting = (currentTick + 100);

		switch( msgRoot->Protocol )
		{
		case NM_CHAT_NORMAL_REQ:
			if ( hero->IsBlock( ) == true )
			{
				HANDLE        handle = NULL;
				MSG_RES_CHAT* msg    = (MSG_RES_CHAT*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_NORMAL_RES );
					msg->ErrorCode = ERROR_CHAT_BLOCK;
					msg->validTrue = hero->mBlockValidThru;
				SendMsgRoot( handle, sizeof(MSG_RES_CHAT) );
			}
			else if ( packetLen <= sizeof(MSG_REQ_CHAT) )
			{
				MSG_REQ_CHAT* recvMsg = (MSG_REQ_CHAT*)msgRoot;

				u_int headLen = sizeof(MSG_REQ_CHAT) - sizeof(recvMsg->Message);
				u_int bodyLen = packetLen - headLen;
				char* bodyPtr = (char*)(recvMsg)+headLen;

				if ( bodyLen < sizeof(recvMsg->Message) )
				{
					MSG_SYN_CHAT sendMsg;
					memset( &sendMsg, 0, sizeof(sendMsg) );

					sendMsg.Category     = NM_CHAT;
					sendMsg.Protocol     = NM_CHAT_NORMAL_SYN;
					sendMsg.CharacterIdx = heroIdx;
					sendMsg.emotionIdx	 = recvMsg->emotionIdx;
                    sendMsg.Job          = (unsigned short)hero->GetJob();
					wcscpy( sendMsg.Name, hero->GetName( ) );

					memcpy( (char*)sendMsg.Message, bodyPtr, bodyLen ); // wcscpy( sendMsg.Message, recvMsg->Message );
					sendMsg.Message[MAX_CHAT_SIZE]=0;

					TB_INVENTORY* inventory = hero->SelectInventory( recvMsg->InventoryNumber );
					if( hero->IsInventory( inventory ) == true )
					{
						Inventory2sInventory( &sendMsg.inventory, inventory );
					}
					else if( recvMsg->itemIndex > 0 )
					{
						TB_INVENTORY tempInven;
						memset( &tempInven, 0, sizeof(tempInven) );
						tempInven.itemDefineIndex = recvMsg->itemIndex;

						Inventory2sInventory( &sendMsg.inventory, &tempInven );
					}

					/// pvp 팀 메세지 발송체크
					if( hero->SendPvPTeamChat( (char*)&sendMsg, sendMsg.GetMsgLength( ) ) == false )
						QuickSend( hero, (char*)&sendMsg, sendMsg.GetMsgLength( ) );
				}
			}
			break;

		case NM_CHAT_SHOUT_REQ:
			if ( hero->IsBlock( ) == true )
			{
				HANDLE        handle = NULL;
				MSG_RES_CHAT* msg    = (MSG_RES_CHAT*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_NORMAL_RES );
					msg->ErrorCode = ERROR_CHAT_BLOCK;
					msg->validTrue = hero->mBlockValidThru;
				SendMsgRoot( handle, sizeof(MSG_RES_CHAT) );
			}
			else if ( packetLen <= sizeof(MSG_REQ_CHAT_SHOUT) )
			{
				MSG_REQ_CHAT_SHOUT* recvMsg = (MSG_REQ_CHAT_SHOUT*)msgRoot;

				u_int headLen = sizeof(MSG_REQ_CHAT_SHOUT) - sizeof(recvMsg->Message);
				u_int bodyLen = packetLen - headLen;
				char* bodyPtr = (char*)(recvMsg)+headLen;

				if ( bodyLen < sizeof(recvMsg->Message) )
				{
					MSG_SYN_CHAT_SHOUT sendMsg;
					memset( &sendMsg, 0, sizeof(sendMsg) );

					sendMsg.Category     = NM_CHAT;
					sendMsg.Protocol     = NM_CHAT_SHOUT_SYN;
					sendMsg.CharacterIdx = heroIdx;
                    sendMsg.Job          = (unsigned short)hero->GetJob();
					wcscpy( sendMsg.Name, hero->GetName( ) );
					memcpy( (char*)sendMsg.Message, bodyPtr, bodyLen );
					sendMsg.Message[MAX_CHAT_SIZE]=0;

					QuickSendMap( hero, (char*)&sendMsg, sendMsg.GetMsgLength( ) );

					/*-- 보류 - 전체 쏘기 (맵, 채널, 서버)
					cSender* sender = g_gameSrv->GetSender( );
					if ( sender != NULL )
					{
						sender->PostChatShout( MAKECID((WORD)mServerNum,(WORD)mChannelNum) , hero->GetMapNumber( ), sendMsg.CharacterIdx, sendMsg.Name, sendMsg.Message );
					}
					*/
				}
			}
			break;

		case NM_CHAT_GUILD_REQ:
			if ( mType != _E_ST_ID_PVP_ )
			{
				if ( hero->IsBlock( ) == true )
				{
					HANDLE        handle = NULL;
					MSG_RES_CHAT* msg    = (MSG_RES_CHAT*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_NORMAL_RES );
						msg->ErrorCode = ERROR_CHAT_BLOCK;
						msg->validTrue = hero->mBlockValidThru;
					SendMsgRoot( handle, sizeof(MSG_RES_CHAT) );
				}
				else if ( packetLen <= sizeof(MSG_REQ_CHAT_GUILD) )
				{
					MSG_REQ_CHAT_GUILD* recvMsg = (MSG_REQ_CHAT_GUILD*)msgRoot;

					u_int headLen = sizeof(MSG_REQ_CHAT_GUILD) - sizeof(recvMsg->Message);
					u_int bodyLen = packetLen - headLen;

					if ( bodyLen < sizeof(recvMsg->Message) )
					{
						mpGuildManager->SendGuildChat( hero, recvMsg->Message, bodyLen, recvMsg->InventoryNumber );

						/// SYN : 길드 채팅
						TB_INVENTORY* inventory = hero->SelectInventory( recvMsg->InventoryNumber );
						sInventory    heroInven;
						memset( &heroInven, 0, sizeof(heroInven) );

						if ( hero->IsInventory( inventory ) == true )
						{
							Inventory2sInventory( &heroInven, inventory );
						}
						else if( recvMsg->itemIndex > 0 )
						{
							TB_INVENTORY tempInven;
							memset( &tempInven, 0, sizeof(tempInven) );
							tempInven.itemDefineIndex = recvMsg->itemIndex;

							Inventory2sInventory( &heroInven, &tempInven );
						}

						if ( mpGuildManager->PostGuildChat( hero, recvMsg->Message, bodyLen, &heroInven ) == false )
						{
							PostGuildEvent( hero->GetObjectID(), hero->GetGuildIndex(), hero->GetGuildPosition(), "FAIL - PostGuildChat()" );
						}
					}
				}
			}
			break;

		case NM_CHAT_TRADE_REQ:
			if ( mType != _E_ST_ID_PVP_ )
			{
				if ( hero->IsBlock( ) == true )
				{
					HANDLE        handle = NULL;
					MSG_RES_CHAT* msg    = (MSG_RES_CHAT*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_NORMAL_RES );
						msg->ErrorCode = ERROR_CHAT_BLOCK;
						msg->validTrue = hero->mBlockValidThru;
					SendMsgRoot( handle, sizeof(MSG_RES_CHAT) );
				}
				else if( packetLen <= sizeof(MSG_REQ_CHAT_TRADE) )
				{
					MSG_REQ_CHAT_TRADE* recvMsg = (MSG_REQ_CHAT_TRADE*)msgRoot;

					u_int headLen = sizeof(MSG_REQ_CHAT_TRADE) - sizeof(recvMsg->Message);
					u_int bodyLen = packetLen - headLen;
					char* bodyPtr = (char*)(recvMsg)+headLen;

					if ( bodyLen < sizeof(recvMsg->Message) )
					{
						MSG_SYN_CHAT_TRADE sendMsg;
						memset( &sendMsg, 0, sizeof(sendMsg) );

						sendMsg.Category	= NM_CHAT;
						sendMsg.Protocol	= NM_CHAT_TRADE_SYN;
                        sendMsg.Job         = (unsigned short)hero->GetJob();
						wcscpy( sendMsg.Name, hero->GetName() );
						memcpy( (char*)sendMsg.Message, bodyPtr, bodyLen );
						sendMsg.Message[MAX_CHAT_SIZE]=0;

						TB_INVENTORY* inventory = hero->SelectInventory( recvMsg->InventoryNumber );
						if ( hero->IsInventory( inventory ) == true )
						{
							Inventory2sInventory( &sendMsg.inventory, inventory );
						}
						else if( recvMsg->itemIndex > 0 )
						{
							TB_INVENTORY tempInven;
							memset( &tempInven, 0, sizeof(tempInven) );
							tempInven.itemDefineIndex = recvMsg->itemIndex;

							Inventory2sInventory( &sendMsg.inventory, &tempInven );
						}
						QuickSendMap( hero, (char*)&sendMsg, sendMsg.GetMsgLength() );
					}
				}
			}
			break;

		case NM_CHAT_WHISPER_REQ:
			if ( hero->IsBlock( ) == true )
			{
				HANDLE        handle = NULL;
				MSG_RES_CHAT* msg    = (MSG_RES_CHAT*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_NORMAL_RES );
					msg->ErrorCode = ERROR_CHAT_BLOCK;
					msg->validTrue  = hero->mBlockValidThru;
				SendMsgRoot( handle, sizeof(MSG_RES_CHAT) );
			}
			else if ( packetLen <= sizeof(MSG_REQ_CHAT_WHISPER) )
			{
				MSG_REQ_CHAT_WHISPER* recvMsg  = (MSG_REQ_CHAT_WHISPER*)msgRoot;
				recvMsg->Name[ MAX_NAME_SIZE ] = 0;  // NULL 오류방지.

				u_int headLen = sizeof(MSG_REQ_CHAT_WHISPER) - sizeof(recvMsg->Message);
				u_int bodyLen = packetLen - headLen;
				char* bodyPtr = (char*)(recvMsg)+headLen;

				if ( (wcslen( recvMsg->Name ) > 0) &&
					 (recvMsg->InventoryNumber != (-1) || (bodyLen > 0 && bodyLen < sizeof(recvMsg->Message))) &&
					 (wcsicmp( recvMsg->Name, hero->GetName( ) ) != 0) )
				{
					cPlayer* findPlayer = mpObjectManager->GetPlayer( recvMsg->Name );

					/// 내 채널 접속중
					if ( findPlayer != NULL )
					{
						// 받는 캐릭터에게.
						/// 실제 게임 접속중인지 검사
						cPlayer* gameInUser = mpGridManager->GetPlayer( findPlayer->GetObjectID() );
						if( gameInUser != NULL )
						{
							// 옵션 상태 검사
							sGameOption* toOption1 = gameInUser->GetOptionData();
							if( toOption1->option1.rejectionWhisper == false )	
							{
								HANDLE                handle  = NULL;
								MSG_SYN_CHAT_WHISPER* sendMsg = (MSG_SYN_CHAT_WHISPER*)GetMsgRoot( &handle, gameInUser->GetConnectionIdx( ), NM_CHAT, NM_CHAT_WHISPER_SYN );
								if ( sendMsg != NULL )
								{
									/// 링크 아이템 정보 
									TB_INVENTORY* inventory = hero->SelectInventory( recvMsg->InventoryNumber );
									if ( hero->IsInventory( inventory ) == true )
									{
										Inventory2sInventory( &sendMsg->inventory, inventory );
									}
									else if( recvMsg->itemIndex > 0 )
									{
										TB_INVENTORY tempInven;
										memset( &tempInven, 0, sizeof(tempInven) );
										tempInven.itemDefineIndex = recvMsg->itemIndex;

										Inventory2sInventory( &sendMsg->inventory, &tempInven );
									}
                                    
                                    sendMsg->Job = (unsigned short)hero->GetJob();       // 보낸 캐릭터 직업
									wcscpy( sendMsg->Name, hero->GetName( ) );			 // 보낸 캐릭터 이름
									memcpy( (char*)sendMsg->Message, bodyPtr, bodyLen ); // wcscpy( sendMsg->Message, recvMsg->Message ); 
																						 // 보낸 메시지
									SendMsgRoot( handle, sendMsg->GetMsgLength( ) );	 // 발송.
								}
							}
						}

						// 보낸 캐릭터에게.
						if ( hero != NULL )
						{
							HANDLE                handle  = NULL;
							MSG_RES_CHAT_WHISPER* sendMsg = (MSG_RES_CHAT_WHISPER*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_WHISPER_RES );

							/// 링크 아이템 정보 
							TB_INVENTORY* inventory = hero->SelectInventory( recvMsg->InventoryNumber );

							if ( hero->IsInventory( inventory ) == true )
								Inventory2sInventory( &sendMsg->inventory, inventory );

							if ( gameInUser != NULL )
							{
								sGameOption* toOption1 = gameInUser->GetOptionData( );
								sendMsg->ErrorCode = (toOption1->option1.rejectionWhisper == true) ? ERROR_CHAT_WHISPER_OPTIONREFUSE : ERROR_CHAT_WHISPER_SUCCESS;
                                sendMsg->Job       = (unsigned short)gameInUser->GetJob();
								wcscpy( sendMsg->Name, recvMsg->Name );				 // 받는 캐릭터이름.
								memcpy( (char*)sendMsg->Message, bodyPtr, bodyLen ); // wcscpy( sendMsg->Message, recvMsg->Message );
																					 // 보낸 메시지.
								SendMsgRoot( handle, sendMsg->GetMsgLength( ) );	 // 발송.
							}
							else
							{
								sendMsg->ErrorCode = ERROR_CHAT_WHISPER_DISCONNECT;
								SendMsgRoot( handle, sendMsg->GetMsgLength( ) );	// 발송.
							}
						}
					}
					/// 내 채널 접속중 아님.
					else if ( !clientInfo->commonDb.characterLogin )
					{
						/// 내 채널 접속중이 아니면, DB 검색
						clientInfo->commonDb.characterLogin = WhisperSelect( perSocketContext, recvMsg->Name, recvMsg->Message, recvMsg->InventoryNumber );
					}
				}
			}
			break;
		case NM_CHAT_POST_OPEN_REQ:
			try {
				hero->PostOpen( (ULONG_PTR)perSocketContext, (MSG_REQ_CHAT_POST_OPEN*)msgRoot );
			} catch ( int error ) {
				SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_POST_OPEN_RES, error );
			}
			break;
		case NM_CHAT_POST_CLOSE_REQ:
			try {
				hero->PostClose( (ULONG_PTR)perSocketContext, (MSG_REQ_CHAT_POST_CLOSE*)msgRoot );
			} catch ( int error ) {
				SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_POST_CLOSE_RES, error );
			}
			break;
		case NM_CHAT_POST_READ_REQ:
			try {
				hero->PostRead( (ULONG_PTR)perSocketContext, (MSG_REQ_CHAT_POST_READ*)msgRoot );
			} catch ( int error ) {
				SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_POST_READ_RES, error );
			}
			break;
		case NM_CHAT_POST_WRITE_REQ:
			try {
				hero->PostWrite( (ULONG_PTR)perSocketContext, (MSG_REQ_CHAT_POST_WRITE*)msgRoot );
			} catch ( int error ) {
				SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_POST_WRITE_RES, error );
			}
			break;
		case NM_CHAT_POST_DOWNLOAD_REQ:
			try {
				hero->PostDownload( (ULONG_PTR)perSocketContext, (MSG_REQ_CHAT_POST_DOWNLOAD*)msgRoot );
			} catch ( int error ) {
				SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_POST_DOWNLOAD_RES, error );
			}
			break;
		case NM_CHAT_POST_DELETE_REQ:
			try {
				MSG_REQ_CHAT_POST_DELETE* msg = (MSG_REQ_CHAT_POST_DELETE*)msgRoot;
				if ( msg->GetMsgLength( ) == packetLen )
				{
					hero->PostDelete( (ULONG_PTR)perSocketContext, msg );
				}
			} catch ( int error ) {
				SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_POST_DELETE_RES, error );
			}
			break;

		case NM_CHAT_FRIEND_ADD_REQ:
			if ( packetLen <= sizeof(MSG_REQ_FRIEND_ADD) && !clientInfo->commonDb.friendInsert )
			{
				MSG_REQ_FRIEND_ADD* recvMsg  = (MSG_REQ_FRIEND_ADD*)msgRoot;
				recvMsg->Name[ MAX_NAME_SIZE ] = 0; // NULL 오류방지.

				// 동일 이름일경우 & 이름이 없는 경우 무시 .
				if ( wcslen( recvMsg->Name ) > 0 && wcsicmp( recvMsg->Name, hero->GetName( ) ) != 0 )
				{
					HANDLE         handle       = NULL;
					FRIEND_INSERT* friendInsert = (FRIEND_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_FRIEND_INSERT );

					friendInsert->characterIdx1 = heroIdx;
					wcscpy( friendInsert->characterName, recvMsg->Name );

					SendSQL( perSocketContext, handle, sizeof(FRIEND_INSERT), COMMON_DB_FRIEND_INSERT );
				}
			}
			break;
		case NM_CHAT_FRIEND_DEL_REQ:
			if ( !clientInfo->commonDb.friendDelete )
			{
				MSG_REQ_FRIEND_DEL* msg = (MSG_REQ_FRIEND_DEL*)msgRoot;

				HANDLE         handle       = NULL;
				FRIEND_DELETE* friendDelete = (FRIEND_DELETE*)GetSQL( &handle, SQL_GAME_PROCESS_FRIEND_DELETE );

				friendDelete->idx           = msg->Idx;
				friendDelete->characterIdx1 = heroIdx;

				SendSQL( perSocketContext, handle, sizeof(FRIEND_DELETE), COMMON_DB_FRIEND_DELETE );
			}
			break;
		case NM_CHAT_FRIEND_REP_REQ:
			if ( !clientInfo->commonDb.friendUpdate )
			{
				MSG_REQ_FRIEND_REP* msg = (MSG_REQ_FRIEND_REP*)msgRoot;

				HANDLE         handle       = NULL;
				FRIEND_UPDATE* friendUpdate = (FRIEND_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_FRIEND_UPDATE );

				friendUpdate->idx           = msg->Idx;
				friendUpdate->characterIdx1 = heroIdx;

				SendSQL( perSocketContext, handle, sizeof(FRIEND_UPDATE), COMMON_DB_FRIEND_UPDATE );
			}
			break;

		case NM_CHAT_PARTY_REQ:
			if ( hero->IsBlock( ) == true )
			{
				HANDLE        handle = NULL;
				MSG_RES_CHAT* msg    = (MSG_RES_CHAT*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_NORMAL_RES );
					msg->ErrorCode = ERROR_CHAT_BLOCK;
					msg->validTrue  = hero->mBlockValidThru;
				SendMsgRoot( handle, sizeof(MSG_RES_CHAT) );
			}
			else if ( packetLen <= sizeof(MSG_REQ_CHAT_PARTY) )
			{
				MSG_REQ_CHAT_PARTY* recvMsg = (MSG_REQ_CHAT_PARTY*)msgRoot;

				u_int headLen = sizeof(MSG_REQ_CHAT_PARTY) - sizeof(recvMsg->Message);
				u_int bodyLen = packetLen - headLen;

				TB_INVENTORY*	inventory = hero->SelectInventory( recvMsg->InventoryNumber );
				bool IsInventory = hero->IsInventory( inventory );

				// 파티가 있으면 send
				unsigned long partyIndex = hero->GetPartyIndex( );
				cParty* party = mpPartyManager->GetParty( partyIndex );

				if ( party != NULL )
				{
					PARTY_DATA_SYNC sync;
					memset( &sync, 0, sizeof(sync) );

					sync.protocol = PSP_CHT_PARTY;
					sync.chtParty.partyIdx = party->GetIndex( );
					if ( IsInventory )
					{
						Inventory2sInventory( &sync.chtParty.inventory, inventory );
					}
					else if ( recvMsg->itemIndex > 0 )
					{
						sync.chtParty.inventory.ItemIndex = recvMsg->itemIndex;
					}
					sync.chtParty.playerIdx = heroIdx;
                    sync.chtParty.job       = (unsigned short)hero->GetJob();
					wcscpy( sync.chtParty.Name, hero->GetName( ) );
					memcpy( sync.chtParty.Message, recvMsg->Message, bodyLen );

					cSender* sender = g_gameSrv->GetSender( );
					sender->PostPartySync( GetCID( ), (char*)&sync, sync.CHTPARTY( ) );

					party->SendChat( &sync.chtParty.inventory, 
                                      sync.chtParty.Name, 
                                      sync.chtParty.Message, 
                                      heroIdx, 
                                      (unsigned short)hero->GetJob() );
				}
				else
				{
					// 파티연합중이면
					cPartyUnion* partyUnion = mpPartyUnionManager->GetUnion( hero->GetPartyUnionIndex() );
					if( partyUnion != NULL )
					{
						unsigned long* userArr = partyUnion->GetUserArr();

						for ( unsigned int i = 0; i < MAX_PARTYUNION; ++i, ++userArr )
						{ 
							cPlayer* user = OBJECTMANAGER->GetPlayer( (*userArr) );
							if ( user != NULL )
							{
								HANDLE              handle  = NULL;
								MSG_SYN_CHAT_PARTYUNION* sendMsg = (MSG_SYN_CHAT_PARTYUNION*)GetMsgRoot( &handle, user->GetConnectionIdx( ), NM_CHAT, NM_CHAT_PARTYUNION_SYN );
								if ( sendMsg != NULL )
								{
									wcscpy( sendMsg->Name, hero->GetName( ) );
									memcpy( sendMsg->Message, recvMsg->Message, bodyLen );      //wcscpy( sendMsg->Message, recvMsg->Message );

									sendMsg->playerIndex = heroIdx;
                                    sendMsg->job         = (unsigned short)hero->GetJob();

									if( IsInventory )
									{
										Inventory2sInventory( &sendMsg->inventory, inventory );
									}
									else if( recvMsg->itemIndex > 0 )
									{
										TB_INVENTORY tempInven;
										memset( &tempInven, 0, sizeof(tempInven) );
										tempInven.itemDefineIndex = recvMsg->itemIndex;

										Inventory2sInventory( &sendMsg->inventory, &tempInven );
									}

									SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
								}
							}
						}
					}
				}
				
			}
			break;

		case NM_CHAT_GM_NORMAL_REQ:
			if ( packetLen <= sizeof(MSG_REQ_CHAT) )
			{
				MSG_REQ_CHAT* recvMsg = (MSG_REQ_CHAT*)msgRoot;

				u_int headLen = sizeof(MSG_REQ_CHAT) - sizeof(recvMsg->Message);
				u_int bodyLen = packetLen - headLen;
				char* bodyPtr = (char*)(recvMsg)+headLen;

				if ( bodyLen < sizeof(recvMsg->Message) )
				{
					MSG_SYN_CHAT sendMsg;
					memset( &sendMsg, 0, sizeof(sendMsg) );

					sendMsg.Category     = NM_CHAT;
					sendMsg.Protocol     = NM_CHAT_GM_NORMAL_SYN;
					sendMsg.CharacterIdx = heroIdx;
					sendMsg.emotionIdx	 = recvMsg->emotionIdx;
					wcscpy( sendMsg.Name, hero->GetName( ) );
					memcpy( (char*)sendMsg.Message, bodyPtr, bodyLen ); // wcscpy( sendMsg.Message, recvMsg->Message );
					sendMsg.Message[MAX_CHAT_SIZE]=0;

					TB_INVENTORY*	inventory = hero->SelectInventory( recvMsg->InventoryNumber );
					if( hero->IsInventory( inventory ) == true )
					{
						Inventory2sInventory( &sendMsg.inventory, inventory );
					}
					else if( recvMsg->itemIndex > 0 )
					{
						TB_INVENTORY tempInven;
						memset( &tempInven, 0, sizeof(tempInven) );
						tempInven.itemDefineIndex = recvMsg->itemIndex;

						Inventory2sInventory( &sendMsg.inventory, &tempInven );
					}

					/// pvp 팀 메세지 발송체크
					if( hero->SendPvPTeamChat( (char*)&sendMsg, sendMsg.GetMsgLength( ) ) == false )
						QuickSend( hero, (char*)&sendMsg, sendMsg.GetMsgLength( ) );
				}
			}
			break;

		case NM_CHAT_MEGAPHONE_REQ:
			if ( hero->IsBlock( ) == true )
			{
				HANDLE                  handle  = NULL;
				MSG_RES_CHAT_MEGAPHONE* sendMsg = (MSG_RES_CHAT_MEGAPHONE*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_MEGAPHONE_RES );
					sendMsg->ErrorCode = ERROR_CHAT_MEGAPHONE_BLOCK;
					sendMsg->validTrue = hero->mBlockValidThru;
				SendMsgRoot( handle, sizeof(MSG_RES_CHAT_MEGAPHONE) );
			}
			else if ( packetLen <= sizeof(MSG_REQ_CHAT_MEGAPHONE) )
			{
				MSG_REQ_CHAT_MEGAPHONE* recvMsg = (MSG_REQ_CHAT_MEGAPHONE*)msgRoot;

				u_int headLen = sizeof(MSG_REQ_CHAT_MEGAPHONE) - sizeof(recvMsg->Message);
				u_int bodyLen = packetLen - headLen;
				char* bodyPtr = (char*)(recvMsg)+headLen;

				if ( bodyLen < sizeof(recvMsg->Message) )
				{
					/*-- 쿨타임 검사 및 적용. --*/
					if ( hero->IsInventoryMegaphone( recvMsg->InventoryIdx ) == false )
					{
						SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_MEGAPHONE_RES, ERROR_CHAT_MEGAPHONE_COOLTIME );
					}
					else
					{
						MSG_SYN_CHAT_MEGAPHONE sendMsg;
						memset( &sendMsg, 0, sizeof(sendMsg) );

						sendMsg.Category     = NM_CHAT;
						sendMsg.Protocol     = NM_CHAT_MEGAPHONE_SYN;
						sendMsg.ChannenNo    = mChannelNum;
						sendMsg.CharacterIdx = heroIdx;
						sendMsg.EmotionIdx   = recvMsg->EmotionIdx;
                        sendMsg.Job          = (unsigned short)hero->GetJob();
						wcscpy( sendMsg.Name, hero->GetName( ) );

						memcpy( (char*)sendMsg.Message, bodyPtr, bodyLen );
						sendMsg.Message[MAX_CHAT_SIZE]=0;

						TB_INVENTORY* inventory = hero->SelectInventory( recvMsg->InventoryNumber );
						if ( hero->IsInventory( inventory ) == true )
						{
							Inventory2sInventory( &sendMsg.Inventory, inventory );
						}
						else if ( recvMsg->ItemIndex > 0 )
						{
							TB_INVENTORY tempInven;
							memset( &tempInven, 0, sizeof(tempInven) );
							tempInven.itemDefineIndex = recvMsg->ItemIndex;

							Inventory2sInventory( &sendMsg.Inventory, &tempInven );
						}

						cSender* sender = g_gameSrv->GetSender( );
						if ( sender != NULL )
						{
							sender->PostChatMegaphone( MAKECID((WORD)mServerNum,(WORD)mChannelNum), (char*)&sendMsg, sendMsg.GetMsgLength( ) );
						}
					}
				}
			}
			break;
		}
		break; // NM_CHAT

	case NM_SKILL:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_SKILL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_SKILL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_SKILL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_SKILL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_SKILL_USED_REQ:
			{
				MSG_REQ_SKILL_USED* pmsg = (MSG_REQ_SKILL_USED*)msgRoot;

				// 스킬공격과 노멀 공격 구분
				if( pmsg->mSkillClassIndex <= NORMAL_ATTACK_SKILL_MAX )
				{
					SKILLMANAGER->PlayerNormalAttackRequest( heroIdx, pmsg );
				}
				else
				{
					SKILLMANAGER->PlayerObjectSkillRequest( heroIdx, pmsg, false );
				}
			}
			break;
		case NM_SKILL_CAST_USED_REQ:
			{
				MSG_REQ_SKILL_USED* pmsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerObjectSkillRequest( heroIdx, pmsg, true );
			}
			break;
		case NM_SKILL_POS_USED_REQ:
			{
				MSG_REQ_POS_USED* pmsg = (MSG_REQ_POS_USED*)msgRoot;
				SKILLMANAGER->PlayerPositionSkillRequest( heroIdx, pmsg, false );
			}break;
		case NM_SKILL_POS_CAST_USED_REQ:
			{
				MSG_REQ_POS_USED* pmsg = (MSG_REQ_POS_USED*)msgRoot;
				SKILLMANAGER->PlayerPositionSkillRequest( heroIdx, pmsg, true );
			}break;

		//case NM_SKILL_AUTOPOS_USED_REQ:
		//	{
		//		MSG_REQ_AUTOPOS_USED* pmsg = (MSG_REQ_AUTOPOS_USED*)msgRoot;
		//		SKILLMANAGER->PlayerAutoPosSkillRequest( heroIdx, pmsg, false );
		//	}
		//	break;
		//case NM_SKILL_AUTOPOS_CAST_USED_REQ:
		//	{
		//		MSG_REQ_AUTOPOS_USED* pmsg = (MSG_REQ_AUTOPOS_USED*)msgRoot;
		//		SKILLMANAGER->PlayerAutoPosSkillRequest( heroIdx, pmsg, true );
		//	}
		//	break;

		case NM_SKILL_USED_MON_REQ:
			{
				MSG_REQ_SKILL_USED* pmsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerMonSkillRequest( heroIdx, pmsg, false );
			}
			break;
		case NM_SKILL_CAST_USED_MON_REQ:
			{
				MSG_REQ_SKILL_USED* pmsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerMonSkillRequest( heroIdx, pmsg, true );
			}
			break;
		case NM_SKILL_POS_USED_MON_REQ:
			{
				MSG_REQ_POS_USED* pmsg = (MSG_REQ_POS_USED*)msgRoot;
				SKILLMANAGER->PlayerMonPosSkillRequest( heroIdx, pmsg, false );
			}break;
		case NM_SKILL_POS_CAST_USED_MON_REQ:
			{
				MSG_REQ_POS_USED* pmsg = (MSG_REQ_POS_USED*)msgRoot;
				SKILLMANAGER->PlayerMonPosSkillRequest( heroIdx, pmsg, true );
			}break;
		case NM_SKILL_ITEMMAPCHANGE_REQ:
			{
				MSG_REQ_SKILL_ITEMMAPCHANGE* pmsg = (MSG_REQ_SKILL_ITEMMAPCHANGE*)msgRoot;
				SKILLMANAGER->MapChangeSkillRequest( heroIdx, pmsg );
			}
			break;
		case NM_SKILL_ITEMVEHICLE_REQ:
			{
				MSG_REQ_SKILL_VEHICLE* pmsg = (MSG_REQ_SKILL_VEHICLE*)msgRoot;
				SKILLMANAGER->VehiCleSkillRequest( heroIdx, pmsg );
			}
			break;
		case NM_SKILL_CASHMAPCHANGE_REQ:
			{
				MSG_REQ_SKILL_CASHMAPCHANGE* pmsg = (MSG_REQ_SKILL_CASHMAPCHANGE*)msgRoot;
				SKILLMANAGER->CashMapChangeSkillRequest( heroIdx, pmsg );
			}
			break;
		case NM_SKILL_BLINK_REQ:
			{
				MSG_REQ_SKILL_BLINK* pmsg = (MSG_REQ_SKILL_BLINK*)msgRoot;
				SKILLMANAGER->BlinkSkillRequest( heroIdx, pmsg );
			}
			break;
		case NM_SKILL_NONSTOP_REQ:
			{
				MSG_REQ_SKILL_USED* pReqMsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerNonStopSkillRequest( heroIdx, pReqMsg );				
			}
			break;
		case NM_SKILL_NONSTOP_MON_REQ:
			{
				MSG_REQ_SKILL_USED* pReqMsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerMonNonStopSkillRequest( heroIdx, pReqMsg );				
			}
			break;
		case NM_SKILL_RUSH_REQ:
			{
				MSG_REQ_SKILL_USED* pReqMsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerRushSkillRequest( heroIdx, pReqMsg, false );				
			}
			break;
		case NM_SKILL_RUSHBLINK_REQ:
			{
				MSG_REQ_SKILL_USED* pReqMsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerRushSkillRequest( heroIdx, pReqMsg, true );				
			}
			break;
		case NM_SKILL_RUSHAFTER_REQ:
			{
				MSG_REQ_SKILL_USED* pReqMsg = (MSG_REQ_SKILL_USED*)msgRoot;
				SKILLMANAGER->PlayerRushAfterSkillRequest( heroIdx, pReqMsg );
			}
			break;
		case NM_SKILL_CAST_CANCEL_REQ:
			{
				MSG_REQ_SKILL_CAST_CANCEL* pReqMsg = (MSG_REQ_SKILL_CAST_CANCEL*)msgRoot;
				SKILLMANAGER->CastSkillCancel( pReqMsg->mAttacker, false, eATTRIBUTETYPE_NONE );				
			}
			break;
		case NM_SKILL_CHANNELING_CANCEL_REQ:
			{
				MSG_REQ_CHANNELING_SKILL_CANCEL* pReqMsg = (MSG_REQ_CHANNELING_SKILL_CANCEL*)msgRoot;
				SKILLMANAGER->ChannelingSkillCancel( pReqMsg->mAttacker, false, eATTRIBUTETYPE_NONE );				
			}
			break;
		case NM_SKILL_INFLUENCE_DELETE_REQ:
			{
				MSG_REQ_INFLUENCE_DELETE* msg = (MSG_REQ_INFLUENCE_DELETE*)msgRoot;
				SKILLMANAGER->ClientDeleteInfluenceObject( heroIdx, msg->mUniqueIdx );
			}
			break;
		case NM_SKILL_INFLUENCE_AURA_DELETE_REQ:
			{
				MSG_REQ_INFLUENCE_AURA_DELETE* msg = (MSG_REQ_INFLUENCE_AURA_DELETE*)msgRoot;

				if( SKILLMANAGER->ClientAuraInfluenceOff( heroIdx, msg->mUniqueIdx, msg->mSkillClassIdx ) == false )
					SKILLMANAGER->AuraInfluenceOff( heroIdx, msg->mSkillClassIdx );
			}
			break;
		case NM_SKILL_RESURRECTION_RES:	// 스킬 부활 확인
			{
				/// 부활 처리 통합
				hero->Resurrection( eRESURRECTIONTYPE_SKILL );

				//hero->SkillResurrection( );
				//// 부활 메세지 싱크
				//MSG_SYN_SKILL_RESURRECTION synMsg;

				//synMsg.Category = NM_SKILL;
				//synMsg.Protocol = NM_SKILL_RESURRECTION_SYN;
				//synMsg.mTarget.index = heroIdx;
				//synMsg.mTarget.type  = eOBJECTTYPE_PLAYER;
				//synMsg.mTargetHP = hero->GetHP();
				//synMsg.mTargetMP = hero->GetMP();

				//QuickSend( hero, (char*)&synMsg, sizeof(synMsg) );
			}
			break;
		case NM_SKILL_ADD_REQ:
			{
				MSG_RES_SKILL_ADD* msg = (MSG_RES_SKILL_ADD*)msgRoot;

				//////////////////////////////////////////////////////////////////////////
				/// 가능 여부 체크
				//////////////////////////////////////////////////////////////////////////

				/// 스킬 관련 DB 사용중
				if ( clientInfo->skillDb.skillDbUse == true || hero->IsDbUpdate() == true )
				{
					SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_DBERROR );                    
					break;
				}

				unsigned short needSP = 0;

				// skill 이외에 다른 객체에서 감산을 해야할 수치를 얻어옴
				if( !SKILLMANAGER->NeedAddPlayerHaveSkill( msg->mSkillClassIdx, msg->mSkillStep, &needSP ) )
				{
					/// 스크립트 오류
					SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_SCRIPT );
					break;
				}

				// 필요 sp 체크
				if( hero->GetSkillPoint() < needSP )
				{
					SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_SP );
					break;
				}

				//////////////////////////////////////////////////////////////////////////
				/// 실제 적용
				//////////////////////////////////////////////////////////////////////////

				/// 스킬 추가
				unsigned char errorcode = SKILLMANAGER->AddPlayerHaveSkill( heroIdx, msg->mSkillClassIdx, msg->mSkillStep, 0 );
				if( errorcode != SKILL_ADD_ERR_SUCCESS )
				{
					SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, errorcode );
					break;
				}

				/// 포인트 감소
				if( hero->SkillPointMinus( needSP ) == false )
				{
					// 필요항목 오류
					PostServerEvent( "ERROR - NM_SKILL_ADD_REQ - cPlayer(=%d)::SkillPointMinus return value is false.", heroIdx );
				}

				// 스킬 DB저장
				if( msg->mSkillStep == 0 )
				{
					// 스킬 신규생성 DB추가
					if( SkillInsert( perSocketContext, heroIdx, msg->mSkillClassIdx, needSP, hero->GetSkillLevel(), hero->GetSkillPoint() ) == true )
					{
						clientInfo->skillDb.skillDbUse = true;
						hero->PauseDBUpdate();	/// 스킬 업데이트중 player 정보를 업데이트 시키지 않는다.
					}
					else
						SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_DBERROR );
				}
				else
				{
					// 스킬 단계갱신 DB변경
					if( SkillUpdate( perSocketContext, heroIdx, msg->mSkillClassIdx, msg->mSkillStep, needSP, hero->GetSkillLevel(), hero->GetSkillPoint() ) == true )
					{
						clientInfo->skillDb.skillDbUse = true;
						hero->PauseDBUpdate();	/// 스킬 업데이트중 player 정보를 업데이트 시키지 않는다.
					}
					else
						SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_DBERROR );
				}
			}
			break;
		}
		break; // NM_SKILL

	case NM_TAROT:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_TAROT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_TAROT_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_TAROT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_TAROT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_TAROT_SEEKER_OPEN_REQ:
			try {
				if ( clientInfo->complete.pvpJoin )
					throw ERROR_TAROT_SEEKER_OPEN_FAIL;
				hero->TarotSeekerOpen( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SEEKER_OPEN*)msgRoot );
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SEEKER_OPEN_RES, error );
			}
			break;

		case NM_TAROT_SEEKER_CLOSE_REQ:
			try {
				hero->TarotSeekerClose( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SEEKER_CLOSE*)msgRoot );
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SEEKER_CLOSE_RES, error );
			}
			break;

		case NM_TAROT_SEEKER_JOIN_REQ:
			try {
				hero->TarotSeekerJoin( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SEEKER_JOIN*)msgRoot );
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SEEKER_JOIN_RES, error );
			}
			break;

		case NM_TAROT_SEEKER_RESULT_REQ:
			try {
				hero->TarotSeekerResult( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SEEKER_RESULT*)msgRoot );
				clientInfo->commonDb.spreadValue = true;
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SEEKER_RESULT_RES, error );
			}
			break;
		case NM_TAROT_SEEKER_ACCEPT_REQ:
			try {
				hero->TarotSeekerAccept( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SEEKER_ACCEPT*)msgRoot );
				clientInfo->commonDb.tarotResult = true;
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SEEKER_ACCEPT_RES, error );
			}
			break;

		case NM_TAROT_READER_READY_REQ:
			try {
				if ( clientInfo->complete.pvpJoin )
					throw ERROR_TAROT_READER_READY_FAIL;
				hero->TarotReaderReady( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_READER_READY*)msgRoot );
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_READER_READY_RES, error );
			}
			break;
		case NM_TAROT_READER_OPEN_REQ:
			try {
				hero->TarotReaderOpen( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_READER_OPEN*)msgRoot );
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_READER_OPEN_RES, error );
			}
			break;

		case NM_TAROT_READER_CLOSE_REQ:
			try {
				hero->TarotReaderClose( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_READER_CLOSE*)msgRoot );
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_READER_CLOSE_RES, error );
			}
			break;

		//-- 튜토리얼 타롯 
		case NM_TAROT_TUTORIAL_SEEKER_OPEN_REQ:
			try 
			{
				MSG_REQ_TAROT_TUTORIAL_SEEKER_OPEN* msg = (MSG_REQ_TAROT_TUTORIAL_SEEKER_OPEN*)msgRoot;

				if ( clientInfo->complete.pvpJoin )
					throw ERROR_TAROT_TUTORIAL_SEEKER_OPEN_FAIL;

				if( hero->GetMapDataNumber() != MAP_TUTORIAL )
					throw ERROR_TAROT_TUTORIAL_SEEKER_OPEN_FAIL;

				// 2차상태 체크
				if ( hero->IsRequestRejection() == true )
					throw ERROR_TAROT_TUTORIAL_SEEKER_OPEN_FAIL;			// 캐릭터 상태2오류.

				// 상태검사.
				if ( hero->GetStateStop( ) != eSTOP_NPCSPEECH )
					throw ERROR_TAROT_TUTORIAL_SEEKER_OPEN_FAIL;			// 캐릭터 상태오류.
				
				// npc 검사
				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( msg->npcIndex, hero, eNPCTALK_TAROT, 0 );
				if( retValue != ERROR_NPC_OPEN_SUCCESS )
				{
					PostServerEvent( "player[%d,%d,%d,%d] return[%d] NM_TAROT_TUTORIAL_SEEKER_OPEN_REQ retValue != ERROR_NPC_OPEN_SUCCESS",
												hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), msg->npcIndex, retValue );
					throw retValue;
				}
				
				throw ERROR_TAROT_TUTORIAL_SEEKER_OPEN_SUCCESS;
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_OPEN_RES, error );
			}
			break;
		case NM_TAROT_TUTORIAL_SEEKER_CLOSE_REQ:
			try {

				if( hero->GetMapDataNumber() != MAP_TUTORIAL )
					throw ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_FAIL;

				// 상태검사.
				if ( hero->GetStateStop( ) != eSTOP_NPCSPEECH )
					throw ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_FAIL;		// 캐릭터 상태오류 - 상태변경 불가.
			
				if ( hero->IsChangeState( eOBJECT_STATE_IDLE ) == false )
					throw ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_FAIL;		// 캐릭터 상태오류 - 상태변경 불가.

				// 상태변경.
				hero->ChangeState( eOBJECT_STATE_IDLE );
				hero->SetStateStop( eSTOP_NONE );

				throw ERROR_TAROT_TUTORIAL_SEEKER_CLOSE_SUCCESS;
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_CLOSE_RES, error );
			}
			break;
		case NM_TAROT_TUTORIAL_SEEKER_JOIN_REQ:
			try {
				hero->TarotTutorialSeekerJoin();
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_JOIN_RES, error );
			}
			break;
		case NM_TAROT_TUTORIAL_SEEKER_RESULT_REQ:
			try {
				hero->TarotTutorialSeekerResult( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_TUTORIAL_SEEKER_RESULT*)msgRoot );
				clientInfo->commonDb.spreadValue = true;
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_RESULT_RES, error );
			}
			break;
		case NM_TAROT_TUTORIAL_SEEKER_ACCEPT_REQ:
			try {
				hero->TarotTutorialSeekerAccept( (ULONG_PTR)perSocketContext );
				clientInfo->commonDb.tarotResult = true;
			}
			catch ( int error ) {
				SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_ACCEPT_RES, error );
			}
			break;
        case NM_TAROT_SELF_OPEN_REQ:
            try 
            {
                if ( clientInfo->complete.pvpJoin )
                    throw ERROR_TAROT_SELF_OPEN_FAIL;

                // 2차상태 체크
                if ( hero->IsRequestRejection() == true )
                    throw ERROR_TAROT_SELF_OPEN_FAIL;

                // 상태검사.
                if ( hero->IsChangeState( eOBJECT_STATE_STOP ) == false || hero->GetStateStop( ) != eSTOP_NONE )
                    throw ERROR_TAROT_SELF_OPEN_FAIL;

                // 상태변경.
                hero->ChangeState( eOBJECT_STATE_STOP );
                hero->SetStateStop( eSTOP_SELFTAROT );

                throw ERROR_TAROT_SELF_OPEN_SUCCESS;
            }
            catch ( int error )
            {
                SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SELF_OPEN_RES, error );
            }
			break;
        case NM_TAROT_SELF_CLOSE_REQ:
            try
            {
				if ( hero->GetStateStop( ) != eSTOP_SELFTAROT )
					throw ERROR_TAROT_SELF_CLOSE_FAIL;
			
				if ( hero->IsChangeState( eOBJECT_STATE_IDLE ) == false )
					throw ERROR_TAROT_SELF_CLOSE_FAIL;

				// 상태변경.
				hero->ChangeState( eOBJECT_STATE_IDLE );
				hero->SetStateStop( eSTOP_NONE );

				throw ERROR_TAROT_SELF_CLOSE_SUCCESS;
            }
            catch ( int error )
            {
                SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SELF_CLOSE_RES, error );
            }
			break;
        case NM_TAROT_SELF_JOIN_REQ:
            try 
            {
                throw hero->TarotSelfJoin( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SELF_JOIN*)msgRoot  );
            }
            catch ( int error )
            {
                if ( error != ERROR_TAROT_SELF_JOIN_SUCCESS )
                    SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SELF_JOIN_RES, error );
            }
			break;
        case NM_TAROT_SELF_RESULT_REQ:
            try
            {
				throw hero->TarotSelfResult( (ULONG_PTR)perSocketContext, (MSG_REQ_TAROT_SELF_RESULT*)msgRoot );
            }
            catch ( int error )
            {
                if ( error == ERROR_TAROT_SELF_RESULT_SUCCESS )
				    clientInfo->commonDb.spreadValue = true;
                else
                    SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SELF_RESULT_RES, error );
            }
			break;
        case NM_TAROT_SELF_ACCEPT_REQ:
            try
            {
                throw hero->TarotSelfAccept( (ULONG_PTR)perSocketContext ); 
            }
            catch ( int error )
            {
                if ( error == ERROR_TAROT_SELF_ACCEPT_SUCCESS )
                    clientInfo->commonDb.tarotResult = true;
                else
                    SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SELF_ACCEPT_RES, error );
            }
			break;
		}
		break; // NM_TAROT

	case NM_NPC:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_NPC::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_NPC_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_NPC::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_NPC::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch ( msgRoot->Protocol )
		{
		case NM_NPC_OPEN_REQ:	/// npc 대화 시작
			{
				MSG_REQ_NPC_OPEN* pmsg = (MSG_REQ_NPC_OPEN*)msgRoot;

				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->mNpcIdx, hero, eNPCTALK_OPEN, 0, (ULONG_PTR)perSocketContext );
				
				// 성공/실패 메세지 발송
				HANDLE            handle  = NULL;
				MSG_RES_NPC_OPEN* sendMsg = (MSG_RES_NPC_OPEN*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_OPEN_RES );

				sendMsg->ErrorCode = retValue;
				sendMsg->mNpcIdx   = pmsg->mNpcIdx;
				sendMsg->mPosX     = hero->GetXPos();
				sendMsg->mPosY     = hero->GetYPos();

				SendMsgRoot( handle, sizeof(MSG_RES_NPC_OPEN) );

				if( retValue == ERROR_NPC_OPEN_SUCCESS )
				{
					// 다른 플레이어들에게도 알림
					MSG_SYN_NPC_OPEN synMsg;
					synMsg.Category = NM_NPC;
					synMsg.Protocol = NM_NPC_OPEN_SYN;
					synMsg.mCharacterIdx = hero->GetObjectID();
					synMsg.mPosX = hero->GetXPos();
					synMsg.mPosY = hero->GetYPos();
					QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
				}
			}
			break;
		case NM_NPC_CLOSE_REQ:	/// 대화 종료
			{
				/// 대화중인 npc인덱스를 제거
				hero->SetNpcIdx( 0 );

				/// 상태 변경
				if( hero->ChangeState( eOBJECT_STATE_IDLE ) == true )
				{
					/// 정지 상태 변경
					hero->SetStateStop( eSTOP_NONE );

					// 다른 플레이어들에게도 알림
					MSG_SYN_NPC_CLOSE synMsg;
					synMsg.Category = NM_NPC;
					synMsg.Protocol = NM_NPC_CLOSE_SYN;
					synMsg.mCharacterIdx = hero->GetObjectID();
					QuickSendExcept( hero, (char*)&synMsg, sizeof(synMsg) );
				}
			}
			break;
        case NM_NPC_ITEM_MOV2_INVENTORY_REQ:
            {
                MSG_REQ_NPC_ITEM_MOV2_INVENTORY* pReqMsg = (MSG_REQ_NPC_ITEM_MOV2_INVENTORY*)msgRoot;
                try
                {
                    if ( hero->GetStateStop() == eSTOP_NPCSPEECH )
                    {
                        /// npc 인증
                        unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pReqMsg->npcIdx, hero, eNPCTALK_INVENTORY, 0 );
                        if( retValue != ERROR_NPC_OPEN_SUCCESS )
                        {
                            PostServerEvent( "player[%d,%d,%d] return[%d]NM_NPC_ITEM_MOV2_INVENTORY_REQ retValue != ERROR_NPC_OPEN_SUCCESS", 
                                              hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue );
                            throw ERROR_ITEM_MOV2_INVENTORY_NPC;
                        }
                    }
                    else
                    {
                        if ( hero->GetStateStop() != eSTOP_SUMMON_PAPER )
                        {
                            PostServerEvent( "ERROR player[%d,%d,%d] NM_NPC_ITEM_MOV2_INVENTORY_REQ GetStateStop() != eSTOP_SUMMON_PAPER", 
                                              hero->GetObjectID(), hero->GetState(), hero->GetStateStop() );
                            throw ERROR_ITEM_MOV2_INVENTORY_FAIL;
                        }
                    }

                    hero->ItemMov2Inventory( (ULONG_PTR)perSocketContext, pReqMsg );
                }
                catch ( int error )
                {
                    SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_MOV2_INVENTORY_RES, error );
                }
            }
            break;
        case NM_NPC_ITEM_SELL_REQ:
			try
			{
				MSG_REQ_NPC_ITEM_SELL* pReqMsg = (MSG_REQ_NPC_ITEM_SELL*)msgRoot;

				/// npc 인증
				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pReqMsg->npcIdx, hero, eNPCTALK_ITEMSELL, 0 );
				if( retValue != ERROR_NPC_OPEN_SUCCESS )
				{
					PostServerEvent( "NM_NPC_ITEM_SELL_REQ retValue != ERROR_NPC_OPEN_SUCCESS player[%d,%d,%d] return[%d]", 
						hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue );
					throw ERROR_ITEM_SELL_NPC;
				}

				hero->ItemSell( (ULONG_PTR)perSocketContext, pReqMsg );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_SELL_RES, error );
			}
			break;
		case NM_NPC_ITEM_BUY_REQ:
			try
			{
				MSG_REQ_NPC_ITEM_BUY* pReqMsg = (MSG_REQ_NPC_ITEM_BUY*)msgRoot;

				/// npc 인증
				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pReqMsg->npcIdx, hero, eNPCTALK_ITEMBUY, pReqMsg->itemDefineIndex );
				if(  retValue != ERROR_NPC_OPEN_SUCCESS )
				{
                    PostServerEvent( "NM_NPC_ITEM_BUY_REQ retValue != ERROR_NPC_OPEN_SUCCESS player[%d,%d,%d] return[%d]", 
                        hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue );
                    throw ERROR_ITEM_BUY_NPC;
                }

                hero->ItemBuy( (ULONG_PTR)perSocketContext, pReqMsg );
            }
            catch ( int error )
            {
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_BUY_RES, error );
			}
			break;
		case NM_NPC_DEPOSIT_REQ:
			try
			{
				hero->NpcDeposit( (MSG_REQ_NPC_DEPOSIT*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_DEPOSIT_RES, error );
			}
			break;
		case NM_NPC_WITHDRAW_REQ:
			try
			{
				hero->NpcWithdraw( (MSG_REQ_NPC_WITHDRAW*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_WITHDRAW_RES, error );
			}
			break;
		case NM_NPC_QUEST_ADD_REQ:
			{
				/// 퀘스트 신규 등록 요청 
				MSG_REQ_QUEST_ADD* pmsg = (MSG_REQ_QUEST_ADD*)msgRoot;

				try
				{
					/// DB 사용중인지 체크
					if( clientInfo->questDb.questInsert )
						throw ERROR_QUEST_ADD_NOTYET;

					/// npc 인증
					unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_QUESTADD, pmsg->questIndex );
					if( retValue != ERROR_NPC_OPEN_SUCCESS )
					{
						PostServerEvent( "player[%d,%d,%d,%d,%d] return[%d]NM_NPC_QUEST_ADD_REQ retValue != ERROR_NPC_OPEN_SUCCESS",
							hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue, pmsg->npcIndex, pmsg->questIndex );
						throw ERROR_QUEST_ADD_NPC;
					}

					/// 신규 가능한 퀘스트 조건 검색
					int errorCode = hero->IsRegistQuest( eQUESTADD_NPC, pmsg->questIndex, pmsg->npcIndex );
					if( errorCode != ERROR_QUEST_ADD_SUCCESS )
						throw errorCode;
			
					/// 신규퀘스트
					errorCode = hero->SaveQuestInsert( (ULONG_PTR)perSocketContext, heroIdx, pmsg->questIndex );
					if( errorCode != ERROR_QUEST_ADD_SUCCESS )
						throw errorCode;

					clientInfo->questDb.questInsert = true;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_ADD_RES, error );
				}
			}
			break;
		case NM_NPC_QUEST_COMPLETE_REQ:
			{
				/// 퀘스트 진행 완료 요청
				MSG_REQ_NPC_QUEST_COMPLETE* pmsg = (MSG_REQ_NPC_QUEST_COMPLETE*)msgRoot;

				try
				{
					/// DB 사용중인지 검사
					if( clientInfo->questDb.questComplete )
						throw ERROR_QUEST_COMPLETE_NOTYET;

					/// 보유중인 퀘스트인지 검사
					int dbIndex = hero->IsKeepQuest( pmsg->arrayIdx, pmsg->questIndex );
					if( dbIndex == -1 )
						throw ERROR_QUEST_COMPLETE_FAIL;

					/// npc 인증
					unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_QUESTREWARD, pmsg->questIndex );
					if( retValue != ERROR_NPC_OPEN_SUCCESS )
					{
						PostServerEvent( "player[%d,%d,%d,%d,%d] return[%d]NM_NPC_QUEST_COMPLETE_REQ retValue != ERROR_NPC_OPEN_SUCCESS", 
							hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue, pmsg->npcIndex, pmsg->questIndex );
						throw ERROR_QUEST_REWARD_NPC;
					}

					/// 완료가능한 퀘스트인지 검사 (의무조건 충족 검사)
					if( !mpQuestManager->IsPossibleComplete( heroIdx, pmsg->arrayIdx ) )
						throw ERROR_QUEST_COMPLETE_POSSIBLE;

					/// 진행완료 검사 및 요청 
					int errorCode = hero->SaveQuestComplete( (ULONG_PTR)perSocketContext, pmsg, dbIndex );
					if( errorCode != ERROR_QUEST_COMPLETE_SUCCESS )
						throw errorCode;

					clientInfo->questDb.questComplete = true;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_COMPLETE_RES, error );
				}
			}
			break;
		case NM_NPC_QUEST_REWARD_REQ:
			{
				/// 퀘스트 완료 요청 
				MSG_REQ_NPC_QUEST_REWARD* pmsg = (MSG_REQ_NPC_QUEST_REWARD*)msgRoot;

				try
				{
					/// DB 사용중인지 검사
					if( clientInfo->questDb.questReward )
						throw ERROR_QUEST_REWARD_NOTYET;

					/// 보유퀘스트 검사
					int dbIndex = hero->IsKeepQuest( pmsg->arrayIdx, pmsg->questIndex );
					if( dbIndex == -1 )
					{
						PostServerEvent( "ERROR NM_NPC_QUEST_REWARD_REQ IsKeepQuest (%d, %d, %d, %d)", heroIdx, pmsg->npcIndex, pmsg->questIndex, pmsg->arrayIdx );
						throw ERROR_QUEST_REWARD_FAIL;
					}

					/// npc 인증
					unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_QUESTREWARD, pmsg->questIndex );
					if( retValue != ERROR_NPC_OPEN_SUCCESS )
					{
						PostServerEvent( "player[%d,%d,%d,%d,%d] return[%d]NM_NPC_QUEST_REWARD_REQ retValue != ERROR_NPC_OPEN_SUCCESS", 
							hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue, pmsg->npcIndex, pmsg->questIndex );
						throw ERROR_QUEST_REWARD_NPC;
					}

					/// 보상용 검사 및 요청 
					int errorCode = hero->SaveQuestReward( (ULONG_PTR)perSocketContext, pmsg, dbIndex );
					if( errorCode != ERROR_QUEST_REWARD_SUCCESS )
					{
						if( errorCode == ERROR_QUEST_REWARD_FAIL || errorCode == ERROR_QUEST_REWARD_NPC || errorCode == ERROR_QUEST_REWARD_COMPLETE )
						{
							PostServerEvent( "ERROR NM_NPC_QUEST_REWARD_REQ SaveQuestReward (%d, %d, %d, %d, %d)", errorCode, heroIdx, pmsg->npcIndex, pmsg->questIndex, pmsg->arrayIdx );
						}
						throw errorCode;
					}

					clientInfo->questDb.questReward = true;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_REWARD_RES, error );
				}
			}
			break;
		case NM_NPC_GUILD_CREATE_REQ:
			{
				// 길드 생성 요청
				MSG_REQ_NPC_GUILD_CREATE* pmsg = (MSG_REQ_NPC_GUILD_CREATE*)msgRoot;
				try
				{
					// DB 사용중인지 체크
					if ( clientInfo->guildDb.guildCreate )
						throw ERROR_NPC_GUILD_CREATE_FAIL;

					// 1. npc 인증
					unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_GUILDCREATE, 0 );
					if( retValue != ERROR_NPC_OPEN_SUCCESS )
					{
						PostServerEvent( "player[%d,%d,%d,%d] return[%d]NM_NPC_GUILD_CREATE_REQ retValue != ERROR_NPC_OPEN_SUCCESS", 
													hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue, pmsg->npcIndex );
						throw ERROR_NPC_GUILD_CREATE_NPC;
					}

					// 2. 길드 가입중인지 검사
					sPlayerExrInfo* exrInfo = hero->GetExrInfo( );
					if ( exrInfo->mGuildIndex != 0 )
						throw ERROR_NPC_GUILD_CREATE_ALREADY;

					// 3. 길드명 검사
					if ( wcslen( pmsg->name ) == 0 || wcslen( pmsg->name ) > MAX_GUILDNAME_SIZE )
						throw ERROR_NPC_GUILD_CREATE_ERRORNAME;

					// 4. 중복검사
					if ( mpGuildManager->FindGuildName( pmsg->name ) )
						throw ERROR_NPC_GUILD_CREATE_EXISTNAME;

					// 5. 제한 조건검사 ( 레벨, 금액 )
					sPlayerInfo* info = hero->GetPlayerInfo();
					if( info->Level < GUILDLIMIT_LEVEL || hero->GetMoney() < GUILDLIMIT_MONEY )
						throw ERROR_NPC_GUILD_CREATE_CONDITION;

					// 세팅
					pmsg->name[ MAX_GUILDNAME_SIZE ] = 0;
					clientInfo->guildDb.guildCreate = GuildCreate( perSocketContext, heroIdx, pmsg->name, hero->GetName() ); 
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_GUILD_CREATE_RES, error );
				}
			}
			break;
		case NM_NPC_TAROT_OPEN_REQ:
			{
				/// 운세 비트 세팅후 리턴
				time_t    ltime;
				time( &ltime );

				unsigned long value = 0;
				unsigned long gaptime = 60*60*4;

				for( int i = 0; i < 5; ++i )
				{
					value = value << 4;

					TIMESTAMP_STRUCT* heroThru = hero->GetFortuneThru( i );
					if( heroThru )
					{
						/// 4시간 검사 
						tm herotm = TIMER->ConvertTStoTM( *heroThru );
						time_t heroTime = mktime( &herotm );

						if( heroTime != -1 && difftime( ltime, heroTime ) < gaptime )
						{
							value = value | 1;
						}
					}
					else
					{
						/// 오류
						PostServerEvent( "NM_NPC_TAROT_OPEN_REQ - not exist fortuneThru data, heroIdx = %d", hero->GetObjectID() );
						continue;
					}
				}

				HANDLE					handle = NULL;
				MSG_RES_NPC_TAROT_OPEN* sendmsg = (MSG_RES_NPC_TAROT_OPEN*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_TAROT_OPEN_RES );
					sendmsg->fortuneData = value;
				SendMsgRoot( handle, sizeof(MSG_RES_NPC_TAROT_OPEN) );
			}
			break;
		case NM_NPC_TAROT_JOIN_REQ:
			{
				// NPC 점집 이용 요청
				MSG_REQ_NPC_TAROT_JOIN* pmsg = (MSG_REQ_NPC_TAROT_JOIN*)msgRoot;

				try
				{
					// 1. npc 인증 및 금액 체크
					int retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_FORTUNETAROT, 0 );
					if( retValue != ERROR_NPC_OPEN_SUCCESS )
					{
						PostServerEvent( "player[%d,%d,%d,%d] return[%d]NM_NPC_TAROT_JOIN_REQ retValue != ERROR_NPC_OPEN_SUCCESS", 
							hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), pmsg->npcIndex, retValue );
						throw retValue;
					}

					/// 2. 금액 지불 처리
					cNpc* pNpc = GRIDMANAGER->GetNpc( pmsg->npcIndex );
					if( pNpc == NULL )
					{
						PostServerEvent("NM_NPC_TAROT_JOIN_REQ [%d,%d,%d,%d,%d] pNpc == NULL", pmsg->npcIndex, hero->GetNpcIdx(), hero->GetObjectID(), hero->GetState(), hero->GetStateStop() );
						throw ERROR_NPC_TAROT_JOIN_FAIL;
					}
					
					unsigned long npcClassIdx = pNpc->GetRaceGender();
					sNpcFuncData* data = NPCSCRIPT->GetNpcFuncData( npcClassIdx );
					if( !data )
						throw ERROR_NPC_TAROT_JOIN_FAIL;

					/// 금액 검사
					if( !(hero->GetMoney() >= data->mTarotPrice) )
						throw ERROR_NPC_TAROT_JOIN_MONEY;

					/// 차감
					hero->AddMoney( hero->GetObject( ), -((long)data->mTarotPrice) );

					// 성공 리턴
					throw ERROR_NPC_TAROT_JOIN_SUCESS;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_TAROT_JOIN_RES, error );	
				}
			}
			break;
		case NM_NPC_TAROT_FORTUNE_REQ:
			{
				MSG_REQ_NPC_TAROT_FORTUNE* pmsg = (MSG_REQ_NPC_TAROT_FORTUNE*)msgRoot;

				try
				{
					// 운세 체크
					TIMESTAMP_STRUCT* heroThru = hero->GetFortuneThru( pmsg->fortuneProp );
					if( heroThru == NULL )
						throw ERROR_NPC_TAROT_FORTUNE_FAIL;

					time_t    ltime;
					time( &ltime );

					tm presentThru = *localtime( &ltime );
					
					/// 4시간 검사 
					tm herotm = TIMER->ConvertTStoTM( *heroThru );
					time_t	heroTime = mktime( &herotm );

					unsigned long gaptime = 60*60*4;

					if( heroTime != -1 && difftime( ltime, heroTime ) < gaptime )
						throw ERROR_NPC_TAROT_FORTUNE_ONCE;

					TIMESTAMP_STRUCT thru = TIMER->ConvertTMtoTS( presentThru );

					// 운세 저장
					*heroThru = thru;

					// 성공 리턴
					throw ERROR_NPC_TAROT_FORTUNE_SUCESS;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_TAROT_FORTUNE_RES, error );	
				}
			}
			break;
		case NM_NPC_THEME_JOIN_REQ:
			{
				MSG_REQ_NPC_THEME_JOIN* pmsg = (MSG_REQ_NPC_THEME_JOIN*)msgRoot;

				/// npc 인증
				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_THEME, 0 );
				if ( retValue != ERROR_NPC_OPEN_SUCCESS )
				{
					PostServerEvent( "player[%d,%d,%d] return[%d]NM_NPC_THEME_JOIN_REQ retValue != ERROR_NPC_OPEN_SUCCESS"
											  ,hero->GetObjectID()
											  ,hero->GetState()
											  ,hero->GetStateStop()
											  ,retValue );

					SendMsgError( perSocketContext, NM_NPC, NM_NPC_THEME_JOIN_RES, retValue );
					break;
				}
				
				/// 테마 방번호 처리
				cNpc* pNpc = GRIDMANAGER->GetNpc( pmsg->npcIndex );
				if( pNpc != NULL )
				{
					unsigned long npcClassIdx = pNpc->GetRaceGender();
					sNpcFuncData* data = NPCSCRIPT->GetNpcFuncData( npcClassIdx );
					if( data != NULL )
					{
						if( MAP_THEME_MIN <= data->mThemeIndex && data->mThemeIndex <= MAP_THEME_MAX )
							THEMEMANAGER->ThemeRequest( hero->GetObjectID(), (unsigned short)data->mThemeIndex );
						else
						{
							PostServerEvent("NM_NPC_THEME_JOIN_REQ data->mThemeIndex[%d] ERROR", npcClassIdx, data->mThemeIndex );
							SendMsgError( perSocketContext, NM_NPC, NM_NPC_THEME_JOIN_RES, ERROR_NPC_OPEN_DEALLIST );
						}
					}
					else
					{
						PostServerEvent("NM_NPC_THEME_JOIN_REQ data[%d] != NULL", npcClassIdx );
						SendMsgError( perSocketContext, NM_NPC, NM_NPC_THEME_JOIN_RES, ERROR_NPC_OPEN_DEALLIST );
					}
				}
				else
				{
					PostServerEvent("NM_NPC_THEME_JOIN_REQ pNpc[%d] == NULL", pmsg->npcIndex );
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_THEME_JOIN_RES, ERROR_NPC_OPEN_DEALLIST );
				}
			}
			break;
			/*-- 판매대행 [등록/검색/구매]
			*/
		case NM_NPC_ITEM_AGENT_OPEN_REQ:
			try
			{
				///npc 인증하기
				MSG_REQ_NPC_ITEM_AGENT_OPEN* pmsg = (MSG_REQ_NPC_ITEM_AGENT_OPEN *)msgRoot;
				int retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIndex, hero, eNPCTALK_DEAL, 0 );
				if ( retValue != ERROR_NPC_OPEN_SUCCESS )
				{
					PostServerEvent( "player[%d,%d,%d] return[%d]NM_NPC_ITEM_AGENT_OPEN_REQ retValue != ERROR_NPC_OPEN_SUCCESS"
						,hero->GetObjectID()
						,hero->GetState()
						,hero->GetStateStop()
						,retValue );
					throw retValue;
				}

				if ( clientInfo->commonDb.itemAgentSearch )
					throw ERROR_NPC_ITEM_AGENT_OPEN_NOTYET;
				clientInfo->commonDb.itemAgentSearch = hero->ItemAgentPageOpen( (ULONG_PTR)perSocketContext, 1 );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_OPEN_RES, error );
			}
			break;
		case NM_NPC_ITEM_AGENT_OPEN_PAGE_REQ:
			try
			{
				if ( clientInfo->commonDb.itemAgentSearch )
					throw ERROR_NPC_ITEM_AGENT_OPEN_PAGE_NOTYET;
				clientInfo->commonDb.itemAgentSearch = hero->ItemAgentPageOpen( (ULONG_PTR)perSocketContext, ((MSG_REQ_NPC_ITEM_AGENT_PAGE_OPEN*)msgRoot)->StartPage  );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_OPEN_PAGE_RES, error );
			}
			break;
		case NM_NPC_ITEM_AGENT_ADD_REQ:
			try
			{
				hero->ItemAgentAdd( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_ADD*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_ADD_RES, error );
			}
			break;
		case NM_NPC_ITEM_AGENT_DEL_REQ:
			try
			{
				hero->ItemAgentDel( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_DEL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_DEL_RES, error );
			}
			break;
		case NM_NPC_ITEM_AGENT_GET_REQ:
			try
			{
				hero->ItemAgentGet( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_GET*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_GET_RES, error );
			}
			break;
		/*case NM_NPC_ITEM_AGENT_SEARCH_REQ:
			try
			{
				if ( clientInfo->commonDb.itemAgentSearch )
					throw ERROR_NPC_ITEM_AGENT_SEARCH_NOTYET;
				clientInfo->commonDb.itemAgentSearch = hero->ItemAgentSearch( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_SEARCH*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES, error );
			}
			break;
		case NM_NPC_ITEM_AGENT_SEARCH_NAME_REQ:
			try
			{
				if ( clientInfo->commonDb.itemAgentSearch )
					throw ERROR_NPC_ITEM_AGENT_SEARCH_NOTYET;
				clientInfo->commonDb.itemAgentSearch = hero->ItemAgentSearchName( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_SEARCH_NAME*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES, error );
			}
			break;*/
		case NM_NPC_ITEM_AGENT_SEARCH_TYPE_REQ:
			try
			{
				if ( clientInfo->commonDb.itemAgentSearch )
					throw ERROR_NPC_ITEM_AGENT_SEARCH_NOTYET;
				clientInfo->commonDb.itemAgentSearch = hero->ItemAgentSearchType( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_SEARCH_TYPE*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES, error );
			}
			break;
		case NM_NPC_ITEM_AGENT_SEARCH_ALL_REQ:
			try
			{
				if ( clientInfo->commonDb.itemAgentSearch )
					throw ERROR_NPC_ITEM_AGENT_SEARCH_NOTYET;
				clientInfo->commonDb.itemAgentSearch = hero->ItemAgentSearchAll( (ULONG_PTR)perSocketContext, (MSG_REQ_NPC_ITEM_AGENT_SEARCH_ALL*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES, error );
			}
			break;

		case NM_NPC_OBT_EVENT_REQ:
			try
			{
				//MSG_REQ_NPC_OBT_EVENT* pmsg = (MSG_REQ_NPC_OBT_EVENT*)msgRoot;

				/// npc 인증
//				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pmsg->npcIdx, hero, eNPCTALK_EVENT, 0 );
//				if( retValue != ERROR_NPC_OPEN_SUCCESS )
//				{
//					PostServerEvent( "player[%d,%d,%d] return[%d]NM_NPC_OBT_EVENT_REQ retValue != ERROR_NPC_OPEN_SUCCESS", 
//						hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue );
//					throw ERROR_NPC_OBT_EVENT_NPC;
//				}

				hero->ItemObtEvent( (ULONG_PTR)perSocketContext, clientInfo->cbtUser, clientInfo->obtUser, clientInfo->fourGamer );
			}
			catch ( int error )
			{
				if ( error != ERROR_NPC_OBT_EVENT_SUCCESS )
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_OBT_EVENT_RES, ERROR_NPC_OBT_EVENT_FAIL );
			}
			break;

		case NM_NPC_EVENT_ITEM_GIVE_REQ:
			try
			{
				MSG_REQ_NPC_EVENT_ITEM_GIVE* pMsg = (MSG_REQ_NPC_EVENT_ITEM_GIVE*)msgRoot;

				// npc 인증
				unsigned long retValue = OBJECTMANAGER->IsNpcTalk( pMsg->npcIndex, hero, eNPCTALK_EVENT, 0 );
				if( retValue != ERROR_NPC_OPEN_SUCCESS )
				{
					PostServerEvent( "NM_NPC_EVENT_ITEM_GIVE_REQ retValue != ERROR_NPC_OPEN_SUCCESS player[%d,%d,%d] return[%d]", 
						hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), retValue );
					throw ERROR_NPC_EVENT_ITEM_GIVE_NPC;
				}

				hero->ItemNpcEvent( (ULONG_PTR)perSocketContext, pMsg->eventIndex );
			}
			catch ( int error )
			{
				if ( error != ERROR_NPC_EVENT_ITEM_GIVE_SUCCESS )
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_EVENT_ITEM_GIVE_RES, error );
			}
			break;
		}
		break; // NM_NPC

	case NM_CHEAT:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_CHEAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- ClientInfo::SLEVEL 필터링 --*/
		if ( clientInfo->slevel != 1 )
		{
			PostServerEvent( "END TASK - SLEVEL(=%d) NM_CHEAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 clientInfo->slevel,
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_CHEAT_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_CHEAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_CHEAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_CHEAT_GM_NAME_REQ:
			if ( clientInfo->commonDb.characterGmName == false )
			{
				try
				{
					MSG_REQ_CHEAT_GM_NAME* recvMsg =  (MSG_REQ_CHEAT_GM_NAME*)msgRoot;
					recvMsg->mCharName[MAX_NAME_SIZE] = 0;

					wchar_t* name = hero->GetName( );

					if ( wcscmp( name, recvMsg->mCharName ) == 0 )
						throw ERROR_CHEAT_GM_NAME_ALREADY;

					HANDLE             handle = NULL;
					CHARACTER_GM_NAME* gmName = (CHARACTER_GM_NAME*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_GM_NAME );

					gmName->characterIndex = heroIdx;
					::wcscpy( gmName->characterName, recvMsg->mCharName );

					clientInfo->commonDb.characterGmName = SendSQL( perSocketContext, handle, sizeof(CHARACTER_GM_NAME) );
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_NAME_RES, error );
				}
			}
			break;
		case NM_CHEAT_SPEEDUP_REQ:
			hero->SetCheatSpeedUp( ((MSG_REQ_CHEAT_SPEEDUP*)msgRoot)->mSpeedUp );
			break;
		case NM_CHEAT_HIDE_REQ:
			{
				MSG_REQ_CHEAT_HIDE* recvMsg = (MSG_REQ_CHEAT_HIDE*)msgRoot;

				if( hero->SetCheatHideMode( recvMsg->mMode ) == true )
				{
					HANDLE              handle  = NULL;
					MSG_RES_CHEAT_HIDE* sendMsg = (MSG_RES_CHEAT_HIDE*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_HIDE_RES );
						sendMsg->mMode = recvMsg->mMode;
					SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_HIDE) );
				}
			}
			break;
		case NM_CHEAT_UNDEAD_REQ:
			{
				MSG_REQ_CHEAT_UNDEAD* recvMsg = (MSG_REQ_CHEAT_UNDEAD*)msgRoot;

				hero->SetCheatUndeadMode( recvMsg->mMode );

				HANDLE                handle  = NULL;
				MSG_RES_CHEAT_UNDEAD* sendMsg = (MSG_RES_CHEAT_UNDEAD*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_UNDEAD_RES );
					sendMsg->mMode = recvMsg->mMode;
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_UNDEAD) );
			}
			break;
		case NM_CHEAT_INFO_REQ:
			{
				MSG_REQ_CHEAT_INFO* recvMsg = (MSG_REQ_CHEAT_INFO*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					HANDLE              handle  = NULL;
					MSG_RES_CHEAT_INFO* sendMsg = (MSG_RES_CHEAT_INFO*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_INFO_RES );
						sendMsg->mMapNum = player->GetMapNumber( );
						sendMsg->mPosX   = player->GetXPos( );
						sendMsg->mPosY   = player->GetYPos( );
						sendMsg->mJob    = (unsigned short)player->GetJob( );
						sendMsg->mLevel  = player->GetLevel( );
						sendMsg->mMoney  = player->GetMoney( );
					SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_INFO) );
				}
			}
			break;
		case NM_CHEAT_KICK_REQ:
			{
				MSG_REQ_CHEAT_KICK* recvMsg = (MSG_REQ_CHEAT_KICK*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				HANDLE					handle = NULL;
				CHARACTER_KICK_SELECT*  characterKickSelect = (CHARACTER_KICK_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_KICK_SELECT );
				if( handle )
				{
					wcscpy_s( characterKickSelect ->characterName, recvMsg->mCharName );
					characterKickSelect->kickCharacterIdx = 0;
					characterKickSelect->channelNum       = -1;

					SendSQL( perSocketContext, handle, sizeof(CHARACTER_KICK_SELECT) );
				}
				else
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_KICK_RES_GM, ERROR_CHEAT_KICK_FAIL );
				}

			}
			break;
		case NM_CHEAT_RECOVER_REQ:
			{
				hero->CheatRecover( );

				HANDLE                 handle  = NULL;
				MSG_RES_CHEAT_RECOVER* sendMsg = (MSG_RES_CHEAT_RECOVER*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_RECOVER_RES );
					sendMsg->mMaxHP = hero->GetHP( );
					sendMsg->mMaxMP = hero->GetMP( );
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_RECOVER) );

				MSG_SYN_CHEAT_RECOVER syn;
					syn.Category  = NM_CHEAT;
					syn.Protocol  = NM_CHEAT_RECOVER_SYN;
					syn.mObjIndex = heroIdx;
					syn.mMaxHP    = hero->GetHP();
					syn.mMaxMP    = hero->GetMP();
				QuickSendExcept( hero, (char*)&syn, sizeof(MSG_SYN_CHEAT_RECOVER) );
			}
			break;
		case NM_CHEAT_REBIRTH_REQ:
			{
				hero->CheatRebirth( );

				HANDLE                 handle  = NULL;
				MSG_RES_CHEAT_REBIRTH* sendMsg = (MSG_RES_CHEAT_REBIRTH*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_REBIRTH_RES );
					sendMsg->mMaxHP = hero->GetHP();
					sendMsg->mMaxMP = hero->GetMP();
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_REBIRTH) );

				MSG_SYN_CHEAT_REBIRTH syn;
					syn.Category  = NM_CHEAT;
					syn.Protocol  = NM_CHEAT_REBIRTH_SYN;
					syn.mObjIndex = heroIdx;
					syn.mMaxHP    = hero->GetHP();
					syn.mMaxMP    = hero->GetMP();
				QuickSendExcept( hero, (char*)&syn, sizeof(MSG_SYN_CHEAT_REBIRTH) );
			}
			break;
		case NM_CHEAT_TELEPORT_REQ:
			{
				MSG_REQ_CHEAT_TELEPORT* recvMsg = (MSG_REQ_CHEAT_TELEPORT*)msgRoot;

				if( hero->IsChangeState( eOBJECT_STATE_MOVE ) == true )
				{
					// 061208 PKH 이동 목적지를 설정
					hero->SetMoveTargetPos( recvMsg->mDestX, recvMsg->mDestY );
					hero->SetPos( recvMsg->mDestX, recvMsg->mDestY );
					hero->ClearMoveSyncCalc();

					if( hero->GetCheatHideMode( ) == false )
					{
						// 다른 플레이어들에게도 알림
						MSG_SYN_CHEAT_TELEPORT syn;
							syn.Category  = NM_CHEAT;
							syn.Protocol  = NM_CHEAT_TELEPORT_SYN;
							syn.mObjIndex = heroIdx;
							syn.mDestX    = recvMsg->mDestX;
							syn.mDestY    = recvMsg->mDestY;
						QuickSend( hero, (char*)&syn, sizeof(MSG_SYN_CHEAT_TELEPORT) );
					}
					else
					{
						HANDLE                 handle  = NULL;
						MSG_SYN_CHEAT_TELEPORT* sendMsg = (MSG_SYN_CHEAT_TELEPORT*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_TELEPORT_SYN );
							sendMsg->mObjIndex = heroIdx;
							sendMsg->mDestX    = recvMsg->mDestX;
							sendMsg->mDestY    = recvMsg->mDestY;
						SendMsgRoot( handle, sizeof(MSG_SYN_CHEAT_TELEPORT) );
					}
				}
				else
				{
					// 맵이동 응답 메시지 - 실패
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_TELEPORT_RES, 1 );
				}
			}
			break;
		case NM_CHEAT_GO_REQ:
			{
				MSG_REQ_CHEAT_GO* recvMsg = (MSG_REQ_CHEAT_GO*)msgRoot;

				if ( hero->GetMapNumber() == recvMsg->mMapNum )
				{
					hero->SetMoveTargetPos( recvMsg->mDestX, recvMsg->mDestY );
					hero->SetPos( recvMsg->mDestX, recvMsg->mDestY );
					hero->ClearMoveSyncCalc();

					if ( hero->GetCheatHideMode( ) == true )
					{
						HANDLE                  handle = 0;
						MSG_SYN_CHEAT_TELEPORT* synMsg = (MSG_SYN_CHEAT_TELEPORT*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_TELEPORT_SYN );
							synMsg->mObjIndex = heroIdx;
							synMsg->mDestX    = recvMsg->mDestX;
							synMsg->mDestY    = recvMsg->mDestY;
						SendMsgRoot( handle, sizeof(MSG_SYN_CHEAT_TELEPORT) );
					}
					else
					{
						// 다른 플레이어들에게도 알림
						MSG_SYN_CHEAT_TELEPORT syn;
							syn.Category  = NM_CHEAT;
							syn.Protocol  = NM_CHEAT_TELEPORT_SYN;
							syn.mObjIndex = heroIdx;
							syn.mDestX    = recvMsg->mDestX;
							syn.mDestY    = recvMsg->mDestY;
						QuickSend( hero, (char*)&syn, sizeof(syn) );
					}
				}
				// 이동 성공메세지는 여기서 보내지 않고 후에 맵이동에 필요한 데이타와 함게 보낸다.
				else if ( hero->CheatMapChange( recvMsg->mMapNum, recvMsg->mDestX, recvMsg->mDestY ) == false )
				{
					// 맵이동 응답 메시지 - 실패
					SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, 1 );
				}
			}
			break;
		case NM_CHEAT_CHASE_REQ:
			{
				char				 error = ERROR_CHEAT_NO_ERROR;
				MSG_REQ_CHEAT_CHASE* recvMsg = (MSG_REQ_CHEAT_CHASE*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					if ( player->GetMapNumber() == hero->GetMapNumber() )
					{
						hero->SetMoveTargetPos( player->GetXPos(), player->GetYPos() );
						hero->SetPos( player->GetXPos(), player->GetYPos() );
						hero->ClearMoveSyncCalc();

						if ( hero->GetCheatHideMode() == true )
						{
							HANDLE                  handle = NULL;
							MSG_SYN_CHEAT_TELEPORT* syn    = (MSG_SYN_CHEAT_TELEPORT*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_TELEPORT_SYN );
								syn->mObjIndex = heroIdx;
								syn->mDestX = player->GetXPos();
								syn->mDestY = player->GetYPos();
							SendMsgRoot( handle, sizeof(MSG_SYN_CHEAT_TELEPORT) );
						}
						else
						{
							// 다른 플레이어들에게도 알림
							MSG_SYN_CHEAT_TELEPORT syn;
								syn.Category  = NM_CHEAT;
								syn.Protocol  = NM_CHEAT_TELEPORT_SYN;
								syn.mObjIndex = heroIdx;
								syn.mDestX    = player->GetXPos();
								syn.mDestY    = player->GetYPos();
							QuickSend( hero, (char*)&syn, sizeof(MSG_SYN_CHEAT_TELEPORT) );
						}
					}
					// 이동 성공메세지는 여기서 보내지 않고 후에 맵이동에 필요한 데이타와 함게 보낸다.
					else if( hero->CheatMapChange( player->GetMapNumber(), player->GetXPos(), player->GetYPos() ) == false )
					{
						// 맵이동 응답 메시지 - 실패
						SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, 1 );
					}
				}
				else
					error = ERROR_CHEAT_UNKNOWN_USER ;

				HANDLE handle = NULL;
				MSG_RES_CHEAT_CHASE_GM* sendMsg = (MSG_RES_CHEAT_CHASE_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_CHASE_RES_GM );
					sendMsg->ErrorCode = error;
					wcscpy( sendMsg->mCharName, recvMsg->mCharName );
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_CHASE_GM) );
			}
			break;
		case NM_CHEAT_MOVE_REQ:
			{
				MSG_REQ_CHEAT_MOVE* recvMsg = (MSG_REQ_CHEAT_MOVE*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;
				
				char error      = ERROR_CHEAT_NO_ERROR;
				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					if ( player->GetMapNumber() == recvMsg->mMapNum )
					{
						player->SetMoveTargetPos( recvMsg->mDestX, recvMsg->mDestY );
						player->SetPos( recvMsg->mDestX, recvMsg->mDestY );
						player->ClearMoveSyncCalc();

						if ( player->GetCheatHideMode( ) == true )
						{
							HANDLE                  handle = 0;
							MSG_SYN_CHEAT_TELEPORT* syn    = (MSG_SYN_CHEAT_TELEPORT*)GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_CHEAT, NM_CHEAT_TELEPORT_SYN );
							if ( syn != NULL )
							{
								syn->mObjIndex = player->GetObjectID();
								syn->mDestX    = recvMsg->mDestX;
								syn->mDestY    = recvMsg->mDestY;
								SendMsgRoot( handle, sizeof(MSG_SYN_CHEAT_TELEPORT) );
							}
						}
						else
						{
							// 다른 플레이어들에게도 알림
							MSG_SYN_CHEAT_TELEPORT syn;
								syn.Category  = NM_CHEAT;
								syn.Protocol  = NM_CHEAT_TELEPORT_SYN;
								syn.mObjIndex = player->GetObjectID();
								syn.mDestX    = recvMsg->mDestX;
								syn.mDestY    = recvMsg->mDestY;
							QuickSend( player, (char*)&syn, sizeof(syn) );
						}
					}
					// 이동 성공메세지는 여기서 보내지 않고 후에 맵이동에 필요한 데이타와 함게 보낸다.
					else if ( player->CheatMapChange( recvMsg->mMapNum, recvMsg->mDestX, recvMsg->mDestY ) == false )
					{
						// 맵이동 응답 메시지 - 실패
						SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, 1 );
					}
				}
				else 
					error = ERROR_CHEAT_UNKNOWN_USER ;

				HANDLE handle = NULL;
				MSG_RES_CHEAT_MOVE_GM* sendMsg = (MSG_RES_CHEAT_MOVE_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_MOVE_RES_GM );
					sendMsg->ErrorCode = error;
					wcscpy( sendMsg->mCharName, recvMsg->mCharName );
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_MOVE_GM) );
			}
			break;
		case NM_CHEAT_CALL_REQ:
			{
				MSG_REQ_CHEAT_CALL* recvMsg = (MSG_REQ_CHEAT_CALL*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;
				int err_code = ERROR_CHEAT_CALL_SUCCESS;
				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					/// 소환 할 캐릭터가 소환 할 수 있는 상태인지 검사.
					if( !(player->GetState() == eOBJECT_STATE_IDLE || player->GetState() == eOBJECT_STATE_MOVE) )
					{
						err_code = ERROR_CHEAT_CALL_FAIL_USER_STOP_STATE;
					}
					else if ( player->GetMapNumber() == hero->GetMapNumber() )
					{
						player->SetMoveTargetPos( hero->GetXPos(), hero->GetYPos() );
						player->SetPos( hero->GetXPos(), hero->GetYPos() );
						player->ClearMoveSyncCalc();

						if ( player->GetCheatHideMode() == true )
						{
							HANDLE                  handle = 0;
							MSG_SYN_CHEAT_TELEPORT* syn    = (MSG_SYN_CHEAT_TELEPORT*)GetMsgRoot( &handle, player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_TELEPORT_SYN );
							if ( syn != NULL )
							{
								syn->mObjIndex = player->GetObjectID( );
								syn->mDestX    = hero->GetXPos( );
								syn->mDestY    = hero->GetYPos( );
								SendMsgRoot( handle, sizeof(MSG_SYN_CHEAT_TELEPORT) );
							}
						}
						else
						{
							// 다른 플레이어들에게도 알림
							MSG_SYN_CHEAT_TELEPORT syn;
								syn.Category  = NM_CHEAT;
								syn.Protocol  = NM_CHEAT_TELEPORT_SYN;
								syn.mObjIndex = player->GetObjectID( );
								syn.mDestX    = hero->GetXPos( );
								syn.mDestY    = hero->GetYPos( );
							QuickSend( player, (char*)&syn, sizeof(syn) );
						}
					}
					else
					{
						err_code = ERROR_CHEAT_CALL_FAIL_DIFF_MAP;
					}

					 
				}
				else
					err_code = ERROR_CHEAT_CALL_FAIL_UNKNOWN_USER ;

				// GM에게 치트 성공 메시지 보내기
				HANDLE                 handle = 0;
				MSG_RES_CHEAT_CALL_GM* res = (MSG_RES_CHEAT_CALL_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_CALL_RES_GM );
				res->ErrorCode = err_code;
				wcscpy( res->mCharName, recvMsg->mCharName );
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_CALL_GM) );
			}
			break;
		case NM_CHEAT_SEARCH_REQ:
			{
				MSG_REQ_CHEAT_SEARCH* recvMsg = (MSG_REQ_CHEAT_SEARCH*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					HANDLE					 handle  = NULL;
					MSG_RES_CHEAT_SEARCH_GM* sendMsg = (MSG_RES_CHEAT_SEARCH_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_SEARCH_RES_GM );
					//wcscpy_s( sendMsg->mCharName, recvMsg->mCharName );
					sendMsg->mMapNum   = player->GetMapNumber();
					sendMsg->mPosX     = player->GetXPos();
					sendMsg->mPosY     = player->GetYPos();
					sendMsg->ErrorCode = ERROR_CHEAT_SEARCH_SUCCESS;
					SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_SEARCH_GM) );
				}
				else
				{
					/// 어느 채널에 있는지 DB에서 체크 하기.
					HANDLE			   handle       = NULL;
					CHARACTER_CHANNEL* characterChannel = (CHARACTER_CHANNEL*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_CHANNEL );

					characterChannel->channelNum = 0;
					wcscpy_s( characterChannel->name, recvMsg->mCharName );
					
					SendSQL( perSocketContext, handle, sizeof(CHARACTER_CHANNEL) );
				}
			}
			break;
		case NM_CHEAT_NO_CHAT_REQ:
			{
				MSG_REQ_CHEAT_NO_CHAT* recvMsg = (MSG_REQ_CHEAT_NO_CHAT*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				HANDLE					handle = NULL;
				CHARACTER_BLOCK_INSERT* characterBlockInsert = (CHARACTER_BLOCK_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_BLOCK_INSERT );
				if( handle )
				{
					wcscpy_s( characterBlockInsert->characterName, recvMsg->mCharName );
					characterBlockInsert->adminCharacterIdx = heroIdx;
					characterBlockInsert->validTrue         = (long)recvMsg->mDuration * 60;
					characterBlockInsert->blockCharacterIdx = 0;
					characterBlockInsert->channelNum        = -1;
					
					SendSQL( perSocketContext, handle, sizeof(CHARACTER_BLOCK_INSERT) );
				}
				else
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_NO_CHAT_RES_GM, ERROR_CHEAT_NO_CHAT_FAIL );
				}
			}
			break;
		case NM_CHEAT_ALLOW_CHAT_REQ:
			{
				MSG_REQ_CHEAT_ALLOW_CHAT* recvMsg = (MSG_REQ_CHEAT_ALLOW_CHAT*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				HANDLE					handle = NULL;
				CHARACTER_BLOCK_INSERT* characterBlockInsert = (CHARACTER_BLOCK_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_BLOCK_INSERT );
				if( handle )
				{
					wcscpy_s( characterBlockInsert->characterName, recvMsg->mCharName );
					characterBlockInsert->adminCharacterIdx = heroIdx;
					characterBlockInsert->validTrue         = 0;
					characterBlockInsert->blockCharacterIdx = 0;
					characterBlockInsert->channelNum	    = -1;

					SendSQL( perSocketContext, handle, sizeof(CHARACTER_BLOCK_INSERT) );
				}
				else
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_ALLOW_CHAT_RES_GM, ERROR_CHEAT_ALLOW_CHAT_FAIL );
				}
			}
			break;
		case NM_CHEAT_NO_CHAT_LIST_REQ:
			{
				MSG_REQ_CHEAT_NO_CHAT_LIST* recvMsg = (MSG_REQ_CHEAT_NO_CHAT_LIST*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;
			
				HANDLE						 handle             = NULL;
				CHARACTER_BLOCK_SELECT_LIST* characterBlockList = (CHARACTER_BLOCK_SELECT_LIST*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT_LOG );
				if( handle )
				{
					wcscpy_s( characterBlockList->characterName, recvMsg->mCharName );
					SendSQL( perSocketContext, handle, sizeof(CHARACTER_BLOCK_SELECT_LIST) );
				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_NO_CHAT_LIST_RES_GM, ERROR_CHEAT_NO_CHAT_LIST_FAIL );

			}
			break;
		case NM_CHEAT_GOTO_INSTANT_DUNGEON_REQ:
			{
				MSG_REQ_GOTO_INSTANT_DUNGEON* recvMsg = (MSG_REQ_GOTO_INSTANT_DUNGEON*)msgRoot;

				try 
				{
					// 정상적인 입장 후 GM 툴로는 던젼이동(채널이동)은 불가.
					if( clientInfo->isCheatDungeonIn == false && mType != _E_ST_NORMAL_MAP_ )
						throw ERROR_GOTO_INSTANT_DUNGEON_FAIL_ALREADY_IN;

					//switch ( recvMsg->type )
					//{
					//case GOTO_ID_PVP01:
					//case GOTO_ID_PvP02:
					//	switch ( mPVPData.status )
					//	{
					//	case _E_PVP_READY_:
					//	case _E_PVP_PLAYERLISTWAIT_:
					//	case _E_PVP_RUNNING_:
					//		{
					//			GotoInstantDungeon( perSocketContext, mPVPData.cid );
					//			hero->SetStateStop( eSTOP_MAPCHANGE );

					//			SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_INSTANT_DUNGEON_RES, ERROR_GOTO_INSTANT_DUNGEON_SUCCESS );
					//		}
					//		break;
					//	default:
					//		throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;
					//	}
					//	break;
					//case GOTO_ID_THEME01:
					//	switch ( mThemeData[GOTO_ID_THEME01].status )
					//	{
					//	case _E_CIRCUS_READY_:
					//	case _E_CIRCUS_RUNNING_:
					//		{
					//			GotoInstantDungeon( perSocketContext, mThemeData[GOTO_ID_THEME01].cid );
					//			hero->SetStateStop( eSTOP_MAPCHANGE );

					//			SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_INSTANT_DUNGEON_RES, ERROR_GOTO_INSTANT_DUNGEON_SUCCESS );
					//		}
					//		break;
					//	default:
					//		throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;
					//	}
					//	break;
					//default:
					//	throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;

					if( MAP_PVP_MIN <= recvMsg->type && recvMsg->type <= MAP_PVP_MAX )
					{
						/// 입장하려는 전장서버가 같은 타입인지 체크
						if( mPVPData[0].pvpType != recvMsg->type )
							throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;

						///// 상태 확인
						//switch ( mPVPData[0].status )
						//{
						//case _E_PVP_READY_:
						//case _E_PVP_PLAYERLISTWAIT_:
						//case _E_PVP_RUNNING_:
						//	{
								GotoInstantDungeon( perSocketContext, mPVPData[0].cid );
								hero->SetStateStop( eSTOP_MAPCHANGE );
								PostPvPGmChannelMove( heroIdx, recvMsg->type );

								SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_INSTANT_DUNGEON_RES, ERROR_GOTO_INSTANT_DUNGEON_SUCCESS );								
						//	}
						//	break;
						//default:
						//	throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;
						//}
					}
					else if( MAP_THEME_MIN <= recvMsg->type && recvMsg->type <= MAP_THEME_MAX )
					{
						/// 입장하려는 테마서버의 배열번호를 산출
						unsigned long arryPos = recvMsg->type - MAP_THEME_MIN;

						/// 입장하려는 테마서버 확인
						switch ( mThemeData[arryPos].status )
						{
						case _E_THEME_READY_:
						case _E_THEME_RUNNING_:
							{
								GotoInstantDungeon( perSocketContext, mThemeData[arryPos].cid );
								hero->SetStateStop( eSTOP_MAPCHANGE );

								SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_INSTANT_DUNGEON_RES, ERROR_GOTO_INSTANT_DUNGEON_SUCCESS );
							}
							break;
						default:
							throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;
						}
					}
					else if( MAP_ETC_MIN <= recvMsg->type && recvMsg->type <= MAP_ETC_MAX )
					{
						/// 튜토리얼 테마서버의 배열번호 산출
						unsigned long arryPos = MAP_THEME_CNT + recvMsg->type - MAP_ETC_MIN;

						/// 튜토리얼 서버 상태 확인
						switch ( mThemeData[arryPos].status )
						{
						case _E_THEME_READY_:
						case _E_THEME_RUNNING_:
							{
								GotoInstantDungeon( perSocketContext, mThemeData[arryPos].cid );
								hero->SetStateStop( eSTOP_MAPCHANGE );

								SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_INSTANT_DUNGEON_RES, ERROR_GOTO_INSTANT_DUNGEON_SUCCESS );
							}
							break;
						default:
							throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;
						}
					}						 
					else
						throw ERROR_GOTO_INSTANT_DUNGEON_FAIL;

				} 
				catch ( int error ) 
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_INSTANT_DUNGEON_RES, error );
				}
			}
			break;
		case NM_CHEAT_GOTO_GAMESERVER_REQ:
			{
				MSG_REQ_GOTO_GAMESERVER* recvMsg = (MSG_REQ_GOTO_GAMESERVER*)msgRoot;

				try 
				{
					if( hero->GMMapChangeByTutorial( recvMsg->mMapNum, recvMsg->mPosX, recvMsg->mPosY ) == false )
						throw ERROR_GOTO_GAMESERVER_FAIL;
				} 
				catch ( int error ) 
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GOTO_GAMESERVER_RES, error );
				}
			}
			break;
		case NM_CHEAT_USED_ROOM_ID_REQ:
			{
				MSG_REQ_USED_ROOM_ID* recvMsg = (MSG_REQ_USED_ROOM_ID*)msgRoot;
				recvMsg;

				try {
					if ( mType == _E_ST_ID_PVP_ )
					{
						sPvPObjectRoot* root = mpPVPManager->GetPvPObjectRoot( );
						PerNode*        node = root->pool;

						HANDLE                handle   = NULL;
						MSG_RES_USED_ROOM_ID* sendMsg  = (MSG_RES_USED_ROOM_ID*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_USED_ROOM_ID_RES );
						long&                 rowCount = sendMsg->rowCount;
						long*                 roomInfo = sendMsg->roomInfo;

						sendMsg->ErrorCode = ERROR_USED_ROOM_ID_SUCCESS;
						while ( node != NULL )
						{
							(*roomInfo) = ((cBaseDeathMatch*)node)->GetDMIdx( );

							rowCount++; roomInfo++;
							node = node->next;
						}
						SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
						break;
					}
					else if ( mType == _E_ST_ID_THEME_ || mType == _E_ST_ID_TUTORIAL_ )
					{
						typedef tPointerHashMap<unsigned long, void*> cPHashMap;
						cPHashMap* themeMap = mpThemeManager->GetThemeObjectMap( );

						HANDLE                handle   = NULL;
						MSG_RES_USED_ROOM_ID* sendMsg  = (MSG_RES_USED_ROOM_ID*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_USED_ROOM_ID_RES );
						long&                 rowCount = sendMsg->rowCount;
						long*                 roomInfo = sendMsg->roomInfo;

						sendMsg->ErrorCode = ERROR_USED_ROOM_ID_SUCCESS;

						for ( cPHashMap::cIterator begin=themeMap->Begin(), end=themeMap->End(); begin != end; ++begin )
						{
							cThemeObject* pObject = (cThemeObject*)(*begin).mSecond;
							if ( pObject != NULL )
							{
								(*roomInfo) = pObject->GetThemeIndex( );
								rowCount++; roomInfo++;
							}
						}
						SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
						break;
					}
				} catch ( int error ) {
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_USED_ROOM_ID_RES, error );
				}
			}
			break;
		case NM_CHEAT_ROOMCHANGE_INSTANT_DUNGEON_REQ:
			{
				MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON* recvMsg = (MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON*)msgRoot;
				
				try {
					// 정상적인 입장 후 GM 툴로는 던젼이동(채널이동)은 불가.
					if( clientInfo->isCheatDungeonIn == false && mType != _E_ST_NORMAL_MAP_ )
						throw ERROR_GOTO_INSTANT_DUNGEON_FAIL_ALREADY_IN;

					if( recvMsg->mapNum > 0 )
					{
						if( hero->CheatMapChange( recvMsg->mapNum, 0, 0 ) == false )
							throw ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL;
					}
					else
						throw ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL;
				} catch ( int error ) {
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_ROOMCHANGE_INSTANT_DUNGEON_RES, error );
				}
			}
			break;
		case NM_CHEAT_STOP_REQ:
			{
				MSG_REQ_CHEAT_STOP* recvMsg = (MSG_REQ_CHEAT_STOP*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE] = 0;

				int      err_code = ERROR_CHEAT_NO_ERROR;
				cPlayer* player  = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
					player->CheatStop( recvMsg->mDuration );
				else
					err_code = ERROR_CHEAT_UNKNOWN_USER ;

				HANDLE                 handle = NULL;
				MSG_RES_CHEAT_STOP_GM* res    = (MSG_RES_CHEAT_STOP_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_STOP_RES_GM );
					res->ErrorCode = ERROR_CHEAT_NO_ERROR;
					wcscpy( res->mCharName, recvMsg->mCharName );
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_STOP_GM) );
			}
			break;
		case NM_CHEAT_CANCEL_STOP_REQ:
			{
				MSG_REQ_CHEAT_CANCEL_STOP* recvMsg = (MSG_REQ_CHEAT_CANCEL_STOP*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				int      err_code = ERROR_CHEAT_STOP_SUCCESS;
				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					if( player->GetStateStop() != eSTOP_CHEATSTOP )
						err_code = ERROR_CHEAT_STOP_NOT;

					player->CheatCancelStop( );
				}
				else
					err_code = ERROR_CHEAT_UNKNOWN_USER ;

				HANDLE                        handle = NULL;
				MSG_RES_CHEAT_CANCEL_STOP_GM* res    = (MSG_RES_CHEAT_CANCEL_STOP_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_CANCEL_STOP_RES_GM );
					res->ErrorCode = err_code;
					wcscpy( res->mCharName, recvMsg->mCharName );
				SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_CANCEL_STOP_GM) );
			}
			break;
		case NM_CHEAT_REMOVE_ITEM_REQ:
			OBJECTMANAGER->CheatRemoveItemRange( hero, 5000.0f );
			break;
		case NM_CHEAT_REMOVE_MONSTER_REQ:
			OBJECTMANAGER->CheatRemoveMonsterRange( hero, 5000.0f);
			break;
		case NM_CHEAT_KILL_MONSTER_REQ:
			OBJECTMANAGER->CheatRemoveMonsterRange( hero, 5000.0f, true );
			break;
		case NM_CHEAT_LEVELUP_REQ:
			{
				MSG_REQ_CHEAT_LEVELUP* recvMsg = (MSG_REQ_CHEAT_LEVELUP*)msgRoot;
				if( recvMsg->mAddedValue != 0 )
					hero->LevelUp( recvMsg->mAddedValue, 0, true );
			}
			break;
		case NM_CHEAT_SKILLUP_REQ:
			{
				MSG_REQ_CHEAT_SKILLUP* recvMsg = (MSG_REQ_CHEAT_SKILLUP*)msgRoot;
				if( recvMsg->mAddedValue != 0 )
					hero->SkillLevelUp( recvMsg->mAddedValue, 0, true );
			}
			break;
		case NM_CHEAT_MAKE_REQ:
			try
			{
				hero->ItemInsInventory( (ULONG_PTR)perSocketContext, (MSG_REQ_CHEAT_MAKE*)msgRoot );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_MAKE_RES, error );
			}
			break;
		case NM_CHEAT_MAKE_ITEMS_REQ:
			try
			{
				MSG_REQ_CHEAT_MAKE_ITEMS* recvMsg = (MSG_REQ_CHEAT_MAKE_ITEMS*)msgRoot;

				if ( packetLen != recvMsg->GetMsgLength() )
					ERROR_CHEAT_MAKE_FAIL;

				hero->ItemInsInventory( (ULONG_PTR)perSocketContext, recvMsg );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_MAKE_RES, error );
			}
			break;
		case NM_CHEAT_GOLD_REQ:
			{
				MSG_REQ_CHEAT_GOLD* recvMsg = (MSG_REQ_CHEAT_GOLD*)msgRoot;
				hero->AddMoney( hero->GetObject( ), recvMsg->mValue );
			}
			break;
		case NM_CHEAT_SUMMON_REQ:
			{
				MSG_REQ_CHEAT_SUMMON* recvMsg = (MSG_REQ_CHEAT_SUMMON*)msgRoot;
				AIMANAGER->CheatSummonMonRegen( hero->GetMapNumber(), hero->GetXPos(), hero->GetYPos(), recvMsg->mMonsterNum, recvMsg->mValue );
			}
			break;
		case NM_CHEAT_REMOVE_REQ:
			OBJECTMANAGER->CheatRemoveMonster( ((MSG_REQ_CHEAT_REMOVE*)msgRoot)->mObjIndex );
			break;

		case NM_CHEAT_KILL_REQ:
			{
				if( hero->GetCheatHideMode() == false )
					OBJECTMANAGER->CheatKillMonster( ((MSG_REQ_CHEAT_KILL*)msgRoot)->mObjIndex, hero->GetObjectID() );
			}
			break;
		case NM_CHEAT_SKILL_GET_REQ:
			{
				MSG_REQ_CHEAT_SKILL_ADD* msg = (MSG_REQ_CHEAT_SKILL_ADD*)msgRoot;

				/// 스킬 관련 DB 사용중
				if( clientInfo->skillDb.skillDbUse == true || hero->IsDbUpdate() == true )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_ERR_RES, ERROR_CHEAT_SKILL_ADD_DBERROR );
					break;
				}

				/// 스킬 추가
				unsigned char errorcode = SKILLMANAGER->AddPlayerHaveSkillByCheat( heroIdx, msg->mSkillClassIdx, msg->mSkillStep, 0 );
				if( errorcode != ERROR_CHEAT_SKILL_ADD_SUCCESS )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_ERR_RES, errorcode );
					break;
				}

				// 스킬 DB저장
				if( msg->mIsSkillUp == false )
				{
					// 스킬 신규생성 DB추가
					if( SkillCheatInsert( perSocketContext, heroIdx, msg->mSkillClassIdx, msg->mSkillStep ) == true )
					{
						clientInfo->skillDb.skillDbUse = true;
						hero->PauseDBUpdate();	/// 스킬 업데이트중 player 정보를 업데이트 시키지 않는다.
					}
					else
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_ERR_RES, ERROR_CHEAT_SKILL_ADD_DBERROR );
				}
				else
				{
					// 스킬 단계갱신 DB변경
					if( SkillCheatUpdate( perSocketContext, heroIdx, msg->mSkillClassIdx, msg->mSkillStep ) == true )
					{
						clientInfo->skillDb.skillDbUse = true;
						hero->PauseDBUpdate();	/// 스킬 업데이트중 player 정보를 업데이트 시키지 않는다.
					}
					else
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_ERR_RES, ERROR_CHEAT_SKILL_ADD_DBERROR );
				}
			}
			break;
		case NM_CHEAT_GM_SKILL_USE_PLAYER_REQ:
			{
				MSG_REQ_CHEAT_GM_SKILL_USE_PLAYER* msg = (MSG_REQ_CHEAT_GM_SKILL_USE_PLAYER*)msgRoot;
				/// 해당 유저에게 스킬 걸기
				sObject attacker = { eOBJECTTYPE_PLAYER, heroIdx };
				sObject target   = { eOBJECTTYPE_PLAYER, msg->mCharacterIdx };
				if( SKILLMANAGER->AddInfluence( attacker, target, msg->mInfluenceIndex, 0, true ) == true )
				{
					cPlayer* player = OBJECTMANAGER->GetPlayer( target.index );
					if( player )
					{
						///해당 유저에게 운영자에게 스킬을 받았다는 메시지 보내기.
						HANDLE handle = NULL;
						MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER* res = (MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER*)GetMsgRoot( &handle, player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_GM_SKILL_USE_PLAYER_SYN );
						res->mInfluenceIndex = msg->mInfluenceIndex;
						SendMsgRoot( handle, sizeof( MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER ) );
					}
					else
					{
						///GM에게 실패 메시지 보내기.
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_SKILL_USE_PLAYER_RES_GM, ERROR_CHEAT_GM_SKILL_USE_ERR_TARGET );
					}
				}
				else
				{
					///GM에게 실패 메시지 보내기.
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_SKILL_USE_PLAYER_RES_GM, ERROR_CHEAT_GM_SKILL_USE_ERR_TARGET );
				}
			}
			break;
		case NM_CHEAT_GM_SKILL_USE_AREA_REQ:
			{
				MSG_REQ_CHEAT_GM_SKILL_USE_AREA* msg = (MSG_REQ_CHEAT_GM_SKILL_USE_AREA*)msgRoot;
				/// 해당 지역 버프 걸기
				tArray<unsigned long> userArr;
				GRIDMANAGER->FindPlayers( msg->mPosX, msg->mPosY, hero->GetMapNumber(), 1600, &userArr );

				sObject attacker = { eOBJECTTYPE_PLAYER, heroIdx };
				unsigned int userSize = userArr.GetSize();
				if( userSize < 1 )
				{
					///GM에게 실패 메시지 보내기.
					SendMsgError( perSocketContext, NM_CHEAT, ERROR_CHEAT_GM_SKILL_AREA_ERR_TARGET, NM_CHEAT_GM_SKILL_USE_AREA_RES_GM );
					break;
				}

				for( unsigned int index = 0; index < userSize; ++index )
				{
					/// 해당 유저에게 스킬 걸기
					sObject target   = { eOBJECTTYPE_PLAYER, userArr[index] };
					if( SKILLMANAGER->AddInfluence( attacker, target, msg->mInfluenceIndex, 0, true ) == true )
					{
						cPlayer* player = OBJECTMANAGER->GetPlayer( target.index );
						if( player )
						{
							///해당 유저에게 운영자에게 스킬을 받았다는 메시지 보내기.
							HANDLE handle = NULL;
							MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER* res = (MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER*)GetMsgRoot( &handle, player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_GM_SKILL_USE_PLAYER_SYN );
							res->mInfluenceIndex = msg->mInfluenceIndex;
							SendMsgRoot( handle, sizeof( MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER ) );
						}					
					}
				}
			}
			break;
		case NM_CHEAT_GM_JOB_CHANGE_REQ:
			{
				MSG_REQ_CHEAT_GM_JOB_CHANGE* msg = (MSG_REQ_CHEAT_GM_JOB_CHANGE*)msgRoot;
				
				if( hero )
				{
					if( hero->IsDbUpdate() == true )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_JOB_CHANGE_RES, ERROR_CHEAT_GM_JOB_CHANGE_DB_ERR );
						break;
					}

					/// 전직이 가능한 직업인지 체크
					bool isCorrelateJob = false;
					unsigned long currentJob = (unsigned long)hero->GetJob();
					if( currentJob == msg->mChangeJobIndex )
					{
						///에러 출력
						isCorrelateJob = false;
					}

					typedef tArray<unsigned long> cJobIndexArray;
					cJobIndexArray* pArray = (cJobIndexArray*)STATUSSCRIPT->GetParentJob( msg->mChangeJobIndex );
					if( pArray )
					{
						for( unsigned int i=0; i<pArray->GetSize(); ++i )
						{
							if( (*pArray)[i] == currentJob )
							{
								isCorrelateJob =  true;
								break;
							}
						}
					}

					if( isCorrelateJob == false )
					{
						///GM에게 실패 메시지 보내기.
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_JOB_CHANGE_RES, ERROR_CHEAT_GM_JOB_CHANGE_ERR_NOT_MATCH );
						break;
					}

					/// 1차 전직인지 2차 전직인지 체크 하여 래밸 업 시켜줌.
					unsigned char heroLevel = hero->GetLevel();
					unsigned int jobStep = STATUSSCRIPT->GetJobStep( msg->mChangeJobIndex );
					if( jobStep == 1 )
					{
						if( heroLevel < eCHANGE_JOB_LEVEL_1 )
							hero->LevelUp( (unsigned char)(eCHANGE_JOB_LEVEL_1 - heroLevel), 0, true );

					}
					else if( jobStep == 2 )
					{
						if( heroLevel < eCHANGE_JOB_LEVEL_2 )
							hero->LevelUp( (unsigned char)(eCHANGE_JOB_LEVEL_2 - heroLevel), 0, true );
					}
					
					/// 전직 시키기.
					CheatJobUpdate( perSocketContext, heroIdx, msg->mChangeJobIndex, hero->GetRace(), (long)jobStep );
				}
			}
			break;
		case NM_CHEAT_GM_FORCE_CHANGE_REQ:
			{
				MSG_REQ_CHEAT_GM_FORCE_CHANGE* msg = (MSG_REQ_CHEAT_GM_FORCE_CHANGE*)msgRoot;

				if ( hero )
				{
					if( hero->GetForceType() == msg->mForceType )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_CHANGE_RES, ERROR_CHEAT_GM_FORCE_CHANGE_SAMETYPE );	
						break;
					}

					if( msg->mForceType < eFORCETYPE_FIRE ||msg->mForceType >= eFORCETYPE_MAX )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_CHANGE_RES, ERROR_CHEAT_GM_FORCE_CHANGE_BADTYPE );	
						break;
					}

					HANDLE                  handle    = NULL;
					CHARACTER_FORCE*		forceInfo = (CHARACTER_FORCE*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_FORCETYPE );

					forceInfo->mIdx		  = heroIdx;
					forceInfo->mForceType = msg->mForceType;
					forceInfo->mRetValue  = 0;
					
					SendSQL( perSocketContext, handle, sizeof(CHARACTER_FORCE) );
				}
			}
			break;
		case NM_CHEAT_GM_FORCE_ADDPOINT_REQ:
			{
				MSG_REQ_CHEAT_GM_FORCE_POINT* msg = (MSG_REQ_CHEAT_GM_FORCE_POINT*)msgRoot;

				if ( hero )
				{
					if( hero->GetForceType() == eFORCETYPE_NONE || hero->GetForceType() == eFORCETYPE_MAX )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_ADDPOINT_RES, ERROR_CHEAT_GM_FORCE_POINT_NOFORCE );	
						break;
					}
					
					if( msg->mForceType < eFORCETYPE_FIRE ||msg->mForceType >= eFORCETYPE_MAX )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_ADDPOINT_RES, ERROR_CHEAT_GM_FORCE_POINT_BADTYPE );	
						break;
					}

					if( msg->mForcePoint < 1 )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_ADDPOINT_RES, ERROR_CHEAT_GM_FORCE_POINT_BADPOINT );	
						break;
					}

					if( hero->AddPvPPoint( msg->mForcePoint, msg->mForceType ) == 0 )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_ADDPOINT_RES, ERROR_CHEAT_GM_FORCE_POINT_FAIL );	
						break;
					}
				}
			}
			break;
		case NM_CHEAT_SYSTEM_PLAYERINFO_REQ:
			{
				MSG_REQ_CHEAT_PLAYERINFO* recvMsg = (MSG_REQ_CHEAT_PLAYERINFO*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE]=0;

				cPlayer* player = OBJECTMANAGER->GetPlayer( recvMsg->mCharName );
				if ( player != NULL )
				{
					PerSocketContext* perSC = mSocketContextPool->GetCID( player->GetConnectionIdx() );
					if ( perSC != NULL )
					{
						ClientInfo* targetInfo = (ClientInfo*)perSC->buffer;

						HANDLE					 handle  = NULL;
						MSG_RES_CHEAT_PLAYERINFO* sendMsg = (MSG_RES_CHEAT_PLAYERINFO*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_SYSTEM_PLAYERINFO_RES );
						sendMsg->ErrorCode = ERROR_CHEAT_SEARCH_SUCCESS;
						sendMsg->mState = player->GetState();
						sendMsg->mStateStop = player->GetStateStop();
						sendMsg->mRequestRejection = player->GetRequestRejection();
						sendMsg->mCashInfo = player->IsCashInfSave();
						sendMsg->mInventoryDB = player->InventoryDatabase();
						sendMsg->mCommonDatabase = targetInfo->commonDatabase;
						sendMsg->mGameInDatabase = targetInfo->gameInDatabase;
						sendMsg->mGameInDBEnd = targetInfo->gameInDBEnd;
						sendMsg->mSkillDatabase = targetInfo->skillDatabase;
						sendMsg->mQuestDatabase = targetInfo->questDatabase;
						sendMsg->mGuildDatabase = targetInfo->guildDatabase;
						sendMsg->mRequestData = targetInfo->requestData;
						sendMsg->mCompleteData = targetInfo->completeData;
						SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_PLAYERINFO) );
					}
					else
					{
						// GM에게 치트 실패 메시지 보내기
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SYSTEM_PLAYERINFO_RES, ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER2 );
					}
				}
				else
				{
					// GM에게 치트 실패 메시지 보내기
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SYSTEM_PLAYERINFO_RES, ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER );
				}
			}
			break;
		case NM_CHEAT_GM_THEME_INFO_REQ:
			{
				MSG_REQ_CHEAT_THEME_INFO* recvMsg = (MSG_REQ_CHEAT_THEME_INFO*)msgRoot;
				recvMsg->mCharName[MAX_NAME_SIZE] = 0;

				HANDLE					handle = NULL;
				THEME_GM_USER_SELECT* themeUserSelect = (THEME_GM_USER_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_GM_THEME_USER_SELECT );
				if( handle )
				{
					wcscpy_s( themeUserSelect->characterName, recvMsg->mCharName );
					themeUserSelect->characterIdx = 0;
					themeUserSelect->rowCount = 0;

					unsigned long      length = sizeof(THEME_GM_USER_SELECT) - sizeof(themeUserSelect->table);
					
					if( SendSQL( perSocketContext, handle, length ) == false )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_INFO_REQ, ERROR_CHEAT_GM_THEME_INFO_FAIL );
						break;
					}
				}
				else
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_INFO_REQ, ERROR_CHEAT_GM_THEME_INFO_FAIL );
				}
			}
			break;
		case NM_CHEAT_GM_THEME_RESET_REQ:
			{
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				MSG_REQ_CHEAT_THEME_RESET* recvMsg = (MSG_REQ_CHEAT_THEME_RESET*)msgRoot;
				
				if( clientInfo->commonDb.gmThemeUserReset )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_RES, ERROR_CHEAT_GM_THEME_RESET_DB_USE );
					break;
				}

				/// db 저장
				HANDLE                 handle = NULL;
				CHARACTER_GM_THEME_RESET* reset  = (CHARACTER_GM_THEME_RESET*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_GM_THEMERESET );
				unsigned long			  length = sizeof(CHARACTER_GM_THEME_RESET) - sizeof(reset->mTable);

				if( handle )
				{
					reset->mGMCharacterIdx = heroIdx;
					reset->mCharacterIdx = recvMsg->mCharacterIdx;
					reset->mThemeIdx = recvMsg->mThemeIdx;
					reset->mThemeMode = recvMsg->mThemeMode;
					reset->mRetvalue = 0;
					reset->mRowCount = 0;

					if( NETWORK2->SendSQL( perSocketContext, handle, length ) == false )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_RES, ERROR_CHEAT_GM_THEME_RESET_FAIL );
						break;
					}
					
					clientInfo->commonDb.gmThemeUserReset = true;

				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_RES, ERROR_CHEAT_GM_THEME_RESET_FAIL );
			}
			break;
		case NM_CHEAT_GM_EVENT_SEARCH_REQ:
			{
				///DB검색 SQL문
				HANDLE			  handle  = NULL;
				VERIFY_GM_EVENT*  gmEvent = (VERIFY_GM_EVENT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_GM_EVENT_SEARCH );
				unsigned long	  length  = sizeof(VERIFY_GM_EVENT) - sizeof(gmEvent->table);

				if( NETWORK2->SendSQL( perSocketContext, handle, length ) == false )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_SEARCH_RES, ERROR_CHEAT_GM_EVENT_SEARCH_FAIL );
					break;
				}
			}
			break;
		case NM_CHEAT_GM_EVENT_APPLY_REQ:
			{
				MSG_REQ_CHEAT_GM_EVENT_APPLY* recvMsg = (MSG_REQ_CHEAT_GM_EVENT_APPLY*)msgRoot;

				if( clientInfo->commonDb.gmEventInsert )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_DB_USE );
					break;
				}

				tm startTM = TIMER->ConvertSTtoTM ( recvMsg->mStartTime );
				tm validthruTM = TIMER->ConvertSTtoTM ( recvMsg->mValidThru );
				time_t recvStartTime = mktime( &startTM );
				time_t recvValidThru = mktime( &validthruTM );
				time_t currenttime;
				time( &currenttime );

				/// 이미 만기일이 지나간 것들이라면 에러 리턴
				if( difftime( recvValidThru, currenttime ) < 1 )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_BAD_VALIDTHRU );
					break;
				}

				sInfluenceScript* pRecvInfluence = SKILLSCRIPT->GetInfluenceInfo( recvMsg->mInfluenceIdx );
				if( pRecvInfluence == NULL )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_BAD_INFLUENCE );
					break;
				}

				/// 이미 적용된 것인지 체크
				cGMEventList::cIterator i = mGMEventList.Begin();
				for ( ; i != mGMEventList.End(); ++i )
				{
					sGMEventInfo& eventInfo = (*i);

					sInfluenceScript* pReadyInfluence = SKILLSCRIPT->GetInfluenceInfo( eventInfo.influenceIdx );
					if( pReadyInfluence == NULL )
					{
						PostServerEvent( "UNKNOWN EVENT-INFLUENCE BatchComplete::NM_CHEAT_GM_EVENT_APPLY_REQ (DB_IDX=%d):INFLUENCE_IDX(=%d)", eventInfo.index, eventInfo.influenceIdx );
						continue;
					}

					if( pReadyInfluence->mNotOverlapGroupIdx != pRecvInfluence->mNotOverlapGroupIdx )
						continue;

					/// 이미 만기일이 지나간 것들이라면 체크 시간에서 뺀다.
					if( difftime( eventInfo.validThru, currenttime ) < 1 )
						continue;

					/// 새로 들어갈 효과의 시작 시간이 이미 적용된 시간에 걸쳐 있는 경우
					if( difftime( eventInfo.startTime, recvStartTime ) <= 0 && difftime( eventInfo.validThru, recvStartTime ) >= 0 )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_ALREADY_REGIST );
						break;
					}
					/// 새로 들어갈 효과의 끝나는 시간이 이미 적용된 시간에 걸쳐 있는 경우
					else if( difftime( eventInfo.startTime, recvValidThru ) <= 0 && difftime( eventInfo.validThru, recvValidThru ) >= 0 )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_ALREADY_REGIST );
						break;
					}
					/// 새로 들어갈 효과의 시간이 이미 적용된 시간을 통채로 포함하는 경우
					else if( difftime( eventInfo.startTime, recvStartTime ) >= 0 && difftime( eventInfo.validThru, recvValidThru ) <= 0 )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_ALREADY_REGIST );
						break;
					}
				}
				/// 에러가 발생했다면 리턴
				if( i != mGMEventList.End() )
					break;

				/// db 저장
				HANDLE           handle = NULL;
				GM_EVENT_INSERT* insert = (GM_EVENT_INSERT*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_GM_EVENT_INSERT );
				unsigned long	 length = sizeof(GM_EVENT_INSERT);

				if( handle )
				{
					insert->influenceIndex = recvMsg->mInfluenceIdx;
					insert->startTime	   = TIMER->ConvertTMtoTS(startTM);
					insert->validThru      = TIMER->ConvertTMtoTS(validthruTM);
					insert->index		   = 0;
					insert->retvalue	   = 0;

					if( NETWORK2->SendSQL( perSocketContext, handle, length ) == false )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_FAIL );
						break;
					}

					clientInfo->commonDb.gmEventInsert = true;
				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_FAIL );
			}
			break;
		case NM_CHEAT_GM_EVENT_RELEASE_REQ:
			{
				MSG_REQ_CHEAT_GM_EVENT_RELEASE* pRecvMsg = (MSG_REQ_CHEAT_GM_EVENT_RELEASE*)msgRoot;
				
				///내부 정보 체크
				///없을경우 에러

				if( clientInfo->commonDb.gmEventUpdate )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_RELEASE_RES, ERROR_CHEAT_GM_EVENT_RELEASE_DB_USE );
					break;
				}

				/// db 저장
				HANDLE           handle = NULL;
				GM_EVENT_UPDATE* update = (GM_EVENT_UPDATE*)NETWORK2->GetSQL( &handle, SQL_GAME_PROCESS_GM_EVENT_UPDATE );
				unsigned long	 length = sizeof(GM_EVENT_UPDATE);

				if( handle )
				{
					update->index	 = pRecvMsg->mDBIndex;
					update->influenceIndex = 0;
					update->retvalue = 0;

					if( NETWORK2->SendSQL( perSocketContext, handle, length ) == false )
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_RELEASE_RES, ERROR_CHEAT_GM_EVENT_RELEASE_FAIL );
						break;
					}

					clientInfo->commonDb.gmEventUpdate = true;
				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_RELEASE_RES, ERROR_CHEAT_GM_EVENT_RELEASE_FAIL );

			}
			break;
		}
		break; // NM_CHEAT
	
	case NM_PARTY:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_PARTY::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_PARTY_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_PARTY::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_PARTY::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_PARTY_ADD_REQ:
			try
			{
				MSG_REQ_PARTY_ADD* pmsg = (MSG_REQ_PARTY_ADD*)msgRoot;
				pmsg->Name[ MAX_NAME_SIZE ] = 0;

				/*-- 자신의 상태 검사 --*/

				wchar_t*           heroName  = hero->GetName( );
				ePLAYER_PARTYSTATE heroState = hero->GetPartyState( );
				unsigned long      partyIdx  = hero->GetPartyIndex( );
				cParty*            party     = NULL;

				if ( wcsicmp( pmsg->Name, heroName ) == 0 )			// 이름검사.
					throw ERROR_PARTY_ADD_FAIL;

				if ( hero->IsRequestRejection( ) == true )			// 신청가능 상태 검사
					throw ERROR_PARTY_ADD_STATUS;

				if ( hero->GetPartyUnionIndex( ) > 0 )				// 파티연합 상태 검사
					throw ERROR_PARTY_ADD_PARTYUNION;

				if ( heroState == ePARTY_RECVJOIN )					// 파티 응답중 상태 검사
					throw ERROR_PARTY_ADD_STATUS;

				if ( heroState == ePARTY_SENDJOIN )					// 파티 신청중 상태 검사
					throw ERROR_PARTY_ADD_CONTINUE;

				// 파티 정보 검사
				if ( partyIdx > 0 )
				{
					party = mpPartyManager->GetParty( partyIdx );
					if ( party == NULL )
					{
						PostServerEvent( "ERROR NM_PARTY_ADD_REQ - TB_PARTY::IDX(=%d)", partyIdx );
						throw ERROR_PARTY_ADD_CREATEFAIL;
					}

					if ( !party->IsLeader( heroIdx ) )				// 파티장 검사
						throw ERROR_PARTY_ADD_NOTLEADER;

					if ( !(party->GetCount( ) < MAX_PARTYUSER) )	// 파티인원 검사
						throw ERROR_PARTY_ADD_COUNT;
				}

				/*-- 초대할 캐릭터 상태 검사 --*/

				// 초대할 케릭터.
				cPlayer* player = mpObjectManager->GetPlayer( pmsg->Name );
				int      retvalue;
				if ( player != NULL )
				{
					retvalue = mpPartyManager->CheckPlayer( player );
					if ( retvalue != ERROR_PARTY_ADD_SUCCESS )
						throw retvalue;

					player->SetPartyState( ePARTY_RECVJOIN );
					player->StartRequestRejection( eREQREJCT_PARTY );

					// 초대 받은 유저
					HANDLE		        handle = NULL;
					MSG_SYN_PARTY_ADD*	synMsg = (MSG_SYN_PARTY_ADD*)GetMsgRoot( &handle, player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_SYN );
					if ( synMsg != NULL )
					{
						wcscpy( synMsg->Name, hero->GetName() );
						SendMsgRoot( handle, sizeof(MSG_SYN_PARTY_ADD) );
					}
				}
				else
				{
					if ( party != NULL )
					{
						mpPartyManager->PostSynParty( party );
					}
					mpPartyManager->PostAskParty( partyIdx, hero, pmsg->Name );
				}

				// 신청등록.
				PartyJoin* join = mpPartyManager->GetJoin( pmsg->Name );
				if ( join != NULL )
				{
					join->partyIdx = partyIdx;
					hero->GetPartyUserData( &join->playerDat );
				}

				// 플레이어 상태 변경
				hero->SetPartyState( ePARTY_SENDJOIN );
				// 신청한 유저 - 신청성공 알림
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_SUCCESS );
			}
			catch ( int errorCode )
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_ADD_RES, errorCode );
			}
			break;
		case NM_PARTY_REP_REQ:
			try
			{
				MSG_REQ_PARTY_REP* pmsg     = (MSG_REQ_PARTY_REP*)msgRoot;
				wchar_t*           heroName = hero->GetName( );
				PartyJoin*         join     = mpPartyManager->SearchJoin( heroName );

				unsigned long      partyIdx;
				sPartyUserData     readerDat;
				cParty*            party;

				if ( join == NULL )
					throw ERROR_PARTY_REP_FAIL;

				partyIdx  = join->partyIdx;
				readerDat = join->playerDat;

				mpPartyManager->ReleaseJoin( join );	// 등록해지.

				if ( partyIdx > 0 )
				{
					party = mpPartyManager->GetParty( partyIdx );

					if ( party == NULL )
						throw ERROR_PARTY_REP_FAIL;

					if ( !(party->GetCount( ) < MAX_PARTYUSER) )
						throw ERROR_PARTY_REP_FULL;
				}
				else
					party = NULL;

				// 초대한 캐릭터 및 파티처리.
				cPlayer* reader = mpObjectManager->GetPlayer( readerDat.userIndex );
				if ( reader != NULL )
				{
					ePLAYER_PARTYSTATE state = (party != NULL) ? ePARTY_COMPLETE : ePARTY_NONE;
					reader->SetPartyState( state );

					switch ( pmsg->Reply )
					{
					case PARTY_REP_REJECT:
						SendMsgError( reader->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_REFUSE );
						break;
					case PARTY_REP_REJECTTIME:
						SendMsgError( reader->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_REFUSETIME );
						break;
					case PARTY_REP_ACCEPT:
						break;
					}
				}
				else
				{
					mpPartyManager->PostRepParty( readerDat.userIndex, heroName, 0, pmsg->Reply );
				}

				// 나의 응답처리.
				switch ( pmsg->Reply )
				{
				case PARTY_REP_REJECT:
					hero->SetPartyState( ePARTY_NONE );
					SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_REP_RES, ERROR_PARTY_REP_REFUSE );
					break;
				case PARTY_REP_REJECTTIME:
					hero->SetPartyState( ePARTY_NONE );
					SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_REP_RES, ERROR_PARTY_REP_REFUSETIME );
					break;
				case PARTY_REP_ACCEPT:
					if ( party != NULL )
					{
						party->AddUser( heroIdx );
						party->SetUserData( hero, 0 );

						hero->SetPartyIndex( partyIdx );
						hero->SetPartyState( ePARTY_COMPLETE );

						// 파티정보 send
						mpPartyManager->ResPartyList( hero, party );	// 파티원 정보 가져오기( PARTY TO HERO )
						mpPartyManager->ResPartyAddOne( hero, party );	// 파티원 정보 내보내기( HERO TO PARTY )
						mpPartyManager->PostAddUser( partyIdx, hero );	// 채널동기화
					}
					else
					{
						// 파티 생성.
						HANDLE        handle = NULL;
						PARTY_INSERT* insert = (PARTY_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_PARTY_INSERT );

						insert->leader = readerDat;
						hero->GetPartyUserData( &insert->player );

						SendSQL( handle, sizeof(PARTY_INSERT) );
					}
					break;
				}
				hero->EndRequestRejection( eREQREJCT_PARTY );
			}
			catch ( int error )
			{
				// 세팅 초기화
				hero->SetPartyState( ePARTY_NONE );
				hero->EndRequestRejection( eREQREJCT_PARTY );

				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_REP_RES, error );
			}
			break; // NM_PARTY_REP_REQ
		case NM_PARTY_OUT_REQ:
			try
			{
				mpPartyManager->PartyOutPlayer( hero );
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_OUT_RES, ERROR_PARTY_OUT_SUCCESS );
			}
			catch (int error)
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_OUT_RES, error );
			}
			break;
		case NM_PARTY_CHANGELEADER_REQ:
			try
			{
				MSG_REQ_PARTY_CHANGELEADER* pmsg = (MSG_REQ_PARTY_CHANGELEADER*)msgRoot;
				mpPartyManager->PartyChangeLeader( hero, pmsg->UserIndex );
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_CHANGELEADER_RES, ERROR_PARTY_CHANGELEADER_SUCESS );
			}
			catch (int error)
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_CHANGELEADER_RES, error );
			}
			break;
		case NM_PARTY_CHANGEDIVIDE_REQ:
			try
			{
				MSG_REQ_PARTY_CHANGEDIVIDE* pmsg = (MSG_REQ_PARTY_CHANGEDIVIDE*)msgRoot;
				mpPartyManager->PartyChangeDivide( hero, (PARTY_DIVIDE_TYPE)pmsg->type );
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_CHANGEDIVIDE_RES, ERROR_PARTY_CHANGEDIVIDE_SUCESS );
			}
			catch (int error)
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_CHANGEDIVIDE_RES, error );
			}
			break;
		case NM_PARTY_THROW_REQ:
			try
			{
				MSG_REQ_PARTY_THROW* pmsg = (MSG_REQ_PARTY_THROW*)msgRoot;
				mpPartyManager->PartyThrowPlayer( hero, pmsg->UserIndex );
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_THROW_RES, ERROR_PARTY_THROW_SUCESS );
			}
			catch (int error)
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_THROW_RES, error );
			}
            break;
        case NM_PARTY_DICE_START_REQ:
			try
			{
				mpPartyManager->PartyDiceStart( hero );
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_DICE_START_RES, ERROR_DICE_START_SUCCESS );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_DICE_START_RES, error );
			}
            break;
        case NM_PARTY_DICE_ROLL_REQ:
			try
			{
				MSG_REQ_PARTY_DICE_ROLL* pMsg = (MSG_REQ_PARTY_DICE_ROLL*)msgRoot;
				unsigned short           value;

				// 주사위 굴리기 결과.
				value = (pMsg->mRoll == 1) ? (unsigned short)(RANDOMTABLE->Get( ) * 100 + 1) : 0;
				mpPartyManager->PartyDiceResult( hero, value );

				HANDLE                   handle  = NULL;
				MSG_RES_PARTY_DICE_ROLL* sendMsg = (MSG_RES_PARTY_DICE_ROLL*)GetMsgRoot( &handle, perSocketContext, NM_PARTY, NM_PARTY_DICE_ROLL_RES );
				sendMsg->ErrorCode = ERROR_DICE_ROLL_SUCCESS;
				sendMsg->mRandom   = value;
				SendMsgRoot( handle, sizeof(MSG_RES_PARTY_DICE_ROLL) );
			}
			catch ( int error )
			{
				SendMsgError( perSocketContext, NM_PARTY, NM_PARTY_DICE_ROLL_RES, error );
			}
            break;
		}
		break; // NM_PARTY

	case NM_QUEST:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_QUEST::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_QUEST_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_QUEST::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_QUEST::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		if ( !clientInfo->complete.quest )
			return false;

		switch( msgRoot->Protocol )
		{
		case NM_QUEST_ADDBYITEM_REQ:
			{
				// 의뢰아이템에 의한 퀘스트 시작
				MSG_REQ_QUEST_ADDBYITEM* pmsg = (MSG_REQ_QUEST_ADDBYITEM*)msgRoot;
				try
				{
					// DB 사용중인지 체크
					if( clientInfo->questDb.questInsert || hero->InventoryDatabase( ) )
						throw ERROR_QUEST_ADD_NOTYET;

					// 아이템 사용조건 체크
					unsigned long questIndex = 0;
					int errorCode = hero->IsUseQuestItem( pmsg->number, questIndex );
					if ( errorCode != ERROR_QUEST_ADD_SUCCESS )
						throw errorCode;

					// 퀘스트 가능 조건 검색
					errorCode = hero->IsRegistQuest( eQUESTADD_ETC, questIndex, 0 );
					if ( errorCode != ERROR_QUEST_ADD_SUCCESS )
						throw errorCode;
	
					// 퀘스트 발급 
					errorCode = hero->SaveQuestInsertByItem( (ULONG_PTR)perSocketContext,
														     heroIdx,
															 questIndex,
															 (unsigned short)pmsg->number );
					if ( errorCode != ERROR_QUEST_ADD_SUCCESS )
						throw errorCode;

					clientInfo->questDb.questInsert = true;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_ADDBYITEM_RES, error );
				}
			}
			break;
		case NM_QUEST_DEL_REQ:
			{
				// 퀘스트 포기 요청
				MSG_REQ_QUEST_DEL* pmsg = (MSG_REQ_QUEST_DEL*)msgRoot;
				try
				{
					// DB 사용중인지 검사
					if( clientInfo->questDb.questDelete )
						throw ERROR_QUEST_DEL_NOTYET;

					// 퀘스트번호 유효성 검사
					cQuestDefine* define = mpQuestManager->GetQuestDefine( pmsg->questIndex );
					if ( define == NULL )
						throw ERROR_QUEST_DEL_QUEST;

					// 보유퀘스트 검사
					int dbIndex = hero->IsKeepQuest( pmsg->arrayIdx, pmsg->questIndex );
					if ( dbIndex == -1 )
					{
						PostServerEvent( "ERROR NM_QUEST_DEL_REQ (%d, %d, %d)", hero->GetObjectID(), pmsg->questIndex, pmsg->arrayIdx );
						throw ERROR_QUEST_DEL_QUEST;
					}

					// 포기 요청 
					if ( hero->SaveQuestDelete( (ULONG_PTR)perSocketContext, pmsg, dbIndex ) == false )
						throw ERROR_QUEST_DEL_FAIL;
					
					// 삭제 요청
					clientInfo->questDb.questDelete = true;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_DEL_RES, error );
				}
			}
			break;
		case NM_QUEST_DELAUTO_REQ:
			{
				MSG_REQ_QUEST_DELAUTO* pmsg = (MSG_REQ_QUEST_DELAUTO*)msgRoot;
				try
				{
					if ( clientInfo->questDb.questDeleteAuto )
						throw ERROR_QUEST_DELAUTO_NOTYET;

					// 퀘스트번호 유효성 검사
					cQuestDefine* define = mpQuestManager->GetQuestDefine( pmsg->questIndex );
					if ( define == NULL )
						throw ERROR_QUEST_DELAUTO_FAIL;

					// 보유퀘스트 검사
					int dbIndex = hero->IsKeepQuest( pmsg->arrayIdx, pmsg->questIndex );
					if ( dbIndex == -1 )
					{
						PostServerEvent( "ERROR NM_QUEST_DELAUTO_REQ (%d, %d, %d)", hero->GetObjectID(), pmsg->questIndex, pmsg->arrayIdx );
						throw ERROR_QUEST_DELAUTO_FAIL;
					}

					// 포기 요청 
					if ( hero->SaveQuestDeleteAuto( (ULONG_PTR)perSocketContext, pmsg, dbIndex ) == false )
						throw ERROR_QUEST_DELAUTO_FAIL;

					// 삭제 요청
					clientInfo->questDb.questDeleteAuto = true;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_DELAUTO_RES, error );
				}
			}
			break;
		}
		break;

	case NM_DUEL:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_DUEL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_DUEL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_DUEL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_DUEL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_DUEL_ADD_REQ:
			{
				MSG_REQ_DUEL_ADD* pMsg = (MSG_REQ_DUEL_ADD*)msgRoot;

				int error = DUELMANAGER->ReqDuel( hero->GetObjectID(), pMsg->mTargetIdx );
				if( error != ERR_DUEL_ADD_SUCCESS )
				{
					/// 결투 조건 불만족시 결투 요청자에게 실패 메세지 발송
					SendMsgError( perSocketContext, NM_DUEL, NM_DUEL_ADD_RES, error );
				}
				else
				{
					/// 결투 조건 만족시 결투 대상자에게 수락요청 메세지 발송
					cPlayer* pTarget = OBJECTMANAGER->GetPlayer( pMsg->mTargetIdx );
					if ( pTarget != NULL )
					{
						HANDLE handle  = NULL;
						MSG_RES_DUEL_ADD* sendMsg = (MSG_RES_DUEL_ADD*)GetMsgRoot( &handle, pTarget->GetConnectionIdx(), NM_DUEL, NM_DUEL_ADD_RES );
						if ( sendMsg != NULL )
						{
							sendMsg->mAttackerIdx = hero->GetObjectID();
							sendMsg->ErrorCode = ERR_DUEL_ADD_SUCCESS;
							SendMsgRoot( handle, sizeof(MSG_RES_DUEL_ADD) );

							/// 결투 대상 설정
							pTarget->SetDuelPlayerIdx( hero->GetObjectID(), false );
							hero->SetDuelPlayerIdx( pMsg->mTargetIdx, true );		

							/// 결투 대상자 메세지 대기상태 킴
							pTarget->StartRequestRejection( eREQREJCT_DUELWAIT );
						}
					}
				}
			}
			break;
		case NM_DUEL_ACCEPT_REQ:
			{
				MSG_REQ_DUEL_ACCEPT* pMsg = (MSG_REQ_DUEL_ACCEPT*)msgRoot;
				DUELMANAGER->DuelAllReady( pMsg->mIsAccept, hero );
			}
			break;
		case NM_DUEL_GIVEUP_RES:
			DUELMANAGER->DuelGiveUp( hero->GetDuelIdx(), hero->GetObjectID() );
			break;
		}
		break; // NM_DUEL

	case NM_SERVERTOOL:
		/*-- TOS_SERVERTOOL 메세지는 암호화 체크를 하지 않아 tos검사를 해준다. --*/
		if ( packet->tos != TOS_SERVERTOOL )
		{
			PostServerEvent( "END TASK - WRONG TYPE OF SERVICE(=%d):INETNUM(=%s).",
							 packet->tos,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_SERVERTOOL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_SERVERTOOL::PROTOCOL(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- ClientInfo 사용 데이터 검사 --*/
		if ( clientInfo->requestData != 0 && clientInfo->completeData != 0 )
		{
			PostServerEvent( "END TASK - RequestData(=%d)/CompleteData(=%d):INETNUM(=%s)."
							,clientInfo->requestData
							,clientInfo->completeData
							,inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}

		clientInfo->mHSIsSkip = true;

		switch( msgRoot->Protocol )
		{
		case NM_SERVERTOOL_PING_REQ:
			{
				MSG_REQ_SRVTOOL_PING* reqMsg  = (MSG_REQ_SRVTOOL_PING*)msgRoot;

				SIZE_T quotaPagedPoolUsage;
				SIZE_T quotaNonePagedPoolUsage;
				SIZE_T workingSetSize;

				mSocketContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );

				HANDLE                handle  = NULL;
				MSG_RES_SRVTOOL_PING* sendMsg = (MSG_RES_SRVTOOL_PING*)GetMsgRoot( &handle, perSocketContext, NM_SERVERTOOL, NM_SERVERTOOL_PING_RES );
					sendMsg->mToolSendTime  = reqMsg->mToolSendTime;
					sendMsg->mUserCnt       = (unsigned long)quotaPagedPoolUsage;
					sendMsg->mSecurityLevel = (unsigned long)SRVTOOL_SECURITY_NONE;
					sendMsg->mHackShield    = 0;//mHackShieldPause == false && mHackShieldEnable == 1 ? 1 : 0;
				return SendMsgRoot( handle, sizeof(MSG_RES_SRVTOOL_PING) );
			}
			break;
		case NM_SERVERTOOL_CHANNEL_INFO_REQ:
			{
				HANDLE                         handle  = NULL;
				MSG_RES_SRVTOOL_CHANNEL_INFO* sendMsg = (MSG_RES_SRVTOOL_CHANNEL_INFO*)GetMsgRoot( &handle, perSocketContext, NM_SERVERTOOL, NM_SERVERTOOL_CHANNEL_INFO_RES );
					sendMsg->mServerNum  = mServerNum;
					sendMsg->mChannelNum = mChannelNum;
					wcscpy( sendMsg->mServerName, mServerName );
				return SendMsgRoot( handle, sizeof(MSG_RES_SRVTOOL_CHANNEL_INFO) );
			}
			break;
		case NM_SERVERTOOL_HACKSHIELD_ON_REQ:
			{
//				NETWORK2->PostServerEvent("ServerTool - mHackShieldEnable[%d] / HackShield Resume", mHackShieldEnable);
//				mHackShieldPause = 0;
			}
			break;
		case NM_SERVERTOOL_HACKSHIELD_OFF_REQ:
			{
//				NETWORK2->PostServerEvent("ServerTool - mHackShieldEnable[%d] / HackShield Pause", mHackShieldEnable);
//				mHackShieldPause = 1;
			}
			break;

		}
		break; // NM_SERVERTOOL

	case NM_PVP:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_PVP::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_PVP_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_PVP::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_PVP::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_PVP_READY_REQ:
			{
				/// 레벨 구간에 맞는 배열번호를 가져와야 한다.
				unsigned long aryPos = PVPMANAGER->GetSectionAryPos( hero->GetLevel() );
				
				if ( PVP_DM_LEVEL_SECTION_MAX > aryPos && mPVPData[aryPos].status == _E_PVP_READY_ )
				{
					/// 접속자 명단 목록으로 등록 쿼리 발송
					HANDLE            handle         = NULL;
					PVP_USER_INSERT* pPvPInsert = (PVP_USER_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_PVP_USER_INSERT );
					if( pPvPInsert != NULL )
					{
						/// 레벨로 배열 번호를 찾는다.
						pPvPInsert->pvpIdx = mPVPData[aryPos].pvpType;
						pPvPInsert->characterChannel = (short)mChannelNum;
						pPvPInsert->characterIdx = heroIdx;
						pPvPInsert->characterLevel = hero->GetLevel();
						pPvPInsert->retvalue = 0;
						SendSQL( (PerSocketContext*)perSocketContext, handle, sizeof(PVP_USER_INSERT) );
					}
				}
				else
				{
					/// 클라이언트에 등록 실패 알림
					SendMsgError( perSocketContext, NM_PVP, NM_PVP_READY_RES, 1 );
				}
			}
			break;
		case NM_PVP_READYCANCEL_REQ:
			{
				unsigned long aryPos = PVPMANAGER->GetSectionAryPos( hero->GetLevel() );

				if ( PVP_DM_LEVEL_SECTION_MAX > aryPos && mPVPData[aryPos].status == _E_PVP_READY_ )
				{
					/// 접속자 명단 목록으로 삭제 쿼리 발송
					HANDLE            handle         = NULL;
					PVP_USER_REMOVE* pPvPRemove = (PVP_USER_REMOVE*)GetSQL( &handle, SQL_GAME_PROCESS_PVP_USER_REMOVE );
					if( pPvPRemove != NULL )
					{
						/// 레벨로 배열 번호를 찾는다.
						pPvPRemove->pvpIdx = mPVPData[aryPos].pvpType;
						pPvPRemove->characterIdx = heroIdx;
						pPvPRemove->retvalue = 0;
						SendSQL( (PerSocketContext*)perSocketContext, handle, sizeof(PVP_USER_REMOVE) );
					}
				}
				else
				{
					/// 클라이언트에 삭제 실패 알림
					SendMsgError( perSocketContext, NM_PVP, NM_PVP_READYCANCEL_RES, 1 );
				}
			}
			break;
		case NM_PVP_RESURRECTION_REQ:
			if ( PVPMANAGER->GetPvPDMObject( hero->GetPvPDMIdx() ) != NULL )
			{
				hero->PvPResurrection( );
			}
			else
			{
				PostServerEvent( "NM_PVP_RESURRECTION_REQ pDM[%d,%d,%d,%d] == NULL",
											hero->GetObjectID( ),
											hero->GetState( ),
											hero->GetStateStop( ),
											hero->GetPvPDMIdx() );
			}
			break;
		case NM_PVP_DM_ENDACCEPT_REQ:
			if ( ( mType == _E_ST_ID_PVP_ ) && hero->GetPvPDMIdx() == 0 )
			{
				// 맵서버 이동 요청 메시지.
				clientInfo->request.returnToMap = true;
				clientInfo->gotoCID = MAKECID( (WORD)mServerNum, (WORD)clientInfo->lastChannel );
			}
			break;
		case NM_PVP_DM_END_REQ:
			{
				cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( hero->GetPvPDMIdx() );
				if ( pDM != NULL )
				{
					pDM->EndDMReq( hero->GetObjectID( ), hero->GetPvPDMTeam( ) );

					hero->SetStateStop( eSTOP_MAPCHANGE );

					// 맵서버 이동 요청 메시지.
					clientInfo->request.returnToMap = true;
					clientInfo->gotoCID = MAKECID( (WORD)mServerNum, (WORD)clientInfo->lastChannel );
				}
				else
				{
					PostServerEvent( "NM_PVP_RESURRECTION_REQ pDM[%d,%d,%d,%d] == NULL",
											   hero->GetObjectID( ),
											   hero->GetState( ),
											   hero->GetStateStop( ),
											   hero->GetPvPDMIdx() );
				}
			}
			break;
		case NM_PVP_REPORT_REQ:
			{
				// 전장 자리비움 신고
				MSG_REQ_PVP_REPORT* pmsg = (MSG_REQ_PVP_REPORT*)msgRoot;

				try
				{
					// 현재 채널이 pvp 인지 검사
					if ( mType != _E_ST_ID_PVP_ )
						throw ERROR_PVP_REPORT_FAIL;

					wchar_t* heroName = hero->GetName();
					if( wcsicmp( pmsg->mName, heroName ) == 0 )
						throw ERROR_PVP_REPORT_FAIL;

					pmsg->mName[MAX_NAME_SIZE]=0;

					cPlayer* target = mpObjectManager->GetPlayer( pmsg->mName );
					if ( target == NULL )
						throw ERROR_PVP_REPORT_FAIL;

					if ( target->GetObjectID() == 0 )
						throw ERROR_PVP_REPORT_FAIL;

					// 신고 유저 검증
					cBaseDeathMatch* pDM = PVPMANAGER->GetPvPDMObject( hero->GetPvPDMIdx() );
					if( pDM == NULL )
					{
						// 로그
						throw ERROR_PVP_REPORT_FAIL;
					}

					int ret = pDM->AddReport( hero, target->GetObjectID() );
					if( ret != ERROR_PVP_REPORT_SUCCESS )
						throw ret;
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_PVP, NM_PVP_REPORT_RES, error );
				}
			}
			break;
		}
		break; // NM_PVP

	case NM_THEME:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_THEME::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_THEME_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_THEME::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s)."
							,msgRoot->Protocol
							,clientInfo->memberIdx
							,clientInfo->characterIdx
							,inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_THEME::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_THEME_READY_REQ:
			{
				// 테마서버 이동 요청 메시지.
				MSG_REQ_THEME_READY* pmsg = (MSG_REQ_THEME_READY*)msgRoot;

				THEMEMANAGER->ThemeReadyAccept( heroIdx, pmsg->mThemeMapNum, pmsg->mThemeMode );
			}
			break;
		case NM_THEME_ENDACCEPT_REQ:
			{
				if( mType == _E_ST_ID_THEME_ && hero->GetThemeRoomIdx() == 0 )
				{
					clientInfo->request.returnToMap = true;
					clientInfo->gotoCID = MAKECID( (WORD)mServerNum, (WORD)clientInfo->lastChannel );
				}				
			}
			break;
		}
		break; // NM_THEME

	case NM_GUILD:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_GUILD::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
 		if ( (unsigned char)msgRoot->Protocol >= NM_GUILD_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_GUILD::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_GUILD::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch ( msgRoot->Protocol )
		{
		case NM_GUILD_ADD_REQ:
			{
				// 길드 초대
				MSG_REQ_GUILD_ADD* pmsg = (MSG_REQ_GUILD_ADD*)msgRoot;

				try
				{
					// 길드가입 여부 
					if ( !( hero->GetGuildIndex() > 0 ) )
						throw ERROR_GUILD_ADD_FAIL;

					// 직위 검사
					char position = hero->GetGuildPosition();
					if( position == eGUILD_NONE || position > eGUILD_ELDER )
						throw ERROR_GUILD_ADD_FAIL;

					// HERO가 신청가능한 상태 검사
					if ( hero->IsRequestRejection() == true )
						throw ERROR_GUILD_ADD_STATUSHERO;

					// 이미 신청중인지 여부
					if ( hero->IsGuildAddReq() == true )
						throw ERROR_GUILD_ADD_REQUEST;

					// 상대방 이름 입력 검사
					pmsg->name[MAX_NAME_SIZE]=0;
					if ( wcslen( pmsg->name ) == 0 )
						throw ERROR_GUILD_ADD_FAIL;

					// 자기 자신 이름인지 검사
					wchar_t* heroName = hero->GetName();
					if( wcsicmp( pmsg->name, heroName ) == 0 )
						throw ERROR_GUILD_ADD_FAIL;

					/// 이름으로 유저 찾기
					cPlayer* other = mpObjectManager->GetPlayer( pmsg->name );
					if ( other != NULL )
					{
						if ( other->GetGameIn() == false )			/// 맵이동등 상황이 아님.
							throw ERROR_GUILD_ADD_FAIL;

						if ( other->GetGuildIndex() > 0 )
							throw ERROR_GUILD_ADD_HAVEGUILD;

						// other가 신청받을수 있는 상태인지 검사
						if ( other->IsRequestRejection() == true )
							throw ERROR_GUILD_ADD_STATUS;

						cGuild* guild = mpGuildManager->GetGuild( hero->GetGuildIndex() );
						if ( guild == NULL )
							throw ERROR_GUILD_ADD_FAIL;

						if ( !(guild->GetGuildSize() < MAX_GUILD) )
							throw ERROR_GUILD_ADD_MAXGUILD;

						// 1. hero에게 신청되었음을 알림
						SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_ADD_RES, ERROR_GUILD_ADD_SUCCESS );

						// 2. 상대방에게 신청하기
						HANDLE  toHandle = NULL;
						MSG_SYN_GUILD_ADD* sendMsg = (MSG_SYN_GUILD_ADD*)GetMsgRoot( &toHandle, other->GetConnectionIdx(), NM_GUILD, NM_GUILD_ADD_SYN );
						if( sendMsg != NULL )
						{
							wcscpy( sendMsg->name,		hero->GetName() );
							wcscpy( sendMsg->guildName, guild->GetGuildName() );
							SendMsgRoot( toHandle, sizeof(MSG_SYN_GUILD_ADD) );
						}

						// 신청 정보 저장
						hero->SetGuildAddReq( true );
						mpGuildManager->AddRequest( other->GetObjectID(), hero->GetObjectID() );

						// 신청받고있는 상태로 바꿈
						other->StartRequestRejection( eREQREJCT_GUILD );
					}
					else
						throw ERROR_GUILD_ADD_NOTEXIST;
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_ADD_RES, error );
				}
			}
			break;
		case NM_GUILD_REPLY_REQ: 
			{
				// 길드 초대 답변 보냄
				MSG_REQ_GUILD_REPLY* pmsg = (MSG_REQ_GUILD_REPLY*)msgRoot;

				try
				{
					unsigned long otherIdx = mpGuildManager->GetRequest( heroIdx );

					// 세팅 초기화 (신청정보 삭제)
					hero->EndRequestRejection( eREQREJCT_GUILD );
					mpGuildManager->DelRequest( heroIdx );

					/// 자기자신이면 오류
					if( heroIdx == otherIdx )
						throw ERROR_GUILD_ADDREPLY_FAIL;

					cPlayer* other = mpObjectManager->GetPlayer( otherIdx );
					if( other == NULL )
						throw ERROR_GUILD_ADDREPLY_FAIL;

					// 신청상태 풀기
					other->SetGuildAddReq( false );

					// 거절 
					if( pmsg->reply == GUILD_REPLAY_REFUSE )
					{
						SendMsgError( other->GetConnectionIdx(), NM_GUILD, NM_GUILD_ADD_RES, ERROR_GUILD_ADD_REFUSE );
						throw ERROR_GUILD_ADDREPLY_REFUSE;
					}
					else if( pmsg->reply == GUILD_REPLAY_REFUSETIME )
					{
						// 응답시간 초과
						SendMsgError( other->GetConnectionIdx(), NM_GUILD, NM_GUILD_ADD_RES, ERROR_GUILD_ADD_REFUSETIME );
						throw ERROR_GUILD_ADDREPLY_REFUSETIME;
					}
					// 수락
					else
					{
						// 1. DB 사용중인지 체크
						if ( clientInfo->guildDb.guildAdd )
							throw ERROR_GUILD_ADDREPLY_FAIL;

						// 2. 길드 존재 여부 체크
						cGuild* guild = mpGuildManager->GetGuild( other->GetGuildIndex() );
						if( guild == NULL )
							throw ERROR_GUILD_ADDREPLY_FAIL;

						// 길드에 삽입
						clientInfo->guildDb.guildAdd = GuildUserAdd( perSocketContext, other->GetGuildIndex(), heroIdx );
					}
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_REPLY_RES, error );
				}
			}
			break;
		case NM_GUILD_OUT_REQ:
			{
				// 길드 탈퇴 요청
				try
				{
					if( clientInfo->guildDb.guildOut )
						throw ERROR_GUILD_OUT_FAIL;

					// 길드 가입중 아니면 
					unsigned long guildIndex = hero->GetGuildIndex();
					if( !(guildIndex > 0) )
						throw ERROR_GUILD_OUT_FAIL;

					char heroPos = hero->GetGuildPosition();
					if( heroPos == eGUILD_NONE )
						throw ERROR_GUILD_OUT_FAIL;

					// 길드장 탈퇴불가
					if( hero->GetGuildPosition() == eGUILD_MASTER )
						throw ERROR_GUILD_OUT_MASTER;

					clientInfo->guildDb.guildOut = GuildUserOut( perSocketContext, guildIndex, heroIdx );
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_OUT_RES, error );
				}
			}
			break;
		case NM_GUILD_CUT_REQ:
			{
				MSG_REQ_GUILD_CUT* pmsg = (MSG_REQ_GUILD_CUT*)msgRoot;

				// 길드 추방 유청
				try
				{
					if( clientInfo->guildDb.guildCut )
						throw ERROR_GUILD_CUT_FAIL;

					// 길드 가입중 아니면 
					unsigned long guildIndex = hero->GetGuildIndex();
					if( !(guildIndex > 0) )
						throw ERROR_GUILD_CUT_FAIL;

					// 자기자신은 불가
					if( pmsg->characterIdx == heroIdx )
						throw ERROR_GUILD_CUT_FAIL;

					// 길드장 검사
					char position = hero->GetGuildPosition();
					if( position != eGUILD_MASTER )
						throw ERROR_GUILD_CUT_MASTER;

					// 내 길드원인지 검사
					TB_GUILD_USER* user = mpGuildManager->IsGuildUser( guildIndex, pmsg->characterIdx );
					if( user == NULL )
						throw ERROR_GUILD_CUT_FAIL;
	
					clientInfo->guildDb.guildCut = GuildUserCut( perSocketContext, guildIndex, pmsg->characterIdx );
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_CUT_RES, error );
				}
			}
			break;
		case NM_GUILD_DELETE_REQ:
			{
				try
				{
					if( clientInfo->guildDb.guildDelete )
						throw ERROR_GUILD_DELETE_FAIL;

					// 길드 가입중 아니면 
					unsigned long guildIndex = hero->GetGuildIndex();
					if( !(guildIndex > 0) )
						throw ERROR_GUILD_DELETE_FAIL;

					// 길드장 검사
					char position = hero->GetGuildPosition();
					if( position != eGUILD_MASTER )
						throw ERROR_GUILD_DELETE_MASTER;

					// 길드원이 남아있는지 여부
					cGuild* guild = mpGuildManager->GetGuild( guildIndex );
					if ( guild == NULL )
						throw ERROR_GUILD_DELETE_FAIL;

					if ( guild->GetGuildSize() > 1 )
						throw ERROR_GUILD_DELETE_USEREXIST;

					clientInfo->guildDb.guildDelete = GuildDelete( perSocketContext, guildIndex, heroIdx );
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_DELETE_RES, error );
				}
			}
			break;
		case NM_GUILD_GIVEPOSITION_REQ:
			{
				MSG_REQ_GUILD_GIVEPOSITION* pmsg = (MSG_REQ_GUILD_GIVEPOSITION*)msgRoot;

				try
				{	
					if( clientInfo->guildDb.guildGivePosition )
						throw ERROR_GUILD_GIVEPOSITION_FAIL;

					// 길드 가입중 아니면 
					unsigned long guildIndex = hero->GetGuildIndex();
					if( !(guildIndex > 0) )
						throw ERROR_GUILD_GIVEPOSITION_FAIL;

					// 자기자신은 불가
					if( pmsg->characterIdx == heroIdx )
						throw ERROR_GUILD_GIVEPOSITION_FAIL;

					// 권한 검사
					if( mpGuildManager->CheckChangePosition( hero, pmsg->characterIdx, pmsg->position ) == false )
						throw ERROR_GUILD_GIVEPOSITION_POSITION;

					clientInfo->guildDb.guildGivePosition = GuildUserGivePosition( perSocketContext, guildIndex, pmsg->characterIdx, pmsg->position );
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_GIVEPOSITION_RES, error );
				}
			}
			break;
		case NM_GUILD_NOTICE_REQ:
			{
				MSG_REQ_GUILD_NOTICE* pmsg = (MSG_REQ_GUILD_NOTICE*)msgRoot;
				pmsg->notice[MAX_GUILD_NOTICE_SIZE]=0;

				try
				{
					// 길드 가입중 아니면 
					unsigned long guildIndex = hero->GetGuildIndex();
					if( !(guildIndex > 0) )
						throw ERROR_GUILD_NOTICE_FAIL;

					// 길드장 검사
					char position = hero->GetGuildPosition();
					if( position != eGUILD_MASTER )
						throw ERROR_GUILD_NOTICE_MASTER;

					// 공지 변경 및 보내기
					if( mpGuildManager->SendGuildNotice( guildIndex, pmsg->notice ) == true )
					{
						/// SYN : 길드 공지 변경
						if( mpGuildManager->PostGuildNotice( guildIndex, pmsg->notice ) == false )
						{
							PostGuildEvent( heroIdx, guildIndex, hero->GetGuildPosition(), "FAIL - PostGuildUpdateMark()" );
						}
					}
					else
						throw ERROR_GUILD_NOTICE_FAIL;
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_NOTICE_RES, error );
				}
			}
			break;
		case NM_GUILDMARK_UPDATE_REQ:
			{
				MSG_REQ_GUILDMARK_UPDATE* pmsg = (MSG_REQ_GUILDMARK_UPDATE*)msgRoot;

				try
				{
					if( clientInfo->guildDb.guildMarkUpdate )
						throw ERROR_GUILDMARK_UPDATE_FAIL;

					// 길드 가입중 아니면 
					unsigned long guildIndex = hero->GetGuildIndex();
					if( !(guildIndex > 0) )
						throw ERROR_GUILDMARK_UPDATE_FAIL;

					// 길드장 검사
					char position = hero->GetGuildPosition();
					if( position != eGUILD_MASTER )
						throw ERROR_GUILDMARK_UPDATE_POSITION;

					// 시간 뽑기
					TIMESTAMP_STRUCT registDate = TIMER->GetRealTime();

					clientInfo->guildDb.guildMarkUpdate = GuildMarkUpdate( perSocketContext, guildIndex, registDate, pmsg->mark );
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILDMARK_UPDATE_RES, error );
				}
			}
			break;
		case NM_GUILDMARK_SEND_REQ:
			{
				MSG_REQ_GUILDMARK_SEND* pmsg = (MSG_REQ_GUILDMARK_SEND*)msgRoot;

				try
				{
					if( mpGuildManager->SendGuildMark( pmsg->guildIndex, hero ) == false )
						throw ERROR_GUILDMARK_SEND_FAIL; 
				}
				catch (int error)
				{
					SendMsgError( perSocketContext, NM_GUILD, NM_GUILDMARK_SEND_RES, error );
				}
			}
			break;
		}
		break; // NM_GUILD

	case NM_GATHERING:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_GATHERING::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_GATHERING_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_GUILD::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_GATHERING::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch ( msgRoot->Protocol )
		{
		case NM_GATHERING_START_REQ:
			{
				MSG_REQ_GATHERING_START* pMsg = (MSG_REQ_GATHERING_START*)msgRoot;
				cGathering* pGathering = OBJECTMANAGER->GetGathering( pMsg->mObjectIdx );
				if( pGathering != NULL )
					pGathering->AddGatherPlayer( hero );
				else
					SendMsgError( perSocketContext, NM_GATHERING, NM_GATHERING_START_RES, ERROR_GATHERING_FAIL );
			}
			break;
		case NM_GATHERING_CANCEL_REQ:
			{
				unsigned long gatheringIdx = hero->GetGatheringIdx();
				cGathering* pGathering = OBJECTMANAGER->GetGathering( gatheringIdx );
				if( pGathering != NULL )
					pGathering->ReqEraseGatherPlayer( hero );
			}
			break;
		}
		break; // NM_GATHERING

	case NM_MAKESKILL:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_MAKESKILL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_MAKESKILL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_GUILD::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_MAKESKILL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch ( msgRoot->Protocol )
		{
		case NM_MAKESKILL_ADD_REQ:
			try
			{
				MSG_REQ_MAKESKILL_ADD* pReq = (MSG_REQ_MAKESKILL_ADD*)msgRoot;

				if( clientInfo->skillDb.makeSkillDbUse == true )
					throw (unsigned char)ERROR_MAKESKILL_ADD_DBUSED;

				unsigned char ret = hero->AddMakeSkill( pReq->mSlotNum );
				if( ret != ERROR_MAKESKILL_ADD_SUCCESS )
					throw ret;

				clientInfo->skillDb.makeSkillDbUse = true;
			}
			catch ( unsigned char error )
			{
				SendMsgError( perSocketContext, NM_MAKESKILL, NM_MAKESKILL_ADD_RES, error );
			}
			break;
		case NM_MAKESKILL_RECIPE_ADD_REQ:
			try
			{
				MSG_REQ_MAKESKILL_RECIPE_ADD* pReq = (MSG_REQ_MAKESKILL_RECIPE_ADD*)msgRoot;

				if( clientInfo->skillDb.makeSkillDbUse == true )
					throw (unsigned char)ERROR_MAKESKILL_RECIPE_ADD_DBUSED;

				unsigned char ret = hero->AddRecipe( pReq->mSlotNum );
				if( ret != ERROR_MAKESKILL_RECIPE_ADD_SUCCESS )
					throw ret;

				clientInfo->skillDb.makeSkillDbUse = true;
			}
			catch ( unsigned char error )
			{
				SendMsgError( perSocketContext, NM_MAKESKILL, NM_MAKESKILL_RECIPE_ADD_RES, error );
			}
			break;
		case NM_MAKESKILL_DEL_REQ:
			try
			{
				MSG_REQ_MAKESKILL_DELETE* msg = (MSG_REQ_MAKESKILL_DELETE*)msgRoot;

				if( clientInfo->skillDb.makeSkillDbUse )
					throw (unsigned char)ERROR_MAKESKILL_DEL_DBUSED;

				unsigned char ret = hero->ReqDelMakeSkill( msg->mMakeSkill );
				if( ret != ERROR_MAKESKILL_DEL_SUCCESS )
					throw ret;

				clientInfo->skillDb.makeSkillDbUse = true;
			}
			catch ( unsigned char error )
			{
				SendMsgError( perSocketContext, NM_MAKESKILL, NM_MAKESKILL_DEL_RES, error );
			}
			break;
		case NM_MAKESKILL_ITEMMIX_START_REQ:
			try
			{
				MSG_REQ_MAKESKILL_ITEMMIX_START* pReq = (MSG_REQ_MAKESKILL_ITEMMIX_START*)msgRoot;

				if ( pReq->mMakeCnt < 1 )
					throw (int)ERROR_MAKESKILL_START_COUNT;

				/// 조합 시작 가능 체크
				throw hero->ItemMixReq( pReq->mMakeSkill, pReq->mRecipeIdx, pReq->mMakeCnt );
			}
			catch ( int error )
			{
				/// 가능 / 불가 메세지 발송
				SendMsgError( perSocketContext, NM_MAKESKILL, NM_MAKESKILL_ITEMMIX_START_RES, error );

				/// 가능시 모두에게 알림
				if( error == ERROR_MAKESKILL_START_SUCCESS )
				{
					/// 상태 바뀜을 모두에게 전송
					MSG_SYN_MAKESKILL_ITEMMIX_START synMsg;
					synMsg.Category = NM_MAKESKILL;
					synMsg.Protocol = NM_MAKESKILL_ITEMMIX_START_SYN;
					synMsg.mCharacterIdx = heroIdx;
					QuickSendExcept( hero, (char*)&synMsg, sizeof(MSG_SYN_MAKESKILL_ITEMMIX_START) );
				}
			}
			break;
		case NM_MAKESKILL_ITEMMIX_CANCEL_REQ:
			/// 취소 설정
			hero->ItemMixCancel();
			break;
		}
		break;

	case NM_PARTYUNION:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_PARTYUNION::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_PARTYUNION_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_PARTYUNION::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_PARTYUNION::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_PARTYUNION_CHANGELEADER_REQ:
			{
				// 연합장 위임
				MSG_REQ_PARTYUNION_CHANGELEADER* pmsg = (MSG_REQ_PARTYUNION_CHANGELEADER*)msgRoot;

				try
				{
					int retcode = mpPartyUnionManager->ChangeLeader( hero->GetObjectID(), pmsg->mPlayerIndex );
					throw retcode;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_PARTYUNION, NM_PARTYUNION_CHANGELEADER_RES, error );
				}
			}
			break;
		case NM_PARTYUNION_MOVEGROUP_REQ:
			{
				// 그룹이동
				MSG_REQ_PARTYUNION_MOVEGROUP* pmsg = (MSG_REQ_PARTYUNION_MOVEGROUP*)msgRoot;

				try
				{
					int retcode = mpPartyUnionManager->MoveUser( hero->GetObjectID(), pmsg->mIndex0, pmsg->mIndex1 );
					throw retcode;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_PARTYUNION, NM_PARTYUNION_MOVEGROUP_RES, error );
				}
			}
			break;
		}
		break;
		
	case NM_USERPORTAL:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_USERPORTAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_USERPORTAL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_USERPORTAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_USERPORTAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_USERPORTAL_ENTER_REQ:
			{
				if ( clientInfo->complete.gameIn )
				{
					MSG_REQ_USERPORTAL_MAPCHANGE* reqMsg  = (MSG_REQ_USERPORTAL_MAPCHANGE*)msgRoot;

					char error = hero->UserPortalMapChange( reqMsg->mUserPortalObjectIdx );
					if( error != ERROR_RES_MAPCHANGE_SUCCESS )
					{
						// 맵이동 응답 메시지 - 실패
						// 이동 성공메세지는 여기서 보내지 않고 후에 맵이동에 필요한 데이타와 함게 보낸다.
						SendMsgError( perSocketContext, NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, error );

						// 상태 풀기
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->SetStateStop( eSTOP_NONE );

						return true;
					}

					if( mType != _E_ST_NORMAL_MAP_ )
					{
						clientInfo->request.returnToMap = true;
						clientInfo->gotoCID = MAKECID( (WORD)mServerNum, (WORD)clientInfo->lastChannel );
					}
				}
				break;
			}
		}
		break;

	case NM_TELEPORTPORTAL:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_TELEPORTPORTAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_TELEPORTPORTAL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_TELEPORTPORTAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_TELEPORTPORTAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_TELEPORTPORTAL_USE_REQ:
			{
				if ( clientInfo->complete.gameIn )
				{
					MSG_REQ_TELEPORTPORTAL_USE* reqMsg  = (MSG_REQ_TELEPORTPORTAL_USE*)msgRoot;

					try
					{
						if( mType == _E_ST_ID_THEME_ )
						{
							cThemeObject* pCirucs = THEMEMANAGER->GetThemeObject( hero->GetThemeRoomIdx() );
							if( pCirucs == NULL )
							{
								PostServerEvent("cThemeObject::UseTeleportPortal _E_ST_ID_THEME_ b == e %d, %d", heroIdx, reqMsg->mMapChangePosIdx );
								throw ERROR_TELEPORTPORTAL_USE_THEME;
							}

							throw pCirucs->UseTeleportPortal( hero, reqMsg->mMapChangePosIdx );
						}
						else if( mType == _E_ST_ID_PVP_ )
						{
							cBaseDeathMatch* pPvP = PVPMANAGER->GetPvPDMObject( hero->GetPvPDMIdx() );
							if( pPvP == NULL )
							{
								PostServerEvent("cThemeObject::UseTeleportPortal _E_ST_ID_PVP_ b == e %d, %d", heroIdx, reqMsg->mMapChangePosIdx );
								throw ERROR_TELEPORTPORTAL_USE_PVP;
							}

							throw pPvP->UseTeleportPortal( hero, reqMsg->mMapChangePosIdx );
						}
						else
							throw ERROR_TELEPORTPORTAL_USE_UNKNOWN;
					}
					catch( int error )
					{
						HANDLE handle  = NULL;
						MSG_RES_TELEPORTPORTAL_USE* sendMsg = (MSG_RES_TELEPORTPORTAL_USE*)GetMsgRoot( &handle, perSocketContext,
							NM_TELEPORTPORTAL, NM_TELEPORTPORTAL_USE_RES );
						sendMsg->ErrorCode = error;
						sendMsg->mDestX = hero->GetXPos();
						sendMsg->mDestY = hero->GetYPos();
						SendMsgRoot( handle, sizeof(MSG_RES_TELEPORTPORTAL_USE) );

						if( error == ERROR_TELEPORTPORTAL_USE_SUCCESS )
						{
							MSG_SYN_TELEPORTPORTAL_USE syn;
							syn.Category = NM_TELEPORTPORTAL;
							syn.Protocol = NM_TELEPORTPORTAL_USE_SYN;
							syn.mObjectIdx = hero->GetObjectID();
							syn.mDestX = hero->GetXPos();
							syn.mDestY = hero->GetYPos();
							QuickSendExcept( hero, (char*)&syn, sizeof(syn) );
						}
					}
				}
			}
			break;
		}
		break;

	case NM_TUTORIAL:
		/*-- ClientInfo::Complete 필터링 - Check / CharacterSelect / GameIn --*/
		if ( (clientInfo->completeData & 0x0007) != 0x0007 )
		{
			PostServerEvent( "WRONG NM_TUTORIAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_TUTORIAL_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_TUTORIAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_TUTORIAL::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
				msgRoot->Protocol,
				clientInfo->memberIdx,
				clientInfo->characterIdx,
				inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_TUTORIAL_GIVEUP_REQ:
			{
				try
				{
					if( hero->IsTutorialMode() == false )
						throw ERROR_TUTORIAL_GIVEUP_TUTORIAL; 

					// 맵이동 가능 상태인지 체크
					if( hero->MapChangeByTutorial( CHARACTER_CREATE_POS_IDX ) == false )
						throw ERROR_TUTORIAL_GIVEUP_MAP;

					// 튜토리얼 모드 풀기
					hero->SetTutorialMode( -1 );
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_TUTORIAL, NM_TUTORIAL_GIVEUP_RES, error );
				}
			}
			break;
		case NM_TUTORIAL_REVIVAL_REQ:
			{
				try
				{
					// 튜토리얼 부활요청
					int error = hero->TutorialResurrection( (ULONG_PTR)perSocketContext );
					if( error != ERROR_TUTORIAL_RESURRECTION_SUCCESS )
						throw error;
				}
				catch ( int error )
				{
					SendMsgError( perSocketContext, NM_TUTORIAL, NM_TUTORIAL_REVIVAL_RES, error );
				}
			}
			break;
		}
		break;

	case NM_NOTICE:
		/*-- ClientInfo::SLEVEL 필터링 --*/
		if ( clientInfo->slevel != 1 )
		{
			PostServerEvent( "END TASK - SLEVEL(=%d) NM_CHEAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 clientInfo->slevel,
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
		if ( (unsigned char)msgRoot->Protocol >= NM_NOTICE_MAX )
		{
			PostServerEvent( "END TASK - UNKNOWN NM_CHEAT::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return false;
		}
		/*-- cPlayer=Hero 검사. --*/
		if ( hero == NULL )
		{
			PostServerEvent( "PLAYER IS NULL - NM_NOTICE::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 clientInfo->characterIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
			return true;
		}

		switch( msgRoot->Protocol )
		{
		case NM_NOTICE_NOTICE_SERVER_REQ:
			{
				MSG_REQ_NOTICE_SERVER* recvMsg = (MSG_REQ_NOTICE_SERVER*)msgRoot;
				recvMsg->mMessage[MAX_CHAT_SIZE]=0;

				if ( wcslen( recvMsg->mMessage ) > 0 )
				{
					u_int headLen = sizeof(MSG_REQ_NOTICE_SERVER) - sizeof(recvMsg->mMessage);
					u_int bodyLen = packetLen - headLen;

					MSG_SYN_NOTICE_SERVER syn;
					memset( &syn, 0, sizeof(syn) );

					syn.Category = NM_NOTICE;
					syn.Protocol = NM_NOTICE_NOTICE_SERVER_SYN;
					memcpy( syn.mMessage, recvMsg->mMessage, bodyLen );		//wcscpy( syn.mMessage, recvMsg->mMessage );
					syn.mMessage[MAX_CHAT_SIZE] = 0;
					OBJECTMANAGER->SendNotice( (char*)&syn, syn.GetMsgLength( ) );

					cSender* sender = g_gameSrv->GetSender( );
					sender->PostServerNotice( MAKECID((WORD)mServerNum,(WORD)mChannelNum), syn.mMessage, bodyLen );
				}
			}
			break;
		case NM_NOTICE_NOTICE_CHANNEL_REQ:
			{
				MSG_REQ_NOTICE_CHANNEL* recvMsg = (MSG_REQ_NOTICE_CHANNEL*)msgRoot;
				recvMsg->mMessage[MAX_CHAT_SIZE]=0;

				if ( wcslen( recvMsg->mMessage ) > 0 )
				{
					u_int headLen = sizeof(MSG_REQ_NOTICE_CHANNEL) - sizeof(recvMsg->mMessage);
					u_int bodyLen = packetLen - headLen;

					MSG_SYN_NOTICE_CHANNEL syn;
					memset( &syn, 0, sizeof(syn) );
					syn.Category = NM_NOTICE;
					syn.Protocol = NM_NOTICE_NOTICE_CHANNEL_SYN;
					memcpy( syn.mMessage, recvMsg->mMessage, bodyLen );		//wcscpy( syn.mMessage, recvMsg->mMessage );
					syn.mMessage[MAX_CHAT_SIZE] = 0;
					OBJECTMANAGER->SendNotice( (char*)&syn, syn.GetMsgLength( ) );
				}
			}
			break;
		case NM_NOTICE_NOTICE_AREA_REQ:
			{
				MSG_REQ_NOTICE_AREA* recvMsg = (MSG_REQ_NOTICE_AREA*)msgRoot;
				recvMsg->mMessage[MAX_CHAT_SIZE]=0;

				if ( wcslen( recvMsg->mMessage ) > 0 )
				{
					u_int headLen = sizeof(MSG_REQ_NOTICE_AREA) - sizeof(recvMsg->mMessage);
					u_int bodyLen = packetLen - headLen;

					MSG_SYN_NOTICE_AREA syn;
					memset( &syn, 0, sizeof(syn) );
					syn.Category = NM_NOTICE;
					syn.Protocol = NM_NOTICE_NOTICE_AREA_SYN;
					memcpy( syn.mMessage, recvMsg->mMessage, bodyLen );		//wcscpy( syn.mMessage, recvMsg->mMessage );
					QuickSendMap( hero, (char*)&syn, syn.GetMsgLength() );
				}
			}
			break;
		case NM_NOTICE_DOWN_COUNT_NOTICE_SERVER_REQ:
			{
				MSG_REQ_NOTICE_DOWN_COUNT_SERVER * recvMsg = (MSG_REQ_NOTICE_DOWN_COUNT_SERVER *)msgRoot;

				MSG_SYN_NOTICE_DOWN_COUNT_SERVER syn;
				memset( &syn, 0, sizeof(syn) );

				syn.Category = NM_NOTICE;
				syn.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_SERVER_SYN;
				syn.mCount   = recvMsg->mCount;
				OBJECTMANAGER->SendNotice( (char*)&syn, sizeof(syn) );

				cSender* sender = g_gameSrv->GetSender( );
				sender->PostServerDownCountNotice( MAKECID((WORD)mServerNum,(WORD)mChannelNum), syn.mCount );				
			}
			break;
		case NM_NOTICE_DOWN_COUNT_NOTICE_CHANNEL_REQ:
			{
				MSG_REQ_NOTICE_DOWN_COUNT_CHANNEL* recvMsg = (MSG_REQ_NOTICE_DOWN_COUNT_CHANNEL*)msgRoot;

				MSG_SYN_NOTICE_DOWN_COUNT_CHANNEL  syn;
				memset( &syn, 0, sizeof(syn) );
				syn.Category = NM_NOTICE;
				syn.Protocol = NM_NOTICE_DOWN_COUNT_NOTICE_CHANNEL_SYN;
				syn.mCount   = recvMsg->mCount;
				OBJECTMANAGER->SendNotice( (char*)&syn, sizeof(syn) );
			}
			break;
		}
		break;

//	case NM_HACKSHIELD:
		/*-- MSGROOT::Protocol 필터링 - 알수없는 경우 소켓연결을 해제한다. --*/
//		if ( (unsigned char)msgRoot->Protocol >= NM_HACKSHIELD_MAX )
//		{
//			PostServerEvent( "END TASK - UNKNOWN NM_HACKSHIELD_MAX::PROTOCOL(=%d):MEMBER_IDX(=%d):CHARACTER_IDX(=%d):INETNUM(=%s).",
//				msgRoot->Protocol,
//				clientInfo->memberIdx,
//				clientInfo->characterIdx,
//				inet_ntoa( perSocketContext->addr.sin_addr) );
//			return false;
//		}

//		if ( msgRoot->Protocol == NM_HACKSHIELD_REQ && clientInfo->mHSClientHandle != ANTICPX_INVALID_HANDLE_VALUE )
//		{
//			MSG_REQ_HACKSHIELD* pReq = (MSG_REQ_HACKSHIELD*)msgRoot;

			// -----------------------------------------------------------------
			// [_AhnHS_VerifyResponse ]
			// -----------------------------------------------------------------
			// - DESCRIPTION:
			//  _AhnHS_MakeRequest 함수를 통한 요청(메시지)에 대한 클라이언트의 
			//   응답이 올바른지 검사하는 함수입니다.
			//
			// - SYNTAX:
			//  unsigned long __stdcall _AhnHS_VerifyResponse (IN AHNHS_CLIENT_HANDLE hClient,
			//													IN unsigned char *pbyResponse,
			//												IN unsigned long nResponseLength)
			// - PARAMETERS:
			//  .hClient		AHNHS_CLIENT_HANDLE 	:클라이언트 핸들
			//	.pbyResponse	char *					:클라이언트로 부터 받은 데이터 버퍼
			//  .nResponseLength	unsigned long		:클라이언트로 부터 받은 데이터 길이
			//
			// - RETURN VALUE:
			//   . ERROR_SUCCESS    :
			//     함수 호출을 성공했을 때 리턴하는 값입니다. (Value = 0x00000000) 게임과 HackShield가 
			//     정상적으로 실행되고 있다는 의미입니다.
			//   . 기타				: 기타 리턴값에 대해서는 핵쉴드 프로그래밍 가이드 참고바람.
//			unsigned long errorCode = 0;
//			unsigned long ulRet;

//			ulRet = _AhnHS_VerifyResponseEx( clientInfo->mHSClientHandle, pReq->byBuffer, pReq->nLength, &errorCode );

//			if ( pReq->nLength > ANTICPX_TRANS_BUFFER_MAX )
//			{
//				PostServerEvent("cGameProcess::BatchComplete pReq->nLength > ANTICPX_TRANS_BUFFER_MAX[%d,%d,%d,%d,%d,%x]",
//					clientInfo->memberIdx, clientInfo->characterIdx, pReq->nLength, ANTICPX_TRANS_BUFFER_MAX, ulRet, errorCode );

//				clientInfo->mHSNextCheckTime = ULONG_MAX;				
//				clientInfo->mHSIsWait = false;
//				Close( perSocketContext );
//			}
//			else
//			{
//				Verbose->WriteLog("_AhnHS_VerifyResponseEx H[%u]Size[%d]Ret[%d]", clientInfo->mHSClientHandle, pReq->nLength, ulRet );

				/// 다음 체크 메세지 발송시간 설정
//				clientInfo->mHSNextCheckTime = GetAccumTime() + mHackShieldCheckTime;				
//				clientInfo->mHSIsWait        = false;

//				if ( ulRet == ANTICPX_RECOMMAND_CLOSE_SESSION )
//				{ 
					/// ANTICPX_RECOMMAND_CLOSE_SESSION
					// 설 명 : 클라이언트에서 해킹 행위가 감지되었으므로 게임 서버에서 해당 게임 클라이언트와의 접속을 종료해야 합니다. 
					// (해킹 행위는 주로 _AhnHS_VerifyResponse()함수에서 설명되어진 에러코드 중에 클라이언트와의 접속을 종료하도록 권장한 에러가 반환되는 경우를 의미함)

//					switch( errorCode )
//					{
//					case ERROR_ANTICPXSVR_BAD_MESSAGE:
//						//	 설 명 : 메시지 암복호화에 실패하였습니다.
						//	 원 인 : 검증 받을 버퍼 값이 클라이언트로부터 정확히 받지 않아서 발생한 문제입니다.
						//	 확인사항 : 클라이언트에서 보낸 버퍼가 정확하게 pbyResponse 으로 들어왔는지 확인하시기 바랍니다.
//					case ERROR_ANTICPXSVR_REPLY_ATTACK:
						//	 설 명 : 패킷 분석을 위한 재전송 공격이 감지되었습니다.
						//	 원 인 : 위 에러는 핵쉴드 클라이언트가 동작하지 않은 상태에서 버퍼가 전송되었을 경우 발생합니다.
						//	 확인사항 : 패킷 분석을 위한 해커의 공격일 수 있습니다.
//					case ERROR_ANTICPXSVR_HSHIELD_FILE_ATTACK:
						//	 설 명 : 핵쉴드 모듈 변조가 감지되었습니다.
						//	 원 인 : 핵쉴드 파일 ehsvc.dll 이 변조된 경우 발생합니다.
						//	 확인사항 : 클라이언트의 ehsvc.dll 파일이 변조되었는지 확인해 봅니다.
//					case ERROR_ANTICPXSVR_CLIENT_FILE_ATTACK:
						//	 설 명 : 클라이언트 파일 변조가 감지되었습니다
						//	 원 인 : 게임 클라이언트 파일이 변조된 경우 발생합니다.
						//	 확인사항 : 게임 클라이언트가 정상적으로 업데이트가 되었는지 확인해 봅니다. 바이러스에 의하여 클라이언트가 감염이 되었는지 확인해 봅니다.
//					case ERROR_ANTICPXSVR_MEMORY_ATTACK:
						//	 설 명 : 메모리 변조가 감지되었습니다.
						//	 원 인 : 클라이언트의 메모리가 변조된 경우 발생합니다.
						//	 확인사항 : 클라이언트의 메모리를 조작하는 해킹 툴이 있는지 확인해 보아야 합니다.
//					case ERROR_ANTICPXSVR_NANOENGINE_FILE_ATTACK:
						//	 설 명 : 3N.mhe 파일의 변조가 감지되었습니다.
						//	 원 인 : 3N.mhe 파일이 변조 되었을 경우 발생하는 에러 입니다.
						//	 확인사항: 3N.mhe 파일이 변조되었는지 확인해 봅니다.
//					case ERROR_ANTICPXSVR_UNKNOWN_CLIENT:
						//	 설 명 : HSB 파일을 생성할 때 지정한 클라이언트와 짝이 맞지	않습니다
						//	 원 인 : HSBGen으로 hsb 를 생성할 때 사용된 클라이언트 버전이 아닌 경우 발생합니다.
						//	 확인사항 : 개발과정인 경우 hsb 파일을 다시 생성하여 테스트를 진행 주시고 서비스 단계에서 특정 유저에게 발생된다면 클라이언트 파일 패치가 정상적으로 이루어지지 않았거나 바이러스에 클라이언트 파일이 감염되었는지 확인해 봐야합니다.
//					case ERROR_ANTICPXSVR_INVALID_HACKSHIELD_VERSION:
						//	 설 명 : 서버에서 지원하지 않는 핵쉴드 버전입니다..
						//	 원 인: 서버에 존재하는 HShield.dat 파일보다 클라이언트에 존재하는 HShield.dat 파일이 더 낮은 버전은 경우 발생합니다.
						//	 확인사항: 접속하는 클라이언트의 HShield.dat 파일 버전과 서버의 HShield.dat 파일이 동일한지 확인해 보아야 합니다.
//					case ERROR_ANTICPXSVR_INVALID_ENGINE_VERSION:
						//	 설 명 : 서버에서 지원하지 않는 휴리스틱 엔진 버전입니다..
						//	 원 인 : 서버에 존재하는 3N.mhe 파일보다 클라이언트에 존재하는 3N.mhe 파일이 더 낮은 버전은 경우 발생합니다
						//	 확인사항 : 클라이언트의 핵쉴드 폴더 있는 3N.mhe 파일이 존재하지 않거나 구 버전인지 확인해 보아야 합니다.
						//   주의 : 하기의 리턴값일 경우(bVerify = FALSE)만 해당 클라이언트 접속을 *종료하며
						//          성공(ERROR_SUCCESS)이 아닌 그 외의 리턴값인 경우는 접속을 종료하지 않고 
						//          더이상 AhnHS_MakeRequest 함수도 호출하지 않는것을 권장합니다.
//						PostServerEvent("NM_HACKSHIELD_REQ ulRet == ANTICPX_RECOMMAND_CLOSE_SESSION [%d,%d,%x,%u]", clientInfo->memberIdx, ulRet, errorCode, errorCode );
//						break;
//					default:
						//   주의 : 하기의 리턴값일 경우(bVerify = FALSE)만 해당 클라이언트 접속을 *종료하며
						//          성공(ERROR_SUCCESS)이 아닌 그 외의 리턴값인 경우는 접속을 종료하지 않고 
						//          더이상 AhnHS_MakeRequest 함수도 호출하지 않는것을 권장합니다.
//						PostServerEvent("NM_HACKSHIELD_REQ ulRet == ANTICPX_RECOMMAND_CLOSE_SESSION default [%d,%d,%x,%u]", clientInfo->memberIdx, ulRet, errorCode, errorCode );
//						break;
//					}

//					clientInfo->mHSNextCheckTime = ULONG_MAX;
//					clientInfo->mHSIsWait        = false;
//					Close( perSocketContext );
//				}
//				else if ( ulRet == ANTICPX_RECOMMAND_KEEP_SESSION )
//				{
					/// ANTICPX_RECOMMAND_KEEP_SESSION
					/// 설 명 : 클라이언트에서의 응답이 정상적입니다. 게임 서버에서 해당 게임 클라이언트와의 접속을 계속 진행하시기 바랍니다.
//					Verbose->WriteLog("NM_HACKSHIELD_REQ OK");
//				}
//				else
//				{
					/// 알수 없는 에러 핵쉴드 체크는 중지하고 현재 에러에 대한내용만 기록해두자
//					clientInfo->mHSNextCheckTime = ULONG_MAX;
//					clientInfo->mHSIsWait        = false;
//					PostServerEvent("_AhnHS_VerifyResponseEx except ret[%d,%d,%x,%u]", clientInfo->memberIdx, ulRet, errorCode, errorCode);
//				}
//			}
//		}
//		break;

	default:
		if ( (clientInfo->completeData & 0x0001) != 0x0001 )
		{
			PostServerEvent( "END TASK - UNKNOWN CATEGORY(=%d)::PROTOCOL(=%d):INETNUM(=%s).",
							 msgRoot->Category,
							 msgRoot->Protocol,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
		}
		else
		{
			PostServerEvent( "END TASK - UNKNOWN CATEGORY(=%d)::PROTOCOL(=%d):MEMBER_IDX(=%d):INETNUM(=%s).",
							 msgRoot->Category,
							 msgRoot->Protocol,
							 clientInfo->memberIdx,
							 inet_ntoa( perSocketContext->addr.sin_addr) );
		}
		return false;
	}
	return true;
}

// UpdateComplete Method
bool cGameProcess::UpdateComplete(PerSocketContext* perSocketContext, bool logout, bool shutdown)
{
	ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

	// Common Database.
	if ( clientInfo->commonDatabase )
		return false;

	// PC 방 빌링 종료
	if( g_BillType == 1 )
	{
		if ( clientInfo->complete.pgBilling )
		{
			clientInfo->complete.pgBilling = false;

			char user_id[17];
			char user_ip[24];
			sprintf( user_id, "%d", clientInfo->memberIdx );
			sprintf( user_ip
					,"%d.%d.%d.%d"
					,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b1
					,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b2
					,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b3
					,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b4 );
			PGBILLING->GameEnd( clientInfo->session, user_id, user_ip, clientInfo->status );
			//inet_ntoa( )
		}
	}
	else if( g_BillType == 2 )
	{
		if ( clientInfo->complete.pgBilling )
		{
			clientInfo->complete.pgBilling = false;
			PGBILLPAYLATTER->UserOut( clientInfo->memberIdx );
		}
	}

	if ( clientInfo->complete.characterSelect )
	{
		if ( clientInfo->gameOutDBCnt == false && shutdown == false )
		{
			// GameOutDbCount 증가.
			if ( mGameOutDbCount < MAX_GAME_OUT_DB )
			{
				clientInfo->gameOutDBCnt = true;
				++mGameOutDbCount;
			}
			else
				return false;
		}

		if ( clientInfo->gameInDatabase || clientInfo->skillDatabase || clientInfo->questDatabase || clientInfo->guildDatabase )
			return false;

		unsigned long heroIdx = clientInfo->characterIdx;
		cPlayer*      hero    = mpObjectManager->GetPlayer( heroIdx );

		if ( hero == NULL )
		{
			PostServerEvent( "CriticalError UpdateComplete::CharacterIdx(=%d) - cPlayer Pointer is NULL.", clientInfo->characterIdx );
			return true; // Skip - Character Update.
		}
		else if ( hero->InventoryDatabase( ) )
		{
			return false;
		}
		else if ( hero->UpdateItemBill( ) )
		{
			return false;
		}
		else if ( hero->GetDuelIdx() != 0 )
		{
			try {
				hero->DuelEnd( );	// 결투 종료 처리
			} catch ( ... ) {
				PostServerEvent( "In cPlayer::DuelEnd. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if( !(hero->GetRequestRejection() == eREQREJCT_NONE || hero->GetRequestRejection() == eREQREJCT_GAMEFINISH) )
		{
			try {
				hero->EndRequestRejectionAll();
			} catch ( ... ) {
				PostServerEvent( "In cPlayer::EndRequestRejectionAll. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( hero->GetPartyUnionIndex() > 0 )
		{
			try {
				// 파티연합 체크
				mpPartyUnionManager->DeleteUser( hero );
			} catch ( ... ) {
				PostServerEvent( "In cPartyUnionManager::DeleteUser. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( hero->GetPartyIndex( ) > 0 )
		{
			try {
				cParty* party = mpPartyManager->GetParty( hero->GetPartyIndex( ) );

				if ( party != NULL )
				{
					if ( clientInfo->gotoCID != 0 )
					{
						WORD gotoCID = LOWORD( clientInfo->gotoCID );

						if ( MAP_PVP_MIN <= gotoCID && MAP_PVP_MAX >= gotoCID )
						{
							mpPartyManager->PartyDelPlayer( party, heroIdx );
						}
						else
						{
							party->SetUserData( hero, GetTickCount( ) );
							mpPartyManager->PostSynParty( party );
							mpPartyManager->PostSynUser( party->GetIndex( ), heroIdx, clientInfo->gotoCID );
						}
					}
					else
					{
						mpPartyManager->PartyDelPlayer( party, heroIdx );
					}
				}

				hero->SetPartyIndex( 0 );
				hero->SetPartyState( ePARTY_NONE );
			} catch ( ... ) {
				PostServerEvent( "In cPartyManager::PartyOutPlayer. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( hero->IsCashInfSave() == true )
		{
			return false;
		}
		else if ( !hero->IsGameFinish( ) )
		{
			try {
				// 강제진행.
				switch ( hero->GetState( ) )
				{
				case eOBJECT_STATE_STOP:
					switch ( hero->GetStateStop( ) )
					{
					case eSTOP_READYTAROT:
					case eSTOP_OPENTAROT:
						hero->TarotReaderShutdown( shutdown );
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						break;
					case eSTOP_ENTERTAROT:
					case eSTOP_USETAROT:
						hero->TarotSeekerShutdown( shutdown );
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						break;
                    case eSTOP_SELFTAROT:
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						break;
					case eSTOP_OPENSTALL:
						hero->StallSellOpenShutdown( shutdown );
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						break;
					case eSTOP_USESTALL:
						hero->StallSellUseShutdown( shutdown );
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						break;
					case eSTOP_NPCSPEECH:
					case eSTOP_CHEATSTOP:
                    case eSTOP_SUMMON_PAPER:
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
						break;
					case eSTOP_GAMEFINISH:
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
                        if ( !shutdown && hero->IsAutoTrial() )
                        {
                            hero->TrialEnd( true );
                        }
						break;
					case eSTOP_ITEMMIX:
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
						break;
					case eSTOP_ENHANCED:
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
						break;
					default:
						NETWORK2->PostServerEvent("cGAmeProcess::UpdateComplete !hero->IsGameFinish( ) default[%d,%d,%d,%d]",
							hero->GetObjectID(), hero->GetState(), hero->GetStateStop(), hero->GetRequestRejection() );
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
						break;
					}
					break;
				case eOBJECT_STATE_DIE:
					{
						switch( mType )
						{
						case _E_ST_NORMAL_MAP_:		hero->Resurrection( eRESURRECTIONTYPE_SAFE ); break;
						case _E_ST_ID_PVP_:			hero->PvPResurrection( false ); break;
						case _E_ST_ID_THEME_:		hero->Resurrection( eRESURRECTIONTYPE_SAFE ); break;
						case _E_ST_ID_TUTORIAL_:	hero->TutorialResurrection( (ULONG_PTR)perSocketContext ); break;
						}

						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						hero->ChangeState( eOBJECT_STATE_IDLE, eCHANGESTATEOPTION_RESURRECTION );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
					}
					break;
				case eOBJECT_STATE_ATTACK:
				case ePLAYER_STATE_GATHERING:
					{
						hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->ChangeState( eOBJECT_STATE_STOP );
						hero->SetStateStop( eSTOP_GAMEFINISH );
					}
					break;
				default:
					hero->StartRequestRejection( eREQREJCT_GAMEFINISH );
					hero->SetStateStop( eSTOP_GAMEFINISH );
					hero->ChangeState( eOBJECT_STATE_IDLE );
					hero->ChangeState( eOBJECT_STATE_STOP );
					break;
				}

				hero->ExchangeCancel( false );

				/// 상태 위에서 안바뀐경우 강제처리
				if( shutdown == true )
					hero->ShutdownChangeState();
			} catch ( ... ) {
				PostServerEvent( "In if ( hero->IsGameFinish( ) ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.gameIn )
		{
			try {
				mpGridManager->RemovePlayer( hero );
				clientInfo->complete.gameIn = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.gameIn ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.pvpJoin )
		{
			try {
				/// 인던 방안에서 자신 제거
				PVPMANAGER->MemberOut( hero->GetPvPDMIdx(), hero->GetObjectID(), (ePVPDM_TEAM_TYPE)hero->GetPvPDMTeam() );

				/// 돌아갈 위치 적용
				hero->ApplyReturnGameSrv();

				clientInfo->complete.pvpJoin = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.pvpJoin ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.themeUser )
		{
			try {
				// THEME_USER_REMOVE.
				if ( mType == _E_ST_ID_THEME_ )
				{
					HANDLE             handle          = NULL;
					THEME_USER_REMOVE* themeUserRemove = (THEME_USER_REMOVE*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_REMOVE );

					themeUserRemove->themeIdx     = mInDunMapNumber;
					themeUserRemove->roomIdx      = clientInfo->themeRoomIdx;
					themeUserRemove->themeMode    = clientInfo->themeMode;
					themeUserRemove->characterIdx = clientInfo->characterIdx;

					clientInfo->commonDb.themeUserRemove = SendSQL( perSocketContext, handle, sizeof(THEME_USER_REMOVE) );

					/// 방안의 목록에서 자신 제거
					cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( hero->GetThemeRoomIdx() );
					if ( pTheme != NULL )
						pTheme->ThemeOutPlayer( heroIdx );

					hero->SetThemeRoomIdx( 0 );

					/// 돌아갈 위치 적용
					hero->ApplyReturnGameSrv();
				}
				else if ( mType == _E_ST_ID_TUTORIAL_ )
				{
					/// 방안의 목록에서 자신 제거
					cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( hero->GetThemeRoomIdx() );
					if ( pTheme != NULL )
						pTheme->ThemeOutPlayer( heroIdx );

					hero->SetThemeRoomIdx( 0 );

					/// 돌아갈 위치 적용
					hero->ApplyReturnGameSrv();

					clientInfo->complete.themeUser = false;
				}
				else
					clientInfo->complete.themeUser = false;

				ThemeUserRoot* themeUserRoot = hero->GetThemeUserRoot( );
				if ( themeUserRoot != NULL )
				{
					while ( themeUserRoot->pool )
					{
						PerThemeUser* perThemeUser = (PerThemeUser*)themeUserRoot->pool;
						THEMEUSERPOOL->ReleaseThemeUser( themeUserRoot, perThemeUser );
					}
				}
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.themeUser ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.characterUpdate )
		{
			try {
				hero->DBUpdate( true, PLAYER_DBUPDATE_LOGOUT );
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.characterUpdate ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.skillInfluenceUpdate )
		{
			try {

				HANDLE                  handle          = NULL;
				SKILL_INFLUENCE_UPDATE* influenceUpdate = (SKILL_INFLUENCE_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_INFLUENCE_UPDATE );
				unsigned long           length          = sizeof(SKILL_INFLUENCE_UPDATE) - sizeof(influenceUpdate->mTable);

				if ( !SKILLMANAGER->SaveAllPlayerInfluence( heroIdx, influenceUpdate ) )
				{
					length += influenceUpdate->mRowCount * sizeof(TB_CHARACTER_INFLUENCE);
					// 저장 효과가 없는 경우	
					ReleaseSQL( handle, length );
				}
				else
				{
					length += influenceUpdate->mRowCount * sizeof(TB_CHARACTER_INFLUENCE);
					SendSQL( perSocketContext, handle, length );
				}

				/// 전체 저장 하고 난 이후에 효과객체가 지워지면서 저장하는 것은 막는다.
				hero->SetInfNoSave();
				clientInfo->complete.skillInfluenceUpdate = false;

			} catch ( ... ) {
				PostServerEvent( "In clientInfo->complete.skillInfluenceUpdate. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.characterActiveWeapon )
		{
			try {
				HANDLE                   handle                = NULL;
				CHARACTER_ACTIVE_WEAPON* characterActiveWeapon = (CHARACTER_ACTIVE_WEAPON*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_ACTIVE_WEPON );

				characterActiveWeapon->idx          = heroIdx;
				characterActiveWeapon->activeWeapon = (char)hero->GetActiveWeapon( );

				SendSQL( perSocketContext, handle, sizeof(CHARACTER_ACTIVE_WEAPON), COMMON_DB_CHARACTER_ACTIVE_WEAPON );
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.characterActiveWeapon ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.skillCoolTime )
		{
			try {
				if ( SkillCoolTime( perSocketContext, heroIdx ) )
					clientInfo->complete.skillCoolTime = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.skillCoolTime ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.skillRelease )
		{
			try {
				SKILLMANAGER->ReleasePlayerHaveSkill( heroIdx );	// 게임서버 보유 스킬 제거
				clientInfo->complete.skillRelease = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.skillRelease ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.influenceRelease )
		{
			try {
				hero->ReleaseInfluenceSet( true );						// 게임서버 보유 효과 제거
				clientInfo->complete.influenceRelease = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.influenceRelease ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.recipeCoolTime )
		{
			try {
				if ( RecipeCoolTime( perSocketContext, hero ) )
					clientInfo->complete.recipeCoolTime = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.recipeCoolTime ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.shortcut )
		{
			try {
				HANDLE           handle         = NULL;
				SHORTCUT_UPDATE* shortcutUpdate = (SHORTCUT_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_SHORTCUT_UPDATE );

				shortcutUpdate->characterIdx = heroIdx;
				memcpy( &shortcutUpdate->table, &clientInfo->shortcut, sizeof(TB_SHORTCUT) );

				SendSQL( perSocketContext, handle, sizeof(SHORTCUT_UPDATE), COMMON_DB_SHORTCUT_UPDATE );
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.shortcut ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.quest )
		{
			try {
				/// 유저가 게임종료전, 소환했던 몬스터 삭제
				mpQuestManager->DeleteAllQuestMonster( heroIdx );

				/// 유저가 게임을 종료하기전에 전체 저장 
				hero->SaveQuestProgress( (ULONG_PTR)perSocketContext );
				clientInfo->complete.quest = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.quest ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.fortuneData )
		{
			try {
				/// 운세 시간 정보 저장
				hero->SaveFortuneData( (ULONG_PTR)perSocketContext );
				clientInfo->complete.fortuneData = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.fortuneData ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else if ( clientInfo->complete.inventoryCooltime )
		{
			try {
				CooltimeRoot* cooltimeRoot = hero->GetCooltimeRoot( );
				if ( cooltimeRoot != NULL )
				{
					time_t ltime;
					long   cooltime;
					time( &ltime );

					while ( cooltimeRoot->pool )
					{
						PerCooltime* perCooltime = (PerCooltime*)cooltimeRoot->pool;
						cooltime = (long)difftime( perCooltime->validThru, ltime );

						if ( cooltime > MIN_SAVE_ITEM_COOLTIME )
						{
							HANDLE                     handle                  = NULL;
							INVENTORY_COOLTIME_INSERT* inventoryCooltimeInsert = (INVENTORY_COOLTIME_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_INVENTORY_COOLTIME_INSERT );
							TB_INVENTORY_COOLTIME*     table                   = inventoryCooltimeInsert->table;

							inventoryCooltimeInsert->characterIdx = heroIdx;
							table->itemDefineIndex = perCooltime->index;
							table->cooltime1       = perCooltime->cooltime1;
							table->cooltime2       = perCooltime->cooltime2;
							table->cooltime        = cooltime*1000;

							SendSQL( perSocketContext, handle, sizeof(INVENTORY_COOLTIME_INSERT) );
						}

						COOLTIMEPOOL->ReleaseCooltime( cooltimeRoot, perCooltime );
					}
				}
				clientInfo->complete.inventoryCooltime = false;
			} catch ( ... ) {
				PostServerEvent( "In if ( clientInfo->complete.inventoryCooltime ) {...}. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else
		{
			bool gameOut = (clientInfo->gotoCID == 0);
			WORD gotoCID = LOWORD( clientInfo->gotoCID );

			// 종료시
			if( gameOut == true )
			{
				if( hero->GetGuildIndex() > 0 )
				{
					mpGuildManager->SendGuildUserConnect( hero->GetGuildIndex(), heroIdx, 0, 0, true );
					mpGuildManager->PostGuildUserConnect( hero->GetGuildIndex(), heroIdx, 0, 0, true );
				}

				hero->SendFriendConnect( 0 );
				hero->PostFriendConnect( 0 );
			}
			else
			{
				// 인던이동은 종료로 보지 않는다
				if( hero->GetGuildIndex() > 0 )
				{
					mpGuildManager->SendGuildUserConnect( hero->GetGuildIndex(), heroIdx, 1, gotoCID, false );
					mpGuildManager->PostGuildUserConnect( hero->GetGuildIndex(), heroIdx, 1, gotoCID, false );
				}
			}

			try {
				CharacterTTL( clientInfo->characterIdx, 0, 0 );
				mpObjectManager->RemovePlayer( heroIdx );
				clientInfo->complete.characterSelect = false;	// unselect.

				// GameOutDbCount 감소.
				if ( mGameOutDbCount > 0 ) { --mGameOutDbCount; }
			} catch ( ... ) {
				PostServerEvent( "In Remove Player. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
	}
	else if ( clientInfo->complete.themeUser )
	{
		if ( mType == _E_ST_ID_THEME_ )
		{
			HANDLE             handle          = NULL;
			THEME_USER_REMOVE* themeUserRemove = (THEME_USER_REMOVE*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_REMOVE );

			themeUserRemove->themeIdx     = mInDunMapNumber;
			themeUserRemove->roomIdx      = clientInfo->themeRoomIdx;
			themeUserRemove->characterIdx = clientInfo->characterIdx;

			clientInfo->commonDb.themeUserRemove = SendSQL( perSocketContext, handle, sizeof(THEME_USER_REMOVE) );
		}
		else
			clientInfo->complete.themeUser = false;
	}
	else if ( clientInfo->complete.check )
	{
		if ( logout == true )
		{
			try {
				HANDLE         handle       = NULL;
				MEMBER_LOGOUT* memberLogout = (MEMBER_LOGOUT*)GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_LOGOUT );

				memberLogout->loginIdx  = clientInfo->loginIdx;
				memberLogout->memberIdx = clientInfo->memberIdx;
//				memberLogout->cbtUser   = clientInfo->cbtUser;
//				memberLogout->obtUser   = clientInfo->obtUser;
//				memberLogout->fourGamer = clientInfo->fourGamer;
				memberLogout->retvalue  = 0;

				SendSQL( perSocketContext, handle, sizeof(MEMBER_LOGOUT), COMMON_DB_LOGOUT );
			} catch ( ... ) {
				PostServerEvent( "In Member Logout. Throwing 'cGameProcess::UpdateComplete' exception." );
				throw;
			}
		}
		else
			return true;	// 업데이트 완  료.
	}
	else
		return true;		// 업데이트 완  료.

	return false;			// 업데이트 진행중.
}

// Decrypt Method
void cGameProcess::Decrypt(BYTE key, long len, char* ptr)
{
	Packet* packet = (Packet*)ptr;

	len -= packet->hlen;
	ptr += packet->hlen;

	if ( len > 0 )
	{
		for ( long i = 0; i < len; i++, ptr++ )
		{
			(*ptr) = (*ptr) ^ SY_KEY_TABLE[key][i%3];
		}
	}
}

// Encrypt Method
void cGameProcess::Encrypt(BYTE key, long len, char* ptr)
{
	Packet* packet = (Packet*)ptr;

	len -= packet->hlen;
	ptr += packet->hlen;

	if ( len > 0 )
	{
		for ( LONG i = 0; i < len; i++, ptr++ )
		{
			(*ptr) = (*ptr) ^ SY_KEY_TABLE[key][i%3];
		}
	}
}

// SendExec Method
bool cGameProcess::SendExec(PerIoContext* perIoContext)
{
	WSABUF wsaBuf;
	DWORD  sendBytes = 0;
	DWORD  flags     = 0;
	int    retcode;

	wsaBuf.len = perIoContext->offset;
	wsaBuf.buf = perIoContext->buffer;

	retcode = WSASend( perIoContext->socket,
					   &wsaBuf,
					   1,
					   &sendBytes,
					   flags,
					   &(perIoContext->wsaOverlapped),
					   NULL );

	if ( retcode == SOCKET_ERROR )
	{
		DWORD lastError = WSAGetLastError( );
		if ( lastError != WSA_IO_PENDING )
		{
			switch ( lastError )
			{
			case WSAECONNRESET:		// 현재 연결은 원격 호스트에 의해 강제로 끊겼습니다.
			case WSAENOTSOCK:		// 소켓 이외의 개체에 작업을 시도했습니다.
			case WSAECONNABORTED:	// 현재 연결은 사용자의 호스트 시스템의 소프트웨어의 의해 중단되었습니다.
				break;
			default:
				PostServerEvent( "WARNING - cGameProcess::SendExec:WSAGetLastError(=0x%08xh)", lastError );
				break;
			}

			DWORD error = 0;
			mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
			if ( error != 0 )
				PostServerEvent( "WARNING - cGameProcess::SendExec:Error(=0x%08xh)", error );
			return false;
		}
	}
	return true;
}

// SendPost Method
bool cGameProcess::SendPost(PerSocketContext* perSocketContext, PerIoContext* perIoContext)
{
	cCSLock lock( &mCs );

	if ( (perSocketContext->wsaOff + perIoContext->offset) <= perSocketContext->wsaLen )
	{
		u_long   length  = perIoContext->offset;
		char*    buffer  = perIoContext->buffer;

		// 암호와 적용.
		Packet*  packet  = (Packet*)buffer;
		MSGROOT* msgRoot = (MSGROOT*)(buffer + packet->hlen);

		BYTE  key = (BYTE)(rand( ) & 0x0f);
		(*buffer) = ((*buffer) & 0xf0) | key;

		if ( packet->tlen > packet->hlen )
		{
			if ( msgRoot->Category != NM_SERVERTOOL )
			{
				Encrypt( key, length, buffer );
			}
		}

		// 메모리 버퍼링.
		char* wsaBuf = (perSocketContext->wsaBuf + perSocketContext->wsaOff);

		memcpy( wsaBuf, buffer, length );
		perSocketContext->wsaOff += length;
	}
	else
		Close( perSocketContext );

	DWORD error = 0;
	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::SendExec:Error(=0x%08xh)", error );

	return true;
}

// AcceptComplete Method
bool cGameProcess::AcceptComplete(PerSocketContext* perSocketContext)
{
	cCSLock lock( &mCs );

	Verbose->AcceptComplete( &mCs, perSocketContext );

	try {
		// 클라이언트 메모리(버퍼) 사용량 설정.
		ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
		GUID        guid;

		if ( CoCreateGuid( &guid ) == S_OK )
		{
			sprintf( clientInfo->session
					,"%04X%04X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"
					,HIWORD(guid.Data1)
					,LOWORD(guid.Data1)
					,guid.Data2
					,guid.Data3
					,guid.Data4[ 0 ]
					,guid.Data4[ 1 ]
					,guid.Data4[ 2 ]
					,guid.Data4[ 3 ]
					,guid.Data4[ 4 ]
					,guid.Data4[ 5 ]
					,guid.Data4[ 6 ]
					,guid.Data4[ 7 ] );
			if ( g_verbose == true )
			{
				printf( "\t[SESSION] %s\n", clientInfo->session );
			}
			perSocketContext->offset = (long)sizeof(ClientInfo);

			//	_AhnHS_CreateClientObject
			//
			//	DESCRIPTION
			//		서버 핸들을 입력 받아 클라이언트 핸들을 생성합니다. 클라이언트 핸들은 클라이언트가 접속할 때 마다 생성하며, 
			//		세션이 유지되는 동안 핸들을 닫지 않고 재사용합니다.
			//	
			//	SYNTAX
			//		AHNHS_CLIENT_HANDLE	_AhnHS_CreateClientObject ( IN AHNHS_SERVER_HANDLE hServer );
			//	PARAMETERS
			//		Parameter		Type					Description
			//		hServer			AHNHS_SERVER_HANDLE		서버핸들(Server Handle)
			//
			//	RETURN VALUE
			//		클라이언트 핸들(Client Handle) 서버 핸들을 올바르게 생성하지 못했을 경우 ANTICPX_INVALID_HANDLE_ VALUE 값을 리턴 합니다.
			//		다음 과 같은 상황에서 발생할 수 있습니다.
			//		(1) 전달된 hServer 인자가 유효하지 않은 경우.
			//			_AhnHS_CreateServerObject 함수를 통하여 hServer 서버 핸들이 정상적으로 생성 되었는지 확인해 봅니다.
			//		(2) _AhnHS_CreateServerObject 함수 수행 중 예외(Exception)가 발생하는 경우
			//			(예외가 발생한 경우, 내부적으로 MiniDump 파일을 남기도록 되어 있습니다. 
			//			단, Windows 가 아닌 환경에서는 생성되지 않습니다. "CREATESERVEROBJECT_년_월_일 시-분-초.dmp"
			//			의 파일명 형식으로 덤프 파일이 생성되므로 해당 파일을 ㈜안철수연구소 로 전달해 주시기 바랍니다. )
//			if ( mHackShieldHandle != ANTICPX_INVALID_HANDLE_VALUE )
//			{
//				clientInfo->mHSClientHandle = _AhnHS_CreateClientObject( mHackShieldHandle );

//				if ( clientInfo->mHSClientHandle == ANTICPX_INVALID_HANDLE_VALUE )
//				{
					/// 모듈단에서 에러가나면 체크가 불가능하다 유저와의 통신을 안하게 설정한다.
//					clientInfo->mHSNextCheckTime = ULONG_MAX;
//					clientInfo->mHSIsWait        = false;
//					PostServerEvent( "cGameProcess::AhnhsClientInit perSocketContext->mHSClientHandle == NULL[%d]", clientInfo->memberIdx );
//				}
//				else
//				{
					/// 메세지 발송 시간과 답변받을 메세지 대기시간 설정
//					clientInfo->mHSNextCheckTime = GetAccumTime() + mHackShieldCheckTime;
//					clientInfo->mHSIsWait        = false;
//				}

//				Verbose->WriteLog("_AhnHS_CreateClientObject SH[%u], H[%u]", mHackShieldHandle, clientInfo->mHSClientHandle );
//			}

			// 게임서버 패킷정보.
			PerIoContext* perIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_WRITE );
			Packet*       packet       = (Packet*)perIoContext->buffer;

			packet->ver  = PHVer;
			packet->hlen = PHLen;
			packet->tos  = TOS_GAME_ACCEPT;
			packet->tlen = packet->hlen;
			packet->seq  = (++perSocketContext->seq);

			perIoContext->offset = packet->tlen;
			return SendPost( perSocketContext, perIoContext );
		}

		return false;
	} catch ( char* str ) {
		SIZE_T quotaPagedPoolUsage = 0
			  ,quotaNonPagedPoolUsage = 0
			  ,workingSetSize = 0;
		mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );
		PostServerEvent( "Caught 'AcceptThread' exception type: '%s'. Throwing 'cGameProcess::AcceptComplete' exception. I/O Context PagedPool(=%d) NonePagedPool(=%d) WorkingSetSize(=%d)"
			            ,str
						,quotaPagedPoolUsage
						,quotaNonPagedPoolUsage
						,workingSetSize );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In AcceptThread. Throwing 'cGameProcess::AcceptComplete' exception." );
		Sleep( 100 );
		throw;
	}
}

// SendComplete Method
bool cGameProcess::SendComplete(PerSocketContext* /*perSocketContext*/, PerIoContext* perIoContext, DWORD /*bytesTransferred*/)
{
	DWORD error = 0;
	mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
	if ( error != 0 )
		PostServerEvent( "WARNING - cGameProcess::SendComplete:Error(=0x%08xh)", error );
	return true;
}

// RecvComplete Method
bool cGameProcess::RecvComplete(PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred)
{
	cCSLock lock( &mCs );

	char*   recvBuf    = (perSocketContext->buffer + (perSocketContext->offset + perSocketContext->InternalHigh));
	DWORD   recvBufLen = (perSocketContext->length - (perSocketContext->offset + perSocketContext->InternalHigh));
	u_long& recvBufOff = perSocketContext->InternalHigh;

	// 오프셋의 크기를 누적한다.
	perIoContext->offset += bytesTransferred;

	DWORD length = perIoContext->offset; // 수신된 누적버퍼의 크기
	DWORD offset = 0;                    // 수신된 누적버퍼의 오프셋

	while ( length >= PHLen )
	{
		Packet* packet    = (Packet*)(perIoContext->buffer + offset);
		DWORD   packetLen = packet->tlen;

		// 헤더길이가 정의된 길이와 다를 경우 오류 처리한다.
		if ( packet->hlen != PHLen )
		{
			Close( perSocketContext, perIoContext );
			return false;
		}

		// 데이터의 길이가 최소 단위보다 작을 경우 오류 처리해야 한다.
		if ( packetLen < PHLen )
		{
			Close( perSocketContext, perIoContext );
			return false;
		}

		// 데이터의 길이가 완전할때까지 이어 받기를 해야 한다.
		if ( packetLen > length )
			break;

		// Sequence Number & Acknowledgment Number 검사.
		if ( packet->seq != (++perSocketContext->ack) )
		{
			Close( perSocketContext, perIoContext );
			return false;
		}

		if ( g_packet == true )
		{
			// 패킷헤더(Packet Header) 정보출력 - 버전/헤더길이/서비스 종류/전체길이.
			printf( "GameProcess::RecvComplete::VER:%02x/HLEN:%02x/TOS:%02x/TLEN:%04x/SEQ:%08x\n",
					packet->ver,
					packet->hlen,
					packet->tos,
					packet->tlen,
					packet->seq );
		}

		// Type of service를 검사한다.
		switch ( packet->tos )
		{
		case TOS_CLIENT:
			Decrypt( packet->ver, packet->tlen, (char*)packet );
		case TOS_SERVERTOOL:
			// Receive Data 복사.
			if ( packetLen <= recvBufLen )
			{
				memcpy( recvBuf, packet, packetLen );
				recvBuf    += packetLen;
				recvBufLen -= packetLen;
				recvBufOff += packetLen;
			}
			else
			{
				// Buffer Overflow (버퍼 오버플로).
				Close( perSocketContext, perIoContext );
				return false;
			}
			break;
		case TOS_TTL:
			perSocketContext->timeToLive = (GetTickCount( ) + MAX_TTL);     // 최대 03분
			break;
		case TOS_HACKSHIELD:
			Decrypt( packet->ver, packet->tlen, (char*)packet );
			// Receive Data 복사.
			if ( packetLen <= recvBufLen )
			{
				memcpy( recvBuf, packet, packetLen );
				recvBuf    += packetLen;
				recvBufLen -= packetLen;
				recvBufOff += packetLen;
			}
			else
			{
				// Buffer Overflow (버퍼 오버플로).
				Close( perSocketContext, perIoContext );
				return false;
			}
			break;
		default:
			// Error.
			Close( perSocketContext, perIoContext );
			return false;
		}

		// 처리후 Offset 및 Length를 다시 계산한다.
		length -= packetLen;
		offset += packetLen;
	}

	// 이어 받기를 위해 데이터를 보관한한다.
	if ( offset > 0 )
	{
		PerIoContext* recvIoContext = mIoContextPool->GetIoContext( perSocketContext->socket, IOCP_REQUEST_READ );
		DWORD         error         = 0;

		if ( length > 0 )
		{
			memcpy( recvIoContext->buffer, (perIoContext->buffer + offset), length );
			recvIoContext->offset = length;
		}

		mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
		if ( error != 0 )
			PostServerEvent( "WARNING - cGameProcess::RecvComplete:Error(=0x%08xh)", error );

		perIoContext = recvIoContext;
	}

	// 수신을 위해 I/O Context를 준비한다.
	return RecvPost( perIoContext );
}

// CallbackComplete Method - cCSLock lock( &mCs )는 개별 사용한다.
bool cGameProcess::CallbackComplete(PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD /*bytesTransferred*/)
{
	try {
		switch ( perIoContext->iParam )
		{
		case SQL_GAME_PROCESS_VERIFY_ITEM_DEFINE:
			if ( mRequestImport.itemDefine )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 정의 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemDefine( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemDefine( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_DEFINE - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemDefine = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_DEFINE:
			{
				cCSLock lock( &mCs );
				ITEM_DEFINE_TABLE* itemDefineTable = (ITEM_DEFINE_TABLE*)perIoContext->buffer;

				// 아이템 정의 테이블 설정.
				mpItemManager->SetItemDefine( itemDefineTable->table );

				if ( mpItemManager->CheckItemDefine( ) == false )
				{
					long nextIdx = itemDefineTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemDefineLength( ) )
						ItemDefine( nextIdx );
				}
				else
					mCompleteImport.itemDefine = true;
			}
			break;

		case SQL_GAME_PROCESS_VERIFY_ITEM_ABILITY:
			if ( mRequestImport.itemAbility )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 속성 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemAbility( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemAbility( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_ABILITY - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemAbility = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_ABILITY:
			{
				cCSLock lock( &mCs );
				ITEM_ABILITY_TABLE* itemAbilityTable = (ITEM_ABILITY_TABLE*)perIoContext->buffer;

				// 아이템 속성 테이블 설정.
				mpItemManager->SetItemAbility( itemAbilityTable->table );

				if ( mpItemManager->CheckItemAbility( ) == false )
				{
					long nextIdx = itemAbilityTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemAbilityLength( ) )
						ItemAbility( nextIdx );
				}
				else
					mCompleteImport.itemAbility = true;
			}
			break;

		case SQL_GAME_PROCESS_VERIFY_ITEM_COOLTIME2:
			if ( mRequestImport.itemCoolitme2 )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 쿨타임 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemCooltime2( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemCooltime2( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_COOLTIME2 - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemCoolitme2 = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_COOLTIME2:
			{
				cCSLock lock( &mCs );
				ITEM_COOLTIME2_TABLE* itemCooltime2Table = (ITEM_COOLTIME2_TABLE*)perIoContext->buffer;

				// 아이템 쿨타임 테이블 설정.
				mpItemManager->SetItemCooltime2( itemCooltime2Table->idx, itemCooltime2Table->rowCount, itemCooltime2Table->table );

				if ( mpItemManager->CheckItemCooltime2( ) == false )
				{
					long nextIdx = itemCooltime2Table->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemCooltime2Length( ) )
						ItemCooltime2( nextIdx );
				}
				else
					mCompleteImport.itemCooltime2 = true;
			}
			break;

		case SQL_GAME_PROCESS_VERIFY_ITEM_LIMIT:
			if ( mRequestImport.itemLimit )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 제한 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemLimit( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemLimit( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_LIMIT - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemLimit = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_LIMIT:
			{
				cCSLock lock( &mCs );
				ITEM_LIMIT_TABLE* itemLimitTable = (ITEM_LIMIT_TABLE*)perIoContext->buffer;

				// 아이템 속성 테이블 설정.
				mpItemManager->SetItemLimit( itemLimitTable->table );

				if ( mpItemManager->CheckItemLimit( ) == false )
				{
					long nextIdx = itemLimitTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemLimitLength( ) )
						ItemLimit( nextIdx );
				}
				else
					mCompleteImport.itemLimit = true;
			}
			break;

		case SQL_GAME_PROCESS_VERIFY_ITEM_CARD:
			if ( mRequestImport.itemCard )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 카드 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemCard( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemCard( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_CARD - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemCard = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_CARD:
			{
				cCSLock lock( &mCs );
				ITEM_CARD_TABLE* itemCardTable = (ITEM_CARD_TABLE*)perIoContext->buffer;

				// 아이템 카드 테이블 설정.
				mpItemManager->SetItemCard( itemCardTable->table );

				if ( mpItemManager->CheckItemCard( ) == false )
				{
					long nextIdx = itemCardTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemCardLength( ) )
						ItemCard( nextIdx );
				}
				else
					mCompleteImport.itemCard = true;
			}
			break;

		case SQL_GAME_PROCESS_VERIFY_ITEM_CARD_SLOT:
			if ( mRequestImport.itemCardSlot )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 카드슬롯 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemCardSlot( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemCardSlot( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_CARD_SLOT - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemCardSlot = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_CARD_SLOT:
			{
				cCSLock lock( &mCs );
				ITEM_CARD_SLOT_TABLE* itemCardSlotTable = (ITEM_CARD_SLOT_TABLE*)perIoContext->buffer;

				// 아이템 카드슬롯 테이블 설정.
				mpItemManager->SetItemCardSlot( itemCardSlotTable->table );

				if ( mpItemManager->CheckItemCardSlot( ) == false )
				{
					long nextIdx = itemCardSlotTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemCardSlotLength( ) )
						ItemCardSlot( nextIdx );
				}
				else
					mCompleteImport.itemCardSlot = true;
			}
			break;

		case SQL_GAME_PROCESS_VERIFY_ITEM_TAROT:
			if ( mRequestImport.itemTarot )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 타로 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemTarot( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemTarot( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_TAROT - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemTarot = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_TAROT:
			{
				cCSLock lock( &mCs );
				ITEM_TAROT_TABLE* itemTarotTable = (ITEM_TAROT_TABLE*)perIoContext->buffer;

				// 아이템 타로 테이블 설정.
				mpItemManager->SetItemTarot( itemTarotTable->table );

				if ( mpItemManager->CheckItemTarot( ) == false )
				{
					long nextIdx = itemTarotTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemTarotLength( ) )
						ItemTarot( nextIdx );
				}
				else
					mCompleteImport.itemTarot = true;
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCED:
			if ( mRequestImport.itemEnhanced )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 강화 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemEnhanced( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemEnhanced( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCED - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemEnhanced = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_ENHANCED:
			{
				cCSLock lock( &mCs );
				ITEM_ENHANCED_TABLE* itemEnhancedTable = (ITEM_ENHANCED_TABLE*)perIoContext->buffer;

				// 아이템 강화 테이블 설정.
				mpItemManager->SetItemEnhanced( itemEnhancedTable->table );

				if ( mpItemManager->CheckItemEnhanced( ) == false )
				{
					long nextIdx = itemEnhancedTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemEnhancedLength( ) )
						ItemEnhanced( nextIdx );
				}
				else
					mCompleteImport.itemEnhanced = true;
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCEDRATE:
			if ( mRequestImport.itemEnhancedRate )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 강화확률 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemEnhancedRate( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemEnhancedRate( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_ENHANCEDRATE - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemEnhancedRate = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_ENHANCEDRATE:
			{
				cCSLock lock( &mCs );
				ITEM_ENHANCED_RATE_TABLE* itemEnhancedRateTable = (ITEM_ENHANCED_RATE_TABLE*)perIoContext->buffer;

				// 아이템 강화확률 테이블 설정.
				mpItemManager->SetItemEnhancedRate( itemEnhancedRateTable->table );

				if ( mpItemManager->CheckItemEnhancedRate( ) == false )
				{
					long nextIdx = itemEnhancedRateTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemEnhancedRateLength( ) )
						ItemEnhancedRate( nextIdx );
				}
				else
					mCompleteImport.itemEnhancedRate = true;
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_DISJOINT:
			if ( mRequestImport.itemDisjoint )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 강화확률 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemDisjoint( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemDisjoint( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_DISJOINT - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemDisjoint = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_DISJOINT:
			{
				cCSLock lock( &mCs );
				ITEM_DISJOINT_TABLE* itemDisjointTable = (ITEM_DISJOINT_TABLE*)perIoContext->buffer;

				// 아이템 분해 테이블 설정.
				mpItemManager->SetItemDisjoint( itemDisjointTable->table );

				if ( mpItemManager->CheckItemDisjoint( ) == false )
				{
					long nextIdx = itemDisjointTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemDisjointLength( ) )
						ItemDisjoint( nextIdx );
				}
				else
					mCompleteImport.itemDisjoint = true;
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_ITEM_CHANGE:
			if ( mRequestImport.itemChange )
			{
				VERIFY_ITEM_TABLE* verifyItemTable = (VERIFY_ITEM_TABLE*)perIoContext->buffer;

				// 아이템 교환 테이블 생성 및 가져오기.
				if ( verifyItemTable->retvalue == 0 )
				{
					mpItemManager->AllocItemChange( verifyItemTable->count );

					for ( long idx=1, count=min( verifyItemTable->count, 10 ); idx <= count; idx++ )
						ItemChange( idx );
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_ITEM_CHANGE - Count(=%d), Return Value(=%d)."
									 ,verifyItemTable->count
									 ,verifyItemTable->retvalue );

				mRequestImport.itemChange = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_CHANGE:
			{
				cCSLock lock( &mCs );
				ITEM_CHANGE_TABLE* itemChangeTable = (ITEM_CHANGE_TABLE*)perIoContext->buffer;

				// 아이템 교환 테이블 설정.
				mpItemManager->SetItemChange( itemChangeTable->table );

				if ( mpItemManager->CheckItemChange( ) == false )
				{
					long nextIdx = itemChangeTable->idx + 10;
					if ( nextIdx <= (long)mpItemManager->ItemChangeLength( ) )
						ItemChange( nextIdx );
				}
				else
					mCompleteImport.itemChange = true;
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_DEFAULT_ITEMS:
			if ( mRequest.verifyDefaultItems )
			{
				cCSLock               lock( &mCs );
				VERIFY_DEFAULT_ITEMS* verifyDefaultItems = (VERIFY_DEFAULT_ITEMS*)perIoContext->buffer;

				if ( verifyDefaultItems->retvalue != 0 )
				{
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_DEFAULT_ITEMS - Count(=%d), Return Value(=%d)."
									 ,verifyDefaultItems->rowCount
									 ,verifyDefaultItems->retvalue );
				}

				mComplete.verifyDefaultItems = (verifyDefaultItems->retvalue == 0) ? true : false;
				mRequest.verifyDefaultItems  = false;
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_GUILD:
			if( mRequestImport.guild )
			{
				VERIFY_GUILD* verifyGuild = (VERIFY_GUILD*)perIoContext->buffer;

				//// 길드 분할 select
				if ( verifyGuild->retvalue == 0 )
				{	
					if ( verifyGuild->total > 0 )
					{
						mpGuildManager->SetGuildTotalLength( verifyGuild->total );
						mpGuildManager->SetGuildLength( verifyGuild->count );

						for ( long idx = 1; idx <= verifyGuild->total; ++idx )
						{
							if ( GuildSelect( idx ) == false )
								PostServerEvent( "SQL_GAME_PROCESS_VERIFY_GUILD - SQL call failed." );
						}
					}
					else
					{
						mCompleteImport.guild = true;
					}
				}
				else
					PostServerEvent( "Error - SQL_GAME_PROCESS_VERIFY_GUILD - Count(=%d), Return Value(=%d)."
									 , verifyGuild->total
									 , verifyGuild->retvalue );

				mCompleteImport.guild = true;
				mRequestImport.guild = false;
			}
			break;
		case SQL_GAME_PROCESS_GUILD_SELECT:
			{
				cCSLock	lock( &mCs );
				GUILD_SELECT* guildSelect = (GUILD_SELECT*)perIoContext->buffer;

				TB_GUILD* tbGuild = guildSelect->table;
				if( tbGuild->apply == 0 )
				{
					if ( mpGuildManager->VerifyGuildSelect( tbGuild ) == true )
					{
						// 길드 유저 분할 로딩
						if ( GuildUserSelect( tbGuild->idx, guildSelect->restore ) == false )
							PostServerEvent( "SQL_GAME_PROCESS_VERIFY_GUILD - SQL call failed(%d)", guildSelect->restore );
					}
					else
					{
						PostServerEvent( "SQL_GAME_PROCESS_GUILD_SELECT - Failed to create a guild information(%d)", guildSelect->restore );
					}
				}

				// 복구용일때는 체크 하지 않는다
				if( guildSelect->restore == false )
				{
					// 길드 테이블 로딩 완료
					if( mpGuildManager->CheckGuildTotal() == true )
						mCompleteImport.guild = mpGuildManager->CheckGuildLength();
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILDUSER_SELECT:
			{
				cCSLock lock( &mCs );
				GUILDUSER_SELECT* guildUserSelect = (GUILDUSER_SELECT*)perIoContext->buffer;
				TB_GUILD_USER*    guildUserTable  = guildUserSelect->table;

				bool error = false;

				/// 길드 유저 리스트 세팅
				for ( long i = 0; i < guildUserSelect->rowCount; ++i, ++guildUserTable )
				{
					if ( mpGuildManager->VerifyGuildUser( guildUserSelect->idx, guildUserTable ) == false )
					{
						PostServerEvent( "Warning - SQL_GAME_PROCESS_GUILDUSER_SELECT - GUILD_IDX(=%d), CHARACTER_IDX(=%d), RESTORE(=%d)"
										,guildUserSelect->idx
										,guildUserTable->characterIdx
										,guildUserSelect->restore );

						error = true;
					}
				}
				if ( guildUserSelect->rowCount == 0 )
				{
					PostServerEvent( "Error - SQL_GAME_PROCESS_GUILDUSER_SELECT - RowCount(%d,%d,%d)"
									,guildUserSelect->idx
									,guildUserSelect->rowCount
									,guildUserSelect->restore );

					error = true;
				}
				if ( error == false && guildUserSelect->restore == true )
				{
					// 복구 정보 send
					mpGuildManager->SendGuildRestore( guildUserSelect->idx );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHANNEL_CHECK:
			if ( mRequest.channelCheck )
			{
				cCSLock        lock( &mCs );
				CHANNEL_CHECK* channelCheck = (CHANNEL_CHECK*)perIoContext->buffer;

				wcscpy( mServerName, channelCheck->serverName );
				mChannelNum = channelCheck->channelNum;

				if ( g_verbose == true )
				{
					printf( "ServerNum(=%d) - ChannelNum(=%d) is started.\n", mServerNum, mChannelNum );
				}
				PostServerEvent( "ServerNum(=%d) - ChannelNum(=%d) is started.", mServerNum, mChannelNum );

				mRequest.channelCheck  = false;
				mComplete.channelCheck = true;
			}
			break;
        case SQL_GAME_PROCESS_CAPTCHA_OFFSET:
            if ( mRequest.captcha )
            {
				cCSLock    lock( &mCs );
				CAPTCHA_OFFSET* captcha = (CAPTCHA_OFFSET*)perIoContext->buffer;

				if ( captcha->offset > 0 )
				{
					mCaptchaMax = captcha->offset;
					mCaptchaOffset = (unsigned long)(mpRandomTable->Get( ) * 100000.f) % mCaptchaMax ;
				}

				PostServerEvent( "ServerNum(=%d)::ChannelNum(=%d)is the result of Captcha Image Offset Max(=%d), Offset(=%d).",
								 mServerNum,
								 mChannelNum,
								 mCaptchaMax,
                                 mCaptchaOffset );

                mComplete.captcha = true;
                mRequest.captcha  = false;
            }
            break;
		case SQL_REQUEST_GAME_INIT:
			if ( mRequest.initGameDb )
			{
				cCSLock    lock( &mCs );
				INIT_GAME* initGame = (INIT_GAME*)perIoContext->buffer;

				PostServerEvent( "ServerNum(=%d)::ChannelNum(=%d)is the result of the Game Database initialization. Remove Cooltime (=%d).",
								 mServerNum,
								 mChannelNum,
								 initGame->removeCoolTimeCount );

				mStatus = _E_STATUS_RUNABLE_;

				mComplete.initGameDb = true;
				mRequest.initGameDb = false;
			}
			break;
		case SQL_REQUEST_GAME_SHUTDOWN:
			if ( mRequest.shutdownGameDb )
			{
				cCSLock        lock( &mCs );
				SHUTDOWN_GAME* shutdownGame = (SHUTDOWN_GAME*)perIoContext->buffer;

				PostServerEvent( "ServerNum(=%d)::ChannelNum(=%d)is the result of termination of the Game Database. Remove Cooltime (=%d).",
								 mServerNum,
								 mChannelNum,
								 shutdownGame->removeCoolTimeCount );

				mComplete.shutdownGameDb = true;
				mRequest.shutdownGameDb = false;
			}
			break;

		case SQL_GAME_PROCESS_THEME_CREATE:
			{
				cCSLock       lock( &mCs );
				THEME_CREATE* create = (THEME_CREATE*)perIoContext->buffer;

				PostServerEvent( "ThemeIdx(=%d)::FixTime(=%02d)::VarTime(=%02d)::MaxRoom(=%d):Result(=%d) - LastDate(%04d-%02d-%02d %02d) - ValidThru(%04d-%02d-%02d %02d)."
								,create->themeIdx
								,create->fixTime
								,create->varTime
								,create->maxRoom
								,create->retvalue
								,create->lastDate.year
								,create->lastDate.month
								,create->lastDate.day
								,create->lastDate.hour
								,create->validThru.year
								,create->validThru.month
								,create->validThru.day
								,create->validThru.hour );

				time_t ltime = timestamp2time_t( create->validThru );
				sThemeData* pData = GetThemeData( create->themeIdx );
				if( pData != NULL )
				{
					long  result = (long)difftime( ltime, pData->validThru );
					if ( result > 0 )
					{
						pData->themeIdx  = create->themeIdx;
						pData->fixTime   = create->fixTime;
						pData->varTime   = create->varTime;
						pData->maxRoom   = create->maxRoom;
						pData->lastDate  = timestamp2time_t( create->lastDate );
						pData->validThru = ltime;
						THEMEMANAGER->ThemeRestart();
					}
					else
					{
						time( &ltime );
						result = (long)difftime( pData->validThru, ltime );
						if( result < 0 )
							result = 0;
						THEMEMANAGER->SetThemeRestartTime( result * SECOND );
					}
				}

				mComplete.initThemeDb = true;
				mRequest.initThemeDb  = false;
			}
			break;
		case SQL_GAME_PROCESS_THEME_ROOM_CHECK:
			{
				cCSLock           lock( &mCs );
				THEME_ROOM_CHECK* themeRoomCheck = (THEME_ROOM_CHECK*)perIoContext->buffer;

				PerSocketContext* socketContext  = NULL;
				cPlayer*          player         = NULL;

				if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
				{
					player = mpObjectManager->GetPlayer( themeRoomCheck->characterIdx );
					if( player != NULL )
					{
						if( themeRoomCheck->retvalue == 0 )		/// 성공
						{
							socketContext = mSocketContextPool->GetCID( player->GetConnectionIdx( ) );
							if ( socketContext != NULL )
							{
								HANDLE             handle          = NULL;
								THEME_USER_INSERT* themeUserInsert = (THEME_USER_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_INSERT );

								themeUserInsert->themeIdx     = themeRoomCheck->themeIdx;
								themeUserInsert->themeMode    = themeRoomCheck->themeMode;
								themeUserInsert->roomIdx      = themeRoomCheck->roomIdx;
								themeUserInsert->characterIdx = themeRoomCheck->characterIdx;
								themeUserInsert->retvalue     = 0;

								sMapInfo* info = STAGESCRIPT->GetMapFolderIdx( themeRoomCheck->themeIdx, themeRoomCheck->themeMode );
								themeUserInsert->characterMaxCnt = ( info ) ? info->maxPlayer : 0;

								SendSQL( socketContext, handle, sizeof(THEME_USER_INSERT) );
							}
						}
						else
						{
							THEMEMANAGER->ThemeDBErr( themeRoomCheck->characterIdx, ERROR_THEME_READY_FULL_MAKEROOM );

							PostServerEvent( "Warning - SQL_GAME_PROCESS_THEME_ROOM_CHECK - CHARACTER_IDX(=%d) RETVALUE(=%d)"
											,themeRoomCheck->characterIdx
											,themeRoomCheck->retvalue );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_USER_CHECK:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock           lock( &mCs );
				ClientInfo*       clientInfo     = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.themeUserCheck )
				{
					clientInfo->commonDb.themeUserCheck = false;
					clientInfo->request.characterSelect = true;

					THEME_USER_CHECK* themeUserCheck = (THEME_USER_CHECK*)perIoContext->buffer;

					if ( themeUserCheck->roomIdx != 0 )
					{
						clientInfo->complete.themeUser = true;
						clientInfo->themeRoomIdx = themeUserCheck->roomIdx;
						clientInfo->themeMode = themeUserCheck->themeMode;
						clientInfo->isCheatDungeonIn = false;
					}
					else if ( clientInfo->slevel == 1 )
					{
						clientInfo->themeRoomIdx = INDUN_MAP_MAX;
						clientInfo->themeMode = DEFAULT_MAPMODE;
						clientInfo->isCheatDungeonIn = true;

						for( unsigned char i = eINDUNMODE_MIN ; i < eINDUNMODE_MAX ; ++i )
						{
							if( STAGESCRIPT->GetMapFolderIdx( GetInDunMapNum(), i ) != NULL )
							{
								clientInfo->themeMode = i;
								break;
							}
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_USER_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock            lock( &mCs );
				ClientInfo*        clientInfo      = (ClientInfo*)perSocketContext->buffer;
				cPlayer*           hero            = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				THEME_USER_INSERT* themeUserInsert = (THEME_USER_INSERT*)perIoContext->buffer;

				if( themeUserInsert->retvalue == 0 )
				{
					sThemeData* pData = GetThemeData( themeUserInsert->themeIdx );
					if( pData != NULL )
					{
						GotoInstantDungeon( perSocketContext, pData->cid );
						hero->EndRequestRejection( eREQREJCT_THEME );
					}
					else
					{
						PostServerEvent("SQL_GAME_PROCESS_THEME_USER_INSERT pData[%d] != NULL", themeUserInsert->themeIdx );
						THEMEMANAGER->ThemeDBErr( clientInfo->characterIdx, ERROR_THEME_READY_SERVER_NOTRUN );
					}
				}
				else
				{
					unsigned char errorCode = ERROR_THEME_READY_ALREADY_JOIN;
					if( themeUserInsert->retvalue == 3 )
						errorCode = ERROR_THEME_READY_PLAYER_FULL;
					else
					{
						PostServerEvent("SQL_GAME_PROCESS_THEME_USER_INSERT themeUserInsert->retvalue != 3[%d,%d]", 
							themeUserInsert->retvalue, clientInfo->characterIdx );
					}

					THEMEMANAGER->ThemeDBErr( clientInfo->characterIdx, errorCode );
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_USER_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.themeUser )
				{
					clientInfo->gameInDb.themeUser = false; // DB; 사용종료.
					clientInfo->complete.themeUser = true;

					THEME_USER_SELECT* themeUserSelect = (THEME_USER_SELECT*)perIoContext->buffer;
					THEME_USER*        table           = themeUserSelect->table;

					hero->AddThemeUser( table, themeUserSelect->rowCount );
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_USER_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock            lock( &mCs );
				ClientInfo*        clientInfo      = (ClientInfo*)perSocketContext->buffer;
				cPlayer*           hero            = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				THEME_USER_UPDATE* themeUserUpdate = (THEME_USER_UPDATE*)perIoContext->buffer;

				// themeUserUpdate->retvalue == 0 성공
				//							 == 1 실패-등록안된 캐릭터
				//							 == 2 실패-인원초과
				//							 == 3 실패-알수없는 오류
				if ( themeUserUpdate->roomIdx != 0 && themeUserUpdate->retvalue == 0 )
				{
					/// 업데이트 결과후 채널이동
					sThemeData* pData = GetThemeData( themeUserUpdate->themeIdx );
					if( pData != NULL )
					{
						GotoInstantDungeon( perSocketContext, pData->cid );
						hero->EndRequestRejection( eREQREJCT_THEME );
					}
					else
					{
						PostServerEvent("SQL_GAME_PROCESS_THEME_USER_UPDATE pData[%d] != NULL", themeUserUpdate->themeIdx );
						THEMEMANAGER->ThemeDBErr( clientInfo->characterIdx, ERROR_THEME_READY_SERVER_NOTRUN );
					}
				}
				else
				{
					unsigned char errorCode = ERROR_THEME_READY_SERVER_NOTRUN;
					if( themeUserUpdate->retvalue == 1 )
						errorCode = ERROR_THEME_READY_PARTY;
					else if( themeUserUpdate->retvalue == 2 )
						errorCode = ERROR_THEME_READY_PLAYER_FULL;
					
					/// 입장했던 방이 없는경우 - 파티/초대 가 아닌이상 입장불가
					THEMEMANAGER->ThemeDBErr( clientInfo->characterIdx, errorCode );
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_USER_REMOVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock            lock( &mCs );
				ClientInfo*        clientInfo      = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.themeUserRemove )
				{
					clientInfo->commonDb.themeUserRemove = false; // DB; 사용종료
					clientInfo->complete.themeUser = false;

					THEME_USER_REMOVE* themeUserRemove = (THEME_USER_REMOVE*)perIoContext->buffer;
					if ( themeUserRemove->retvalue == 0 )
					{
						// success
					}
					else
					{
						// error
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_USER_SAVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock          lock( &mCs );
				ClientInfo*      clientInfo    = (ClientInfo*)perSocketContext->buffer;
				THEME_USER_SAVE* themeUserSave = (THEME_USER_SAVE*)perIoContext->buffer;

				if ( themeUserSave->retvalue == 0 )
				{
					cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
					hero->AddThemeUser( (THEME_USER*)themeUserSave, 1 );
				}
				else
				{
					// error database?
				}
			}
			break;
		case SQL_GAME_PROCESS_GM_THEME_USER_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );

				THEME_GM_USER_SELECT* themeUserSelect = (THEME_GM_USER_SELECT*)perIoContext->buffer;
				THEME_USER*        table           = themeUserSelect->table;
				
				if( themeUserSelect->characterIdx == 0 )
				{
					char name[MAX_NAME_BUF*2]  = {0,};
					::WideCharToMultiByte(CP_ACP, 0, themeUserSelect->characterName , (int)-1, (LPSTR)name, MAX_NAME_BUFFER_SIZE, NULL, NULL);

					PostServerEvent( "SQL_GAME_PROCESS_GM_THEME_USER_SELECT NAME(=%s), CHARACTER_IDX(=%d), ROWCOUNT(=%d)",name ,themeUserSelect->characterIdx, themeUserSelect->rowCount );

					NETWORK2->SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_INFO_RES, ERROR_CHEAT_GM_THEME_INFO_NO_PLAYER );
					break;
				}

				if( themeUserSelect->rowCount == 0 )
				{
					NETWORK2->SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_INFO_RES, ERROR_CHEAT_GM_THEME_INFO_NO_DATA );
					break;
				}

				HANDLE              handle  = NULL;
				MSG_RES_CHEAT_THEME_INFO* sendMsg = (MSG_RES_CHEAT_THEME_INFO*)NETWORK2->GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_INFO_RES );
				sCheat_Theme_Info*		  themeInfoTable = sendMsg->mThemeInfo;

				if( handle )
				{
					for( int i = 0 ; i < themeUserSelect->rowCount ; ++i, ++table, ++themeInfoTable )
					{
						themeInfoTable->themeIdx  = table->themeIdx;
						themeInfoTable->uniqueIdx  = MAKETHEMECID( table->themeIdx, table->roomIdx );
						themeInfoTable->mode      = table->themeMode;						// 모드
						themeInfoTable->lastDate  = timestamp2time_t( table->lastDate );	// 마지막 업데이트(일)
						themeInfoTable->validThru = timestamp2time_t( table->validThru );	// 유효만기(일)
					}

					sendMsg->mRowCount = themeUserSelect->rowCount;
					sendMsg->ErrorCode = ERROR_CHEAT_GM_THEME_INFO_SUCCESS;
					sendMsg->mCharacterIdx = themeUserSelect->characterIdx;

					NETWORK2->SendMsgRoot( handle, sendMsg->GetMsgLength() );
				}
			}
			break;
		case SQL_GAME_PROCESS_THEME_PARTY_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock          lock( &mCs );
				ClientInfo*      clientInfo    = (ClientInfo*)perSocketContext->buffer;
				cPlayer*         hero          = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				THEME_PARTYINFO_SELECT* pPartyInfo = (THEME_PARTYINFO_SELECT*)perIoContext->buffer;

				HANDLE handle  = NULL;
				MSG_SYN_THEME_READYQUESTION* sendMsg = (MSG_SYN_THEME_READYQUESTION*)GetMsgRoot( &handle, perSocketContext,
					NM_THEME, NM_THEME_READYQUESTION_SYN );
				if( sendMsg != NULL )
				{
					sendMsg->mThemeMapNum = pPartyInfo->mThemeIdx;					
					sendMsg->mCount = 0;
					for( unsigned long i = 0 ; i < pPartyInfo->mCount ; ++ i )
					{
						if( pPartyInfo->mPartyInfo[i].mIsJoin == false && pPartyInfo->mPartyInfo[i].mIsDependency == false )
							continue;

						sendMsg->mPartyInfo[sendMsg->mCount].mPlayerIdx = pPartyInfo->mPartyInfo[i].mPlayerIdx;
						sendMsg->mPartyInfo[sendMsg->mCount].mThemeMode = pPartyInfo->mPartyInfo[i].mMode;
						sendMsg->mPartyInfo[sendMsg->mCount].mIsJoin = pPartyInfo->mPartyInfo[i].mIsJoin;
						if( pPartyInfo->mPartyInfo[sendMsg->mCount].mIsDependency == true )
							sendMsg->mPartyInfo[sendMsg->mCount].mThemeUniqueIdx = MAKETHEMECID( pPartyInfo->mThemeIdx, pPartyInfo->mPartyInfo[i].mRoomIdx );
						else
							sendMsg->mPartyInfo[sendMsg->mCount].mThemeUniqueIdx = 0;

						++sendMsg->mCount;
					}

					SendMsgRoot( handle, sendMsg->GetMsgLength() );
				}

				hero->SetThemePartyDB( false );
			}
			break;
		case SQL_GAME_PROCESS_THEME_PARTYROOM_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock          lock( &mCs );
				ClientInfo*      clientInfo    = (ClientInfo*)perSocketContext->buffer;
				unsigned long    heroIdx       = clientInfo->characterIdx;
				cPlayer*         hero          = mpObjectManager->GetPlayer( heroIdx );
				THEME_PARTYROOM_SELECT* pRoomSelect = (THEME_PARTYROOM_SELECT*)perIoContext->buffer;

				PerSocketContext* perSocketContext = mSocketContextPool->GetCID( hero->GetConnectionIdx( ) );
				if( perSocketContext != NULL )
				{
					if( pRoomSelect->mRoomIdx == 0 )
					{
						/// 이미 들어가 있는 유저가 없어서 신규입장한다.
						HANDLE handle = NULL;
						THEME_ROOM_CHECK* themeRoomCheck = (THEME_ROOM_CHECK*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_ROOM_CHECK );
						themeRoomCheck->characterIdx = heroIdx;
						themeRoomCheck->themeIdx   = pRoomSelect->mThemeIdx;
						themeRoomCheck->themeMode  = pRoomSelect->mMode;

						SendSQL( perSocketContext, handle, sizeof(THEME_ROOM_CHECK) );
					}
					else
					{
						/// 이미 들어가 있는 파티원의 방으로 들어간다.
						HANDLE handle = NULL;
						THEME_USER_INSERT* themeUserInsert = (THEME_USER_INSERT*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_INSERT );

						themeUserInsert->themeIdx     = pRoomSelect->mThemeIdx;
						themeUserInsert->themeMode    = pRoomSelect->mMode;
						themeUserInsert->roomIdx      = pRoomSelect->mRoomIdx;
						themeUserInsert->characterIdx = heroIdx;
						themeUserInsert->retvalue     = 0;

						sMapInfo* info = STAGESCRIPT->GetMapFolderIdx( pRoomSelect->mThemeIdx, pRoomSelect->mMode );
						themeUserInsert->characterMaxCnt = ( info ) ? info->maxPlayer : 0;

						SendSQL( perSocketContext, handle, sizeof(THEME_USER_INSERT) );								
					}
				}

				hero->SetThemePartyDB( false );
			}
			break;

		case SQL_REQUEST_ACCOUNT_SHUTDOWN:
			{
				mComplete.shutdownAccountDb = true;
				mRequest.shutdownAccountDb = false;
			}
			break;
		case SQL_GAME_PROCESS_MEMBER_CHECK:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.check )
				{
					clientInfo->commonDb.check = false;
					// DB 사용종료.

					MEMBER_CHECK* memberCheck = (MEMBER_CHECK*)perIoContext->buffer;

					try
					{
						if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
							throw -1;						// Error [-1=구문실패].

						// 1. 결과 처리.
						if ( memberCheck->retvalue != 0 )
							throw memberCheck->retvalue;	// Error [1=체크실패/2=알수없는 오류].

						if ( memberCheck->memberIdx == 0 )
							throw -2;						// Error [-2=클라이언트 & 서버 통신오류].

						/*-- 중요1. CID 등록 --*/
						bool success = mSocketContextPool->SetCID( perSocketContext, memberCheck->memberIdx );
						if ( success == true )
						{
							// Login
							clientInfo->complete.check = true;
							clientInfo->delay.loginUpdate = GetTickCount( ) + 90000;

							clientInfo->memberIdx    = memberCheck->memberIdx;
							clientInfo->loginIdx     = memberCheck->loginIdx;
							clientInfo->lastChannel  = memberCheck->lastChannel;
							clientInfo->characterIdx = memberCheck->characterIdx;
							clientInfo->slevel       = memberCheck->slevel;
							clientInfo->pc           = memberCheck->pc;

							// 일본OBT 이벤트
//							clientInfo->cbtUser		 = memberCheck->cbtUser;
//							clientInfo->obtUser		 = memberCheck->obtUser;
//							clientInfo->fourGamer	 = memberCheck->fourGamer;

							// 운영자 클라이언트 확인 - 핵쉴드 사용 중지.
//							if ( clientInfo->slevel == 1 && clientInfo->mHSClientHandle != ANTICPX_INVALID_HANDLE_VALUE )
//							{
//								_AhnHS_CloseClientHandle( clientInfo->mHSClientHandle );
//								clientInfo->mHSClientHandle = ANTICPX_INVALID_HANDLE_VALUE;
//							}

							if ( memberCheck->instantDungeon != 0 )
							{
								switch ( mType )
								{
								default:
								case _E_ST_NORMAL_MAP_:
									clientInfo->request.joinMap = true;
									clientInfo->request.characterSelect = (clientInfo->characterIdx > 0);
									break;
								case _E_ST_ID_PVP_:
									clientInfo->request.joinInstantDungeon = true;
									clientInfo->request.characterSelect = (clientInfo->characterIdx > 0);
									break;
								case _E_ST_ID_THEME_:
									if ( clientInfo->characterIdx > 0 )
									{
										clientInfo->request.joinInstantDungeon = true;

										HANDLE            handle         = NULL;
										THEME_USER_CHECK* themeUserCheck = (THEME_USER_CHECK*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_USER_CHECK );

										themeUserCheck->themeIdx     = mInDunMapNumber;
										themeUserCheck->characterIdx = clientInfo->characterIdx;

										clientInfo->commonDb.themeUserCheck = SendSQL( perSocketContext, handle, sizeof(THEME_USER_CHECK) );
									}
									break;
								case _E_ST_ID_TUTORIAL_:
									{
										clientInfo->request.joinInstantDungeon = true;
										clientInfo->request.characterSelect = (clientInfo->characterIdx > 0);
									}
									break;
								}
							}
							else
							{
								clientInfo->request.characterSelect = (clientInfo->characterIdx > 0);
							}

							// 2. 결과 전송 [0성공].
							SendMsgError( perSocketContext, NM_USER, (char)NM_USER_GAMESRV_RES, memberCheck->retvalue );
						}
						else
							throw (long)3; // Error [3=EXISTS CID].
					}
					catch ( long error )
					{
						HANDLE      handle = NULL;
						MEMBER_OUT* out    = (MEMBER_OUT*)GetSQL( &handle, SQL_GAME_PROCESS_MEMBER_OUT );

						out->loginIdx  = memberCheck->loginIdx;
						out->memberIdx = memberCheck->memberIdx;
						out->error     = error;

						clientInfo->commonDb.out= SendSQL( perSocketContext, handle, sizeof(MEMBER_OUT) );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_MEMBER_LOGOUT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.logout )
				{
					clientInfo->commonDb.logout = false; // DB 사용종료.
					clientInfo->complete.check = false;	 // Logout

					MEMBER_LOGOUT* memberLogout = (MEMBER_LOGOUT*)perIoContext->buffer;

					// 로그.
					PostMemberEvent( EVENT_MEMBER_SUCCESS_AUDIT
									,EVENT_MEMBER_LOGOUT
									,memberLogout->memberIdx
									,"Inetnum(=%s) - ErrorCode(=%d)"
									,inet_ntoa( perSocketContext->addr.sin_addr )
									,memberLogout->retvalue );
				}
			}
			break;
		case SQL_GAME_PROCESS_MEMBER_OUT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.out )
				{
					clientInfo->commonDb.out = false; // DB 사용종료.

					// 로그.
					MEMBER_OUT* out = (MEMBER_OUT*)perIoContext->buffer;

					PostMemberEvent( EVENT_MEMBER_SUCCESS_AUDIT
									,EVENT_MEMBER_LOGOUT
									,out->memberIdx
									,"CriticalError - SQL_GAME_PROCESS_MEMBER_OUT Exists LOGIN_IDX(=%d), MEMBER_IDX(=%d), ERROR(=%d)"
									,out->loginIdx
									,out->memberIdx
									,out->error );

					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_LOGIN_UPDATE:
			break;
		case SQL_GAME_PROCESS_GAME_TO_GAME:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.gameToGame )
				{
					clientInfo->commonDb.gameToGame = false; // DB 사용종료.

					if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
					{
						GAME_TO_GAME* gameToGame = (GAME_TO_GAME*)perIoContext->buffer;

						if ( clientInfo->gameInDatabase )
						{
							PostServerEvent( "CriticalError - SQL_GAME_PROCESS_GAME_TO_GAME = 0x%x 0x%x"
											,clientInfo->gameInDatabase
											,clientInfo->completeData );
							Close( perSocketContext );
						}
						// 결과 전송 [0성공/1미등록서버/2알수없는 오류].
						else if ( clientInfo->complete.gameToGame && !gameToGame->retvalue )
						{
							/*-- 중요1. CID 정리 --*/
							mSocketContextPool->ClearCID( perSocketContext );

							// 게임서버 이동이므로, 로그아웃 처리를 무시.
							clientInfo->complete.check = false;
							SendGame2Game( perSocketContext, gameToGame->ipv4, gameToGame->port );
						}
						else
						{
							PostServerEvent( "Error - SQL_GAME_PROCESS_GAME_TO_GAME LOGIN_IDX(=%d), CHARACTER_IDX(=%d), CHANNEL(=%d)"
											,gameToGame->loginIdx
											,gameToGame->characterIdx
											,gameToGame->channelNum );
							Close( perSocketContext );
						}
					}
					else
					{
						PostServerEvent( "Error - SQL_GAME_PROCESS_GAME_TO_GAME - CHARACTER_IDX(=%d)", clientInfo->characterIdx );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_MEMBER_TWITTER:
			// Skip
			break;
		case SQL_GAME_PROCESS_MEMBER_OBT_EVENT_COMPLETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo  = (ClientInfo*)perSocketContext->buffer;

                if ( clientInfo->commonDb.obtEvent )
                {
                    clientInfo->commonDb.obtEvent = false;

                    cPlayer*					hero     = mpObjectManager->GetPlayer( clientInfo->characterIdx );
                    MEMBER_OBT_EVENT_COMPLETE*	obtEvent = (MEMBER_OBT_EVENT_COMPLETE*)perIoContext->buffer;

                    if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                    {
						clientInfo->cbtUser		= obtEvent->cbtUser;
						clientInfo->obtUser		= obtEvent->obtUser;
						clientInfo->fourGamer	= obtEvent->fourGamer;
                    }
                    else
                    {
						NETWORK2->PostServerEvent( "SQL_GAME_PROCESS_MEMBER_OBT_EVENT_COMPLETE - memberIdx(=%d), retval(=%d) [cbt:%d,obt:%d,four:%d]", 
                                                    hero->GetConnectionIdx(), 
                                                    obtEvent->retvalue,
													obtEvent->cbtUser,
													obtEvent->obtUser,
													obtEvent->fourGamer);
                    }
                }
			}
			break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_SELECT:
            if ( perSocketContext->socket == perIoContext->socket )
            {
                cCSLock lock( &mCs );
                ClientInfo*     clientInfo  = (ClientInfo*)perSocketContext->buffer;
                TRIAL_SELECT*   trialSelect = (TRIAL_SELECT*)perIoContext->buffer;

                if ( clientInfo->gameInDb.trial )
                {
                    clientInfo->gameInDb.trial = false;
				    cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );

                    if ( hero != NULL )
                    {
                        if ( trialSelect->trialTime > 0 )
                            hero->SetTrialTime( plusSecTime_t( trialSelect->trialTime ) );
                        else
                            hero->SetTrialTime( 0 );

                        if ( trialSelect->trialRejectTime > 0 )
                            hero->SetTrialRejectTime( plusSecTime_t( trialSelect->trialRejectTime ) );
                        else
                            hero->SetTrialRejectTime( 0 );
                    }
                }
            }
            break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_CHECK:
            if ( perSocketContext->socket == perIoContext->socket )
            {
                cCSLock     lock( &mCs );
                ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

                if ( clientInfo->commonDb.trial )
                {
                    clientInfo->commonDb.trial = false;
                    cPlayer*      hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
                    MEMBER_TRIAL* trialCheck = (MEMBER_TRIAL*)perIoContext->buffer;

                    if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                    {

                        cPlayer* pTarget = mpObjectManager->GetPlayer( trialCheck->autoCharacterIdx );
                        int      error   = ERROR_PLAYER_TRIAL_OPEN_FAIL;

                        // 결과 [0:성공/1:알수없는오류/2:재사용시간 안됨]
                        if( trialCheck->retvalue == 0 && pTarget != NULL )
                        {
                            HANDLE          handle = NULL;
                            CAPTCHA_SELECT*  captchaSelect  = (CAPTCHA_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_CAPTCHA_SELECT );
                            captchaSelect->tiralMemberIdx   = clientInfo->memberIdx;
                            captchaSelect->autoCharacterIdx = trialCheck->autoCharacterIdx;
                            captchaSelect->captchaOffset    = mCaptchaOffset;

                            if ( SendSQL( perSocketContext, handle, sizeof(CAPTCHA_SELECT)) == true )
                            {
                                clientInfo->commonDb.trial = true;
                                error = ERROR_PLAYER_TRIAL_OPEN_SUCCESS;
                            }
                        }
                        else if( trialCheck->retvalue == 2 )
                        {
                            error = ERROR_PLAYER_TRIAL_OPEN_TIME;
                        }

                        /// 신고 결과 전송
                        hero->SendTrialReqResult( (ULONG_PTR)perSocketContext, error, pTarget );
                    }
                }
            }
            break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
                if ( clientInfo->commonDb.trial )
                {
                    clientInfo->commonDb.trial = false;
                    MEMBER_TRIAL* trialUpdate = (MEMBER_TRIAL*)perIoContext->buffer;

                    if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                    {
                        cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
                        if ( hero )
                            hero->SetTrialTime( plusSecTime_t( trialUpdate->trialTime ) );
                    }
                    else
                    {
                        PostServerEvent( "FAIL - SQL_GAME_PROCESS_MEMBER_TRIAL_UPDATE, heroIdx=%d, autoIdx=%d, retval=%d",
                            clientInfo->characterIdx, trialUpdate->autoCharacterIdx, trialUpdate->retvalue);
                    }
                }
            }
            break;
        case SQL_GAME_PROCESS_MEMBER_TRIAL_REJECT_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo  = (ClientInfo*)perSocketContext->buffer;

                if ( clientInfo->commonDb.trial )
                {
                    clientInfo->commonDb.trial = false;

                    cPlayer*      hero        = mpObjectManager->GetPlayer( clientInfo->characterIdx );
                    TRIAL_REJECT* trialReject = (TRIAL_REJECT*)perIoContext->buffer;

                    if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                    {
                        if ( hero && trialReject->rejectTime > 0 )
                            hero->SetTrialRejectTime( plusSecTime_t( trialReject->rejectTime ) );
                    }
                    else
                    {
                        NETWORK2->PostServerEvent( "SQL_GAME_PROCESS_MEMBER_TRIAL_REJECT_UPDATE - memberIdx(=%d), retval(=%d)", 
                                                    hero->GetConnectionIdx(), 
                                                    trialReject->retvalue );
                    }
                }
            }
            break;
        case SQL_GAME_PROCESS_CAPTCHA_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
                if ( clientInfo->commonDb.trial == true )
                {
                    clientInfo->commonDb.trial = false;
                    
                    CAPTCHA_SELECT* captchaSelect = (CAPTCHA_SELECT*)perIoContext->buffer;

                    // captcha 이미지 offset 증가
                    CaptchaOffsetPlus( );

                    if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                    {
                        if ( captchaSelect->captchaSize > MAX_CAPTCHA_SIZE )
                        {
                            PostServerEvent( "FAIL - SQL_GAME_PROCESS_TRIAL_CAPTCHA_SELECT, heroIdx=%d, trialIdx=%d, autoIdx=%d, offset=%d,size=%d,retval=%d",
                                clientInfo->characterIdx, 
                                captchaSelect->tiralMemberIdx, 
                                captchaSelect->autoCharacterIdx, 
                                captchaSelect->captchaOffset,
                                captchaSelect->captchaSize, 
                                captchaSelect->retvalue );
                        }
                        else
                        {
                            cPlayer* hero   = mpObjectManager->GetPlayer( clientInfo->characterIdx );
                            cPlayer* pTarget= mpObjectManager->GetPlayer( captchaSelect->autoCharacterIdx );

                            if ( pTarget != NULL )
                            {
                                PerSocketContext*           pOtherContext = mSocketContextPool->GetCID( pTarget->GetConnectionIdx() );
                                HANDLE					    handle        = NULL;
                                MSG_SYN_PLAYER_TRIAL_OPEN*  sendMsg       = (MSG_SYN_PLAYER_TRIAL_OPEN*)GetMsgRoot( &handle, pOtherContext, NM_PLAYER, NM_PLAYER_TRIAL_OPEN_SYN );
                                if ( sendMsg != NULL )
                                {
                                    sendMsg->size = captchaSelect->captchaSize;
                                    memcpy( &(sendMsg->captcha), captchaSelect->captcha, sizeof(BYTE)*captchaSelect->captchaSize );

                                    SendMsgRoot( handle, sendMsg->GetMsgLength() );
                                }

                                // 심판 시작
                                pTarget->TrialStart( captchaSelect->tiralMemberIdx, captchaSelect->captchaName );
                            }

                            // 신고자 재사용시간 기본 3분
                            if ( hero )
                                hero->SetTrialTime( plusSecTime_t( 180 ) );
                        }
                    }
                    else
                    {
                        PostServerEvent( "FAIL - SQL_GAME_PROCESS_TRIAL_CAPTCHA_SELECT, heroIdx=%d, trialIdx=%d, autoIdx=%d, retval=%d",
                            clientInfo->characterIdx, captchaSelect->tiralMemberIdx, captchaSelect->autoCharacterIdx, captchaSelect->retvalue);
                    } // IOCP_REQUEST_SUCCESS
                }
            }
            break;
        case SQL_GAME_PROCESS_CAPTCHA_RELOAD:
            if ( perSocketContext->socket == perIoContext->socket )
            {
                cCSLock     lock( &mCs );
                ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

                // captcha 이미지 offset 증가
                CaptchaOffsetPlus( );

                HANDLE  handle  = NULL;
                MSG_RES_PLAYER_TRIAL_CAPTCHA* sendMsg = (MSG_RES_PLAYER_TRIAL_CAPTCHA*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_TRIAL_CAPTCHA_RES );
                sendMsg->ErrorCode  = ERROR_PLAYER_TRIAL_CAPTCHA_FAIL;

                if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                {
                    CAPTCHA_RELOAD* captchaReload = (CAPTCHA_RELOAD*)perIoContext->buffer;
                    if ( captchaReload->captchaSize <= MAX_CAPTCHA_SIZE )
                    {
                        cPlayer* hero = mpObjectManager->GetPlayer( clientInfo->characterIdx );
                        if( hero )
                        {
                            hero->SetCaptchaName( captchaReload->captchaName );
                            hero->CaptChaCountDown( );

                            sendMsg->ErrorCode  = ERROR_PLAYER_TRIAL_CAPTCHA_SUCCESS;
                            sendMsg->count      = hero->GetCaptChaCount();
                            sendMsg->size       = captchaReload->captchaSize;
                            memcpy( &sendMsg->captcha, captchaReload->captcha, sizeof(BYTE)*captchaReload->captchaSize );
                        }
                    }
                    else
                    {
                        PostServerEvent( "FAIL - SQL_GAME_PROCESS_TRIAL_CAPTCHA_RELOAD, heroIdx(%d), offset(%d),captchaSize(%d)", 
                            clientInfo->characterIdx, captchaReload->captchaOffset, captchaReload->captchaSize);
                    }
                }

                SendMsgRoot( handle, sendMsg->GetMsgLength() );
            }
			break;
		case SQL_GAME_PROCESS_WHISPER:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				unsigned long	heroIdx	   = hero->GetObjectID();

				if ( clientInfo->commonDb.characterLogin == true )
				{
					clientInfo->commonDb.characterLogin = false; // DB; 사용종료

					if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
					{
						WHISPER_SELECT* select = (WHISPER_SELECT*)perIoContext->buffer;

						if ( select->idx > 0 )
						{
							// 링크 아이템 정보 
							sInventory inven;
							memset( &inven, 0, sizeof(inven) );

							TB_INVENTORY* tbInventory = hero->SelectInventory( select->slotIndex );
							if ( hero->IsInventory( tbInventory ) == true )
								Inventory2sInventory( &inven, tbInventory );

							// 해당 유저가 다른 채널에 접속중임.
							PostChatWhisper( heroIdx, (unsigned short)hero->GetJob(), hero->GetName(), select->name, select->message, &inven );
						}
						else
						{
							// 해당 유저가 다른 채널에 접속중이 아님
							// 보낸 캐릭터에게.
							SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_WHISPER_RES, ERROR_CHAT_WHISPER_DISCONNECT );
						}
					}
				}
			}
			break;
        case SQL_GAME_PROCESS_MEMBER_LOGIN_LIMIT:
            if ( perSocketContext->socket == perIoContext->socket )
            {
                cCSLock     lock( &mCs );
                ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
                
                //if ( clientInfo->commonDb.loginLimit )
                {
                    //clientInfo->commonDb.loginLimit = false;
                    LOGIN_LIMIT* loginLimit = (LOGIN_LIMIT*)perIoContext->buffer;
                    if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS || loginLimit->retvalue != 0 )
                    {
                        PostServerEvent( "FAIL - SQL_GAME_PROCESS_MEMBER_LOGIN_LIMIT, heroIdx(%d) count(%d) retvalue(=%d)", 
                                        clientInfo->memberIdx, loginLimit->count, loginLimit->retvalue );
                    }
                    CloseCID( clientInfo->memberIdx );
                }
            }
            break;

		case SQL_GAME_PROCESS_CHARACTER_LIST:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterList )
				{
					clientInfo->commonDb.characterList = false; // DB 사용종료.

					// 결과 전송.
					CHARACTER_LIST* characterList = (CHARACTER_LIST*)perIoContext->buffer;

					HANDLE              handle  = NULL;
					MSG_CHARACTER_LIST* sendMsg = (MSG_CHARACTER_LIST*)GetMsgRoot( &handle, perSocketContext, NM_USER, NM_USER_CHARACTER_LIST_RES );
					if ( sendMsg != NULL )
					{
						TB_CHARACTER_LIST*      table    = characterList->table;
						MSG_CHARACTER_BASEINFO* baseInfo = sendMsg->BaseInfo;

						sPlayerInfo*            playerInfo;
						sPlayerWeaponInfo*      weaponInfo;
						sPlayerWearInfo*        wearInfo;

                        if ( characterList->retvalue == 0 )
                        {
                            if ( characterList->rowCount > 0 )
                            {
                                sendMsg->StandIdx = table->standIdx;
                            }

                            for ( long i = 0; i < characterList->rowCount; i++, table++ )
                            {
                                if ( table->apply == 2 && table->delWaitTime < 0 )
                                {
                                    CharacterDelete( perSocketContext, clientInfo->memberIdx, table->idx );

                                    // 마지막 사용 캐릭터 삭제시 기본 0
                                    if ( sendMsg->StandIdx == table->standIdx )
                                    {
                                         sendMsg->StandIdx = 0;
                                    }
                                    continue;
                                }

                                // sPlayerInfo
                                playerInfo = &baseInfo->PlayerInfo;
                                playerInfo->CharacterIdx                = table->idx;
                                wcscpy( playerInfo->strName, table->name );
                                playerInfo->Level                       = table->level;
                                playerInfo->Race                        = table->race;
                                playerInfo->Gender                      = table->gender;
                                playerInfo->Job                         = (ePLAYER_JOB)table->job;
                                playerInfo->HeadInfo[ eHEAD_HAIR      ] = table->hair;
                                playerInfo->HeadInfo[ eHEAD_HAIRCOLOR ] = table->hairColor;
                                playerInfo->HeadInfo[ eHEAD_FACE      ] = table->face;

                                // sPlayerWeaponInfo
                                weaponInfo = &baseInfo->WeaponInfo;
                                if ( table->activeWeapon == ItemActiveFront )
                                {
                                    weaponInfo->WeaponIdx[ eHAND_LEFT  ]      = table->equipItem[ INVENTORY_HAND_LEFT2  ];
                                    weaponInfo->WeaponIdx[ eHAND_RIGHT ]      = table->equipItem[ INVENTORY_HAND_RIGHT2 ];
                                    weaponInfo->WeaponEnhanced[ eHAND_LEFT  ] = table->enhancedItem[ INVENTORY_HAND_LEFT2  ];
                                    weaponInfo->WeaponEnhanced[ eHAND_RIGHT ] = table->enhancedItem[ INVENTORY_HAND_RIGHT2 ];
                                }
                                else
                                {
                                    weaponInfo->WeaponIdx[ eHAND_LEFT  ]      = table->equipItem[ INVENTORY_HAND_LEFT1  ];
                                    weaponInfo->WeaponIdx[ eHAND_RIGHT ]      = table->equipItem[ INVENTORY_HAND_RIGHT1 ];
                                    weaponInfo->WeaponEnhanced[ eHAND_LEFT  ] = table->enhancedItem[ INVENTORY_HAND_LEFT1  ];
                                    weaponInfo->WeaponEnhanced[ eHAND_RIGHT ] = table->enhancedItem[ INVENTORY_HAND_RIGHT1 ];
                                }

                                // sPlayerWearInfo
                                wearInfo = &baseInfo->WearInfo;
                                wearInfo->WearIdx[ eWEAR_BODY1 ]         = table->equipItem[ INVENTORY_WEAR_BODY1    ];
                                wearInfo->WearIdx[ eWEAR_BODY2 ]         = table->equipItem[ INVENTORY_WEAR_BODY2    ];
                                wearInfo->WearIdx[ eWEAR_HAND  ]         = table->equipItem[ INVENTORY_WEAR_HAND     ];
                                wearInfo->WearIdx[ eWEAR_FOOT  ]         = table->equipItem[ INVENTORY_WEAR_FOOT     ];
                                wearInfo->WearIdx[ eWEAR_HAT   ]         = table->equipItem[ INVENTORY_WEAR_HAT      ];
                                wearInfo->Earring                        = table->equipItem[ INVENTORY_WEAR_EARRING  ];
                                wearInfo->Necklace                       = table->equipItem[ INVENTORY_WEAR_NECKLACE ];
                                wearInfo->Brooch                         = table->equipItem[ INVENTORY_WEAR_BROOCH   ];
                                wearInfo->Bracelet                       = table->equipItem[ INVENTORY_WEAR_BRACELET ];
                                wearInfo->Ring                           = table->equipItem[ INVENTORY_WEAR_RING     ];

                                wearInfo->CostumeIdx[ eWEAR_BODY1 ]      = table->equipItem[ INVENTORY_COSTUME_BODY1 ];
                                wearInfo->CostumeIdx[ eWEAR_BODY2 ]      = table->equipItem[ INVENTORY_COSTUME_BODY2 ];
                                wearInfo->CostumeIdx[ eWEAR_HAND  ]      = table->equipItem[ INVENTORY_COSTUME_HAND  ];
                                wearInfo->CostumeIdx[ eWEAR_FOOT  ]      = table->equipItem[ INVENTORY_COSTUME_FOOT  ];
                                wearInfo->CostumeIdx[ eWEAR_HAT   ]      = table->equipItem[ INVENTORY_COSTUME_HAT   ];
                                wearInfo->CostumeAccIdx[ eCOSTUME_HEAD ] = table->equipItem[ INVENTORY_COSTUME_FACE  ];
                                wearInfo->CostumeAccIdx[ eCOSTUME_BACK ] = table->equipItem[ INVENTORY_COSTUME_BACK  ];

                                // Etc...
                                baseInfo->MapNum        = table->mapNum;
                                baseInfo->StandIdx      = table->standIdx;
                                baseInfo->DelWaitTime   = table->delWaitTime * 1000;

                                baseInfo++;
                                sendMsg->CharacterNum++;
                            }
                        }
                        else
                        {
                            sendMsg->ErrorCode = 1;
                        }
						SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_NAME:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterName )
				{
					clientInfo->commonDb.characterName = false; // DB 사용종료.

					// 결과 전송.
					CHARACTER_NAME* characterName = (CHARACTER_NAME*)perIoContext->buffer;
					SendMsgError( perSocketContext, NM_USER, (char)NM_USER_CHARACTER_NAME_RES, characterName->retvalue );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterInsert )
				{
					clientInfo->commonDb.characterInsert = false; // DB 사용종료.

					CHARACTER_INSERT* characterInsert = (CHARACTER_INSERT*)perIoContext->buffer;

					if ( characterInsert->retvalue == 0 )
					{
					    // 캐릭터 이벤트.
					    PostCharacterEvent( EVENT_CHARACTER_CREATE
						    			   ,clientInfo->memberIdx
							    		   ,characterInsert->idx
								    	   ,1						// 기본값 또는 바인딩(DB)
									       ,0						// "
    									   ,1						// "
	    								   ,0						// "
		    							   ,0						// "
			    						   ,"Inetnum(=%s)"
				    					   ,inet_ntoa( perSocketContext->addr.sin_addr ) );

                        TB_INVENTORY* table = NULL;
    
						// 캐릭터 생성시 지급된 기본 아이템 내역 기록(Log).
						for ( long i = 0; i < characterInsert->rowCount; i++ )
						{
							table = &characterInsert->table[ i ];

							PostInventoryEvent( EVENT_INVENTORY_CREATE, characterInsert->idx, table, "DEFAULT_ITEMS" );
						}
					}

					// 결과 전송.
					SendMsgError( perSocketContext, NM_USER, NM_USER_CHARACTER_CREATE_RES, characterInsert->retvalue );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_DELETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock             lock( &mCs );
				ClientInfo*         clientInfo      = (ClientInfo*)perSocketContext->buffer;
                CHARACTER_DELETE*   characterDelete = (CHARACTER_DELETE*)perIoContext->buffer;

                if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS && characterDelete->retvalue == 0 )
                {
                    if ( characterDelete->guild_index > 0 )
                    {
                        // 길드 정보 삭제
                        cGuild* guild = mpGuildManager->GetGuild( characterDelete->guild_index );
                        if( guild && guild->DeletePlayer( characterDelete->idx ) == true )
                        {
                            // 성공 로그
                            PostGuildEvent( characterDelete->idx, characterDelete->guild_index, eGUILD_NONE, 
                                            "SUCCESS - SQL_GAME_PROCESS_CHARACTER_DELETE DeletePlayer()" );


                            // 길드원들에게 쏘기
                            mpGuildManager->SendGuildUserOut( guild, characterDelete->idx );

                            /// SYN : 길드 탈퇴
                            if( mpGuildManager->PostGuildUserOut( characterDelete->guild_index, characterDelete->idx ) == false )
                            {
                                PostGuildEvent( characterDelete->idx, characterDelete->guild_index, eGUILD_NONE, 
                                                "FAIL - SQL_GAME_PROCESS_CHARACTER_DELETE, PostGuildUserOut()" );
                            }
                        }
                        else
                        {
                            PostGuildEvent( characterDelete->idx, characterDelete->guild_index, 0, 
                                            "FAIL - SQL_GAME_PROCESS_CHARACTER_DELETE DeletePlayer()" );
                        }
                    }

                    // 캐릭터 이벤트.
                    PostCharacterEvent( EVENT_CHARACTER_DELETE
                                        ,clientInfo->memberIdx
                                        ,characterDelete->idx
                                        ,characterDelete->level
                                        ,characterDelete->exp
                                        ,characterDelete->skillLevel
                                        ,characterDelete->skillExp
                                        ,characterDelete->skillPointTotal
                                        ,"Inetnum(=%s)"
                                        ,inet_ntoa( perSocketContext->addr.sin_addr ) );

                    // 클라이언트에 의한 요청의 경우 결과 전송
                    if ( characterDelete->request )
                    {
                        HANDLE                    handle  = NULL;
                        MSG_RES_CHARACTER_DELETE* sendMsg = (MSG_RES_CHARACTER_DELETE*)GetMsgRoot( &handle, 
                                                                                                   perSocketContext, 
                                                                                                   NM_USER, 
                                                                                                   NM_USER_CHARACTER_DELETE_RES );
                        sendMsg->ErrorCode = characterDelete->retvalue;
                        sendMsg->StandIdx  = characterDelete->standIdx;
                        SendMsgRoot( handle, sizeof(MSG_RES_CHARACTER_DELETE) );
                    }
                }
                else
                {
                    PostServerEvent( "FAIL - SQL_GAME_PROCESS_CHARACTER_DELETE requestResult error : retvalue(%d) characterIdx(%d) heroIdx(%d)", 
                                     characterDelete->retvalue, characterDelete->idx, characterDelete->userIdx );
                }
            }
			break;
        case SQL_GAME_PROCESS_CHARACTER_DELETE_WAIT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterDelete )
				{
					clientInfo->commonDb.characterDelete = false; // DB 사용종료.

					CHARACTER_DELETE_WAIT* characterDelWait = (CHARACTER_DELETE_WAIT*)perIoContext->buffer;

					// 결과 전송.
					HANDLE                         handle  = NULL;
					MSG_RES_CHARACTER_DELETE_WAIT* sendMsg = (MSG_RES_CHARACTER_DELETE_WAIT*)GetMsgRoot( &handle, 
                                                                                                         perSocketContext, 
                                                                                                         NM_USER, 
                                                                                                         NM_USER_CHARACTER_DELETE_WAIT_RES );
					sendMsg->ErrorCode      = characterDelWait->retvalue;
					sendMsg->StandIdx       = characterDelWait->standIdx;
                    sendMsg->DelWaitTime    = characterDelWait->delWaitTime * 1000;

                    SendMsgRoot( handle, sizeof(MSG_RES_CHARACTER_DELETE_WAIT) );

                    if ( characterDelWait->retvalue == 0 )
                    {
						// 캐릭터 이벤트.
                        PostCharacterEvent( EVENT_CHARACTER_DELETE_WAIT
                                           ,clientInfo->memberIdx
                                           ,characterDelWait->idx
                                           ,characterDelWait->level
                                           ,characterDelWait->exp
                                           ,characterDelWait->skillLevel
                                           ,characterDelWait->skillExp
                                           ,characterDelWait->skillPointTotal
                                           ,"Inetnum(=%s)"
                                           ,inet_ntoa( perSocketContext->addr.sin_addr ) );
                    }
				}
			}
            break;
        case SQL_GAME_PROCESS_CHARACTER_DELETE_CANCEL:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

                if ( clientInfo->commonDb.characterDelete )
				{
					clientInfo->commonDb.characterDelete = false; // DB 사용종료.
                        
                    CHARACTER_DELETE_CANCEL* characterDelCancel = (CHARACTER_DELETE_CANCEL*)perIoContext->buffer;

                    HANDLE                           handle  = NULL;
                    MSG_RES_CHARACTER_DELETE_CANCEL* sendMsg = (MSG_RES_CHARACTER_DELETE_CANCEL*)GetMsgRoot( &handle, 
                                                                                                             perSocketContext, 
                                                                                                             NM_USER, 
                                                                                                             NM_USER_CHARACTER_DELETE_CANCEL_RES );
                    sendMsg->StandIdx  = characterDelCancel->standIdx;

                    if ( characterDelCancel->retvalue == 0 )
                    {
                        sendMsg->ErrorCode = ERROR_CHARACTER_DELETE_CANCEL_SUCCESS;

						// 캐릭터 이벤트.
                        PostCharacterEvent( EVENT_CHARACTER_DELETE_CANCEL
                                           ,clientInfo->memberIdx
                                           ,characterDelCancel->idx
                                           ,characterDelCancel->level
                                           ,characterDelCancel->exp
                                           ,characterDelCancel->skillLevel
                                           ,characterDelCancel->skillExp
                                           ,characterDelCancel->skillPointTotal
                                           ,"Inetnum(=%s)"
                                           ,inet_ntoa( perSocketContext->addr.sin_addr ) );
                    }
                    else
                    {
                        sendMsg->ErrorCode = ERROR_CHARACTER_DELETE_CANCEL_FAIL;
                        PostServerEvent( "FAIL - SQL_GAME_PROCESS_CHARACTER_DELETE_CANCEL, heroIdx(%d), retvalue(%d)", 
                                            characterDelCancel->idx, characterDelCancel->retvalue );
                    }

                    SendMsgRoot( handle, sizeof(MSG_RES_CHARACTER_DELETE_CANCEL) );
                }
            }
            break;
		case SQL_GAME_PROCESS_CHARACTER_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.characterUpdate )
				{
					clientInfo->commonDb.characterUpdate = false;
					clientInfo->complete.characterUpdate = false;
				}
				if ( hero != NULL )
				{
					hero->EndDBUpdate( );
				}

				CHARACTER_UPDATE* update = (CHARACTER_UPDATE*)perIoContext->buffer;
				if ( update->retvalue == 0 )
				{
					// 캐릭터 이벤트.
					char category = EVENT_CHARACTER_UPDATE;
					char buffer[260];

					switch ( update->flag )
					{
					case PLAYER_DBUPDATE_PROCESS:
						sprintf( buffer, "Process" );
						break;
					case PLAYER_DBUPDATE_LEVELUP:
						sprintf( buffer, "LevelUp" );
						break;
					case PLAYER_DBUPDATE_LEVELCHEAT:
						sprintf( buffer, "Cheat < LevelUp" );
						break;
					case PLAYER_DBUPDATE_SKILLUP:
						sprintf( buffer, "SkillUp" );
						break;
					case PLAYER_DBUPDATE_SKILLCHEAT:
						sprintf( buffer, "Cheat < SkillUp" );
						break;
					case PLAYER_DBUPDATE_SKILLADD:
						sprintf( buffer, "Skill < Add" );
						break;
					case PLAYER_DBUPDATE_SKILLRESET:
						sprintf( buffer, "Skill < Reset" );
						break;
					case PLAYER_DBUPDATE_POINTPLUS:
						sprintf( buffer, "Skill < PointPlus" );
						break;
					case PLAYER_DBUPDATE_LOGOUT:
						category = EVENT_CHARACTER_LOGOUT;
						sprintf( buffer, "Out" );
						break;
					case PLAYER_DBUPDATE_EXPRECOVER:
						sprintf( buffer, "Exp Recover" );
						break;
					case PLAYER_DBUPDATE_LOGIN:
						sprintf( buffer, "Map0 Login" );
						break;
					default:
						sprintf( buffer, "Unknown(%d)", update->flag );
						break;
					}
					PostCharacterEvent( category
									   ,update->cid
									   ,update->idx
									   ,update->level
									   ,update->exp
									   ,update->skillLevel
									   ,update->skillExp
									   ,update->skillPointTotal
									   ,"UpdateComplete < %s"
									   ,buffer );
				}
				else
					PostServerEvent("SQL_GAME_PROCESS_CHARACTER_UPDATE[%d] ERROR[%d]", update->idx, update->retvalue );
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterSelect )
				{
					// 캐릭터 선택 카운트 감소.
					if ( mCharacterSelectCount > 0 ) { --mCharacterSelectCount; }

					clientInfo->commonDb.characterSelect = false;
					// DB; 사용종료.

					CHARACTER_SELECT* characterSelect = (CHARACTER_SELECT*)perIoContext->buffer;
					TB_CHARACTER*     character       = &characterSelect->character;

					/*-- 응답메시지 발송.
					*/
					try {
						if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
							throw 1;

						// 선택 캐릭터 인덱스 확인.
						if ( characterSelect->characterIdx != character->idx )
						{
							PostServerEvent( "ERROR - SQL_GAME_PROCESS_CHARACTER_SELECT - WARNING CHARACTER_IDX(=%d) MEMBER_IDX=(%d)"
											 ,characterSelect->characterIdx
											 ,characterSelect->userIdx );
							throw 1;
						}

						// 070915 PKH 플레이어 map에 등록 시점 변경
						cPlayer* hero = mpObjectManager->AddPlayer( perSocketContext->cid, character->idx, character->name, (u_long)character->money, (u_long)character->deposit );
						if ( hero == NULL )
						{
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_CHARACTER_SELECT - EXIST CHARACTER_IDX(=%d)", character->idx );
							throw 1;
						}

						// PC룸 이벤트.
						//if ( clientInfo->pc ) hero->Pg_Game_Start( "", "", "", 0 );

						// GM버프 이벤트  [3/17/2010 Jo-Server]
						if ( mGMEventList.IsEmpty() == false)
						{
							hero->AddAllGMEvent();
						}

						// Select On.
						clientInfo->complete.characterSelect = true;


						// 캐릭터 이벤트.
						PostCharacterEvent( EVENT_CHARACTER_LOGIN
													 ,perSocketContext->cid
													 ,character->idx
													 ,character->level
													 ,character->exp
													 ,character->skillLevel
													 ,character->skillExp
													 ,character->skillPointTotal
													 ,"Inetnum(=%s)"
													 ,inet_ntoa( perSocketContext->addr.sin_addr ) );

						unsigned long heroIdx = hero->GetObjectID( );

						// 캐릭터 관련-DB 검색.
						clientInfo->gameInDBEnd                   = true;
						clientInfo->gameInDb.inventorySelect      = InventorySelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.inventoryCooltime    = InventoryCooltimeSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.itemBillSelect       = ItemBillSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.shortcutSelect       = ShortcutSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.questList		      = QuestSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.questEndList	      = QuestCompleteSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.questValidList	      = QuestValidSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.titleList		      = TitleSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.skillSelect          = SkillSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.skillInfluenceSelect = SkillInfluenceSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.makeSkillList        = MakeSkillSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.fortuneData		  = FortuneSelect( perSocketContext, heroIdx );
						clientInfo->gameInDb.themeUser            = ThemeUserSelect( perSocketContext, heroIdx );
                        clientInfo->gameInDb.trial                = TrialTimeSelect( perSocketContext, perSocketContext->cid );

						// GameInDbCount 증가.
						if ( clientInfo->gameInDatabase != 0 )
						{
							clientInfo->gameInDBCnt = true;
							++mGameInDbCount;
						}

						// 친구 목록 검색
						hero->SetFriendUseDB( FriendSelect( heroIdx ) );

						clientInfo->complete.skillInfluenceUpdate = true;

						// 캐릭터 옵션 [option1] 보관.
						clientInfo->optionData1 = character->option1;
						clientInfo->request.characterOptionSend = true;
						clientInfo->complete.characterOption	= true;

						// sPlayerInfo
						sPlayerInfo* playerInfo = hero->GetPlayerInfo( );

							wcscpy( playerInfo->strName, character->name );
							playerInfo->Level  = character->level;
							playerInfo->Race   = character->race;
							playerInfo->Gender = character->gender;
							playerInfo->Job    = (ePLAYER_JOB)character->job;
							playerInfo->HeadInfo[ eHEAD_HAIR ] = character->hair;
							playerInfo->HeadInfo[ eHEAD_HAIRCOLOR ] = character->hairColor;
							playerInfo->HeadInfo[ eHEAD_FACE ] = character->face;

						// sPlayerWearInfo
						sPlayerWearInfo* wearInfo = hero->GetWearInfo( );

							wearInfo->WearIdx[ eWEAR_HAT   ] = character->equipItem[ INVENTORY_WEAR_HAT   ];
							wearInfo->WearIdx[ eWEAR_BODY1 ] = character->equipItem[ INVENTORY_WEAR_BODY1 ];
							wearInfo->WearIdx[ eWEAR_BODY2 ] = character->equipItem[ INVENTORY_WEAR_BODY2 ];
							wearInfo->WearIdx[ eWEAR_HAND  ] = character->equipItem[ INVENTORY_WEAR_HAND  ];
							wearInfo->WearIdx[ eWEAR_FOOT  ] = character->equipItem[ INVENTORY_WEAR_FOOT  ];

							wearInfo->Earring  = character->equipItem[ INVENTORY_WEAR_EARRING    ];
							wearInfo->Necklace = character->equipItem[ INVENTORY_WEAR_NECKLACE   ];
							wearInfo->Brooch   = character->equipItem[ INVENTORY_WEAR_BROOCH     ];
							wearInfo->Bracelet = character->equipItem[ INVENTORY_WEAR_BRACELET   ];
							wearInfo->Ring     = character->equipItem[ INVENTORY_WEAR_RING       ];

							wearInfo->CostumeIdx[ eWEAR_HAT   ] = character->equipItem[ INVENTORY_COSTUME_HAT    ];
							wearInfo->CostumeIdx[ eWEAR_BODY1 ] = character->equipItem[ INVENTORY_COSTUME_BODY1  ];
							wearInfo->CostumeIdx[ eWEAR_BODY2 ] = character->equipItem[ INVENTORY_COSTUME_BODY2  ];
							wearInfo->CostumeIdx[ eWEAR_HAND  ] = character->equipItem[ INVENTORY_COSTUME_HAND   ];
							wearInfo->CostumeIdx[ eWEAR_FOOT  ] = character->equipItem[ INVENTORY_COSTUME_FOOT   ];

                            wearInfo->CostumeAccIdx[ eCOSTUME_HEAD ] = character->equipItem[ INVENTORY_COSTUME_FACE   ];
                            wearInfo->CostumeAccIdx[ eCOSTUME_BACK ] = character->equipItem[ INVENTORY_COSTUME_BACK   ];


						// 무기정보 - sPlayerWeaponInfo
						sPlayerWeaponInfo* weaponInfo = hero->GetWeaponInfo( );

							if ( character->activeWeapon == 0 )
							{
								weaponInfo->WeaponIdx[ eHAND_LEFT  ] = character->equipItem[ INVENTORY_HAND_LEFT2    ];
								weaponInfo->WeaponIdx[ eHAND_RIGHT ] = character->equipItem[ INVENTORY_HAND_RIGHT2   ];

								weaponInfo->WeaponEnhanced[ eHAND_LEFT  ] = character->enhancedItem[ INVENTORY_HAND_LEFT2  ];
								weaponInfo->WeaponEnhanced[ eHAND_RIGHT ] = character->enhancedItem[ INVENTORY_HAND_RIGHT2 ];
							}
							else
							{
								weaponInfo->WeaponIdx[ eHAND_LEFT  ] = character->equipItem[ INVENTORY_HAND_LEFT1    ];
								weaponInfo->WeaponIdx[ eHAND_RIGHT ] = character->equipItem[ INVENTORY_HAND_RIGHT1   ];

								weaponInfo->WeaponEnhanced[ eHAND_LEFT  ] = character->enhancedItem[ INVENTORY_HAND_LEFT1  ];
								weaponInfo->WeaponEnhanced[ eHAND_RIGHT ] = character->enhancedItem[ INVENTORY_HAND_RIGHT1 ];
							}

						// sPlayerExrInfo
						sPlayerExrInfo* exrInfo = hero->GetExrInfo();

							exrInfo->mForceType       = character->mForceType;
							exrInfo->mTitleIndex	  = character->mTitleIndex;
							exrInfo->mGuildIndex	  = character->mGuildIndex;
							
							// 초기화
							struct tm initTm;
							memset( &initTm, 0, sizeof(initTm) );
							initTm.tm_year = 1900;
							initTm.tm_mon  = 1;
							initTm.tm_mday = 1;

							exrInfo->mGuildMarkDate = initTm;
							exrInfo->mUseGuildMark	= false;

							// 길드 유효성검사
							if( exrInfo->mGuildIndex > 0 )
							{
								// 직위
								exrInfo->mGuildPosition = character->mGuildPosition;

								cGuild* guild = mpGuildManager->GetGuild( exrInfo->mGuildIndex );
								if( guild != NULL )
								{
									if( mpGuildManager->IsGuildUser( exrInfo->mGuildIndex, heroIdx ) != NULL )
									{
										// 정보 존재
										// 길드마크 등록정보 가져오기
										mpGuildManager->GetGuildMarkDate( exrInfo->mGuildIndex, exrInfo->mGuildMarkDate, exrInfo->mUseGuildMark );

										// 길드명
										wchar_t* name = guild->GetGuildName();
										if ( name != NULL )
										{
											wcscpy( exrInfo->mGuildName, name ); 
										}
										else
											PostServerEvent("FAIL - SQL_GAME_PROCESS_CHARACTER_SELECT GetGuildName [%d,%d,%d]", mChannelNum, exrInfo->mGuildIndex, heroIdx );
									}
									else
									{
										PostServerEvent("FAIL - SQL_GAME_PROCESS_CHARACTER_SELECT find user [%d,%d,%d]", mChannelNum, exrInfo->mGuildIndex, heroIdx );

										// HERO 정보 존재하지 않음 - 동기화 하기
										GuildSelect( exrInfo->mGuildIndex, true );
									}
								}
								else
								{
									PostServerEvent("FAIL - SQL_GAME_PROCESS_CHARACTER_SELECT find guild [%d,%d,%d]", mChannelNum, exrInfo->mGuildIndex, heroIdx );

									// GUILD 정보 존재하지 않음 - 동기화 하기
									GuildSelect( exrInfo->mGuildIndex, true );
								}
							}

						// sHeroInfo
						sHeroInfo* heroInfo = hero->GetHeroInfo();

							heroInfo->Exp              = character->exp;
							heroInfo->SkillLevel       = character->skillLevel;
							heroInfo->SkillPointRemain = (unsigned short)character->skillPointRemain;
							heroInfo->SkillPointTotal  = (unsigned short)character->skillPointTotal;
							heroInfo->SkillExp         = character->skillExp;
							heroInfo->TarotPoint       = character->tarotPoint;
							heroInfo->mFirePoint       = character->mFirePoint; 
							heroInfo->mFireFriendly    = character->mFireFriendly;
							heroInfo->mWaterPoint      = character->mWaterPoint;
							heroInfo->mWaterFriendly   = character->mWaterFriendly;
							heroInfo->mWindPoint       = character->mWindPoint;
							heroInfo->mWindFriendly    = character->mWindFriendly;
							heroInfo->mEarthPoint      = character->mEarthPoint;
							heroInfo->mEarthFriendly   = character->mEarthFriendly;

							/// 전문기술
							heroInfo->MakeSkill1.mIndex = character->mMakeSkill1;
							heroInfo->MakeSkill2.mIndex = character->mMakeSkill2;
							heroInfo->MakeSkill1.mStep	= character->mMakeSkillStep1;
							heroInfo->MakeSkill1.mExp	= character->mMakeSkillExp1;
							heroInfo->MakeSkill2.mStep	= character->mMakeSkillStep2;
							heroInfo->MakeSkill2.mExp	= character->mMakeSkillExp2;

                            /// 인벤토리 크기
                            heroInfo->BagEnd            = character->mBagEnd;
                            heroInfo->WearEnd           = character->mWearEnd;
                            heroInfo->WareHouseEnd      = character->mWareHouseEnd;
                            heroInfo->MakeSkillEnd      = character->mMakeSkillEnd;
							heroInfo->mLoseExp			= character->mExpRecover;

						// Default Set
						hero->SetActiveWeapon( (eItemActiveWeapon)character->activeWeapon );
						hero->SetMapNumber( character->mapNum );
						hero->SetPos( character->xPos, character->yPos );
						hero->SetOptionData( character->option1 );
						hero->SetExpRecoverEndTime( character->mExpRecoverendDate );
						
						if( mType == _E_ST_NORMAL_MAP_ )
						{
 							if( character->mapNum == 0 )		/// 생성후 튜토리얼을 거치지 않은 케릭터
							{
								sTargetPos pos = STAGESCRIPT->GetStageChangePos( CHARACTER_CREATE_POS_IDX );
								hero->SetMapNumber( pos.mMapNumber );
								hero->SetPos( pos.mPosX, pos.mPosY );
								hero->SetMapNum0Save( true );
							}
							else
							{
								// 못가는 지역에 저장되 있는 경우 위치보정
								if ( AIMANAGER->IsPossible( character->mapNum, character->xPos, character->yPos, hero->GetObject() ) == false )
								{
									NiPoint2 pos = STAGESCRIPT->CalcNearTargetMapPos( hero->GetMapNumber(), hero->GetXPos(), hero->GetYPos() );
									hero->SetPos( pos.x, pos.y );

									/// 보정 후에도 못가는 지역이면 - 실패시 캐릭터 생성위치로 이동
									if ( AIMANAGER->IsPossible( character->mapNum, pos.x, pos.y, hero->GetObject() ) == false )
									{
										sTargetPos pos = STAGESCRIPT->GetStageChangePos( CHARACTER_CREATE_POS_IDX );
										hero->SetMapNumber( pos.mMapNumber );
										hero->SetPos( pos.mPosX, pos.mPosY );
									}
								}
							}

							// 첫 게임 접속 여부
							clientInfo->complete.firstGameIn = ( !clientInfo->request.joinMap && !clientInfo->request.joinInstantDungeon );
						}

						STATUSCALC->CalcPlayerInit( hero->GetObject() );
						hero->SetJumpTime();

						// HP & MP.
						hero->InitHP( character->HP > 0 ? character->HP : 7 );
						hero->InitMP( character->MP );

						// hp/mp db에서 읽어온값 임시 장소에 저장
						hero->SetInitRestHP( );
						hero->SetInitRestMP( );

						/// 호칭정보 
						hero->InitTitle( );

						// 인던서버 이동후 맵 로딩.
						if ( clientInfo->request.joinInstantDungeon )
						{			
							char category = NM_ERROR;
							char protocol = 0;
							NiPoint2 goPos = hero->GetPos();
							unsigned short roomNum = 0;
							unsigned short mapDataNum = mInDunMapNumber;

							switch( mType )
							{
							case _E_ST_ID_PVP_:
								{
									roomNum = PVPMANAGER->JoinDeathMatch( hero, &goPos.x, &goPos.y );
									if( roomNum == 0 )
									{
										// 실패처리
										PostServerEvent( "_E_ST_ID_PVP_ roomNum[%d,%d] == 0", roomNum, mInDunMapNumber );
										throw 1;
									}

									if( roomNum == INDUN_MAP_MAX )
										clientInfo->isCheatDungeonIn = true;

									category = NM_PVP;
									protocol = NM_PVP_JOIN_SYN;

									clientInfo->complete.pvpJoin = true;
									
									hero->CalcPvPReturnGameSrv( character->mapNum, character->xPos, character->yPos );
								}
								break;
							case _E_ST_ID_THEME_:
								{
									cThemeObject* pTheme = THEMEMANAGER->ThemeJoin( hero, clientInfo->themeRoomIdx, clientInfo->themeMode, &goPos.x, &goPos.y );
									if( pTheme == NULL )
									{
										// 실패처리
										PostServerEvent( "_E_ST_ID_THEME_ pTheme[%d,%d] == NULL", clientInfo->themeRoomIdx, mInDunMapNumber );
										throw 1;
									}

									hero->SetThemeRoomIdx( clientInfo->themeRoomIdx );
									hero->SetThemeModeIdx( clientInfo->themeMode );
									roomNum = clientInfo->themeRoomIdx;

									category = NM_PLAYER;
									protocol = NM_PLAYER_MAPCHANGE_RES;

									hero->CalcThemeReturnGameSrv( character->mapNum, character->xPos, character->yPos, true );
								}
								break;
							case _E_ST_ID_TUTORIAL_:
								{									
									cThemeObject* pTheme = NULL;
									if( MAP_MIN <= character->mapNum && character->mapNum <= MAP_MAX )	/// 게임서버에서 gm툴을통해 이동해옴
									{
										category = NM_PLAYER;
										protocol = NM_PLAYER_MAPCHANGE_RES;
										pTheme = THEMEMANAGER->TutorialJoin( hero, clientInfo->themeMode, character->tutorialModeIndex, &goPos.x, &goPos.y, true );
									}
									else	/// 신규유저
									{
										category = NM_USER;
										protocol = NM_USER_CHARACTER_SELECT_RES;
										pTheme = THEMEMANAGER->TutorialJoin( hero, clientInfo->themeMode, character->tutorialModeIndex, &goPos.x, &goPos.y, false );
									}
									
									if( pTheme == NULL )
									{
										// 실패처리
										PostServerEvent( "_E_ST_ID_TUTORIAL_ pTheme[%d,%d] == NULL", clientInfo->themeRoomIdx, mInDunMapNumber );
										throw 1;
									}

									clientInfo->themeRoomIdx = pTheme->GetThemeIndex();

									hero->SetTutorialMode( character->tutorialModeIndex, false );

									hero->SetThemeRoomIdx( clientInfo->themeRoomIdx );
									hero->SetThemeModeIdx( clientInfo->themeMode );
									roomNum = clientInfo->themeRoomIdx;

									sTargetPos targetPos = STAGESCRIPT->GetStageChangePos( CHARACTER_CREATE_POS_IDX );
									hero->CalcThemeReturnGameSrv( targetPos.mMapNumber, targetPos.mPosX, targetPos.mPosY, true );
								}
								break;
							}

							if ( hero->InDunMapChange( roomNum, mapDataNum, goPos.x, goPos.y ) == false )
							{
								// 실패처리
								PostServerEvent( "hero->InDunMapChange( %d, %d, %d, %f, %f ) == false", heroIdx, roomNum, mapDataNum, goPos.x, goPos.y );
								throw 1;
							}

							// 캐릭터 맵 이동 적용.
							hero->ApplyMapTargetPos( );
							hero->SendPlayerInfo( category, protocol, (ULONG_PTR)perSocketContext, (unsigned char)mType );
							hero->SendMoney( hero->GetObject() );
							hero->SendDeposit( hero->GetObject( ) );

							// 상태 풀기
							hero->ChangeState( eOBJECT_STATE_IDLE );
							hero->SetStateStop( eSTOP_NONE );
						}
						// 맵서버 이동후 맵 로딩.
						else if ( clientInfo->request.joinMap )
						{
							hero->CheatMapChange( hero->GetMapNumber( ), hero->GetXPos( ), hero->GetYPos( ) );
							hero->ApplyMapTargetPos( );				// 캐릭터 맵 이동 적용.
							hero->SendPlayerInfo( NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, (ULONG_PTR)perSocketContext, (unsigned char)mType );
							hero->SendMoney( hero->GetObject() );
							hero->SendDeposit( hero->GetObject( ) );
						}
						else
						{
							// 캐릭터 정보 발송 - 성공.
							hero->SendPlayerInfo( NM_USER, NM_USER_CHARACTER_SELECT_RES, (ULONG_PTR)perSocketContext, (unsigned char)mType );
							hero->SendMoney( hero->GetObject( ) );
							hero->SendDeposit( hero->GetObject( ) );
						}

						if( g_BillType == 1 )
						{
							// PC 방 빌링여부 검색.
							if ( !clientInfo->complete.pgBilling )
							{
								char user_id[17];
								char user_ip[24];
								sprintf( user_id, "%d", clientInfo->memberIdx );
								sprintf( user_ip
										,"%d.%d.%d.%d"
										,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b1
										,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b2
										,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b3
										,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b4 );
								PGBILLING->GameStart( g_gameSrv->GetServerName( ), clientInfo->session, user_id, user_ip );
								// inet_ntoa( );
							}
						}
						else if( g_BillType == 2 )
						{
							// PC 방 빌링여부 검색.
							if ( !clientInfo->complete.pgBilling )
							{
								char user_ip[24];
								sprintf( user_ip
									,"%d.%d.%d.%d"
									,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b1
									,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b2
									,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b3
									,perSocketContext->addr.sin_addr.S_un.S_un_b.s_b4 );
								PGBILLPAYLATTER->UserAuthentication( inet_addr( user_ip ), clientInfo->memberIdx );
								// inet_ntoa( );
							}
						}

						// Block 적용여부 검색.
						{
							HANDLE                  handle               = NULL;
							CHARACTER_BLOCK_SELECT* characterBlockSelect = (CHARACTER_BLOCK_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT );

							characterBlockSelect->characterIdx = clientInfo->characterIdx;

							SendSQL( perSocketContext, handle, sizeof(CHARACTER_BLOCK_SELECT) );
						}
					} catch ( int error ) {
						// 캐릭터 정보 발송 - 실패.
						SendMsgError( perSocketContext, NM_USER, NM_USER_CHARACTER_SELECT_RES, error );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_MONEY:
			{
				CHARACTER_MONEY* characterMoney = (CHARACTER_MONEY*)perIoContext->buffer;
				bool             skipLog        = false;
				char             buffer[260];

				switch ( characterMoney->objectType )
				{
				case eOBJECTTYPE_MONSTER:
					if ( characterMoney->objectRaceGender != 0 )
						skipLog = true; // SKIP - sprintf( buffer, "MONSTER::%d(%d):%I64d", characterMoney->objectRaceGender, characterMoney->objectIndex, characterMoney->money );
					else
						sprintf( buffer, "ERROR MONSTER::%d:%I64d", characterMoney->objectIndex, characterMoney->money );
					break;
				case eOBJECTTYPE_HERO:
					sprintf( buffer, "HERO::%d:%I64d", characterMoney->objectIndex, characterMoney->money );
					break;
				case eOBJECTTYPE_PLAYER:
					sprintf( buffer, "PLAYER::%d:%I64d", characterMoney->objectIndex, characterMoney->money );
					break;
				case eOBJECTTYPE_NPC:
					if ( characterMoney->objectRaceGender != 0 )
						sprintf( buffer, "NPC::%d(%d):%I64d", characterMoney->objectRaceGender, characterMoney->objectIndex, characterMoney->money );
					else
						sprintf( buffer, "ERROR NPC::%d:%I64d", characterMoney->objectIndex, characterMoney->money );
					break;
				case eOBJECTTYPE_ITEM:
					sprintf( buffer, "ITEM::%d:%I64d", characterMoney->objectIndex, characterMoney->money );
					break;
				default:
					sprintf( buffer, "ERROR UNKNOWN::%d:%I64d", characterMoney->objectIndex, characterMoney->money );
					break;
				}

				if ( skipLog != true )
					PostMoneyEvent( characterMoney->idx, characterMoney->befor, characterMoney->after, buffer );
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_DEPOSIT:
			{
				CHARACTER_DEPOSIT* characterDeposit = (CHARACTER_DEPOSIT*)perIoContext->buffer;
				char               buffer[260];

				if ( characterDeposit->objectType == eOBJECTTYPE_NPC )
				{
					if ( characterDeposit->objectRaceGender != 0 )
						sprintf( buffer, "NPC::%d(%d):%I64d", characterDeposit->objectRaceGender, characterDeposit->objectIndex, characterDeposit->money );
					else
						sprintf( buffer, "ERROR NPC::%d:%I64d", characterDeposit->objectIndex, characterDeposit->money );

					PostDepositEvent( characterDeposit->idx, characterDeposit->befor, characterDeposit->after, buffer );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_TAROT_POINT:
			// Skip
			break;
		case SQL_GAME_PROCESS_CHARACTER_ACTIVE_WEPON:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterActiveWeapon )
				{
					clientInfo->commonDb.characterActiveWeapon = false;	// DB; 사용종료.
					clientInfo->complete.characterActiveWeapon = false;	// Active Weapon 업데이트 완료.
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_OPTION:
			// Skip
			break;
		case SQL_GAME_PROCESS_CHARACTER_FORCETYPE:
			{
				CHARACTER_FORCE*  characterForce = (CHARACTER_FORCE*)perIoContext->buffer;

				if( characterForce->mRetValue != 0 )
				{
					PostServerEvent("SQL_GAME_PROCESS_CHARACTER_FORCETYPE ERROR[%d][%d,%d]", characterForce->mRetValue,
						characterForce->mIdx, characterForce->mForceType );
				}

				/// 세력 세팅
				if( characterForce->mForceType > eFORCETYPE_NONE && characterForce->mForceType < eFORCETYPE_MAX )
				{
					cPlayer*	hero = mpObjectManager->GetPlayer( characterForce->mIdx );
					if( hero )
					{
						hero->SetForceType( characterForce->mForceType );
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_CHANGE_RES, ERROR_CHEAT_GM_FORCE_CHANGE_SUCCESS );	
					}
				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_FORCE_CHANGE_RES, ERROR_CHEAT_GM_FORCE_CHANGE_BADTYPE );	

			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_TTL:
			// Skip
			break;
		case SQL_GAME_PROCESS_CHARACTER_CHANNEL:
			{
				if ( perSocketContext->socket == perIoContext->socket )
				{
					CHARACTER_CHANNEL* characterChannel = (CHARACTER_CHANNEL*)perIoContext->buffer;

					HANDLE			   handle       = NULL;
					MSG_RES_CHEAT_SEARCH_CHANNEL_GM* sendMsg = (MSG_RES_CHEAT_SEARCH_CHANNEL_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_SEARCH_CHANNEL_RES_GM );
					
					sendMsg->ErrorCode   = ERROR_CHEAT_SEARCH_CHANNEL_SUCCESS; 
					sendMsg->mChannelNum = (short)characterChannel->channelNum;

					SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_SEARCH_GM) );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
						hero->CbInventoryInsert( (ULONG_PTR)perSocketContext, (INVENTORY_INSERT*)perIoContext->buffer, perIoContext->requestResult );
				}
				catch ( u_long error ) {
					PostServerEvent( "WARNING(%d) - SQL_GAME_PROCESS_INVENTORY_INSERT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock lock( &mCs );

				INVENTORY_SELECT* inventorySelect = (INVENTORY_SELECT*)perIoContext->buffer;
				TB_INVENTORY*     table           = inventorySelect->table;

				ClientInfo*    clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*       hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				TB_INVENTORY*  inventory  = NULL;
				TB_ITEM_LIMIT* itemLimit  = NULL;

				if ( clientInfo->gameInDb.inventorySelect )
				{
					clientInfo->gameInDb.inventorySelect = false; // DB; 사용종료.

					if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
					{
						// complete flag
						clientInfo->complete.inventory = true;      // 인벤토리 불러오기 완료.
						// request flag
						clientInfo->request.inventorySend   = true; // 인벤토리 발송     요청.
						clientInfo->request.characterEquip  = true; // 캐릭터   장비교채 요청.

						// 불러온 인벤토리 복사.
						for ( long i = 0; i < inventorySelect->rowCount; i++, table++ )
						{
							try {
								/*-- 인벤토리의 유효성 검사 --*/
								if ( !hero->IsInventory( table ) )
									throw IW_ERROR_INVENTORY;			// 인벤토리 오류 - 삭제.

								if ( !ITEMMANAGER->IsItemDefineIndex( table->itemDefineIdx ) )
									throw IW_ERROR_ITEM_DEFINE_INDEX;	// 인벤토리 오류 - 삭제.

								inventory = hero->SelectInventory( table->number );
								if ( hero->IsInventory( inventory ) )
									throw IW_ERROR_DUPLICATE_NUMBER;	// 인벤토리 중복 - 삭제.

								if ( mType != _E_ST_ID_TUTORIAL_ )
								{
									itemLimit = mpItemManager->GetItemLimit( table->itemDefineIdx );
									if ( itemLimit != NULL )
									{
										if ( itemLimit->serverType == ITEM_LIMIT_TUTORIAL )
											throw IW_ERROR_TUTORIAL;	// 튜토리얼 전용 - 삭제.
									}
								}

								/*-- 인벤토리 복사 --*/
								hero->XCopyInventory( table );
							}
							catch ( INVENTORY_WARNING warning ) {
								InventoryDelete( perSocketContext, warning, clientInfo->characterIdx, table );
							}
						}
					}
					else
					{
						PostServerEvent( "ERROR - SQL_GAME_PROCESS_INVENTORY_SELECT - CHARACTER_IDX(=%d)", clientInfo->characterIdx );
						Close( perSocketContext ); // 강제종료.
					}
				}
			}
			break;
        case SQL_GAME_PROCESS_INVENTORY_CASH_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryCashSelect( (ULONG_PTR)perSocketContext, (INVENTORY_SELECT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "WARNING((u_long)%d) - SQL_GAME_PROCESS_INVENTORY_CASH_SELECT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
            }
            break;
		case SQL_GAME_PROCESS_INVENTORY_DELETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				INVENTORY_DELETE* inventoryDelete = (INVENTORY_DELETE*)perIoContext->buffer;
				if ( !perIoContext->requestResult )
				{
					TB_INVENTORY* table = &inventoryDelete->table;
					char          message[260];
					switch ( inventoryDelete->warning )
					{
					case IW_ERROR_DUPLICATE_NUMBER:
						PostServerEvent( "WARNING(DUPLICATE) - SQL_GAME_PROCESS_INVENTORY_DELETE - CHARACTER_IDX(%u):INVENTORY_IDX(=%d):INVENTORY_NUMBER(=%d)"
										,inventoryDelete->characterIdx
										,table->idx
										,table->number );
						sprintf( message, "DELETE - DUPLICATE NUMBER" );
						break;
					case IW_ERROR_ITEM_DEFINE_INDEX:
						PostServerEvent( "WARNING(ERROR) - SQL_GAME_PROCESS_INVENTORY_DELETE - CHARACTER_IDX(%u):INVENTORY_IDX(=%d):ITEM_DEFINE_INDEX(=%d)"
										,inventoryDelete->characterIdx
										,table->idx
										,table->itemDefineIndex );
						sprintf( message, "DELETE - ITEM DEFINE INDEX" );
						break;
					case IW_ERROR_INVENTORY:
						PostServerEvent( "WARNING(ERROR) - SQL_GAME_PROCESS_INVENTORY_DELETE - CHARACTER_IDX(%u):INVENTORY_IDX(=%d):INVENTORY_COUNT(=%d)"
										,inventoryDelete->characterIdx
										,table->idx
										,table->count );
						sprintf( message, "DELETE - ERROR INVENTORY" );
						break;
					case IW_ERROR_TUTORIAL:
						sprintf( message, "DELETE - TUTORIAL" );
						break;
					default:
						sprintf( message, "DELETE - UNKNOW" );
						break;
					}
					PostInventoryEvent( EVENT_INVENTORY_DELETE, inventoryDelete->characterIdx, table, message );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_COOLTIME_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.inventoryCooltime )
				{
					clientInfo->gameInDb.inventoryCooltime = false;     // DB; 사용종료.
					clientInfo->complete.inventoryCooltime = true;      // 인벤토리 쿨타임 불러오기 완료.
					clientInfo->request.inventoryCooltimeSend = true;   // 인벤토리 쿨타임 발송 요청.

					if ( hero != NULL )
					{
						INVENTORY_COOLTIME_SELECT* inventoryCooltimeSelect = (INVENTORY_COOLTIME_SELECT*)perIoContext->buffer;
						TB_INVENTORY_COOLTIME*     table                   = inventoryCooltimeSelect->table;

						for ( long i = 0; i < inventoryCooltimeSelect->rowCount; i++, table++ )
						{
							hero->AddCooltime( table );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_COOLTIME_INSERT:
			// SKIP
			break;
		case SQL_GAME_PROCESS_INVENTORY_REMOVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryRemove( (ULONG_PTR)perSocketContext, (INVENTORY_REMOVE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "WARNING(%d) - SQL_GAME_PROCESS_INVENTORY_REMOVE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_REMOVE - CHARACTER_IDX(%d) - INVENTORY_IDX(%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_USE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryUse( (ULONG_PTR)perSocketContext, (INVENTORY_USE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "WARNING(%d) - SQL_GAME_PROCESS_INVENTORY_USE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_USE - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_SWAP:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventorySwap( (ULONG_PTR)perSocketContext, (INVENTORY_SWAP*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_SWAP - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_SWAP - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_MOVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryMove( (ULONG_PTR)perSocketContext, (INVENTORY_MOVE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_MOVE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_MOVE - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_EXCEPT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryExcept( (ULONG_PTR)perSocketContext, (INVENTORY_EXCEPT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_EXCEPT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_EXCEPT - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_MERGE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryMerge( (ULONG_PTR)perSocketContext, (INVENTORY_MERGE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_MERGE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_MERGE - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_DIVIDE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryDivide( (ULONG_PTR)perSocketContext, (INVENTORY_DIVIDE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_DIVIDE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_DIVIDE - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_MOVE2:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryMove2( (ULONG_PTR)perSocketContext, (INVENTORY_MOVE2*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_MOVE2 - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_MOVE2 - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_ENHANCED:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryEnhanced( (ULONG_PTR)perSocketContext, (INVENTORY_ENHANCED*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_ENHANCED - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_ENHANCED - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_DISJOINT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try{
					if ( hero != NULL )
					{
						hero->CbInventoryDisjoint( (ULONG_PTR)perSocketContext, (INVENTORY_DISJOINT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long error ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_INVENTORY_DISJOINT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_DISJOINT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_PUT_CARD:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryPutCard( (ULONG_PTR)perSocketContext, (INVENTORY_PUT_CARD*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_PUT_CARD - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_INVENTORY_PUT_CARD - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_INVENTORY_CHANGE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventoryChange( (ULONG_PTR)perSocketContext, (INVENTORY_CHANGE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long error ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_INVENTORY_CHANGE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_CHANGE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
			break;
        case SQL_GAME_PROCESS_INVENTORY_SEAL:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbInventorySeal( (ULONG_PTR)perSocketContext, (INVENTORY_SEAL*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "ERROR - SQL_GAME_PROCESS_INVENTORY_SEAL - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
				catch ( long error ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_INVENTORY_SEAL - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_INVENTORY_SEAL - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
            break;
		case SQL_GAME_PROCESS_ITEM_BILL_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.itemBillSelect )
				{
					clientInfo->gameInDb.itemBillSelect = false;

					try {
						if ( perIoContext->requestResult != 0 )
						{
							throw perIoContext;
						}
						else if ( hero != NULL )
						{
							ITEM_BILL_SELECT* select   = (ITEM_BILL_SELECT*)perIoContext->buffer;
							TB_ITEM_BILL*     itemBill = select->table;

							for ( long i = 0; i < select->rowCount; i++, itemBill++ )
							{
								hero->AddItemBill( itemBill );
							}
						}
					}
					catch ( u_long error ) {
						PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_BILL_SELECT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_BILL_REMOVE:
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						ITEM_BILL_REMOVE* remove = (ITEM_BILL_REMOVE*)perIoContext->buffer;

						hero->CbItemBillDel( (ULONG_PTR)perSocketContext, remove, perIoContext->requestResult );

						if ( remove->inventory.idx > 0 )
						{
							if ( hero->IsInventoryEquip( remove->inventory.number ) )
							{
								clientInfo->request.characterEquip  = true;
							}
							PostInventoryEvent( EVENT_INVENTORY_DELETE, clientInfo->characterIdx, &remove->inventory, "ITEM BILL" );
						}
					}
				}
				catch ( long error ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_ITEM_BILL_REMOVE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_BILL_REMOVE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_BILL_UPDATE:
			{
				ITEM_BILL_UPDATE* update = (ITEM_BILL_UPDATE*)perIoContext->buffer;

				try {
					if ( perIoContext->requestResult != 0 ) throw perIoContext->requestResult;
					if ( update->retvalue != 0 ) update->retvalue;

					PostItemBillEvent( update->characterIdx, update->inventoryIdx, update->before, update->after );
				}
				catch ( long error ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_ITEM_BILL_UPDATE - CHARACTER_IDX(%d)", error, update->characterIdx );

					cCSLock  lock( &mCs );
					cPlayer* hero = mpObjectManager->GetPlayer( update->characterIdx );
					if ( hero != NULL )
					{
						CloseCID( hero->GetConnectionIdx( ) );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_BILL_UPDATE - CHARACTER_IDX(%d)", error, update->characterIdx );

					cCSLock  lock( &mCs );
					cPlayer* hero = mpObjectManager->GetPlayer( update->characterIdx );
					if ( hero != NULL )
					{
						CloseCID( hero->GetConnectionIdx( ) );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_SELL:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemSell( (ULONG_PTR)perSocketContext, (ITEM_SELL*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_SELL - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_ITEM_SELL - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_BUY:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemBuy( (ULONG_PTR)perSocketContext, (ITEM_BUY*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_BUY - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_ITEM_BUY - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_COLLECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemCollect( (ULONG_PTR)perSocketContext, (ITEM_COLLECT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long retvalue ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_ITEM_COLLECT - CHARACTER_IDX(%d)", retvalue, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_COLLECT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
			break;
        case SQL_GAME_PROCESS_ITEM_COLLECT_CHANGE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemCollectChange( (ULONG_PTR)perSocketContext, (ITEM_COLLECT_CHANGE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long retvalue ) {
					PostServerEvent( "RETVALUE(%d) - SQL_GAME_PROCESS_ITEM_COLLECT_CHANGE - CHARACTER_IDX(%d)", retvalue, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_COLLECT_CHANGE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
            break;
		case SQL_GAME_PROCESS_ITEM_GET:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemGet( (ULONG_PTR)perSocketContext, (ITEM_GET*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_GET - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_ITEM_GET - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_GET_QUEST:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemGetQuest( (ULONG_PTR)perSocketContext, (ITEM_GET*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_GET_QUEST - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_ITEM_GET_QUEST - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
        case SQL_GAME_PROCESS_ITEM_GET_PARTY_GIVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbItemPartyGive( (ULONG_PTR)perSocketContext, (ITEM_PARTY_GIVE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_GET_PARTY_GIVE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_USE:
			{
				ITEM_USE*     itemUse = (ITEM_USE*)perIoContext->buffer;
				TB_INVENTORY* table   = itemUse->table;

				for ( long i = 0; i < itemUse->rowCount; i++, table ++ )
				{
					PostInventoryEvent( table->apply == InventoryApplyRemoved ? EVENT_INVENTORY_DELETE : EVENT_INVENTORY_UPDATE
									   ,itemUse->characterIdx
									   ,table
									   ,"USE - SKILL" );
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_EXCHANGE:
			{
				cCSLock        lock( &mCs );
				ITEM_EXCHANGE* itemExchange = (ITEM_EXCHANGE*)perIoContext->buffer;

				for ( int i0 = 0; i0 < itemExchange->rowCount; i0++ )
				{
					cPlayer* player = mpObjectManager->GetPlayer( itemExchange->table[ i0 ].characterIdx );

					// 거래금액
					if ( itemExchange->table[ i0 ].characterMoney != 0 )
					{
						PostMoneyEvent( itemExchange->table[ i0 ].characterIdx,
										itemExchange->table[ i0 ].characterBefor,
										itemExchange->table[ i0 ].characterAfter,
										"ITEM_EXCHANGE::Character(=%d):Money(=%d)",
										itemExchange->table[ i0 ].characterFrom,
										itemExchange->table[ i0 ].characterMoney );
					}

					// 거래완료.
					if ( player != NULL )
					{
						try {
							player->CbExchangeEnd( itemExchange->table[ i0 ].table, itemExchange->table[ i0 ].rowCount, perIoContext->requestResult );
						}
						catch ( u_long error ) {
							PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_EXCHANGE - CHARACTER_IDX(%d)"
											,error
											,itemExchange->table[ i0 ].characterIdx );
							CloseCID( player->GetConnectionIdx( ) );	// 강제 종료.
						}
						catch ( TB_INVENTORY* table ) {
							PostServerEvent( "ERROR - SQL_GAME_PROCESS_ITEM_EXCHANGE - CHARACTER_IDX(%d) - INVENTORY_IDX(%d)"
											,itemExchange->table[ i0 ].characterIdx
											,table->idx );
							CloseCID( player->GetConnectionIdx( ) );	// 강제 종료.
						}
					}
					else
						PostServerEvent( "CriticalError SQL_GAME_PROCESS_ITEM_EXCHANGE CharacterIDx(=%d) is NULL.", itemExchange->table[ i0 ].characterIdx );

					// Log - 거래아이템
					TB_INVENTORY* table = itemExchange->table[ i0 ].table;
					for ( int i1 = 0; i1 < itemExchange->table[ i0 ].rowCount; i1++, table++ )
					{
						PostInventoryEvent( EVENT_INVENTORY_UPDATE, itemExchange->table[ i0 ].characterIdx, table, "ITEM_EXCHANGE" );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_MIX:
			{
                cCSLock       lock( &mCs );
                ClientInfo*   clientInfo = (ClientInfo*)perSocketContext->buffer;
                unsigned long heroIdx    = clientInfo->characterIdx;
                cPlayer*      hero       = mpObjectManager->GetPlayer( heroIdx );

                if ( hero != NULL )
                {
                    ITEM_MIX*     itemMix = (ITEM_MIX*)perIoContext->buffer;
                    TB_INVENTORY* table   = itemMix->table;

                    HANDLE              handle     = NULL;
                    MSG_RES_ITEM_MIXED* sendMsg    = (MSG_RES_ITEM_MIXED*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_MIXED_RES );
                    sInventory*         sinventory = sendMsg->inventory;
                    TB_INVENTORY*       inventory;

                    sendMsg->rowCount = 0;
                    for ( long i = 0; i < itemMix->rowCount; i++, table++, sinventory++ )
                    {
                        if ( table->apply == InventoryApplyRemoved )
                        {
                            PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "ITEM_MIX" );
                        }
                        else
                        {
                            inventory = hero->SelectInventory( table->number );
							PostInventoryEvent( inventory->idx == table->idx ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE
											   ,heroIdx
											   ,table
											   ,"ITEM_MIX" );
                            hero->XCopyInventory( table );
                        }

                        Inventory2sInventory( sinventory, table );
                        sendMsg->rowCount++;
                    }

                    SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
                    hero->CbInventoryItemMix( );	/// DB사용종료

                    clientInfo->request.characterEquip = true;
                }
            }
			break;

		case SQL_GAME_PROCESS_ITEM_SPREAD_VALUE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock            lock( &mCs );
				ClientInfo*        clientInfo      = (ClientInfo*)perSocketContext->buffer;
				cPlayer*           hero            = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				ITEM_SPREAD_VALUE* itemSpreadValue = (ITEM_SPREAD_VALUE*)perIoContext->buffer;

				if ( clientInfo->commonDb.spreadValue )
				{
					clientInfo->commonDb.spreadValue = false;

					if ( hero != NULL )
					{
						HANDLE                       handle  = NULL;
						MSG_RES_TAROT_SEEKER_RESULT* sendMsg = (MSG_RES_TAROT_SEEKER_RESULT*)GetMsgRoot( &handle, perSocketContext, NM_TAROT, NM_TAROT_SEEKER_RESULT_RES );

						sendMsg->ErrorCode   = ERROR_TAROT_SEEKER_RESULT_SUCCESS;
						sendMsg->SpreadIndex = itemSpreadValue->itemDefineIndex;
						for ( long i = 0; i < 10; i++ )
						{
							sendMsg->Result[ i ].Dir            = (BYTE)(itemSpreadValue->values[ i ].dir - 1);
							sendMsg->Result[ i ].TarotCardIndex = itemSpreadValue->values[ i ].itemDefineIndex;
							sendMsg->Result[ i ].Value          = itemSpreadValue->values[ i ].spread;
						}
						sendMsg->ResultValue = itemSpreadValue->spreadValue;

						SendMsgRoot( handle, sizeof(MSG_RES_TAROT_SEEKER_RESULT) );

						hero->SetTarotResultValue( itemSpreadValue->spreadValue );
					}
				}
			}
			break;

		case SQL_GAME_PROCESS_ITEM_TAROT_RESULT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.tarotResult )
				{
					clientInfo->commonDb.tarotResult = false; // DB종료.
					if ( hero != NULL )
					{
						ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)perIoContext->buffer;
						sObject            heroObject      = hero->GetObject( );

						// influence_idx 가 '0'이 아닐경우 시킬의 효과 호출.
						if ( itemTarotResult->influence_idx1 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx1, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx1 );
						}
						if ( itemTarotResult->influence_idx2 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx2, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx2 );
						}
						if ( itemTarotResult->influence_idx3 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx3, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx3 );
						}

						// 성공메시지.
						SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SEEKER_ACCEPT_RES, ERROR_TAROT_SEEKER_ACCEPT_SUCCESS );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_TUTORIALSPREAD_VALUE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock            lock( &mCs );
				ClientInfo*        clientInfo      = (ClientInfo*)perSocketContext->buffer;
				cPlayer*           hero            = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				ITEM_SPREAD_VALUE* itemSpreadValue = (ITEM_SPREAD_VALUE*)perIoContext->buffer;

				if ( clientInfo->commonDb.spreadValue )
				{
					clientInfo->commonDb.spreadValue = false;

					if ( hero != NULL )
					{
						HANDLE								  handle  = NULL;
						MSG_RES_TAROT_TUTORIAL_SEEKER_RESULT* sendMsg = (MSG_RES_TAROT_TUTORIAL_SEEKER_RESULT*)GetMsgRoot( &handle, perSocketContext, 
																															NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_RESULT_RES );

						sendMsg->ErrorCode   = ERROR_TAROT_TUTORIAL_SEEKER_RESULT_SUCCESS;
						sendMsg->SpreadIndex = itemSpreadValue->itemDefineIndex;
						for ( long i = 0; i < 10; i++ )
						{
							sendMsg->Result[ i ].Dir            = (BYTE)(itemSpreadValue->values[ i ].dir - 1);
							sendMsg->Result[ i ].TarotCardIndex = itemSpreadValue->values[ i ].itemDefineIndex;
							sendMsg->Result[ i ].Value          = itemSpreadValue->values[ i ].spread;
						}
						sendMsg->ResultValue = itemSpreadValue->spreadValue;

						SendMsgRoot( handle, sizeof(MSG_RES_TAROT_TUTORIAL_SEEKER_RESULT) );

						hero->SetTarotResultValue( itemSpreadValue->spreadValue );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_TUTORIALTAROT_RESULT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.tarotResult )
				{
					clientInfo->commonDb.tarotResult = false; // DB종료.
					if ( hero != NULL )
					{
						ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)perIoContext->buffer;
						sObject            heroObject      = hero->GetObject( );

						// influence_idx 가 '0'이 아닐경우 시킬의 효과 호출.
						if ( itemTarotResult->influence_idx1 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx1, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx1 );
						}
						if ( itemTarotResult->influence_idx2 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx2, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx2 );
						}
						if ( itemTarotResult->influence_idx3 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx3, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx3 );
						}

						// 성공메시지.
						SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_TUTORIAL_SEEKER_ACCEPT_RES, ERROR_TAROT_TUTORIAL_SEEKER_ACCEPT_SUCCESS );
					}
				}
			}
			break;

		case SQL_GAME_PROCESS_ITEM_SELF_SPREAD_VALUE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock            lock( &mCs );
				ClientInfo*        clientInfo      = (ClientInfo*)perSocketContext->buffer;
				cPlayer*           hero            = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				ITEM_SPREAD_VALUE* itemSpreadValue = (ITEM_SPREAD_VALUE*)perIoContext->buffer;

				if ( clientInfo->commonDb.spreadValue )
				{
					clientInfo->commonDb.spreadValue = false;

					if ( hero != NULL )
					{
						HANDLE								  handle  = NULL;
						MSG_RES_TAROT_SELF_RESULT* sendMsg = (MSG_RES_TAROT_SELF_RESULT*)GetMsgRoot( &handle, 
                                                                                                     perSocketContext,
                                                                                                     NM_TAROT, 
                                                                                                     NM_TAROT_SELF_RESULT_RES );

						sendMsg->ErrorCode   = ERROR_TAROT_SELF_RESULT_SUCCESS;
						sendMsg->SpreadIndex = itemSpreadValue->itemDefineIndex;
						for ( long i = 0; i < 10; i++ )
						{
							sendMsg->Result[ i ].Dir            = (BYTE)(itemSpreadValue->values[ i ].dir - 1);
							sendMsg->Result[ i ].TarotCardIndex = itemSpreadValue->values[ i ].itemDefineIndex;
							sendMsg->Result[ i ].Value          = itemSpreadValue->values[ i ].spread;
						}
						sendMsg->ResultValue = itemSpreadValue->spreadValue;

						SendMsgRoot( handle, sizeof(MSG_RES_TAROT_SELF_RESULT) );

						hero->SetTarotResultValue( itemSpreadValue->spreadValue );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_SELF_TAROT_RESULT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.tarotResult )
				{
					clientInfo->commonDb.tarotResult = false; // DB종료.
					if ( hero != NULL )
					{
						ITEM_TAROT_RESULT* itemTarotResult = (ITEM_TAROT_RESULT*)perIoContext->buffer;
						sObject            heroObject      = hero->GetObject( );

						// influence_idx 가 '0'이 아닐경우 시킬의 효과 호출.
						if ( itemTarotResult->influence_idx1 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx1, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx1 );
						}
						if ( itemTarotResult->influence_idx2 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx2, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx2 );
						}
						if ( itemTarotResult->influence_idx3 > 0 )
						{
							bool retvalue = mpSkillManager->AddInfluence( heroObject, heroObject, itemTarotResult->influence_idx3, 0, true );
							if ( retvalue == false )
								PostServerEvent( "Error(%d): mpSkillManager->AddItemInfluence", itemTarotResult->influence_idx3 );
						}

						// 성공메시지.
						SendMsgError( perSocketContext, NM_TAROT, NM_TAROT_SELF_ACCEPT_RES, ERROR_TAROT_SELF_ACCEPT_SUCCESS );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_STALL_SELL_GET:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbStallSellGet( (ULONG_PTR)perSocketContext, (STALL_SELL_GET*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "WARNING(%d) - SQL_GAME_PROCESS_STALL_SELL_GET - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_STALL_SELL_GET - CHARACTER_IDX(%d) - INVENTORY_IDX(%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;

		case SQL_GAME_PROCESS_ITEM_AGENT_CHECK:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock           lock( &mCs );
				ITEM_AGENT_CHECK* check  = (ITEM_AGENT_CHECK*)perIoContext->buffer;
				cPlayer*          player = mpObjectManager->GetPlayer( check->characterIdx );

				if ( player != NULL )
					player->CbItemAgentCheck( check );
			}
			break;

		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_OWNER:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				clientInfo->commonDb.itemAgentSearch = false;

				ITEM_AGENT_SELECT_OWNER* select = (ITEM_AGENT_SELECT_OWNER*)perIoContext->buffer;
				ITEM_AGENT_RESULT_OWNER* table  = select->table;

				HANDLE                       handle  = NULL;
				MSG_RES_NPC_ITEM_AGENT_OPEN* sendMsg = (MSG_RES_NPC_ITEM_AGENT_OPEN*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_OPEN_PAGE_RES );
				NPC_ITEM_AGENT_OPEN_RESULT*  result  = sendMsg->Result;

					sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_OPEN_PAGE_SUCCESS;
					sendMsg->StartPage = select->startPage;
					sendMsg->EndPage   = select->endPage;
					sendMsg->RowCount  = select->rowCount;

					for ( long i = 0; i < select->rowCount; i++, table++, result++ )
					{
						result->Idx       = table->idx;
						result->ValidThru = table->validThru;
						result->Price     = table->price;
						Inventory2sInventory( &result->Inventory, &table->table );
					}

				SendMsgRoot( handle, sendMsg->GetMsgLength() );
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemAgentAdd( (ULONG_PTR)perSocketContext, (ITEM_AGENT_INSERT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long retvalue ) {
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_ADD_RES, retvalue );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_AGENT_INSERT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_DELETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemAgentDel( (ULONG_PTR)perSocketContext, (ITEM_AGENT_DELETE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long error ) {
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_DEL_RES, error );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_AGENT_DELETE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemAgentGet( (ULONG_PTR)perSocketContext, (ITEM_AGENT_UPDATE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( long error ) {
					SendMsgError( perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_GET_RES, error );
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_AGENT_UPDATE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				clientInfo->commonDb.itemAgentSearch = false;

				ITEM_AGENT_SELECT* select = (ITEM_AGENT_SELECT*)perIoContext->buffer;
				ITEM_AGENT_RESULT* table  = select->table;

				HANDLE                                handle  = NULL;
				MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT* sendMsg = (MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES );
				NPC_ITEM_AGENT_SEARCH_RESULT*         result  = sendMsg->Result;

					sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS;
					sendMsg->StartPage = select->startPage;
					sendMsg->EndPage   = select->endPage;
					sendMsg->RowCount  = select->rowCount;

					for ( long i = 0; i < select->rowCount; i++, table++, result++ )
					{
						result->Idx = table->idx;
						wcscpy_s( result->CharacterName, table->characterName );
						result->ValidThru = table->validThru;
						result->Price = table->price;
						Inventory2sInventory( &result->Inventory, &table->table );
					}

				SendMsgRoot( handle, sendMsg->GetMsgLength() );
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_NAME:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				clientInfo->commonDb.itemAgentSearch = false;

				ITEM_AGENT_SELECT_NAME* select = (ITEM_AGENT_SELECT_NAME*)perIoContext->buffer;
				ITEM_AGENT_RESULT*      table  = select->table;

				HANDLE                                handle  = NULL;
				MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT* sendMsg = (MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES );
				NPC_ITEM_AGENT_SEARCH_RESULT*         result  = sendMsg->Result;

					sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS;
					sendMsg->StartPage = select->startPage;
					sendMsg->EndPage   = select->endPage;
					sendMsg->RowCount  = select->rowCount;

					for ( long i = 0; i < select->rowCount; i++, table++, result++ )
					{
						result->Idx = table->idx;
						wcscpy_s( result->CharacterName, table->characterName );
						result->ValidThru = table->validThru;
						result->Price = table->price;
						Inventory2sInventory( &result->Inventory, &table->table );
					}

				SendMsgRoot( handle, sendMsg->GetMsgLength() );
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_TYPE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				clientInfo->commonDb.itemAgentSearch = false;

				ITEM_AGENT_SELECT_TYPE* select = (ITEM_AGENT_SELECT_TYPE*)perIoContext->buffer;
				ITEM_AGENT_RESULT*      table  = select->table;

				HANDLE                                handle  = NULL;
				MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT* sendMsg = (MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES );
				NPC_ITEM_AGENT_SEARCH_RESULT*         result  = sendMsg->Result;

					sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS;
					sendMsg->StartPage = select->startPage;
					sendMsg->EndPage   = select->endPage;
					sendMsg->RowCount  = select->rowCount;

					for ( long i = 0; i < select->rowCount; i++, table++, result++ )
					{
						result->Idx = table->idx;
						wcscpy_s( result->CharacterName, table->characterName );
						result->ValidThru = table->validThru;
						result->Price = table->price;
						Inventory2sInventory( &result->Inventory, &table->table );
					}

				SendMsgRoot( handle, sendMsg->GetMsgLength() );
			}
			break;
		case SQL_GAME_PROCESS_ITEM_AGENT_SELECT_ALL:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				clientInfo->commonDb.itemAgentSearch = false;

				ITEM_AGENT_SELECT_ALL* select = (ITEM_AGENT_SELECT_ALL*)perIoContext->buffer;
				ITEM_AGENT_RESULT*     table  = select->table;

				HANDLE     handle  = NULL;
				MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT* sendMsg = (MSG_RES_NPC_ITEM_AGENT_SEARCH_RESULT*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_ITEM_AGENT_SEARCH_RESULT_RES );
				NPC_ITEM_AGENT_SEARCH_RESULT*         result  = sendMsg->Result;

					sendMsg->ErrorCode = ERROR_NPC_ITEM_AGENT_SEARCH_SUCCESS;
					sendMsg->StartPage = select->startPage;
					sendMsg->EndPage   = select->endPage;
					sendMsg->RowCount  = select->rowCount;

					for ( long i = 0; i < select->rowCount; i++, table++, result++ )
					{
						result->Idx = table->idx;
						wcscpy_s( result->CharacterName, table->characterName );
						result->ValidThru = table->validThru;
						result->Price = table->price;
						Inventory2sInventory( &result->Inventory, &table->table );
					}

				SendMsgRoot( handle, sendMsg->GetMsgLength() );
			}
			break;
		case SQL_GAME_PROCESS_ITEM_OBT_EVENT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try
				{
					if ( hero != NULL )
					{
						hero->CbItemObtEvent( (ULONG_PTR)perSocketContext, (ITEM_OBT_EVENT*)perIoContext->buffer, perIoContext->requestResult );
						
						ITEM_OBT_EVENT* obtEvent = (ITEM_OBT_EVENT*)perIoContext->buffer;
						clientInfo->cbtUser		 = obtEvent->cbtUser;
						clientInfo->obtUser		 = obtEvent->obtUser;
						clientInfo->fourGamer	 = obtEvent->fourGamer;
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_OBT_EVENT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;

        case SQL_GAME_PROCESS_ITEM_SUMMON_POST:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemSummonPost( (ULONG_PTR)perSocketContext, (ITEM_SUMMON_POST*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_SUMMON_POST - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
            break;
        case SQL_GAME_PROCESS_ITEM_SUMMON_AGENT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemSummonAgent( (ULONG_PTR)perSocketContext, (ITEM_SUMMON_AGENT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_SUMMON_AGENT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
            break;
        case SQL_GAME_PROCESS_ITEM_SUMMON_WAREHOUSE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemSummonWareHouse( (ULONG_PTR)perSocketContext, (ITEM_SUMMON_WAREHOUSE*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_SUMMON_WAREHOUSE - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
            break;

		case SQL_GAME_PROCESS_ITEM_NPC_EVENT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				try {
					if ( hero != NULL )
					{
						hero->CbItemNpcEvent( (ULONG_PTR)perSocketContext, (ITEM_NPC_EVENT*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_ITEM_NPC_EVENT - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );	// 강제 종료.
				}
			}
			break;

		case SQL_GAME_PROCESS_POST_CHECK:
			{
				cCSLock     lock( &mCs );
				POST_CHECK* check  = (POST_CHECK*)perIoContext->buffer;
				cPlayer*    player = mpObjectManager->GetPlayer( check->characterIdx );
				if ( player != NULL )
				{
					player->CbPostCheck( check );
				}
			}
			break;

		case SQL_GAME_PROCESS_POST_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
                if ( clientInfo )
                {
                    // 우편 리스트 전송
                    POST_SELECT* select = (POST_SELECT*)perIoContext->buffer;
                    POST_RESULT* table = select->table;

                    HANDLE                  handle  = NULL;
                    MSG_RES_CHAT_POST_OPEN* sendMsg = (MSG_RES_CHAT_POST_OPEN*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_POST_OPEN_RES );
                    POST_OPEN_RESULT*       result  = sendMsg->Result;

                    sendMsg->ErrorCode = ERROR_CHAT_POST_OPEN_SUCCESS;
                    sendMsg->startPage = select->startPage;
                    sendMsg->endPage   = select->endPage;
                    sendMsg->noneRead  = select->noneRead;
                    sendMsg->RowCount  = select->rowCount;
                    for ( long i = 0; i < select->rowCount; i++, table++, result++ )
                    {
                        result->idx	= table->idx;

                        result->registerDate.wYear   = table->registerDate.year;
                        result->registerDate.wMonth  = table->registerDate.month;
                        result->registerDate.wDay    = table->registerDate.day;
                        result->registerDate.wHour   = table->registerDate.hour;
                        result->registerDate.wMinute = table->registerDate.minute;
                        result->registerDate.wSecond = table->registerDate.second;

                        result->validThru = table->validThru;
                        result->type      = table->type;
                        result->status    = table->status;

                        wcscpy( result->fromName, table->fromName );

                        switch ( result->type )
                        {
                        case POST_TYPE_NORMAL:
                            wcscpy( result->content.title, table->contenTtitle );
                            wcscpy( result->content.message, table->contentMessage );
                            break;
                        case POST_TYPE_AGENT_CANCEL:
                            break;
                        case POST_TYPE_AGENT_SALE:
                            result->receipt.price        = table->receiptPrice;
                            result->receipt.salesCharges = table->receiptSalesCharges;
                            Inventory2sInventory( &result->receipt.inventory, &table->receiptInventory );
                            result->receipt.inventory.count = table->receiptInventoryCount;
                            break;
                        case POST_TYPE_AGENT_PURCHASE:
                            Inventory2sInventory( &result->receipt.inventory, &table->receiptInventory );
                            result->receipt.inventory.count = table->receiptInventoryCount;
                            break;
                        }

                        Inventory2sInventory( &result->inventory1, &table->inventory1 );
                        Inventory2sInventory( &result->inventory2, &table->inventory2 );
                        Inventory2sInventory( &result->inventory3, &table->inventory3 );
                        result->money = table->money;
                    }
                    SendMsgRoot( handle, sendMsg->GetMsgLength() );
                }
            }
			break;
		case SQL_GAME_PROCESS_POST_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				if ( hero != NULL )
				{
					hero->CbPostWrite( (ULONG_PTR)perSocketContext, (POST_INSERT*)perIoContext->buffer );
				}
			}
			break;
		case SQL_GAME_PROCESS_POST_UPDATE_STATUS:
			{
				cCSLock             lock( &mCs );
				POST_UPDATE_STATUS* status = (POST_UPDATE_STATUS*)perIoContext->buffer;
				cPlayer*            player = mpObjectManager->GetPlayer( status->characterIdx );
				if ( player != NULL )
				{
					player->CbPostRead( status );
				}
			}
			break;
		case SQL_GAME_PROCESS_POST_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				if ( hero != NULL )
				{
					hero->CbPostDownload( (ULONG_PTR)perSocketContext, (POST_UPDATE*)perIoContext->buffer );
				}
			}
			break;
		case SQL_GAME_PROCESS_POST_DELETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				if ( clientInfo )
				{
					POST_DELETE* del   = (POST_DELETE*)perIoContext->buffer;
					POST_RESULT* table = del->table;

					if ( del->retvalue == 0 )
					{
						HANDLE                    handle  = NULL;
						MSG_RES_CHAT_POST_DELETE* sendMsg = (MSG_RES_CHAT_POST_DELETE*)GetMsgRoot( &handle, perSocketContext, NM_CHAT, NM_CHAT_POST_DELETE_RES );
						POST_OPEN_RESULT*         result  = sendMsg->Result;

						sendMsg->ErrorCode = ERROR_CHAT_POST_DELETE_SUCCESS;
						sendMsg->startPage = del->startPage;
						sendMsg->endPage   = del->endPage;
						sendMsg->noneRead  = del->noneRead;
						sendMsg->RowCount  = (BYTE)del->rowCount;
						for ( long i = 0; i < del->rowCount; i++, table++, result++ )
						{
							result->idx	= table->idx;

							result->registerDate.wYear   = table->registerDate.year;
							result->registerDate.wMonth  = table->registerDate.month;
							result->registerDate.wDay    = table->registerDate.day;
							result->registerDate.wHour   = table->registerDate.hour;
							result->registerDate.wMinute = table->registerDate.minute;
							result->registerDate.wSecond = table->registerDate.second;

							result->validThru = table->validThru;
							result->type      = table->type;
							result->status    = table->status;

							wcscpy( result->fromName, table->fromName );

							switch ( result->type )
							{
							case POST_TYPE_NORMAL:
								wcscpy( result->content.title, table->contenTtitle );
								wcscpy( result->content.message, table->contentMessage );
								break;
							case POST_TYPE_AGENT_CANCEL:
								break;
							case POST_TYPE_AGENT_SALE:
								result->receipt.price        = table->receiptPrice;
								result->receipt.salesCharges = table->receiptSalesCharges;
								Inventory2sInventory( &result->receipt.inventory, &table->receiptInventory );
								result->receipt.inventory.count = table->receiptInventoryCount;
								break;
							case POST_TYPE_AGENT_PURCHASE:
								break;
							}

							Inventory2sInventory( &result->inventory1, &table->inventory1 );
							Inventory2sInventory( &result->inventory2, &table->inventory2 );
							Inventory2sInventory( &result->inventory3, &table->inventory3 );
							result->money = table->money;
						}
						SendMsgRoot( handle, sendMsg->GetMsgLength() );
					}
				}
			}
			break;

		case SQL_GAME_PROCESS_FRIEND_SELECT:
			{
				cCSLock        lock( &mCs );
				FRIEND_SELECT* friendSelect = (FRIEND_SELECT*)perIoContext->buffer;
				cPlayer*       player       = mpObjectManager->GetPlayer( friendSelect->characterIdx );

				if ( player != NULL && player->GetFriendUseDB( ) )
				{
					TB_FRIEND* friends = NULL;
					TB_FRIEND* table   = NULL;

					player->ClearFriends( );
					friends = player->SelectFriend( );
					table   = friendSelect->table;

					// 기본정보.
					for ( long i = 0; i < friendSelect->rowCount; i++, friends++, table++ )
					{
						(*friends) = (*table);
					}

					player->UpdateFriend( );
					player->SetFriendUseDB( false );
					player->SetFriendSend( true );
				}
			}
			break;
		case SQL_GAME_PROCESS_FRIEND_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.friendInsert )
				{
					clientInfo->commonDb.friendInsert = false; // DB; 사용종료.
					if ( hero != NULL )
					{
						FRIEND_INSERT* friendInsert = (FRIEND_INSERT*)perIoContext->buffer;

						SendMsgError( perSocketContext, NM_CHAT, NM_CHAT_FRIEND_ADD_RES, friendInsert->retvalue );

						if ( friendInsert->retvalue == 0 )
						{
							// 자신의 친구리스트 갱신
							hero->SetFriendUseDB( FriendSelect( hero->GetObjectID( ) ) );

							// 친구의 친구리스트 갱신
							cPlayer* player = mpObjectManager->GetPlayer( friendInsert->characterIdx2 );
							if ( player != NULL )
							{
								player->SetFriendUseDB( FriendSelect( friendInsert->characterIdx2 ) );
							}
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_FRIEND_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.friendUpdate )
				{
					clientInfo->commonDb.friendUpdate = false; // DB; 사용종료.
					if ( hero != NULL )
					{
						FRIEND_UPDATE* friendUpdate = (FRIEND_UPDATE*)perIoContext->buffer;

						// 자신의 친구리스트 갱신
						hero->SetFriendUseDB( FriendSelect( hero->GetObjectID( ) ) );

						// 친구의 친구리스트 갱신
						cPlayer* player = mpObjectManager->GetPlayer( friendUpdate->characterIdx2 );
						if ( player != NULL )
						{
							player->SetFriendUseDB( FriendSelect( friendUpdate->characterIdx2 ) );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_FRIEND_DELETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->commonDb.friendDelete )
				{
					clientInfo->commonDb.friendDelete = false; // DB; 사용종료.
					if ( hero != NULL )
					{
						FRIEND_DELETE* friendDelete = (FRIEND_DELETE*)perIoContext->buffer;

						// 자신의 친구리스트 갱신
						hero->SetFriendUseDB( FriendSelect( hero->GetObjectID( ) ) );

						// 친구의 친구리스트 갱신
						cPlayer* player = mpObjectManager->GetPlayer( friendDelete->characterIdx2 );
						if ( player != NULL )
						{
							player->SetFriendUseDB( FriendSelect( friendDelete->characterIdx2 ) );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_SKILL_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.skillSelect )
				{
					clientInfo->gameInDb.skillSelect = false; // DB 사용종료.
					if ( hero != NULL )
					{
						SKILL_SELECT* skillSelect = (SKILL_SELECT*)perIoContext->buffer;

						if ( SKILLMANAGER->RestorePlayerSkill( skillSelect ) )
						{
							clientInfo->complete.skillRelease = true;
							// 070828 PKH  스킬 정보 전송
							SKILLMANAGER->SendMsgSkillHaveToPlayer( perSocketContext->cid, hero->GetObjectID( ) );
						}
						else
						{
							PostServerEvent( "END TASK - SQL_GAME_PROCESS_SKILL_SELECT - CHARACTER_IDX(=%d)", clientInfo->characterIdx );
							CloseCID( hero->GetConnectionIdx( ) );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_SKILL_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.skillDbUse = false;	// DB; 사용종료.
				if ( hero != NULL )
				{
					SKILL_INSERT*  skillInsert = (SKILL_INSERT*)perIoContext->buffer;
					HANDLE         handle      = NULL;
					if( skillInsert->mRetvalue == 0 )
					{
						MSG_RES_SKILL_ADD* sendMsg = (MSG_RES_SKILL_ADD*)GetMsgRoot( &handle, perSocketContext, NM_SKILL, NM_SKILL_ADD_SUC_RES );
						sendMsg->mSkillClassIdx = skillInsert->mSkillClassIdx;
						SendMsgRoot( handle, sizeof(MSG_RES_SKILL_ADD) );

						// 추가된 스킬이 패시브 스킬이면 스테이터스 재계산함
						sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillInsert->mSkillClassIdx );
						if( pInfo != NULL && pInfo->mType == eSKILLTYPE_PASSIVE )
							STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );

						/// sp 정리 / 클라이언트에 전송
						hero->UpdateJobUsedSP();
						hero->SendSkillPoint();

						PostSkillEvent( hero->GetObjectID(), false, skillInsert->mSkillClassIdx, 1 );
					}
					else
					{
						SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_DBERROR );

						/// sp,money 복구 / 습득스킬 제거
						hero->SkillPointMinusRestore( skillInsert->mSP );

						SKILLMANAGER->RollBackPlayerHaveSkill( hero->GetObjectID(), skillInsert->mSkillClassIdx );

						PostServerEvent( "SQL_GAME_PROCESS_SKILL_INSERT ERROR[%d][%d,%d,%d]", skillInsert->mRetvalue,
							skillInsert->mCharacterIdx, skillInsert->mSkillClassIdx, skillInsert->mSP );
					}

					// 캐릭터 이벤트.
					hero->PostCharacterEvent( EVENT_CHARACTER_UPDATE, "Skill - Insert : SkillClassIdx=%d", skillInsert->mSkillClassIdx );
					hero->EndDBUpdate();
				}
			}
			break;
		case SQL_GAME_PROCESS_SKILL_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.skillDbUse = false;	// DB 사용종료.
				if ( hero != NULL )
				{
					SKILL_UPDATE*  skillUpdate = (SKILL_UPDATE*)perIoContext->buffer;
					HANDLE         handle      = NULL;

					if( skillUpdate->mRetvalue == 0 )
					{
						MSG_RES_SKILL_ADD* sendMsg = (MSG_RES_SKILL_ADD*)GetMsgRoot( &handle, perSocketContext, NM_SKILL, NM_SKILL_ADD_SUC_RES );

						if ( sendMsg != NULL )
						{
							sendMsg->mSkillClassIdx = skillUpdate->mSkillClassIdx;
							sendMsg->mSkillStep = skillUpdate->mSkillStep;

							SendMsgRoot( handle, sizeof(MSG_RES_SKILL_ADD) );

							// 변경된 스킬이 패시브 스킬이면 스테이터스 재계산함
							sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillUpdate->mSkillClassIdx );
							if( pInfo != NULL && pInfo->mType == eSKILLTYPE_PASSIVE )
								STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );
						}

						/// sp 정리 / 클라이언트에 전송
						hero->UpdateJobUsedSP();
						hero->SendSkillPoint();

						PostSkillEvent( hero->GetObjectID(), false, skillUpdate->mSkillClassIdx, skillUpdate->mSkillStep + 1 );
					}				
					else
					{
						SendMsgError( perSocketContext, NM_SKILL, NM_SKILL_ADD_ERR_RES, SKILL_ADD_ERR_DBERROR );

						/// sp,money 복구 / 습득스킬 제거
						hero->SkillPointMinusRestore( skillUpdate->mSP );

						/*sObject player = { hero->GetObjectType(), hero->GetObjectID() };
						if( hero->AddMoney( player, skillUpdate->mMoney ) != (long)skillUpdate->mMoney )
							PostServerEvent( "SQL_GAME_PROCESS_SKILL_UPDATE[%d] AddMoney[%d]", hero->GetObjectID(), skillUpdate->mMoney );*/

						SKILLMANAGER->RollBackPlayerHaveSkill( hero->GetObjectID(), skillUpdate->mSkillClassIdx );

						/// 에러 기록
						PostServerEvent( "SQL_GAME_PROCESS_SKILL_UPDATE ERROR[%d][%d,%d,%d,%d]", skillUpdate->mRetvalue,
							skillUpdate->mCharacterIdx, skillUpdate->mSkillClassIdx, skillUpdate->mSkillStep, skillUpdate->mSP );
					}

					// 캐릭터 이벤트.
					hero->PostCharacterEvent( EVENT_CHARACTER_UPDATE, "Skill - Update : SkillClassIdx=%d", skillUpdate->mSkillClassIdx );
					hero->EndDBUpdate();
				}
			}
			break;
		case SQL_GAME_PROCESS_SKILL_CHEAT_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.skillDbUse = false;	// DB; 사용종료.
				if ( hero != NULL )
				{
					SKILL_CHEAT_INSERT*  skillInsert = (SKILL_CHEAT_INSERT*)perIoContext->buffer;
					HANDLE         handle      = NULL;
					if( skillInsert->mRetvalue == 0 )
					{
						MSG_RES_SKILL_ADD* sendMsg = (MSG_RES_SKILL_ADD*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_RES_GM );
						
						if( sendMsg != NULL)
						{
							sendMsg->mSkillClassIdx = skillInsert->mSkillClassIdx;
							sendMsg->mSkillStep     = skillInsert->mSkillStep;
							SendMsgRoot( handle, sizeof(MSG_RES_SKILL_ADD) );

							// 추가된 스킬이 패시브 스킬이면 스테이터스 재계산함
							sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillInsert->mSkillClassIdx );
							if( pInfo != NULL && pInfo->mType == eSKILLTYPE_PASSIVE )
								STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );
						}

						PostSkillEvent( hero->GetObjectID(), false, skillInsert->mSkillClassIdx, skillInsert->mSkillStep + 1 );
					}
					else
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_ERR_RES, ERROR_CHEAT_SKILL_ADD_DBERROR );
						
						SKILLMANAGER->RollBackPlayerHaveSkill( hero->GetObjectID(), skillInsert->mSkillClassIdx );

						PostServerEvent( "SQL_GAME_PROCESS_SKILL_CHEAT_INSERT ERROR[%d][%d,%d]", skillInsert->mRetvalue, skillInsert->mCharacterIdx, skillInsert->mSkillClassIdx );
					}

					// 캐릭터 이벤트.
					hero->PostCharacterEvent( EVENT_CHARACTER_UPDATE, "Skill - Insert(Cheat) : SkillClassIdx=%d", skillInsert->mSkillClassIdx );
					hero->EndDBUpdate();
				}
			}
			break;
		case SQL_GAME_PROCESS_SKILL_CHEAT_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.skillDbUse = false;	// DB 사용종료.
				if ( hero != NULL )
				{
					SKILL_CHEAT_UPDATE*  skillUpdate = (SKILL_CHEAT_UPDATE*)perIoContext->buffer;
					HANDLE         handle      = NULL;

					if( skillUpdate->mRetvalue == 0 )
					{
						MSG_RES_SKILL_ADD* sendMsg = (MSG_RES_SKILL_ADD*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_RES_GM );

						if ( sendMsg != NULL )
						{
							sendMsg->mSkillClassIdx = skillUpdate->mSkillClassIdx;
							sendMsg->mSkillStep     = skillUpdate->mSkillStep;

							SendMsgRoot( handle, sizeof(MSG_RES_SKILL_ADD) );

							// 변경된 스킬이 패시브 스킬이면 스테이터스 재계산함
							sPlayerSkillBaseInfo* pInfo = SKILLSCRIPT->GetPlayerSkillInfo( skillUpdate->mSkillClassIdx );
							if( pInfo != NULL && pInfo->mType == eSKILLTYPE_PASSIVE )
								STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );
						}

						PostSkillEvent( hero->GetObjectID(), false, skillUpdate->mSkillClassIdx, skillUpdate->mSkillStep + 1 );
					}				
					else
					{
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_SKILL_GET_ERR_RES, ERROR_CHEAT_SKILL_ADD_DBERROR );
						
						SKILLMANAGER->RollBackPlayerHaveSkill( hero->GetObjectID(), skillUpdate->mSkillClassIdx );

						/// 에러 기록
						PostServerEvent( "SQL_GAME_PROCESS_SKILL_CHEAT_UPDATE ERROR[%d][%d,%d,%d]", skillUpdate->mRetvalue,
							skillUpdate->mCharacterIdx, skillUpdate->mSkillClassIdx, skillUpdate->mSkillStep );
					}

					// 캐릭터 이벤트.
					hero->PostCharacterEvent( EVENT_CHARACTER_UPDATE, "Skill - Update(Cheat) : SkillClassIdx=%d", skillUpdate->mSkillClassIdx );
					hero->EndDBUpdate();
				}
			}
			break;
		case SQL_GAME_PROCESS_SKILL_COOLTIME:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock        lock( &mCs );
				ClientInfo*    clientInfo   = (ClientInfo*)perSocketContext->buffer;
				SKILL_COOLTIME* skillCoolTime = (SKILL_COOLTIME*)perIoContext->buffer;

				if( skillCoolTime->mRetvalue != 0 )	
				{ 
					PostServerEvent( "SKILL COOLTIME ERROR - DB[%d]", skillCoolTime->mRetvalue );
				}

				clientInfo->complete.skillCoolTime = false;
			}
			break;
		case SQL_GAME_PROCESS_INFLUENCE_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.skillInfluenceSelect )
				{
					clientInfo->gameInDb.skillInfluenceSelect = false;	// DB 사용종료.
					clientInfo->complete.influenceRelease = true;

					if ( hero != NULL )
					{
						SKILL_INFLUENCE_SELECT* influenceSelect = (SKILL_INFLUENCE_SELECT*)perIoContext->buffer;
						if ( influenceSelect->mRowCount > 0 )
							SKILLMANAGER->RestorePlayerInfluence( influenceSelect->mCharacterIdx, influenceSelect, influenceSelect->mRowCount );

						// 효과 리스트 발송
						SKILLMANAGER->SendMsgInfluenceToPlayer( perSocketContext->cid, hero->GetObjectID( ) );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_INFLUENCE_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock        lock( &mCs );
				ClientInfo*    clientInfo   = (ClientInfo*)perSocketContext->buffer;
				cPlayer*       hero = OBJECTMANAGER->GetPlayer( clientInfo->characterIdx );
				SKILL_INFLUENCE_UPDATE* influenceUpdate = (SKILL_INFLUENCE_UPDATE*)perIoContext->buffer;

				if ( influenceUpdate->mRetvalue == 0 )	
				{ 
					TB_CHARACTER_INFLUENCE* pTBInf = influenceUpdate->mTable;
					for( long i = 0 ; i < influenceUpdate->mRowCount ; ++i )
					{
						if( pTBInf->mDel == 0 )
						{
							cInfluenceObject* pInf = SKILLMANAGER->GetInfluence( pTBInf->mUniqueIdx );
							if( pInf != NULL )
								pInf->SetAllocDbIdx( pTBInf->mDbIdx );
							else
							{
								PostServerEvent("SQL_GAME_PROCESS_INFLUENCE_UPDATE pInf != NULL[%d,%d]", influenceUpdate->mCharacterIdx,
									pTBInf->mInfluenceClassIdx );
							}
						}
					}
				}
				else
				{
					PostServerEvent( "SQL_GAME_PROCESS_INFLUENCE_UPDATE influenceUpdate->mRetvalue == 0[%d,%d]", 
						influenceUpdate->mCharacterIdx, influenceUpdate->mRetvalue );

					if( influenceUpdate->mRetvalue != 10 )
						Close( perSocketContext );
				}

				if( hero != NULL )
				{
					if( influenceUpdate->mCycleSave == true )
						hero->SetCashInfSaveCycle( false );
					else
						hero->SetCashInfSave( false );
				}				
			}
			break;
		case SQL_GAME_PROCESS_SHORTCUT_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*	hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.shortcutSelect )
				{
					clientInfo->gameInDb.shortcutSelect = false;	// DB 사용종료.
					clientInfo->complete.shortcut = true;
					clientInfo->request.shortcutSend = true;		// 단축창 발송 요청.

					if ( hero != NULL )
					{
						SHORTCUT_SELECT* shortcutSelect = (SHORTCUT_SELECT*)perIoContext->buffer;

						/// 캐릭터 첫 생성시는 데이터가 없다
						if( shortcutSelect->rowCount == 0 )
						{
							// 직업 구분 가져오기
							tArray<unsigned long>* playerJobParent = STATUSSCRIPT->GetParentJob( hero->GetJob() );
							unsigned long firstJob = hero->GetJob();
							if( playerJobParent != NULL )
								firstJob = (*playerJobParent)[0];

							/// 퀵슬롯 기본 스킬로 등록 세팅
							sShortcut* data = SHORTCUTSCRIPT->GetShortCut( firstJob );
							if( data != NULL )
								memcpy( clientInfo->shortcut.binary, data, sizeof(sShortcut) * MAX_SHORTCUT_SIZE );
						}
						else
						{
							memcpy( &clientInfo->shortcut, &shortcutSelect->table, sizeof(TB_SHORTCUT) );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_SHORTCUT_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				clientInfo->commonDb.shortcutUpdate = false;	// DB 사용종료.
				clientInfo->complete.shortcut = false;
			}
			break;
		case SQL_GAME_PROCESS_QUEST_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				/// 퀘스트 신규등록결과 RES
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				/// 이미 소유하고 있는 퀘스트를 절대 또 소유할수 없다!
				if ( clientInfo->questDb.questInsert )
				{
					clientInfo->questDb.questInsert = false;
					if ( hero != NULL )
					{
						QUEST_INSERT* questInsert = (QUEST_INSERT*)perIoContext->buffer;

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if( questInsert->retvalue == 0 )
							{
								unsigned long      heroIdx = hero->GetObjectID();
								TB_QUEST_PROGRESS* table   = &questInsert->quest;

								/// 1. 아이템 제공
								if ( questInsert->rowCount > 0 )
								{
									HANDLE               handle     = NULL;
									MSG_RES_ITEM_REWARD* sendMsg    = (MSG_RES_ITEM_REWARD*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_REWARD_RES );
									sInventory*          sinventory = sendMsg->inventory;

									TB_INVENTORY*        table      = questInsert->inventory;
									TB_INVENTORY*        inventory;
									bool                 newOrUpdate;

									for ( long i = 0; i < questInsert->rowCount; i++, table++ )
									{
										inventory   = hero->SelectInventory( table->number );
										newOrUpdate = (inventory->idx == table->idx);
										inventory   = hero->XCopyInventory( inventory, table ); // (*inventory) = (*table);

										Inventory2sInventory( sinventory, inventory );
										sinventory++;

										sendMsg->rowCount++;

										PostInventoryEvent( newOrUpdate ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, heroIdx, inventory, "QUEST INSERT" );
									}

									SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
								}

								/// 등록 결과 send
								/// 2. hero 의 퀘스트 목록 갱신 및 추가된 내용만 발송
								HANDLE			   handle  = NULL;
								MSG_RES_QUEST_ADD* sendMsg = (MSG_RES_QUEST_ADD*)GetMsgRoot( &handle, perSocketContext, NM_NPC, NM_NPC_QUEST_ADD_RES );
								if( sendMsg != NULL )
								{
									sendMsg->ErrorCode = questInsert->retvalue;

									unsigned long questIdx = 0;
									int arrIdx = hero->FindEmptyQuest();

									if( arrIdx > -1 )
									{
										TB_QUEST_PROGRESS* heroQuest = hero->GetQuest( arrIdx );
										(*heroQuest) = (*table);

										/// 시작 시간체크 저장
										cQuestDefine* define = mpQuestManager->GetQuestDefine( heroQuest->questIdx );
										if( define && define->mQuestLimit && define->mQuestLimit->mTimeType != eTIME_NONE )
										{
											heroQuest->startTime = GetAccumTime();
										}

										/// 갱신된 목록 send 
										sendMsg->table.arrIdx	   = arrIdx;
										sendMsg->table.questIdx	   = heroQuest->questIdx;
										sendMsg->table.check	   = heroQuest->check;
										sendMsg->table.restTime	   = heroQuest->restTime;
										sendMsg->table.status	   = heroQuest->status;

										questIdx = heroQuest->questIdx;

										PostQuestEvent( EVENT_QUEST_ADD, hero->GetObjectID(), heroQuest->questIdx, "ADD" );
									}
									else
									{
										PostQuestEvent( EVENT_QUEST_ADD, hero->GetObjectID(), table->questIdx, "FAIL : quest list full" );
									}

									/// 결과 보내기
									SendMsgRoot( handle, sizeof(MSG_RES_QUEST_ADD) );

									/// 신규퀘스트 목록 보내기
									hero->SendNewQuestList();

									/// 제약 사항 걸기 ( Limit )
									hero->SendLimitPenalty( questIdx );
								}

								/// 금액 정산
								if( questInsert->takemoney != 0 )
								{
									unsigned long befor = hero->GetMoney();
									long result = hero->AddMoney( hero->GetObject(), questInsert->takemoney, false	);

									PostMoneyEvent( hero->GetObjectID(), befor, hero->GetMoney(), "QUEST_INSERT::Money(=%d)", questInsert->takemoney );

									if ( result != questInsert->takemoney )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERT AddMoney(=%d) != ResultMoney(=%d).", questInsert->takemoney, result );
								}

								if( questInsert->firepoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->firepoint, eFORCETYPE_FIRE );
									if ( result != questInsert->firepoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERT AddFirePoint(=%d) != ResultFirePoint(=%d).", questInsert->firepoint, result );
								}

								if( questInsert->waterpoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->waterpoint, eFORCETYPE_WATER );
									if ( result != questInsert->waterpoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERT AddWaterPoint(=%d) != ResultWaterPoint(=%d).", questInsert->waterpoint, result );
								}

								if( questInsert->windpoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->windpoint, eFORCETYPE_WIND );
									if ( result != questInsert->windpoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERT AddWindPoint(=%d) != ResultWindPoint(=%d).", questInsert->windpoint, result );
								}

								if( questInsert->earthpoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->earthpoint, eFORCETYPE_EARTH );
									if ( result != questInsert->earthpoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERT AddEarthPoint(=%d) != ResultEarthPoint(=%d).", questInsert->earthpoint, result );
								}

								// 튜토리얼 변경 보내주기
								if( questInsert->tutorialindex > 0 )
								{
									hero->SetTutorialMode( questInsert->tutorialindex );
								}

								// 기본 수집 아이템 체크
								hero->UpdateDutyItem();
							}
							else
							{
								/// 에러결과 보내주기
								SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_ADD_RES, questInsert->retvalue );
								PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERT(%d,%d,%d)", hero->GetObjectID(), questInsert->questIdx, questInsert->retvalue );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_QUEST_INSERT - CHARACTER_IDX(%u)", clientInfo->characterIdx );
							Close( perSocketContext );
						}

						hero->CbInventoryReward( ); // DB; 사용종료.
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_INSERTBYITEM:
			if( perSocketContext->socket == perIoContext->socket )
			{
				/// 의뢰아이템으로 인한 퀘스트 처리
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				/// 이미 소유하고 있는 퀘스트를 절대 또 소유할수 없다!
				if ( clientInfo->questDb.questInsert )
				{
					clientInfo->questDb.questInsert = false;
					if ( hero != NULL )
					{
						QUEST_INSERT*	questInsert = (QUEST_INSERT*)perIoContext->buffer;

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if( questInsert->retvalue == 0 )
							{
								unsigned long      heroIdx = hero->GetObjectID();
								TB_QUEST_PROGRESS* table   = &questInsert->quest;

								/// 2. 아이템 제공
								if ( questInsert->rowCount > 0 )
								{
									HANDLE               handle     = NULL;
									MSG_RES_ITEM_REWARD* sendMsg    = (MSG_RES_ITEM_REWARD*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_REWARD_RES );
									sInventory*          sinventory = sendMsg->inventory;

									TB_INVENTORY*        table      = questInsert->inventory;
									TB_INVENTORY*        inventory;

									for ( long i = 0; i < questInsert->rowCount; i++, table++ )
									{
										if ( table->apply == InventoryApplyRemoved )
										{
											PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "QUEST INSERT BY ITEM" );
											table->count = 0;	// 클라이언트 삭제 요청를 위해 0으로 설정.
										}
										else
										{
											inventory = hero->SelectInventory( table->number );

											if (inventory->idx == table->idx)
												PostInventoryEvent( EVENT_INVENTORY_UPDATE, heroIdx, table, "QUEST INSERT BY ITEM" );
											else
												PostInventoryEvent( EVENT_INVENTORY_CREATE, heroIdx, table, "QUEST INSERT BY ITEM" );

											inventory = hero->XCopyInventory( inventory, table );
										}

										Inventory2sInventory( sinventory, table );
										sinventory++;

										sendMsg->rowCount++;
									}

									SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
								}

								/// 등록 결과 send
								/// 3. hero 의 퀘스트 목록 갱신 및 추가된 내용만 발송
								HANDLE                   handle  = NULL;
								MSG_RES_QUEST_ADDBYITEM* sendMsg = (MSG_RES_QUEST_ADDBYITEM*)GetMsgRoot( &handle, perSocketContext, NM_QUEST, NM_QUEST_ADDBYITEM_RES );
								if( sendMsg != NULL )
								{
									sendMsg->ErrorCode = questInsert->retvalue;

									unsigned long questIdx = 0;
									int arrIdx = hero->FindEmptyQuest();

									if ( arrIdx > -1 )
									{
										TB_QUEST_PROGRESS* heroQuest = hero->GetQuest( arrIdx );
										(*heroQuest) = (*table);

										/// 갱신된 목록 send 
										sendMsg->table.arrIdx	   = arrIdx;
										sendMsg->table.questIdx	   = heroQuest->questIdx;
										sendMsg->table.check	   = heroQuest->check;
										sendMsg->table.restTime	   = heroQuest->restTime;
										sendMsg->table.status	   = heroQuest->status;

										questIdx = heroQuest->questIdx;

										PostQuestEvent( EVENT_QUEST_ADD, hero->GetObjectID(), heroQuest->questIdx, "ADD" );
									}
									else
									{
										PostQuestEvent( EVENT_QUEST_ADD, hero->GetObjectID(), table->questIdx, "FAIL : quest list full" );
									}

									/// 결과 보내기
									SendMsgRoot( handle, sizeof(MSG_RES_QUEST_ADDBYITEM) );

									// 신규 퀘스트
									hero->SendNewQuestList();

									/// 3. 그외 처리 하기
									/// 제약 사항 걸기 ( Limit )
									hero->SendLimitPenalty( questIdx );
								}

								/// 금액 정산
								if( questInsert->takemoney != 0 )
								{
									unsigned long befor = hero->GetMoney();
									long result = hero->AddMoney( hero->GetObject(), questInsert->takemoney, false	);

									PostMoneyEvent( hero->GetObjectID(), befor, hero->GetMoney(), "QUEST_INSERTBYITEM::Money(=%d)", questInsert->takemoney );

									if ( result != questInsert->takemoney )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERTBYITEM AddMoney(=%d) != ResultMoney(=%d).", questInsert->takemoney, result );
								}

								if( questInsert->firepoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->firepoint, eFORCETYPE_FIRE );
									if ( result != questInsert->firepoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERTBYITEM AddFirePoint(=%d) != ResultFirePoint(=%d).", questInsert->firepoint, result );
								}

								if( questInsert->waterpoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->waterpoint, eFORCETYPE_WATER );
									if ( result != questInsert->waterpoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERTBYITEM AddWaterPoint(=%d) != ResultWaterPoint(=%d).", questInsert->waterpoint, result );
								}

								if( questInsert->windpoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->windpoint, eFORCETYPE_WIND );
									if ( result != questInsert->windpoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERTBYITEM AddWindPoint(=%d) != ResultWindPoint(=%d).", questInsert->windpoint, result );
								}

								if( questInsert->earthpoint != 0 )
								{
									long result = hero->AddPvPPoint( questInsert->earthpoint, eFORCETYPE_EARTH );
									if ( result != questInsert->earthpoint )
										PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERTBYITEM AddEarthPoint(=%d) != ResultEarthPoint(=%d).", questInsert->earthpoint, result );
								}

								// 튜토리얼 변경 보내주기
								if( questInsert->tutorialindex > 0 )
								{
									hero->SetTutorialMode( questInsert->tutorialindex );
								}

								// 기본 수집 아이템 체크
								hero->UpdateDutyItem();
							}
							else
							{
								SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_ADDBYITEM_RES, questInsert->retvalue );
								PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_INSERTBYITEM(%d,%d,%d)", hero->GetObjectID(), questInsert->questIdx, questInsert->retvalue );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_QUEST_INSERTBYITEM - CHARACTER_IDX(%u)", clientInfo->characterIdx );
							Close( perSocketContext );
						}

						hero->CbInventoryReward( );	// DB; 사용종료.
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_SELECT:
 			if( perSocketContext->socket == perIoContext->socket )
			{	
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				/// 게임접속시에만 플래그를 세움. 즉 전체리스트는 일정경우에만 보낸다.
				if( clientInfo->gameInDb.questList )
				{
					// request flag 
					clientInfo->gameInDb.questList = false;							/// DB 사용종료
					clientInfo->request.questSend = true;							/// 퀘스트 리스트 발송요청.
					clientInfo->complete.quest = true;								/// complete flag - 퀘스트 리스트 가져오기 완료.
					clientInfo->delay.questSave = GetTickCount( ) + QUEST_SAVETIME;	/// DB 저장 지연

					if ( hero != NULL )
					{
						QUEST_SELECT*		questSelect = (QUEST_SELECT*)perIoContext->buffer;
						TB_QUEST_PROGRESS*	table		= questSelect->table;
						TB_QUEST_PROGRESS*  heroQuest	= NULL;

						unsigned long count = 0;
						hero->ClearQuestList( );
						for( long i = 0; i < questSelect->rowCount; i++, table++ )
						{
							cQuestDefine* define = mpQuestManager->GetQuestDefine( table->questIdx );
							if( define == NULL )
							{
								QuestRemove( perSocketContext, table->idx );
								continue;
							}

							// 튜토리얼 서버가 아닌데, 튜토리얼형 퀘스트면 강제삭제
							if( mType != _E_ST_ID_TUTORIAL_ )
							{
								if( define->mType == eQUEST_TYPE_TUTORIAL )
								{
									QuestRemove( perSocketContext, table->idx );
									continue;
								}
							}

							if( mpQuestManager->IsValid( define ) == false )
							{
								// 유효하지 않은 퀘스트 - 강제삭제 후 로그 처리함.
								QuestRemove( perSocketContext, table->idx );
							}
							else
							{
								if( count < MAX_KEEPQUEST )
								{
									/// player 퀘스트 목록 복사
									heroQuest = hero->GetQuest( count );
									(*heroQuest) = (*table);

									count++;
									/// 시작 시간체크 저장
									if( define && define->mQuestLimit && define->mQuestLimit->mTimeType != eTIME_NONE )
									{
										heroQuest->startTime = GetAccumTime();
									}
								}
								else
								{
									PostServerEvent( "ERROR - SQL_GAME_PROCESS_QUEST_SELECT - CHARACTER_IDX(%u,%d)", clientInfo->characterIdx, questSelect->rowCount );
								}
							}
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_UPDATE:
			break;
		case SQL_GAME_PROCESS_QUEST_DELETE:
			/// 해당 리퀘스트를 보낸 소켓이 맞는지 검사
			if( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				
				if( clientInfo->questDb.questDelete )
				{
					clientInfo->questDb.questDelete = false;	/// 삭제 관련 db 사용 완료
					if ( hero != NULL )
					{
						QUEST_DELETE* questDelete = (QUEST_DELETE*)perIoContext->buffer;
						unsigned long heroIdx     = hero->GetObjectID();

						TB_QUEST_PROGRESS* delQuest  = hero->GetQuest(questDelete->arrIdx);
						unsigned long     questIndex = delQuest->questIdx;

						if( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							/// 삭제 성공시
							if( questDelete->retvalue == 0 )
							{
								PostQuestEvent( EVENT_QUEST_DELETE, hero->GetObjectID(), questIndex, "DELETE" );

								/// 서버메모리로부터 삭제
								hero->DeleteQuest( questDelete->arrIdx );

								/// 패널티 처리
								hero->SendFailPenalty( questIndex );
								hero->SendEndPenalty( questIndex );

								/// 아이템 회수
								if ( questDelete->rowCount > 0 )
								{
									HANDLE             handle     = NULL;
									MSG_RES_ITEM_TAKE* sendMsg    = (MSG_RES_ITEM_TAKE*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_TAKE_RES );
									sInventory*        sinventory = sendMsg->inventory;

									TB_INVENTORY*      table      = questDelete->table;
									TB_INVENTORY*      inventory;

									for ( long i = 0; i < questDelete->rowCount; i++, table++ )
									{
										inventory = hero->XCopyInventory( table ); // (*inventory) = (*table);

										Inventory2sInventory( sinventory, inventory );
										sinventory++;

										sendMsg->rowCount++;

										if ( table->apply == InventoryApplyRemoved )
										{
											PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "QUEST TAKE" );
											hero->RemoveInventory( inventory );
										}
										else
										{
											PostInventoryEvent( EVENT_INVENTORY_UPDATE, heroIdx, table, "QUEST TAKE" );
										}
									}

									SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
								}

								SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_DEL_RES, ERROR_QUEST_DEL_SUCCESS );

								/// 퀘스트 리스트 재요청
								clientInfo->complete.quest = true;
								clientInfo->request.questSend = true;
							}
							/// 실패시
							else
							{
								SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_DEL_RES, questDelete->retvalue );

								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL SQL_GAME_PROCESS_QUEST_DELETE [status : %d], [check : %ld]", delQuest->status, delQuest->check );
								PostQuestEvent( EVENT_QUEST_DELETE, hero->GetObjectID(), delQuest->questIdx, buffer );
							}
						}
						else
						{
							char	buffer[1024]   = "\0";
							sprintf( buffer, "FAIL SQL_GAME_PROCESS_QUEST_DELETE [status : %d], [check : %ld]", delQuest->status, delQuest->check );
							PostQuestEvent( EVENT_QUEST_DELETE, hero->GetObjectID(), delQuest->questIdx, buffer );

							/// 삭제 오류시, 클라이언트 강제 종료.
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_QUEST_DELETE - CHARACTER_IDX(%u)", clientInfo->characterIdx );
							Close( perSocketContext );
						}

						hero->CbInventoryTake( ); // DB; 사용 완료.
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_DELETEAUTO:
			if( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->questDb.questDeleteAuto )
				{				
					clientInfo->questDb.questDeleteAuto = false;	/// 삭제 관련 db 사용 완료
					if ( hero != NULL )
					{
						QUEST_DELETE* questDelete  = (QUEST_DELETE*)perIoContext->buffer;
						unsigned long heroIdx      = hero->GetObjectID();

						TB_QUEST_PROGRESS* delQuest   = hero->GetQuest(questDelete->arrIdx);
						unsigned long      questIndex = delQuest->questIdx;

						if ( questDelete->retvalue == 0 && perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							PostQuestEvent( EVENT_QUEST_DELETE, hero->GetObjectID(), questIndex, "DELETEAUTO" );

							/// 서버메모리로부터 삭제
							hero->DeleteQuest( questDelete->arrIdx );

							/// 패널티 처리
							hero->SendFailPenalty( questIndex );
							hero->SendEndPenalty( questIndex );

							/// 아이템 회수
							if ( questDelete->rowCount > 0 )
							{
								HANDLE             handle     = NULL;
								MSG_RES_ITEM_TAKE* sendMsg    = (MSG_RES_ITEM_TAKE*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_TAKE_RES );
								sInventory*        sinventory = sendMsg->inventory;

								TB_INVENTORY*      table      = questDelete->table;
								TB_INVENTORY*      inventory;

								for ( long i = 0; i < questDelete->rowCount; i++, table++ )
								{
									inventory = hero->XCopyInventory( table ); // (*inventory) = (*table);

									Inventory2sInventory( sinventory, inventory );
									sinventory++;

									sendMsg->rowCount++;

									if ( table->apply == InventoryApplyRemoved )
									{
										PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "QUEST TAKE" );
										hero->RemoveInventory( inventory );
									}
									else
									{
										PostInventoryEvent( EVENT_INVENTORY_UPDATE, heroIdx, table, "QUEST TAKE" );
									}
								}

								SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
							}

							SendMsgError( perSocketContext, NM_QUEST, NM_QUEST_DELAUTO_RES, questDelete->retvalue );

							/// 퀘스트 리스트 재요청
							clientInfo->complete.quest = true;
							clientInfo->request.questSend = true;
						}
						else
						{
							char	buffer[1024]   = "\0";
							sprintf( buffer, "FAIL SQL_GAME_PROCESS_QUEST_DELETEAUTO [status : %d], [check : %ld]", delQuest->status, delQuest->check );
							PostQuestEvent( EVENT_QUEST_DELETE, hero->GetObjectID(), delQuest->questIdx, buffer );

							// 삭제 오류시, 클라이언트 강제 종료.
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_QUEST_DELETEAUTO - CHARACTER_IDX(%u)", clientInfo->characterIdx );
							Close( perSocketContext );
						}

						hero->CbInventoryTake( ); // DB; 사용 완료.
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_COMPLETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				/// 퀘스트 진행 완료 요청 결과
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->questDb.questComplete )
				{
					clientInfo->questDb.questComplete = false;	/// 진행완료 관련 db사용 완료
					if ( hero != NULL )
					{
						/// DB 저장정보
						QUEST_COMPLETE*		questComplete = (QUEST_COMPLETE*)perIoContext->buffer;
						unsigned long		heroIdx		  = hero->GetObjectID();

						/// 서버 저장정보
						TB_QUEST_PROGRESS*	completeQuest = hero->GetQuest( questComplete->arrIdx );

						if( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							/// 완료 성공시
							if( questComplete->retvalue == 0 )
							{
								/// 정보 저장
								completeQuest->status = questComplete->status;

								/// 
								hero->SendEndPenalty( completeQuest->questIdx );

								if ( questComplete->status == eQUEST_COMPLETE )
									PostQuestEvent( EVENT_QUEST_COMPLETE, heroIdx, completeQuest->questIdx, "COMPLETE" );
								else
									PostQuestEvent( EVENT_QUEST_COMPLETE, heroIdx, completeQuest->questIdx, "ERROR" );

								/// 아이템 회수
								if ( questComplete->rowCount > 0 )
								{
									HANDLE             handle     = NULL;
									MSG_RES_ITEM_TAKE* sendMsg    = (MSG_RES_ITEM_TAKE*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_TAKE_RES );
									sInventory*        sinventory = sendMsg->inventory;

									TB_INVENTORY*      table      = questComplete->table;
									TB_INVENTORY*      inventory;

									for ( long i = 0; i < questComplete->rowCount; i++, table++ )
									{
										inventory = hero->XCopyInventory( table ); // (*inventory) = (*table);

										Inventory2sInventory( sinventory, inventory );
										sinventory++;

										sendMsg->rowCount++;

										if ( table->apply == InventoryApplyRemoved )
										{
											PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "QUEST TAKE" );
											hero->RemoveInventory( inventory ); // memset( inventory, 0, sizeof(TB_INVENTORY) );
										}
										else
										{
											PostInventoryEvent( EVENT_INVENTORY_UPDATE, heroIdx, table, "QUEST TAKE" );
										}
									}

									SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
								}

								// 튜토리얼 변경 보내주기
								if( questComplete->tutorialIndex > 0 )
								{
									hero->SetTutorialMode( questComplete->tutorialIndex );
								}

								/// 진행결과 보내기
								HANDLE			   handle  = NULL;
								MSG_RES_NPC_QUEST_COMPLETE* sendMsg = (MSG_RES_NPC_QUEST_COMPLETE*)GetMsgRoot( &handle, perSocketContext );
								sendMsg->Category = NM_NPC;
								sendMsg->Protocol = NM_NPC_QUEST_COMPLETE_RES;
								sendMsg->ErrorCode	= questComplete->retvalue;
								sendMsg->arrayIdx	= questComplete->arrIdx;
								sendMsg->status		= questComplete->status;
								SendMsgRoot( handle, sizeof(MSG_RES_NPC_QUEST_COMPLETE) );
							}
							/// 실패시
							else
							{
								SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_COMPLETE_RES, questComplete->retvalue );

								PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_COMPLETE(%d,%d)", hero->GetObjectID(), questComplete->retvalue );

								/// 로그 남기기
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL SQL_GAME_PROCESS_QUEST_COMPLETE [status : %d], [check : %ld]", questComplete->status, questComplete->check );
								PostQuestEvent( EVENT_QUEST_COMPLETE, heroIdx, completeQuest->questIdx, buffer );
							}
						}
						else
						{
							/// 로그 남기기
							char	buffer[1024]   = "\0";
							sprintf( buffer, "FAIL SQL_GAME_PROCESS_QUEST_COMPLETE [status : %d], [check : %ld]", questComplete->status, questComplete->check );
							PostQuestEvent( EVENT_QUEST_COMPLETE, heroIdx, completeQuest->questIdx, buffer );

							// 삭제 오류시, 클라이언트 강제 종료.
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_QUEST_COMPLETE - CHARACTER_IDX(%u)", clientInfo->characterIdx );
							Close( perSocketContext );
						}

						hero->CbInventoryReward( ); // DB; 사용 완료.
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_REWARD:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				/// 퀘스트 완료 요청 결과
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*        hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->questDb.questReward )
				{
					clientInfo->questDb.questReward = false;/// 보상요청 관련 db사용 완료
					if ( hero != NULL )
					{
						QUEST_REWARD*      questReward  = (QUEST_REWARD*)perIoContext->buffer;
						unsigned long      heroIdx		= hero->GetObjectID();

						TB_QUEST_PROGRESS* rewardQuest  = hero->GetQuest( questReward->arrIdx );

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							/// 성공시 처리
							unsigned long questIndex = rewardQuest->questIdx;

							PostQuestEvent( EVENT_QUEST_REWARD, heroIdx, questIndex, "REWARD" );

							/// 기본보상 처리
							hero->SendDefaultReward( questIndex, questReward->skillClassIdx, questReward->skillRowCount );

							/// 선택보상중 버프 처리
							for ( long i = 0; i < questReward->bufRowCount; ++i )
								hero->SendRewardBuff( questReward->bufIndex[i] );

							/// 서버메모리로부터 삭제
							hero->DeleteQuest( questReward->arrIdx );

							/// 완료된 퀘스트 목록에 추가
							hero->AddCompleteQuest( questIndex );

							/// 만기일 저장
							if( hero->AddValidQuest( questIndex, questReward->thru ) == false )
							{
								PostServerEvent( "Error(%d, %d): QUEST mValidQuestMap.Insert - already exist questIdx", heroIdx, questIndex );
							}

							if ( questReward->characterMoney > 0 )
							{
								unsigned long befor  = hero->GetMoney( );
								long          result = hero->AddMoney( hero->GetObject( ), questReward->characterMoney, false );
								if ( result == questReward->characterMoney )
								{
									PostMoneyEvent( questReward->characterIdx, befor, hero->GetMoney( ), "QUEST_REWARD::QuestReward(=%d):Money(=%d)", questReward->idx, questReward->characterMoney );
								}
								else
								{
									PostServerEvent( "Error - SQL_GAME_PROCESS_QUEST_REWARD AddMoney(=%d) != ResultMoney(=%d).", questReward->characterMoney, result );
								}
							}
							if ( questReward->itemRowCount > 0 )
							{
								HANDLE               handle     = NULL;
								MSG_RES_ITEM_REWARD* sendMsg    = (MSG_RES_ITEM_REWARD*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_REWARD_RES );
								sInventory*          sinventory = sendMsg->inventory;

								TB_INVENTORY*        table      = questReward->table;
								TB_INVENTORY*        inventory;
								bool                 newOrUpdate;

								for ( long i = 0; i < questReward->itemRowCount; i++, table++ )
								{
									inventory   = hero->SelectInventory( table->number );
									newOrUpdate = (inventory->idx == table->idx);
									inventory   = hero->XCopyInventory( table ); // (*inventory) = (*table);

									Inventory2sInventory( sinventory, inventory );
									sinventory++;

									sendMsg->rowCount++;

									PostInventoryEvent( newOrUpdate ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, heroIdx, inventory, "QUEST REWARD" );
								}

								SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
							}

							SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_REWARD_RES, ERROR_QUEST_REWARD_SUCCESS );

							clientInfo->complete.quest = true;
							clientInfo->request.questSend = true;
						}
						else
						{
							char buffer[1024];
							sprintf( buffer, "FAIL SQL_GAME_PROCESS_QUEST_REWARD [status : %d], [check : %ld]", rewardQuest->status, rewardQuest->check );
							PostQuestEvent( EVENT_QUEST_REWARD, heroIdx, rewardQuest->questIdx, buffer );

							SendMsgError( perSocketContext, NM_NPC, NM_NPC_QUEST_REWARD_RES, ERROR_QUEST_REWARD_FAIL );

							// 보상 오류시, 클라이언트 강제 종료.
							PostServerEvent( "WARNING - SQL_GAME_PROCESS_QUEST_REWARD - CHARACTER_IDX(%u)", clientInfo->characterIdx );
							Close( perSocketContext );
						}
						hero->CbInventoryReward( ); // DB; 사용 완료.
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_ENDLISTSELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if( clientInfo->gameInDb.questEndList )
				{
					clientInfo->gameInDb.questEndList = false; /// DB 사용종료
					if ( hero != NULL )
					{
						QUEST_ENDLIST* questEndList = (QUEST_ENDLIST*)perIoContext->buffer;
						unsigned long* questIdx     = questEndList->questIdx;

						/// 완료목록과 완료그룹목록 초기화
						hero->ClearCompleteList();
						for( long i = 0; i < questEndList->rowCount; i++, questIdx++ )
						{
							hero->AddCompleteQuest( (*questIdx) ); /// 완료목록 저장 
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_VALIDLISTSELECT:
			if( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if( clientInfo->gameInDb.questValidList )
				{
					clientInfo->gameInDb.questValidList = false; /// DB 사용종료
					if ( hero != NULL )
					{
						QUEST_VALIDLIST* questValidList = (QUEST_VALIDLIST*)perIoContext->buffer;

						bool ret = true;
						hero->ClearValidQuestList();
						for ( long i = 0; i < questValidList->rowCount; i++ )
						{
							ret = hero->AddValidQuest( questValidList->table[i].questIdx, questValidList->table[i].valid );
						}

						if( ret == false )
						{
							PostServerEvent( "Error(%d): QUEST mValidQuestMap - already exist questIdx", clientInfo->characterIdx );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_QUEST_REMOVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock			lock( &mCs );

				QUEST_REMOVE*	questRemove = (QUEST_REMOVE*)perIoContext->buffer;
				ClientInfo*		clientInfo	= (ClientInfo*)perSocketContext->buffer;

				if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
				{
					PostQuestEvent( EVENT_QUEST_REMOVE, clientInfo->characterIdx, questRemove->idx, "REMOVE" );
				}
				else
				{
					PostServerEvent( "Warning! Synchronization between DB and the server error - SQL_GAME_PROCESS_QUEST_REMOVE" );
				}
			}
			break;
		case SQL_GAME_PROCESS_TITLE_INSERT:
			if( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock       lock( &mCs );
				TITLE_INSERT* titleInsert = (TITLE_INSERT*)perIoContext->buffer;
				ClientInfo*   clientInfo  = (ClientInfo*)perSocketContext->buffer;
				cPlayer*      hero        = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( hero != NULL )
				{
					if ( hero->AddHaveTitle( titleInsert->titleIdx ) )
						hero->SendHaveTitle( titleInsert->titleIdx );
				}
			}
			break;
		case SQL_GAME_PROCESS_TITLE_SELECT:
			if( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if( clientInfo->gameInDb.titleList == true )
				{
					clientInfo->gameInDb.titleList = false;
					clientInfo->request.titleSend = true;
					clientInfo->complete.title = true;

					if ( hero != NULL )
					{
						TITLE_SELECT*	titleSelect = (TITLE_SELECT*)perIoContext->buffer;
						TB_TITLE*		table		= titleSelect->table;

						/// 소유 호칭 목록 삭제
						hero->ClearHaveTitleList();

						/// 소유목록에 추가
						for( long i = 0; i < titleSelect->rowCount; i++, table++ )
						{
							if ( hero->AddHaveTitle( table->titleIdx ) == false )
							{
								PostServerEvent( "SQL_GAME_PROCESS_TITLE_SELECT, heroIdx = %d titleIdx = %d", hero->GetObjectID(), table->titleIdx );
							}
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILD_CREATE:
			if( perSocketContext->socket == perIoContext->socket )
			{
				/// 길드 생성
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				
				if ( clientInfo->guildDb.guildCreate == true )
				{
					clientInfo->guildDb.guildCreate = false;
					if ( hero != NULL )
					{
						GUILD_CREATE* guildCreate = (GUILD_CREATE*)perIoContext->buffer;

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if ( guildCreate->retvalue == 0 )
							{
								/// 1. 길드 추가.
								if ( mpGuildManager->AddGuild( guildCreate, hero ) == true )
								{
									PostGuildEvent( hero->GetObjectID(), guildCreate->idx, hero->GetGuildPosition(), "SUCCESS - SQL_GAME_PROCESS_GUILD_CREATE AddGuild()" );

									/// 2. HERO에 길드 세팅
									hero->ChangeGuild( guildCreate->idx, eGUILD_MASTER );

									/// 결과 보내기
									SendMsgError( perSocketContext, NM_NPC, NM_NPC_GUILD_CREATE_RES, ERROR_NPC_GUILD_CREATE_SUCESS );

									/// 3. 길드정보
									HANDLE                    handle = NULL;
									MSG_RES_GUILD_CREATEDATA* sendMsg = (MSG_RES_GUILD_CREATEDATA*)GetMsgRoot( &handle, perSocketContext, NM_GUILD, NM_GUILD_CREATEDATA_RES );
									sendMsg->idx = guildCreate->idx;
									wcscpy( sendMsg->name, guildCreate->name );
									SendMsgRoot( handle, sizeof(MSG_RES_GUILD_CREATEDATA) );

									/// 4. 금액 정산
									if( guildCreate->money != 0 )
									{
										unsigned long befor = hero->GetMoney();
										long		  result = hero->AddMoney( hero->GetObject(), guildCreate->money, false	);

										PostMoneyEvent( guildCreate->characterIdx, befor, hero->GetMoney(), "GUILD_CREATE::Money(=%d)", guildCreate->money );

										if ( result != guildCreate->money )
											PostServerEvent( "Error - SQL_GAME_PROCESS_GUILD_CREATE AddMoney(=%d) != ResultMoney(=%d).", guildCreate->money, result );
									}

									/// SYN : 길드 생성
									if( mpGuildManager->PostGuildCreate( guildCreate, hero ) == false )
									{
										PostGuildEvent( hero->GetObjectID(), guildCreate->idx, hero->GetGuildPosition(), "FAIL - SQL_GAME_PROCESS_GUILD_CREATE PostGuildCreate()" );
									}
								}
								else
								{
									PostGuildEvent( hero->GetObjectID(), guildCreate->idx, hero->GetGuildPosition(), "fail - SQL_GAME_PROCESS_GUILD_CREATE AddGuild()" );

									PostServerEvent( "1 : SQL_GAME_PROCESS_GUILD_CREATE, heroIdx = %d guildIndex = %d", hero->GetObjectID(), guildCreate->idx );
									Close( perSocketContext );
								}
							}
							else
							{
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILD_CREATE retvalue(%d)", guildCreate->retvalue );
								PostGuildEvent( hero->GetObjectID(), guildCreate->idx, hero->GetGuildPosition(), buffer );

								SendMsgError( perSocketContext, NM_NPC, NM_NPC_GUILD_CREATE_RES, guildCreate->retvalue );
								PostServerEvent( "2 : SQL_GAME_PROCESS_GUILD_CREATE, heroIdx = %d guildIndex = %d", hero->GetObjectID(), guildCreate->idx );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "3 : SQL_GAME_PROCESS_GUILD_CREATE, heroIdx = %d guildIndex = %d", hero->GetObjectID(), guildCreate->idx );
							Close( perSocketContext );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILDUSER_ADD:
			if( perSocketContext->socket == perIoContext->socket )
			{
				// 길드원 추가
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long	heroIdx	   = clientInfo->characterIdx;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( heroIdx );

				if ( clientInfo->guildDb.guildAdd == true )
				{
					clientInfo->guildDb.guildAdd = false;
					if ( hero != NULL )
					{
						GUILDUSER_ADD*	guildUserAdd = (GUILDUSER_ADD*)perIoContext->buffer;

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							/// 추가 성공
							if( guildUserAdd->retvalue == 0 )
							{
								cGuild* guild = mpGuildManager->GetGuild( guildUserAdd->idx );
								if( guild && guild->AddPlayer( hero, eGUILD_GENERAL, mChannelNum ) == true )
								{
									PostGuildEvent( guildUserAdd->charaterIdx, guildUserAdd->idx, eGUILD_GENERAL, "SUCCESS - SQL_GAME_PROCESS_GUILDUSER_ADD AddPlayer()" );

									// HERO에 길드 세팅
									hero->ChangeGuild( guildUserAdd->idx, eGUILD_GENERAL );

									// 결과 보내기
									SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_REPLY_RES, ERROR_GUILD_ADDREPLY_SUCCESS );

									// HERO에 길드 리스트 보내주기
									mpGuildManager->SendGuildInfo( (ULONG_PTR)perSocketContext, hero, guildUserAdd->idx );

									// 길드원들에게 추가되었음 보내주기
									short mapnum = hero->GetMapDataNumber();

									mpGuildManager->SendGuildUserAdd( guild, hero->GetObjectID(), hero->GetLevel(), hero->GetJob(), 
										mapnum, hero->GetName(), hero->GetGuildPosition(), mChannelNum );

									/// SYN : 길드원 추가
									if( mpGuildManager->PostGuildUserAdd( guildUserAdd->idx, hero, mapnum ) == false )
									{
										PostGuildEvent( guildUserAdd->charaterIdx, guildUserAdd->idx, eGUILD_GENERAL, "FAIL - SQL_GAME_PROCESS_GUILDUSER_ADD PostGuildUserAdd()" );
									}
								}
								else
								{
									PostGuildEvent( guildUserAdd->charaterIdx, guildUserAdd->idx, eGUILD_GENERAL, "FAIL - SQL_GAME_PROCESS_GUILDUSER_ADD AddPlayer()" );

									PostServerEvent( "1: SQL_GAME_PROCESS_GUILDUSER_ADD, heroIdx = %d guildIndex = %d", heroIdx, guildUserAdd->idx );
									Close( perSocketContext );
								}
							}
							else
							{
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILDUSER_ADD retvalue(%d)", guildUserAdd->retvalue );
								PostGuildEvent( guildUserAdd->charaterIdx, guildUserAdd->idx, eGUILD_NONE, buffer );

								SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_REPLY_RES, ERROR_GUILD_ADDREPLY_FAIL );
								PostServerEvent( "2: SQL_GAME_PROCESS_GUILDUSER_ADD, heroIdx = %d, heroPosition = %d, characterIdx = %d, guildIndex = %d", 
									heroIdx, hero->GetGuildPosition(), guildUserAdd->charaterIdx, guildUserAdd->idx );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "3: SQL_GAME_PROCESS_GUILDUSER_ADD, heroIdx = %d guildIndex = %d", heroIdx, guildUserAdd->idx );
							Close( perSocketContext );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILDUSER_OUT:
			if( perSocketContext->socket == perIoContext->socket )
			{
				// 길드원 탈퇴처리
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long	heroIdx	   = clientInfo->characterIdx;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->guildDb.guildOut == true )
				{
					clientInfo->guildDb.guildOut = false;
					if ( hero != NULL )
					{
						GUILDUSER_OUT* guildUserOut = (GUILDUSER_OUT*)perIoContext->buffer;

						if( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if( guildUserOut->retvalue == 0 )
							{
								cGuild* guild = mpGuildManager->GetGuild( guildUserOut->idx );
								if( guild && guild->DeletePlayer( hero->GetObjectID() ) == true )
								{
									PostGuildEvent( heroIdx, guildUserOut->idx, eGUILD_NONE, "SUCCESS - SQL_GAME_PROCESS_GUILDUSER_OUT DeletePlayer()" );

									// HERO 길드 정보 삭제
									hero->ChangeGuild( 0, eGUILD_NONE );

									// 자신에게 쏘기
									SendMsgRoot( perSocketContext, NM_GUILD, NM_GUILD_OUT_RES );

									// 길드원들에게 쏘기
									mpGuildManager->SendGuildUserOut( guild, hero->GetObjectID() );

									// 길드 신청정보 있으면 삭제
									hero->SetGuildAddReq( false );

									/// SYN : 길드 탈퇴
									if( mpGuildManager->PostGuildUserOut( guildUserOut->idx, guildUserOut->charaterIdx ) == false )
									{
										PostGuildEvent( heroIdx, guildUserOut->idx, eGUILD_NONE, "FAIL - PostGuildUserOut()" );
									}
								}
								else
								{
									PostGuildEvent( heroIdx, guildUserOut->idx, hero->GetGuildPosition(), "FAIL - SQL_GAME_PROCESS_GUILDUSER_OUT DeletePlayer()" );

									PostServerEvent( "1: SQL_GAME_PROCESS_GUILDUSER_OUT, heroIdx = %d guildIndex = %d", heroIdx, guildUserOut->idx );
									Close( perSocketContext );
								}
							}
							else
							{
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILDUSER_OUT retvalue(%d)", guildUserOut->retvalue );
								PostGuildEvent( heroIdx, guildUserOut->idx, hero->GetGuildPosition(), buffer );

								SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_OUT_RES, ERROR_GUILD_OUT_FAIL );
								PostServerEvent( "2: SQL_GAME_PROCESS_GUILDUSER_OUT, heroIdx = %d guildIndex = %d", heroIdx, guildUserOut->idx );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "3: SQL_GAME_PROCESS_GUILDUSER_OUT, heroIdx = %d guildIndex = %d", heroIdx, guildUserOut->idx );
							Close( perSocketContext );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILDUSER_CUT:
			if( perSocketContext->socket == perIoContext->socket )
			{
				// 길드원 추방
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;

				if( clientInfo->guildDb.guildCut == true )
				{
					clientInfo->guildDb.guildCut = false;

					GUILDUSER_OUT* guildUserCut = (GUILDUSER_OUT*)perIoContext->buffer;

					if( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
					{
						if( guildUserCut->retvalue == 0 )
						{
							cGuild* guild = mpGuildManager->GetGuild( guildUserCut->idx );
							if( guild && guild->DeletePlayer( guildUserCut->charaterIdx ) == true )
							{
								PostGuildEvent( guildUserCut->charaterIdx, guildUserCut->idx, eGUILD_NONE, "SUCCESS - SQL_GAME_PROCESS_GUILDUSER_CUT DeletePlayer()" );

								// 추방 성공 알림
								SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_CUT_RES, ERROR_GUILD_CUT_SUCCESS );

								// 추방당한 유저가 게임중이면 처리
								cPlayer* other = mpObjectManager->GetPlayer( guildUserCut->charaterIdx );
								if( other )
								{
									other->ChangeGuild( 0, eGUILD_NONE );
	
									// 추방 
									SendMsgRoot( other->GetConnectionIdx(), NM_GUILD, NM_GUILD_CUT_SYN );

									// 길드 신청 정보 있으면 삭제
									other->SetGuildAddReq( false );
								}

								// 길드원들에게 쏘기
								mpGuildManager->SendGuildUserCut( guild, guildUserCut->charaterIdx );

								/// SYN : 길드원 추방
								if( mpGuildManager->PostGuildUserCut( guildUserCut->idx, guildUserCut->charaterIdx ) == false )
								{
									PostGuildEvent( guildUserCut->charaterIdx, guildUserCut->idx, eGUILD_NONE, "FAIL - SQL_GAME_PROCESS_GUILDUSER_CUT PostGuildUserCut()" );
								}
							}
							else
							{
								PostGuildEvent( guildUserCut->charaterIdx, guildUserCut->idx, eGUILD_NONE, "FAIL - SQL_GAME_PROCESS_GUILDUSER_CUT DeletePlayer()" );

								PostServerEvent( "1: SQL_GAME_PROCESS_GUILDUSER_CUT, charaterIdx = %d guildIndex = %d", guildUserCut->charaterIdx, guildUserCut->idx );
								Close( perSocketContext );
							}
						}
						else
						{
							char	buffer[1024]   = "\0";
							sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILDUSER_CUT retvalue(%d)", guildUserCut->retvalue );
							PostGuildEvent( guildUserCut->charaterIdx, guildUserCut->idx, eGUILD_NONE, buffer );

							SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_CUT_RES, ERROR_GUILD_CUT_FAIL );
							PostServerEvent( "2: SQL_GAME_PROCESS_GUILDUSER_CUT, otherIdx = %d guildIndex = %d", guildUserCut->charaterIdx, guildUserCut->idx );
						}
					}
					else
					{
						// 등록 오류시, 클라이언트 강제 종료.
						PostServerEvent( "3: SQL_GAME_PROCESS_GUILDUSER_CUT, otherIdx = %d guildIndex = %d", guildUserCut->charaterIdx, guildUserCut->idx );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILD_DELETE:
			if( perSocketContext->socket == perIoContext->socket )
			{
				// 길드해체
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long	heroIdx	   = clientInfo->characterIdx;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( heroIdx );

				if( clientInfo->guildDb.guildDelete == true )
				{
					clientInfo->guildDb.guildDelete = false;
					if ( hero != NULL )
					{
						GUILD_DELETE* guildDelete = (GUILD_DELETE*)perIoContext->buffer;

						if( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if( guildDelete->retvalue == 0 )
							{
								if( mpGuildManager->DeleteGuild( guildDelete->idx ) == true )
								{
									PostGuildEvent( heroIdx, guildDelete->idx, eGUILD_NONE, "SUCCESS - SQL_GAME_PROCESS_GUILD_DELETE DeleteGuild()" );

									// HERO 길드 정보 삭제
									hero->ChangeGuild( 0, eGUILD_NONE );

									// 자신에게 쏘기
									SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_DELETE_RES, ERROR_GUILD_DELETE_SUCCESS );

									//
									hero->SetGuildAddReq( false );

									/// SYN : 길드 삭제
									if( mpGuildManager->PostGuildDelete( guildDelete->idx ) == false )
									{
										PostGuildEvent( heroIdx, guildDelete->idx, eGUILD_NONE, "FAIL - SQL_GAME_PROCESS_GUILD_DELETE PostGuildDelete()" );
									}
								}
								else
								{
									PostGuildEvent( heroIdx, guildDelete->idx, hero->GetGuildPosition(), "FAIL - SQL_GAME_PROCESS_GUILD_DELETE DeleteGuild()" );

									PostServerEvent( "1: SQL_GAME_PROCESS_GUILD_DELETE, heroIdx = %d guildIndex = %d", heroIdx, guildDelete->idx );
									Close( perSocketContext );
								}
							}
							else
							{
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILD_DELETE retvalue(%d)", guildDelete->retvalue );
								PostGuildEvent( heroIdx, guildDelete->idx, hero->GetGuildPosition(), buffer );

								SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_DELETE_RES, ERROR_GUILD_DELETE_FAIL );
								PostServerEvent( "2: SQL_GAME_PROCESS_GUILD_DELETE, heroIdx = %d guildIndex = %d", heroIdx, guildDelete->idx );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "3: SQL_GAME_PROCESS_GUILD_DELETE, heroIdx = %d guildIndex = %d", heroIdx, guildDelete->idx );
							Close( perSocketContext );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILD_UPDATE:
			if( mRequest.guildDb )
			{
				// 길드 업데이트
				cCSLock		  lock( &mCs );
				GUILD_UPDATE* guildUpdate = (GUILD_UPDATE*)perIoContext->buffer;

				// 에러
				if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS || guildUpdate->retvalue != 0 )
				{
					PostServerEvent( "SQL_GAME_PROCESS_GUILD_UPDATE - fail [%d,%d,%d]", guildUpdate->idx, perIoContext->requestResult, guildUpdate->retvalue );
				}

				// 분할 체크
				if ( GUILDMAN->CheckGuildSaveTotal() == true )
				{
					mRequest.guildDb = false;
					mComplete.guildDb = true;
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION:
			if( perSocketContext->socket == perIoContext->socket )
			{
				// 직위 변경
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long	heroIdx	   = clientInfo->characterIdx;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( heroIdx );
				
				if ( clientInfo->guildDb.guildGivePosition == true )
				{
					clientInfo->guildDb.guildGivePosition = false;
					if ( hero != NULL )
					{
						GUILDUSER_GIVEPOSITION* guildUserGivePos = (GUILDUSER_GIVEPOSITION*)perIoContext->buffer;

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if ( guildUserGivePos->retvalue == 0 )
							{
								// 해당 유저의 길드 직위를 변경한다
								cGuild* guild = mpGuildManager->GetGuild( guildUserGivePos->idx );
								if ( guild != NULL )
								{
									PostGuildEvent( guildUserGivePos->characterIdx, guildUserGivePos->idx, guildUserGivePos->position, "SUCCESS - SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION" );

									// 직위 변경 및 send
									mpGuildManager->SendGuildChangePosition( guild, guildUserGivePos->characterIdx, guildUserGivePos->position );

									// SYN : 길드직위 변경
									if( mpGuildManager->PostGuildGivePosition( guildUserGivePos->idx, guildUserGivePos->characterIdx, guildUserGivePos->position ) == false )
									{
										PostGuildEvent( guildUserGivePos->characterIdx, guildUserGivePos->idx, guildUserGivePos->position, "FAIL - PostGuildGivePosition()" );
									}
								}
								else
								{
									PostGuildEvent( heroIdx, guildUserGivePos->idx, hero->GetGuildPosition(), "FAIL - SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION not exist guild" );

									PostServerEvent( "1: SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION, heroIdx = %d guildIndex = %d", heroIdx, guildUserGivePos->idx );
									Close( perSocketContext );
								}
							}
							else
							{
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION retvalue(%d)", guildUserGivePos->retvalue );
								PostGuildEvent( guildUserGivePos->characterIdx, guildUserGivePos->idx, eGUILD_NONE, buffer );

								SendMsgError( perSocketContext, NM_GUILD, NM_GUILD_GIVEPOSITION_RES, ERROR_GUILD_GIVEPOSITION_FAIL );
								PostServerEvent( "2: SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION, characterIdx = %d guildIndex = %d", guildUserGivePos->characterIdx, guildUserGivePos->idx );
							}
						}
						else
						{
							// 등록 오류시, 클라이언트 강제 종료.
							PostServerEvent( "3: SQL_GAME_PROCESS_GUILDUSER_GIVEPOSITION, heroIdx = %d guildIndex = %d", heroIdx, guildUserGivePos->idx );
							Close( perSocketContext );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_GUILDMARK_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				// 길드 마크 변경
				cCSLock			lock( &mCs );
				ClientInfo*		clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long	heroIdx	   = clientInfo->characterIdx;
				cPlayer*		hero	   = mpObjectManager->GetPlayer( heroIdx );

				if ( clientInfo->guildDb.guildMarkUpdate == true )
				{
					clientInfo->guildDb.guildMarkUpdate = false;
					if ( hero != NULL )
					{
						GUILDMARK_UPDATE* guildMarkUpdate = (GUILDMARK_UPDATE*)perIoContext->buffer;

						if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
						{
							if ( guildMarkUpdate->retvalue == 0 )
							{
								cGuild* guild = mpGuildManager->GetGuild( guildMarkUpdate->idx );
								if ( guild != NULL )
								{
									PostGuildEvent( heroIdx, guildMarkUpdate->idx, eGUILD_MASTER, "SUCCESS - SQL_GAME_PROCESS_GUILDMARK_UPDATE" );

									// 길드 마크 정보 변경
									int size = sizeof(guildMarkUpdate->mark.binary);
									guild->UpdateMark( guildMarkUpdate->markRegistDate, guildMarkUpdate->mark.binary, size );

									// 길드원들에게 변경된 마크인덱스 전송
									mpGuildManager->SendGuildMarkUpdate( guild );

									// SYN : 길드 마크 변경
									if ( mpGuildManager->PostGuildUpdateMark( guildMarkUpdate->idx, guildMarkUpdate->markRegistDate, guildMarkUpdate->mark.binary, size ) == false )
									{
										PostGuildEvent( heroIdx, guildMarkUpdate->idx, eGUILD_MASTER, "FAIL - PostGuildUpdateMark()" );
									}
								}
								else
								{
									PostGuildEvent( heroIdx, guildMarkUpdate->idx, hero->GetGuildPosition(), "FAIL - SQL_GAME_PROCESS_GUILDMARK_UPDATE not exist guild" );

									PostServerEvent( "1: SQL_GAME_PROCESS_GUILDMARK_UPDATE, heroIdx = %d guildIndex = %d", heroIdx, guildMarkUpdate->idx );
									Close( perSocketContext );
								}
							}
							else
							{
								char	buffer[1024]   = "\0";
								sprintf( buffer, "FAIL - SQL_GAME_PROCESS_GUILDMARK_UPDATE retvalue(%d)", guildMarkUpdate->retvalue );
								PostGuildEvent( heroIdx, guildMarkUpdate->idx, eGUILD_NONE, buffer );

								SendMsgError( perSocketContext, NM_GUILD, NM_GUILDMARK_UPDATE_RES, ERROR_GUILDMARK_UPDATE_FAIL );
								PostServerEvent( "2: SQL_GAME_PROCESS_GUILDMARK_UPDATE, heroIdx = %d guildIndex = %d", heroIdx, guildMarkUpdate->idx );
							}
						}
						else
						{
							// 등록 오류시, 클라 강제 종료
							PostServerEvent( "3: SQL_GAME_PROCESS_GUILDMARK_UPDATE, heroIdx = %d guildIndex = %d", heroIdx, guildMarkUpdate->idx );
							Close( perSocketContext );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_MAKESKILL_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.makeSkillDbUse = false;
				if ( hero != NULL )
				{
					MAKESKILL_INSERT* pInsert = (MAKESKILL_INSERT*)perIoContext->buffer;

					if ( pInsert->mRetvalue == 0 )
					{
						unsigned long heroIdx = hero->GetObjectID();
						hero->SetMakeSkill( pInsert );

                        clientInfo->request.characterEquip = true;

						// 아이템 정보 갱신
						if ( pInsert->mInventory.idx != 0 )
						{
							HANDLE                  handle     = NULL;
							MSG_RES_MAKESKILL_ITEM* sendMsg    = (MSG_RES_MAKESKILL_ITEM*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_MAKESKILL_RES );
							sInventory*             sinventory = &sendMsg->inventory;

							TB_INVENTORY* table = &pInsert->mInventory;
							TB_INVENTORY* inventory;
							bool          newOrUpdate;

							if ( table->apply == InventoryApplyRemoved )
							{
								PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "MAKESKILL INSERT BY ITEM" );
							}
							else
							{
								inventory   = hero->SelectInventory( table->number );
								newOrUpdate = (inventory->idx == table->idx);
								inventory   = hero->XCopyInventory( inventory, table ); // (*inventory) = (*table);

								PostInventoryEvent( newOrUpdate ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, heroIdx, inventory, "MAKESKILL INSERT BY ITEM" );
							}

							Inventory2sInventory( sinventory, table );
							sinventory++;

							SendMsgRoot( handle, sizeof( MSG_RES_MAKESKILL_ITEM ) );
						}
					}
					else
					{
						// 등록 오류시, 클라이언트 강제 종료.
						PostServerEvent( "WARNING - SQL_GAME_PROCESS_MAKESKILL_INSERT - CHARACTER_IDX(%u), ERRCODE(%d)", clientInfo->characterIdx, pInsert->mRetvalue );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_RECIPE_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.makeSkillDbUse = false;
				if ( hero != NULL )
				{
					RECIPE_INSERT* pInsert = (RECIPE_INSERT*)perIoContext->buffer;

					if( pInsert->retvalue == 0 )
					{
						unsigned long heroIdx = hero->GetObjectID();

						hero->SetRecipe( pInsert->mMakeSkillIdx, pInsert->mRecipeIdx );

						// 아이템 정보 갱신
						if ( pInsert->mInventory.idx != 0 )
						{
							HANDLE               handle     = NULL;
							MSG_RES_MAKESKILL_ITEM* sendMsg    = (MSG_RES_MAKESKILL_ITEM*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_RECIPE_RES );
							sInventory*          sinventory = &sendMsg->inventory;

							TB_INVENTORY*        table = &pInsert->mInventory;
							TB_INVENTORY*        inventory;
							bool                 newOrUpdate;

							if ( table->apply == InventoryApplyRemoved )
							{
								PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "QUEST INSERT BY ITEM" );
							}
							else
							{
								inventory   = hero->SelectInventory( table->number );
								newOrUpdate = (inventory->idx == table->idx);
								inventory   = hero->XCopyInventory( inventory, table ); // (*inventory) = (*table);

								PostInventoryEvent( newOrUpdate ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, heroIdx, inventory, "QUEST INSERT BY ITEM" );
							}

							Inventory2sInventory( sinventory, table );
							sinventory++;

							SendMsgRoot( handle, sizeof( MSG_RES_MAKESKILL_ITEM ) );
						}
					}
					else
					{
						// 등록 오류시, 클라이언트 강제 종료.
						PostServerEvent( "WARNING - SQL_GAME_PROCESS_RECIPE_INSERT - CHARACTER_IDX(%u), ERRCODE(%d)", clientInfo->characterIdx, pInsert->retvalue );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_SKILLRESET:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbCharacterSkillreset( (ULONG_PTR)perSocketContext, (CHARACTER_SKILL_RESET*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_CHARACTER_SKILLRESET - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_CHARACTER_SKILLRESET - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_THEMERESET:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				try {
					if ( hero != NULL )
					{
						hero->CbCharacterThemeReset( (ULONG_PTR)perSocketContext, (CHARACTER_THEME_RESET*)perIoContext->buffer, perIoContext->requestResult );
					}
				}
				catch ( u_long error ) {
					PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_CHARACTER_THEMERESET - CHARACTER_IDX(%d)", error, clientInfo->characterIdx );
					Close( perSocketContext );
				}
				catch ( TB_INVENTORY* table ) {
					PostServerEvent( "WARNING - SQL_GAME_PROCESS_CHARACTER_THEMERESET - CHARACTER_IDX(%d) - INVENTORY_IDX(=%d)", clientInfo->characterIdx, table->idx );
					Close( perSocketContext );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_GM_THEMERESET:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				CHARACTER_GM_THEME_RESET* gmThemeReset = (CHARACTER_GM_THEME_RESET*)perIoContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );
				
				/// db 플레그 끄기
				if( clientInfo->commonDb.gmThemeUserReset )
					clientInfo->commonDb.gmThemeUserReset = false;
				else
					break;
			
				if ( hero != NULL )
				{
					if( gmThemeReset )
					{
						if ( perIoContext->requestResult != 0 )
						{
							PostServerEvent( "ERROR(%d) - SQL_GAME_PROCESS_CHARACTER_GM_THEMERESET - CHARACTER_IDX(%d)", perIoContext->requestResult, gmThemeReset->mCharacterIdx  );
							break;	
						}

						/// 초기화 해야할 유저 존재 하지 않음
						if( gmThemeReset->mCharacterIdx == 0 )
						{
							SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_RES, ERROR_CHEAT_GM_THEME_RESET_NO_PLAYER );
							break;
						}

						if( gmThemeReset->mRetvalue != 0 )
						{
							/// 에러메시지	
							NETWORK2->SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_RES, gmThemeReset->mRetvalue );
							break;
						}

						/// 초기화 해야할 유저가 접속해져 있는 경우 테마 정보 재셋팅
						cPlayer* player = mpObjectManager->GetPlayer( gmThemeReset->mCharacterIdx );
						if( player )
							player->CbCharacterGMThemeReset( (CHARACTER_GM_THEME_RESET*)perIoContext->buffer );
						
						NETWORK2->SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_THEME_RESET_RES, ERROR_CHEAT_GM_THEME_RESET_SUCCESS );

						/// 로그 남기기
						PostServerEvent( "SQL_GAME_PROCESS_CHARACTER_GM_THEMERESET - CHARACTER_IDX(%d), GM_IDX(%d)", gmThemeReset->mCharacterIdx, clientInfo->characterIdx );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_RECIPE_GROUP_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		  lock( &mCs );
				ClientInfo*	  clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long heroIdx    =  clientInfo->characterIdx;
				cPlayer*      hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.makeSkillDbUse = false;
				if ( hero != NULL )
				{
					RECIPE_GROUP_INSERT* pInsert = (RECIPE_GROUP_INSERT*)perIoContext->buffer;

					if ( pInsert->mRetvalue == 0 )
					{
						hero->SetRecipeGroup( pInsert->mRecipeGroup, pInsert->mRowCnt );

						/// 아이템 정보 갱신
						if ( pInsert->mInventory.idx != 0 )
						{
							HANDLE                  handle     = NULL;
							MSG_RES_MAKESKILL_ITEM* sendMsg    = (MSG_RES_MAKESKILL_ITEM*)GetMsgRoot( &handle, perSocketContext, NM_ITEM, NM_ITEM_RECIPE_RES );
							sInventory*             sinventory = &sendMsg->inventory;

							TB_INVENTORY* table = &pInsert->mInventory;
							TB_INVENTORY* inventory;
							bool          newOrUpdate;

							if ( table->apply == InventoryApplyRemoved )
							{
								PostInventoryEvent( EVENT_INVENTORY_DELETE, heroIdx, table, "QUEST INSERT BY ITEM" );
							}
							else
							{
								inventory   = hero->SelectInventory( table->number );
								newOrUpdate = (inventory->idx == table->idx);
								inventory   = hero->XCopyInventory( inventory, table ); // (*inventory) = (*table);

								PostInventoryEvent( newOrUpdate ? EVENT_INVENTORY_UPDATE : EVENT_INVENTORY_CREATE, heroIdx, inventory, "QUEST INSERT BY ITEM" );
							}

							Inventory2sInventory( sinventory, table );
							sinventory++;

							SendMsgRoot( handle, sizeof( MSG_RES_MAKESKILL_ITEM ) );
						}
					}
					else
					{
						// 등록 오류시, 클라이언트 강제 종료.
						PostServerEvent( "WARNING - SQL_GAME_PROCESS_RECIPE_INSERT - CHARACTER_IDX(%u)", clientInfo->characterIdx );
						Close( perSocketContext );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_RECIPE_COOLTIME:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock        lock( &mCs );
				ClientInfo*    clientInfo   = (ClientInfo*)perSocketContext->buffer;
				RECIPE_COOLTIME* skillCoolTime = (RECIPE_COOLTIME*)perIoContext->buffer;

				if( skillCoolTime->mRetvalue != 0 )	
					PostServerEvent( "SQL_GAME_PROCESS_RECIPE_COOLTIME - DB[%d]", skillCoolTime->mRetvalue );

				clientInfo->complete.recipeCoolTime = false;	// DB 사용종료.
			}
			break;
		case SQL_GAME_PROCESS_MAKESKILL_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( clientInfo->gameInDb.makeSkillList )
				{
					clientInfo->gameInDb.makeSkillList = false;
					if ( hero != NULL )
					{
						MAKESKILL_SELECT* pMakeSkillSelect = (MAKESKILL_SELECT*)perIoContext->buffer;

						/// 플레이어에 제조스킬을 담는다.
						if( hero->InitMakeSkill( pMakeSkillSelect ) == false )
						{
							PostServerEvent("SQL_GAME_PROCESS_MAKESKILL_SELECT pHero[%d]->InitMakeSkill( pMakeSkillSelect ) == false", hero->GetObjectID() );
						}
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_MAKESKILL_DELETE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				clientInfo->skillDb.makeSkillDbUse = false;
                if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                {
                    if ( hero != NULL )
                    {
                        MAKESKILL_DELETE* pMakeSkillDelete = (MAKESKILL_DELETE*)perIoContext->buffer;

                        if( pMakeSkillDelete->mRetvalue == 0 )
                        {
                            hero->DelMakeSkill( pMakeSkillDelete->mMakeSkillIdx );
                            clientInfo->request.characterEquip = true;
                        }
                        else
                        {
                            PostServerEvent("SQL_GAME_PROCESS_MAKESKILL_DELETE pMakeSkillDelete[%d,%d,%d]->mRetvalue != 0",
                                pMakeSkillDelete->mCharacterIdx, pMakeSkillDelete->mMakeSkillIdx, pMakeSkillDelete->mRetvalue );

                            SendMsgError( perSocketContext, NM_MAKESKILL, NM_MAKESKILL_DEL_RES, ERROR_MAKESKILL_DEL_DBERROR );
                        }
                    }
                    else
                    {
                        PostServerEvent("SQL_GAME_PROCESS_MAKESKILL_DELETE hero[%d] == NULL", clientInfo->characterIdx );
                        Close( perSocketContext );
                    }
                }
			}
			break;
		case SQL_GAME_PROCESS_FORTUNE_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		  lock( &mCs );
				ClientInfo*	  clientInfo = (ClientInfo*)perSocketContext->buffer;
				unsigned long heroIdx	 = clientInfo->characterIdx;
				cPlayer*      hero       = mpObjectManager->GetPlayer( heroIdx );

				if ( clientInfo->gameInDb.fortuneData == true )
				{
					clientInfo->gameInDb.fortuneData = false;
					clientInfo->complete.fortuneData = true;
					if ( hero != NULL )
					{
						FORTUNE_SELECT* fortuneSelect = (FORTUNE_SELECT*)perIoContext->buffer;

						// TIMESTAMP_STRUCT -> tm 로 변환
						TIMESTAMP_STRUCT* validThru = fortuneSelect->thru;

						/// 정보 복사
						TIMESTAMP_STRUCT* heroThru = 0;
						for( int i = 0; i < 5; ++i, ++validThru )
						{
							heroThru = hero->GetFortuneThru( i );
							if ( heroThru )
							{
								(*heroThru) = (*validThru);
							}
							else
							{
								/// 오류
								PostServerEvent( "SQL_GAME_PROCESS_FORTUNE_SELECT - not exist fortuneThru data, heroIdx = %d, index = %d", heroIdx, i );
							}
						}

						/// 오늘의 한마디 정보 복사
						sPlayerExrInfo* exrInfo = hero->GetExrInfo();

						exrInfo->mTodayColor = fortuneSelect->color;
						wcscpy( exrInfo->mTodayWord, fortuneSelect->word ); 
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_FORTUNE_UPDATE:
			{
				cCSLock		  lock( &mCs );
				FORTUNE_UPDATE* fortuneUpdate = (FORTUNE_UPDATE*)perIoContext->buffer;

				/// 에러
				if( perIoContext->requestResult != IOCP_REQUEST_SUCCESS || fortuneUpdate->retvalue != 0 )
				{
					PostServerEvent( "SQL_GAME_PROCESS_FORTUNE_UPDATE - fail, heroIdx = %d", fortuneUpdate->characterIdx );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock                 lock( &mCs );
				CHARACTER_BLOCK_SELECT* characterBlockSelect = (CHARACTER_BLOCK_SELECT*)perIoContext->buffer;

				if ( characterBlockSelect->rowCount > 0 && characterBlockSelect->validTrue > 0 )
				{
					ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
					cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

					if ( hero != NULL )
						hero->SetBlock( true, characterBlockSelect->validTrue );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_BLOCK_SELECT_LOG:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock                      lock( &mCs );
				CHARACTER_BLOCK_SELECT_LIST* characterBlockSelectList = (CHARACTER_BLOCK_SELECT_LIST*)perIoContext->buffer;

				HANDLE						handle  = NULL;
				MSG_RES_CHEAT_NO_CHAT_LIST* sendMsg = (MSG_RES_CHEAT_NO_CHAT_LIST*)GetMsgRoot( &handle 
																							  ,perSocketContext
																							  ,NM_CHEAT
																							  ,NM_CHEAT_NO_CHAT_LIST_RES );
				NO_CHAT_CHARACTER*			table       = characterBlockSelectList->table;
				NO_CHAT_CHARACTER_DUMMY*	sendTable   = sendMsg->table;

				if ( characterBlockSelectList->retValue == 0 && sendMsg )
				{
					sendMsg->ErrorCode = ERROR_CHEAT_NO_CHAT_LIST_SUCCESS;
					sendMsg->mRowCount = 0;

					for( int i = 0; i < characterBlockSelectList->rowCount; i++, table++, sendTable++ )
					{
						//sendTable->index = table->index;
						wcscpy_s( sendTable->characterName, sizeof( sendTable->characterName ), table->characterName );
						wcscpy_s( sendTable->adminName,     sizeof( sendTable->adminName ),     table->adminName );
						memcpy( &sendTable->registDate,     &table->registDate, sizeof( table->validThru ) );						
						memcpy( &sendTable->validThru,		&table->validThru, sizeof( table->validThru ) );						
						sendTable->registDate = timestamp2time_t( table->registDate );
						sendTable->validThru  = timestamp2time_t( table->validThru );
						sendMsg->mRowCount++;
					}

					SendMsgRoot( handle, sendMsg->GetMsgLength() );
				}
				else
				{
					///해당 유저 케릭터 삭제됨 에러 메시지 발송.
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_NO_CHAT_LIST_RES_GM, ERROR_CHEAT_NO_CHAT_LIST_NO_PLAYER );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_BLOCK_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock                 lock( &mCs );
				CHARACTER_BLOCK_INSERT* characterBlockInsert = (CHARACTER_BLOCK_INSERT*)perIoContext->buffer;

				if ( characterBlockInsert->blockCharacterIdx != 0 )
				{
					// 유저에게 통보
					if( characterBlockInsert->channelNum != -1 )
					{
						cPlayer* player = OBJECTMANAGER->GetPlayer( characterBlockInsert->blockCharacterIdx );
						if ( player != NULL )
						{
							if ( characterBlockInsert->validTrue != 0 )
							{
								if ( player->SetBlock( true, characterBlockInsert->validTrue ) == true )
									SendMsgRoot( player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_NO_CHAT_SYN );
							}
							else
							{
								if ( player->SetBlock( false, 0 ) == true )
									SendMsgRoot( player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_ALLOW_CHAT_SYN );
							}
						}
						else
						{
							// 채널 동기화 메시지 보내기.
							PostNoChatEvent( characterBlockInsert->blockCharacterIdx, characterBlockInsert->validTrue );
						}
					}

					/// GM에게 결과 통보
					if ( characterBlockInsert->validTrue != 0 )
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_NO_CHAT_RES_GM, ERROR_CHEAT_NO_CHAT_SUCCESS );
					else
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_ALLOW_CHAT_RES_GM, ERROR_CHEAT_ALLOW_CHAT_SUCCESS );
				}
				else
				{
					/// GM에게 결과 통보
					if ( characterBlockInsert->validTrue != 0 )
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_NO_CHAT_RES_GM, ERROR_CHEAT_NO_CHAT_NO_PLAYER );
					else
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_ALLOW_CHAT_RES_GM, ERROR_CHEAT_ALLOW_CHAT_NO_PLAYER );
				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_GM_NAME:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock		lock( &mCs );
				ClientInfo*	clientInfo = (ClientInfo*)perSocketContext->buffer;

				if ( clientInfo->commonDb.characterGmName )
				{
					clientInfo->commonDb.characterGmName = false;

					CHARACTER_GM_NAME* name = (CHARACTER_GM_NAME*)perIoContext->buffer;
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_NAME_RES, name->retvalue );

					if( name->retvalue == ERROR_CHEAT_GM_NAME_SUCCESS )
					{
						cPlayer* hero = OBJECTMANAGER->GetPlayer( name->characterIndex );
						if( hero )
						{
							char     buffer[256]={0,};
							char     beforeName[MAX_NAME_BUFFER_SIZE]={0,};
							char     afterName[MAX_NAME_BUFFER_SIZE]={0,};

							::WideCharToMultiByte(CP_ACP, 0, hero->GetName() , (int)-1, (LPSTR)beforeName, MAX_NAME_BUFFER_SIZE, NULL, NULL);
							::WideCharToMultiByte(CP_ACP, 0, name->characterName , (int)-1, (LPSTR)afterName, MAX_NAME_BUFFER_SIZE, NULL, NULL);

							sprintf( buffer, "CharacterName Change ID:%u Before:%s After:%s", name->characterIndex, beforeName, afterName );

							hero->PostCharacterEvent( EVENT_CHARACTER_UPDATE, buffer );
						}
					}

				}
			}
			break;
		case SQL_GAME_PROCESS_CHARACTER_INVEN_SIZE:
			if ( perSocketContext->socket == perIoContext->socket )
            {
                cCSLock		lock( &mCs );
                CHARACTER_INVEN_SIZE* invenSize = (CHARACTER_INVEN_SIZE*)perIoContext->buffer;

                if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
                {
                    cPlayer* player = OBJECTMANAGER->GetPlayer( invenSize->idx );
                    if ( player != NULL )
                    {
                        sHeroInfo* heroInfo     = player->GetHeroInfo();
                        unsigned short bagEnd   = heroInfo->BagEnd;

                        heroInfo->BagEnd        = invenSize->bag;
                        heroInfo->WearEnd       = invenSize->wear;
                        heroInfo->WareHouseEnd  = invenSize->wareHouse;
                        heroInfo->MakeSkillEnd  = invenSize->makeSkill;

                        // 가방 확장이라면 무게 계산
                        if ( invenSize->bag != bagEnd )
                        {
						    STATUSCALC->CalcPlayerExtensionGlobal( player->GetObject() );
                        }


				        HANDLE					    handle  = NULL;
                        MSG_SYN_PLAYER_INVEN_SIZE*  sendMsg = (MSG_SYN_PLAYER_INVEN_SIZE*)GetMsgRoot( &handle, perSocketContext, NM_PLAYER, NM_PLAYER_INVEN_SIZE_SYN );
                        sendMsg->mBag       = heroInfo->BagEnd;
                        sendMsg->mWear      = heroInfo->WearEnd;
                        sendMsg->mWareHouse = heroInfo->WareHouseEnd;
                        sendMsg->mMakeSkill = heroInfo->MakeSkillEnd;
                        SendMsgRoot( handle, sizeof(MSG_SYN_PLAYER_INVEN_SIZE) );
                    }
                }
                else
                {
                    PostServerEvent( "WARNING - SQL_GAME_PROCESS_CHARACTER_INVEN_SIZE - CHARACTER_IDX(%u) / BACK_END(%d) / WEAR_END(%d) / WAREHOUSE_END(%d) / MAKESKILL_END(%d)"
									,invenSize->idx
									,invenSize->bag
									,invenSize->wear
									,invenSize->wareHouse
									,invenSize->makeSkill );
                }
            }
            break;
		case SQL_GAME_PROCESS_CHARACTER_KICK_SELECT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock                 lock( &mCs );
				CHARACTER_KICK_SELECT*  characterKickSelect = (CHARACTER_KICK_SELECT*)perIoContext->buffer;

				if ( characterKickSelect->kickCharacterIdx != 0 )
				{
					// 유저에게 통보
					if( characterKickSelect->channelNum != -1 )
					{
						cPlayer* player = OBJECTMANAGER->GetPlayer( characterKickSelect->kickCharacterIdx );
						if ( player != NULL )
						{
							SendMsgRoot( player->GetConnectionIdx(), NM_CHEAT, NM_CHEAT_KICK_RES );
							CloseCID( player->GetConnectionIdx() );
						}
						else
						{
							// 채널 동기화 메시지 보내기.
							PostGMKickEvent( characterKickSelect->kickCharacterIdx );
						}

						/// GM에게 결과 통보
						HANDLE handle = NULL;
						MSG_RES_CHEAT_KICK_GM* sendMsg = (MSG_RES_CHEAT_KICK_GM*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_KICK_RES_GM );
						if( sendMsg != NULL )
						{
							sendMsg->ErrorCode = ERROR_CHEAT_KICK_SUCCESS;
							wcscpy( sendMsg->mCharName, characterKickSelect->characterName );
							SendMsgRoot( handle, sizeof(MSG_RES_CHEAT_KICK_GM) );
						}

					}
					else
					{
						/// GM에게 결과 통보-접속 안함
						SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_KICK_RES_GM, ERROR_CHEAT_KICK_NOT_CONNECTED );
					}
				}
				else
				{
					/// GM에게 결과 통보-없는 유저
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_KICK_RES_GM, ERROR_CHEAT_KICK_UNKNOWN_USER );
				}
			}
			break;

		case SQL_GAME_PROCESS_FORCEPOINT_UPDATE:
			{
				cCSLock             lock( &mCs );
				FORCE_POINT_UPDATE* pUpdate = (FORCE_POINT_UPDATE*)perIoContext->buffer;

				if ( perIoContext->requestResult == IOCP_REQUEST_SUCCESS )
				{
					if ( pUpdate->mRetValue != 0 )
					{
						PostServerEvent( "SQL_GAME_PROCESS_FORCEPOINT_UPDATE[%d] pUpdate->mRetValue == [%d] [%d,%d,%d,%d,%d,%d,%d,%d]"
										,pUpdate->mCharacterIdx
										,pUpdate->mRetValue
										,pUpdate->mFirePoint
										,pUpdate->mFireFriendly
										,pUpdate->mWaterPoint
										,pUpdate->mWaterFriendly
										,pUpdate->mWindPoint
										,pUpdate->mWindFriendly
										,pUpdate->mEarthPoint
										,pUpdate->mEarthFriendly );
					}
				}
			}
			break;

			/*-- PVP Table --*/
		case SQL_GAME_PROCESS_PVP_USER_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( hero != NULL )
				{
					PVP_USER_INSERT* insert = (PVP_USER_INSERT*)perIoContext->buffer;

					if ( insert->retvalue == 0 )
					{
						/// 등록
						PVPMANAGER->AddReadyPlayer( hero );

						/// 클라이언트에 등록 성공 알림
						SendMsgError( perSocketContext, NM_PVP, NM_PVP_READY_RES, 0 );
					}
					else	/// 클라이언트에 등록 실패 알림
						SendMsgError( perSocketContext, NM_PVP, NM_PVP_READY_RES, 1 );
				}
			}
			break;
		case SQL_GAME_PROCESS_PVP_USER_SELECT:
			{
				cCSLock             lock( &mCs );
				PVP_USER_SELECT* select = (PVP_USER_SELECT*)perIoContext->buffer;

				/// 읽어온 정보에서 인던 배분을하고 ePVPPROCESS_JOINDBWAIT 상태를 ePVPPROCESS_RUN상태로 바꾼다.
				PVPMANAGER->TeamDivision( select );

				/// 접속자 명단 삭제 쿼리 발송
				HANDLE handle = NULL;
				PVP_USER_CLEAR* pPvPClear = (PVP_USER_CLEAR*)GetSQL( &handle, SQL_GAME_PROCESS_PVP_USER_CLEAR );
				if( pPvPClear != NULL )
				{
					pPvPClear->pvpIdx = select->pvpIdx;
					pPvPClear->minLevel = select->minLevel;
					pPvPClear->maxLevel = select->maxLevel;
					SendSQL( handle, sizeof(PVP_USER_CLEAR) );
				}
			}
			break;
		case SQL_GAME_PROCESS_PVP_USER_REMOVE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock     lock( &mCs );
				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;
				cPlayer*    hero       = mpObjectManager->GetPlayer( clientInfo->characterIdx );

				if ( hero != NULL )
				{
					PVP_USER_REMOVE* remove = (PVP_USER_REMOVE*)perIoContext->buffer;

					if( remove->retvalue == 0 )
					{
						/// 등록취소
						PVPMANAGER->DelReadyPlayer( hero );

						/// 클라이언트에 등록취소 성공 알림
						SendMsgError( perSocketContext, NM_PVP, NM_PVP_READYCANCEL_RES, 0 );
					}
					else
					{
						/// 클라이언트에 등록취소 실패 알림
						SendMsgError( perSocketContext, NM_PVP, NM_PVP_READYCANCEL_RES, 1 );
					}
				}
			}
			break;
		case SQL_GAME_PROCESS_PVP_USER_CLEAR:
			{
				cCSLock lock( &mCs );
				PVP_USER_CLEAR* clear = (PVP_USER_CLEAR*)perIoContext->buffer;

				if( clear->retvalue != 0 )
				{
					PostServerEvent("SQL_GAME_PROCESS_PVP_USER_CLEAR clear->retvalue[%d,%d,%d,%d] != 0", 
						clear->retvalue, clear->pvpIdx, clear->minLevel, clear->maxLevel );
				}
			}
			break;

		case SQL_GAME_PROCESS_PARTY_INSERT:
			{
				cCSLock       lock( &mCs );
				PARTY_INSERT* insert    = (PARTY_INSERT*)perIoContext->buffer;
				DWORD         tickCount = GetTickCount( );

				cPlayer*      leader = mpObjectManager->GetPlayer( insert->leader.userIndex );
				cPlayer*      player = mpObjectManager->GetPlayer( insert->player.userIndex );
				cParty*       party  = NULL;

				if ( insert->partyIdx > 0 )
				{
					party = mpPartyManager->AddParty( insert->partyIdx, insert->leader.userIndex, insert->player.userIndex );
					if ( party != NULL )
					{
						if ( leader != NULL )
						{
							leader->SetPartyIndex( party->GetIndex( ) );
							leader->SetPartyState( ePARTY_COMPLETE );
							party->SetUserData( leader, 0 );

							SendMsgError( leader->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_CREATE_SUCCESS );
						}
						else
							party->SetUserData( &insert->leader, tickCount );

						if ( player != NULL )
						{
							player->SetPartyIndex( party->GetIndex( ) );
							player->SetPartyState( ePARTY_COMPLETE );
							party->SetUserData( player, 0 );

							SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_CREATE_SUCCESS );
						}
						else
							party->SetUserData( &insert->player, tickCount );

						if ( leader != NULL ) mpPartyManager->ResPartyList( leader, party );
						if ( player != NULL ) mpPartyManager->ResPartyList( player, party );

						mpPartyManager->PostAddParty( party->GetIndex( ), &insert->leader, &insert->player );
					}
				}
				if ( party == NULL )
				{
					if ( leader != NULL )
						SendMsgError( leader->GetConnectionIdx( ), NM_PARTY, NM_PARTY_ADD_RES, ERROR_PARTY_ADD_CREATEFAIL );

					if ( player != NULL )
						SendMsgError( player->GetConnectionIdx( ), NM_PARTY, NM_PARTY_REP_RES, ERROR_PARTY_REP_FAIL );

					PostServerEvent( "ERROR - SQL_GAME_PROCESS_PARTY_INSERT - TB_PARTY::IDX(%d)", insert->partyIdx );
				}
			}
			break;
		case SQL_GAME_PROCESS_JOB_CHEAT_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				/// 퀘스트 완료 요청 결과
				cCSLock			lock( &mCs );
				GM_JOB_UPDATE*	jobInfo = (GM_JOB_UPDATE*)perIoContext->buffer;
				cPlayer*        hero       = mpObjectManager->GetPlayer( jobInfo->characterIdx );

				if ( hero )
				{
					hero->SetJob( (ePLAYER_JOB)jobInfo->jobIndex, jobInfo->skillClassIdx, jobInfo->skillRowCount );
				}
			}
			break;
		case SQL_GAME_PROCESS_VERIFY_GM_EVENT:
			if( mRequestImport.gmEvent )
			{
				cCSLock	lock( &mCs );

				VERIFY_GM_EVENT* verifyGmEvent = (VERIFY_GM_EVENT*)perIoContext->buffer;

				// 에러
				if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
				{
					PostServerEvent( "SQL_GAME_PROCESS_VERIFY_GM_EVENT - fail [requestResult:%d][index:%d][influenceIndex:%d]", perIoContext->requestResult, verifyGmEvent->table[0].index, verifyGmEvent->table[0].infIndex );
					break;
				}

				//// GM이벤트 select
				GM_INFLUENCE_LIST* table = verifyGmEvent->table;
				for( long idx = 0; idx < verifyGmEvent->rowcount; ++idx, ++table )
				{
					if( table )
					{
						/// 신규효과 스크립트
						sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( table->infIndex );
						if( pNewScript == NULL ) 	
						{ 
							NETWORK2->PostServerEvent("ERROR-GameProcess::CallbackComplete GM_INFLUENCE_LIST [%d]==NULL", table->infIndex );
							break; 
						}
					
						sGMEventInfo eventInfo;

						eventInfo.index		   = table->index;
						eventInfo.influenceIdx = table->infIndex;
						eventInfo.startTime    = timestamp2time_t( table->startTime );
						eventInfo.validThru    = timestamp2time_t( table->validThru );
						
						mGMEventList.PushBack( eventInfo );
					}
				}

				mCompleteImport.gmEvent = true;
				mRequestImport.gmEvent = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_DROPLIMIT_SELECT:
			if( mRequestImport.itemDropLimit )
			{
				cCSLock	lock( &mCs );

				ITEM_DROPLIMIT_TABLE* pDropLimit = (ITEM_DROPLIMIT_TABLE*)perIoContext->buffer;

				// 에러
				if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
				{
					PostServerEvent( "SQL_GAME_PROCESS_ITEM_DROPLIMIT_SELECT - fail [requestResult:%d][index:%d][influenceIndex:%d]", perIoContext->requestResult );
					break;
				}
				
				/// 드랍제약 정보 기록
				for( int i = 0 ; i < pDropLimit->mCount ; ++i )
				{
					TB_ITEM_DROPLIMIT* pTable = &pDropLimit->mTable[i];
					if( pTable != NULL )
					{
						sDropLimit* pDropLimit = new sDropLimit;
						pDropLimit->mItemIdx = pTable->mItemIdx;
						pDropLimit->mDropTerm = pTable->mDropTerm;
						pDropLimit->mEndTime = pTable->mDropEndTime;
						pDropLimit->mIsDrop = pTable->mIsDrop == 1;
						if( DROP->InsertDropMonCantTime( pTable->mItemIdx, pDropLimit ) == false )
							NETWORK2->PostServerEvent("SQL_GAME_PROCESS_ITEM_DROPLIMIT_SELECT DROP->InsertDropMonCantTime( %d ) == false", pTable->mItemIdx );
					}
				}

				mCompleteImport.itemDropLimit = true;
				mRequestImport.itemDropLimit = false;
			}
			break;
		case SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE:
			{
				cCSLock	lock( &mCs );

				ITEM_DROPLIMIT_UPDATE* pUpdate = (ITEM_DROPLIMIT_UPDATE*)perIoContext->buffer;
				
				// 에러
				if( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
					PostServerEvent( "SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE IOCP_REQUEST_SUCCESS %d %d", perIoContext->requestResult, pUpdate->mItemIdx );

				if( pUpdate->mRetValue != 0 )
					PostServerEvent( "SQL_GAME_PROCESS_ITEM_DROPLIMIT_UPDATE pUpdate->mRetValue %d %d", pUpdate->mRetValue, pUpdate->mItemIdx );
			}
			break;
		case SQL_GAME_PROCESS_GM_EVENT_SEARCH:
			if ( perSocketContext->socket == perIoContext->socket )
			{
				cCSLock	lock( &mCs );

				VERIFY_GM_EVENT* verifyGmEvent = (VERIFY_GM_EVENT*)perIoContext->buffer;

				// 에러
				if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
				{
					PostServerEvent( "SQL_GAME_PROCESS_GM_EVENT_SEARCH - fail [requestResult:%d][index:%d][influenceIndex:%d]", perIoContext->requestResult, verifyGmEvent->table[0].index, verifyGmEvent->table[0].infIndex );
					break;
				}

				if( verifyGmEvent->rowcount < 1 )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_SEARCH_RES, ERROR_CHEAT_GM_EVENT_SEARCH_NO_DATA );
					break;
				}

				HANDLE						    handle    = NULL;
				GM_INFLUENCE_LIST*				table     = verifyGmEvent->table;
				MSG_RES_CHEAT_GM_EVENT_SEARCH*  sendMsg   = (MSG_RES_CHEAT_GM_EVENT_SEARCH*)GetMsgRoot( &handle, perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_SEARCH_RES );
				GM_EVENT_INFO*					sendTable = sendMsg->mTable;

				sendMsg->mRowCount = 0;					   
				sendMsg->ErrorCode = ERROR_CHEAT_GM_EVENT_SEARCH_SUCCESS;
				//// GM이벤트 select
				for( long idx = 0; idx < verifyGmEvent->rowcount; ++idx, ++table )
				{
					if( table )
					{
						/// 신규효과 스크립트
						sInfluenceScript* pNewScript = SKILLSCRIPT->GetInfluenceInfo( table->infIndex );
						if( pNewScript == NULL ) 	
						{ 
							NETWORK2->PostServerEvent("ERROR-GameProcess::CallbackComplete GM_INFLUENCE_LIST [%d]==NULL", table->infIndex );
							break; 
						}

						/// 실제 적용할 수 있는 것인지 인덱스 체크
						if( sendTable )
						{
							sendTable->index		= table->index;
							sendTable->influenceIdx = table->infIndex;
							sendTable->startTime    = timestamp2time_t( table->startTime );
							sendTable->validThru    = timestamp2time_t( table->validThru );
						}
						else
						{
							assert(0);
							break;
						}

						++sendTable;
						++sendMsg->mRowCount;
					}
				}
			
				SendMsgRoot( handle, sendMsg->GetMsgLength() );
			}
			break;
		case SQL_GAME_PROCESS_GM_EVENT_INSERT:
			if ( perSocketContext->socket == perIoContext->socket )
			{				
				cCSLock	lock( &mCs );

				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				/// db 플레그 끄기
				if( clientInfo->commonDb.gmEventInsert )
					clientInfo->commonDb.gmEventInsert = false;
				else
					break;
			
				GM_EVENT_INSERT* insertGmEvent = (GM_EVENT_INSERT*)perIoContext->buffer;

				// 에러
				if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
				{
					PostServerEvent( "SQL_GAME_PROCESS_GM_EVENT_INSERT - fail [requestResult:%d][index:%d][influenceIndex:%d]", perIoContext->requestResult, insertGmEvent->index, insertGmEvent->influenceIndex );
					break;
				}

				if( insertGmEvent->retvalue != 0 )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, insertGmEvent->retvalue );
					break;
				}
				
				if( insertGmEvent->index == 0 )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_DB_INSERT );
					break;
				}

				///적용된 효과를 추가
				sGMEventInfo eventInfo;
				eventInfo.index		   = insertGmEvent->index;
				eventInfo.influenceIdx = insertGmEvent->influenceIndex;
				eventInfo.startTime    = timestamp2time_t( insertGmEvent->startTime );
				eventInfo.validThru    = timestamp2time_t( insertGmEvent->validThru );

				mGMEventList.PushBack( eventInfo );

				/// 해당 효과를 서버의 모든 유저에게 뿌리기.
				OBJECTMANAGER->AddGMEvent( eventInfo );

				/// 채널 동기화
				if( PostAddGMEvent( eventInfo ) == false )
				{
					// 에러
					PostServerEvent( "SQL_GAME_PROCESS_GM_EVENT_INSERT:PostAddGMEvent() - fail [index:%d][influenceIndex:%d]", insertGmEvent->index, insertGmEvent->influenceIndex );
					break;
				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_APPLY_RES, ERROR_CHEAT_GM_EVENT_APPLY_SUCCESS );
			}
			break;
		case SQL_GAME_PROCESS_GM_EVENT_UPDATE:
			if ( perSocketContext->socket == perIoContext->socket )
			{				
				cCSLock	lock( &mCs );

				ClientInfo* clientInfo = (ClientInfo*)perSocketContext->buffer;

				/// db 플레그 끄기
				if( clientInfo->commonDb.gmEventUpdate )
					clientInfo->commonDb.gmEventUpdate = false;
				else
					break;

				GM_EVENT_UPDATE* updateGmEvent = (GM_EVENT_UPDATE*)perIoContext->buffer;

				// 에러
				if ( perIoContext->requestResult != IOCP_REQUEST_SUCCESS )
				{
					PostServerEvent( "SQL_GAME_PROCESS_GM_EVENT_UPDATE - fail [requestResult:%d][index:%d][retvalue:%d]", perIoContext->requestResult, updateGmEvent->index, updateGmEvent->retvalue);
					break;
				}

				if( updateGmEvent->retvalue != 0 )
				{
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_RELEASE_RES, updateGmEvent->retvalue );
					break;
				}

				///적용된 효과를 삭제
				cGMEventList::cIterator pos = mGMEventList.Begin();
				while( pos != mGMEventList.End() )
				{
					sGMEventInfo& eventInfo = (*pos);
					if( eventInfo.index == (unsigned long)updateGmEvent->index )
					{
						pos = mGMEventList.Erase( pos );
						continue;
					}
					++pos;
				}

				/// 해당 효과를 서버의 모든 유저에게 뿌리기.
				OBJECTMANAGER->DelGMEvent( updateGmEvent->index, updateGmEvent->influenceIndex );

				/// 채널 동기화
				if( PostDelGMEvent( updateGmEvent->index, updateGmEvent->influenceIndex ) == false )
				{
					// 에러
					PostServerEvent( "SQL_GAME_PROCESS_GM_EVENT_UPDATE:PostDelGMEvent() - fail [index:%d][influenceIndex:%d]", updateGmEvent->index, updateGmEvent->influenceIndex );
					break;
				}
				else
					SendMsgError( perSocketContext, NM_CHEAT, NM_CHEAT_GM_EVENT_RELEASE_RES, ERROR_CHEAT_GM_EVENT_RELEASE_SUCCESS );
			}
			break;
		}
	} catch ( char* str ) {
		PostServerEvent( "Caught 'switch ( PerIoContext::iParam(=%d) )' exception type: '%s'. Throwing 'cGameProcess::CallbackComplete' exception", perIoContext->iParam, str );
		Sleep( 100 );
		throw;
	} catch ( ... ) {
		PostServerEvent( "In 'switch ( PerIoContext::iParam(=%d) )'. Throwing 'cGameProcess::CallbackComplete' exception.", perIoContext->iParam );
		Sleep( 100 );
		throw;
	}

	Verbose->CallbackComplete( &mCs, perIoContext );

	// 사용이 완료된 I/O Context는 회수한다.
	if ( perIoContext->iParam >= DEFAULT_SQL_ACCOUNT && perIoContext->iParam < MAX_SQL_ACCOUNT )
	{
		g_gameSrv->GetSQLAccount( )->Release( perIoContext );
	}
	else if ( perIoContext->iParam >= DEFAULT_SQL_GAME && perIoContext->iParam < MAX_SQL_GAME )
	{
		g_gameSrv->GetSQLGame( )->Release( perIoContext );
	}
	else if ( perIoContext->iParam >= DEFAULT_GAME_PROCESS && perIoContext->iParam < MAX_GAME_PROCESS )
	{
		DWORD error = 0;
		mIoContextPool->ReleaseIoContext( perIoContext, false, &error );
		if ( error != 0 )
			PostServerEvent( "WARNING - cGameProcess::CallbackComplete:Error(=0x%08xh)", error );
	}
	else
		PostServerEvent( "WARNING - cGameProcess::CallbackComplete:iParam(=0x%04dxh)", perIoContext->iParam );

	return true;
}

/*-- IoContextPresent Method
*/
void cGameProcess::IoContextPresent( )
{
	if ( mSocketContextPool != NULL )
	{
		cCSLock           lock( &mCs );
		PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
		PerIoContext*     ioContext;

		char*             buffer;
		u_long            offset;
		u_long            length;

		while ( socketContext != NULL )
		{
			if ( socketContext->wsaOff > 0 )
			{
				offset = 0;
				length = socketContext->wsaOff;

				do {

					ioContext = mIoContextPool->GetIoContext( socketContext->socket, IOCP_REQUEST_WRITE );
					buffer    = socketContext->wsaBuf + offset;

					if ( length < ioContext->length )
					{
						memcpy( ioContext->buffer, buffer, length );
						ioContext->offset = length;
					}
					else
					{
						memcpy( ioContext->buffer, buffer, ioContext->length );
						ioContext->offset = ioContext->length;
					}

					offset += ioContext->offset;
					length -= ioContext->offset;

					SendExec( ioContext );

				} while ( length > 0 );

				memset( socketContext->wsaBuf, 0, socketContext->wsaOff );
				socketContext->wsaOff = 0;
			}

			socketContext = socketContext->next;
		}
	}
}

// SocketManager Method - 소켓(클라이언트) 관리.
void cGameProcess::SocketManager(DWORD currentTick)
{
	if ( mSocketContextPool != NULL )
	{
		cCSLock           lock( &mCs );
		PerSocketContext* socketContext = mSocketContextPool->GetPagedPoolUsage( );
		PerSocketContext* next          = NULL;

		ClientInfo*       clientInfo    = NULL;
		char*             recvBuf       = NULL;

		cPlayer*          hero          = NULL;
		unsigned long     heroIdx       = 0;

		while ( socketContext != NULL )
		{
			next       = socketContext->next;
			clientInfo = (ClientInfo*)socketContext->buffer;
			recvBuf    = (socketContext->buffer + socketContext->offset);

			// 중요1, 계정관리.
			if ( clientInfo->complete.check && clientInfo->delay.loginUpdate < currentTick )
			{
				HANDLE        handle      = NULL;
				LOGIN_UPDATE* loginUpdate = (LOGIN_UPDATE*)GetSQL( &handle, SQL_GAME_PROCESS_LOGIN_UPDATE );

				loginUpdate->loginIdx = clientInfo->loginIdx;

				SendSQL( socketContext, handle, sizeof(LOGIN_UPDATE) );
				clientInfo->delay.loginUpdate = currentTick + MAX_LOGIN_UPDATE;
			}

			// 중요2, GameInDbCount 감소.
			if ( clientInfo->gameInDBCnt == true )
			{
				if ( clientInfo->gameInDatabase == 0 )
				{
					if ( mGameInDbCount > 0 ) { --mGameInDbCount; }
					clientInfo->gameInDBCnt = false;
				}
			}

			try {
				// Time To Live 검사.
				if ( socketContext->timeToLive < currentTick )
					Close( socketContext );

				// Receive 데이터 처리를 위한 코드.
				if ( socketContext->InternalHigh )
				{
					u_long& offset = socketContext->Internal;
					u_long& length = socketContext->InternalHigh;

					if ( socketContext->status.connectionDead == false )
					{
						do {
							Packet* packet = (Packet*)(recvBuf + offset);

							if ( BatchComplete( currentTick, socketContext, packet ) == false )
							{
								Close( socketContext );
								break;
							}

							offset += packet->tlen;
						} while ( offset < length );
					}

					memset( recvBuf, 0, length );
					offset = 0;
					length = 0;
				}
			} catch ( ... ) {
				PostServerEvent( "In BatchComplete Method - MsgRoot(=%d,%d,%d). Throwing 'cGameProcess::SocketManager' exception.",
								 mLastCategory,
								 mLastProtocol,
								 mLastPacketSize );
				Sleep( 100 );
				throw;
			}

			// SocketContext 처리.
			if ( socketContext->status.connectionDead == false )
			{
				/*-- Game 처리. --*/
				heroIdx = clientInfo->characterIdx;
				hero    = mpObjectManager->GetPlayer( heroIdx );

				if( g_BillType == 2 )
				{
					if( clientInfo->complete.pgBilling == true && clientInfo->mBPNextCheckTime < GetAccumTime() )
					{
						clientInfo->mBPNextCheckTime = GetAccumTime() + 10 * SECOND;
						PGBILLPAYLATTER->UserAlive( clientInfo->memberIdx );
					}
				}
/*
				if ( (clientInfo->mHSClientHandle != ANTICPX_INVALID_HANDLE_VALUE) && (clientInfo->mHSIsSkip == false) )
				{
					unsigned long accumTime = GetAccumTime( );

					/// 핵쉴드 주기적으로 확인
					if ( clientInfo->mHSIsWait == false && clientInfo->mHSNextCheckTime <= accumTime )
					{
						if( mHackShieldPause == true )
						{
							/// 서버툴에서 핵쉴드 체크를 정지 시킨경우 다음 체크시간만 설정한다
							clientInfo->mHSNextCheckTime = GetAccumTime() + mHackShieldCheckTime;
						}
						else
						{
							// -----------------------------------------------------------
							// [_AhnHS_MakeRequest ]
							// -----------------------------------------------------------
							// - DESCRIPTION:
							//    현재 세션에 맞는 클라이언트 핸들을 입력하여 요청 메시지를 생성합니다. 
							//    요청 메시지는 AHNHS_TRANS_BUFFER 구조체 형태로 출력되며, 
							//    멤버 변수 값은 다음과 같습니다.
							//    typedef struct _AHNHS_TRANS_BUFFER
							//    {
							//      	unsigned short nLength;			// 요청 메시지 생성에 사용된 버퍼 길이
							//          unsigned char byBuffer[ANTICPX_TRANS_BUFFER_MAX]; // 요청 메시지 생성에 사용될 수 있는 최대 바이트 버퍼
							//    } AHNHS_TRANS_BUFFER, *PAHNHS_TRANS_BUFFER;
							//
							//  ※주의:
							//    byBuffer는 요청 메시지 생성에 사용될 수 있는 최대 버퍼 크기이므로 네트워크로 전송할 때 nLength 
							//    만큼만 전송해야 합니다.
							//
							// - SYNTAX:
							//   unsigned long __stdcall _AhnHS_MakeRequest (IN AHNHS_CLIENT_HANDLE hClient, 
							//												OUT PAHNHS_TRANS_BUFFER pRequestBuffer);
							// - PARAMETERS:
							//   .hClient			:AHNHS_CLIENT_HANDLE	클라이언트 핸들
							//   .pRequestBuffer	:PAHNHS_TRANS_BUFFER	보낼 데이터 버퍼/길이
							// 
							// - RETURN VALUE:
							//   . ERROR_SUCCESS:
							//	   함수 호출을 성공했을 때 리턴하는 값입니다. (Value = 0x00000000)
							//   . 기타				: 기타 리턴값에 대해서는 핵쉴드 프로그래밍 가이드 참고바람.
							AHNHS_TRANS_BUFFER transBuffer;
							unsigned long      ulRet;

							ulRet = _AhnHS_MakeRequest( clientInfo->mHSClientHandle, &transBuffer );

							if ( transBuffer.nLength > ANTICPX_TRANS_BUFFER_MAX )
							{
								/// ahnhs 함수에서 메모리사이즈가 비정상적으로 처리된경우 핵쉴드 체크 중지
								clientInfo->mHSNextCheckTime = ULONG_MAX;
								clientInfo->mHSIsWait        = true;
								PostServerEvent( "cGameProcess::SocketManager transBuffer.nLength > ANTICPX_TRANS_BUFFER_MAX[%d,%d,%d,%x]"
												,clientInfo->memberIdx
												,transBuffer.nLength
												,ANTICPX_TRANS_BUFFER_MAX
												,ulRet );
							}
							else
							{
								switch( ulRet )
								{
								case ERROR_SUCCESS:
									// 함수 호출을 성공했을 때 리턴하는 값입니다.
									{
										/// 클라이언트에서 답변이 오기까지 대기시간 설정
										clientInfo->mHSNextCheckTime = accumTime + mHackShieldWaitTime;
										clientInfo->mHSIsWait        = true;

										HANDLE              handle  = NULL;
										MSG_SYN_HACKSHIELD* sendMsg = (MSG_SYN_HACKSHIELD*)GetHackShieldMsgRoot( &handle, socketContext, NM_HACKSHIELD, NM_HACKSHIELD_SYN );

										memcpy( sendMsg->byBuffer, transBuffer.byBuffer, transBuffer.nLength );
										sendMsg->nLength = transBuffer.nLength;

										SendMsgRoot( handle, sendMsg->GetMsgLength( ) );
										Verbose->WriteLog( "_AhnHS_MakeRequest H[%u]Size[%d]Ret[%d]", clientInfo->mHSClientHandle, sendMsg->nLength, ulRet );
									}
									break;
								case ERROR_ANTICPXSVR_INVALID_PARAMETER:
									//설 명 : 입력 값이 올바르지 않습니다.
									//원 인 : hClient, pRequestBuffer 값이 NULL 인 경우 발생합니다.
									//확인사항 : hClient, pRequestBuffer 값이 NULL 이 아닌지 확인합니다.
								case ERROR_ANTICPXSVR_BAD_FORMAT:
									//설 명 : HSB 파일을 읽는데 문제가 발생하였습니다.
									//원 인 : HSB 파일이 정상적으로 생성이 되지 않은 경우 발생합니다.
									//확인사항 : HSBGen.exe Tool 을 최신 버전으로 사용하였는지 확인해 보고 다시 HSB 파일을 생성해 봅니다.
								case ERROR_ANTICPXSVR_NOT_YET_RECEIVED_RESPONSE:
									//설 명 : 이전 요청 메시지에 대한 응답을 받지 않았습니다
									//원 인 : _AhnHS_MakeRequest 함수를 호출하면 클라이언트로부터 Ack 메시지를 받아서 
									//			_AhnHS_VerifyResponse 함수를 거쳐서 다시 호출 되어야 합니다. 이 동기화 과정이 
									//			정상적으로 이루어지지 않으면 발생 합니다.
									//확인사항 : 클라언트 세션 관리상의 동기화 문제가 발생하지 않았는지 확인해 봅니다.
								case ERROR_ANTICPXSVR_NOT_ENOUGH_MEMORY:
									//설 명 : 메모리가 부족합니다.
									//원 인 : 서버의 메모리가 부족하여 발생된 에러입니다.
									//확인사항 : 서버에서 메모리 leak 현상이 발생하는지 확인해 보아야 합니다.
								case ERROR_ANTICPXSVR_BAD_MESSAGE:
									//설 명 : 버퍼 암호화에 실패하였습니다.
									//원 인 : 요청 보낼 버퍼를 암호화 하는데 실패한 경우 발생합니다.
									//확인사항 : 위 에러가 발생한다면 구조적인 문제일 수 있으므로 안철수 연구소 기술지원에 
									//				문의하시기 바립니다.
								case ERROR_ANTICPXSVR_MAKEREQ_EXCEPTION:
									//설 명 : _AhnHS_MakeRequest 함수 수행 중 예외(Exception)가 발생하였습니다. 
									//			(본 에러는 Windwos용 모듈에서만 반환됩니다)
									//원 인 : 여러 가지 원인에 의해서 예외가 발생할 수 있습니다.
									//확인사항 : 예외가 발생한 경우, 내부적으로 MiniDump 파일을 남기도록 되어 있습니다. 
									//				"MAKEREQUEST_년_월_일 시-분-초.dmp" 의 파일명 형식으로 덤프 파일이 생성되므로 
									//				해당 파일을 ㈜안철수연구소 로 전달해 주시기 바랍니다.
									clientInfo->mHSNextCheckTime = ULONG_MAX;
									clientInfo->mHSIsWait        = false;
									PostServerEvent("cGameProcess::SocketManager ulRet != ERROR_SUCCESS[%d,%u,%x]", clientInfo->memberIdx, ulRet, ulRet );
									break;
								default:
									clientInfo->mHSNextCheckTime = ULONG_MAX;
									clientInfo->mHSIsWait        = false;
									PostServerEvent("cGameProcess::SocketManager default ulRet != ERROR_SUCCESS[%d,%d,%x]", clientInfo->memberIdx, ulRet, ulRet );
									break;
								}
							}
						}
					}
					/// 지정 시간까지 응답을 안 준경우 클라이언트 종료
					else if ( clientInfo->mHSIsWait == true && clientInfo->mHSNextCheckTime <= accumTime )
					{
						PostServerEvent("cGameProcess::SocketManager clientInfo->mHSWaitEndTime <= GetAccumTime()[%d,%d,%d]", clientInfo->memberIdx, clientInfo->mHSIsWait == true ? 1 : 0, accumTime );
						clientInfo->mHSNextCheckTime = ULONG_MAX;
						clientInfo->mHSIsWait = false;
						Close( socketContext );
					}
				}
*/
				/*-- Game In/Out 전용 처리. --*/
				if ( clientInfo->complete.gameIn )
				{
					/*-- 인벤토리(착용정보) 처리 - Select / Move / Switch / Item Bill.
						 퀘 스 트(업데이트) 처리 - Reward.
					*/
					if ( clientInfo->complete.inventory )
					{
						if ( clientInfo->request.characterEquip )
						{
							clientInfo->request.characterEquip  = false;

							// 캐릭터 장비 아이템 정보 갱신.
							hero->EquipItems( );
							STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );
						}
					}
					// 친구 목록 전송
					if ( hero->GetFriendSend( ) == true )
					{
						hero->SendFriendList( (ULONG_PTR)socketContext );
						hero->SetFriendSend( false );
					}
					// 맵이동 처리
					if ( hero->IsMapChange( ) && !hero->InventoryDatabase( ) && !clientInfo->commonDatabase && !clientInfo->skillDatabase )
					{
						if ( mpGridManager->RemovePlayer( hero ) == true )
						{
							// 맵이동 응답 메시지 - 성공.
							clientInfo->complete.gameIn = false;
							hero->ApplyMapTargetPos( );				// 캐릭터 맵 이동 적용.
							clientInfo->gameInDBEnd = true;

							/*-- 1. 캐릭터정보(재전송). -- */

							// 캐릭터 최신정보(Character Select) - 080908 PKH 
							hero->SendPlayerInfo( NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, (ULONG_PTR)socketContext, (unsigned char)mType );
							hero->SendMoney( hero->GetObject() );
							hero->SendDeposit( hero->GetObject( ) );
			
							clientInfo->request.characterOptionSend = true;		// Character Select.

							// 캐릭터 관련-DB 검색.
							clientInfo->request.inventorySend         = true;	// InventorySelect( socketContext, heroIdx );
							clientInfo->request.characterEquip        = true;	// Sub InventorySelect
							clientInfo->request.inventoryCooltimeSend = true;	// InventoryCooltime( socketContext, heroIdx );
							clientInfo->request.shortcutSend          = true;	// ShortcutSelect( socketContext, heroIdx );
							clientInfo->request.questSend             = true;	// QuestSelect( socketContext, heroIdx );
							clientInfo->request.titleSend             = true;	// TitleSelect( socketContext, heroIdx );

							SKILLMANAGER->SendMsgSkillHaveToPlayer( socketContext->cid, heroIdx );	// SkillSelect( socketContext, heroIdx );
							SKILLMANAGER->SendMsgInfluenceToPlayer( socketContext->cid, heroIdx );	// SkillInfluenceSelect( socketContext, heroIdx );
							/// 레시피 보유목록 발송
							hero->SendMsgRecipeList();

							/*-- 2. LOAD DATA(재전송). --*/

							// MONSTER & NPC MODEL.
							mpObjectManager->SendMsgMonsterModelNumberToPlayer( (ULONG_PTR)socketContext, hero->GetMapNumber( ) );
							mpObjectManager->SendMsgNpcModelNumberToPlayer( (ULONG_PTR)socketContext, hero->GetMapNumber( ) );

							/*-- 3. 캐릭터 상태설정 --*/

							// 대기상태로 변경

							if ( hero->ChangeState( eOBJECT_STATE_IDLE ) == false )
							{
								PostServerEvent( "hero[%d,%d,%d]->MapChange( true ) - ChangeState( eOBJECT_STATE_IDLE ) = false",
												 heroIdx,
												 hero->GetState(),
												 hero->GetStateStop() );
							}
							// 평화 대기 풀림
							hero->SetStateIdle( eIDLE_NORMAL );
						}
						else
						{
							// 맵이동 응답 메시지 - 실패.
							SendMsgError( socketContext, NM_PLAYER, NM_PLAYER_MAPCHANGE_RES, 1 );
						}

						hero->MapChange( false );

						hero->ChangeState( eOBJECT_STATE_IDLE );
						hero->SetStateStop( eSTOP_NONE );
					}
				}
				else
				{
					// 채널리스트.
					if ( clientInfo->request.channelList && clientInfo->delay.channelList < currentTick )
					{
						clientInfo->request.channelList = !SendChannelList( socketContext );
					}
					// 캐릭터리스트.
					if ( clientInfo->request.characterList && clientInfo->delay.characterList < currentTick )
					{
						clientInfo->commonDb.characterList = CharacterList( socketContext, clientInfo->memberIdx );
						clientInfo->request.characterList = !clientInfo->commonDb.characterList;
					}
					// 캐릭터 선택.
					if ( clientInfo->request.characterSelect  == true
					  && clientInfo->commonDb.characterSelect == false
					  && mCharacterSelectCount                <  MAX_CHARACTER_SELECT
					  && mGameInDbCount                       <  MAX_GAME_IN_DB )
					{
						if ( clientInfo->complete.gameToGame )
						{
							PostServerEvent( "CriticalError - Already Complete::gameToGame" );
						}
						else if ( !clientInfo->complete.characterSelect )
						{
							// DATABASE 요청.
							HANDLE            handle          = NULL;
							CHARACTER_SELECT* characterSelect = (CHARACTER_SELECT*)GetSQL( &handle, SQL_GAME_PROCESS_CHARACTER_SELECT );

							characterSelect->characterIdx = clientInfo->characterIdx;
							characterSelect->userIdx      = clientInfo->memberIdx;

							clientInfo->commonDb.characterSelect = SendSQL( socketContext, handle, sizeof(CHARACTER_SELECT) );

							// 캐릭터 선택 카운트 증가.
							if ( clientInfo->commonDb.characterSelect == true ) { ++mCharacterSelectCount; }
						}
						clientInfo->request.characterSelect = false;
					}
					/// gameInDatabase 처리가 다끝난후 gameInDBEnd 처리를 한다.
					if ( clientInfo->gameInDatabase == 0 && clientInfo->gameInDBEnd == true )
					{
						/// 효과중 hero정보 발송할때 같이 해야할 적용항목 먼저 계산
						hero->GameInInfApply();

						/// db 처리가 끝나고 난후 스텟계산함
						STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );

						// hero 정보 발송 - 클라이언트에 스테이지를 변경하라고 알림
						hero->SendHeroInfo( );
						clientInfo->gameInDBEnd = false;
					}
					// 게임-인 처리.
					if ( clientInfo->request.gameIn == true && clientInfo->gameInDBEnd == false )
					{
						clientInfo->request.gameIn = false;

						if ( mpGridManager->GetPlayer( heroIdx ) == NULL )
						{
							// 게임인 응답 메시지 - 성공.
							HANDLE          handle  = NULL;
							MSG_RES_GAMEIN* sendMsg = (MSG_RES_GAMEIN*)GetMsgRoot( &handle, socketContext, NM_USER, NM_USER_GAMEIN_RES );
								sendMsg->ErrorCode  = 0;
								sendMsg->channelNum = (unsigned short)mChannelNum;
							SendMsgRoot( handle, sizeof(MSG_RES_GAMEIN) );

							if ( mpGridManager->AddPlayer( hero ) == true )
							{
								// gameIn on.
								clientInfo->complete.gameIn               = true;
								clientInfo->complete.characterUpdate      = true;
								clientInfo->complete.skillInfluenceUpdate = true;
								clientInfo->complete.skillCoolTime        = true;
								clientInfo->complete.recipeCoolTime       = true;

								/*-- 인벤토리(착용정보) 처리 - Select / Move / Switch / Item Bill.
									 퀘 스 트(업데이트) 처리 - Reward.
								*/
								if ( clientInfo->complete.inventory )
								{
									if ( clientInfo->request.characterEquip )
									{
										clientInfo->request.characterEquip  = false;

										// 캐릭터 장비 아이템 정보 갱신.
										hero->EquipItems( );
										STATUSCALC->CalcPlayerExtensionGlobal( hero->GetObject() );

										/// 로그인시 스텟 계산후에 db에서 읽어왔던 hp/mp값을 적용
										hero->ApplyInitRest();
									}
								}

								hero->SetFriendUseDB( FriendSelect( heroIdx ) );

								// 첫 접속시 처리
								if( clientInfo->complete.firstGameIn )
								{
									// 길드
									unsigned long guildIndex = hero->GetGuildIndex();
									if ( guildIndex > 0 )
									{
										mpGuildManager->SendGuildUserConnect( guildIndex, heroIdx, 1, mChannelNum, true );
										mpGuildManager->PostGuildUserConnect( guildIndex, heroIdx, 1, NETWORK2->GetChannelNum(), true );
									}

									// 친구
									hero->SendFriendConnect( 1 );
									hero->PostFriendConnect( 1 );
									
									clientInfo->complete.firstGameIn = false;
								}

								hero->SendThemeInfo();

								switch( mType )
								{
								case _E_ST_NORMAL_MAP_:
									{
										/// 레벨 구간을 가져온다
										unsigned long aryPos = PVPMANAGER->GetSectionAryPos( hero->GetLevel() );

										if( PVP_DM_LEVEL_SECTION_MAX > aryPos )
										{
											if( mPVPData[aryPos].timer != 0 && mPVPData[aryPos].timer - mAccumTime  > 0 && hero->GetForceType() != eFORCETYPE_NONE )
											{
												if( hero->CheckPvPJoin() == true )
												{
													sDMInfo* pDMInfo = PVPSCRIPT->GetDmInfo( mPVPData[aryPos].pvpType );
													if( pDMInfo != NULL )
													{
														sDMBlockInfo* pBlockInfo = pDMInfo->GetBlockInfo( hero->GetLevel() );
														if( pBlockInfo != NULL )
														{
															/// pvp 참여 중이 아닌경우 참여 여부를 물어봄
															HANDLE          handle  = NULL;
															MSG_SYN_PVP_READYQUESTION* sendMsg = (MSG_SYN_PVP_READYQUESTION*)GetMsgRoot( &handle, socketContext, NM_PVP, NM_PVP_READYQUESTION_SYN );
															sendMsg->mLeftTime = mPVPData[aryPos].timer - mAccumTime;
															sendMsg->mMinLevel = pBlockInfo->mLevelMin;
															sendMsg->mMaxLevel = pBlockInfo->mLevelMax;
															sendMsg->mPvPType = mPVPData[aryPos].pvpType;
															SendMsgRoot( handle, sizeof(MSG_SYN_PVP_READYQUESTION) );
														}
													}
												}
											}
										}
									}
									break;
								case _E_ST_ID_THEME_:
									{
										/// 테마 관리정보 전송
										hero->SendThemeInfo();

										/// 테마 초기화까지 남는 시간 보내주기.
 										hero->SendThemeExpireTime();

										cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( hero->GetThemeRoomIdx() );
										if ( pTheme != NULL )
										{
											pTheme->SendObstruction( hero );
											pTheme->SendTeleportPortal( hero );
										}
										else
											Close( socketContext );
									}
									break;
								case _E_ST_ID_TUTORIAL_:
									{
										cThemeObject* pTheme = THEMEMANAGER->GetThemeObject( hero->GetThemeRoomIdx() );
										if ( pTheme != NULL )
										{
											pTheme->SendObstruction( hero );
											pTheme->SendTeleportPortal( hero );
										}
										else
											Close( socketContext );
									}
									break;
								case _E_ST_ID_PVP_:
									{
										cBaseDeathMatch* pDMObject = PVPMANAGER->GetPvPDMObject( hero->GetPvPDMIdx() );
										if( pDMObject != NULL )
										{
											/// 접속한 플레이어에게 자신의 팀원 정보 발송
											pDMObject->SendTeamList( hero );

											/// 장애물 목록 발송
											pDMObject->SendObstruction( hero );

											/// 진행상태 && 남은 시간 발송
											pDMObject->SendReadyLeftTime( hero );

											///// 인원 부족시 알림창 발송
											//if( pDMObject->GetProcess() == ePVPDM_PROCESS_READY_OUT )
											//{
											//	MSGROOT* sendMsg = (MSGROOT*)GetMsgRoot( &handle, (PerSocketContext*)socketContext );
											//	if ( sendMsg != NULL )
											//	{
											//		sendMsg->Category = NM_PVP;
											//		sendMsg->Protocol = NM_PVP_DM_NOTREADY_SYN;
											//		SendMsgRoot( handle, sizeof(MSGROOT) );
											//	}

											//}

											/// 파티연합 추가
											unsigned long partyUnionIndex = pDMObject->GetPartyUnionIndex( (ePVPDM_TEAM_TYPE)hero->GetPvPDMTeam() );

											// 연합 없으면 생성
											if( mpPartyUnionManager->GetUnion( partyUnionIndex ) == NULL )
											{
												partyUnionIndex = mpPartyUnionManager->CreateUnion();

												if( mpPartyUnionManager->AddUser( partyUnionIndex, hero->GetObjectID() ) == true )
												{
													pDMObject->SetPartyUnionIndex( (ePVPDM_TEAM_TYPE)hero->GetPvPDMTeam(), partyUnionIndex );				
												}
												else
												{
													PostServerEvent( "mpPartyUnionManager::AddUser : failed to find union(%d, %d)", partyUnionIndex, hero->GetObjectID() );
												}
											}
											else
											{
												// 있으면 기존에 추가
												if( mpPartyUnionManager->AddUser( partyUnionIndex, hero->GetObjectID() ) == false )
												{
													PostServerEvent( "mpPartyUnionManager::AddUser : failed to find union(%d, %d)", partyUnionIndex, hero->GetObjectID() );
												}
											}
										}
									}
									break;
								}

								if( hero->IsMapNum0Save() == true )
								{
									hero->DBUpdate( false, PLAYER_DBUPDATE_LOGIN );
									hero->SetMapNum0Save( false );
								}
							}
						}
						else
						{
							// 게임인 응답 메시지 - 실패.
							SendMsgError( socketContext, NM_USER, NM_USER_GAMEIN_RES, 1 );
						}
					}
					if ( clientInfo->complete.characterSelect )
					{
						// 맵이동 처리 - 인던, 맵.
						if ( clientInfo->request.joinInstantDungeon || clientInfo->request.joinMap )
						{
							if ( hero->IsMapChange( ) && !hero->InventoryDatabase( ) && !clientInfo->commonDatabase && !clientInfo->skillDatabase )
							{
								clientInfo->gameInDBEnd = true;

								/*-- 1. 캐릭터정보(재전송). -- */

								/*-- 2. LOAD DATA(재전송). --*/

								// MONSTER & NPC MODEL.
								mpObjectManager->SendMsgMonsterModelNumberToPlayer( (ULONG_PTR)socketContext, hero->GetMapNumber( ) );
								mpObjectManager->SendMsgNpcModelNumberToPlayer( (ULONG_PTR)socketContext, hero->GetMapNumber( ) );

								/*-- 3. 캐릭터 상태설정 --*/

								// 대기상태로 변경

								if ( hero->ChangeState( eOBJECT_STATE_IDLE ) == false )
								{
									PostServerEvent( "hero[%d,%d,%d]->MapChange( true ) - ChangeState( eOBJECT_STATE_IDLE ) = false",
										heroIdx,
										hero->GetState(),
										hero->GetStateStop() );
								}
								// 평화 대기 풀림
								hero->SetStateIdle( eIDLE_NORMAL );
								hero->MapChange( false );

								clientInfo->request.joinInstantDungeon = false;
								clientInfo->request.joinMap = false;
							}
						}
					}
				}
				// 게임아웃
				if ( clientInfo->request.gameOut )
				{
					bool retvalue = false;

					try {
						retvalue = UpdateComplete( socketContext );
					} catch ( ... ) {
						PostServerEvent( "In cGameProcess::UpdateComplete Method. Throwing 'cGameProcess::SocketManager - Gameout' exception." );
						throw;
					}
					if ( retvalue == true )
					{
						SendMsgError( socketContext, NM_PLAYER, NM_PLAYER_GAMEOUT_RES, ERROR_GAMEFINISH_SUCCESS );

						// 게임서버 인증 백업.
						SecureInfo secure;
						memcpy( &secure, clientInfo->session, sizeof(SecureInfo) );

						memset( clientInfo, 0, sizeof(ClientInfo) );

						clientInfo->complete.check = true;
						memcpy( clientInfo->session, &secure, sizeof(SecureInfo) );
					}
				}
				// UpdateComplete 인던이동
				if ( clientInfo->request.gotoInstantDungeon || clientInfo->request.returnToMap )
				{
					try {
						if ( UpdateComplete( socketContext ) == true )
						{
							/* 전장참여 이벤트 기록. */
							if ( clientInfo->request.returnToMap )
							{
								switch ( mChannelNum )
								{
								case MAP_DEATHMATCH:
									PostReservedEvent( EVENT_RESERVED_PVP_01, clientInfo->characterIdx, clientInfo->memberIdx );
									break;
								case MAP_FLAG:
									PostReservedEvent( EVENT_RESERVED_PVP_02, clientInfo->characterIdx, clientInfo->memberIdx );
									break;
								}
							}

							/* Reset Client Information.(클라이언트 정보 재설정) */

							// 1. 클라이언트 정보 백업(서버인증).
							SecureInfo secure;
							memcpy( &secure, clientInfo->session, sizeof(SecureInfo) );

							// 2. 클라이언트 정보 삭제.
							memset( clientInfo, 0, sizeof(ClientInfo) );

							// 3. 클라이언트 정보 복구.
							clientInfo->complete.check = true;
							memcpy( clientInfo->session, &secure, sizeof(SecureInfo) );

							// 4. 클라이언트 서버 이동(채널간 이동).
							HANDLE        handle     = NULL;
							GAME_TO_GAME* gameToGame = (GAME_TO_GAME*)GetSQL( &handle, SQL_GAME_PROCESS_GAME_TO_GAME );

							gameToGame->loginIdx       = clientInfo->loginIdx;
							gameToGame->serverNum      = HIWORD( clientInfo->gotoCID );
							gameToGame->channelNum     = LOWORD( clientInfo->gotoCID );
							gameToGame->lastChannel    = mChannelNum;
							gameToGame->characterIdx   = clientInfo->characterIdx;
							gameToGame->instantDungeon = 1;

							clientInfo->commonDb.gameToGame = SendSQL( socketContext, handle, sizeof(GAME_TO_GAME) );
							clientInfo->complete.gameToGame = clientInfo->commonDb.gameToGame;
						}
					} catch ( ... ) {
						PostServerEvent( "In cGameProcess::UpdateComplete Method. Throwing 'cGameProcess::SocketManager - GameToGame' exception." );
						throw;
					}
				}
				// 캐릭터 관리.
				if ( clientInfo->complete.characterSelect && clientInfo->delay.characterTTL < currentTick )
				{
					CharacterTTL( clientInfo->characterIdx, MAX_CHARACTER_TTL, mChannelNum );
					clientInfo->delay.characterTTL = currentTick + MAX_CHARACTER_TTL;
				}
				// 인벤토리 발송.
				if ( clientInfo->complete.inventory && clientInfo->request.inventorySend )
				{
					hero->SendInventory( (ULONG_PTR)socketContext );
					clientInfo->request.inventorySend = false;
				}
				// 인벤토리 쿨타임 발송.
				if ( clientInfo->complete.inventoryCooltime && clientInfo->request.inventoryCooltimeSend )
				{
					hero->SendInventoryCooltime( (ULONG_PTR)socketContext );
					clientInfo->request.inventoryCooltimeSend = false;
				}
				// 퀘스트 발송 : 퀘스트관련 작업이 가능하고, 요청이 있으면 발송
				if( clientInfo->complete.quest && clientInfo->request.questSend )
				{
					hero->SendQuest( (ULONG_PTR)socketContext );
					hero->SendNewQuestList();
					clientInfo->request.questSend = false;
				}
				// 호칭 발송
				if( clientInfo->complete.title && clientInfo->request.titleSend )
				{
					hero->SendHaveTitleList( (ULONG_PTR)socketContext );
					clientInfo->request.titleSend = false;
				}
				// 옵션 발송.
				if ( clientInfo->complete.characterOption && clientInfo->request.characterOptionSend )
				{
					HANDLE                       handle  = NULL;
					MSG_RES_PLAYER_OPTION_VALUE* sendMsg = (MSG_RES_PLAYER_OPTION_VALUE*)GetMsgRoot( &handle, socketContext, NM_PLAYER, NM_PLAYER_OPTION_VALUE_RES );
					sendMsg->option1 = clientInfo->optionData1;
					SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_OPTION_VALUE) );
					clientInfo->request.characterOptionSend = false;
				}
				// 단축창 발송.
				if ( clientInfo->complete.shortcut && clientInfo->request.shortcutSend )
				{
					HANDLE                        handle  = NULL;
					MSG_RES_PLAYER_SHORTCUT_LIST* sendMsg = (MSG_RES_PLAYER_SHORTCUT_LIST*)GetMsgRoot( &handle, socketContext, NM_PLAYER, NM_PLAYER_SHORTCUT_LIST_RES );
					memcpy( sendMsg->mShortcut, clientInfo->shortcut.binary, sizeof(sendMsg->mShortcut) );
					SendMsgRoot( handle, sizeof(MSG_RES_PLAYER_SHORTCUT_LIST) );
					clientInfo->request.shortcutSend = false;
				}
				// 친구목록 발송.
				if ( clientInfo->complete.characterSelect && clientInfo->delay.firendSend < currentTick )
				{
					hero->SetFriendUseDB( FriendSelect( heroIdx ) );
					clientInfo->delay.firendSend = currentTick + ( 5 * 60 * 1000 );			// 다음검사 시간. 5분
				}
				// 퀘스트db 저장
				if ( clientInfo->complete.quest && currentTick > clientInfo->delay.questSave )
				{
					hero->SaveQuestProgress( (ULONG_PTR)socketContext );		// 퀘스트 진행정보 저장.
					clientInfo->delay.questSave = currentTick + QUEST_SAVETIME;	// 다음검사 시간.
				}
			}
			else
			{
				try {
					socketContext->status.closeSocket = UpdateComplete( socketContext, true );
				} catch ( ... ) {
					PostServerEvent( "In cGameProcess::UpdateComplete Method. Throwing 'cGameProcess::SocketManager - CloseSocket' exception." );
					throw;
				}
			}

			// SocketContext 회수.
			if ( socketContext->status.closeSocket )
			{
				// ----------------------------------------------------------
				// [_AhnHS_CloseClientHandle]
				// ----------------------------------------------------------
				// - DESCRIPTION:
				//   생성한 클라이언트 핸들은 클라이언트 세션이 종료될 때 해제되어야 합니다. 
				//   이때 클라이언트 핸들 생성에 사용되었던 메모리나 시스템 자원을 해제하게 됩니다.
				//
				// - SYNTAX:
				//	 void __stdcall_AhnHS_CloseClientHandle (IN AHNHS_CLIENT_HANDLE hClient)
				//
				// - PARAMETERS:
				//   hClient	:AHNHS_CLIENT_HANDLE	클라이언트 핸들(Client Handle)
				//
				// - RETURN VALUE:
				//   없음.
				//
				// - ※주의 : 클라이언트 핸들이 모두 해제된 이후 서버핸들이 해제되어야 한다 
//				if ( clientInfo->mHSClientHandle != ANTICPX_INVALID_HANDLE_VALUE )
//				{
//					Verbose->WriteLog("_AhnHS_CloseClientHandle H[%u]", clientInfo->mHSClientHandle );
//					_AhnHS_CloseClientHandle( clientInfo->mHSClientHandle );
//				}
				Verbose->CloseSocket( &mCs, socketContext );
				mSocketContextPool->ReleasePerSocketContext( socketContext );
			}
			// SocketContext 링크.
			socketContext = next;
		} // while ( socketContext != NULL )

		/*-- 캐릭터 IN/OUT 지연 - 자동해제 --*/
		if ( mReleaseCountTick < currentTick )
		{
			if ( mCharacterSelectCount > 0 ) { --mCharacterSelectCount; }
			if ( mGameInDbCount        > 0 ) { --mGameInDbCount;        }
			if ( mGameOutDbCount       > 0 ) { --mGameOutDbCount;       }

			mReleaseCountTick = currentTick + MAX_RELEASE_COUNT;
		}
	}
}

// ChannelManager Method - 채널 관리 - 서버상태 발송.
void cGameProcess::ChannelManager(DWORD currentTick)
{
	cSender* sender = g_gameSrv->GetSender( );
	if ( mStatusSyncTick < currentTick )
	{
		long cid = MAKECID((WORD)mServerNum, (WORD)mChannelNum);

		if ( mComplete.channelSyn )
		{
			SIZE_T quotaPagedPoolUsage = 0
				  ,quotaNonPagedPoolUsage = 0
				  ,workingSetSize = 0;

			mSocketContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );

			if ( quotaPagedPoolUsage >= CHANNEL_CCU_S4 )
			{
				mStatus = _E_STATUS_RUNNING_S4_;
			}
			else if ( quotaPagedPoolUsage >= CHANNEL_CCU_S3 )
			{
				mStatus = _E_STATUS_RUNNING_S3_;
			}
			else if ( quotaPagedPoolUsage >= CHANNEL_CCU_S2 )
			{
				mStatus = _E_STATUS_RUNNING_S2_;
			}
			else /*if ( quotaPagedPoolUsage >= CHANNEL_CCU_S1 )*/
			{
				mStatus = _E_STATUS_RUNNING_S1_;
			}
		}

		sender->PostChStatus( cid, (BYTE)mStatus );

		mStatusSyncTick = (currentTick + STATUS_SYNC_TIME);
	}
}

// GameManager Method
void cGameProcess::GameManager(DWORD currentTick)
{
	cCSLock lock( &mCs );

	mElapsedTime = TIMER->GetDeltaTime();
	mAccumTime  += mElapsedTime;

	// cDrop::ProcessDrop
	try {
		mpDrop->ProcessDrop( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In cDrop::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cObjectManager::Process
	try {
		mpObjectManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In cObjectManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cAIManager::Process
	try {
		mpAIManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In cAIManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cSkillManager::Process
	try {
		mpSkillManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In cSkillManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cDuelManager::Process
	try {
		mpDuelManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In cDuelManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cPVPManager::Process, cThemeManager::Process
	switch ( mType )
	{
	case _E_ST_ID_PVP_:
		try {
			mpPVPManager->Process( mElapsedTime, mAccumTime );
		} catch ( ... ) {
			PostServerEvent( "In cPVPManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
			throw;
		}
		break;
	case _E_ST_ID_THEME_:
	case _E_ST_ID_TUTORIAL_:
		try {
			mpThemeManager->Process( mElapsedTime, mAccumTime );
		} catch ( ... ) {
			PostServerEvent( "In cThemeManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
			throw;
		}
		break;
	}

	// cPartyManager::Process
	try {
		mpPartyManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In mpPartyManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cPartyUnionManager::Process
	try {
		mpPartyUnionManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In mpPartyUnionManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cGuildManager::Process
	try {
		mpGuildManager->Process( mElapsedTime, mAccumTime );
	} catch ( ... ) {
		PostServerEvent( "In mpGuildManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cQuestManager::Process
	try	{
		mpQuestManager->Process( mElapsedTime, mAccumTime );
	}
	catch ( ... )
	{
		PostServerEvent( "In mpQuestManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}

	// cGridManager::Process
	try {
		mpGridManager->Process( currentTick );
	} catch ( ... ) {
		PostServerEvent( "In cGridManager::Process Method. Throwing 'cGameProcess::GameManager' exception." );
		throw;
	}
}

// BackendThreadBegin Method
void cGameProcess::BackendThreadBegin(void)
{
	while ( true )
	{
		DWORD currTickCount = GetTickCount( );
		{
			cCSLock lock( &mCs );

			// 서버종료, Thread를 나간다.
			if ( mEndServer == true )
				break; // while ( true )

			if ( mComplete.startLog == false )
			{
				mComplete.startLog = PostServerEvent( "To start from the GameProcess '%s:%d'.", inet_ntoa( mAddr.sin_addr ), mPort );
			}
			else if ( !mComplete.importItemTables )
			{
				if ( !mRequest.importItemTables )
				{
					mRequestImport.itemDefine       = VerifyItemDefine( );
					mRequestImport.itemAbility      = VerifyItemAbility( );
					mRequestImport.itemCoolitme2    = VerifyItemCooltime2( );
					mRequestImport.itemLimit        = VerifyItemLimit( );
					mRequestImport.itemCard         = VerifyItemCard( );
					mRequestImport.itemCardSlot     = VerifyItemCardSlot( );
					mRequestImport.itemTarot        = VerifyItemTarot( );
					mRequestImport.itemEnhanced     = VerifyItemEnhanced( );
					mRequestImport.itemEnhancedRate = VerifyItemEnhancedRate( );
					mRequestImport.itemDisjoint     = VerifyItemDisjoint( );
					mRequestImport.itemChange       = VerifyItemChange( );
					mRequestImport.guild			= VerifyGuild( );
					mRequestImport.gmEvent			= VerifyGmEvent( );
					mRequestImport.itemDropLimit    = VerifyItemDropLimit( );

					mRequest.importItemTables = (mRequestImportData == 0x3FFF);
				}
				else
					mComplete.importItemTables = (mCompleteImportData == 0x3FFF);
			}
			else if ( mComplete.verifyDefaultItems == false )
			{
				if ( mRequest.verifyDefaultItems == false )
				{
					mRequest.verifyDefaultItems = VerifyDefaultItems( );
					if ( !mRequest.verifyDefaultItems )
					{
						PostServerEvent( "Default Items Table can not start the scan." );
										// Default Items Table 검사를 시작할 수 없습니다.
					}
				}
			}
			else if ( mComplete.channelCheck == false )
			{
				if ( mRequest.channelCheck == false )
				{
					mRequest.channelCheck = ChannelCheck( (short)mServerNum, (short)mChannelNum, mServerInetnum, inet_addr( mServerInetnum ), mPort );
					if ( !mRequest.channelCheck )							  
					{
						PostServerEvent( "You can not ask for the channel number of the server(=%d).", mServerNum );
																		// (=%d) 서버의 채널번호를 요청할 수 없습니다.
					}
				}
			}
			else if ( mComplete.initGameDb == false )
			{
				if ( mRequest.initGameDb == false )
				{
					mRequest.initGameDb = InitGameDb( );
					if ( !mRequest.initGameDb )
					{
						PostServerEvent( "Game Database initialization of the request can not be [%d-%d].", mServerNum, mChannelNum );
																				// [%d-%d]의 Game Database 초기화를 요청할 수 없습니다.
					}
				}
			}
			else if ( mComplete.initThemeDb == false )
			{
				switch ( mType )
				{
				default:
					mComplete.initThemeDb = true;
					break;

				case _E_ST_ID_THEME_:
					if ( mRequest.initThemeDb == false )
					{
						HANDLE        handle = NULL;
						THEME_CREATE* create = (THEME_CREATE*)GetSQL( &handle, SQL_GAME_PROCESS_THEME_CREATE );

						create->themeIdx = mInDunMapNumber;
						create->maxRoom  = INDUN_MAP_MAX;
						sThemeTime* pTime = THEMETIMESCRIPT->GetThemeTime( mInDunMapNumber );
						if( pTime != NULL )
						{
							create->fixTime  = pTime->mResetClockHour;
							create->varTime  = pTime->mResetIntervalHour;
						}
						else
						{
							create->fixTime  = 12;
							create->varTime  = 24;
						}
						mRequest.initThemeDb = SendSQL( handle, sizeof(THEME_CREATE) );
					}
					break;
				}
			}
			else if ( mComplete.channelSyn == false )
			{
				ChannelManager( currTickCount );
			}
            else if ( mComplete.captcha == false )
            {
                mRequest.captcha = CaptChaOffset( );
            }
			else
				break; // while ( true )
		}

		// I/O Context 처리.
		IoContextPresent( );

		// FPS 처리.
		BackendThreadFps( currTickCount );
	}
}

// BackendThreadEnd Method
void cGameProcess::BackendThreadEnd(void)
{
	// 1. 서버종료.
	if ( mSocketContextPool != NULL )
	{
		PerSocketContext* socketContext = NULL;
		PerSocketContext* next;

		PostServerEvent( "cGameProcess - Disconnect the client is connected to the server." );

		CSBlock( &mCs )
		{
			socketContext = mSocketContextPool->GetPagedPoolUsage( );
			while ( socketContext != NULL )
			{
				Close( socketContext );
				socketContext = socketContext->next;
			}
		}

		PostServerEvent( "cGameProcess - Disconnection has been completed." );

		PostServerEvent( "cGameProcess - Client's data is stored in the Database." );

		bool complete = false;
		while ( !complete )
		{
			CSBlock( &mCs )
			{
				socketContext = mSocketContextPool->GetPagedPoolUsage( );
				if ( socketContext != NULL )
				{
					do {
						next = socketContext->next;
						try {
							socketContext->status.closeSocket = UpdateComplete( socketContext, true, true );
						} catch ( ... ) {
							PostServerEvent( "In cGameProcess::UpdateComplete Method. Throwing 'cGameProcess::BackendThreadEnd' exception." );
							throw;
						}
						if ( socketContext->status.closeSocket )
						{
//							ClientInfo* clientInfo = (ClientInfo*)socketContext->buffer;
							// ----------------------------------------------------------
							// [_AhnHS_CloseClientHandle]
							// ----------------------------------------------------------
							// - DESCRIPTION:
							//   생성한 클라이언트 핸들은 클라이언트 세션이 종료될 때 해제되어야 합니다. 
							//   이때 클라이언트 핸들 생성에 사용되었던 메모리나 시스템 자원을 해제하게 됩니다.
							//
							// - SYNTAX:
							//	 void __stdcall_AhnHS_CloseClientHandle (IN AHNHS_CLIENT_HANDLE hClient)
							//
							// - PARAMETERS:
							//   hClient	:AHNHS_CLIENT_HANDLE	클라이언트 핸들(Client Handle)
							//
							// - RETURN VALUE:
							//   없음.
							//
							// - ※주의 : 클라이언트 핸들이 모두 해제된 이후 서버핸들이 해제되어야 한다 
//							if ( clientInfo->mHSClientHandle != ANTICPX_INVALID_HANDLE_VALUE )
//							{
//								_AhnHS_CloseClientHandle( clientInfo->mHSClientHandle );
//							}
							mSocketContextPool->ReleasePerSocketContext( socketContext );
						}
						socketContext = next;
					} while ( socketContext != NULL );
				}
				else
					complete = true;
			}
			BackendThreadFps( GetTickCount( ) );
		}

		PostServerEvent( "cGameProcess - Client's data storage has been completed." );
	}

	// 2. 서버종료.
	if ( mIoContextPool != NULL )
	{
		PostServerEvent( "cGameProcess - I/O is waiting for processing." );

		DWORD lastTick = GetTickCount( ) + 10000;
		while ( true )
		{
			if ( mIoContextPool->GetPagedPoolUsage( ) == NULL )
			{
				PostServerEvent( "cGameProcess - I/O processing is completed." );
				break;
			}
			else if ( GetTickCount( ) > lastTick )
			{
				PostServerEvent( "cGameProcess - I/O processing timeout." );
				break;
			}
			else
				BackendThreadFps( GetTickCount( ) );
		}
	}

	// 3. 서버종료.
	while ( true )
	{
		if ( mComplete.guildDb == false )
		{
			if ( mRequest.guildDb == false )
			{
				mpGuildManager->SaveAllDb( );
				mComplete.guildDb = true;
			}
		}
		else if ( mComplete.shutdownGameDb == false )
		{
			if ( mRequest.shutdownGameDb == false )
				mRequest.shutdownGameDb = ShutdownGameDb( );
		}
		else if ( mComplete.shutdownAccountDb == false )
		{
			if ( mRequest.shutdownAccountDb == false )
				mRequest.shutdownAccountDb = ShutdownAccountDb( );
		}
		else if ( mComplete.endLog == false )
		{
			SIZE_T quotaPagedPoolUsage;
			SIZE_T quotaNonePagedPoolUsage;
			SIZE_T workingSetSize;

			// SocketContextPool 메모리 사용량
			mSocketContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
			PostServerEvent( "cGameProcess - SocketContextPool Working Set Size (=%d), Quota Paged Pool Usage (=%d), Quota None Paged Pool Usage (=%d)."
							 ,workingSetSize
							 ,quotaPagedPoolUsage
							 ,quotaNonePagedPoolUsage );

			// IoContextPool 메모리 사용량
			mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonePagedPoolUsage, workingSetSize );
			PostServerEvent( "cGameProcess - IoContextPool Working Set Size (=%d), Quota Paged Pool Usage (=%d), Quota None Paged Pool Usage (=%d)."
							 ,workingSetSize
							 ,quotaPagedPoolUsage
							 ,quotaNonePagedPoolUsage );

			// IoContextPool Buffering 사용량
			PostServerEvent( "cGameProcess - IoContextPool BackBuffer Offset (=%d).", mIoContextBackBuffer->offset );
			PostServerEvent( "cGameProcess - IoContextPool FrontBuffer Offset (=%d).", mIoContextFrontBuffer->offset );

			// 지연시간.
			PostServerEvent( "cGameProcess - Min Elapsed Tick (=%d), Max Elapsed Tick (=%d).", mMinElapsedTick, mMaxElapsedTick );
			mComplete.endLog = true;
		}
		else if ( mEndServer == true )
		{
			Sleep( 100 );
			PostServerEvent( "cGameProcess - Shutdown has completed." );
			return;
		}

		// FPS 처리.
		BackendThreadFps( GetTickCount( ) );
	}
}

// BackendThreadProc Method
void cGameProcess::BackendThreadProc(void)
{
	DWORD currTickCount = 0;
	while ( true )
	{
		currTickCount = GetTickCount( );

		// 서버종료, Thread를 나간다.
		if ( mEndServer == true )
		{
			g_gameSrv->GetSender( )->PostChStatus( MAKECID((WORD)mServerNum, (WORD)mChannelNum), (BYTE)_E_STATUS_CLOSING_ );
			break; // while ( true )
		}

		// 1. 채널관리.
		try {
			ChannelManager( currTickCount );
		} catch ( char* str ) {
			PostServerEvent( "Caught 'ChannelManager' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception", str );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In ChannelManager Method. Throwing 'cGameProcess::BackendThreadProc' exception" );
			Sleep( 50 );
			throw;
		}

		// 2. 소켓관리.
		try {
			SocketManager( currTickCount );
		} catch ( char* str ) {
			PostServerEvent( "Caught 'SocketManager' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception", str );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In SocketManager Method. Throwing 'cGameProcess::BackendThreadProc' exception" );
			Sleep( 50 );
			throw;
		}

		// 3. 게임진행.
		try {
			GameManager( currTickCount );
		} catch ( char* str ) {
			PostServerEvent( "Caught 'GameManager' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception", str );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In GameManager Method. Throwing 'cGameProcess::BackendThreadProc' exception" );
			Sleep( 50 );
			throw;
		}

		// 4. I/O Context 처리.
		try {
			IoContextPresent( );
		} catch ( char* str ) {
			PostServerEvent( "Caught 'IoContextPresent' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception", str );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In IoContextPresent Method. Throwing 'cGameProcess::BackendThreadProc' exception" );
			Sleep( 50 );
			throw;
		}

		// 5. FPS 관리.
		try {
			BackendThreadFps( currTickCount );
		} catch ( char* str ) {
			PostServerEvent( "Caught 'BackendThreadFps' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception", str );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In BackendThreadFps Method. Throwing 'cGameProcess::BackendThreadProc' exception" );
			Sleep( 50 );
			throw;
		}
	} // while ( true )
}

// BackendThread Method
DWORD cGameProcess::BackendThread(void)
{
	try {
		// 1. 게임서버 준비중.
		try {
			BackendThreadBegin( );
		} catch ( char* str ) {
			SIZE_T quotaPagedPoolUsage = 0
				  ,quotaNonPagedPoolUsage = 0
				  ,workingSetSize = 0;
			mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );
			PostServerEvent( "Caught 'BackendThreadBegin' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception. I/O Context PagedPool(=%d) NonePagedPool(=%d) WorkingSetSize(=%d)"
							,str
							,quotaPagedPoolUsage
							,quotaNonPagedPoolUsage
							,workingSetSize );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In BackendThreadBegin Method. Throwing 'cGameProcess::BackendThread' exception." );
			Sleep( 50 );
			throw;
		}

		// 2. 게임서버 처리중.
		try {
			BackendThreadProc( );
		} catch ( char* str ) {
			SIZE_T quotaPagedPoolUsage = 0
				  ,quotaNonPagedPoolUsage = 0
				  ,workingSetSize = 0;
			mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );
			PostServerEvent( "Caught 'BackendThreadProc' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception. I/O Context PagedPool(=%d) NonePagedPool(=%d) WorkingSetSize(=%d)"
							,str
							,quotaPagedPoolUsage
							,quotaNonPagedPoolUsage
							,workingSetSize );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In BackendThreadProc Method. Throwing 'cGameProcess::BackendThread' exception." );
			Sleep( 50 );
			throw;
		}

		// 3. 게임서버 종료중.
		try {	
			BackendThreadEnd( );
		} catch ( char* str ) {
			SIZE_T quotaPagedPoolUsage = 0
				  ,quotaNonPagedPoolUsage = 0
				  ,workingSetSize = 0;
			mIoContextPool->GetProcessMemoryInfo( quotaPagedPoolUsage, quotaNonPagedPoolUsage, workingSetSize );
			PostServerEvent( "Caught 'BackendThreadEnd' exception type: '%s'. Throwing 'cGameProcess::BackendThreadProc' exception. I/O Context PagedPool(=%d) NonePagedPool(=%d) WorkingSetSize(=%d)"
							,str
							,quotaPagedPoolUsage
							,quotaNonPagedPoolUsage
							,workingSetSize );
			Sleep( 50 );
			throw;
		} catch ( ... ) {
			PostServerEvent( "In BackendThreadEnd Method. Throwing 'cGameProcess::BackendThread' exception." );
			Sleep( 50 );
			throw;
		}
	} catch ( ... ) {
		g_gameSrv->GetSender( )->PostChStatus( MAKECID((WORD)mServerNum, (WORD)mChannelNum), (BYTE)_E_STATUS_ERROR_ );

		mEndServer = true;
		BackendThreadEnd( );
		throw;
	}

	return 0;
}

// BackendThreadFps Method
void cGameProcess::BackendThreadFps(DWORD currentTick)
{
	if ( mFpsTick < currentTick )
	{
		// Console 모드 사용시 FPS 출력.
		if ( g_fps == true )
		{
			float fps = (float)mFrame / (float)(currentTick - (mFpsTick - 1000)) * 1000;
			printf( "FPS:%6.2f / FRAME:%d\tGameProcess Thread\n", fps, mFrame );
		}

		// Frame 변수 초기화.
		mFpsTick = currentTick + 1000;
		mFrame   = 0;
	}
	else
		mFrame++;

	/*-- 지연시간 - CPU 과부화 방지 & 내부 동기화. --*/

	// 경과시간 기록.
	mElapsedTick = GetTickCount( ) - currentTick;

	// 최소 지연시간 - None Sleep 지연시간 기록.
	mMinElapsedTick = max( mMinElapsedTick, mElapsedTick );

	if ( mElapsedTick > 1000 )
	{
		PostServerEvent( "Warning - Delays may occur on the server. Elapsed Tick(=%d).", mElapsedTick );
															// 경고 - 서버에 지연이 발생할 수 있습니다.
	}

	if ( mElapsedTick < 12 )
	{
		Sleep( 16 - mElapsedTick );
	}
	else
		Sleep( 5 );

	// 최대 지연시간 - Sleep 지연시간 기록.
	mMaxElapsedTick = max( mMaxElapsedTick, (GetTickCount( ) - currentTick) );
}

cGMEventList* cGameProcess::GetGMEvent( void )
{
	if( mGMEventList.IsEmpty() == false )
		return &mGMEventList;

	return 0;
}
