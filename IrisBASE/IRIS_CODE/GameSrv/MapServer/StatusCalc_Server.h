/* ========================================================================== 
* 파    일 : StatusCalc_Server.h 
* 목    적 : 스텟 계산기 서버 
* 작 성 자 : 박경희 
* 작 성 일 : 2007-10-19 
* 주의사항 : 
*===========================================================================*/ 
#pragma once 

#include "Status_Common.h" 
#include "Player_Common.h" 

class cPlayer;

struct sPlayerSkillStepInfo;

enum ePLAYER_STATUS_EXT;
enum eSTATUS_PLUS;

class cStatusCalc_Server 
{ 
public: 

	static cStatusCalc_Server* mpStatusCalc;

public: 

	cStatusCalc_Server(void);
	~cStatusCalc_Server(void);

	/// 플레이어 전체 계산 - 처음 접속할때만 호출한다. 
	void				CalcPlayerInit( sObject object );

	/// 플레이어 스텟1(+스텟2) 계산
	void				CalcPlayerBaseGlobal( sObject object );
	/// 플레이어 스텟1Plus+스텟2Plus(+스텟2) 계산
	void				CalcPlayerExtensionGlobal( sObject object );

	/// 몬스터 스텟 계산 인터페이스 - 추가 효과
	void				CalcMonster( unsigned long monsterIdx );

protected: 

	/// 스킬 효과에서 스텟에 더해질 수치를 읽어와 더함 
	void				CalcPlayerStatusSkillInfluenceAdd( cPlayer* pPlayer );
	/// 스킬 패시브에서 스텟에 더해질 수치를 읽어와 더함 
	void				CalcPlayerStatusSkillPassiveAdd( cPlayer* pPlayer );
	/// 아이템에서 스텟에 더해질 수치를 읽어와 더함 
	void				CalcPlayerStatusItemAdd( cPlayer* pPlayer );

	void				CalcPlayerStatusItemSkillAddDetail( cPlayer* pPlayer, eSTATUS_PLUS statusType, float value );

	/// 플레이어 기본스텟 부터 이후 스텟이 변할때 사용 - 기본확장스텟(+)계산 안함 
	void				CalcPlayerBaseOnly( cPlayer* pPlayer );
	/// 플레이어 기본확장스텟(+) 부터 이후 스텟이 변할때 사용 - 기본 스텟 계산 안함 
	void				CalcPlayerExtensionOnly( cPlayer* pPlayer );

	/// 부모 직업군 코드를 읽어옴 
	unsigned long		GetParantJob( unsigned long job, unsigned int jobPos );

	/// 플레이어 기본 스텟 계산 
	void				CalcPlayerStatus( cPlayer* pPlayer );
	/// 확장 스텟 계산 
	void				CalcPlayerExtStatus( cPlayer* pPlayer, ePLAYER_STATUS_EXT statusExt );

	/// 플레이어 스텟 계산 외부 인터페이스의 공용항목 
	void				CalcPlayerGlobal( cPlayer* pPlayer );

	/// 몬스터 스텟 세부 계산 
	void				CalcStatusPlusDetailMonster( unsigned long monsterIdx, eSTATUS_PLUS statusType, float value );

protected: 
	float				mBaseStatus[ePLAYER_STATUS_LEVEL];		/// 070207 PKH 기본 스텟(힘,민첩,체력,지능,지혜) 
	float				mBaseStatusPlus[ePLAYER_STATUS_LEVEL];	/// 070207 PKH 기본 스텟  (힘,민첩,체력,지능,지혜) (+)
	float				mBaseStatusPer[ePLAYER_STATUS_LEVEL]; // 기본 스탯 (힘,민첩,체력,지능,지혜)(%)
	
	float				mExtStatus[ePLAYER_STATUS_EXT_MAX];			/// 070207 PKH 확장 스텟 
	float				mExtStatusPlus[ePLAYER_STATUS_EXT_MAX];		/// 070207 PKH 확장 스텟에 더해질 값 
	float				mExtStatusPer[ePLAYER_STATUS_EXT_MAX];		/// 070207 PKH 확장 스텟에 확률로 곱해질 값 
	
	float				mExtStatusLast[ePLAYER_STATUS_EXT_MAX];		/// 070522 PKH 확장 스텟 최종값 

	/// 070510 PKH 경험치 관련 
	float				mExtAddStatus[ePLAYER_STATUS_EXT_ADD_MAX];

	/// 상태 이상 스텟
	long				mOddity[eODDITYTYPE_MAX];

	/// 스텟 계산전 원본값 - 계산후 값비교해서 틀리면 메세지 전송
	long				mMoveSpeed;
	float				mFixedObjectSize;
	float				mSizePer;
	float				mAttackSpeed;
	unsigned long		mMaxHP;
	unsigned long		mMaxMP;

	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;
	typedef tHashSet<unsigned long> cHashSet;

};

#define STATUSCALC cStatusCalc_Server::mpStatusCalc 
