/* ==========================================================================
* 파    일 : ScrollBar.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-10-26
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "UINodeData.h"

class cButton;
class cBarButton;
class cPlaneObject;
class cScrollBar : public cUIWindow
{
public:
	cScrollBar( eUINodeType type = eUINODE_SCROLLBAR );
	virtual ~cScrollBar();

private:
	/// 업 버튼 스킨 이름
	cString mUpButtonSkin;
	/// 다운 버튼 스킨 이름
	cString mDownButtonSkin;
	/// 바 버튼 스킨 이름 
	cString mBarButtonSkin;

	/// 위 버튼
	cButton* mpUpButton;
	/// 아래 버튼 
	cButton* mpDownButton;
	/// 바 버튼 
	cBarButton* mpBarButton;

	/// 스크롤바의 배경 이미지
//	cPlaneObject*	mpScrollImage;
};

////////////////////////////////////////////////////////////////////////////
class cScrollBarSkin : public cUINodeSkin
{
public:
	cScrollBarSkin( eUINodeType type = eUINODE_SCROLLBAR );
	virtual ~cScrollBarSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 업 버튼 스킨 이름
	cString mUpButtonSkin;
	/// 바 버튼 스킨 이름
	cString mBarButtonSkin;
	/// 다운 버튼 스킨 이름
	cString mDownButtonSkin;
};

