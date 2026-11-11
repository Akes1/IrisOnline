/* ==========================================================================
* 파    일 : QuestPlane.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-04-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "DepthPlane.h"

const unsigned int QUEST_ACTIVE_TIME = 2000;
const unsigned int QUEST_TEXT_LENGTH = 40;

class cQuestPlane : public cPlane
{
public:
	cQuestPlane();
	virtual ~cQuestPlane();

	void Update( unsigned long time );
	
	void Active( LPCTSTR msg, unsigned long color, NiPoint3& pos );
	void DeActive() { mActive = false; }
	bool IsActive() { return mActive; }

private:
	void Draw();

protected:
	TCHAR			mQuestMsg[QUEST_TEXT_LENGTH];
	unsigned int	mMsgLen;
	unsigned long	mQuestColor;
	unsigned int	mMsgWidth;
	float			mAlpha;

	/// 출력 위치
	unsigned int	mPosX;
	unsigned int	mPosY;

	/// 활성화 여부
	bool			mActive;
	unsigned long 	mActiveTime;
};

