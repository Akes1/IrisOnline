/* ==========================================================================
* 파    일 : StatusCalc_Client.h
* 목    적 : 스텟 계산기 클라이언트
* 작 성 자 : 박경희
* 작 성 일 : 2007-10-19
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Status_Common.h"
#include "Player_Common.h"

//class cStatusScript;
//class cPlayer;

//struct sPlayerSkillStepInfo;

enum ePLAYER_STATUS_EXT;
enum eSTATUS_PLUS;
//enum eSTATUSCOLORPOS;

class cStatusCalc_Client 
{
public:

	static cStatusCalc_Client* mpStatusCalc;

public:

	cStatusCalc_Client(void);
	~cStatusCalc_Client(void);

	/// 플레이어 전체 계산 - 처음 접속할때만 호출한다.
	void				CalcPlayerInit();

	/// 플레이어 스텟1(+스텟2) 계산
	void				CalcPlayerBaseGlobal();
	/// 플레이어 스텟1Plus+스텟2Plus(+스텟2) 계산
	void				CalcPlayerExtensionGlobal();

	/// hero 스텟2 확장값을 읽어온다.
	float				GetStatus2Plus( ePLAYER_STATUS_EXT status2 );
	float				GetStatus2Per( ePLAYER_STATUS_EXT status2 );

private:

	///	스텟창 색상 설정
	void				SetBaseStatusInfoType( ePLAYER_STATUS pos, float value, float totalValue );
	void				SetExtStatusInfoType( ePLAYER_STATUS_EXT pos, float value, float totalValue );

	/// 스킬 효과에서 스텟에 더해질 수치를 읽어와 더함
	void				CalcPlayerStatusSkillInfluenceAdd();
	/// 스킬 패시브에서 스텟에 더해질 수치를 읽어와 더함
	void				CalcPlayerStatusSkillPassiveAdd();
	///// 아이템에서 스텟에 더해질 수치를 읽어와 더함
	void				CalcPlayerStatusItemAdd();

	void				CalcPlayerStatusItemSkillAddDetail( eSTATUS_PLUS statusType, float value, bool passive );

	/// 플레이어 기본스텟 부터 이후 스텟이 변할때 사용 - 기본확장스텟(+)계산 안함 
	void				CalcPlayerBaseOnly();
	/// 플레이어 기본확장스텟(+) 부터 이후 스텟이 변할때 사용 - 기본 스텟 계산 안함 
	void				CalcPlayerExtensionOnly();

	/// 부모 직업군 코드를 읽어옴
	unsigned long		GetParantJob( unsigned long job, unsigned int jobPos );

	/// 플레이어 기본 스텟 계산
	void				CalcPlayerStatus();
	/// 확장 스텟 계산
	void				CalcPlayerExtStatus( ePLAYER_STATUS_EXT statusExt );

	/// 플레이어 스텟 계산 외부 인터페이스의 공용항목
	void				CalcPlayerGlobal();

private:
	float				mBaseStatus[ePLAYER_STATUS_LEVEL];			/// 070207 PKH 기본 스텟(힘,민첩,체력,지능,지혜)
	float				mBaseStatusPlus[ePLAYER_STATUS_LEVEL];		/// 070207 PKH 기본 스텟 (힘,민첩,체력,지능,지혜)(+)
	float				mBaseStatusPer[ePLAYER_STATUS_LEVEL];		/// 기본 스탯 (힘,민첩,체력,지능,지혜)(%)

	float				mExtStatus[ePLAYER_STATUS_EXT_MAX];			/// 070207 PKH 확장 스텟
	float				mExtStatusPlus[ePLAYER_STATUS_EXT_MAX];		/// 070207 PKH 확장 스텟에 더해질 값
	float				mExtStatusPer[ePLAYER_STATUS_EXT_MAX];		/// 070207 PKH 확장 스텟에 확률로 곱해질 값

	float				mExtStatusLast[ePLAYER_STATUS_EXT_MAX];		/// 070522 PKH 확장 스텟 최종값

	/// 070510 PKH 스텟외 스텟
	float				mExtAddStatus[ePLAYER_STATUS_EXT_ADD_MAX];

	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;
	typedef tHashSet<unsigned long> cHashSet;



};


#define STATUSCALC cStatusCalc_Client::mpStatusCalc
