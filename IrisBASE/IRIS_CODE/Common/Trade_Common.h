/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	임소연
 * 작 성 일 :	2010.03.17
 * 주의사항 :	
 * =================================================================== */
#pragma once
#pragma pack(push,1)

#include "Npc_Common.h"

/*-- 소환 주문서 - 우편함
*/
#define ERROR_TRADE_POST_SUMMON_OPEN_SUCCESS         0			// 성공- 소모성 소환 주문서
#define ERROR_TRADE_POST_SUMMON_OPEN_USEFUL_SUCCESS  1			// 성공- 기간성 편의기능 주문서
#define ERROR_TRADE_POST_SUMMON_OPEN_CHANGESTATE     2			// 성공- ChangeState(eOBJECT_STATE_STOP) == false 의 경우 소모만 되도록..
#define ERROR_TRADE_POST_SUMMON_OPEN_FAIL		     3			// 실패
#define ERROR_TRADE_POST_SUMMON_OPEN_LICENSE         4          // 실패- 미인증 아이템
#define ERROR_TRADE_POST_SUMMON_OPEN_ALREADY         5          // 실패- 소환주문서 사용중
#define ERROR_TRADE_POST_SUMMON_OPEN_ITEMPWD_CHECK   6          // 실패- 아이템 인증번호 확인해야 함

// NM_TRADE_POST_SUMMON_OPEN_REQ
struct MSG_REQ_TRADE_POST_SUMMON_OPEN : public MSGROOT
{
    unsigned short number;
};

// NM_TRADE_POST_SUMMON_OPEN_RES
struct MSG_RES_TRADE_POST_SUMMON_OPEN : public MSG_ERROR
{
	sInventory inventory;
};

// NM_TRADE_POST_SUMMON_OPEN_SYN
struct MSG_SYN_TRADE_POST_SUMMON_OPEN : public MSGROOT
{
	unsigned long characterIdx;
};

// NM_TRADE_POST_SUMMON_CLOSE_SYN
struct MSG_SYN_TRADE_POST_SUMMON_CLOSE : public MSGROOT
{
	unsigned long characterIdx;
};

/*-- 소환 주문서 - 판매대행
*/
#define ERROR_TRADE_AGENT_SUMMON_OPEN_SUCCESS               0	// 성공- 소모성 소환 주문서
#define ERROR_TRADE_AGENT_SUMMON_OPEN_USEFUL_SUCCESS        1	// 성공- 기간성 편의기능 주문서
#define ERROR_TRADE_AGENT_SUMMON_OPEN_USEFUL_CHANGESTATE    2	// 성공- ChangeState(eOBJECT_STATE_STOP) == false 의 경우 소모만 되도록..
#define ERROR_TRADE_AGENT_SUMMON_OPEN_FAIL		            3	// 실패
#define ERROR_TRADE_AGENT_SUMMON_OPEN_LICENSE               4   // 실패- 미인증 아이템
#define ERROR_TRADE_AGENT_SUMMON_OPEN_ALREADY               5   // 실패- 소환주문서 사용중
#define ERROR_TRADE_AGENT_SUMMON_OPEN_ITEMPWD_CHECK         6   // 실패- 아이템 인증번호 확인해야 함

// NM_TRADE_AGENT_SUMMON_OPEN_REQ
struct MSG_REQ_TRADE_AGENT_SUMMON_OPEN : public MSGROOT
{
    unsigned short number;
};

// NM_TRADE_AGENT_SUMMON_OPEN_RES
struct MSG_RES_TRADE_AGENT_SUMMON_OPEN : public MSG_ERROR
{
	sInventory                 inventory;           // 소환 주문서

	long                       StartPage;			// 판매대행 페이지 번호 시작(1부터...)
	long                       EndPage;				// 판매대행 페이지 번호 끝
	long                       RowCount;			// 판매대행 영향받은 행 수
	NPC_ITEM_AGENT_OPEN_RESULT Agent[1];			// 판매대행 결과

	unsigned long GetMsgLength( ) { return sizeof(MSG_RES_TRADE_AGENT_SUMMON_OPEN) - sizeof(Agent) + (RowCount * sizeof(Agent)); }
};

// NM_TRADE_AGENT_SUMMON_OPEN_SYN
struct MSG_SYN_TRADE_AGENT_SUMMON_OPEN : public MSGROOT
{
	unsigned long characterIdx;
};

// NM_TRADE_AGENT_SUMMON_CLOSE_SYN
struct MSG_SYN_TRADE_AGENT_SUMMON_CLOSE : public MSGROOT
{
	unsigned long characterIdx;
};

#pragma pack(pop)
