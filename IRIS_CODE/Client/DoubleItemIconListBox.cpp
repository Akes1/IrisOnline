#include "StdAfx.h"
#include "DoubleItemIconListBox.h"
#include "FontAgent.h"
#include "ItemIcon.h"
#include "PlaneObject.h"
#include "ScrollBar.h"
#include "inputsystem.h"
#include "ItemManager.h"

cDoubleItemIconListBox::cDoubleItemIconListBox( eUINodeType type )
: cListBox( type )
, mEnableSelect( true )
, mMaxIconCount(0)
{
	memset( mIcon, 0, sizeof(mIcon) );
}

cDoubleItemIconListBox::~cDoubleItemIconListBox()
{
}

void cDoubleItemIconListBox::Clear()
{
	cListBox::Clear();

	/// 아이콘 초기화
	for( int i = 0; i < mMaxIconCount; ++i )
	{
		if( mIcon[i] )
		{
			mIcon[i]->SetItemIndex(0);
			mIcon[i]->ChangeImage(0);
		}
	}
}

void cDoubleItemIconListBox::ClearText()
{
	cListBox::Clear();
}

void cDoubleItemIconListBox::CreateIcon( cString skinName, int count )
{
	mMaxIconCount = count;

	/// 아이콘 설정
	cString str;
	for( int i = 0; i < count; ++i )
	{
		str.Format( skinName.Cstr(), i );
		cItemIcon* icon = new cItemIcon;
		if( icon->CreateBySkinName( str, this, i ) == false )
			return;

		mIcon[i] = icon;
	}
}

void cDoubleItemIconListBox::SetSlotIndexAllIcon( int count, unsigned short slotIndex )
{
	for( unsigned short i = 0; i < count; ++i )
	{
		if( mIcon[i] )
			mIcon[i]->SetSlotIndex( slotIndex + i );
	}
}

void cDoubleItemIconListBox::RenderData( cUIFontItemKeeper* pKeeper )
{
	/// 전체 리스트를 얻어옴 
	if( GetRowCount() <= 0 )
		return;

	/// 리스트 박스의 좌표 받아오기
	int textposX = 0;
	int textPosY = GetAbsoluteRect().mTop + 1;
	unsigned int colY = 0;

	int rowCount = ( mMaxIconCount > (int)GetRowCount() ) ? (int)GetRowCount() : mMaxIconCount;
	for( int row = mTopRowIdx; row < (int)mTopRowIdx + rowCount; ++row )
	{
		sRowData* rData = GetRowData( row );
		if( !rData )
		{
			/// 해당 열 초기화
			int i = row - mTopRowIdx;
			if( i < 0 || i >= mMaxIconCount )
			{
				assert(0);
			}

			if( mIcon[i] && mIcon[i]->GetItemIndex() > 0 )
			{
				mIcon[i]->SetItemIndex(0);
				mIcon[i]->ChangeImage(0);
			}
			continue;
		}

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sData* data = rData->mColumes[col];
			if( !data )
				continue;

			/// 아이템 아이콘 세팅
			unsigned long itemIndex = (unsigned long)rData->mExtraData;
			cItemDefine* define = ITEMMAN->GetItemDefine( itemIndex );
			int i = row - mTopRowIdx;
			if( i < 0 || i >= mMaxIconCount )
			{
				assert(0);
				return;
			}

			if( define && mIcon[i]->GetItemIndex() != itemIndex )
			{
				mIcon[i]->SetItemIndex( itemIndex );
				mIcon[i]->ChangeImage( define->GetIconParam() );
			}

			textposX = mIcon[i]->GetAbsoluteRect().mRight + 4;
			textPosY = mIcon[i]->GetAbsoluteRect().mTop + 1;

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned long color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = COLOR_RED;

			/// 다음라인 위치 조정
			colY = 0;
			if( col > 0 )
			{
				colY = FONTAGENT->GetTextHeight( cFontAgent::eFont_System ) + 2;

				/// 최종 출력 좌표 계산 
				unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
				textposX = GetAbsoluteRect().mLeft + mSelectBarWidth - strWidth - 5;

				if( row % 2 != 0 )
					textposX += mSelectBarWidth;
			}

			pKeeper->AddFontItem( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), textposX, textPosY + colY, color, mOutLine );
		}
	}
}

void cDoubleItemIconListBox::OnMouseMove( const cUIPos& pos )
{
	cUINode::OnMouseMove( pos );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;

	/// 선택행 결정
	int mX = (int)(pos.mX - GetAbsoluteRect().mLeft);
	int scrollWidth = ( mpScrollBar ) ? mpScrollBar->GetAbsoluteRect().GetWidth() : 0;
	int width = (int)(GetAbsoluteRect().GetWidth() * 0.5f) - scrollWidth;

	int selectrow = row * 2;

	/// 오른쪽 행 선택
	if( mX > width )
		selectrow = (row*2) + 1;

	if( selectrow >= (int)GetRowCount() )
	{
		mHoveredRowIdx = -1;
		return;
	}

	/// 이미지 영역 체크에 관한 이벤트
	unsigned int curRow = mTopRowIdx + selectrow;
	if( curRow >= GetRowCount() )
		curRow = GetRowCount() - 1;

	if( mHoveredRowIdx != selectrow )
	{
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_ROW_HOVERED;
		event.mExtraData = GetExtraData( curRow );
		mpParent->HandleEvent( event );
	}

	/// 아이콘 hover
	if( selectrow < mMaxIconCount )
	{
		if( mIcon[selectrow]->ContainPoint( pos ) == true )
		{
			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mPos = pos;
			event.mExtraData = GetExtraData( curRow );
			event.mType = eUIEVENT_LISTBOX_IMAGE_HOVERED;
			mpParent->HandleEvent( event );

			mImageHoveredIdx = selectrow;
		}
		else
		{
			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mType = eUIEVENT_LISTBOX_IMAGE_LEFT;
			mpParent->HandleEvent( event );

			mImageHoveredIdx = -1;
		}
	}

	if( mEnableSelect )
		mHoveredRowIdx = selectrow;
}

void cDoubleItemIconListBox::OnMouseLeft( const cUIPos& pos )
{
	cListCtrl::OnMouseLeft( pos );

	mHoveredRowIdx = -1;
}

/// 주의사항 : 
void cDoubleItemIconListBox::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUINode::OnLButtonDoubleClick( pos );

	if( mEnableSelect == false )
		return;

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;

	/// 선택행 결정
	int mX = (int)(pos.mX - GetAbsoluteRect().mLeft);
	int scrollWidth = ( mpScrollBar ) ? mpScrollBar->GetAbsoluteRect().GetWidth() : 0;
	int width = (int)(GetAbsoluteRect().GetWidth() * 0.5f) - scrollWidth;

	unsigned int selectrow = row * 2;

	/// 오른쪽 행 선택
	if( mX > width )
		selectrow = (row*2) + 1;

	selectrow += mTopRowIdx;
	if( selectrow >= GetRowCount() )
		return;

	mSelectedRowIdx = selectrow;
	mHoveredRowIdx = selectrow - mTopRowIdx;

	/// 이벤트 보내기 
	cUIEvent event;
	event.mType = eUIEVENT_LISTBOX_SELECTED;
	event.mpCaller = this;
	event.mID = mID;
	mpParent->HandleEvent( event );

	/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
	UpdateSelectImage();
	CheckShowImage();
}

/// 주의사항 : 
void cDoubleItemIconListBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	if( mEnableSelect == false )
		return;

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;

	/// 선택행 결정
	int mX = (int)(pos.mX - GetAbsoluteRect().mLeft);
	int scrollWidth = ( mpScrollBar ) ? mpScrollBar->GetAbsoluteRect().GetWidth() : 0;
	int width = (int)(GetAbsoluteRect().GetWidth() * 0.5f) - scrollWidth;

	unsigned int selectrow = row * 2;

	/// 오른쪽 행 선택
	if( mX > width )
		selectrow = (row*2) + 1;

	selectrow += mTopRowIdx;
	if( selectrow >= GetRowCount() )
		return;

	if( mContinueEvent == true || mSelectedRowIdx != selectrow )
	{
		mSelectedRowIdx = selectrow;

		/// 이벤트 보내기 
		cUIEvent event;
		event.mType = eUIEVENT_LISTBOX_CHANGED;
		event.mpCaller = this;
		event.mID = mID;
		event.mPos = pos;
		mpParent->HandleEvent( event );

		/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
		UpdateSelectImage();
		CheckShowImage();
	}
}

void cDoubleItemIconListBox::OnRButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnRButtonDown( pos, ctrl, alt, shift );

	if( mEnableSelect == false )
		return;

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;

	/// 선택행 결정
	int mX = (int)(pos.mX - GetAbsoluteRect().mLeft);
	int scrollWidth = ( mpScrollBar ) ? mpScrollBar->GetAbsoluteRect().GetWidth() : 0;
	int width = (int)(GetAbsoluteRect().GetWidth() * 0.5f) - scrollWidth;

	unsigned int selectrow = row * 2;

	/// 오른쪽 행 선택
	if( mX > width )
		selectrow = (row*2) + 1;

	selectrow += mTopRowIdx;
	if( selectrow >= GetRowCount() )
		return;

	mSelectedRowIdx = selectrow;
	mHoveredRowIdx = selectrow - mTopRowIdx;

	/// 이벤트 보내기 
	cUIEvent event;
	event.mType = eUIEVENT_LISTBOX_SELECTEDRBUTTON;
	event.mpCaller = this;
	event.mID = mID;
	event.mShift = shift;
	mpParent->HandleEvent( event );

	/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
	UpdateSelectImage();
	CheckShowImage();
}

/// 주의사항 : 버튼의 상태 체크 
void cDoubleItemIconListBox::CheckButtonEvent( unsigned int id )
{
	if( !mpScrollBar )
	{
		/// 여기 들어온다면 잘못된 부분인다.
		assert(0);
		return;
	}
	/// 스크롤바가 그려지지 않는 상태면 리턴
	if( mpScrollBar->IsVisible() == false )
	{
		/// 여기 들어온다면 잘못된 부분인다.
		assert(0);
		return;
	}
	assert(GetRowCount() > mMaxRowInPage);

	switch( id )
	{
	case eSCROOLBAR_UP:
		if( mTopRowIdx > 0 )
		{
			mTopRowIdx-=2;
			UpdateSelectImage();
		}
		break;
	case eSCROOLBAR_DOWN:
		/// 남는 영역이 없게끔 스크롤
		if( GetRowCount() - mTopRowIdx > (mMaxRowInPage*2) )
		{
			mTopRowIdx+=2;
			UpdateSelectImage();
		}		
		break;
	default:
		return;
	}

	/// 스크롤바 버튼 위치 업데이트 
	UpdateScrollBar();
	CheckShowImage();
} 

///
void cDoubleItemIconListBox::OnMouseWheel( const cUIPos& /*pos*/, int wheel )
{
	if( !mpScrollBar || mpScrollBar->IsVisible() == false )
		return;

	assert(GetRowCount() > mMaxRowInPage);

	/// 스크롤바 위로 휠 적용 
	if( wheel > 0 )
	{
		if( mTopRowIdx > 0 )
		{
			mTopRowIdx-=2;
			UpdateSelectImage();
		}
	}
	/// 아래로 휠 적용 
	else if ( wheel < 0 )
	{
		/// 남는 영역이 없게끔 스크롤
		if( GetRowCount() - mTopRowIdx > (mMaxRowInPage*2) )
		{
			mTopRowIdx+=2;
			UpdateSelectImage();
		}		
	}
	else
	{
		assert( 0 && "error mouse wheel message" );
		return;
	}

	/// 스크롤바 업데이트 
	UpdateScrollBar();
	CheckShowImage();
}

void cDoubleItemIconListBox::OnScrollbarMoved( const cUIPos& )
{	
	if( mpScrollBar == 0)
		return;

	if( GetRowCount() <= mMaxRowInPage * 2 )
	{
		assert(0);
		return;
	}

	/// 바 버튼 위치를 얻어옴.
	unsigned int barbuttonPos = mpScrollBar->GetBarButtonPosY();
	double h = mpScrollBar->GetScrollMarginHeight() / (double)(GetRowCount() - (mMaxRowInPage*2));

	/// 현재 바 버튼 위치에 해당하는 아이템 인덱스 구하기
	double x = ((double)barbuttonPos / h);

	if( (int)x % 2 != 0 )
		x++;

	mTopRowIdx = (int)x;
	mHoveredRowIdx = -1;

	UpdateSelectImage();
	CheckShowImage();
}

void cDoubleItemIconListBox::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	SendEvent( mpParent, eUIEVENT_ICON_DRAGGED, caller, id, shift );
}

void cDoubleItemIconListBox::OnIconClicked( cUINode* , unsigned int )
{
	OnLButtonDown( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()), false, false, false );
}

void cDoubleItemIconListBox::OnIconDoubleClicked( cUINode* caller, unsigned int id )
{
	OnLButtonDoubleClick( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()) );
}

void cDoubleItemIconListBox::OnIconRightClicked( cUINode* caller, unsigned int id, bool shift )
{
	SendEvent( mpParent, eUIEVENT_ICON_RIGHTCLICKED, caller, id, shift );

 	if( mEnableSelect == false )
 		return;
 
 	unsigned int row = (caller->GetAbsoluteRect().mTop - GetAbsoluteRect().mTop) / mRowHeight;
 
 	/// 선택행 결정
 	int mX = (int)(caller->GetAbsoluteRect().mLeft - GetAbsoluteRect().mLeft);
 	int scrollWidth = ( mpScrollBar ) ? mpScrollBar->GetAbsoluteRect().GetWidth() : 0;
 	int width = (int)(GetAbsoluteRect().GetWidth() * 0.5f) - scrollWidth;
 
 	unsigned int selectrow = row * 2;
 
 	/// 오른쪽 행 선택
 	if( mX > width )
 		selectrow = (row*2) + 1;
 
 	selectrow += mTopRowIdx;
 	if( selectrow >= GetRowCount() )
 		return;
 
 	mSelectedRowIdx = selectrow;
// 	mHoveredRowIdx = selectrow - mTopRowIdx;
 
 	/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
 	UpdateSelectImage();
 	CheckShowImage();
}

///
void cDoubleItemIconListBox::UpdateScrollBar()
{
	if( !mpScrollBar )
		return;

	/// 양쪽으로 그리니까 재계산
	if( GetRowCount() > (mMaxRowInPage*2) )
	{
		mpScrollBar->Show();

		unsigned int scrollHeight = mpScrollBar->GetAbsoluteRect().GetHeight();
		unsigned int buttonHeight = mpScrollBar->GetScrollButtonHeight() * 2;
		if( scrollHeight > buttonHeight ) 
		{
			scrollHeight -= buttonHeight;
		}

		double hsize = scrollHeight - scrollHeight * (double)GetRowCount() / (double)mTotalMaxNum;

		/// 바 버튼의 최소사이즈랑 비교후 세팅 
		unsigned int minsize = mpScrollBar->GetMinimumHeight();
		if( hsize < minsize )
			hsize = minsize;

		/// 스크롤바 위치 계산..
		/// 한 아이템당 스크롤바가 움직이는 거리 구하기 
		double h = mpScrollBar->GetScrollMarginHeight() / (double)(GetRowCount()-(mMaxRowInPage*2));

		/// 현재 탑인덱스 * 한개영역 = 새로운 스크롤바 위치 
		double newposY = h * mTopRowIdx;
		double oldY = mpScrollBar->GetBarButtonPosY();

		/// 기존 위치랑 비교해서 스크롤바의 위치를 재조정한다. 
		int value = (int)(newposY-oldY);
		if( newposY > oldY )
		{
			mpScrollBar->MoveScrollDown( value );
		}
		else
		{
			mpScrollBar->MoveScrollUp( -value );
		}
	}
	else
		mpScrollBar->Hide();
}

/// 주의사항 : 선택된 이미지 영역 업데이트 
void cDoubleItemIconListBox::UpdateSelectImage()
{
	int selectRow	= mSelectedRowIdx - mTopRowIdx;
	if( selectRow < 0 || selectRow >= mMaxIconCount )
		return;

	cUIRect imageRc;
	imageRc.mLeft = ( mSelectedRowIdx % 2 == 0 ) ? GetAbsoluteRect().mLeft : GetAbsoluteRect().mLeft + mSelectBarWidth + 2;
	imageRc.mRight	= imageRc.mLeft + mSelectBarWidth;
	imageRc.mTop    = mIcon[selectRow]->GetAbsoluteRect().mTop - 4;
	imageRc.mBottom = imageRc.mTop + mRowHeight;

	/// 위치 세팅 
	mpSelectImage->SetScreenXY( (unsigned short)imageRc.mLeft, (unsigned short)imageRc.mTop );
	mpSelectImage->SetScreenWH( (unsigned short)imageRc.GetWidth(), (unsigned short)imageRc.GetHeight() );
}

void cDoubleItemIconListBox::CheckShowImage()
{
	if( mEnableSelect == false )
	{
		mShowImage = false;
	}
	else
	{
		/// 선택이미지 바가, 리스트박스를 넘어가지 않도록 한다.
		if( mSelectedRowIdx >= mTopRowIdx && mSelectedRowIdx < mTopRowIdx + (mMaxRowInPage*2) )
			mShowImage = true;
		else
			mShowImage = false;
	}
}

unsigned int cDoubleItemIconListBox::GetColumeWidth()
{
 	if( mMaxIconCount < 1 )
 	{
 		assert(0);
 		return 0;
 	}	
 
 	/// 선택이미지의 크기에서 아이콘 크기를 빼서 해당 colume 너비를 계산한다.
 	if( mIcon[0] == NULL )
 	{
 		assert(0);
 		return 0;
 	}
 
 	int colWidth = mSelectBarWidth - mIcon[0]->GetAbsoluteRect().GetWidth();
 
 	/// 텍스트 출력 위치와 아이콘 출력 위치를 감안하여 10정도의 차이를 둔다.
 	colWidth -= 10;
 
 	if( colWidth < 0 )
 	{
 		assert(0);
 		return 0;
 	}

	return colWidth;
}
