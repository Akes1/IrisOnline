/* ====================================================================
 * 파    일 :	RandomTable.h
 * 목    적 :	
 * 작 성 자 :	곽철중
 * 작 성 일 :	Created 03/24/08
 * 주의사항 :	
 * =================================================================== */
#ifndef __RANDOM_TABLE_H__
#define __RANDOM_TABLE_H__

//#ifndef WINVER
//#define WINVER          0x0500
//#endif
//
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT    0x0500
//#endif

#pragma once

class cRandomTable
{
private:
	double* m_million;
	DWORD   m_mOffset;
	DWORD   m_mLength;

	double* m_hundred;
	DWORD   m_hOffset;
	DWORD   m_hLength;

public:
// 클래스 생성 및 공용 메소드.
	cRandomTable(void);

	bool   Initialize ( );
	void   Shutdown   ( );

	// Million (default method)
	DWORD  GetSeed    ( );

	double Get        ( );
	double Get        ( DWORD& seed );

	// Hundred
	DWORD  GetSeedx100 ( );

	double Getx100     ( );
	double Getx100     ( DWORD& seed );

public:
// 클래스 파괴 메소드.
	virtual ~cRandomTable(void);

public:
	static cRandomTable* mpRandomTable;
};

#define RANDOMTABLE	cRandomTable::mpRandomTable

#endif // __RANDOM_TABLE_H__