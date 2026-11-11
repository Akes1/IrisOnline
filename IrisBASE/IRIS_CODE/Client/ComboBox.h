/* ==========================================================================
* 파    일 : ComboBox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2006-11-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "UIWindow.h"
#include "UINodeData.h"

class cListBox;
class cButton;
class cEditBox;

/// 콤보 박스
class cComboBox : public cUIWindow
{
public:
	cComboBox( eUINodeType type = eUINODE_COMBOBOX );
	virtual ~cComboBox();

	/// 콤보박스의 자식인 리스트 박스에 아이템을 넣는다 
	void AddItem( LPCTSTR text, void* extraData );
	bool IsEmpty();

	/// 선택된 열의 데이터
	void* GetSelectData();

	///
	void Clear();

	/// 선택 세팅
	void SetSelect( int row, bool sendEvent = true );

	virtual void SetEnabled( bool enabled );

	unsigned int GetSize();
	void* GetExtraData( unsigned int index );
	unsigned int GetSelectIndex();

protected:
	/// 이벤트 처리
	virtual bool HandleEvent( const cUIEvent& event );
	virtual void OnLButtonDown( const cUIPos& pos, bool ctrl, bool /*AltPress*/, bool /*shiftPress*/ );
	virtual void OnMouseOtherClick( const cUIPos& pos );
	virtual bool SetSkin( const cUINodeSkin* pskin );
	virtual void UpdateSkin();
	virtual void OnHide();

	/// 생성 
	bool OnCreate( cUINodeProperty* pproperty );
	/// 버튼 체크
	void CheckButtonEvent( cUINode* pcaller, unsigned int id );

	void UpdateListboxSize();

protected:
	/// 자식 리스트 박스
	cListBox* mpListBox;
	/// 자식 버튼
	cButton* mpButton;
	/// 자식 에디트
	cEditBox* mpEdit;
	/// 리스트 박스 스킨 네임
	cString mListboxSkin;
	/// 버튼 박스 스킨 네임
	cString mButtonSkin;
	/// 에디트 박스 스킨 네임
	cString mEditSkin;	
	/// 스크롤 스킨
	cString mScrollSkin;
	/// 에디트박스의 텍스트
	cStringT mEditText;

	cUISize mSize;
};

///////////////////////////////////////////////////////////////////
class cComboBoxSkin : public cUINodeSkin
{
public:
	cComboBoxSkin( eUINodeType type = eUINODE_COMBOBOX );
	virtual ~cComboBoxSkin();
	/// 로딩
	virtual bool Load( cParser& parser );

public:
	/// 리스트 박스 스킨 네임
	cString mListboxSkin;
	/// 버튼 박스 스킨 네임
	cString mButtonSkin;
	/// 에디트 박스 스킨 네임
	cString mEditSkin;
	/// 스크롤바 스킨 
	cString mScrollSkin;
};
