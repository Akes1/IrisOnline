/* ==========================================================================
* 파    일 : SystemWindow.h
* 목    적 : 게임 옵션 조절용 시스템 윈도우 
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "TabWindow.h"

class cOptionSheet1;
class cOptionSheet2;
class cOptionSheet3;

class cOptionWindow : public cTabWindow
{
public:
	cOptionWindow();
	virtual ~cOptionWindow();

	virtual void Open() {}

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* pcaller, unsigned int id );
	void OnShow();

protected:
	cOptionSheet1*	mpOptSheet1;
	cOptionSheet2*	mpOptSheet2;
	cOptionSheet3*	mpOptSheet3;
};
