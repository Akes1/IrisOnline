#pragma once

#include "GameSrvDefines.h"

struct SecureInfo
{
	char        session[32];                // GUID(UNIQUE)
	char        status[3];                  // 상태.
	u_long      memberIdx;                  // 계  정 인덱스.
	u_long      loginIdx;                   // 로그인 인덱스.
	u_long      lastChannel;                // 이전 채널.
	u_long      characterIdx;               // 캐릭터 인덱스.
	BYTE        slevel;                     // 보안등급.
	long        pc;                        // PC방 사용자.
	long        gotoCID;                    // 이동할 CID.

	ULONG_PTR   mHSClientHandle;            // 핵쉴드
	u_long      mHSNextCheckTime;           // "
    bool        mHSIsWait;                  // "
	bool        mHSIsSkip;					// "
};

struct ClientInfo
{
	union // Database Flag - Common
	{
		struct
		{
			// 0 off / 1 on
			bool check                 : 1; // 로그인 여부 확인.
			bool characterList         : 1; // 캐릭터 목록 검색.
			bool characterName         : 1; // 캐릭터 이름 검색.
			bool characterInsert       : 1; // 캐릭터 생성.
			bool characterDelete       : 1; // 캐릭터 삭제.
			bool characterSelect       : 1; // 캐릭터 선택.
			bool characterUpdate       : 1; // 캐릭터 갱신.
			bool characterMoney        : 1; // 캐릭터 소지금액 갱신.
			bool characterActiveWeapon : 1; // 캐릭터 활성 무기정보 갱신.
			bool characterLogin		   : 1; // 캐릭터 접속중 검색.
			bool friendSelect          : 1; // 친구 목록 검색.
			bool friendInsert          : 1; // 친구 추가.
			bool friendUpdate          : 1; // 친구 갱신.
			bool friendDelete          : 1; // 친구 삭제.
			bool spreadValue           : 1; // 스프레드 결과값 반환.
			bool tarotResult           : 1; // 타로카드 효과값 반환.
			bool gameToGame            : 1; // 게임서버 접속(채널이동).
			bool shortcutUpdate        : 1; // 단축창 갱신.
			bool logout                : 1; // 로그아웃.
			bool itemAgentSearch       : 1; // 판매대행 검색.
			bool out                   : 1; // 로그인 강제 해제.
			bool characterGmName       : 1; // 캐릭터 GM이름 등록.
			bool themeUserCheck        : 1; // 테마 인증 검사.
			bool themeUserRemove       : 1; // 테마 사용 정리.
			bool trial                 : 1; // 자동사냥 신고
			bool loginLimit            : 1; // 로그인 제한 (* 사용하는 곳 없음..)
			bool gmThemeUserReset      : 1; // GM 테마 던젼 초기화
			bool gmEventInsert         : 1; // GM 이벤트 삽입
			bool gmEventUpdate         : 1; // GM 이벤트 삭제
            bool itemPwd               : 1; // 아이템 안전번호
            bool obtEvent              : 1; // OBT 이벤트
			//////////////////////////////////////////////////////////////////////////
			/// ※ 항목 추가나 삭제시 COMMON_DB_CHECK 리스트와 짝을 맞춰야 한다.!!!
			//////////////////////////////////////////////////////////////////////////
		} commonDb;
		long commonDatabase;
	};
	union // Database Flag - GameIn
	{
		struct
		{
			bool inventorySelect      : 1; // 인벤토리     목록검색.
			bool inventoryCooltime    : 1; // 쿨타임       목록검색.
			bool itemBillSelect       : 1; // 아이템 빌    목록검색.
			bool skillSelect          : 1; // 보유스킬     목록검색.
			bool skillInfluenceSelect : 1; // 보유스킬효과 목록검색.
			bool shortcutSelect       : 1; // 퀵슬롯       목록검색.
			bool questList			  : 1; // 퀘스트	   목록검색.
			bool questEndList		  : 1; // 퀘스트 완료  목록검색.
			bool questValidList		  : 1; // 퀘스트 만기일목록검색.
			bool titleList			  : 1; // 보유호칭	   목록검색.
			bool makeSkillList        : 1; // 제조스킬     목록검색.
			bool fortuneData          : 1; // 운세정보     목록검색.
			bool themeUser            : 1; // 테마정보     목록검색.
			bool trial                : 1; // 자동사냥신고 관련시간.
		} gameInDb;
		long gameInDatabase;
	};

	bool gameInDBEnd;
	bool gameInDBCnt;
	bool gameOutDBCnt;

	union // Databse Flag - Skill
	{
		struct
		{
			// 0 off / 1 on
			bool skillDbUse          : 1;
			bool makeSkillDbUse      : 1;
		} skillDb;
		long skillDatabase;
	};
	union
	{
		struct  
		{
			// 0 off / 1on
			bool questInsert		  : 1; // 퀘스트 삽입(db기록중인지 체크)
			bool questDelete		  : 1; // 퀘스트 포기
			bool questDeleteAuto	  : 1; // 퀘스트 자동 삭제
			bool questComplete		  : 1; // 퀘스트 진행완료
			bool questReward		  : 1; // 퀘스트 보상
		} questDb;
		long questDatabase;
	};
	union
	{
		struct 
		{
			// 0 off / 1on
			bool guildCreate		  : 1; // 길드 생성
			bool guildAdd			  : 1; // 길드원 추가
			bool guildOut			  : 1; // 길드 탈퇴
			bool guildCut			  : 1; // 길드 추방
			bool guildDelete		  : 1; // 길드 해체
			bool guildGivePosition	  : 1; // 길드 직위부여
			bool guildMarkUpdate	  : 1; // 길드 마크 갱신
		} guildDb;
		long guildDatabase;
	};
	union // Request Flag
	{
		struct
		{
			// 0 off / 1 on
			bool channelList           : 1; // 채널목록.
			bool characterList         : 1; // 캐릭터목록.
			bool characterSelect       : 1; // 캐릭터선택.
			bool characterEquip        : 1; // 캐릭터장비 교채.
			bool characterOptionSend   : 1; // 캐릭터옵션 전송.
			bool gameIn                : 1; // 게임인.
			bool gameOut               : 1; // 게임아웃.
			bool inventorySend         : 1; // 인벤토리 전송.
			bool inventoryCooltimeSend : 1; // 인벤토리 쿨타임 전송.
			bool shortcutSend          : 1; // 당 축 창 전송.
			bool questSend			   : 1; // 퀘스트 목록 전송
			bool titleSend			   : 1; // 호칭 목록 전송
			bool gotoInstantDungeon    : 1; // Instant Dungeon
			bool joinInstantDungeon    : 1; // Instant Dungeon
			bool returnToMap           : 1; // Return to Map
			bool joinMap               : 1; // Join Map
			bool friendList			   : 1; // 친구 전송.
		} request;
		long requestData;
	};
	union // Complete Flag
	{
		struct
		{
			// 0 off / 1 on
			bool check                 : 1; // 로그인 확인.
			bool characterSelect       : 1; // 캐릭터 선택.
			bool gameIn                : 1; // 게임인.
			bool inventory             : 1; // 인벤토리.
			bool inventoryCooltime     : 1; // 인벤토리 쿨 타임.
			bool characterUpdate       : 1; // 캐릭터 업데이트.
			bool characterActiveWeapon : 1; // 캐릭터 활성 무기정보.
			bool characterOption       : 1; // 캐릭터 옵션.
			bool influenceRelease      : 1; // 스킬 효과 목록 제거
			bool skillRelease          : 1; // 스킬 정보 제거
			bool skillInfluenceUpdate  : 1; // 스킬 효과 저장
			bool skillCoolTime         : 1; // 쿨타임 저장
			bool duelEnd               : 1; // 결투 종료 처리
			bool shortcut              : 1; // 단축창 등록정보. 
			bool friendSelect          : 1; // 친구목록 확인.
			bool quest				   : 1; // 퀘스트 확인
			bool pvpJoin               : 1; // pvp중
			bool title				   : 1; // 호칭 확인.
			bool fortuneData		   : 1; // 운세속성 확인.
			bool recipeCoolTime        : 1; // 조합 레시피 쿨타임.
			bool themeUser             : 1; // 테마.
			bool gameToGame            : 1; // 채널간 이동.
			bool firstGameIn		   : 1; // 첫 게임 접속.
			bool pgBilling             : 1; // PC방 빌링.
		} complete;
		long completeData;
	};

	struct
	{
		DWORD loginUpdate;                 // 로그인 업데이트 지연.
		DWORD characterTTL;                // 캐릭터 TTL 지연.
		DWORD channelList;                 // 채  널목록 지연.
		DWORD characterList;               // 캐릭터목록 지연.
		DWORD firendSend;                  // 친  구목록 지연.
		DWORD questSave;                   // 퀘스트저장 지연.
		DWORD chatting;                    // 채팅       지연.
        DWORD cashList;                    // 캐시아이템 목록 지연
	} delay;

	// struct SecureInfo - 시작 (SecureInfo와 동일하게 등록 해야됨)
	char        session[32];               // GUID(UNIQUE)
	char        status[3];                 // 상태.
	u_long      memberIdx;                 // 계  정 인덱스.
	u_long      loginIdx;                  // 로그인 인덱스.
	u_long      lastChannel;               // 이전 채널.
	u_long      characterIdx;              // 캐릭터 인덱스.
	BYTE        slevel;                    // 보안등급.
	long        pc;                        // PC방 사용자.
	long        gotoCID;                   // 이동할 CID.


	// 일본OBT 이벤트 아이템 지급 관련
	// 값 - 0:대상아님/1:이벤트대상/2:아이템지급완료
	BYTE		cbtUser;					// CBT 참여 여부
	BYTE		obtUser;					// OBT 신규 유저 여부
	BYTE		fourGamer;					// 4gamer 유저 여부

	
	ULONG_PTR   mHSClientHandle;           // 핵쉴드
	u_long      mHSNextCheckTime;          // "
    bool        mHSIsWait;                 // "
	bool        mHSIsSkip;					// "
	//////////////////////////////////////////////////////////////////////////
	// ※ 항목 변경시 struct SecureInfo 와 짝을 맞춰야 한다. !!!
	//////////////////////////////////////////////////////////////////////////
	// struct SecureInfo - 종료


	bool	    isCheatDungeonIn;
	short       themeRoomIdx;              // 테마 룸 번호.
	char        themeMode;                 // 테마 모드(난이도)

	unsigned long	mBPNextCheckTime;

	// 임시저장 정보.
	TB_SHORTCUT shortcut;                  // 단축키 데이터.
	long        optionData1;               // 옵  션 데이터.
};
