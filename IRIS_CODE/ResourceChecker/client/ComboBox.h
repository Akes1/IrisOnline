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
