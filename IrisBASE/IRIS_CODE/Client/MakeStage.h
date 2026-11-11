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

#include "Player_Common.h"

class cCardSceneNode;
class cDisplayObject;
class cMakeUIManager;

enum eZoomFlag
{
	eZoom_None = 0,
	eZoom_In,
	eZoom_Out,
};

class cMakeStage : public cBaseStage
{
public:
	cMakeStage(void);
	virtual ~cMakeStage(void);

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit();
	virtual void Open();
	virtual void Close();
	virtual void Process( unsigned long deltaTime, unsigned long accumTime );

	virtual void ProcessMouseover( unsigned long time, cMouseAgent* pMouse );
	virtual void ProcessLButtonDClick( cMouseAgent* /*pMouse*/ );

	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length );
	virtual void MsgBoxEventParser( unsigned int event, unsigned int result, void* exData );

	void SetMakeStandIndex( unsigned int index );
	void SendMakeCharacter( LPCTSTR name, unsigned char gender, unsigned char hair, unsigned char hairColor, unsigned char face );


	void RotateMakeCharacter( int dx );

	void ToggleZoom();

protected:
	void Msg_UserCharacterCreateNack( MSGROOT* pMsg );

	bool LoadDisplayItemInfo();

	void UpdateDisplayEquipInfo( sPlayerInfo* pBaseOut, sPlayerWearInfo* pWearOut, sPlayerWeaponInfo* pWeaponOut );

	struct sViewItem
	{
		unsigned long	mJobIdx;
		unsigned long	mItemIdx;
		unsigned short	mInvenNum;

		sViewItem() : mJobIdx(0), mItemIdx(0), mInvenNum(0) {}
	};

protected:
	cMakeUIManager*	mpMakeUIManager;

	cDisplayObject*	mpMakeCharacter;

	tArray<void*>	mViewItemArray;

	/// collision
	tArray<void*> mCollidableArray;

	/// 선택창의 화면연출상태
	unsigned char mViewState;

	unsigned int				mCurrentSelectIndex;
	bool						mUpdateDisplayObj;

	NiPoint3	mCameraDefaultPos;
	NiPoint3	mDummyPos;

	float mZoomValue;

	eZoomFlag mZoomFlag;
	bool mIsZoom;
};
