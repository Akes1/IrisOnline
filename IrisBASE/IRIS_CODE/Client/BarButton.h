/* ==========================================================================
* 파    일 : BarButton.h
* 목    적 : 스크롤바의 바 버튼 & 옵션게이지의 바 버튼
* 작 성 자 : 김가람
* 작 성 일 : 2007-02-26
* 주의사항 :
*===========================================================================*/
#pragma once
#include "Button.h"

class cPlaneObject;
class cBarButton : public cButton
{
public:
	cBarButton( eUINodeType type = eUINODE_BARBUTTON );
	virtual ~cBarButton();

	/// 마우스 좌표를 받아 Y 축으로만 이동
	void MoveY( int moveY, const cUIPos& mousePos );
	/// 마우스 좌표를 받아 X 축으로만 이동
	void MoveX( int moveX, const cUIPos& mousePos );
	/// 처음 드래그 시작한 위치 저장
	cUIPos& GetOldMousePos();

	void Hide();

protected:
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnMouseMove( const cUIPos& pos );
	virtual void OnLButtonDoubleClick( const cUIPos& pos );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool /*AltPress*/, bool /*shiftPress*/ );
	virtual void OnLButtonUp( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateRect();

private:
	/// 드래그 시작 위치
	cUIPos mDragStart;

	cPlaneObject*	mpBarImage;
};

inline cUIPos& cBarButton::GetOldMousePos()
{
	return mDragStart;
}