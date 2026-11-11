/* ====================================================================
 * 파    일 :	gameProcess.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	06/12/22
 * 주의사항 :	
 * =================================================================== */
#ifndef __GAME_PROCESS_H__
#define __GAME_PROCESS_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "iocpserver.h"

#include "stdafx.h"
#include "protocol.h"
#include "serverProtocol.h"
#include "logprotocol.h"
//#include "AntiCpXSvr.h"
#include "Player_Common.h"

class cRandomTable;
class cFileSystem;
class cMemoryPool;
class cCooltimePool;
class cTakeDamagePool;
class cItemBillPool;
class cItemGetPool;
class cItemCountPool;
class cThemeUserPool;
class cStageScript;
class cItemManager;
class cTitleManager;
class cGuildManager;
class cPartyManager;
class cTriggerManager;
class cQuestManager;
class cDrop;
class cPVPManager;
class cDuelManager;
class cDamageCalc;
class cSkillScript;
class cSkillManager;
class cObjectManager;
class cAIManager;
class cGridManager;
class cThemeManager;
class cPartyUnionManager;
class cPvPObjectPool;
class cPvPPlayerListPool;
class cSummonPatternPool;
class cSkillTargetPool;
class cTwitterScript;

class cAppTimer;

struct sPvPPlayerList;

// Common Database Flag
#define COMMON_DB_CHECK						0x00000001	// COMMON DB 01 bit
#define COMMON_DB_CHARACTER_LIST			0x00000002	// COMMON DB 02 bit
#define COMMON_DB_CHARACTER_NAME			0x00000004	// COMMON DB 03 bit
#define COMMON_DB_CHARACTER_INSERT			0x00000008	// COMMON DB 04 bit
#define COMMON_DB_CHARACTER_DELETE			0x00000010	// COMMON DB 05 bit
#define COMMON_DB_CHARACTER_SELECT			0x00000020	// COMMON DB 06 bit
#define COMMON_DB_CHARACTER_UPDATE			0x00000040	// COMMON DB 07 bit
#define COMMON_DB_CHARACTER_MONEY			0x00000080	// COMMON DB 08 bit
#define COMMON_DB_CHARACTER_ACTIVE_WEAPON   0x00000100	// COMMON DB 09 bit
#define COMMON_DB_CHARACTER_LOGIN           0x00000200	// COMMON DB 10 bit
#define COMMON_DB_FRIEND_SELECT				0x00000400	// COMMON DB 11 bit
#define COMMON_DB_FRIEND_INSERT				0x00000800	// COMMON DB 12 bit
#define COMMON_DB_FRIEND_UPDATE				0x00001000	// COMMON DB 13 bit
#define COMMON_DB_FRIEND_DELETE				0x00002000	// COMMON DB 14 bit
#define COMMON_DB_SPREAD_VALUE				0x00004000	// COMMON DB 15 bit
#define COMMON_DB_TAROT_RESULT				0x00008000	// COMMON DB 16 bit
#define COMMON_DB_COMEINGAME				0x00010000	// COMMON DB 17 bit
#define COMMON_DB_SHORTCUT_UPDATE			0x00020000	// COMMON DB 18 bit
#define COMMON_DB_LOGOUT					0x00040000	// COMMON DB 19 bit
#define COMMON_DB_ITEM_AGENT_SEARCH			0x00080000	// COMMON DB 20 bit
#define COMMON_DB_OUT						0x00100000	// COMMON DB 21 bit
#define COMMON_DB_CHARACTER_GM_NAME			0x00200000	// COMMON DB 22 bit
#define COMMON_DB_THEME_USER_CHECK			0x00400000	// COMMON DB 23 bit
#define COMMON_DB_THEME_USER_REMOVE			0x00800000	// COMMON DB 24 bit
#define COMMON_DB_MEMBER_TRIAL			    0x01000000	// COMMON DB 25 bit
#define COMMON_DB_MEMBER_LOGIN_LIMIT	    0x02000000	// COMMON DB 26 bit
#define COMMON_DB_GM_THEME_USER_RESET	    0x04000000	// COMMON DB 27 bit
#define COMMON_DB_GM_EVENT_INSERT		    0x08000000	// COMMON DB 28 bit
#define COMMON_DB_GM_EVENT_UPDATE		    0x10000000	// COMMON DB 29 bit
#define COMMON_DB_ITEM_PWD					0x20000000	// COMMON DB 30 bit
#define COMMON_DB_MEMBER_OBT_EVENT_COMPLETE	0x40000000	// COMMON DB 31 bit

enum GAME_PROCESS_REQUEST_TYPE
{
	// DEFAULT_GAME_PROCESS
	GAME_PROCESS_REQUEST = 0x4000,		// DEFAULT_GAME_PROCESS(0x4000) - defined gamesrv.h file
	// MAX
	MAX_GAME_PROCESS
};

struct ChannelList
{
	bool used;
	BYTE status;
};

struct sThemeData
{
	long          cid;
	THEME_STATUS  status;

	short         themeIdx;		// 테마 인덱스
	BYTE          fixTime;		// 고정 시간(0~23, 시간단위)
	BYTE          varTime;		// 가변 시간(1~NN, 시간단위)
	short         maxRoom;		// 최대 룸 갯수
	time_t        lastDate;		// 마지막 업데이트(일)
	time_t        validThru;	// 유효만기(일)
};

class cGameProcess :
	public cIocpServer
{
private:
	cFileSystem*        mpFileSystem;
	cAppTimer*          mpAppTimer;		// 070110 PKH 타이머
	unsigned long	    mElapsedTime;	// 061127 PKH 프로세스간 경과 시간
	unsigned long	    mAccumTime;		// 061201 PKH 처음부터 진행된 시간

	cMemoryPool*        mpMemoryPool;

	cRandomTable*       mpRandomTable;
	cSummonPatternPool* mpSummonPatternPool;
	cSkillTargetPool*   mpSkillTargetPool;
	cGridManager*       mpGridManager;
	cObjectManager*     mpObjectManager;
	cAIManager*         mpAIManager;
	cDrop*              mpDrop;	
	cSkillManager*      mpSkillManager;
	cStageScript*	    mpStageScript;
	cItemManager*       mpItemManager;
	cCooltimePool*      mpCooltimePool;
	cTakeDamagePool*    mpTakeDamagePool;
	cItemBillPool*      mpItemBillPool;
	cItemGetPool*       mpItemGetPool;
	cItemCountPool*     mpItemCountPool;
	cPvPObjectPool*     mpPvPObjectPool;
	cPvPPlayerListPool* mpPvPPlayerListPool;
	cThemeUserPool*     mpThemeUserPool;
	cPartyManager*	    mpPartyManager;
	cTriggerManager*    mpTriggerManager;
	cQuestManager*	    mpQuestManager;
	cDuelManager*       mpDuelManager;
	cPVPManager*        mpPVPManager;
	cThemeManager*      mpThemeManager;
	cTitleManager*	    mpTitleManager;
	cGuildManager*	    mpGuildManager;
	cPartyUnionManager* mpPartyUnionManager;
	cTwitterScript* mpTwitterScript;

private:
// 환경변수.
	union
	{
		struct
		{
			bool characterCreationLimited : 1;
		} mSystemManager;
		long mSystemManagerData;
	};
	union
	{
		struct
		{
			bool itemDefine       : 1;
			bool itemAbility      : 1;
			bool itemCoolitme2    : 1;
			bool itemLimit        : 1;
			bool itemCard         : 1;
			bool itemCardSlot     : 1;
			bool itemTarot        : 1;
			bool itemEnhanced     : 1;
			bool itemEnhancedRate : 1;
			bool itemDisjoint     : 1;
			bool itemChange       : 1;
			bool guild			  : 1;
			bool gmEvent  	      : 1;
			bool itemDropLimit    : 1;
		} mRequestImport;
		long mRequestImportData;
	};
	union
	{
		struct
		{
			bool itemDefine       : 1;
			bool itemAbility      : 1;
			bool itemCooltime2    : 1;
			bool itemLimit        : 1;
			bool itemCard         : 1;
			bool itemCardSlot     : 1;
			bool itemTarot        : 1;
			bool itemEnhanced     : 1;
			bool itemEnhancedRate : 1;
			bool itemDisjoint     : 1;
			bool itemChange       : 1;
			bool guild			  : 1;
			bool gmEvent  	      : 1;
			bool itemDropLimit    : 1;
		} mCompleteImport;
		long mCompleteImportData;
	};
	union
	{
		struct
		{
			bool importItemTables   : 1;
			bool verifyDefaultItems : 1;

			bool channelCheck       : 1;
			bool initGameDb         : 1;
			bool initThemeDb        : 1;

			bool channelSyn         : 1;

			bool shutdownGameDb     : 1;
			bool shutdownAccountDb  : 1;

			bool guildDb            : 1;

			bool captcha            : 1;
		} mRequest;
		long mRequestData;
	};
	union
	{
		struct
		{
			bool startLog          : 1;
			bool socketContext     : 1;
			bool ioContext         : 1;

			bool importItemTables  : 1;
			bool verifyDefaultItems: 1;

			bool channelCheck      : 1;
			bool initGameDb        : 1;
			bool initThemeDb       : 1;

			bool channelSyn        : 1;

			bool shutdownGameDb    : 1;
			bool shutdownAccountDb : 1;
			bool endLog            : 1;

			bool guildDb           : 1;

			bool captcha           : 1;
		} mComplete;
		long mCompleteData;
	};
	struct
	{
		long           cid;
		PVP_STATUS     status;
		long           timer;
		unsigned short pvpType;
		unsigned long  pvpEndTime;
	} mPVPData[PVP_DM_LEVEL_SECTION_MAX];

	sThemeData          mThemeData[MAX_THEMEDATA_MAX];

	char                mLastCategory;
	char                mLastProtocol;
	u_long              mLastPacketSize;

	DWORD               mFrame;
	DWORD               mFpsTick;
	bool                mThreadPriority;
	DWORD               mThreadPriorityTick;

	DWORD               mElapsedTick;
	DWORD               mMinElapsedTick;
	DWORD               mMaxElapsedTick;

	TCHAR*              mServerInetnum;
	wchar_t             mServerName[51];
	DWORD               mServerNum;
	DWORD               mChannelNum;
	SERVER_TYPE         mType;
	unsigned short      mInDunMapNumber;
	SERVER_STATUS       mStatus;

	ChannelList         mChannelList[MAX_CHANNEL];

	DWORD               mTTLTick;
	DWORD               mStatusSyncTick;

	unsigned long       mCaptchaOffset;
	unsigned long       mCaptchaMax;

//	AHNHS_SERVER_HANDLE mHackShieldHandle;
//	int                 mHackShieldEnable;
//	DWORD               mHackShieldCheckTime;
//	DWORD               mHackShieldWaitTime;

//	bool				mHackShieldPause;		/// false:기존작동유지, true:작동상태정지

	DWORD               mReleaseCountTick;
	DWORD               mCharacterSelectCount;
	DWORD               mGameInDbCount;
	DWORD               mGameOutDbCount;
	
	cGMEventList		mGMEventList;

public:
	unsigned long    GetAccumTime            ( ) { return mAccumTime; }
	unsigned long    GetElapsedTime          ( ) { return mElapsedTime; }
	unsigned long    GetItemTimeToLive       ( );
	unsigned long    GetItemTimeToOwner      ( );
	unsigned long    GetItemTimeToParty      ( );

public:
// LOG 관련 메소드.
	bool             PostServerEvent         ( LPCTSTR format, ... );
	bool             PostMemberEvent         ( char type, char category, long memberIdx, LPCTSTR format, ... );
	bool             PostCharacterEvent      ( char category, long memberIdx, long characterIdx, char _level, long _exp, char _skillLevel, long _skillExp, long _skillPoint, LPCTSTR format, ... );
	bool             PostMoneyEvent          ( long characterIdx, long before, long after, LPCTSTR format, ... );
	bool             PostDepositEvent        ( long characterIdx, long before, long after, char* message );
	bool             PostInventoryEvent      ( char category, long characterIdx, TB_INVENTORY* inventory, LPCTSTR format, ... );
	bool			 PostQuestEvent		     ( char category, long characterIdx, long questIdx, char* message );
	bool			 PostGuildEvent			 ( long characterIdx, long guildIdx, char guildPosition, char* message );
	bool			 PostDropItemEvent		 ( sDropItem* pDropItem, unsigned long count );
	bool			 PostSkillEvent          ( long characterIdx, bool isReset, long skillIdx, char skillStep );
	bool             PostItemBillEvent       ( long characterIdx, long inventory, long before, long after );
	bool             PostInfCashEvent        ( long characterIdx, char category, SKILL_INFLUENCE_UPDATE* pInf );
	bool             PostReservedEvent       ( short type, long characterIdx, long memberIdx );
	bool             PostPostEvent           ( long postIdx, long characterIdx, long inventoryIdx1, long inventoryIdx2, long inventoryIdx3, long money );
	
public:
// GAME SYNC 관련 메소드.
	bool             PostPVPStatus           ( BYTE status, unsigned short pvpType, unsigned long aryPos );
	bool             PostPvPNotice           ( unsigned char leftMinute, unsigned short pvpType, unsigned long aryPos );
	bool             PostPVPPlayerList       ( char* msg, unsigned long len );
	bool             PostThemeStatus         ( BYTE status );
	bool             PostPvPGmChannelMove    ( unsigned long playerIdx, unsigned short mapType );
	bool             PostNoChatEvent         ( long characterIdx, long validThru );
	bool			 PostAddGMEvent			 ( sGMEventInfo eventInfo );
	bool			 PostDelGMEvent			 ( unsigned long dbIndex, unsigned long influenceIdx );
	bool			 PostGMKickEvent		 ( long characterIdx );

	///
	bool             PostMonRegenSync        ( unsigned short levelMode, unsigned long switchGroup );

	/// 귓속말 
	bool			 PostChatWhisper		 ( unsigned long senderIdx, unsigned short senderJob, wchar_t* senderName, wchar_t* recvName, wchar_t* message, sInventory* inven );
	bool			 PostChatWhisperResult	 ( int errorCode, unsigned long senderIdx, wchar_t* recvName, ePLAYER_JOB recvJob, wchar_t* message, sInventory* inven );

	bool			 PostPostReceived        ( long characterIdx );

	/// 드랍제약
	bool             PostItemDropLimitUpdate ( unsigned long itemIdx, TIMESTAMP_STRUCT endTime );

private:
// SQL 관련 메소드 System.
	bool             VerifyItemDefine        ( void );
	bool             VerifyItemAbility       ( void );
	bool             VerifyItemCooltime2     ( void );
	bool             VerifyItemLimit         ( void );
	bool             VerifyItemCard          ( void );
	bool             VerifyItemCardSlot      ( void );
	bool             VerifyItemTarot         ( void );
	bool             VerifyItemEnhanced      ( void );
	bool             VerifyItemEnhancedRate  ( void );
	bool             VerifyItemDisjoint      ( void );
	bool             VerifyItemChange        ( void );
	bool             VerifyItemDropLimit     ( void );

	bool             ItemDefine              ( long idx );
	bool             ItemAbility             ( long idx );
	bool             ItemCooltime2           ( long idx );
	bool             ItemLimit               ( long idx );
	bool             ItemCard                ( long idx );
	bool             ItemCardSlot            ( long idx );
	bool             ItemTarot               ( long idx );
	bool             ItemEnhanced            ( long idx );
	bool             ItemEnhancedRate        ( long idx );
	bool             ItemDisjoint            ( long idx );
	bool             ItemChange              ( long idx );

	bool             VerifyDefaultItems      ( void );

	bool             ChannelCheck            ( short serverNum, short channelNum, char* inetnum, unsigned long ipv4, unsigned short port );
	bool             InitGameDb              ( );
	bool             ShutdownGameDb          ( );
	bool             ShutdownAccountDb       ( );

    // 자동 사냥 신고 CAPTCHA 이미지 관련
    void             CaptchaOffsetPlus       ( ) {  ( mCaptchaMax < (mCaptchaOffset + 1) ) ? mCaptchaOffset=1 : ++mCaptchaOffset; }
    bool             CaptChaOffset           ( );

public:
// SQL 관련 메소드 Game.
	bool             CharacterList           ( PerSocketContext* perSocketContext, long memberIdx );
	bool             CharacterDelete         ( PerSocketContext* perSocketContext, long memberIdx, long characterIdx, bool reuest=false );
	bool             CharacterTTL            ( long characterIdx, long ttl, DWORD channelNum );

	// 인벤토리
	bool             InventorySelect         ( PerSocketContext* perSocketContext, long characterIdx );
    bool             InventoryDelete         ( PerSocketContext* perSocketContext, INVENTORY_WARNING warning, long characterIdx, TB_INVENTORY* table );
	bool             InventoryCooltimeSelect ( PerSocketContext* perSocketContext, long characterIdx );
	bool             InventorySwap           ( PerSocketContext* perSocketContext, long idx1, short number1, long idx2, short number2 );
	bool             InventoryMove           ( PerSocketContext* perSocketContext, long idx, short number, bool cashBox );
	bool             InventoryExcept         ( PerSocketContext* perSocketContext, long* idx, short* number );
    bool             InventoryMerge          ( PerSocketContext* perSocketContext, long idx1, long idx2, short capacity );

	// 아이템 빌
	bool             ItemBillSelect          ( PerSocketContext* perSocketContext, long characterIdx );

	// 친구
	bool             FriendSelect            ( long characterIdx );
	// 타로카드(스킬효과)
	bool             ItemTarotResult         ( PerSocketContext* perSocketContext, long itemDefineIndex, long value );

	// 스킬
	bool             SkillSelect		     ( PerSocketContext* perSocketContext, unsigned long characterIdx );
	bool             SkillInsert		     ( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, /*unsigned long money,*/ unsigned short sp, unsigned char skillLevel, unsigned long skillPoint );
	bool             SkillUpdate		     ( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, unsigned char skillStep, /*unsigned long money,*/ unsigned short sp, unsigned char skillLevel, unsigned long skillPoint );
	bool             SkillCoolTime		     ( PerSocketContext* perSocketContext, unsigned long characterIdx );

	// 치트로 스킬 배우기
	bool			 SkillCheatInsert		 ( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, unsigned char skillStep );
	bool			 SkillCheatUpdate		 ( PerSocketContext* perSocketContext, unsigned long characterIdx, unsigned long skillClassIdx, unsigned char skillStep );

	// 효과
	bool             SkillInfluenceSelect    ( PerSocketContext* perSocketContext, unsigned long characterIdx );

	// 큇슬롯(Shortcut)
	bool             ShortcutSelect          ( PerSocketContext* perSocketContext, unsigned long characterIdx );

	// 퀘스트
	bool			 QuestSelect		     ( PerSocketContext* perSocketContext, long characterIdx );
	bool			 QuestCompleteSelect     ( PerSocketContext* perSocketContext, long characterIdx );
	bool			 QuestValidSelect		 ( PerSocketContext* perSocketContext, long characterIdx );
	bool			 QuestRemove		     ( PerSocketContext* perSocketContext, long dbIdx );

	// 호칭
	bool			 TitleSelect			 ( PerSocketContext* perSocketContext, long characterIdx );

	// 길드
	bool			 VerifyGuild			 ( void );
	bool			 GuildSelect			 ( long idx, bool restore = false );	/// DB 고유번호 restore : 길드 정보 복구용
	bool			 GuildUserSelect		 ( long idx, bool restore = false );	/// 길드 인덱스 restore : 길드원 정보 복구용

	bool			 GuildCreate			 ( PerSocketContext* perSocketContext, long characterIdx, wchar_t* guildName, wchar_t* masterName );
	bool			 GuildDelete			 ( PerSocketContext* perSocketContext, long idx, long characterIdx );
	bool			 GuildUserAdd			 ( PerSocketContext* perSocketContext, long idx, long characterIdx );
	bool			 GuildUserOut			 ( PerSocketContext* perSocketContext, long idx, long characterIdx );
	bool			 GuildUserCut			 ( PerSocketContext* perSocketContext, long idx, long characterIdx );
	bool			 GuildUserGivePosition   ( PerSocketContext* perSocketContext, long idx, long characterIdx, char position );
	bool			 GuildMarkUpdate		 ( PerSocketContext* perSocketContext, long idx, TIMESTAMP_STRUCT registDate, BYTE* mark );

	/// 제조 스킬
	bool             MakeSkillSelect         ( PerSocketContext* perSocketContext, unsigned long characterIdx );
	bool             RecipeCoolTime		     ( PerSocketContext* perSocketContext, cPlayer* hero );

	/// 운세 저장 정보
	bool			 FortuneSelect			 ( PerSocketContext* perSocketContext, unsigned long characterIdx );

	/// 귓말 - 캐릭터 접속정보 검색
	bool			 WhisperSelect	         ( PerSocketContext* perSocketContext, wchar_t* recvName, wchar_t* message, short slotIndex );

	// 테마
	bool             ThemeUserSelect         ( PerSocketContext* perSocketContext, long characterIdx );

    // 자동사냥 신고
	bool             TrialTimeSelect         ( PerSocketContext* perSocketContext, long characterIdx );

	// 치트
	bool			 CheatJobUpdate			 ( PerSocketContext* perSocketContext, long characterIdx, long updateJobIndex, unsigned char race, long jobStep ); //케릭터 직업 치트로 변경
	bool			 VerifyGmEvent			 (void); ///GM Event 관리

private:
// 서버 관련 메소드 - 보조.
	bool             IsCharacterName         ( wchar_t* name, int len );
	bool             SendChannelList         ( PerSocketContext* perSocketContext );

private:
// 서버 관련 메소드 - 보조.
	bool             BatchComplete           ( DWORD currentTick, PerSocketContext* perSocketContext, void* ptr );
	bool             UpdateComplete          ( PerSocketContext* perSocketContext, bool logout=false, bool shutdown=false );

	void             GameManager             ( DWORD currentTick );
	void             SocketManager           ( DWORD currentTick );
	void             ChannelManager          ( DWORD currentTick );

	void             BackendThreadBegin      ( void );
	void             BackendThreadProc       ( void );
	void             BackendThreadEnd        ( void );
	void             BackendThreadFps        ( DWORD currentTick );

private:
// 서버 관련 메소드 - 보조.
	void             Decrypt                 ( BYTE key, long len, char* ptr );
	void             Encrypt                 ( BYTE key, long len, char* ptr );

private:
// 서버 관련 메소드 - 주.
	bool             SendExec                ( PerIoContext* perIoContext );
	bool             SendPost                ( PerSocketContext* perSocketContext, PerIoContext* perIoContext );

	bool             AcceptComplete          ( PerSocketContext* perSocketContext );
	bool             SendComplete            ( PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred );
	bool             RecvComplete            ( PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred );
	bool             CallbackComplete        ( PerSocketContext* perSocketContext, PerIoContext* perIoContext, DWORD bytesTransferred );

	void             IoContextPresent        ( );

public:
// 클래스 생성 및 공용 메소드.
	cGameProcess(DWORD serverNum, DWORD channelNum, TCHAR* inetnum/*, int ahnhsEnable, DWORD ahnhsCheck, DWORD ahnhsWait*/);

	bool             Initialize              ( char* ipAddr="", unsigned short port=10000, unsigned short numWorkerThreads=2, u_long bufferLength=HAL_TCP_PACKET_SIZE );
	void             Shutdown                ( DWORD maxWait=INFINITE );
	bool             CloseCID                ( DWORD cid );

	long             GetCID                  ( ) { return MAKECID( (WORD)mServerNum, (WORD)mChannelNum); }
	DWORD            GetServerNum            ( ) { return mServerNum; }
	DWORD            GetChannelNum           ( ) { return mChannelNum; }
	SERVER_TYPE      GetServerType           ( ) { return mType; }
	unsigned short   GetInDunMapNum          ( ) { return mInDunMapNumber; }
	sThemeData*      GetThemeData            ( unsigned short mapType );
	
	char*            GetSQL                  ( void** handle, UINT_PTR iParam );
	bool             SendSQL                 ( void* handle, unsigned long length );
	bool             SendSQL                 ( PerSocketContext* perSocketContext, void* handle, unsigned long length, long commonDatabase=0 );
	bool             SendSQL                 ( unsigned long connectionIdx, void* handle, unsigned long length, long commonDatabase=0 );
	void             ReleaseSQL              ( void* handle, unsigned long length );

	PerIoContext*    GetIoContext            ( u_long request );
	void             ReleaseIoContext        ( PerIoContext* perIoContext, char* callStack=NULL );

	bool             SendGame2Game           ( PerSocketContext* perSocketContext, u_long ipv4, u_short port );

	bool             GotoInstantDungeon      ( PerSocketContext* perSocketContext, long cid );
	bool             GotoInstantDungeon      ( unsigned long connectionIdx, long cid );
	bool             ReturnGameServer        ( unsigned long connectionIdx );

	bool             SendToUser              ( unsigned long connectionIdx, char* msg, unsigned long length );

	bool             SendMsgRoot             ( PerSocketContext* perSocketContext, char category, char protocol );
	bool             SendMsgRoot             ( unsigned long connectionIdx, char category, char protocol );
	bool             SendMsgError            ( PerSocketContext* perSocketContext, char category, char protocol, long error );
	bool             SendMsgError            ( unsigned long connectionIdx, char category, char protocol, long error );

	MSGROOT*         GetMsgRoot              ( void** handle, PerSocketContext* perSocketContext, char category=0, char protocol=0 );
	MSGROOT*         GetMsgRoot              ( void** handle, unsigned long connectionIdx, char category=0, char protocol=0 );
	bool             SendMsgRoot             ( void* handle, unsigned long length );
	void             SendMsgRoot             ( char* buffer, unsigned long& internal, unsigned long& internalHigh, char* msg, int msgLen );
	void             ReleaseMsgRoot          ( void* handle, unsigned long length );

	MSGROOT*         GetHackShieldMsgRoot    ( void** handle, PerSocketContext* perSocketContext, char category=0, char protocol=0 );

	bool             QuickSend               ( cBaseObject* object, char* msg, int length );
	bool             QuickSendExcept         ( cBaseObject* object, char* msg, int length );
	bool             QuickSendMap            ( cBaseObject* object, char* msg, int length );
	bool             QuickSendMap            ( unsigned short mapNumber, char* msg, int length );
	bool			 QuickSendExceptMap	     ( cPlayer* object1, cPlayer* object2, char* msg, int length );

// 채널 동기 메소드.
	bool             IsChannelCheck          ( ) { return mComplete.channelCheck; }

	void             ChannelSync             ( );
	void             ChannelStatus           ( long cid, BYTE status );
	void             ServerNotice            ( long cid, wchar_t* message );
	void			 ServerDownCountNotice   ( long cid, unsigned short count );
	void             ChatMegaphone           ( long cid, char* msg, int len );
	void             PVPStatus               ( long cid, BYTE status, unsigned short pvpType, unsigned long aryPos );
	void             PVPNotice               ( long cid, unsigned char leftMinute, unsigned short pvpType, unsigned long aryPos );
	void             PVPPlayerList           ( MB_SYN_PVP_PLAYERLIST* pMBMsg );
	void             PvPGMChannelRes         ( MB_SYN_PVPGM_CHANNELMOVE* pMBMsg );
	void             ThemeStatus             ( long cid, BYTE status, unsigned short mapNum );
	void			 FriendSync				 ( long cid, char* msg, int len );
	void             GuildSync               ( long cid, char* msg, int len );
	void             MonsterSync             ( long cid, char* msg, int len );
	void			 ChatSync                ( long cid, char* msg, int len );
	void			 PostReceived            ( MB_SYN_POST_RECEIVED* msgBuf );
	void             PartySync               ( MB_SYN_PARTY* msgBuf );
    void             EnhancedSync            ( MB_SYN_ENHANCED_RESULT* msgBuf );
	void			 NoChatSync				 ( MB_SYN_NO_CHAT* msgBuf );
	void			 UniqueItemSync			 ( MB_SYN_UNIQUEITEM_GET* msgBuf );
	void			 AddGMEventSync			 ( MB_SYN_ADD_GM_EVENT* msgBuf );
	void			 DelGMEventSync			 ( MB_SYN_DEL_GM_EVENT* msgBuf );
	void             ItemDropLimitUpdate     ( MB_SYN_DROPITEM_UPDATE* msgBuf );

	void			 GMKickEventSync		 ( MB_SYN_GM_KICK_EVENT* msgBuf );

// 빌링 동기 메소드.
	void             Pg_Game_Start           ( char* Session, char* User_ID, char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain );
	void             Pg_User_Alert           ( long Packet_Result, char* Session, char* User_ID, char* Bill_Method, long Bill_Remain );
	void             Pg_User_Sync            ( char* Session, char* User_ID, char* User_Status );
	void             Pg_Billing_Auth         ( char* Session, char* User_ID, char* User_IP, char* User_Status, char* Bill_Method, char* Bill_Expire, long Bill_Remain );
	void             Pg_Client_Move          ( char* Game_Server, char* Session, char* User_ID, char* User_IP, char* User_Status, long Game_No );
	void             Pg_Close                ( );

/// 페이레터 빌링
	void			BP_User_Auth( unsigned long memberIdx );
	void			BP_User_End( unsigned long memberIdx );
	void			BP_Item_Give( unsigned long memberIdx );
	void			BP_Close( );

// Backend Thread.
	DWORD            BackendThread           ( void );

// GM 이벤트 메소드
	cGMEventList*	 GetGMEvent				 ( void );

public:
// 클래스 파괴 메소드.
	virtual ~cGameProcess(void);

public:
	static cGameProcess* mGameProcess;
};

#define NETWORK2     cGameProcess::mGameProcess

// Prototypede
time_t timestamp2time_t (TIMESTAMP_STRUCT& ts);
time_t plusSecTime_t    ( long sec );

#endif // __GAME_PROCESS_H__
