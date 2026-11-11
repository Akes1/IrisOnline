/* ==========================================================================
* 파    일 : IntroStage.h
* 목    적 : 인트로 스테이지 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-30
* 주의사항 :
*===========================================================================*/
#pragma once
#include "BaseStage.h"

enum eViewState
{
	eVIEW_NONE = 0,
	eVIEW_GRADE,
	eVIEW_INTRO,

	eVIEW_END,
};

class cIntroUIManager;
class cIntroStage : public cBaseStage
{
public:
	cIntroStage();
	virtual ~cIntroStage();

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void NetworkMsgParser( unsigned long connectionIdx, char* pMsg, unsigned long length );

	bool IsProcessEnd();

	void ClearBackImage();

protected:
	/// NM_USER
    /*void Msg_UserConnectDisSuccessAck( MSGROOT* pMsg );
	void Msg_UserConnectDisSuccessNack( unsigned long connectionIdx, MSGROOT* pMsg );
	void Msg_UserLoginAck( unsigned long connectionIdx, MSGROOT* pMsg );
	void Msg_UserLoginNack( MSGROOT* pMsg );
	void Msg_UserConnectAgentSuccessAck( MSGROOT* pMsg );
	void Msg_UserConnectAgentSuccessNack( unsigned long connectionIdx, MSGROOT* pMsg );*/

private:
	/// UI 집합소
	cIntroUIManager*	mpIntroUIManager;

	eViewState	mViewState;
	float mAlpha;
};
