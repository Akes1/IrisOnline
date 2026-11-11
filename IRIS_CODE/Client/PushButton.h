/* ==========================================================================
* 파    일 : PushButton.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-01-11
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Button.h"

class cPushButton : public cButton
{
public:
	cPushButton( eUINodeType type = eUINODE_PUSHBUTTON );
	virtual ~cPushButton();

	/// 버튼 눌림 상태 조절 
	virtual void SetPress( bool press );
	void Hide();

protected:
	/// 이벤트 처리
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnLButtonUp( const cUIPos& pos );
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );

protected:
	/// 오버 되었는지 체크 
	bool mHovered;
};