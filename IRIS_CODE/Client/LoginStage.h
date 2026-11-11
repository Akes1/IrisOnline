/* ==========================================================================
* 파    일 : LoginStage.h
* 목    적 : 로그인 스테이지
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-30
* 주의사항 :
*===========================================================================*/
#pragma once
#include "BaseStage.h"

class cLoginUIManager;
class cLoginStage : public cBaseStage
{
public:
	cLoginStage();
	virtual ~cLoginStage();

	void CreateLoginMap();

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void NetworkMsgParser( unsigned long connectionIdx, char* pMsg, unsigned long length );

	virtual void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );

private:
	/// UI 집합소
	cLoginUIManager* mpLoginUIManager;

	bool mLoadComplete;
	bool mCheckLoginComplete;
};
