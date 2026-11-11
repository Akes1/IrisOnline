/* ====================================================================
* 파    일 :	
* 목    적 :	
* 작 성 자 :	정성대
* 작 성 일 :	2006.08.29
*
* 추가내용 :
*		추가날짜	작업자	추가내용
*		2006.08.29	정성대	생성
* 
* 주의사항 :	
* =================================================================== */
#pragma once
#include "CommonNetworkMsgStruct.h"
#include "Protocol.h"

struct sLoadFlag
{
	union
	{
		struct
		{
			bool heroBaseInfoOn		: 1;
			bool monsterModelOn		: 1;
			bool npcModelOn			: 1;

			bool skillInfoOn		: 1;
			bool InfluenceInfoOn	: 1;
			bool guardInfoOn		: 1;

			bool itemInfoOn			: 1;
			bool itemCoolTimeInfoOn	: 1;

			bool questInfoOn		: 1;
			bool optionInfoOn		: 1;

			bool shortcutInfoOn		: 1;
			bool titleInfoOn		: 1;
			bool makeSkillInfoOn	: 1;

		} flag;
		long loadInfo;
	};
};


enum eSTAGE
{
	eStage_Intro		= 0,
	eStage_Login,
	eStage_Server,
	eStage_Select,
	eStage_Make,
	eStage_Load,
	eStage_MapChange,
	eStage_Game,

	eStage_Max,
};

class cMouseAgent;
class cKeyboardAgent;

class cStageManager;

class cBaseStage
{
public:
	cBaseStage();
	virtual ~cBaseStage();

	virtual void Init( cStageManager* pStageManager ) = 0;	// 프로그램 실행시 한번 실행..
	virtual void InitStage() = 0;							// 스테이지 진입시 한번 실행..
	virtual void Exit() = 0;								// 프로그램 종료시 한번 실행..
	virtual void Open() = 0;								// 스테이지 진입시 매번 실행..
	virtual void Close() = 0;								// 스테이지 종료시 매번 실행..
	virtual void Process( unsigned long deltaTime, unsigned long accumTime ) = 0;
	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length ) = 0;

	virtual void MsgBoxEventParser( unsigned int /*event*/, unsigned int /*result*/, void* /*exData*/ ) {}

	virtual void ProcessKeyPressed( cKeyboardAgent* ) {}
	virtual void ProcessKeyUpDown( unsigned long, cKeyboardAgent* ) {}

	/// 마우스 클릭에 대한 처리
	virtual void ProcessLButtonClick( cMouseAgent* /*pMouse*/ ) {}
	virtual void ProcessLButtonDClick( cMouseAgent* /*pMouse*/ ) {}

	virtual void ProcessRButtonDown( cMouseAgent* /*pMouse*/ ) {}
	virtual void ProcessRButtonUp( cMouseAgent* /*pMouse*/ ) {}

	virtual void ProcessMouseover( unsigned long /*time*/, cMouseAgent* /*pMouse*/ ) {}

	/// 치트를 위해 스테이지 매니저를 액세스한다.	070102 이웅주
	/// 게임스테이지매니저가 베이스 클래스가 없으므로 어쩔 수 없이 void*로 반환한다. 위험하지만 캐스팅해서 사용해야 한다.
	//void* GetStageManager() const;

protected:
	cStageManager*	mpStageManager;
	bool			mInit;
};
