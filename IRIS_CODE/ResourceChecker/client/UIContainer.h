/* ==========================================================================
* 파    일 : UIContainer.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.29
* 주의사항 : 
*===========================================================================*/
#pragma once
#include "UINode.h"
#include "UINodeData.h"

/// UI 컨테이너 스킨
class cUIContainerSkin : public cUINodeSkin
{
public:
	cUIContainerSkin( eUINodeType type = eUINODE_CONTAINER );
	virtual ~cUIContainerSkin();
};

/////////////////////////////////////////////////////////////////////////////////////

/// UI 컨테이너
/// 다른 UI 노드를 자식으로 가질 수 있다.
class cUIContainer : public cUINode
{
public:

protected:
	cUIContainer( eUINodeType type = eUINODE_CONTAINER );
	virtual ~cUIContainer();

protected:
	/// 이벤트
	virtual void OnOk() {}
	virtual void OnCancel() {}
	virtual void OnCommand( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnCheckBoxChanged( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnEditBoxEntered( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnListBoxChanged( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnListBoxSelected( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnListBoxSelectedRButton( cUINode* /*caller*/, unsigned int /*id*/, bool /*shift*/ ) {}
	virtual void OnScrollbarMoved( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnButtonDowned( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnButtonContinueDowned( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnRadioPressed( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnPushButtonPressed( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnTabButtonPressed( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnIconHovered( cUINode* /*caller*/, unsigned int /*id*/, const cUIPos& /*pos*/ ) {}
	virtual void OnIconLeft( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnIconDragged( cUINode* /*caller*/, unsigned int /*id*/, const cUIPos& /*pos*/, bool /*shift*/ ) {}
	virtual void OnIconDoubleClicked( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnIconClicked( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnIconRightClicked( cUINode* /*caller*/, unsigned int /*id*/, bool /*shift*/ ) {}
	virtual void OnImageHovered( cUINode* /*caller*/, unsigned int /*id*/, const cUIPos& /*pos*/ ) {}
	virtual void OnImageClicked( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnListBoxRowHovered( cUINode* /*caller*/, unsigned int /*id*/, const cUIPos& /*pos*/, void* /*extraData*/ ) {}
	virtual void OnListBoxRowLeft( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnListBoxImageHorvered( cUINode* /*caller*/, unsigned int /*id*/, const cUIPos& /*pos*/, void* /*extraData*/ ) {}
	virtual void OnListBoxImageLeft( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnGaugeEnd( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnComboSelected( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnOptiongaugeUpdate( cUINode* /*caller*/, unsigned int /*id*/ ) {}
	virtual void OnMouseOtherClick( const cUIPos& /*pos*/ ) {}

protected:
	/// 자식 리스트
	typedef tPointerList<void*> cChildList;
	cChildList mChildList;

	/// 에디트 박스 리스트 (동일한부모 끼리만 적용됨)
	typedef tPointerArray<void*> cEditArr;
	cEditArr mEditArr;

	/// 헤드 라디오 버튼 배열
	typedef tPointerArray<void*> cRadioArr;
	cRadioArr mHeadRadioArr;

public:
	/// 백그라운드로 생성된 윈도우
	bool mOnlyBackground;

	/// 반드시 보여야하는 윈도우 ( visible 로 세팅할수 없도록 한다 )
	bool mHoldView;

private:
	/// 자석기능 적용여부 ( true 이면 다른 컨테이너들이 붙지 않는다 )
	bool mExceptSnap;
};
