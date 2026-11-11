/* ==========================================================================
* 파    일 : Npc.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-06-27
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Npc_Common.h"
#include "BaseObject.h"

class cPlayer;

class cNpc : public cBaseObject
{
private:

	/// NPC 기본정보
	const sNPCList*	mpNpcList;
	/// NPC 리젠정보
	//sNPCRegen		mNpcRegen;

	unsigned long	mNpcClassIdx;
	float			mDirection;

public:
	cNpc(void);
	virtual ~cNpc(void);

	void* operator new    ( size_t n ); 
	void  operator delete ( void* ptr, size_t n );

	/// NPC 기본정보 셋팅
	bool          Init( int uniqueIdx, unsigned long classIdx, unsigned short mapNumber, float x, float y, float dir );

	unsigned int  GetRaceGender()	{ return mNpcClassIdx; }

	/// npc 방향 정보
	float         GetDirection ( ) { return mDirection; }
	/// NPC 종류 번호
	unsigned long GetHP        ( ) { return 0; }
	unsigned long GetMP        ( ) { return 0; }
	unsigned long GetMaxHP     ( ) { return 0; }
	unsigned long GetMaxMP     ( ) { return 0; }

	void          GetNpcData   ( cPlayer*, sNpcData* );

public:
	bool          SendSightIn  ( char category, char protocol, cPlayer* player );
	bool          SendSightOut ( char category, char protocol, unsigned long connectionIdx );
};
