#include "StdAfx.h"
#include "UIRootNode.h"
#include "UIWindow.h"
#include "UIManager.h"
#include "UIMsgboxEvent.h"

/// 주의사항 : 
cUIRootNode::cUIRootNode( eUINodeType type )
: cUIContainer( type )
, mFontItemKeeper( 0 )
, mTipWindow( 0 )
{
	mpParent = 0;
	mID = 0;
	mpImage = 0;

	mRelativeRect.Set( 0, 0, GetScreenWidth(), GetScreenHeight() );
	mAbsoluteRect.Set( 0, 0, GetScreenWidth(), GetScreenHeight() );

	mVisible = true;
	mEnabled = true;

	mFontItemKeeper = new cUIFontItemKeeper;
}

/// 주의사항 : 
cUIRootNode::~cUIRootNode()
{
	SAFE_DELETE(mFontItemKeeper);

	mProcessList.Clear();
	mChildVisArray.Clear();
	mFindChildMap.Clear();
}

void cUIRootNode::HideAllChild( cUIWindow* except )
{
	/// 가시 여부를 백업
	mChildVisArray.Clear();

	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator iend = mChildList.End();
	for( ; i != iend; ++i )
	{
		cUINode* n = (cUINode*)(*i);
		if( n && n->IsKindof( eUINODE_WINDOW ) )
		{
			cUIWindow* w = (cUIWindow*)n;

			if( w->GetModalType() != eMSGBOX_NONE )
				continue;

			/// visible 을 백업하고, visible 상태면 저장
			if( w != except && w->IsBackupVisible() && w->IsVisible() )
			{
				mChildVisArray.PushBack( w );
			}
		}
	}

	/// 전체 UI 숨김 ( except 제외 )
	cChildList::cIterator b = mChildList.Begin();
	cChildList::cIterator bend = mChildList.End();
	for( ; b != bend; ++b )
	{
		cUINode* n = (cUINode*)(*b);
		if( n && n->IsKindof( eUINODE_WINDOW ) )
		{
			cUIWindow* w = (cUIWindow*)n;

			if( w->GetModalType() != eMSGBOX_NONE )
				continue;

			if( n != except && w->IsVisible() )
				w->Hide( false );	
		}
		else
		{
			assert(0);
		}
	}
}

void cUIRootNode::ShowAllChild()
{
	/// 가시 여부를 복구
	for( unsigned int i = 0; i < mChildVisArray.GetSize(); ++i )
	{
		cUIWindow* w = (cUIWindow*)mChildVisArray[i];
		if( w )
			w->Show( false );
		else
			assert(0);
	}

	mChildVisArray.Clear();

	/// 특별 윈도우들 복구
	UIMAN->RestoreShow();
}


/// 주의사항 : 
bool cUIRootNode::Create( const cString&, cUIContainer*, unsigned int, bool )
{
	assert( 0 );
	return true;
}

/// 주의사항 : 
void cUIRootNode::UpdateRect()
{
	assert( 0 );
}

void cUIRootNode::UpdateSkin()
{
	/// 해상도 변화에따른 루트노드의 사이즈도 변경 
	cUIRect rc( 0, 0, GetScreenWidth(), GetScreenHeight() );
	mAbsoluteRect = rc;
	mRelativeRect = rc;

	/// 자식에 스킨 적용
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	cUINode* n = 0;
	for( ; i != end; ++i )
	{
		n = (cUINode*)(*i);
		if( n )
			n->UpdateSkin();
	}
}


/// 주의사항 : 
bool cUIRootNode::SetSkin( const cUINodeSkin* )
{
	return true;
}

/// 주의사항 : 
bool cUIRootNode::IsValid() const
{
	return true;
}

/// 주의사항 : 
bool cUIRootNode::HandleEvent( const cUIEvent& )
{
	return true;
}

void cUIRootNode::OnPreRender()
{
	if( mVisible == false )
		return;

	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n && n->IsVisible() == true )
			n->OnPreRender();
	}
}

/// 주의사항 : 
void cUIRootNode::OnRender( cUIFontItemKeeper* )
{
	if( mVisible )
	{
		/// 자식 렌더링
		cChildList::cIterator i = mChildList.Begin();
		cChildList::cIterator end = mChildList.End();

		for( ; i != end; ++i )
		{
			cUINode* n = (cUINode*)(*i);

			if( n == mTipWindow )
				continue;

			if( n && n->IsVisible() == true )
				n->OnRender( mFontItemKeeper );
		}
		mFontItemKeeper->DrawAll();
	}
}

// 팁 그리기
void cUIRootNode::OnRenderTip()
{
	if( mVisible == false )
		return;

	/// 팁 렌더링
	if( mTipWindow )
	{
		mTipWindow->OnRender( mFontItemKeeper );
		mFontItemKeeper->DrawAll();
	}
}

/// 주의사항 : 
void cUIRootNode::OnProcess( unsigned long deltaTime, unsigned long accumTime )
{
	if( mVisible )
	{
		cProcessChildList::cIterator i = mProcessList.Begin();
		cProcessChildList::cIterator end = mProcessList.End();

		for( ; i != end; ++i )
		{
			cUINode* node = ( cUINode* )( *i );
			if( node )
			{
				node->OnProcess( deltaTime, accumTime );
			}
			else
			{
				assert(0);
			}
		}
	}
}

cUIContainer* cUIRootNode::GetContainer( const cUIPos& pos ) const
{
	/// 화면상 맨 앞에서 뒤로 검사
	cChildList::cConstIterator i = (--mChildList.End());
	cChildList::cConstIterator end = (--mChildList.Begin());

	for( ; i != end; --i )
	{
		cUINode* n = (cUINode*)(*i);
		if( !n )
		{
			assert(0);
			return 0;
		}

		if( n->IsVisible() == false )
		{
			continue;
		}

		if( n->IsKindof(eUINODE_CONTAINER) )
		{
			if( n->ContainPoint( pos ) )
			{
				return (cUIContainer*)n;
			}
		}
	}

	return 0;
}

void cUIRootNode::Open()
{
	if( mChildList.GetSize() > 0 )
	{
		cChildList::cIterator i = mChildList.Begin();
		cChildList::cIterator end = mChildList.End();

		for( ; i != end; i++ )
		{
			cUINode* n = (cUINode*)(*i);
			if( n && n->IsKindof( eUINODE_WINDOW ) )
			{
				cUIWindow* con = (cUIWindow*)n;
				if( con )
					con->Open();
			}
		}
	}
}

void cUIRootNode::Close()
{
	if( mChildList.GetSize() > 0 )
	{
		/// 화면상 앞에서 뒤로 검사 
		cChildList::cConstIterator i = (--mChildList.End());
		cChildList::cConstIterator end = (--mChildList.Begin());

		for( ; i != end; --i )
		{
			cUINode* n = (cUINode*)(*i);
			if( n && n->IsKindof( eUINODE_WINDOW ) )
			{
				cUIWindow* con = (cUIWindow*)n;
				if( con )
					con->Close();
			}
		}
	}
}

/// 윈도우를 닫는다.
bool cUIRootNode::HideESC()
{
	/// 자식갯수가 하나라도 있어야한다.
	if( mChildList.GetSize() > 0 )
	{
		/// 화면상 앞에서 뒤로 검사 
		cChildList::cConstIterator i = (--mChildList.End());
		cChildList::cConstIterator end = (--mChildList.Begin());
		
		for( ; i != end; --i )
		{
			////// node인데 자식인것들 따로 체크해보기 
			cUINode* n = (cUINode*)(*i);

			/// root 의 자식은 반드시 컨테이너여야 한다. 
			if( n && n->IsKindof( eUINODE_WINDOW ) )
			{
				cUIWindow* con = (cUIWindow*)n;

				/// 반드시 보여야하는 윈도우들만 빼고 처리한다 
				if( con && con->IsVisible() && con->IsHoldView() == false )
				{
					con->HideESC( true );
					return true;
				}
			}
		}
	}

	return false;
}

void cUIRootNode::NetUnLockAllUI()
{
	/// 자식갯수가 하나라도 있어야한다.
	if( mChildList.GetSize() > 0 )
	{
		/// 화면상 앞에서 뒤로 검사 
		cChildList::cConstIterator i = (--mChildList.End());
		cChildList::cConstIterator end = (--mChildList.Begin());

		for( ; i != end; --i )
		{
			////// node인데 자식인것들 따로 체크해보기 
			cUINode* n = (cUINode*)(*i);

			/// root 의 자식은 반드시 컨테이너여야 한다. 
			if( n && n->IsKindof( eUINODE_WINDOW ) )
			{
				cUIWindow* con = (cUIWindow*)n;

				if( con )
					con->NetUnLockAllUI();
			}
		}
	}
}

void cUIRootNode::AddProcessChild( cUINode* n )
{
	if( n->IsEnableProcess() )
		mProcessList.PushFront( n );

	if( n->IsKindof( eUINODE_CONTAINER ) == true )
	{
		cUIContainer* con = (cUIContainer*)n;
		tPointerList<void*>* plist = con->GetChildList();
		if( plist->IsEmpty() )
			return;

		tPointerList<void*>::cIterator i = plist->Begin();
		tPointerList<void*>::cIterator iend = plist->End();
		for( ; i != iend; ++i )
		{
			AddProcessChild( (cUINode*)(*i) );
		}
	}
}

void cUIRootNode::RemoveProcessChild( cUINode* n )
{
	if( n->IsEnableProcess() )
		mProcessList.Remove( n );

	if( n->IsKindof( eUINODE_CONTAINER ) == true )
	{
		cUIContainer* con = (cUIContainer*)n;
		tPointerList<void*>* plist = con->GetChildList();
		if( plist->IsEmpty() )
			return;

		tPointerList<void*>::cIterator i = plist->Begin();
		tPointerList<void*>::cIterator iend = plist->End();
		for( ; i != iend; ++i )
		{
			cUINode* p = (cUINode*)(*i);
			RemoveProcessChild( p );
		}
	}
}

// 메세지박스는 제외
void cUIRootNode::AddSearchChild( cUINode* n )
{
	if( n )
	{
		if( n->GetID() > eUIID_ID_BEGIN )
		{
			if( mFindChildMap.Insert( n->GetID(), n ) == false )
			{
				assert(0);
				return;
			}
		}
	}
	else
		assert(0);
}

void cUIRootNode::RemoveSearchChild( unsigned long id )
{
	if( mFindChildMap.IsEmpty() == false )
		mFindChildMap.Erase( id );
}

cUINode* cUIRootNode::FindNodeByID( unsigned long id )
{
	return (cUINode*)mFindChildMap.GetAt( id );
}
