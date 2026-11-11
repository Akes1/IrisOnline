#include "StdAfx.h"
#include "ListBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"

#include "UIEvent.h"
#include "UIImage.h"
#include "PlaneObject.h"
#include "UIContainer.h"
#include "FontAgent.h"

#include "ScrollBar.h"

cListBox::cListBox( eUINodeType type )
: cListCtrl( type )
, mpSelectImage(0)
, mpScrollBar(0)
, mpSkin(0)
, mSelectBarWidth(0)
, mContinueEvent( false )
, mShowImage( true )
, mPrintPos(0, 0)
, mPrintSize(0, 0)
{
	mMaxRowInPage = 0;
	mTotalMaxNum = 0;

	mSelectedRowIdx = 0;
	mHoveredRowIdx = -1;
	mImageHoveredIdx = -1;

	mTopRowIdx = 0;
	mRowHeight = 0;

	mHoverTextColor = COLOR_WHITE;

	mOutLine = true;

	mFontType = cFontAgent::eFont_UI;
}

cListBox::~cListBox()
{
	mpScrollBar = 0;

	SAFE_DELETE( mpSelectImage );
}

void cListBox::Clear()
{
	mSelectedRowIdx = 0;
	mHoveredRowIdx = -1;

	mImageHoveredIdx = -1;

	mTopRowIdx = 0;

	UpdateScrollBar();

	/// 선택바
	if( mpSelectImage )
		UpdateSelectImage();

	cListCtrl::Clear();
}

/// 주의사항 : 생성되자마자 해야할 일 처리
bool cListBox::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;

	/// 스킨으로 생성
	if( mScrollSkin )
	{
		mpScrollBar = new cScrollBar;
		if( mpScrollBar->CreateBySkinName( mScrollSkin, this, eLIST_SCROLL ) == false )
		{
			assert( 0 && "failed to create scroll bar" );
			return false;
		}
	}

	if( mpScrollBar )
	{
		/// 스크롤바 높이 세팅
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() );

		/// 스크롤바 x 위치 세팅 
		mpScrollBar->SetPosX( GetRelativeRect().mRight -  GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth() );

		/// 스크롤바 갱신
		UpdateScrollBar();
	}
	return true;
}


/// 주의사항 :
bool cListBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_LISTBOX ) == false )
	{
		assert( 0 && "not list skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	/// 스크립트로부터 텍스쳐 배경이미지의 속성을 받는다
	cListBoxSkin* p = (cListBoxSkin*)pskin;

	/// 스킨 저장 
	mpSkin = p;
	mScrollSkin = p->mScrollSkin;

	/// 리스트컨트롤에 아이템 높이 세팅 
	mRowHeight = p->mRowHeight;
	assert( mRowHeight );

	/// 리스트에 최대 들어갈수 있는 갯수 세팅
	mTotalMaxNum = p->mTotalMaxNum;
	mSelectBarWidth = p->mSelectBarWidth;

	mPrintPos = p->mPrintPos;
	mPrintSize = p->mPrintSize;

	/// 리스트 한페이지에 최대 들어갈수 있는 갯수 세팅
	mMaxRowInPage = ( mPrintSize.mHeight > 0 ) ? (mPrintSize.mHeight / mRowHeight) : (pskin->mSkinInfo->mHeight / mRowHeight);
	if( mMaxRowInPage == 0 )
		mMaxRowInPage = p->mTotalMaxNum;

	assert( mMaxRowInPage );

	/// 아이템 선택 이미지 텍스쳐 설정
	if( p->mpSelectBarTexture )
	{
		/// 텍스처 영역 설정
		unsigned short tx = (unsigned short)p->mSelectBarSkin->mTexX;
		unsigned short ty = (unsigned short)p->mSelectBarSkin->mTexY;
		unsigned short tw = (unsigned short)p->mSelectBarSkin->mTexWidth;
		unsigned short th = (unsigned short)p->mSelectBarSkin->mTexHeight;

		if( mpSelectImage == 0 )
		{
			mpSelectImage = new cPlaneObject;

			if( mpSelectImage->Create( p->mpSelectBarTexture, (short)GetAbsoluteRect().mLeft, (short)GetAbsoluteRect().mTop, tw, th,
				tx, ty, tx + tw, ty + th ) == false )
			{
				assert( 0 && "failed to create select image");
				return false;
			}
		}
		else
		{
			mpSelectImage->SetTextureRect( tx, ty, tx + tw, ty + th );
		}
	}
	return true;
}

///
int cListBox::AddRow( LPCTSTR str, unsigned long color, void* pExtraData )
{
	/// 수용갯수를 넘어가면 리턴 
	if( GetRowCount() >= mTotalMaxNum )
		return -1;

	sData* data = new sData;
	data->mText = str;
	data->mTextColor = color;

	unsigned int ret = AddRowData( data, pExtraData );

	/// 스크롤바 갱신
	UpdateScrollBar();

	if( mpSelectImage )
		UpdateSelectImage();
	return ret;
}

///
bool cListBox::AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color )
{
	/// 수용갯수를 넘어가면 컬럼수 검사 
	if( GetRowCount() >= mTotalMaxNum )
	{
		unsigned int count = GetColumnCount( rowIdx - 1 );
		unsigned int countNow = GetColumnCount( rowIdx );

		if( countNow > count )
			return 0;
	}

	sData* data = new sData;
	data->mText = str;
	data->mTextColor = color;

	return AddColumeData( rowIdx, data );
}

///
void cListBox::SetExtraData( unsigned int rowIdx, void* pExtraData )
{
	if( rowIdx < GetRowCount() )
	{
		sRowData* p = (sRowData*)mRows[rowIdx];
		p->mExtraData = pExtraData;
	}
}

///
LPCTSTR cListBox::GetText( unsigned int rowIdx, unsigned int colIdx )
{
	sData* data = GetColumeData( rowIdx, colIdx );
	if( data )
		return data->mText.Cstr();
	else
		return _T("");
}

sData* cListBox::GetColumeData( unsigned int rowIdx, unsigned int columeIdx )
{
	if( mRows.GetSize() <= 0 )
		return 0;

	if( rowIdx > mRows.GetSize()-1 )
	{
		assert(0);
		return 0;
	}

	sRowData* pRow = (sRowData*)mRows[rowIdx];
	return pRow->mColumes[columeIdx];
}

/// 주의사항 : 해상도 변경에 따른 재조정
void cListBox::UpdateSkin()
{
	/// 스킨 좌표 업데이트 
	cUIWindow::UpdateSkin();

	/// 선택 이미지 초기화 
	if( mpSelectImage )
		UpdateSelectImage();

	/// 스크롤바가 있으면 스크롤바도 재세팅
	if( mpScrollBar )
	{
		/// 스크롤바 높이 세팅 
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() );

		/// 스크롤바 x 위치 세팅 
		mpScrollBar->SetPosX( GetRelativeRect().mRight - GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth() );

		/// 스크롤바 갱신
		UpdateScrollBar();
	}

	mHoveredRowIdx = -1;
}

/// 주의사항 :
void cListBox::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpSelectImage )
	{
		/// 선택바 이미지 영역 업데이트
		UpdateSelectImage();
		CheckShowImage();
	}	
}

/// 주의사항 :
void cListBox::OnRender( cUIFontItemKeeper* pKeeper )
{
	if( mVisible == false )
		return;

	/// 이미지 렌더링
	if( mpImage )
		mpImage->Draw();

	/// 자식 렌더링
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();

	for( ; i != end; ++i )
	{
		if( ((cUINode*)(*i))->IsVisible() == true )
		{
			((cUINode*)(*i))->OnRender(pKeeper);
		}
	}

	if( mpSelectImage && GetRowCount() && IsEnabled() )
	{
		/// 선택이미지 바가, 리스트박스를 넘어가지 않도록 한다.
		if( mShowImage == true )
			mpSelectImage->Draw();
	}

	/// 리스트 박스의 아이템을 그려줌.
	RenderData( pKeeper );

	pKeeper->DrawAll();
}

/// 주의사항 : 마우스가 움직일때마다 선택된것을 바꾼다
void cListBox::OnMouseMove( const cUIPos& pos )
{
	cUINode::OnMouseMove( pos );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;
	if( row > mMaxRowInPage - 1 )
	{
		mHoveredRowIdx = -1;
		return;
	}

	if( row >= GetRowCount() )
	{
		mHoveredRowIdx = -1;

		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_ROW_LEFT;
		mpParent->HandleEvent( event );
		return;
	}

	/// 이미지 영역 체크에 관한 이벤트
	unsigned int curRow = mTopRowIdx + row;
	if( curRow >= GetRowCount() )
		curRow = GetRowCount() - 1;

	if( mHoveredRowIdx != (int)row )
	{
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mPos = pos;
		event.mType = eUIEVENT_LISTBOX_ROW_HOVERED;
		event.mExtraData = GetExtraData( curRow );
		mpParent->HandleEvent( event );
	}

	mHoveredRowIdx = row;
}

void cListBox::OnMouseLeft( const cUIPos& pos )
{
	cListCtrl::OnMouseLeft( pos );

	mHoveredRowIdx = -1;

	cUIEvent event;
	event.mpCaller = this;
	event.mID = mID;
	event.mType = eUIEVENT_LISTBOX_ROW_LEFT;
	mpParent->HandleEvent( event );
}

/// 주의사항 : 
void cListBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight + mTopRowIdx;
	if( row >= GetRowCount() )
		return;

	mHoveredRowIdx = row - mTopRowIdx;

	/// 새로운 줄이 선택되면 이벤트 보냄
	if( mContinueEvent == true || mSelectedRowIdx != row )
	{
		mSelectedRowIdx = row;

		/// 이벤트 보내기 
		cUIEvent event;
		event.mType = eUIEVENT_LISTBOX_CHANGED;
		event.mpCaller = this;
		event.mID = mID;
 		mpParent->HandleEvent( event );

		/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
		if( mpSelectImage )
		{
			UpdateSelectImage();
			CheckShowImage();
		}
	}
}

void cListBox::OnRButtonDown(const cUIPos& pos, bool, bool, bool shift )
{
	cUINode::OnLButtonDoubleClick( pos );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight + mTopRowIdx;
	if( row >= GetRowCount() )
		return;

	mSelectedRowIdx = row;
	mHoveredRowIdx = row - mTopRowIdx;

	/// 이벤트 보내기 
	cUIEvent event;
	event.mType = eUIEVENT_LISTBOX_SELECTEDRBUTTON;
	event.mpCaller = this;
	event.mID = mID;
	event.mShift = shift;
	mpParent->HandleEvent( event );

	/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
	if( mpSelectImage )
	{
		UpdateSelectImage();
		CheckShowImage();
	}
}

/// 주의사항 : 
void cListBox::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUINode::OnLButtonDoubleClick( pos );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight + mTopRowIdx;
	if( row >= GetRowCount() )
		return;

	mSelectedRowIdx = row;
	mHoveredRowIdx = row - mTopRowIdx;

	/// 이벤트 보내기 
	cUIEvent event;
	event.mType = eUIEVENT_LISTBOX_SELECTED;
	event.mpCaller = this;
	event.mID = mID;
	mpParent->HandleEvent( event );

	/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
	if( mpSelectImage )
	{
		UpdateSelectImage();
		CheckShowImage();
	}
}

/// 주의사항 : 스크롤바가 움직이면, 리스트 박스 아이템 출력 순위 갱신
void cListBox::OnScrollbarMoved( const cUIPos& )
{	
	if( mpScrollBar == 0)
		return;

	assert( GetRowCount() > mMaxRowInPage );

	/// 바 버튼 위치를 얻어옴.
	unsigned int barbuttonPos = mpScrollBar->GetBarButtonPosY();
	double h = mpScrollBar->GetScrollMarginHeight() / (double)(GetRowCount()-mMaxRowInPage);

	/// 현재 바 버튼 위치에 해당하는 아이템 인덱스 구하기
	double x = ((double)barbuttonPos / h);

	mTopRowIdx = (int)x;
	mHoveredRowIdx = -1;

	if( mpSelectImage )
	{
		UpdateSelectImage();
		CheckShowImage();
	}
}

///
void cListBox::OnMouseWheel( const cUIPos& /*pos*/, int wheel )
{
	if( !mpScrollBar || mpScrollBar->IsVisible() == false )
	{
		return;
	}
	assert(GetRowCount() > mMaxRowInPage);

	/// 스크롤바 위로 휠 적용 
	if( wheel > 0 )
	{
		if( mTopRowIdx > 0 )
		{
			mTopRowIdx--;

			if( mpSelectImage )
				UpdateSelectImage();
		}
	}
	/// 아래로 휠 적용 
	else if ( wheel < 0 )
	{
		/// 남는 영역이 없게끔 스크롤
		if( GetRowCount() - mTopRowIdx > mMaxRowInPage )
		{
			mTopRowIdx++;

			if( mpSelectImage )
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

/// 주의사항 : 버튼의 상태 체크 
void cListBox::CheckButtonEvent( unsigned int id )
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
			mTopRowIdx--;

			if( mpSelectImage )
				UpdateSelectImage();
		}
		break;
	case eSCROOLBAR_DOWN:
		/// 남는 영역이 없게끔 스크롤
		if( GetRowCount() - mTopRowIdx > mMaxRowInPage )
		{
			mTopRowIdx++;

			if( mpSelectImage )
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
bool cListBox::ChangeSelectRow( unsigned int rowIdx, bool changeTop )
{
	if( rowIdx >= GetRowCount() )
		return false;
	if( mContinueEvent == false && mSelectedRowIdx == rowIdx )
		return true;

	mSelectedRowIdx = rowIdx;

	if( changeTop )
	{
		/// 페이지가 넘어 간다면 선택 객체를 최상위로 올린다.
		if( rowIdx >= mMaxRowInPage )
		{
			/// 여백이 안남게끔 조절 
			if( GetRowCount() - rowIdx >= mMaxRowInPage )
				mTopRowIdx = mSelectedRowIdx;
			else
				mTopRowIdx = GetRowCount() - mMaxRowInPage;

			/// 스크롤바를 갱신한다.
			UpdateScrollBar();
		}
	}

	/// 마우스 오버된 인덱스도 갱신 
	mHoveredRowIdx = -1;

	/// 선택 이미지 갱신
	if( mpSelectImage )
	{
		UpdateSelectImage();
		CheckShowImage();
	}
	return true;
}

/// 주의사항 : 리스트의 출력시작할 탑인덱스 조절 
bool cListBox::ChangeTopIndex( unsigned int topIdx, bool changeSel )
{
	if( topIdx >= GetRowCount() )
		return false;
	if( mTopRowIdx == topIdx )
		return true;

	mTopRowIdx = topIdx;

	if( changeSel == true )
		mSelectedRowIdx = mTopRowIdx;
	else
		UpdateScrollBar();

	/// 마우스 오버된 인덱스도 갱신 
	mHoveredRowIdx = -1;

	/// 선택 이미지 갱신
	if( mpSelectImage )
	{
		UpdateSelectImage();
		CheckShowImage();
	}
	return true;
}

/// 주의사항 : 리스트 아이템 그려주기
void cListBox::RenderData( cUIFontItemKeeper* pKeeper )
{
	/// 전체 리스트를 얻어옴 
	if( GetRowCount() <= 0 )
		return;

	/// 스킨 받아오기 
	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	if( !pListSkin )
	{
		assert(0);
		return;
	}

	/// 리스트 박스의 좌표 받아오기
	int textposX = 0;
	int textPosY = GetAbsoluteRect().mTop + mPrintPos.mY;

	unsigned int rowCount = ( mMaxRowInPage > GetRowCount() )? GetRowCount() : mMaxRowInPage;
	for( unsigned int row = mTopRowIdx; row < mTopRowIdx + rowCount; ++row )
	{
		sRowData* rData = GetRowData( row );
		if( !rData )
			continue;

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sData* data = rData->mColumes[col];

			/// 각 컬럼의 param data 가져오기 ( 출력위치 등 )
			sListCtrlParam* pParam = pListSkin->GetCtrlParam( col );

			unsigned int tX = 0;
			unsigned int tW = 0;
			eText_Align align = eALIGN_LEFT;

			if( pParam )
			{
				tX = pParam->mColumnX;
				tW = pParam->mColumnW;
				align = pParam->mAlign;
			}

			/// 최종 출력 좌표 계산 
			unsigned int strWidth = FONTAGENT->GetTextExtent( mFontType, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = mHoverTextColor;

			int addY = 0;
			if( (long)mRowHeight > FONTAGENT->GetTextHeight(mFontType) )
				addY = (int)( (mRowHeight-FONTAGENT->GetTextHeight(mFontType))*0.5f );

			pKeeper->AddFontItem( mFontType, (LPTSTR)data->mText.Cstr(), textposX, textPosY + addY, color, mOutLine );
		}

		textPosY += mRowHeight;
	}

}

/// 주의사항 : 정렬에 따른 출력 좌표값 계산 
unsigned int cListBox::UpdateTextPos( eText_Align align, unsigned int strWidth, unsigned int x, unsigned int w )
{
	unsigned int drawX = GetAbsoluteRect().mLeft + x + mPrintPos.mX;

	if( align == eALIGN_RIGHT )
	{
		drawX += (int)(w - strWidth);
	}
	else if( align == eALIGN_CENTER )
	{
		drawX += (int)(w - strWidth) / 2;
	}
	return drawX;
}

///
void cListBox::UpdateScrollBar()
{
	if( !mpScrollBar )
		return;

	if( GetRowCount() > mMaxRowInPage )
	{
		/// 스크롤바의 바 버튼을 그린다 
		mpScrollBar->Show();

		//unsigned int scrollHeight = mpScrollBar->GetScrollBarHeight();
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
		{
			hsize = minsize;
		}

		/// 스크롤바 위치 계산..
		/// 한 아이템당 스크롤바가 움직이는 거리 구하기 
		double h = mpScrollBar->GetScrollMarginHeight() / (double)(GetRowCount()-mMaxRowInPage);

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
	{
		mpScrollBar->Hide();
	}
}

/// 주의사항 : 선택된 이미지 영역 업데이트 
void cListBox::UpdateSelectImage()
{
	int scrollWidth = 0;
	if( mpScrollBar && mpScrollBar->IsVisible() )
	{
		scrollWidth = mpScrollBar->GetAbsoluteRect().GetWidth();
	}

	cUIRect imageRc;
	imageRc.mLeft   = GetAbsoluteRect().mLeft;

	/// 스킨상에서 정해진 선택바 폭이 있으면 적용.
	if( mSelectBarWidth == 0 )
		imageRc.mRight  = imageRc.mLeft + GetAbsoluteRect().GetWidth() - scrollWidth;
	else
		imageRc.mRight  = imageRc.mLeft + mSelectBarWidth;

	imageRc.mTop    = GetAbsoluteRect().mTop + ( mRowHeight * ( mSelectedRowIdx - mTopRowIdx) ) + mPrintPos.mY;
	imageRc.mBottom = imageRc.mTop + mRowHeight;

	/// 위치 세팅 
	if( mpSelectImage )
	{
		mpSelectImage->SetScreenXY( (unsigned short)imageRc.mLeft, (unsigned short)imageRc.mTop );
		mpSelectImage->SetScreenWH( (unsigned short)imageRc.GetWidth(), (unsigned short)imageRc.GetHeight() );
	}
}

void cListBox::CheckShowImage()
{
	/// 선택이미지 바가, 리스트박스를 넘어가지 않도록 한다.
	if( mSelectedRowIdx >= mTopRowIdx && mSelectedRowIdx < mTopRowIdx + mMaxRowInPage )
		mShowImage = true;
	else
		mShowImage = false;
}

/// 주의사항 :
bool cListBox::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
		return mpParent->HandleEvent( event );

	switch( event.mType )
	{
	case eUIEVENT_COMMAND:
	case eUIEVENT_BUTTON_DOWNED:
		CheckButtonEvent( event.mID );
		break;
	case eUIEVENT_SCROLLBAR_MOVE:
		OnScrollbarMoved( event.mPos );
		break;
	default:
		cUIWindow::HandleEvent( event );
		break;
	}
	return true;
}

void cListBox::ChangeTextColor( unsigned int rowIdx, unsigned long color )
{
	sRowData* rData = (sRowData*)GetRowData( rowIdx );
	if( rData )
	{
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sData* data = rData->mColumes[col];
			if( data )
				data->mTextColor = color;
		}
	}
}

unsigned int cListBox::GetColumnMaxWidth( unsigned int colIndex )
{
	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	if( !pListSkin )
	{
		assert(0);
		return 0;
	}

	sListCtrlParam* pParam = pListSkin->GetCtrlParam( colIndex );
	return ( pParam ) ? pParam->mColumnW : 0;
}

///////////////////////////////////////////////////////////////////////////////
cListBoxSkin::cListBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mTotalMaxNum( 0 )
, mRowHeight( 0 )
, mSelectBarSkin(0)
, mpImgParam(0)
, mpSelectBarTexture(0)
, mSelectBarWidth(0)
, mPrintPos(0, 0)
, mPrintSize(0, 0)
{
	mSelectBarSkin = new sSkinInfo;
	mpImgParam = new sListCtrlParam;

	mRowHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_UI);
}

cListBoxSkin::~cListBoxSkin()
{
	///
	for( unsigned int i = 0; i < mCtrlParamArr.GetSize(); ++i)
	{
		delete (sListCtrlParam*)mCtrlParamArr[i];
	}
	mCtrlParamArr.Clear();

	SAFE_DELETE( mpImgParam );
	SAFE_DELETE( mSelectBarSkin );
}

/// 주의사항 : 
bool cListBoxSkin::Load( cParser& parser )
{
	for( unsigned int i = 0; i < mCtrlParamArr.GetSize(); ++i)
	{
		delete (sListCtrlParam*)mCtrlParamArr[i];
	}
	mCtrlParamArr.Clear();

	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			/// 블록 탈출 ^^
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_PRINTPOS:
			{
				mPrintPos.mX = parser.ParseInt();
				mPrintPos.mY = parser.ParseInt();
			}
			break;
		case eTOKEN_PRINTSIZE:
			{
				mPrintSize.mWidth = parser.ParseInt();
				mPrintSize.mHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_IMGCOLX:
			{
				mpImgParam->mColumnX = parser.ParseInt();
				mpImgParam->mColumnW = parser.ParseInt();
				if( mpImgParam->mColumnW <= 0 )
				{
					assert(0);
					return 0;
				}
			}
			break;
		case eTOKEN_COLUMNX:
			{
				/// 좌표 
				unsigned columnX = parser.ParseInt();

				/// 넓이 
				unsigned columnW = parser.ParseInt();
				if( columnW <= 0 )
				{
					assert(0);
					return 0;
				}

				///
				lexer->GetNextToken( &token );

				/// 정렬 
				eText_Align align = eALIGN_LEFT;

				if( token.mType == eTOKEN_CENTER )
				{
					align = eALIGN_CENTER;
				}
				else if( token.mType == eTOKEN_RIGHT )
				{
					align = eALIGN_RIGHT;
				}

				/// 데이터 삽입
				sListCtrlParam* pCtrlParam = new sListCtrlParam;
				pCtrlParam->mColumnX = columnX;
				pCtrlParam->mColumnW = columnW;
				pCtrlParam->mAlign = align;
				mCtrlParamArr.PushBack( pCtrlParam );
			}
			break;
		case eTOKEN_ROWHEIGHT:
			{
				mRowHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_MAXITEMNUM:
			{
				mTotalMaxNum = parser.ParseInt();
			}
			break;
			/// ui image 관련 
		case eTOKEN_CHILD_IMAGEINDEX:
			{
				int i = parser.ParseInt();
				mpSelectBarTexture = UIMAN->GetSkin()->GetTexture( i );
				if( mpSelectBarTexture == 0 )
				{
					assert( 0 && "failed to load texture" );
					return false;
				}
			}
			break;
		case eTOKEN_CHILD_SIZE:
			{
				mSelectBarSkin->mWidth = mSelectBarSkin->mTexWidth = (unsigned short)parser.ParseInt();
				mSelectBarSkin->mHeight = mSelectBarSkin->mTexHeight = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_CHILD_TEXPOS:
			{
				mSelectBarSkin->mTexX = (unsigned short)parser.ParseInt();
				mSelectBarSkin->mTexY = (unsigned short)parser.ParseInt();
			}
			break;
		case eTOKEN_SELECTWIDTH:
			{
				mSelectBarWidth = parser.ParseInt();
			}
			break;
		case eTOKEN_SCROLLSKIN:
			{
				mScrollSkin = parser.ParseString();
			}
			break;
		default:
			if( cUINodeSkin::ParseLine( parser, token ) == false )
			{
				return false;
			}
			break;
		}
	}
	return true;
}

///
sListCtrlParam*	cListBoxSkin::GetCtrlParam( unsigned int index )
{
	if( mCtrlParamArr.GetSize() == 0 )
		return 0;

	if( index > mCtrlParamArr.GetSize() - 1 )
	{
		assert(0);
		return 0;
	}

	sListCtrlParam* pParam = (sListCtrlParam*)mCtrlParamArr[index];
	assert( pParam );

	return pParam;
}
