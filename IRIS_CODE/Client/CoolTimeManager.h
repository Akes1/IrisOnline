/* ==========================================================================
* 파    일 : CoolTimeManager.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-09-11
* 주의사항 :
*===========================================================================*/
#pragma once
#include "ItemManager.h"

struct sCoolTime
{
	/// 그룹번호
	int groupNum;

	/// 남은시간
	long restTime;

	/// 유지시간
	long totalTime;

	/// 시작시간
	long startTime;

	sCoolTime()
	{
		groupNum = 0;
		restTime = 0;
		totalTime = 0;
		startTime = 0;
	}
};

struct sGroupData
{
	/// 아이템 인덱스 리스트
	tArray<unsigned long> mIndexArr;
};

///
class cCoolTimeManager
{
public:
	static cCoolTimeManager* mSingleton;

public: 
	cCoolTimeManager();
	~cCoolTimeManager();

	///
	void Clear();
	void Process( unsigned long accumTime );

	/// 서버로부터 남은 쿨타임 저장
	void NetSetCool( sInventoryCooltime& cool );

	/// 아이템 사용
	void StartCoolTime( unsigned long itemIndex );
	void CopyCoolTime( unsigned long itemIndex );

    /// 데이터 로드
	void LoadData( unsigned long itemIndex, unsigned int groupnum, long cooltime );

	/// 쿨타임있는 아이템 정보맵으로부터 정보 획득
	sCoolTime* GetCoolData( unsigned long itemIndex );

	/// 해당 아이템이 쿨타임중인지 검사.
	sCoolTime* GetProcessData( unsigned long itemIndex );
	
public:
	/// 단일체를 리턴
	static cCoolTimeManager* GetSingleton();

private:
	bool AddProcessCool( unsigned long itemIndex, long restTime, long totalTime );

private:
	/// 쿨타임 관련 아이템 정보 맵 ( 아이템인덱스, 쿨타임정보 )
	typedef tPointerHashMap<unsigned long, void*> cCoolTimeMap;
	cCoolTimeMap	mCoolTimeMap;

	/// 그룹 찾기 위한 정보 (sGroupData)
	typedef tPointerHashMap<unsigned long, void*> cGroupMap;
	cGroupMap mGroupMap;

	/// 쿨타임중인 아이템 
	typedef tPointerHashMap<unsigned long, void*> cProcessMap;
	cProcessMap		mProcessMap;
};


inline cCoolTimeManager* cCoolTimeManager::GetSingleton()
{
	return mSingleton;
}

#define COOLMAN cCoolTimeManager::GetSingleton()
