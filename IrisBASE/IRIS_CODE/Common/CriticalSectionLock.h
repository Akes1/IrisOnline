/* ====================================================================
 * 颇    老 :	criticalSectionLock.h
 * 格    利 :	Critical Section 包府
 * 累 己 老 :	胞枚吝
 * 累 己 老 :	05/04/04
 * 林狼荤亲 :	
 * =================================================================== */
#ifndef __CRITICAL_SECTION_LOCK__
#define __CRITICAL_SECTION_LOCK__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

#include <windows.h>

class cCriticalSection
{
private:
	CRITICAL_SECTION* mCs;

public:
	cCriticalSection(CRITICAL_SECTION* cs) : mCs(cs)
	{
	}

	void Lock(void) 
	{
		EnterCriticalSection(mCs);
	}
	void Unlock(void) 
	{
		LeaveCriticalSection(mCs);
	}

public:
	virtual ~cCriticalSection(void)
	{
	}
};

class cCSLock
{
private:
	CRITICAL_SECTION* mCs;

public:
	cCSLock(CRITICAL_SECTION* cs) : mCs(cs)
	{
		EnterCriticalSection(mCs);
	}

	operator bool ()
	{
		return true;
	}

public:
	virtual ~cCSLock()
	{
		LeaveCriticalSection(mCs);
	}
};

#define CSBlock(x) if ( cCSLock _cs_ = x )

#endif // __CRITICAL_SECTION_LOCK__