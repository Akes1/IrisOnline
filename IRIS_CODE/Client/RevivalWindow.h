/* ==========================================================================
* 파    일 : RevivalWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-10-29
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

const unsigned long REVIVAL_TIME = 5 * 60 * 1000;

class cLabel;
class cButton;
class cRevivalWindow : public cUIWindow
{
public:
	cRevivalWindow();
	virtual ~cRevivalWindow();

	virtual void Open();
	virtual void Close();

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnProcess( unsigned long deltaTime, unsigned long accumTime );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void UpdateSkin();
	void OnShow();
	void OnHide();
	void OnNetLock( int lockTry );
	void OnNetUnLock( int lockTry );

private:
	/// 남은 시간 라벨
	cLabel*		mpTime;

	/// 버튼
	cButton*	mpTownButton;
	cButton*	mpStandButton;
	
	unsigned long	mCheckTime;
	unsigned long	mRestTime;
};
