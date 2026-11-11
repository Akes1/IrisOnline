/* ==========================================================================
* 파    일 : MakeButtonWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-27
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cMakeButtonWindow : public cUIWindow
{
public:
	cMakeButtonWindow();
	virtual ~cMakeButtonWindow();

protected:
	/// 이벤트
	bool OnCreate( cUINodeProperty* pproperty );

};
