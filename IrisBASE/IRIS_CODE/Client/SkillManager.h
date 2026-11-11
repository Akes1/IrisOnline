/* ======================================================================================
* 파    일 :	SkillManager.h , SkillManager.cpp
* 목    적 :	스킬 관리
* 작 성 자 :	이법석
* 작 성 일 :    2007년 07월 06일
*
* 추가내용 :
*		추가날짜    작업자	  추가내용
* 
* 주의사항 : skillobject는 dummy 생성 -> server로 부터 인증후 -> managed로 변경
			 influenceobject는 서버로 부터 받은 정보로만 생성된다.
* ======================================================================================*/
#pragma once
#include "IndexGenerator.h"
#include "SkillObject.h"

class cSkillExecuter;
class cDamagePrintManager;

struct sHaveSkill;
struct sKeepSkill
{
	unsigned char	mStep;
	long	mCastingTime;
	long	mTotalTime;	/// 스킬 스크립트에서 가져오는 총대기시간

	/// 
	long	mStartTime;
	long	mRestTime;

	sKeepSkill()
	{
		mStep = 0;
		mCastingTime = 0;
		mRestTime = 0;
		mTotalTime = 0;
		mStartTime = 0;
	}
};

class cSkillManager
{
public:
	static cSkillManager* mpSkillManager;

public:
	cSkillManager();
	~cSkillManager();

	bool				Init();

	void				Open();
	void				Close();

	void				Process( unsigned long deltaTime, unsigned long accumTime );

	/// 입력 처리
	bool				ProcessLButtonClick( int mouseX, int mouseY );
	bool				ProcessLButtonDClick( int mouseX, int mouseY );

	cSkillObject*			AllocSkillObject();
	void					FreeSkillObject( cSkillObject* ptr );

	cInfluenceObject*		AllocInfluenceObject();
	void					FreeInfluenceObject(cInfluenceObject* ptr);

	cApplyObject*			AllocApplyObject();
	void					FreeApplyObject(cApplyObject* ptr);

	cChannelingSkillObject*	AllocChannelingSkillObject();
	void					FreeChannelingSkillObject( cChannelingSkillObject* ptr );

	unsigned long		CreateHeroSkillObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, cBaseObject* pTarget, float speedFactor = 1.0f );
	unsigned long		CreateHeroSkillObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 fieldPos, float speedFactor = 1.0f );
	unsigned long		CreateHeroMapChangeSkillObject( unsigned long skillIdx, cBaseObject* pUser );
	unsigned long		CreateHeroVehicleSkillObject( unsigned long skillIdx, cBaseObject* pUser );
	unsigned long		CreateHeroBlinkSkillObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser );

	void				CreateSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, cBaseObject* pTarget, float speedFactor = 1.0f );
	void				CreateSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 fieldPos, float speedFactor = 1.0f );
	void				CreateMapChangeSkillObject( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pUser );
	void				CreateVehicleSkillObject( unsigned long uIdx, unsigned long skillIdx, cBaseObject* pUser );
	void				CreateBlinkSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser, NiPoint3 blinkPos );

	cSkillObject*		CreateNoneStopObject( unsigned long skillIdx, unsigned char skillStep, cBaseObject* pUser );


	cSkillObject* 		CreateRevisionSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, unsigned char attackerType, unsigned long dramaKey, bool tranMon );
	cSkillObject*		CreateRevisionChannelingSkillObject( unsigned long uIdx, unsigned long skillIdx, unsigned char skillStep, unsigned char attackerType, unsigned long dramaKey, bool tranMon, float fieldX, float fieldY, unsigned long playerIdx );

	void				DeleteSkillObject( cSkillObject* p );

	cInfluenceObject*	CreateInfluenceObject( unsigned long uIdx, unsigned long infIdx, unsigned long skillIdx, cBaseObject* pTarget, unsigned char deleteType );
	cInfluenceObject*	CreateInfluenceObjectEx( unsigned long uIdx, unsigned long infIdx, unsigned long skillIdx, cBaseObject* pTarget, unsigned char deleteType, unsigned long restTime );


	void				DeleteInfluenceObject( cInfluenceObject* p );
	void				DeleteInfluenceObject( unsigned long uniqueIdx );

	cInfluenceObject*	GetInfluenceObject( unsigned long uniqueIdx );

	cApplyObject*		CreateApplyObject( cBaseObject* pUser, cBaseObject* pTarget, unsigned char weaponType, eAPPLYDRAMA_TYPE type );
	cApplyObject*		CreateApplyObject( cBaseObject* pTarget, unsigned long influenceIdx );
	cApplyObject*		CreateActApplyObject( cBaseObject* pTarget, unsigned long actIdx );
	void				DeleteApplyObject( cApplyObject* p );

	/// Hero 보유 스킬 정보
	void InitKeepSkill( sHaveSkill* pdata );
	void AddKeepSkill( unsigned long skillIdx, unsigned char skillStep );
	void ClearAllKeepSkill();
	void UpdateAllSkill();

	void InitSkillUI();

	bool IsUsedKeepSkill( unsigned long skillIdx, unsigned char skillStep );
	bool IsUsedMonsterSkill( unsigned long skillIdx );

	void StartCoolTime( unsigned long skillIdx, unsigned long restTime = ULONG_MAX );
	void StartCoolTimeMon( unsigned long skillIdx, unsigned long restTime = ULONG_MAX );
	void ClearCoolTimeMon();
	sKeepSkill* GetCoolTimeMon( unsigned long skillIdx );

	void ClearSkillCooltime( unsigned long exceptskillIdx );

	/// 구매 가능한 스킬인지를 체크한다.
	bool CheckBuySkill( unsigned long skillIdx, unsigned char skillStep );

	sKeepSkill*		GetKeepInfo( unsigned long skillIdx );

	void UpdateSkillStep( unsigned long idx, unsigned char step, bool stepUpdateAuto );

	///	network msg parser
	void NetworkMsgParser( char* pMsg );

	/// 071024 PKH 스테이터스 계산기에서 사용
	tPointerHashMap<unsigned long, void*>* GetInfluenceMap()	{ return &mInfluenceMap; }
	tPointerHashMap<unsigned long, void*>* GetKeepSkillMap()	{ return &mKeepSkillMap; }

	unsigned long GetDummySkillCount() { return mDummySkillMap.GetSize(); }
	unsigned long GetSkillCount() { return mSkillObjectMap.GetSize(); }
	unsigned long GetInfluenceCount() { return mInfluenceMap.GetSize(); }
	unsigned long GetApplyCount() { return mApplyObjectMap.GetSize(); }


protected:
	void DeleteProcess();

	void UpdateExtraCooltime( unsigned long skillIdx, unsigned long restTime, unsigned long groupIdx );

protected:
	/// memory
	tPool<cSkillObject>			  mSkillObjectPool;
	tPool<cInfluenceObject>		  mInfluenceObjectPool;
	tPool<cApplyObject>			  mApplyObjectPool;
	tPool<cChannelingSkillObject> mChannelingSkillObjectPool;

	//////////////////////////////////////////////////////////////////////////
	// skill process
	typedef tPointerHashMap<unsigned long, void*> cSkillObjectMap;
	typedef tArray<void*>	cDeleteArray;

	/// 서버로 부터 검증을 받기 전 처리용 객체 (HERO용)
	cSkillObjectMap					mDummySkillMap;
	cIndexGenerator<unsigned long>	mDummyIndexGen;
	cIndexGenerator<unsigned long>	mApplyIndexGen;

	/// 관리 객체 key = server create index
	cSkillObjectMap					mSkillObjectMap;
	cSkillObjectMap					mInfluenceMap;
	cSkillObjectMap					mApplyObjectMap;

	/// 삭제용 객체
	cDeleteArray					mDeleteSkillArray;
	cDeleteArray					mDeleteInfluenceArray;
	cDeleteArray					mDeleteApplyObjectArray;
	//////////////////////////////////////////////////////////////////////////

	typedef cSkillObjectMap	cSkillMap;
	cSkillMap						mKeepSkillMap;

	/// 몬스터 변신상태시 쿨타임 저장
	sKeepSkill			mMonSkillCool[eMONSTERATTACK_MAX];

	/// 스킬 실행자
	cSkillExecuter*	mpSkillExecuter;

	/// 데미지 출력자
	cDamagePrintManager* mpDamagePrintManager;

};

#define SKILLMAN	cSkillManager::mpSkillManager

inline cSkillObject* cSkillManager::AllocSkillObject()
{
	return mSkillObjectPool.Alloc();
}

inline void cSkillManager::FreeSkillObject(cSkillObject* ptr)
{
	mSkillObjectPool.Free(ptr);
}

inline cInfluenceObject*	cSkillManager::AllocInfluenceObject()
{
	return mInfluenceObjectPool.Alloc();
}

inline void	cSkillManager::FreeInfluenceObject(cInfluenceObject* ptr)
{
	mInfluenceObjectPool.Free(ptr);
}

inline cApplyObject* cSkillManager::AllocApplyObject()
{
	return mApplyObjectPool.Alloc();
}

inline void cSkillManager::FreeApplyObject(cApplyObject* ptr)
{
	mApplyObjectPool.Free(ptr);
}

inline cChannelingSkillObject* cSkillManager::AllocChannelingSkillObject()
{
	return mChannelingSkillObjectPool.Alloc();
}

inline void cSkillManager::FreeChannelingSkillObject(cChannelingSkillObject* ptr)
{
	mChannelingSkillObjectPool.Free(ptr);
}

inline void cSkillManager::DeleteSkillObject( cSkillObject* p )
{
	if( p )
	{
		if( p->IsRemoved() == false )
		{
			p->Removed();
			mDeleteSkillArray.PushBack( p );
		}
	}
}

inline void cSkillManager::DeleteApplyObject( cApplyObject* p )
{
	if( p )
	{
		if( p->IsRemoved() == false )
		{
			p->Removed();
			mDeleteApplyObjectArray.PushBack( p );
		}
	}
}


inline sKeepSkill* cSkillManager::GetKeepInfo( unsigned long skillIdx )
{
	return (sKeepSkill*)mKeepSkillMap.GetAt( skillIdx );
}

inline cInfluenceObject* cSkillManager::GetInfluenceObject( unsigned long uniqueIdx )
{
	return (cInfluenceObject*) mInfluenceMap.GetAt( uniqueIdx );
}
