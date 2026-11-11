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
#include "Item_Common.h"

class cCardSceneNode;
class cDisplayObject;
class cMakeUIManager;

class cMakeStage : public cBaseStage
{
public:
	cMakeStage(void);
	virtual ~cMakeStage(void);

	virtual void Init( cStageManager* pStageManager );
	virtual void InitStage();
	virtual void Exit(){};
 	virtual void Open(){};
 	virtual void Close(){};
	virtual void Process( unsigned long deltaTime, unsigned long accumTime ){};

	virtual void NetworkMsgParser( unsigned long connectionindex, char* pMsg, unsigned long length ){};

protected:

	bool LoadDisplayItemInfo();

	struct sViewItem
	{
		unsigned long	mJobIdx;
		unsigned long	mItemIdx;
		unsigned short	mInvenNum;

		sViewItem() : mJobIdx(0), mItemIdx(0), mInvenNum(0) {}
	};

protected:
	cMakeUIManager*	mpMakeUIManager;

	cCardSceneNode*	mCardDramaObj;

	cDisplayObject*	mpMakeCharacter;

	tArray<void*>	mViewItemArray;

	/// collision
	tArray<void*> mCollidableArray;

	/// 선택창의 화면연출상태
	unsigned char mViewState;

	unsigned int				mCurrentSelectIndex;
	bool						mUpdateDisplayObj;
};
