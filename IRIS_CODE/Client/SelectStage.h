/* ====================================================================
 * 파    일 :	
 * 목    적 :	
 * 작 성 자 :	정성대
 * 작 성 일 :	2006.08.29
 * 주의사항 :	
 * =================================================================== */
#pragma once
#include "BaseStage.h"

class cSelectUIManager;

class cCardSceneNode;
class cDisplayObject;

class cSelectStage : public cBaseStage
{
	friend class cMakeStage;

public:
	cSelectStage();
	virtual ~cSelectStage();

	void CreateSelectMap( bool creatmap );
	void ClearSelectMap();

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void ProcessLButtonClick( cMouseAgent* /*pMouse*/ );
	virtual void ProcessLButtonDClick( cMouseAgent* /*pMouse*/ );

	virtual void ProcessMouseover( unsigned long /*time*/, cMouseAgent* /*pMouse*/ );

	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length );
	virtual void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );

	void SendDeleteWaitCharacter();
	void SendDeleteCharacter( cDisplayObject* p );
	void SendConnect( long channelNum, bool istutorial );

	void MoveCard( bool rew );

	void MoveMakeStage();
	void CancelChannel();

	cDisplayObject* GetCurrentDisplayObject() { return mDisplayObject; }

	void SetEnableWindow( bool enable );

protected:

	/// NM_USER
	void Msg_UserCharacterListAck( MSGROOT* pMsg );		
	void Msg_UserChannelListAck( MSGROOT* pMsg );		

	/// NM_HERO
	void Msg_HeroSelectAck( MSGROOT* pMsg );
	void Msg_HeroSelectNack( MSGROOT* pMsg );

	void ClearDisplayObject();
	void DeleteDisplayObject( unsigned int standIdx );

	void PlayCard();
	void RewCard();

protected:
	/// UI 집합소
	cSelectUIManager*		mpSelectUIManager;

	/// collision
	tArray<void*>			mCollidableArray;

	/// 3D object creation info
	typedef tPointerHashMap<unsigned int, void*>	cDisplayObjectMap;
	cDisplayObjectMap		mDisplayObjectMap;

	///
	unsigned int			mCurrentSelectIndex;
	unsigned long			mSelectEffectIdx;

	/// channel state display object
	cDisplayObject*			mDisplayObject;

	cCardSceneNode*			mCardDramaObj[5];
	float					mCardInitTime[5];

	bool					mCardMoving;
	unsigned char			mCardMoveState;
	unsigned char			mCardMoveCount;

	///// 임시 코드
	//bool					mChangeLogin;
};
