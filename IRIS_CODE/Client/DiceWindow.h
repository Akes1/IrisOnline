/* ==========================================================================
* 파    일 : DiceWindow.h
* 목    적 : 주사위 굴리기
* 작 성 자 : 
* 작 성 일 : 2009-09-21
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cDiceWindow : public cUIWindow
{
public:
	cDiceWindow();
	virtual ~cDiceWindow();

	virtual void Open() {}
	virtual void Close() {}

	void StartDice( unsigned long time );

	void HideESC( bool onsound ) {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* , unsigned int );
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	// 굴리기
	cUINode*	mpRollButton;

	// 포기
	cUINode*	mpGiveUpButton;

	// 
	cLabel*		mpTime;

	//
	unsigned long mDelayTime;
};