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
	unsigned long	mCastingTime;

	unsigned long	mRestTime;

	/// 스킬 스크립트에서 가져오는 총대기시간
	unsigned long	mTotalTime;

	/// 
	unsigned long	mStartTime;
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

	cSkillObject*		AllocSkillObject();
	void				FreeSkillObject( cSkillObject* ptr );

	cInfluenceObject*	AllocInfluenceObject();
	void				FreeInfluenceObject(cInfluenceObject* ptr);

	cApplyObject*		AllocApplyObject();
	void				FreeApplyObject(cApplyObject* ptr);

	unsigned long		CreateHeroSkillObject( unsigned long skillIdx, float speedFactor = 1.0f );

	void				DeleteSkillObject( cSkillObject* p );
	void				DeleteAll();

	cInfluenceObject*	CreateInfluenceObject( unsigned long uIdx, unsigned long infIdx );

	void				DeleteInfluenceObject( cInfluenceObject* p );
	void				DeleteInfluenceObject( unsigned long uniqueIdx );
	void				UpdateInfluenceObject();
	void				DamageInfluenceObject();

	cInfluenceObject*	GetInfluenceObject( unsigned long uniqueIdx );

	cApplyObject*		CreateApplyObject( unsigned char weaponType, eAPPLYDRAMA_TYPE type );
	cApplyObject*		CreateApplyObject( unsigned long influenceIdx );
	void				DeleteApplyObject( cApplyObject* p );

	bool IsUsedKeepSkill( unsigned long skillIdx );
	bool IsUsedMonsterSkill( unsigned long skillIdx );

	void StartCoolTime( unsigned long skillIdx, unsigned long restTime = ULONG_MAX, float speedFactor = 1.0f );


	/// 071024 PKH 스테이터스 계산기에서 사용
	tPointerHashMap<unsigned long, void*>* GetInfluenceMap()	{ return &mInfluenceMap; }
	tPointerHashMap<unsigned long, void*>* GetKeepSkillMap()	{ return &mKeepSkillMap; }

	unsigned long GetDummySkillCount() { return mDummySkillMap.GetSize(); }
	unsigned long GetSkillCount() { return mSkillObjectMap.GetSize(); }
	unsigned long GetInfluenceCount() { return mInfluenceMap.GetSize(); }
	unsigned long GetApplyCount() { return mApplyObjectMap.GetSize(); }


protected:
	void DeleteProcess();

protected:
	/// memory
	tPool<cSkillObject>			mSkillObjectPool;
	tPool<cInfluenceObject>		mInfluenceObjectPool;
	tPool<cApplyObject>			mApplyObjectPool;

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

inline cInfluenceObject* cSkillManager::GetInfluenceObject( unsigned long uniqueIdx )
{
	return (cInfluenceObject*) mInfluenceMap.GetAt( uniqueIdx );
}
