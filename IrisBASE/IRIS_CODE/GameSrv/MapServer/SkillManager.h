/* ==========================================================================
* 파    일 : SkillManager.h
* 목    적 : 
* 작 성 자 : 박경희
* 작 성 일 : 2007-07-06
* 주의사항 :
*===========================================================================*/
#pragma once

#include "HashMap.h"
#include "PointerHashMap.h"
#include "Pool.h"

#include "../Common/BaseObject_Common.h"

#include "PlayerSkillObject.h"
#include "MonsterSkillObject.h"
#include "PlayerMonSkillObject.h"
#include "InfluenceObject.h"
#include "HaveSkillObject.h"

#include "DramaturgyManager.h"
#include "IndexGenerator.h"

#include "SQLGameStmt.h"
#include "RangeCheck.h"

class cBaseObject;
class cSkillScript;
class cCommunityScript;
class cMakeSkillScript;
class cPlayer;
class cInfluenceObject;

enum eObject;
enum eAURATYPE;

const long DB_SAVE_COOLTIME = 5000;

struct sDelSkillInfo
{
	sObject			skillUser;
	unsigned long	mSkillIdx;
};

class cSkillManager
{
public:

	static cSkillManager* mpSkillManager;

public:

	cSkillManager();
	~cSkillManager();

	bool				Init();
	void				Release();
	void				Process( unsigned long elapsedTime, unsigned long accumTime );


	/*------------------ 풀에서 가져오고 반납하기 ------------------ */
	/// 사용중인 스킬
	cPlayerSkillObject*	AllocPlayerSkillObject() { return mPlayerSkillObjectPool.Alloc(); }
	void				FreePlayerSkillObject(cPlayerSkillObject* ptr) { mPlayerSkillObjectPool.Free(ptr); }
	cMonsterSkillObject*	AllocMonsterSkillObject() { return mMonsterSkillObjectPool.Alloc(); }
	void				FreeMonsterSkillObject(cMonsterSkillObject* ptr) { mMonsterSkillObjectPool.Free(ptr); }
	cPlayerMonSkillObject*	AllocPlayerMonSkillObject() { return mPlayerMonSkillObjectPool.Alloc(); }
	void				FreePlayerMonSkillObject(cPlayerMonSkillObject* ptr) { mPlayerMonSkillObjectPool.Free(ptr); }
	/// 보유 효과
	cInfluenceObject*	AllocInfluenceObject() { return mInfluenceObjectPool.Alloc(); }
	void				FreeInfluenceObject(cInfluenceObject* ptr) { mInfluenceObjectPool.Free(ptr); }
	/// 보유 스킬
	cHaveSkillObject*	AllocHaveSkillObject() { return mPlayerHaveSkillPool.Alloc(); }
	void				FreeHaveSkillObject(cHaveSkillObject* ptr) { mPlayerHaveSkillPool.Free(ptr); }


	/*--------------- 보유 스킬 처리 ---------------*/
	/// DB 입출력
	bool				SaveSkillCoolTime( unsigned long playerIdx, SKILL_COOLTIME* skillCoolTime );
	bool				RestorePlayerSkill( SKILL_SELECT* skillSelect );
	bool				AddTutorialHaveSkill( cPlayer* pPlayer );
	/// 보유스킬 등록전 검사 항목 리턴
	bool				NeedAddPlayerHaveSkill( unsigned long skillIdx, unsigned char skillStep, /*unsigned long *money,*/ unsigned short *sp );
	/// db입출력 실패시 스킬 단계감소
	void				RollBackPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx );
	/// 보유 스킬 추가
	unsigned char		AddPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx, unsigned char skillStep, unsigned long cooltime );
	cHaveSkillObject*	GetPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx );
	void*				GetPlayerHaveSkill( unsigned long playerIdx );
	
	/// 보유 스킬 제거
	void				ReleasePlayerHaveSkill( unsigned long playerIdx );

	/// 보유 스킬 치트로 추가
	unsigned char		AddPlayerHaveSkillByCheat( unsigned long playerIdx, unsigned long skillIdx, unsigned char skillStep, unsigned long cooltime );
	
	/// 스킬 사용시 관련 스킬들 쿨타임 업데이트
	void				UpdateSkillCoolTime( sObject attacker, unsigned long skillIdx, unsigned char skillStep, unsigned long accumTime, float speedFactor );

	void				AddSkillReset( unsigned long playerIdx, unsigned long skillIdx, unsigned char skillStep );

	void				ClearPlayerHaveSkillCoolTime( unsigned long playerIdx, unsigned long exceptSkillIdx );

	/*--------------- 스킬(+일반) 공격 처리 ---------------*/
	/// 일반 공격
	void				PlayerNormalAttackRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg );
	/// 오브젝트가 타겟인 스킬
	void				PlayerObjectSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg, bool casting );
	/// 좌표가 타겟인 스킬
	void				PlayerPositionSkillRequest( unsigned long characterIdx, MSG_REQ_POS_USED* pMsg, bool casting );
	/// 몬스터 변신 스킬
	void				PlayerMonSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg, bool casting );
	/// 몬스터 변신 좌표 스킬
	void				PlayerMonPosSkillRequest( unsigned long characterIdx, MSG_REQ_POS_USED* pMsg, bool casting );
	/// 맵이동 스킬
	void				MapChangeSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_ITEMMAPCHANGE* pMsg );
	/// 탈것 스킬
	void				VehiCleSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_VEHICLE* pMsg );
	/// 캐시 맵이동 스킬
	void				CashMapChangeSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_CASHMAPCHANGE* pMsg );
	/// 순간이동 스킬
	void				BlinkSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_BLINK* pMsg );
	/// 정지 하지 않는 스킬
	void				PlayerNonStopSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg );
	void				PlayerMonNonStopSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg );
	/// 돌진
	void				PlayerRushSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg, bool blink );
	/// 돌진 후 스킬
	void				PlayerRushAfterSkillRequest( unsigned long characterIdx, MSG_REQ_SKILL_USED* pMsg );


	/// 몬스터 공격
	bool				MonsterAttack( unsigned long monsterIdx, eMONSTERATTACK_TYPE attackType, sObject target );

	/// 스킬 사용 가능 여부 체크
	bool				IsUsingSkill( unsigned char type, unsigned long objectIdx );
	unsigned long		SkillLeftEndTime( unsigned char type, unsigned long objectIdx );

	/// 캐스팅 중인 스킬 사용 종료 / 캐스팅 방해
	void				CastSkillCancel( sObject attacker, bool isObstacle, unsigned long attributeType );
	
	/// 채널링 중인 스킬 사용 종료 / 캐스팅 방해
	void				ChannelingSkillCancel( sObject attacker, bool isObstacle, unsigned long attributeType, bool isDuelEndCancel = false );

	/// 공격 스킬제거
	bool				DeleteSkillObject( unsigned char type, unsigned long objectIdx, unsigned long skillUniqueIdx );
	void				InsertDeleteSkillObject( sObject skillUser, unsigned long skillUniqueIdx );


	/// 스킬객체 소멸시 고유키생성기로 만들어진 키 반환
	void				DelSkillUniqueIdx( unsigned long uniqueIdx ) { mSkillGen.DelIdx( uniqueIdx ); }


	/*--------------- 효과 처리 ---------------*/ 
	/// DB 입출력
	bool				SaveAllPlayerInfluence( unsigned long playerIdx, SKILL_INFLUENCE_UPDATE* pInfluenceUpdate );
	void				SavePlayerInfluence( unsigned long playerIdx, unsigned long infObjectIdx, bool del );
	void				CycleSaveCashInfluence( cPlayer* pPlayer );
	void				RestorePlayerInfluence( unsigned long playerIdx, SKILL_INFLUENCE_SELECT* pInfluenceSelect, unsigned long rowCnt );

	/// 스킬 외 효과 생성
	bool				AddInfluence( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, bool msgSend, sInfluenceCreate* pInf = NULL );
	unsigned long		AddInfSwitch( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, bool msgSend );
	bool				AddInfChildAura( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, 
		unsigned long parentIdx, bool msgSend );
	bool				AddInfluenceTotem( sObject attacker, sObject target, unsigned long infClassIdx, unsigned long skillClassIdx, 
		unsigned long parentIdx, bool msgSend, unsigned long continuanceTime, sInfluenceCreate* pInf = NULL );
	unsigned long		AddInfMode( sObject attacker, sObject target, unsigned long infClassIdx, bool enemyApply, unsigned short range, bool msgSend );
	int					ItemCheckAddInf( cBaseObject* pAttacker, cBaseObject* pTarget, unsigned long infClassIdx );
	bool				ApplyInfluence( cBaseObject* pAttacker, cBaseObject* pTarget, unsigned long infObjectIdx, unsigned long infClassIdx, 
		unsigned long type, long value, bool noProcessApply = false );

	cInfluenceObject*	GetInfluence( unsigned long uniqueIdx )	{ return (cInfluenceObject*)mInfluenceMap.GetAt( uniqueIdx ); }

	/// 개별 효과 제거
	void				DeleteInfluence( sObject target, unsigned long uniqueIdx );
	void				DeleteInfluenceClassIdx( sObject target, unsigned long classIdx );
	void				VehicleDeleteInfluence( sObject target, unsigned long uniqueIdx, bool sendSync = true );
	/// 리스트에서 제거(삭제 예약)
	void				DeleteInfluenceList( unsigned long uniqueIdx );

	/// 오라 효과 종료
	bool				ClientAuraInfluenceOff( unsigned long playerIdx, unsigned long uniqueIdx, unsigned long skillClassIdx );
	void				AuraInfluenceOff( unsigned long playerIdx, unsigned long skillClassIdx );
	void				SwitchParentAllOff(  unsigned long playerIdx );		/// 자신이 사용한 오라효과 모두제거

	void				InfluenceOff( unsigned long playerIdx, unsigned long influenceClassIdx );

	/// 클라이언트에서 효과 삭제메세지가 날라옴
	void				ClientDeleteInfluenceObject( unsigned long playerIdx, unsigned long uniqueIdx );

	/// 효과 디스펠
	void				DispelInfluence( sObject target, unsigned long influenceTypeDetail, unsigned long groupOrder );
	void				DispelInfluenceAll( sObject target, unsigned long groupOrder );

	/// 면역에 해당하는 효과 제거
	void				ClearImmuneKind( sObject target, unsigned long typeDetail );
	void				ClearImmuneApplyType( sObject target, unsigned char InfluenceType, unsigned short applyType );

	/// 효과객체 소멸시 고유키생성기로 만들어진 키 반환
	void				DelInfluenceUniqueIdx( unsigned long uniqueIdx ) { mInfluenceGen.DelIdx( uniqueIdx ); }

	void				SendHPMPInf( cBaseObject* pTarget, sObject attacker,
		char* targetMsg, unsigned short targetLen, char* aroundMsg, unsigned short aroundLen );

	/*--------------- 발동 효과 처리 ---------------*/
	void				CheckActInfluence( eACTSTATUS status, cBaseObject* pObject, cBaseObject* pTarget );
	void				CheckSkillActInfluence( unsigned long actIdx, cBaseObject* pObject, cBaseObject* pTarget );

	/*--------------- hp/mp 변화량 적용 발송 ---------------*/
	void				InsertChangeSet( cBaseObject* pTarget, sObject attacker, unsigned char damageKind );

	/*--------------- 플레이어 접속 처리 ---------------*/
	/// 데이터 로드시점 정보 발송
	void				SendMsgSkillHaveToPlayer( unsigned long connectionIdx, unsigned long characterIdx );
	void				SendMsgInfluenceToPlayer( unsigned long connectionIdx, unsigned long characterIdx );
	/// 그리드 in/out 싱크 메세지 발송
	void				SendAddSynMsgInfluenceToPlayer( unsigned long connectionIdx, unsigned long characterIdx );
	/*-----------------------------------------*/

private:

	void				InsertPlayerSkillMap( unsigned long characterIdx, cBaseSkillObject* pSkillObject );
	void				InsertMonsterSkillMap( unsigned long monsterIdx, cMonsterSkillObject* pSkillObject );

	/// 스킬 보유리스트 처리
	bool				DelPlayerHaveSkill( unsigned long playerIdx, unsigned long skillIdx );
	void				ReleasePlayerHaveSkill();

	/// 효과 리스트 처리
	void				ReleaseInfluence();

private:

	/// 플레이어 스킬 객체를 담을 pool
	typedef tPool<cPlayerSkillObject>	cPlayerSkillObjectPool;
	cPlayerSkillObjectPool	mPlayerSkillObjectPool;
	typedef tPool<cMonsterSkillObject>	cMonsterSkillObjectPool;
	cMonsterSkillObjectPool	mMonsterSkillObjectPool;
	typedef tPool<cPlayerMonSkillObject>	cPlayerMonSkillObjectPool;
	cPlayerMonSkillObjectPool	mPlayerMonSkillObjectPool;
	/// 효과 객체를 담을 pool
	typedef tPool<cInfluenceObject> cInfluenceObjectPool;
	cInfluenceObjectPool	mInfluenceObjectPool;
	/// 보유 스킬을 담을 pool
	typedef tPool<cHaveSkillObject> cHaveSkillObjectPool;
	cHaveSkillObjectPool	mPlayerHaveSkillPool;

	typedef tArray<void*>	cArray;
	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;
	typedef tPointerHashMap<unsigned long, cPlayer*> cPlayerMap;
	typedef tHashSet<unsigned long> cHashSet;
	typedef tHashMap<unsigned long, unsigned long> cHashMap;
	typedef tPointerArray<void*>	cPArray;
	typedef tArray<unsigned long> cLongAry;

	/// 사용 중인 스킬객체 관리 해쉬맵
	//typedef tPointerHashMap<unsigned long, tArray<cSkillObject*>*> cSkillObjectHashMap;
	cPointerHashMap		mPlayerSkillMap;
	cPointerHashMap		mMonsterSkillMap;

	/// 플레이어가 보유한 스킬 리스트
	/// ※ playeridx와 skillidx로 여러가지 검색이 있어서 map이 map을 참조하는 형태로 만들어짐
	//typedef tPointerHashMap<unsigned long, cHaveSkillObject*> cPlayerHaveSkillMap;
	//typedef tPointerHashMap<unsigned long, cPlayerHaveSkillMap*> cPlayerSkillPointerHashMap;
	cPointerHashMap		mPlayerHaveSkillMap;

	/// 효과 전체 맵 - 오브젝트가 자신에 맞는 효과 포인터를 가지고 있다.
	//typedef tPointerHashMap<unsigned long, cInfluenceObject*> cInfluenceMap;
	cPointerHashMap		mInfluenceMap;

	/// 제거항목저장배열
	tArray<sDelSkillInfo> mDeleteSkillAry;
	tArray<unsigned long> mDeleteInfluenceAry;

	/// 스킬 스크립트 파일 로더
	cSkillScript*		mpSkillScript;

	/// 커뮤니티 스크립트 파일 로드
	cCommunityScript*	mpCommuScript;

	/// 스킬 연출 파일 로더
	cDramaturgyManager	mDramaturgyManager;

	/// 전문 기술
	cMakeSkillScript*	mpMakeSkillScript;

	/// 인덱스 생성기
	cIndexGenerator		mSkillGen;
	cIndexGenerator		mInfluenceGen;

	/// 거리 체크
	cRangeCheck			mRangeCheck;

	/// hp/mp 변동 목록
	/// <unsigned long playerIdx>
	cHashSet			mPlayerChangeSet;
	/// <unsigned long monsterIdx>
	cHashSet			mMonsterChangeSet;

	/// <unsigned long connectionIdx>
	cHashSet			mSendMsgSet;

	cLongAry			mTempActTargetAry;
};

#define SKILLMANAGER	cSkillManager::mpSkillManager

