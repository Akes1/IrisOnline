/* ==========================================================================
* 파    일 : DuelManager.h
* 목    적 : 결투 시스템 - 개인
* 작 성 자 : 박경희
* 작 성 일 : 2008-04-23
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Duel_Common.h"
#include "IndexGenerator.h"
#include "RangeCheck.h"
#include "DuelObject.h"

class cDuelObject;

class cDuelManager
{
public:

	static cDuelManager* mpDuelManager;

public:

	cDuelManager();
	~cDuelManager();

	bool				Init();
	void				Release();
	void				Process( unsigned long elapsedTime, unsigned long accumTime );

	/// pool 처리
	cDuelObject*		AllocDuelObject() { return mDuelObjectPool.Alloc(); }
	void				FreeDuelObject(cDuelObject* ptr) { mDuelObjectPool.Free(ptr); }

	cDuelObject*		GetDuel( unsigned long duelIdx )	{ return (cDuelObject*)mDuelMap.GetAt( duelIdx ); }

	/// 결투 요청 가능 여부 체크
	int					ReqDuel( unsigned long attackerIdx, unsigned long targetIdx );
	bool				IsDuel( cPlayer* pAttacker, cPlayer* pTarget );

	/// 결투 동의
	void				DuelAllReady( bool mIsAccept, cPlayer* pTarget );

	/// 결투 생성
	bool				CreateDuel( cPlayer* pAttacker, cPlayer* pTarget );

	/// 결투 포기
	void				DuelGiveUp( unsigned long duelIdx, unsigned long playerIdx );

	/// 마지막 데미지 처리
	bool				DuelLastAttack( sObject attacker, sObject target, unsigned long damage, bool isCri, unsigned long* appDamage );

	/// 삭제 배열에 추가
	void				DelDuel( unsigned long duelIdx );

	/// 결투자 주변에 해당하는 플레이어들에게 QuickSendExcept
	void				TwoPlayerQuickSend( cPlayer* pPlayer1, cPlayer* pPlayer2, char* msg, int length );

	/// 결투자 결투 불가능(패배/로그아웃) 처리
	void				DuelObjectEnd( unsigned long attackerIdx, unsigned long targetIdx, unsigned long duelIdx, bool isAttackerEnd );

private:

	typedef tPointerHashMap<unsigned long, void*> cPHashMap;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tArray<unsigned long> cAry;

	/// 결투 객체를 담을 pool
	typedef tPool<cDuelObject>	cDuelObjectPool;
	cDuelObjectPool		mDuelObjectPool;

	/// 인덱스 생성기
	cIndexGenerator		mDuelGen;

	/// 결투 객체를 담을 맵
	//typedef tPointerHashMap<unsigned long, cDuelObject*> cDuelMap;
	cPHashMap			mDuelMap;

	/// 결투 객체 삭제 배열
	cAry				mDelDuel;

	/// sync메세지를 보낼 대상들
	cHashSet			mSendSet;

	cRangeCheck			mRangeCheck;

};

#define DUELMANAGER	cDuelManager::mpDuelManager

