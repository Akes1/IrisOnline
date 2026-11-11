/* ==========================================================================
* 파    일 : FriendListbox.h
* 목    적 : 
* 작 성 자 : 김가람
* 작 성 일 : 2007-07-06
* 주의사항 :
*===========================================================================*/
#pragma once
#include "ListBox.h"

/// 일반 리스트박스와 다르게 컬럼을 아랫줄에 출력하기 위해
/// 출력부가 변경된 리스트박스
class cFriendListbox : public cListBox
{
public:
	cFriendListbox( eUINodeType type = eUINODE_LISTBOX );
	virtual ~cFriendListbox();

protected:
	/// 리스트 박스의 아이템을 그려줌.
	virtual void RenderData( cUIFontItemKeeper* pKeeper );
};