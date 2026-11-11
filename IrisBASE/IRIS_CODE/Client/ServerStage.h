/* ==========================================================================
* 파    일 : ServerStage.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-22
* 주의사항 :
*===========================================================================*/
#pragma once
#include "BaseStage.h"

class cServerUIManager;
class cServerStage : public cBaseStage
{
public:
	cServerStage();
	virtual ~cServerStage();

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length );
	virtual void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );

protected:
	void ReqServerList();

private:
	/// UI 집합소
	cServerUIManager*	mpServerUIManager;

	bool mLoadComplete;
	bool mCheckServerComplete;
};