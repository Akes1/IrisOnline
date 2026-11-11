/* ====================================================================
 * 파    일 :	sqlAccountStmt.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_ACCOUNT_STATEMENT_H__
#define __SQL_ACCOUNT_STATEMENT_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include "odbcsql.h"
#include "loginsrvdefines.h"

struct MATRIX_CREATE
{
	long retvalue;
};

struct SERVER_LIST
{
	long      rowCount;
	TB_SERVER table[1];
};

struct CHANNEL_UPDATE
{
	long  serverNum;
	long  channelNum;
	BYTE  status;
	long  retvalue;
};

struct CONCURRENT_USER
{
	long serverIdx;
	long value;
};

struct SERVER_DOWN
{
	long serverNum;
	long channelNum;
};

struct MEMBER_LOGIN
{
	long memberIdx;
	char memberSlevel;
	long memberPc;
	long loginIdx;
	long lastServerNum;
	long lastChannelNum;
	char twitterUid[ 101 ];
	char twitterPwd[ 101 ];
	char inetnum[ 21 ];
	long retvalue;
};

struct MEMBER_LOGOUT
{
	long loginIdx;
	long memberIdx;
	long retvalue;
};

struct LOGIN_UPDATE
{
	long loginIdx;
};

struct GAME_TO_GAME
{
	long  loginIdx;
	long  serverNum;
	long  channelNum;
	long  lastChannel;
	long  characterIdx;
	BYTE  instantDungeon;
	long  ipv4;
	short port;
	long  retvalue;
};

class cSQLAccountStmt :
	public cSQLStatement
{
public:
// 클래스 생성 및 공용 메소드.
	cSQLAccountStmt(void);

	SQLRETURN TruncateCaptcha ( );
	SQLRETURN InsertCaptcha   ( TB_CAPTCHA* captcha, long& retvalue );

	SQLRETURN MatrixCreate    ( MATRIX_CREATE* matrixCreate );
	SQLRETURN ServerList      ( SERVER_LIST* serverList, u_long& cbLength );
	SQLRETURN ChannelUpdate   ( CHANNEL_UPDATE* channelUpdate );
	SQLRETURN ConcurrentUser  ( CONCURRENT_USER* concurrentUser );
	SQLRETURN ServerDown      ( SERVER_DOWN* serverDown );

	SQLRETURN MemberLogin     ( MEMBER_LOGIN* memberLogin );
	SQLRETURN MemberLogout    ( MEMBER_LOGOUT* memberLogout );

	SQLRETURN LoginUpdate     ( LOGIN_UPDATE* loginUpdate );

	SQLRETURN GameToGame      ( GAME_TO_GAME* gameToGame );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLAccountStmt(void);
};

#endif // __SQL_ACCOUNT_STATEMENT_H__
