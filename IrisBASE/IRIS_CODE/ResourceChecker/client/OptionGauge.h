/* ==========================================================================
* 파    일 : OptionGauge.h
* 목    적 : 옵션게이지
* 작 성 자 : 김가람
* 작 성 일 : 2007-03-22
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "UINodeData.h"

class cBarButton;
class cOptionGauge : public cUIWindow
{
public:
	cOptionGauge( eUINodeType type = eUINODE_OPTIONGAUGE );
	virtual ~cOptionGauge();
	
private:
	/// 바 버튼 스킨 이름 
	cString mBarButtonSkin;

	/// 바 버튼 
	cBarButton* mpBarButton;

	/// 현재 옵션게이지의 퍼센트 
	float mCurPercent;
};


/////////////////////////////////////////////////////////////////////////
class cOptionGaugeSkin : public cUINodeSkin
{
public:
	cOptionGaugeSkin( eUINodeType type = eUINODE_OPTIONGAUGE );
	virtual ~cOptionGaugeSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 바 버튼 스킨 이름
	cString mBarButtonSkin;
};