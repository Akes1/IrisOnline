/* ==========================================================================
* 파    일 : TabButton.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-17
* 주의사항 :
*===========================================================================*/
#pragma once
#include "PushButton.h"

class cPlaneObject;
class cTabButton : public cPushButton
{
public:
	cTabButton( eUINodeType type = eUINODE_TABBUTTON );
	virtual ~cTabButton();

	/// 눌림 설정
	virtual void SetPress( bool press );

	virtual void SetRelativeRect( const cUIRect& rect );
	virtual void SetRelativePos( const cUIPos& pos  );
	
protected:
	/// 이벤트 처리
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnMouseHovered( const cUIPos& pos );
	virtual void OnMouseLeft( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateRect();
	virtual void UpdateSkin();

private:
	/// 버튼의 사이즈 변환시 사용 
	cUISize mUpSize;
	cUISize mDownSize;

	/// 탭버튼 이미지( 이미지 사이즈가 변환되므로 cPlaneObject 사용 
	cPlaneObject*	mpTabButtonImage;

	/// 이미지가 찍힐 화면상의 좌표 
	short mScreenX, mScreenY;
};
