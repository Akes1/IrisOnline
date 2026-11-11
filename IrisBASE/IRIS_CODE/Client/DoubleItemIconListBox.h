#pragma once
#include "ListBox.h"

// 양 라인 출력용 아이템 리스트 박스
class cItemIcon;
class cDoubleItemIconListBox : public cListBox
{
public:
	cDoubleItemIconListBox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cDoubleItemIconListBox();

	void Clear();
	void ClearText();

	///
	void SetEnableSelect( bool set );

	///
	void CreateIcon( cString skinName, int count );
	void SetSlotIndexAllIcon( int count, unsigned short slotIndex );

	/// Text 출력되는 너비 얻기
	unsigned int GetColumeWidth();

protected:
	void OnMouseMove( const cUIPos& pos );
	void OnMouseLeft( const cUIPos& pos );
	void OnLButtonDoubleClick( const cUIPos& pos );
	void OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	void OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift );
	void OnScrollbarMoved( const cUIPos& pos );
	void OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift );
	void OnIconDoubleClicked( cUINode* , unsigned int );
	void OnIconClicked( cUINode* , unsigned int );
	void OnIconRightClicked( cUINode* , unsigned int , bool );
	void OnMouseWheel( const cUIPos& pos, int wheel );
	void RenderData( cUIFontItemKeeper* pKeeper );
	void UpdateSelectImage();
	void UpdateScrollBar();
	void CheckButtonEvent( unsigned int id );
	void CheckShowImage();

protected:
	/// 아이콘 
	cItemIcon*	mIcon[16];

	int mMaxIconCount;

	///
	bool		mEnableSelect;
};

inline void cDoubleItemIconListBox::SetEnableSelect( bool set )
{
	mEnableSelect	= set;
	mShowImage		= set;
}
