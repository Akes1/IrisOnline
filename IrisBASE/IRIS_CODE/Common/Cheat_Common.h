/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.06.04
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma pack( push, 1 )
#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"
#include "Item_Common.h"
#include "sqltypes.h"

enum eCHEAT_SPEEDUP
{
	eCHEATSPEEDUP_NONE = 0,
	eCHEATSPEEDUP_50UP,
	eCHEATSPEEDUP_100UP,
	eCHEATSPEEDUP_150UP,
	eCHEATSPEEDUP_200UP,
	eCHEATSPEEDUP_250UP,

	eCHEATSPEEDUP_MAX,

};

enum eCHEAT_CHANGE_JOB_LEVEL
{ 
	eCHANGE_JOB_LEVEL_1 = 30, /// 1차 전직 래밸
	eCHANGE_JOB_LEVEL_2 = 60, /// 2차 전직 래밸
};

const int ERROR_CHEAT_NO_ERROR = 0;
const int ERROR_CHEAT_ALREADY_RUN = 1;
const int ERROR_CHEAT_UNKNOWN_USER = 2;

/// 이름등록
#define ERROR_CHEAT_GM_NAME_SUCCESS		0	// 등록성공
#define ERROR_CHEAT_GM_NAME_FAIL		1	// 등록실패
#define ERROR_CHEAT_GM_NAME_ALREADY		2	// 등록실패-이미사용중
#define ERROR_CHEAT_GM_NAME_EXISTS		3	// 등록실패-중복됨

struct MSG_REQ_CHEAT_GM_NAME : public MSGROOT
{
	wchar_t		  mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct MSG_RES_CHEAT_GM_NAME : public MSG_ERROR
{};

/// 입장
struct MSG_REQ_CHEAT_IN : public MSGROOT
{
};

struct MSG_RES_CHEAT_IN : public MSGROOT
{
};

struct MSG_SYN_CHEAT_IN : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
};

/// 속도 증가
struct MSG_REQ_CHEAT_SPEEDUP : public MSGROOT
{
	eCHEAT_SPEEDUP		mSpeedUp;
};

/// 귓말
struct MSG_REQ_CHEAT_EAR : public MSGROOT
{
	bool mMode;
};

struct MSG_RES_CHEAT_EAR : public MSGROOT
{
	bool mMode;
};

/// 투명
struct MSG_REQ_CHEAT_HIDE : public MSGROOT
{
	bool mMode;
};

struct MSG_RES_CHEAT_HIDE : public MSGROOT
{
	bool mMode;
};

struct MSG_SYN_CHEAT_HIDE_ON : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
};

/// 무적
struct MSG_REQ_CHEAT_UNDEAD : public MSGROOT
{
	bool mMode;
};

struct MSG_RES_CHEAT_UNDEAD : public MSGROOT
{
	bool mMode;
};

/// 정보
struct MSG_REQ_CHEAT_INFO : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct MSG_RES_CHEAT_INFO : public MSGROOT
{
	wchar_t mUserID[MAX_UID_BUFFER_SIZE]; /// 아이디
	short   mMapNum; /// 맵 번호
	float mPosX; /// 좌표
	float mPosY;
	unsigned short mJob; /// 직업
	unsigned char mLevel; /// 레벨
	unsigned long mMoney; /// 돈
	/// 길드
};

/// 감시
struct MSG_REQ_CHEAT_WATCH : public MSGROOT
{
	bool mMode;
};

struct MSG_RES_CHEAT_WATCH : public MSGROOT
{
	bool mMode;
};

/// 접속 종료
struct MSG_REQ_CHEAT_KICK : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

const int ERROR_CHEAT_KICK_SUCCESS	     = 0;
const int ERROR_CHEAT_KICK_FAIL		     = 1;
const int ERROR_CHEAT_KICK_UNKNOWN_USER  = 2;
const int ERROR_CHEAT_KICK_NOT_CONNECTED = 3;

struct MSG_RES_CHEAT_KICK : public MSGROOT
{
};

struct MSG_RES_CHEAT_KICK_GM : public MSG_ERROR
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

/// 회복
struct MSG_REQ_CHEAT_RECOVER : public MSGROOT
{
};

struct MSG_RES_CHEAT_RECOVER : public MSGROOT
{
	unsigned long mMaxHP; /// HP
	unsigned long mMaxMP; /// MP
};

struct MSG_SYN_CHEAT_RECOVER : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
	unsigned long mMaxHP; /// HP
	unsigned long mMaxMP; /// MP
};

/// 부활
struct MSG_REQ_CHEAT_REBIRTH : public MSGROOT
{
};

struct MSG_RES_CHEAT_REBIRTH : public MSGROOT
{
	unsigned long mMaxHP; /// HP
	unsigned long mMaxMP; /// MP
};

struct MSG_SYN_CHEAT_REBIRTH : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
	unsigned long mMaxHP; /// HP
	unsigned long mMaxMP; /// MP
};

/// 순간이동
struct MSG_REQ_CHEAT_TELEPORT : public MSGROOT
{
	float mStartX; /// 시작 지점
	float mStartY;
	float mDestX; /// 목표 지점
	float mDestY;
};

struct MSG_SYN_CHEAT_TELEPORT : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
	float mDestX; /// 목표 지점
	float mDestY;
};

/// 가기
struct MSG_REQ_CHEAT_GO : public MSGROOT
{
	short mMapNum; /// 맵 번호
	float mDestX; /// 목표 지점
	float mDestY;
};

/// 추적
struct MSG_REQ_CHEAT_CHASE : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct MSG_RES_CHEAT_CHASE_GM : public MSG_ERROR
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

/// 이동
struct MSG_REQ_CHEAT_MOVE : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
	short   mMapNum; /// 맵 번호
	float mDestX; /// 목표 지점
	float mDestY;
};

struct MSG_RES_CHEAT_MOVE_GM: public MSG_ERROR
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

/// 소환
struct MSG_REQ_CHEAT_CALL : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

const unsigned char ERROR_CHEAT_CALL_SUCCESS			  = 0;
const unsigned char ERROR_CHEAT_CALL_FAIL_UNKNOWN_USER	  = 1;
const unsigned char ERROR_CHEAT_CALL_FAIL_USER_STOP_STATE = 2;
const unsigned char ERROR_CHEAT_CALL_FAIL_MAP_CHANGE      = 3;
const unsigned char ERROR_CHEAT_CALL_FAIL_DIFF_MAP		  = 4;

struct MSG_RES_CHEAT_CALL_GM : public MSG_ERROR
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

/// 유저 위치 추적
struct MSG_REQ_CHEAT_SEARCH : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

const unsigned char ERROR_CHEAT_SEARCH_SUCCESS			  = 0;
const unsigned char ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER  = 1;
const unsigned char ERROR_CHEAT_SEARCH_FAIL_UNKNOWN_USER2 = 2;
struct MSG_RES_CHEAT_SEARCH_GM : public MSG_ERROR
{
	short   mMapNum; /// 맵 번호
	float	mPosX;	 /// 좌표
	float	mPosY;
};

const unsigned char ERROR_CHEAT_SEARCH_CHANNEL_SUCCESS = 0;

struct MSG_RES_CHEAT_SEARCH_CHANNEL_GM : public MSG_ERROR
{
	short   mChannelNum; /// 맵 번호
};

/// 채팅 금지
struct MSG_REQ_CHEAT_NO_CHAT : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
	unsigned long mDuration; /// 지속 시간 (분)
};

const unsigned char ERROR_CHEAT_NO_CHAT_SUCCESS   = 0;
const unsigned char ERROR_CHEAT_NO_CHAT_FAIL      = 1;
const unsigned char ERROR_CHEAT_NO_CHAT_NO_PLAYER = 2;

struct MSG_RES_CHEAT_NO_CHAT_GM : public MSG_ERROR
{
};

struct MSG_SYN_CHEAT_NO_CHAT : public MSGROOT
{
};

struct MSG_REQ_CHEAT_NO_CHAT_LIST : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

const unsigned char ERROR_CHEAT_NO_CHAT_LIST_SUCCESS      = 0;
const unsigned char ERROR_CHEAT_NO_CHAT_LIST_FAIL		  = 1;
const unsigned char ERROR_CHEAT_NO_CHAT_LIST_NO_PLAYER    = 2;

struct MSG_RES_CHEAT_NO_CHAT_LIST_GM : public MSG_ERROR
{
};

struct NO_CHAT_CHARACTER_DUMMY
{
	wchar_t			 characterName[ 17 ];	// 캐릭터 이름
	wchar_t			 adminName[ 17 ];	    // 제한을 가한 운영자 케릭터 이름
	time_t registDate;						// 등록일
	time_t validThru;						// 유효만기
};

struct MSG_RES_CHEAT_NO_CHAT_LIST : public MSG_ERROR
{
	int						mRowCount; /// 적용 행 수
	NO_CHAT_CHARACTER_DUMMY table[1];  /// 채팅 금지된 케릭터 정보

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_CHEAT_NO_CHAT_LIST) - sizeof(table) + (mRowCount * sizeof(table)); }
};

/// 채팅 허용
struct MSG_REQ_CHEAT_ALLOW_CHAT : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

const unsigned char ERROR_CHEAT_ALLOW_CHAT_SUCCESS   = 0;
const unsigned char ERROR_CHEAT_ALLOW_CHAT_FAIL      = 1;
const unsigned char ERROR_CHEAT_ALLOW_CHAT_NO_PLAYER = 2;

struct MSG_RES_CHEAT_ALLOW_CHAT_GM : public MSG_ERROR
{
};

struct MSG_SYN_CHEAT_ALLOW_CHAT : public MSGROOT
{
};

/// 인던이동
/// NM_CHEAT_GOTO_INSTANT_DUNGEON_REQ

enum TYPE_GOTO_INSTANT_DUNGEON
{
	GOTO_ID_PVP01 = 101,
	GOTO_ID_PvP02 = 102,
	GOTO_ID_THEME01 = 200,
	GOTO_ID_THEME02 = 201,
	GOTO_ID_TURORIAL = 501,

};
struct MSG_REQ_GOTO_INSTANT_DUNGEON : public MSGROOT
{ 
	unsigned short type;							// TYPE_GOTO_INSTANT_DUNGEON 참조
};

const int ERROR_GOTO_GAMESERVER_SUCCESS			= 0;	// 성공
const int ERROR_GOTO_GAMESERVER_FAIL			= 1;	// 이동 설정 오류
struct MSG_REQ_GOTO_GAMESERVER : public MSGROOT
{ 
	unsigned short	mMapNum;
	float			mPosX;
	float			mPosY;
};

#define ERROR_GOTO_INSTANT_DUNGEON_SUCCESS	0	// 성공
#define ERROR_GOTO_INSTANT_DUNGEON_FAIL		1	// 실패
#define ERROR_GOTO_INSTANT_DUNGEON_FAIL_ALREADY_IN	2	// 정상적인 입장 후 GM 툴 사용으로 던젼이동은 불가.

struct MSG_RES_GOTO_INSTANT_DUNGEON : public MSG_ERROR
{
};

/// NM_CHEAT_USED_ROOM_ID_REQ
struct MSG_REQ_USED_ROOM_ID : public MSGROOT
{
	unsigned short type;							// TYPE_GOTO_INSTANT_DUNGEON 참조
};

#define ERROR_USED_ROOM_ID_SUCCESS			0	// 성공
#define ERROR_USED_ROOM_ID_FAIL				1	// 실패

struct MSG_RES_USED_ROOM_ID : public MSG_ERROR
{
	long rowCount;
	long roomInfo[1];

	unsigned long GetMsgLength( ) { return (sizeof(MSG_RES_USED_ROOM_ID)-sizeof(roomInfo)) + (rowCount*sizeof(roomInfo)); }
};

///NM_CHEAT_ROOMCHANGE_INSTANT_DUNGEON_REQ
#define ERROR_ROOMCHANGE_INSTANT_DUNGEON_SUCCESS	0	// 성공
#define ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL		1	// 실패
#define ERROR_ROOMCHANGE_INSTANT_DUNGEON_FAIL_ALREADY_IN	2	// 정상적인 입장 후 GM 툴 사용으로 던젼이동은 불가.

struct MSG_REQ_ROOMCHANGE_INSTANT_DUNGEON : public MSG_ERROR
{
	unsigned short mapNum;
};

/// 정지
struct MSG_REQ_CHEAT_STOP : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
	unsigned long mDuration; /// 지속 시간 (밀리초)
};

struct MSG_RES_CHEAT_STOP_GM : public MSG_ERROR
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

const int ERROR_CHEAT_STOP_SUCCESS = 0; /// 성공
const int ERROR_CHEAT_STOP_FAIL = 1; /// 실패
const int ERROR_CHEAT_STOP_NOT = 2; /// 치트 정지상태 아님

struct MSG_SYN_CHEAT_STOP : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
};

/// 정지 해제
struct MSG_REQ_CHEAT_CANCEL_STOP : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct MSG_RES_CHEAT_CANCEL_STOP_GM : public MSG_ERROR
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct MSG_SYN_CHEAT_CANCEL_STOP : public MSGROOT
{
	unsigned long mObjIndex; /// 오브젝트 인덱스
};

/// 아이템 제거
struct MSG_REQ_CHEAT_REMOVE_ITEM : public MSGROOT
{
};

/// 몬스터 제거
struct MSG_REQ_CHEAT_REMOVE_MONSTER : public MSGROOT
{
};

/// 레벨업
struct MSG_REQ_CHEAT_LEVELUP : public MSGROOT
{
	unsigned char mAddedValue; /// 더할 값
};

/// 스킬업
struct MSG_REQ_CHEAT_SKILLUP : public MSGROOT
{
	unsigned char mAddedValue; /// 더할 값
};

/// 아이템 생성
struct MSG_REQ_CHEAT_MAKE : public MSGROOT
{
	unsigned long mItemIndex;	/// 아이템 인덱스
	unsigned short mCount;		/// 수량
	unsigned char mEnhance;		/// 강화( 0 ~ 15 )

	/// 카드장착
	unsigned long mCardSlot[5];	/// 장착한 카드정보
};

const int ERROR_CHEAT_MAKE_SUCCESS = 0; /// 성공
const int ERROR_CHEAT_MAKE_FAIL = 1; /// 실패
const int ERROR_CHEAT_MAKE_NOTYET = 2; /// DB사용중

struct MSG_RES_CHEAT_MAKE : public MSG_ERROR
{
	BYTE       rowCount;		// 영향받은 행 수
	sInventory inventory[1];	// 인벤토리

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_CHEAT_MAKE) - sizeof(inventory) + (rowCount * sizeof(inventory)); }
};

struct sCheat_Make_Item_Info
{
	unsigned long mItemIndex;	/// 아이템 인덱스
	unsigned short mCount;		/// 수량
	unsigned char mEnhance;		/// 강화( 0 ~ 15 )

	/// 카드장착
	unsigned long mCardSlot[5];	/// 장착한 카드정보
};

/// 아이템 여러개 생성
/// 아이템 여러개 생성 응답 처리는 MSG_RES_CHEAT_MAKE 로 받음.
struct MSG_REQ_CHEAT_MAKE_ITEMS : public MSGROOT
{
	unsigned char mCount;
	sCheat_Make_Item_Info mItemInfo[1];

	// 실제보내는 메세지 크기
	unsigned long GetMsgLength( ) { return sizeof(MSG_REQ_CHEAT_MAKE_ITEMS) - sizeof(mItemInfo) + (mCount * sizeof(mItemInfo)); }
};

/// 돈 생성
struct MSG_REQ_CHEAT_GOLD : public MSGROOT
{
	unsigned long mValue; /// 더할 값
};

/// 몬스터를 소환
struct MSG_REQ_CHEAT_SUMMON : public MSGROOT
{
	unsigned long mMonsterNum; /// 몬스터 번호
	unsigned char mValue; /// 값
};

/// 몬스터를 제거
struct MSG_REQ_CHEAT_REMOVE : public MSGROOT
{
	unsigned long mObjIndex;  /// 오브젝트 인덱스
};

struct MSG_REQ_CHEAT_KILL : public MSGROOT
{
	unsigned long mObjIndex;
};

/// Exp 증가
struct MSG_REQ_CHEAT_EXP : public MSGROOT
{
	short mExpUpPercent;
};

const int ERROR_CHEAT_EXP_SUCCESS = 0; /// 성공
const int ERROR_CHEAT_EXP_FAIL = 1; /// 실패
struct MSG_RES_CHEAT_EXP_GM : public MSG_ERROR
{
	short mExpUpPercent;
};

/// SXP 증가
struct MSG_REQ_CHEAT_SXP : public MSGROOT
{
	short mSxpUpPercent;
};

const int ERROR_CHEAT_SXP_SUCCESS = 0; /// 성공
const int ERROR_CHEAT_SXP_FAIL = 1; /// 실패
struct MSG_RES_CHEAT_SXP_GM : public MSG_ERROR
{
	short mSxpUpPercent;
};

/// 드롭율 증가
struct MSG_REQ_CHEAT_DROP : public MSGROOT
{
	short mDropUpPercent;
};

const int ERROR_CHEAT_DROP_SUCCESS = 0; /// 성공
const int ERROR_CHEAT_DROP_FAIL = 1; /// 실패
struct MSG_RES_CHEAT_DROP_GM : public MSG_ERROR
{
	short mDropUpPercent;
};

const unsigned char ERROR_CHEAT_SKILL_ADD_SUCCESS		= 0;
const unsigned char	ERROR_CHEAT_SKILL_ADD_DBERROR		= 1;
const unsigned char	ERROR_CHEAT_SKILL_ADD_ERR_PLAYER	= 2;
const unsigned char	ERROR_CHEAT_SKILL_ADD_ERR_SCRIPT	= 3;
const unsigned char	ERROR_CHEAT_SKILL_ADD_ERR_SKILLSTEP = 4;
const unsigned char	ERROR_CHEAT_SKILL_ADD_ERR_RACE		= 5;
const unsigned char	ERROR_CHEAT_SKILL_ADD_ERR_JOB		= 6;
const unsigned char	ERROR_CHEAT_SKILL_ADD_ERR_MAPINSERT = 7;

struct MSG_REQ_CHEAT_SKILL_ADD : public MSGROOT
{
	unsigned long	mSkillClassIdx;
	unsigned char	mSkillStep;
	bool			mIsSkillUp;
};

const unsigned char ERROR_CHEAT_GM_SKILL_USE_SUCCESS		= 0;
const unsigned char	ERROR_CHEAT_GM_SKILL_USE_ERR_TARGET		= 1;
struct MSG_REQ_CHEAT_GM_SKILL_USE_PLAYER : public MSGROOT
{
	unsigned long	mCharacterIdx;
	unsigned long	mInfluenceIndex;
};

struct MSG_SYN_CHEAT_GM_SKILL_USE_PLAYER : public MSGROOT
{
	unsigned long	mInfluenceIndex;
};

const unsigned char ERROR_CHEAT_GM_SKILL_AREA_SUCCESS		= 0;
const unsigned char	ERROR_CHEAT_GM_SKILL_AREA_ERR_TARGET	= 1;
struct MSG_REQ_CHEAT_GM_SKILL_USE_AREA : public MSGROOT
{
	float			mPosX;
	float			mPosY;
	unsigned long	mInfluenceIndex;
	//unsigned long	mDistance; ///거리
};

const unsigned char ERROR_CHEAT_GM_JOB_CHANGE_SUCCESS		= 0;
const unsigned char	ERROR_CHEAT_GM_JOB_CHANGE_ERR_NOT_MATCH	= 1; /// 전직할 수 있는 직업이 아님
const unsigned char	ERROR_CHEAT_GM_JOB_CHANGE_DB_ERR		= 2; /// DB사용중

struct MSG_REQ_CHEAT_GM_JOB_CHANGE : public MSGROOT
{
	unsigned long mChangeJobIndex;
};

const unsigned char ERROR_CHEAT_GM_FORCE_CHANGE_SUCCESS	 = 0;
const unsigned char	ERROR_CHEAT_GM_FORCE_CHANGE_SAMETYPE = 1;
const unsigned char	ERROR_CHEAT_GM_FORCE_CHANGE_BADTYPE  = 2;

struct MSG_REQ_CHEAT_GM_FORCE_CHANGE : public MSGROOT
{
	unsigned char mForceType;	
};

const unsigned char ERROR_CHEAT_GM_FORCE_POINT_SUCCESS  = 0;
const unsigned char	ERROR_CHEAT_GM_FORCE_POINT_NOFORCE  = 1;
const unsigned char	ERROR_CHEAT_GM_FORCE_POINT_BADTYPE  = 2;
const unsigned char	ERROR_CHEAT_GM_FORCE_POINT_FAIL	    = 3;
const unsigned char	ERROR_CHEAT_GM_FORCE_POINT_BADPOINT = 4;

struct MSG_REQ_CHEAT_GM_FORCE_POINT : public MSGROOT
{
	unsigned char mForceType;	
	unsigned long mForcePoint;
};


struct MSG_REQ_CHEAT_PLAYERINFO : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct MSG_RES_CHEAT_PLAYERINFO : public MSG_ERROR
{
	unsigned int	mState;
	unsigned char	mStateStop;
	unsigned long	mRequestRejection;
	bool			mCashInfo;
	long			mInventoryDB;
	long			mCommonDatabase;
	long			mGameInDatabase;
	bool			mGameInDBEnd;
	long			mSkillDatabase;
	long			mQuestDatabase;
	long			mGuildDatabase;
	long			mRequestData;
	long			mCompleteData;
};

const unsigned char ERROR_CHEAT_GM_THEME_INFO_SUCCESS   = 0;
const unsigned char ERROR_CHEAT_GM_THEME_INFO_FAIL	    = 1;
const unsigned char ERROR_CHEAT_GM_THEME_INFO_NO_PLAYER = 2;
const unsigned char ERROR_CHEAT_GM_THEME_INFO_NO_DATA   = 3;

/// 테마 초기화
struct MSG_REQ_CHEAT_THEME_INFO : public MSGROOT
{
	wchar_t mCharName[MAX_NAME_BUFFER_SIZE]; /// 캐릭터 명
};

struct sCheat_Theme_Info
{
	short  themeIdx;			//   테마 IDX
	long   uniqueIdx;			//   귀속 고유번호
	BYTE   mode;				//   룸   모드(난이도)
	time_t lastDate;			//   룸   마지막 업데이트(일)
	time_t validThru;			//   룸   유효만기(일)
};

struct MSG_RES_CHEAT_THEME_INFO : public MSG_ERROR
{
	unsigned long		mCharacterIdx;
	long				mRowCount;
	sCheat_Theme_Info	mThemeInfo[1];

	unsigned long GetMsgLength(){ return sizeof( MSG_RES_CHEAT_THEME_INFO ) - sizeof( mThemeInfo ) + ( mRowCount * sizeof( mThemeInfo ) ); }
};

/// 테마 정보
struct MSG_REQ_CHEAT_THEME_RESET : public MSGROOT
{
	unsigned long  mCharacterIdx;	/// 플레이어인덱스
	unsigned short mThemeIdx;		/// 테마종류
	unsigned char  mThemeMode;		/// 테마모드
};

const unsigned char ERROR_CHEAT_GM_THEME_RESET_SUCCESS			= 0;
const unsigned char ERROR_CHEAT_GM_THEME_RESET_FAIL				= 1;
const unsigned char ERROR_CHEAT_GM_THEME_RESET_NOT_SAME_CHANNEL = 2; /// 동일 채널 아님
const unsigned char ERROR_CHEAT_GM_THEME_RESET_THEME_IN			= 3; /// 유저가 테마에 있음
const unsigned char ERROR_CHEAT_GM_THEME_RESET_TIME				= 4; /// 테마던젼 초기화 준비중
const unsigned char ERROR_CHEAT_GM_THEME_RESET_NO_PLAYER		= 5; /// 초기화할 유저가 존재하지 않음
const unsigned char ERROR_CHEAT_GM_THEME_RESET_DB_USE			= 6; /// GM유저 DB 사용중
const unsigned char ERROR_CHEAT_GM_THEME_RESET_NO_DATA			= 7; /// 해당 테마가 존재하지 않음


struct MSG_RES_CHEAT_THEME_RESET : public MSG_ERROR
{
};


struct MSG_SYN_CHEAT_THEMERESET : public MSG_ERROR
{
	unsigned short		mThemeNum;						/// 테마 종류(맵타입)
	unsigned char		mThemeMode;						///	난이도
};

/// GM 이벤트 적용된 효과 검색
const unsigned char ERROR_CHEAT_GM_EVENT_SEARCH_SUCCESS			= 0;
const unsigned char ERROR_CHEAT_GM_EVENT_SEARCH_FAIL			= 1;
const unsigned char ERROR_CHEAT_GM_EVENT_SEARCH_NO_DATA			= 2; /// 적용중인 이벤트 없음

struct MSG_REQ_CHEAT_GM_EVENT_SEARCH : public MSGROOT
{
};

struct GM_EVENT_INFO
{
	unsigned long index;			/// DB Index
	unsigned long influenceIdx;		/// 효과 인덱스
	time_t		  startTime;		/// 시작 시간
	time_t		  validThru;		/// 만기 시간

};

struct MSG_RES_CHEAT_GM_EVENT_SEARCH : public MSG_ERROR
{
	long		  mRowCount;
	GM_EVENT_INFO mTable[1];

	unsigned long GetMsgLength(){ return sizeof( MSG_RES_CHEAT_GM_EVENT_SEARCH ) - sizeof( mTable ) + ( mRowCount * sizeof( mTable ) ); }
};

/// GM 이벤트 적용된 효과 적용
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_SUCCESS		  = 0;
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_FAIL			  = 1;
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_ALREADY_REGIST = 2; /// 이미 적용됨
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_DB_USE		  = 3; /// DB사용중
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_DB_INSERT	  = 4; /// DB사용 실패
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_BAD_VALIDTHRU  = 5; /// 효과의 끝나는 시간이 현재시간보다 과거임
const unsigned char ERROR_CHEAT_GM_EVENT_APPLY_BAD_INFLUENCE  = 6; /// 정의되지 않은 효과값임

/// GM 이벤트 효과 적용시키기
struct MSG_REQ_CHEAT_GM_EVENT_APPLY : public MSGROOT
{
	unsigned long mInfluenceIdx;
	SYSTEMTIME	  mStartTime;
	SYSTEMTIME	  mValidThru;
};

struct MSG_RES_CHEAT_GM_EVENT_APPLY : public MSG_ERROR
{
};

/// GM 이벤트 효과 삭제하기
const unsigned char ERROR_CHEAT_GM_EVENT_RELEASE_SUCCESS		= 0;
const unsigned char ERROR_CHEAT_GM_EVENT_RELEASE_FAIL			= 1;
const unsigned char ERROR_CHEAT_GM_EVENT_RELEASE_DB_USE		    = 2; /// DB사용중
const unsigned char ERROR_CHEAT_GM_EVENT_RELEASE_BAD_INDEX      = 3; /// 해당 인덱스를 가진 효과를 찾을 수 없음

struct MSG_REQ_CHEAT_GM_EVENT_RELEASE : public MSGROOT
{
	unsigned long mDBIndex;		/// DB에 저장된 고유 Index
};

struct MSG_RES_CHEAT_GM_EVENT_RELEASE : public MSG_ERROR
{
};


#pragma pack( pop )
