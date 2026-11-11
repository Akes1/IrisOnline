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
	/// 갱신
	virtual void UpdateRect();

	/// 스킨
	virtual void UpdateSkin();
	

	/// 이벤트를 처리
	virtual bool HandleEvent( const cUIEvent& event );

	/// 자식을 추가
	virtual void AddChild( cUINode* pchild );

	/// 자식을 제거
	virtual void RemoveChild( cUINode* pchild );

	/// 부모로부터 자식을 완전히 제거( 포인터까지 없앤다 )
	virtual void RemoveChildComplete( cUINode* pchild );

	/// 자식 리스트를 리턴
	tPointerList<void*>* GetChildList();

	/// 노드를 리턴
	/// 위치에 따라 자식 또는 자기 자신을 리턴한다.
	cUINode* GetNode( const cUIPos& pos ) const;

	/// 컨테이너를 리턴
	/// 루트의 child 중 해당 영역과 가장 가까운 컨테이너를 찾는다 
	cUIContainer* GetNearRootChild( cUIContainer* con, bool axisX );

	/// 백그라운드로 지정 
	void SetOnlyBackground( bool value );

	/// 뷰 홀드 세팅 여부
	void SetHoldView( bool hold );

	/// 백그라운드인지 여부 리턴 
	bool IsOnlyBackground();

	/// 반드시 보여져야하는 윈도우인지 여부 리턴
	bool IsHoldView();

	////// 특별한 자식 관리 
	/// 포커스 갖는 에디트 박스 체인지 
	void ChangeFocus( unsigned int index );

	/// 에디트박스 탭을 삽입
	int AddEditBox( cUINode* pchild );

	/// 라디오 버튼을 삽입 
	void AddRadio( unsigned int groupNum, cUINode* pchild );

	/// 헤드 라디오 버튼을 리턴 
	cUINode* GetHeadRadio( unsigned int groupNum );

	/// 자석기능 사용여부 ( 다른 컨테이너가 붙게 할지 여부 )
	void SetExceptSnap( bool set );
    bool IsExceptSnap();	/// 자석검사시 제외여부

	/// 모든 UI  락 풀기
	void NetUnLockAllUI();

	/// 자식을 리턴
	cUINode* GetChild( int id );

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

inline tPointerList<void*>* cUIContainer::GetChildList()
{
	return &mChildList;
}

inline void cUIContainer::SetOnlyBackground( bool value )
{ 
	mOnlyBackground = value;
}

inline void cUIContainer::SetHoldView( bool hold )
{
	mHoldView = hold;
}

inline bool cUIContainer::IsOnlyBackground()
{
	return mOnlyBackground;
}

inline bool cUIContainer::IsHoldView()
{
	return mHoldView;
}

inline void cUIContainer::SetExceptSnap( bool set )
{
	mExceptSnap = set;
}

inline bool cUIContainer::IsExceptSnap()
{
	return mExceptSnap;
}
