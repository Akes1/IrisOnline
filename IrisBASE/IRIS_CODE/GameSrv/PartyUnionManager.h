/* ==========================================================================
* 파    일 : PartyUnionManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2009-06-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Player.h"
#include "PartyUnion.h"
#include "IndexGenerator.h"

const unsigned long PARTYUNION_UPDATETIME = 5000;

class cPartyUnionManager
{
	static cPartyUnionManager* mpSingleton;

public:
	cPartyUnionManager();
	~cPartyUnionManager();

	bool Init();
	void Release();
	void Process( unsigned long elapsedTime, unsigned long accumTime );

	cPartyUnion* AllocPartyUnion();
	void FreePartyUnion( cPartyUnion* party );

	cPartyUnion* GetUnion( unsigned long index );

	// 연합 생성
	unsigned long CreateUnion();
	
	bool AddUser( unsigned long index, unsigned long userIdx );			// 연합 가입
	bool DeleteUser( cPlayer* player );									// 연합 탈퇴
	int  MoveUser( unsigned long leaderIdx, unsigned char i0, unsigned char i1 );					// 이동할 배열인덱스

	int ChangeLeader( unsigned long leaderIdx, unsigned long userIdx );	// 연합장 위임

	//
	void MapIn( cPlayer* player );
	
protected:
	bool DeleteUnion( unsigned long index );

	//
	void SendUnionList( unsigned long index, unsigned long userIdx );		// 유저에게 연합정보 send
	void SendUnionAddOne( unsigned long index, unsigned long userIdx, unsigned int arrIdx );	// 기존유저에게 새 유저정보 send
	void SendUnionOutSyn( unsigned long index, unsigned long userIdx );							// 유저 탈퇴 알림
	void SendUnionChangeLeader( unsigned long index, unsigned long userIdx );
	void SendUnionMoveUser( unsigned long index, unsigned char i0, unsigned char i1 );
	void SendUnionUpdate( cPartyUnion* p );			// 연합 전체 정보 갱신
	void SendUnionGroupUpdate( cPartyUnion* p );	// 그룹 정보만 갱신

public:
	static cPartyUnionManager* GetSingleton();

private:
	cIndexGenerator	mIndexGen;

	typedef tPool<cPartyUnion> cPartyUnionPool;
	cPartyUnionPool mPartyPool;

	typedef tPointerHashMap<unsigned long, void*> cPartyUnionMap;
	cPartyUnionMap	mPartyUnionMap;
};

inline cPartyUnion* cPartyUnionManager::AllocPartyUnion()
{
	return mPartyPool.Alloc();
}

inline void cPartyUnionManager::FreePartyUnion( cPartyUnion* party )
{
	mPartyPool.Free( party );
}

inline cPartyUnionManager* cPartyUnionManager::GetSingleton()
{
	return mpSingleton;
}

#define PARTYUNIONMAN cPartyUnionManager::GetSingleton()