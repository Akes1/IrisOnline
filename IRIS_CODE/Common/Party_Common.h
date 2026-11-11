/* ==========================================================================
* 파    일 : Party_Common.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-11-05
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Skill_Common.h"

#pragma pack(push,1)
#include "CommonDefines.h"
#include "CommonNetworkMsgStruct.h"

const int MAX_PARTY = 5;			// 클라이언트상의 데이터 저장 사이즈

/*-- 파티 가입권유
*/
#define ERROR_PARTY_ADD_SUCCESS			0	// 성공 - 파티신청 성공
#define ERROR_PARTY_ADD_CREATE_SUCCESS	1	// 성공 - 파티생성됨

#define ERROR_PARTY_ADD_STATUS			2	// 실패 - 신청할수 없는 상태
#define ERROR_PARTY_ADD_COMPLETE		3	// 실패 - 해당유저가 파티 가입중
#define ERROR_PARTY_ADD_CONTINUE		4	// 실패 - 자기자신이 초대중인 유저가 있으므로 초대불가.
#define ERROR_PARTY_ADD_USER			5	// 실패 - 초대한캐릭터 없음.
#define ERROR_PARTY_ADD_COUNT			6	// 실패 - 파티인원 풀
#define ERROR_PARTY_ADD_REFUSE			7	// 실패 - 파티초대 거절
#define ERROR_PARTY_ADD_NOTLEADER		8	// 실패 - 권한없음
#define ERROR_PARTY_ADD_USEROUT			9	// 실패 - 초대한 유저가 게임종료함
#define ERROR_PARTY_ADD_MAP				10	// 실패 - 초대한 유저가 맵이동중
#define ERROR_PARTY_ADD_CREATEFAIL		11	// 실패 - 파티 생성 실패
#define ERROR_PARTY_ADD_OPTIONREFUSE	12	// 실패 - 거부 상태
#define ERROR_PARTY_ADD_FAIL			13	// 실패 - 신청 실패
#define ERROR_PARTY_ADD_PARTYUNION		14	// 실패 - 파티 연합중
#define ERROR_PARTY_ADD_REFUSETIME		15	// 응답시간초과

struct MSG_REQ_PARTY_ADD : MSGROOT 
{
	wchar_t Name[MAX_NAME_BUFFER_SIZE];		/// 파티초대할 캐릭터 이름
};

struct MSG_RES_PARTY_ADD : MSG_ERROR
{};

struct MSG_SYN_PARTY_ADD : MSGROOT
{
	wchar_t Name[MAX_NAME_BUFFER_SIZE];		/// 파티초대한 캐릭터 이름
};


/*-- 파티 가입결과
*/
#define ERROR_PARTY_REP_SUCESS		0	// 성공 - 가입
#define ERROR_PARTY_REP_REFUSE		1	// 성공 - 초대 거절
#define ERROR_PARTY_REP_FULL		2	// 실패 - 파티 풀
#define ERROR_PARTY_REP_FAIL		3	// 실패 - 존재하지 않음
#define ERROR_PARTY_REP_PARTYUNION	4	// 실패 - 파티연합중
#define ERROR_PARTY_REP_REFUSETIME	5	// 실패 - 응답시간 초과

#define PARTY_REP_ACCEPT			0	// 승락
#define PARTY_REP_REJECT			1	// 거절
#define PARTY_REP_REJECTTIME		2	// 거절 - 응답시간 초과

struct MSG_REQ_PARTY_REP : MSGROOT 
{
	char		Reply;					/// 0 : 거절 1: 수락
};
struct MSG_RES_PARTY_REP : MSG_ERROR
{};


/*-- 파티 정보 리스트
*/
enum ePLAYER_JOB;
enum PARTY_DIVIDE_TYPE
{
	PARTY_DIVIDE_NONE = 0,
	PARTY_DIVIDE_TURN,		/// 순서대로
	PARTY_DIVIDE_RANDOM,	/// 랜덤
	PARTY_DIVIDE_DAMAGE,	/// 데미지
	PARTY_DIVIDE_FREE,		/// 자유분배 (획득권한자유)
    PARTY_DIVIDE_LEADER,    /// 파티장분배

	PARTY_DEVIDE_MAX,
};

struct sPartyUserData
{
	unsigned long userIndex;
	wchar_t		  name[MAX_NAME_BUFFER_SIZE];
	unsigned char level;
	unsigned short job;
	short		  mapNum;
	short		  roomNum;
	DWORD		  channelNum;
	float		  posX;
	float		  posY;
	unsigned int  hp;
	unsigned int  maxhp;
	unsigned int  mp;
	unsigned int  maxmp;
};

/// 파티 정보 및 파티원 리스트
struct MSG_SYN_PARTY_LIST : MSGROOT
{
	unsigned long mIndex;				/// 파티 인덱스
	unsigned long mLeaderIndex;			/// 파티장 인덱스
	char		  mDivideType;			/// 파티 분배방식

	unsigned char RowCount;				
	sPartyUserData	mTable[1];
};

/// 파티원 1명 정보
struct MSG_SYN_PARTY_ADDONE : MSGROOT
{
	sPartyUserData	mTable;
};

#define ERROR_PARTY_OUT_SUCCESS		0	// 성공  
#define ERROR_PARTY_OUT_FAIL		1	// 실패

/// 파티 탈퇴 요청
struct MSG_REQ_PARTY_OUT : MSGROOT
{};
struct MSG_RES_PARTY_OUT : MSG_ERROR
{};
struct MSG_SYN_PARTY_OUT : MSGROOT
{
	unsigned long UserIndex;		/// 파티탈퇴한 유저 인덱스
};

/// 파티 해산 ( 인원부족 )
struct MSG_SYN_PARTY_DELETE : MSGROOT
{};

/// 파티 강제 해체 
struct MSG_SYN_PARTY_BREAK : MSGROOT
{};

/*-- 파티장위임 결과
*/
#define ERROR_PARTY_CHANGELEADER_SUCESS		0	// 성공 - 위임
#define ERROR_PARTY_CHANGELEADER_LEADER		1	// 실패 - 권한 없음
#define ERROR_PARTY_CHANGELEADER_FAIL		2	// 실패

/// 파티장위임
struct MSG_REQ_PARTY_CHANGELEADER : MSGROOT
{
	unsigned long UserIndex;					/// 파티장으로 위임할 유저 인덱스
};
struct MSG_RES_PARTY_CHANGELEADER : MSG_ERROR
{};
struct MSG_SYN_PARTY_CHANGELEADER : MSGROOT
{
	unsigned long UserIndex;					/// 파티장이 된 유저 인덱스
};


/*-- 파티 분배타입 변경 결과
*/
#define ERROR_PARTY_CHANGEDIVIDE_SUCESS		0	// 성공 - 분배타입 
#define ERROR_PARTY_CHANGEDIVIDE_LEADER		1	// 실패 - 권한 없음
#define ERROR_PARTY_CHANGEDIVIDE_FAIL		2	// 실패

struct MSG_REQ_PARTY_CHANGEDIVIDE : MSGROOT
{
	char	type;					/// 변경할 분배타입
};
struct MSG_RES_PARTY_CHAGNEDIVIDE : MSG_ERROR
{};
struct MSG_SYN_PARTY_CHANGEDIVDE : MSGROOT
{
	char	type;					/// 변경된 분배타입
};	


/*-- 파티원 추방 요청결과
*/
#define ERROR_PARTY_THROW_SUCESS		0	// 성공 
#define ERROR_PARTY_THROW_LEADER		1	// 실패 - 권한 없음
#define ERROR_PARTY_THROW_FAIL			2	// 실패 - 추방 실패

struct MSG_REQ_PARTY_THROW : MSGROOT
{
	unsigned long UserIndex;					/// 추방하려는 유저 
};
struct MSG_RES_PARTY_THROW : MSG_ERROR 
{};
struct MSG_SYN_PARTY_THROW : MSGROOT
{
	unsigned long UserIndex;					/// 추방된 유저
};

/// 파티로부터 추방당함
struct MSG_SYN_PARTY_CUT : MSGROOT 
{};

/// 파티원 맵체인지
struct MSG_SYN_PARTY_MAPCHANGE : MSGROOT
{
	unsigned long UserIndex;					/// 이동한유저
	short		  MapNum;						/// 맵번호
	short         RoomNum;
	DWORD		  ChannelNum;					/// 채널 번호
};

/// 파티원 정보 : 일정시간마다 쏨
struct MSG_SYN_PARTY_DATA : MSGROOT
{
	unsigned long userIndex;
	unsigned char level;
	unsigned int  maxHP;
	unsigned int  maxMP;
	unsigned int  hp;
	unsigned int  mp;
	float		  posX;							/// 좌표
	float		  posY;

	short				RowCount;	
	sInfluenceClient	mTable[1];				/// 보유효과 리스트
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_PARTY_DATA) - sizeof(mTable) + (RowCount * sizeof(sInfluenceClient)); }
};

/// 다른맵에 존재할때, 레벨정보만 보낸다
struct MSG_SYN_PARTYLEVEL_DATA : MSGROOT
{
	unsigned long userIndex;
	unsigned char level;
};

/// 파티원 아이템 획득
struct MSG_SYN_PARTY_ITEMGET : MSGROOT
{
	unsigned long UserIndex;					/// 파티원 인덱스

	short		   RowCount;
	unsigned long  ItemIndex[1];			/// 아이템
};

struct MSG_SYN_PARTY_CHANGEJOB : MSGROOT
{
	unsigned long	userIndex;
	unsigned short  job;
};

/// 주사위 굴리기
#define PARTY_DICETIME		            30000	/// 굴리기 딜레이 30 초

#define ERROR_DICE_START_SUCCESS        0       /// 성공
#define ERROR_DICE_START_FAIL           1       /// 실패
#define ERROR_DICE_START_PARTY_LEADER   2       /// 실패 - 파티장
#define ERROR_DICE_START_PARTY_STATE    3       /// 실패 - 파티 상태


#define ERROR_DICE_ROLL_SUCCESS       0
#define ERROR_DICE_ROLL_NOT_START     1         /// 실패 - 주사위 굴리기 시작 안함
#define ERROR_DICE_ROLL_ALREADY       2         /// 실패 - 이미 주사위 굴리기 선택함
#define ERROR_DICE_ROLL_PARTY_STATE   3
#define ERROR_DICE_ROLL_MEMBER        4         /// 실패 - 주사위 참여 멤버가 아니다

struct MSG_REQ_PARTY_DICE_ROLL : public MSGROOT
{
    unsigned char   mRoll;                      /// 1:굴리기, 0:포기
};

struct MSG_RES_PARTY_DICE_ROLL : public MSG_ERROR
{
    unsigned short  mRandom;				    /// 0 이면 포기, 그외는 굴린 값
};

struct MSG_SYN_PARTY_DICE_REMAIN_TIME : public MSG_ERROR
{
    unsigned long  mTime;
};

struct sDiceResult
{
    wchar_t         mCharName[MAX_NAME_BUFFER_SIZE];
    unsigned short  mRandom;
};
struct MSG_SYN_PARTY_DICE_END : public MSGROOT
{
	short           mTotalCnt;
    short           mNotRollCnt;

    short           mRowCount;
	sDiceResult	    mResult[1];
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_PARTY_DICE_END) - sizeof(mResult) + (mRowCount * sizeof(sDiceResult)); }
};


//-------------------------------------------------------------------------------
// 파티 연합

// 그룹 정보
enum ePARTYUNION_GROUP
{
	eUNION_GROUP1 = 0,
	eUNION_GROUP2,
	eUNION_GROUP3,

	eUNION_MAX
};

const int MAX_PARTYUSER = 6;
const int MAX_PARTYGROUP = 3;
const int MAX_PARTYUNION = MAX_PARTYUSER * MAX_PARTYGROUP;

struct sUnionUserData
{
	unsigned int	idx;				// 배열상 인덱스
	unsigned long	userIndex;
	wchar_t			name[MAX_NAME_BUFFER_SIZE];
	unsigned char	level;
	unsigned short  job;
	unsigned short	mapNum;
	DWORD			channelNum;
	float			posX;
	float			posY;
	unsigned int	hp;
	unsigned int	maxhp;
	unsigned int	mp;
	unsigned int	maxmp;
};

// 연합 리스트
struct MSG_SYN_PARTYUNION_LIST : MSGROOT
{
	unsigned long mIndex;			
	unsigned long mLeaderIndex;

	unsigned char RowCount;
	sUnionUserData mTable[1];
};

// 연합원 추가
struct MSG_SYN_PARTYUNION_ADDONE : MSGROOT
{
	sUnionUserData  mTable;
};

#define ERROR_PARTYUNION_OUT_SUCESS		0	// 성공  
#define ERROR_PARTYUNION_OUT_FAIL			1	// 실패

// 연합원 탈퇴
struct MSG_RES_PARTYUNION_OUT : MSG_ERROR
{};

struct MSG_SYN_PARTYUNION_OUT : MSGROOT
{
	unsigned long mPlayerIndex;
};

#define ERROR_PARTYUNION_CHANGELEADER_SUCESS		0	// 성공  
#define ERROR_PARTYUNION_CHANGELEADER_LEADER		1	// 실패 - 권한 없음
#define ERROR_PARTYUNION_CHANGELEADER_FAIL			2	// 실패

// 연합장 위임
struct MSG_REQ_PARTYUNION_CHANGELEADER : MSGROOT
{
	unsigned long mPlayerIndex;
};

struct MSG_RES_PARTYUNION_CHANGELEADER : MSG_ERROR
{};

struct MSG_SYN_PARTYUNION_CHANGELEADER : MSGROOT
{
	unsigned long mPlayerIndex;
};

#define ERROR_PARTYUNION_MOVEGROUP_SUCESS		0	// 성공  
#define ERROR_PARTYUNION_MOVEGROUP_LEADER		1	// 실패 - 권한 없음
#define ERROR_PARTYUNION_MOVEGROUP_FAIL			2	// 실패

// 연합내 그룹이동
struct MSG_REQ_PARTYUNION_MOVEGROUP : MSGROOT
{
	unsigned char mIndex0;				// 배열 번호
	unsigned char mIndex1;				// 배열 번호
};

struct MSG_RES_PARTYUNION_MOVEGROUP : MSG_ERROR
{};

struct MSG_SYN_PARTYUNION_MOVEGROUP : MSGROOT
{
	unsigned char mIndex0;				// 배열 번호
	unsigned char mIndex1;				// 배열 번호
};

struct sUnionUpdate
{
	unsigned long idx;
	unsigned long userIndex;
	unsigned int  maxhp;
	unsigned int  hp;
	float		  posX;
	float		  posY;
};

// 전체 정보 갱신
struct MSG_SYN_PARTYUNION_UPDATE : MSGROOT
{
	unsigned char		rowCount;
	sUnionUpdate		table[1];

	unsigned long GetMsgLength() { return sizeof(MSG_SYN_PARTYUNION_UPDATE) - sizeof(table) + (rowCount * sizeof(sUnionUpdate)); }
};

// 그룹 정보 갱신
struct MSG_SYN_PARTYUNION_GROUPUPDATE : MSGROOT
{
	unsigned int  idx;				// 배열상 인덱스
	unsigned long userIndex;
	unsigned char level;
	unsigned int  maxhp;
	unsigned int  maxmp;
	unsigned int  hp;
	unsigned int  mp;
	float		  posX;
	float		  posY;

	short				rowCount;
	sInfluenceClient	table[1];				/// 보유효과 리스트
	unsigned long GetMsgLength() { return sizeof(MSG_SYN_PARTYUNION_GROUPUPDATE) - sizeof(table) + (rowCount * sizeof(sInfluenceClient)); }
};

/*-- 파티 채널간 통신 정의 --*/
enum PARTY_SYNC_PROTOCOL
{
	PSP_SYN_PARTY=0,
	PSP_ASK_PARTY,
	PSP_REP_PARTY,
	PSP_ADD_PARTY,
	PSP_DEL_PARTY,
	PSP_CHT_PARTY,
	PSP_ADD_USER,
	PSP_DEL_USER,
	PSP_OUT_USER,
	PSP_SYN_LEADER,
	PSP_SYN_DIVIDE_TYPE,
	PSP_SYN_LEVEL_DATA,
	PSP_SYN_MAP_DATA,
	PSP_SYN_USER,
	PSP_SYN_DICE_START,
	PSP_SYN_DICE_RESULT,
};

struct PARTY_DATA_SYNC
{
	unsigned long protocol;
	union
	{
		struct
		{
			unsigned long  partyIdx;
			unsigned long  leaderIdx;
			sPartyUserData player[MAX_PARTYUSER];
		} synParty;
		struct
		{
			unsigned long  partyIdx;
			sPartyUserData leader;
			wchar_t        playerName[MAX_NAME_BUFFER_SIZE];
		} askParty;
		struct
		{
			unsigned long leaderIdx;
			wchar_t       playerName[MAX_NAME_BUFFER_SIZE];
			unsigned long error;
			unsigned char replay;
		} repParty;
		struct
		{
			unsigned long  partyIdx;
			sPartyUserData leader;
			sPartyUserData player;
		} addParty;
		struct
		{
			unsigned long  partyIdx;
		} delParty;
		struct
		{
			unsigned long  partyIdx;
			sInventory     inventory;
			unsigned long  playerIdx;
            unsigned short job;
			wchar_t        Name[MAX_NAME_BUFFER_SIZE];
			wchar_t        Message[MAX_CHAT_BUFFER_SIZE];
		} chtParty;
		struct
		{
			unsigned long  partyIdx;
			sPartyUserData player;
		} addUser;
		struct
		{
			unsigned long  partyIdx;
			unsigned long  leaderIdx;
			unsigned long  playerIdx;
		} delUser;
		struct
		{
			unsigned long  partyIdx;
			unsigned long  leaderIdx;
			unsigned long  playerIdx;
		} outUser;
		struct
		{
			unsigned long  partyIdx;
			unsigned long  leaderIdx;
		} synLeader;
		struct
		{
			unsigned long  partyIdx;
			unsigned char  divideType;
		} synDivideType;
		struct
		{
			unsigned long  partyIdx;
			sPartyUserData player;
		} synLevelData;
		struct
		{
			unsigned long  partyIdx;
			sPartyUserData player;
		} synMapData;
		struct
		{
			long           gotoCID;
			unsigned long  partyIdx;
			unsigned long  playerIdx;
		} synUser;
		struct
		{
			unsigned long partyIdx;
		} synDiceStart;
		struct
		{
			unsigned long  partyIdx;
			unsigned long  playerIdx;
			unsigned short value;
		} synDiceResult;
	};

	int SYNPARTY      ( ) { return sizeof(protocol) + sizeof(synParty);      }
	int ASKPARTY      ( ) { return sizeof(protocol) + sizeof(askParty);      }
	int REPPARTY      ( ) { return sizeof(protocol) + sizeof(repParty);      }
	int ADDPARTY      ( ) { return sizeof(protocol) + sizeof(addParty);      }
	int DELPARTY      ( ) { return sizeof(protocol) + sizeof(delParty);      }
	int CHTPARTY      ( ) { return sizeof(protocol) + sizeof(chtParty);      }
	int ADDUSER       ( ) { return sizeof(protocol) + sizeof(addUser);       }
	int DELUSER       ( ) { return sizeof(protocol) + sizeof(delUser);       }
	int OUTUSER       ( ) { return sizeof(protocol) + sizeof(outUser);       }
	int SYNLEADER     ( ) { return sizeof(protocol) + sizeof(synLeader);     }
	int SYNDIVIDETYPE ( ) { return sizeof(protocol) + sizeof(synDivideType); }
	int SYNLEVELDATA  ( ) { return sizeof(protocol) + sizeof(synLevelData);  }
	int SYNMAPDATA    ( ) { return sizeof(protocol) + sizeof(synMapData);    }
	int SYNUSER       ( ) { return sizeof(protocol) + sizeof(synUser);       }
	int SYNDICESTART  ( ) { return sizeof(protocol) + sizeof(synDiceStart);  }
	int SYNDICERESULT ( ) { return sizeof(protocol) + sizeof(synDiceResult); }
};

#pragma pack(pop)

