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
#include "BaseStage.h"

class cGameUIManager;
class cDuelManager;

class cGameStage : public cBaseStage
{
public:
	cGameStage();
	virtual ~cGameStage();

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void ProcessKeyUpDown( unsigned long time, cKeyboardAgent* pKeyboard );

	virtual void ProcessLButtonClick( cMouseAgent* /*pMouse*/ );
	virtual void ProcessLButtonDClick( cMouseAgent* /*pMouse*/ );

	virtual void ProcessRButtonDown( cMouseAgent* /*pMouse*/ );
	virtual void ProcessRButtonUp( cMouseAgent* /*pMouse*/ );

	virtual void ProcessMouseover( unsigned long time, cMouseAgent* /*pMouse*/ );

	virtual void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );
	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length );
	
protected:	
	/// 061122 PKH 서버에 생성되어있는 몬스터의 정보를 받아 처리한다.
	void Msg_MonsterGenerateInfo( MSGROOT* pMsg );
	void Msg_MonsterGenerateTeamInfo( MSGROOT* pMsg );

	void Msg_NpcGenerateInfo( MSGROOT* pMsg );

	void Msg_MapChangeAck( MSGROOT* pMsg );

	void Msg_GatheringGenerateInfo( MSGROOT* pMsg );

protected:
	cGameUIManager* mpUI;

	cDuelManager*	mpDuelManager;
};
