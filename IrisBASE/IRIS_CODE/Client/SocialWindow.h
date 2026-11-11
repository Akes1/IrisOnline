/* ==========================================================================
* 파    일 : SocialWindow.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2008-09-03
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"

class cLabel;
class cSocialWindow : public cUIWindow
{
public:
	cSocialWindow();
	virtual ~cSocialWindow();

	virtual void Open() {}

	void Clear();
	void UpdatePoint();

	void SetFirePoint( unsigned int point );
	void SetWaterPoint( unsigned int point );
	void SetWindPoint( unsigned int point );
	void SetEarthPoint( unsigned int point );

protected:
	bool OnCreate( cUINodeProperty* pproperty );
	void OnCommand( cUINode* , unsigned int );

private:
	cLabel*		mpFirePoint;
	cLabel*		mpWaterPoint;
	cLabel*		mpWindPoint;
	cLabel*		mpEarthPoint;
};
