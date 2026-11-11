/* ==========================================================================
* 파    일 : StatusScript.h
* 목    적 : 데미지 계산에 필요한 리소스파일을 읽어온다.
* 작 성 자 : 박경희
* 작 성 일 : 2007-01-12
* 주의사항 :
*===========================================================================*/
#pragma once

#include "Status_Common.h"

class cParser;
class cLexer;

class cStatusScript
{
public:

	static cStatusScript* mpStatusScript;

public:
	cStatusScript(void);
	~cStatusScript(void){};

	bool				Init();
	void				Release();

	/// 스크립트에서 스텟 기본정보 로드
	bool				LoadStateBase();
	/// 해쉬맵에 위치한 스텟 기본정보 로드
	sStatusBaseScript*	GetStatusBaseInfo( unsigned long idx );

	/// 스크립트에서 스텟 증가수치 로드
	bool				LoadStateMultipl();
	/// 해쉬맵에 위치한 스텟 증가수치 로드
	sStatusMultiplScript*	GetStatusMultiplInfo( unsigned long idx );

	/// 스크립트에서 스텟 확장 기본정보 로드
	bool				LoadStatusExtBase();
	/// 해쉬맵에 위치한 스텟 확장 기본정보 로드
	float				GetStatusExtBaseInfo( unsigned long statusExt, unsigned long jobCode );

	/// 스크립트에서 스텟 확장 증가수치 로드
	bool				LoadStatusExtMultipl();
	/// 해쉬맵에 위치한 스텟 확장 증가수치 로드
	float				GetStatusExtMultiplInfo( eEXT_MULTIPL statusExtBase, ePLAYER_STATUS type );

	/// 스크립트에서 데미지 계산공식에 수치값 로드
	bool				LoadDamageCalcNumerical();
	/// 해쉬맵에 위치한 데미지 계산 공식 수치값 로드
	float				GetDamageCalcNumericalInfo( unsigned long idx );

	/// 직업 정보 로드
	bool				LoadPlayerJob();
	bool				LoadPlayerJobInfo( cParser& parser );
	bool				LoadPlayerJobTree( cParser& parser, unsigned int depth );
	void				RemovePlayerJobInfo();

	/// 직업 정보 획득..
	tArray<unsigned long>* GetParentJob( unsigned long currentJob );

	unsigned int GetJobStep( unsigned long jobIdx );

	tPointerHashMap<unsigned long, void*>* GetJobTree()	{ return &mJobTree; }

protected:
	float SearchParentStatusExtbaseInfo( unsigned long statusExt, unsigned long jobcode );

private:

	typedef tPointerHashMap<unsigned long, void*> cPointerHashMap;	
	typedef tHashMap<unsigned long, float> cHashMap;
	typedef tArray<unsigned long> cJobIndexArray;

	/// 스텟 기본정보
	//	typedef tPointerHashMap<unsigned long, sStatusBaseScript*> cStatusBase;
	cPointerHashMap*	mpStatusBaseMap;

	/// 스텟 직업별 증가 수치
	//	typedef tPointerHashMap<unsigned long, sStatusMultiplScript*> cStatusMultipl;
	cPointerHashMap*	mpStatusMultiplMap;

	/// 스텟 확장 기본정보 - statusext로 검색하면 직업별 기본값이 들어있는 hashmap이 검색된다.
	//	typedef tHashMap<unsigned long, float> cJobDefaultMap;
	//	typedef tPointerHashMap<unsigned long, cJobDefaultMap*> cStatusExtJobDefault;
	cPointerHashMap*	mpStatusExtBaseMap;

	/// 스텟 확장 직업별 증가 수치
	//	typedef tPointerHashMap<unsigned long, float[ePLAYER_STATUS_LEVEL]*> cStatusExtMultipl;
	cPointerHashMap*	mpStatusExtMultiplMap;

	/// 데미지 계산기 조절 값
	cHashMap			mDamageCalcNumericalMap;

	/// 직업을 key로 하는 전직 직업들의 리스트를 준다.
	cPointerHashMap		mJobTree;	

};

#define STATUSSCRIPT cStatusScript::mpStatusScript