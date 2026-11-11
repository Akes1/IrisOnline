/* ====================================================================
 * 파    일 :	verbose.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	07/04/11
 * 주의사항 :	
 * =================================================================== */
#ifndef __VERBOSE_H__
#define __VERBOSE_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

// 윈도우즈 헤더
#include <windows.h>

class cVerbose
{
public:
	cVerbose(void);

	void WriteLog         ( char* msg, ... );

	void AcceptComplete   ( CRITICAL_SECTION* cs, void* ptr );
	void CloseSocket      ( CRITICAL_SECTION* cs, void* ptr );

	void BatchComplete    ( CRITICAL_SECTION* cs, void* ptr );

	void CallbackComplete ( CRITICAL_SECTION* cs, void* ptr );

public:
	virtual ~cVerbose(void);

public:
	static cVerbose* mVerbose;
};

#define Verbose cVerbose::mVerbose

#endif // __VERBOSE_H__