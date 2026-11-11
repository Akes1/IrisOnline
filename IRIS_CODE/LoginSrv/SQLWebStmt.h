/* ====================================================================
 * 파    일 :	sqlWebStmt.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/01/12
 * 주의사항 :	
 * =================================================================== */
#ifndef __SQL_WEB_STATEMENT_H__
#define __SQL_WEB_STATEMENT_H__

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

struct WEB_LAUNCHER
{
	long loginIdx;				// [In]  런처 인증번호
	long propid;				// [Out] 계정 고유번호
	char loginId [20];			// [Out] 계정 ID
	char loginPwd[20];			// [Out] 계정 PWD
};

// 순서변경불가 - 추가시 마지막 번호 부여.
#define WEB_LOGIN_SLEVEL0	0	// 일반 계정
#define WEB_LOGIN_SLEVEL1	1	// GM   "
#define WEB_LOGIN_SLEVEL2	2	// 블록 "
#define WEB_LOGIN_SLEVEL3	3	// 연령 "
#define WEB_LOGIN_SLEVEL4	4	// 개척자
#define WEB_LOGIN_SLEVEL5	5	// 테스터

struct WEB_LOGIN
{
	long idx;
	char loginid  [ 21 ];
	char passwd   [ 21 ];
	char passwdsec[  5 ];
	char ip       [ 24 ];
	char slevel;
	char serversec;
	long retvalue;
	long retage;
	long retpc;
};

struct WEB_TP_SMS
{
	char serverName[ 32 ];
};

class cSQLWebStmt :
	public cSQLStatement
{
public:
// 클래스 생성 및 공용 메소드.
	cSQLWebStmt(void);

	SQLRETURN WebLauncher ( WEB_LAUNCHER* webLauncher );
	SQLRETURN WebCheck    ( WEB_LOGIN* webLogin );
	SQLRETURN WebLogin    ( WEB_LOGIN* webLogin );
	SQLRETURN WebTpSMS    ( WEB_TP_SMS* webTpSMS );

public:
// 클래스 파괴 메소드.
	virtual ~cSQLWebStmt(void);
};

#endif // __SQL_ACCOUNT_STATEMENT_H__
