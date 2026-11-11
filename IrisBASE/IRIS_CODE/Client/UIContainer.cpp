#include "StdAfx.h"
#include "UIContainer.h"

#include "InputSystem.h"
#include "TabWindow.h"
#include "RadioButton.h"
#include "UIRootNode.h"

cUIContainerSkin::cUIContainerSkin( eUINodeType type )
: cUINodeSkin( type )
{
}

cUIContainerSkin::~cUIContainerSkin()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
cUIContainer::cUIContainer( eUINodeType type )
: cUINode( type ) 
{
	mOnlyBackground = false;
	mHoldView = false;
	mExceptSnap = false;
}

/// 주의사항 : 
cUIContainer::~cUIContainer()
{
	/// 자식들에게 알림
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);
		if( n )
		{
			n->mpParent = 0;
			delete n;
		}
		else
		{
			assert(0);
		}
	}
}

/// 주의사항 : 차일드 완전히 삭제 ( 포인터까지 널로 함 )
void cUIContainer::RemoveChildComplete( cUINode* pchild )
{
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n == pchild )
		{
			UIMAN->GetRootNode()->RemoveProcessChild( n );
			UIMAN->GetRootNode()->RemoveSearchChild( n->GetID() );

			mChildList.Erase( i );
			n->mpParent = 0;
			SAFE_DELETE( n );

			return;
		}
	}
}

/// 주의사항 : 
void cUIContainer::RemoveChild( cUINode* pchild )
{
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n == pchild )
		{
			UIMAN->GetRootNode()->RemoveProcessChild( n );
			UIMAN->GetRootNode()->RemoveSearchChild( n->GetID() );

			n->mpParent = 0;
			mChildList.Erase( i );

			return;
		}
	}
}

/// 주의사항 : 
void cUIContainer::UpdateRect()
{
	cUINode::UpdateRect();

	/// 자식 갱신
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n )
		{
			n->UpdateRect();
		}
	}
}

/*
	자기 updateskin 
	자기 updaterect
	자식 updaterect

	자식 updateskin
	자기 updaterect 

	기본 2번 호출 
*/
void cUIContainer::UpdateSkin()
{
	cUINode::UpdateSkin();

	/// 자식 스킨 적용
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n )
			n->UpdateSkin();
	}
}

void cUIContainer::NetUnLockAllUI()
{
	cUINode::NetUnLockAllUI();

	/// 자식 적용
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n )
			n->NetUnLockAllUI();
	}
}

/// 주의사항 : 
void cUIContainer::AddChild( cUINode* pchild )
{
	if( pchild )
	{
//		if( pchild->mpParent )
//		{
//			pchild->mpParent->RemoveChild( pchild );
//		}
//		pchild->mpParent = this;
		mChildList.PushBack( pchild );
	}
}

/// 주의사항 : 에디트 박스 삽입
int cUIContainer::AddEditBox( cUINode* pchild )
{
	int i = 0;
	if( pchild )
	{
		i = mEditArr.GetSize();
		mEditArr.PushBack( pchild );
	}
	else
	{
		assert( 0 && "null node" );
		return -1;
	}
	return i;
}

/// 주의사항 : 포커스를 갖는 에디트 박스 체인지
/// 이 함수는 컨테이너 내부에 존재하는 EditBox들 간의 포커스 이동을 제어한다.
void cUIContainer::ChangeFocus( unsigned int index )
{
	/// error check
	if( index >= mEditArr.GetSize() )
	{
		assert( 0 && "index out of range" );
		return ;
	}

	unsigned int newFocus = index + 1;
	if( newFocus >= mEditArr.GetSize() )
	{
		newFocus = 0;
	}

	((cUINode*)mEditArr[newFocus])->SetFocus();
}


/// 주의사항 : 라디오 버튼을 삽입 ( 그룹넘버, 포인터 )
void cUIContainer::AddRadio( unsigned int groupNum, cUINode* pchild )
{
	if( !pchild )
	{
		assert( 0 && "null node" );
		return;
	}

	/// 그룹넘버가 0부터 시작하지 않았으면 에러!!
	if( groupNum > mHeadRadioArr.GetSize() )
	{	
		assert( 0 && "error!! start group number at 0" );
		return;
	}

	/// 그룹의 처음 등록한 라디오만 삽입.
	if( mHeadRadioArr.GetSize() == groupNum ) 
	{
		mHeadRadioArr.PushBack( pchild );
	}
	else
	{
		/// 이미 존재하는 그룹번호이면, 연결시킨다
		((cRadioButton*)mHeadRadioArr[groupNum])->Link( pchild );
	}
}

/// 주의사항 : 해당그룹의 헤드 라디오 버튼을 리턴 
cUINode* cUIContainer::GetHeadRadio( unsigned int groupNum )
{
	if( groupNum >= mHeadRadioArr.GetSize() )
	{
		assert( 0 && "wrong radio group num");
		return 0;
	}

	return (cUINode*)mHeadRadioArr[groupNum];
}

/// 주의사항 : 부모 밑에 해당id를 갖는 자식 찾기
cUINode* cUIContainer::GetChild( int id )
{
	cUINode* n = 0;
	cChildList::cConstIterator i = mChildList.Begin();
	cChildList::cConstIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		n = (cUINode*)(*i);
		if( n && n->GetID() == id )
			return n;
	}
	return 0;
}

cUINode* cUIContainer::GetNode( const cUIPos& pos ) const
{
	/// 각 영역만 우선 검사
	if( mVisible && cUINode::ContainPoint( pos ) == true )
	{
		cUINode* node = 0;

		/// 화면상 앞에서 뒤로 검사
		cChildList::cConstIterator i = (--mChildList.End());
		cChildList::cConstIterator end = (--mChildList.Begin());

		for( ; i != end; --i )
		{
			cUINode* n = (cUINode*)(*i);

			if( n->IsEnabled() == false || n->IsVisible() == false )
				continue;

			if( n->IsKindof( eUINODE_CONTAINER ) )
			{
				node = ((cUIContainer*)n)->GetNode( pos );
			}
			else if( n->ContainPoint( pos ) )
			{
				if( n->IsExceptEvent() == false )
					node = n;
			}

			/// 텍스쳐 알파 영역 검사까지 함
			if( node && node->ContainPoint( pos ) )
				return node;
		}

		/// 이벤트 무시, 혹은 뚫린 텍스쳐인지 검사
		if( mExceptionEvent == false && ContainPoint( pos ) == true )
			return (cUINode*)this;
		else
			return 0;
	}
	else
		return 0;
}

/// 주의사항 : 주어진 영역과 교차하는 영역이 있는 컨테이너를 리턴 ( 무조건 루트의 자식들만 가능 )
cUIContainer* cUIContainer::GetNearRootChild( cUIContainer* con, bool axisX )
{
	if( con && con->IsExceptSnap() == true )
		return 0;

	cUIRect rect;

	/// 윈도우의 안쪽으로도 붙기때문에 안쪽으로 ( AUTO_MAGNATIC_DIST / 2 ) 픽셀,
	/// 바깥으로 ( AUTO_MAGNATIC_DIST / 2 ) 픽셀 더해서 검사하도록 한다 
	rect.mLeft = con->GetAbsoluteRect().mLeft - ( AUTO_MAGNATIC_DIST / 2 );
	rect.mRight = con->GetAbsoluteRect().mRight + ( AUTO_MAGNATIC_DIST / 2 );
	rect.mTop = con->GetAbsoluteRect().mTop - ( AUTO_MAGNATIC_DIST / 2 );
	rect.mBottom = con->GetAbsoluteRect().mBottom + ( AUTO_MAGNATIC_DIST / 2 );

	cUIContainer* result = 0;
	
	/// 자기 자신이 visible 상태이고 또한 컨테이너야 한다. 
	if( mVisible && IsKindof( eUINODE_CONTAINER ) )
	{
		cChildList::cConstIterator i = (--mChildList.End());
		cChildList::cConstIterator end = (--mChildList.Begin());

		cUIContainer* pleftCon = 0;
		cUIContainer* prightCon = 0;
		cUIContainer* ptopCon = 0;
		cUIContainer* pbottomCon = 0;
		/// 각 방향의 최단거리
		unsigned int rDist = 0;
		unsigned int lDist = 0;
		unsigned int tDist = 0;
		unsigned int bDist = 0;

		for( ; i != end; --i )
		{
			cUIContainer* n = (cUIContainer*)(*i);

			/// 비교대상이 vislble 상태가 아니거나 현재 자신과 동일하거나, 백그라운드면 넘긴다 
			/// 자석기능 검사 제외대상이면 넘긴다
			if( n->IsVisible() == false || con == n || 
				n->IsOnlyBackground() || n->IsExceptSnap() == true )
			{
				continue;
			}

			/// 가장 가까운 컨테이너 찾아서 리턴한다 
			if( rect.IntersectRect( n->GetAbsoluteRect() ) )
			{			
				//// x 축검사 
				if( axisX == true )
				{
					/// 오른쪽 변과 가장 가까운 컨테이너 찾기 
					if( rect.mRight - AUTO_MAGNATIC_DIST <= n->GetAbsoluteRect().mLeft ) 
					{
						if( prightCon == 0 || prightCon->GetAbsoluteRect().mLeft > n->GetAbsoluteRect().mLeft )
						{
							/// right 방향으로 가장 가까운 컨테이너 갱신 
							prightCon = n;
							rDist = n->GetAbsoluteRect().mLeft - rect.mRight - AUTO_MAGNATIC_DIST;
						}
					}
					/// 왼쪽 변과 가장 가까운 컨테이너 찾기 
					if( rect.mLeft + AUTO_MAGNATIC_DIST >= n->GetAbsoluteRect().mRight ) 
					{
						if( pleftCon == 0 || pleftCon->GetAbsoluteRect().mRight < n->GetAbsoluteRect().mRight )
						{	
							/// left 방향으로 더 가까운 컨테이너가 있으면 갱신 
							pleftCon = n;
							lDist = rect.mLeft + AUTO_MAGNATIC_DIST - n->GetAbsoluteRect().mRight;
						}
					}
					/// 옆에 있는 윈도우와 left 값이 일정거리로 가까워지면 붙여버린다 
					if( rect.mLeft >= n->GetAbsoluteRect().mLeft && rect.mLeft - n->GetAbsoluteRect().mLeft <= 0 ||
						rect.mLeft <= n->GetAbsoluteRect().mLeft && n->GetAbsoluteRect().mLeft - rect.mLeft <= AUTO_MAGNATIC_DIST )
					{
						con->mSnapType = eSNAP_LEFT_OUT;
						return n;
					}
					/// 옆에 있는 윈도우와 right 값이 일정거리로 가까워지면 붙여버린다 
					if( rect.mRight >= n->GetAbsoluteRect().mRight && rect.mRight - n->GetAbsoluteRect().mRight <= AUTO_MAGNATIC_DIST ||
						rect.mRight <= n->GetAbsoluteRect().mRight && n->GetAbsoluteRect().mRight - rect.mRight <= 0 )
					{
						con->mSnapType = eSNAP_RIGHT_OUT;
						return n;
					}
				}
				/// Y 축검사 
				else if( axisX == false )
				{
					/// 윗 변과 가장 가까운 컨테이너 찾기 
					if( rect.mTop + AUTO_MAGNATIC_DIST >= n->GetAbsoluteRect().mBottom ) 
					{
						if( ptopCon == 0 || ptopCon->GetAbsoluteRect().mTop > n->GetAbsoluteRect().mTop )
						{
							ptopCon = n;
							tDist = rect.mTop + AUTO_MAGNATIC_DIST - n->GetAbsoluteRect().mBottom;
						}
					}
					/// 아래 변과 가장 가까운 컨테이너 찾기 
					if( rect.mBottom - AUTO_MAGNATIC_DIST <= n->GetAbsoluteRect().mTop ) 
					{
						if( pbottomCon == 0 || pbottomCon->GetAbsoluteRect().mTop < n->GetAbsoluteRect().mTop )
						{
							pbottomCon = n;
							bDist = n->GetAbsoluteRect().mBottom - rect.mBottom - AUTO_MAGNATIC_DIST;
						}
					}
					/// 옆에 있는 윈도우와 top 값이 일정거리로 가까워지면 붙여버린다 
					if( rect.mTop >= n->GetAbsoluteRect().mTop && rect.mTop - n->GetAbsoluteRect().mTop <= 0 ||
						rect.mTop <= n->GetAbsoluteRect().mTop && n->GetAbsoluteRect().mTop - rect.mTop <= AUTO_MAGNATIC_DIST )
					{
						con->mSnapType = eSNAP_TOP_OUT;
						return n;
					}
					/// 옆에 있는 윈도우와 bottom 값이 일정거리로 가까워지면 붙여버린다 
					if( rect.mBottom >= n->GetAbsoluteRect().mBottom && rect.mBottom - n->GetAbsoluteRect().mBottom <= AUTO_MAGNATIC_DIST ||
						rect.mBottom <= n->GetAbsoluteRect().mBottom && n->GetAbsoluteRect().mBottom - rect.mBottom <= 0 )
					{
						con->mSnapType = eSNAP_BOTTOM_OUT;
						return n;
					}
				}
			}
		}

		/// 모든 검사가 끝났으면 가장가까운 컨테이너를 리턴
		/// x 축으로 계산
		if( axisX == true )
		{
			if( prightCon == 0 )
			{
				con->mSnapType = eSNAP_RIGHT;	/// 오른쪽에 붙여라 
				result = pleftCon;
			}
			else if( pleftCon == 0 ) 
			{
				con->mSnapType = eSNAP_LEFT;
				result =  prightCon;
			}
			else if( rDist <= lDist )
			{
				con->mSnapType = eSNAP_LEFT;
				result = prightCon;			
			}
			else if( rDist > lDist )
			{
				con->mSnapType = eSNAP_RIGHT;
				result = pleftCon;
			}
		}
		/// y 축으로 계산 
		else if( axisX == false )
		{
			if( ptopCon == 0 )
			{
				con->mSnapType = eSNAP_TOP;
				result =  pbottomCon;
			}
			else if( pbottomCon == 0 ) 
			{
				con->mSnapType = eSNAP_BOTTOM;
				result =  ptopCon;
			}
			else if( tDist >= bDist )
			{
				con->mSnapType = eSNAP_BOTTOM;
				result =  ptopCon;			
			}
			else if( tDist < bDist )
			{
				con->mSnapType = eSNAP_TOP;
				result =  pbottomCon;
			}
		}
	}
	return result;
}

/// 주의사항 : 
bool cUIContainer::HandleEvent( const cUIEvent& event )
{
	/// 부모가 보이지 않으면 자식들의 이벤트 처리 불가
	if( mVisible == false )
	{
		return false;
	}

	switch( event.mType )
	{
	case eUIEVENT_COMMAND:
		switch( event.mID )
		{
		case eUIID_OK:
			OnOk();
			break;
		case eUIID_CANCEL:
			OnCancel();
			break;
		default:
			OnCommand( event.mpCaller, event.mID );
		}
		break;

	case eUIEVENT_CHECKBOX_CHANGED:
		OnCheckBoxChanged( event.mpCaller, event.mID );
		break;

	case eUIEVENT_EDITBOX_ENTER:
		OnEditBoxEntered( event.mpCaller, event.mID );
		break;

	case eUIEVENT_LISTBOX_CHANGED:
		OnListBoxChanged( event.mpCaller, event.mID );
		break;

	case eUIEVENT_SCROLLBAR_MOVE:
		OnScrollbarMoved( event.mpCaller, event.mID );
		break;

		/// 일정시간마다 버튼이 계속 눌려 있음을 보냄.
	case eUIEVENT_BUTTON_DOWNED:
		OnButtonDowned( event.mpCaller, event.mID );
		break;

		/// 플래그가 세팅되어있으면 매 프로세스마다 눌림 이벤트 발생
	case eUIEVENT_BUTTON_CONTINUEDOWNED:
		OnButtonContinueDowned( event.mpCaller, event.mID );
		break;

	case eUIEVENT_LISTBOX_SELECTED:
		OnListBoxSelected( event.mpCaller, event.mID );
		break;

	case eUIEVENT_LISTBOX_SELECTEDRBUTTON:
		OnListBoxSelectedRButton( event.mpCaller, event.mID, event.mShift );
		break;

		/// 라디오 버튼이 눌림 이벤트 
	case eUIEVENT_RADIO_PRESSED:
		OnRadioPressed( event.mpCaller, event.mID );
		break;

		/// 푸쉬버튼이 눌린 이벤트 
	case eUIEVENT_PUSHBUTTON_PRESSED:
		OnPushButtonPressed( event.mpCaller, event.mID );
		break;

		/// 탭버튼 눌림
	case eUIEVENT_TABBUTTON_PRESSED:
		OnTabButtonPressed( event.mpCaller, event.mID );
		break;

	case eUIEVENT_ICON_DRAGGED:
 		OnIconDragged( event.mpCaller, event.mID, cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()), event.mShift );
		break;

	case eUIEVENT_ICON_DOUBLECLICKED:
		OnIconDoubleClicked( event.mpCaller, event.mID );
		break;

	case eUIEVENT_ICON_HOVERED:
		OnIconHovered( event.mpCaller, event.mID, cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
		break;

	case eUIEVENT_ICON_CLICKED:
		OnIconClicked( event.mpCaller, event.mID );
		break;

	case eUIEVENT_ICON_RIGHTCLICKED:
		OnIconRightClicked( event.mpCaller, event.mID, event.mShift );
		break;
	case eUIEVENT_IMAGE_HOVERED:
		OnImageHovered( event.mpCaller, event.mID, cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
		break;
	case eUIEVENT_IMAGE_CLICKED:
		OnImageClicked( event.mpCaller, event.mID );
		break;
	case eUIEVENT_LISTBOX_ROW_HOVERED:
		OnListBoxRowHovered( event.mpCaller, event.mID, event.mPos, event.mExtraData );
		break;
	case eUIEVENT_LISTBOX_ROW_LEFT:
		OnListBoxRowLeft( event.mpCaller, event.mID );
		break;
	case eUIEVENT_LISTBOX_IMAGE_HOVERED:
		OnListBoxImageHorvered( event.mpCaller, event.mID, event.mPos, event.mExtraData );
		break;
	case eUIEVENT_LISTBOX_IMAGE_LEFT:
		OnListBoxImageLeft( event.mpCaller, event.mID );
		break;

		/// 게이지가 목표치까지 완료됨
	case eUIEVENT_GAUGE_END:
		OnGaugeEnd( event.mpCaller, event.mID );
		break;

	case eUIEVENT_COMBO_SELECTED:
		OnComboSelected( event.mpCaller, event.mID );
		break;

	case eUIEVENT_OPTIONGAUGE_UPDATE:
		OnOptiongaugeUpdate( event.mpCaller, event.mID );
		break;

	case eUIEVENT_MOUSE_OTHERCLICK:
		OnMouseOtherClick( event.mPos );
		break;

	default:
		return cUINode::HandleEvent( event );
	}
	return true;
}