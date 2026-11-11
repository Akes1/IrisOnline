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

	/// 스크롤바의 위치 세팅 
	void SetPosX( unsigned int x );
	void SetPosY( unsigned int y );

	/// 스크롤바의 높이값 세팅 
	void SetHeight( unsigned int h );

	/// 바 버튼만 안보이게 하기( 스크롤바가 바 버튼이 보일만큼 길지 않을떄 사용 )
	void SetVisibleBarButton( bool visible );

	/// 스크롤바 높이 값 얻기 ( 바 버튼이 스크롤 가능한 영역만 계산 )
	unsigned int GetScrollBarHeight();
	unsigned int GetScrollButtonHeight();

	/// 바 버튼 위치 값 얻기 
	unsigned int GetBarButtonPosY();

	/// 바 버튼이 이동할수 있는 영역 크기 얻음
	unsigned int GetScrollMarginHeight();

	/// 바 버튼의 제한 사이즈 구하기( 더이상 작아질수 없음 )
	unsigned int GetMinimumHeight();

	/// 스크롤바 바 버튼 업
	void MoveScrollUp( int h );

	/// 스크롤바 바 버튼 다운
	void MoveScrollDown( int h );

	void Show( bool onsound = true );

protected:
	/// 이벤트 처리
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnRender( cUIFontItemKeeper* pKeeper );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	virtual void OnLButtonUp( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateRect();
	virtual void UpdateSkin();
	virtual void OnShow();
	virtual void OnHide();
	
	/// 생성
	bool OnCreate( cUINodeProperty* pproperty );

	/// 바 버튼 이동 이벤트 발생시 
	void UpdateBarButtonPos( const cUIPos& pos );

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
	cPlaneObject*	mpScrollImage;
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

