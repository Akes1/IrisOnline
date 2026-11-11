/* ====================================================================
 * 파    일 :	sqlLogStmt.h
 * 목    적 :	데이터를 MSSQLServer에 기록
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_LOG_STATEMENT_H__
#define __SQL_LOG_STATEMENT_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "odbcsql.h"
#include "logprotocol.h"

class cSQLLogStmt :
	public cSQLStatement
{
public:
// 클래스 생성 및 공용 메소드.
	cSQLLogStmt(void);

	SQLRETURN ServerEvent     ( SERVER_EVENT*        );
	SQLRETURN MemberEvent     ( MEMBER_EVENT*        );
	SQLRETURN CharacterEvent  ( CHARACTER_EVENT*     );
	SQLRETURN MoneyEvent      ( MONEY_EVENT*         );
	SQLRETURN DepositEvent    ( DEPOSIT_EVENT*       );
	SQLRETURN InventoryEvent  ( INVENTORY_EVENT*     );
	SQLRETURN ConcurrentEvent ( CONCURRENT_EVENT*    );
	SQLRETURN QuestEvent      ( QUEST_EVENT*         );
	SQLRETURN GuildEvent      ( GUILD_EVENT*         );
	SQLRETURN DropItemEvent   ( DROPITEM_EVENT*      );
	SQLRETURN SkillEvent      ( SKILL_EVENT*         );
	SQLRETURN ItemBillEvent   ( ITEM_BILL_EVENT*     );
	SQLRETURN InfCashEvent    ( MB_SYN_INFCASH_SAVE* );
	SQLRETURN ReservedEvent   ( RESERVED_EVENT*      );
	SQLRETURN PostEvent       ( POST_EVENT*          );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLLogStmt(void);
};

#endif // __SQL_LOG_STATEMENT_H__
