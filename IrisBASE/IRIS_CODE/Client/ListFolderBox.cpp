#include "StdAfx.h"
#include "ListFolderBox.h"
#include "UIManager.h"
#include "UISkinLexer.h"
#include "Token.h"
#include "Parser.h"
#include "UIEvent.h"
#include "ScrollBar.h"
#include "FontAgent.h"
#include "UIImage.h"
#include "PlaneObject.h"
#include "GameResourceManager.h"

cListFolderBox::cListFolderBox( eUINodeType type )
:cListCtrl( type )
, mpScrollBar(0)
, mpSelectImage(0)
, mTopColumnIdx(0)
, mSelectedColumnIdx(0)
, mRowHeight(0)
, mMaxRowInPage(0)
, mTotalMaxNum(0)
, mHighCheckPos(0, 0)
, mHighNoCheckPos(0, 0)
, mLowCheckPos(0, 0)
, mLowNoCheckPos(0, 0)
, mSelectedRow(0)
, mSelectedRowColumn(0)
, mColumnCount(0)
, mRenderRow(0)
, mRenderStartColumn(0)
, mLowGap(0)
, mShowNumber(true)
, mShowSelect(true)
{
}

cListFolderBox::~cListFolderBox()
{
	mpScrollBar = 0;
	SAFE_DELETE( mpSelectImage );
}

void cListFolderBox::Clear()
{
	mSelectedColumnIdx = 0;
	mSelectedRow = 0;
	mSelectedRowColumn = 0;

	mTopColumnIdx = 0;
	mColumnCount = 0;

	UpdateScrollBar();
	UpdateSelectImage();

	cListCtrl::Clear();
}

void cListFolderBox::InitSelectRow()
{
	mSelectedColumnIdx = 0;
	mSelectedRow = 0;
	mSelectedRowColumn = 0;
	mTopColumnIdx = 0;

	UpdateScrollBar();
	UpdateSelectImage();
}

bool cListFolderBox::OnCreate( cUINodeProperty* pproperty )
{
	if( cUIWindow::OnCreate( pproperty ) == false )
		return false;
	
	/// 스크롤바 생성
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
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() );
		mpScrollBar->SetPosX( GetRelativeRect().mRight -  GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth() );
		UpdateScrollBar();
	}
	return true;
}

bool cListFolderBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_LISTFOLDERBOX ) == false )
	{
		assert( 0 && "not list skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
		return false;

	/// 스크립트로부터 텍스쳐 배경이미지의 속성을 받는다
	cListFolderBoxSkin* p = (cListFolderBoxSkin*)pskin;

	mScrollSkin = p->mScrollSkin;

	/// 리스트컨트롤에 아이템 높이 세팅 
	mRowHeight = p->mRowHeight;

	/// 리스트에 최대 들어갈수 있는 갯수 세팅
	mTotalMaxNum = p->mTotalMaxNum;

	/// 리스트 한페이지에 최대 들어갈수 있는 갯수 세팅
	mMaxRowInPage = pskin->mSkinInfo->mHeight / mRowHeight;
	assert( mMaxRowInPage );

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

/// 주의사항 :
bool cListFolderBox::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
		return mpParent->HandleEvent( event );

	switch( event.mType )
	{
	case eUIEVENT_COMMAND:
		CheckButtonEvent( event.mID );
	case eUIEVENT_BUTTON_DOWNED:
		//CheckButtonEvent( event.mID );
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

void cListFolderBox::UpdateRect()
{
	cUIWindow::UpdateRect();

	if( mpSelectImage )
		UpdateSelectImage();
}

void cListFolderBox::UpdateSkin()
{
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
}

void cListFolderBox::OnProcess(unsigned long deltaTime, unsigned long accumTime )
{
	cUIWindow::OnProcess( deltaTime, accumTime );

	if( mVisible == false )
		return;

	/// mTopColumnIdx로 해당 row와 row의 컬럼을 찾는다
	if( mColumnCount > 0 )
	{
		mRenderRow = 0;
		mRenderStartColumn = 0;
		unsigned int totalColumn = 0;
		for( unsigned int i = 0; i < GetRowCount(); ++i )
		{
			sRowData* p = GetRowData(i);
			if( p )
			{
				/// 해당 row의 컬럼카운트
				mRenderRow = i;
				mRenderStartColumn = mTopColumnIdx - totalColumn;

				/// 트리가 접혀져있는지 검사.
				sFolderData* data = (sFolderData*)p->mColumes[0];
				if( data->mCheck == true )
					totalColumn += p->mColumes.GetSize();
				else
					totalColumn += 1;

				if( totalColumn > mTopColumnIdx )
					break;

				mRenderStartColumn = 0;
			}
		}
	}
}

void cListFolderBox::OnRender( cUIFontItemKeeper* pKeeper )
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

	/// 전체 리스트를 얻어옴 
	if( mColumnCount > 0 )
	{
		/// 선택이미지 바가, 리스트박스를 넘어가지 않도록 한다.
		if( mShowSelect == true )
		{
			if( mSelectedColumnIdx >= mTopColumnIdx && 
				mSelectedColumnIdx < mTopColumnIdx + mMaxRowInPage )
			{
				mpSelectImage->Draw();
			}
		}

		int textposX = GetAbsoluteRect().mLeft + 1;
		int textPosY = GetAbsoluteRect().mTop + 1;
		unsigned int addX = 0;
		//unsigned int addY = 0;

		/// 출력
		unsigned int maxLine = ( mMaxRowInPage > mColumnCount )? mColumnCount : mMaxRowInPage;
		unsigned int line = 0;

		for( unsigned int i=mRenderRow; i<GetRowCount(); i++ )
		{
			if( line >= maxLine )
				break;

			sRowData* rData = GetRowData( i );
			if( rData == 0 )
				continue;

			unsigned int colCount = rData->mColumes.GetSize();
			for( unsigned int col = mRenderStartColumn; col < colCount; ++col )
			{
				sFolderData* data = (sFolderData*)rData->mColumes[col];
				if( data == 0 )
					break;

				addX = 0;

				cUIImage* image = data->mCheckImg;
				if( image )
				{
					/// 이미지 텍스쳐 좌표 세팅
					cUIPos texPos = mLowCheckPos;
					if( col == 0 && data->mCheck == true )
						texPos = mHighCheckPos;
					else if( col == 0 && data->mCheck == false )
						texPos = mHighNoCheckPos;
					else if( data->mCheck == false )
						texPos = mLowNoCheckPos;

					image->SetTextureRect( cUIRect( texPos.mX, texPos.mY,texPos.mX + image->GetWidth(), texPos.mY + image->GetHeight() ) );
					image->SetPos( GetAbsoluteRect().mLeft + IMAGE_GAP, textPosY );
					image->Draw();

					addX += (image->GetWidth() + IMAGE_GAP + IMAGE_GAP);
				}

				/// 상위열이면
				if( col == 0 )
				{
					/// 하위열 갯수 보여주기
					if( mShowNumber == true )
					{
						TCHAR temp[256] = {0,};
						::_stprintf_s( temp, GAMERESOURCEMAN->GetGameText(470), (LPTSTR)data->mText.Cstr(), colCount-1 );
						pKeeper->AddFontItem( cFontAgent::eFont_UI, temp, textposX + addX, textPosY, data->mTextColor );
					}
					else
					{
						pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), textposX + addX, textPosY, data->mTextColor, mOutLine );
					}
				}
				/// 하위열이면
				else
				{
					// 상위열 출력부와 맞춘다
					if( addX == 0 )
					{
						addX = IMAGE_GAP * 2;
					}

					pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), textposX + addX + mLowGap, textPosY, data->mTextColor, mOutLine );
				}

				textPosY += mRowHeight;
				line++;

				if( line >= maxLine )
					break;

				/// 접혀있으면 밑에것은 출력안함
				if( col == 0 && data->mCheck == false )
					break;
			}

			mRenderStartColumn = 0;
		}
	}

	pKeeper->DrawAll();
}


/// 주의사항 : 
void cListFolderBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
{
	cUINode::OnLButtonDown( pos, ctrl, alt, shift );

	unsigned int column = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight + mTopColumnIdx;
	if( column >= mColumnCount )
		return;

	sRowData* p = 0;
	unsigned int row = 0;
	unsigned int rowColumn = 0;
	unsigned int totalColumn = 0;
	for( unsigned int i = 0; i < GetRowCount(); ++i )
	{
		p = GetRowData( i );
		if( p )
		{			
			/// 해당 row의 컬럼카운트
			row = i;
			rowColumn = totalColumn;

			/// 트리가 접혀져있는지 검사.
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data->mCheck == true )
				totalColumn += p->mColumes.GetSize();
			else
				totalColumn += 1;

			if( totalColumn > column )
				break;
		}
	}

	mSelectedRow = row;
	mSelectedRowColumn = column - rowColumn;

	/// 선택된 라인이 상위열이면,[체크] 갱신
	if( rowColumn == column )
	{
		sRowData* p = GetRowData( mSelectedRow );
		if( p )
		{
			mSelectedColumnIdx = column;

			sFolderData* data = (sFolderData*)p->mColumes[0];
			unsigned int colCount = p->mColumes.GetSize();

			/// 리스트에 이미지가 있으면 이미지에 따른 이벤트 체크 
			if( data->mCheckImg && colCount > 1 )
			{
				int x1 = GetAbsoluteRect().mLeft;
				int x2 = GetAbsoluteRect().mLeft + GetAbsoluteRect().GetWidth();

				if( pos.mX > x1 && pos.mX < x2 )
				{
					data->mCheck = !data->mCheck;

					/// 컬럼 총 갯수 갱신
					UpdateColumnCount();

					/// 한페이지에 충분히 들어가면 갱신
					if( mColumnCount < mMaxRowInPage )
					{
						mTopColumnIdx = 0;
					}
					else if( mColumnCount - rowColumn <= mMaxRowInPage  )
					{
						mTopColumnIdx = mColumnCount - mMaxRowInPage;
					}

					UpdateScrollBar();
				}
			}

			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mType = eUIEVENT_LISTBOX_CHANGED;
			mpParent->HandleEvent( event );
		}
	}
	else
	{
		sRowData* data = GetRowData( mSelectedRow );
		if( data )
		{
			sFolderData* colData = (sFolderData*)data->mColumes[mSelectedRowColumn];
			if( colData )
			{
				/// 리스트에 이미지가 있으면 이미지에 따른 이벤트 체크 
				if( colData->mCheckImg )
				{
					int x1 = GetAbsoluteRect().mLeft;
					int x2 = x1 + colData->mCheckImg->GetWidth();

					if( pos.mX > x1 && pos.mX < x2 )
					{
						/// 이미지 관련 이벤트 발생
						cUIEvent event;
						event.mpCaller = this;
						event.mID = mID;
						event.mType = eUIEVENT_IMAGE_CLICKED;
						mpParent->HandleEvent( event );
					}		
				}	
			}
		}

		/// 하위열이면 이벤트 쏘기
		if( mSelectedColumnIdx != column )
		{
			mSelectedColumnIdx = column;

			/// 이벤트 보내기 
			cUIEvent event;
			event.mType = eUIEVENT_LISTBOX_CHANGED;
			event.mpCaller = this;
			event.mID = mID;
			mpParent->HandleEvent( event );
		}
	}

	/// 선택부 갱신
	UpdateSelectImage();
}

void cListFolderBox::OnLButtonDoubleClick( const cUIPos& pos )
{
	cListCtrl::OnLButtonDoubleClick( pos );

	OnLButtonDown( pos, false, false, false );
}


void cListFolderBox::OnScrollbarMoved( const cUIPos& )
{	
	if( mpScrollBar == 0)
		return;

	assert( mColumnCount > mMaxRowInPage );

	/// 바 버튼 위치를 얻어옴.
	unsigned int barbuttonPos = mpScrollBar->GetBarButtonPosY();
	double h = mpScrollBar->GetScrollMarginHeight() / (double)(mColumnCount-mMaxRowInPage);

	/// 현재 바 버튼 위치에 해당하는 아이템 인덱스 구하기
	double x = ((double)barbuttonPos / h);

	mTopColumnIdx = (int)x;

	UpdateSelectImage();
}

///
void cListFolderBox::OnMouseWheel( const cUIPos& /*pos*/, int wheel )
{
	if( !mpScrollBar || mpScrollBar->IsVisible() == false )
		return;

	assert( mColumnCount > mMaxRowInPage );

	/// 스크롤바 위로 휠 적용 
	if( wheel > 0 )
	{
		if( mTopColumnIdx > 0 )
		{
			mTopColumnIdx--;
			UpdateSelectImage();
		}
	}
	/// 아래로 휠 적용 
	else if ( wheel < 0 )
	{
		/// 남는 영역이 없게끔 스크롤
		if( mColumnCount - mTopColumnIdx > mMaxRowInPage )
		{
			mTopColumnIdx++;
			UpdateSelectImage();
		}		
	}

	/// 스크롤바 업데이트 
	UpdateScrollBar();
}


/// 주의사항 : 버튼의 상태 체크 
void cListFolderBox::CheckButtonEvent( unsigned int id )
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
	assert(mColumnCount > mMaxRowInPage);

	switch( id )
	{
	case eSCROOLBAR_UP:
		if( mTopColumnIdx > 0 )
		{
			mTopColumnIdx--;
			UpdateSelectImage();
		}
		break;
	case eSCROOLBAR_DOWN:
		/// 남는 영역이 없게끔 스크롤
		if( mColumnCount - mTopColumnIdx > mMaxRowInPage )
		{
			mTopColumnIdx++;
			UpdateSelectImage();
		}		
		break;
	}

	/// 스크롤바 버튼 위치 업데이트 
	UpdateScrollBar();
} 

/// 상위열 추가
int cListFolderBox::AddHighData( long key, LPCTSTR str, unsigned long color, void* pExtraData, cUIImage* pImage, bool check )
{
	/// 수용갯수를 넘어가면 리턴 
	if( mColumnCount >= mTotalMaxNum )
		return -1;

	/// 이미 데이터가 있는지 검사
	for( unsigned int i = 0; i < mRows.GetSize(); ++i )
	{
		sRowData* p = (sRowData*)mRows[i];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data && data->key == key )
				return -1;
		}
	}
	
	/// 데이터 삽입
	sFolderData* data = new sFolderData;
	data->key = key;
	data->mText = str;
	data->mTextColor = color;
	data->mCheck = check;
	data->mCheckImg = pImage;
	data->mExtraData = pExtraData;
	
	int ret = cListCtrl::AddRowData( data, pExtraData );

	UpdateColumnCount();
	UpdateScrollBar();
	UpdateSelectImage();
	return ret;
}

/// 하위열 추가
bool cListFolderBox::AddLowData( long key, LPCTSTR str, unsigned long color, void* pExtraData, cUIImage* pImage, bool check )
{
	/// 해당 상위열 가져오기 
	int row = -1;
	for( unsigned int i = 0; i < mRows.GetSize(); ++i )
	{
		sRowData* p = (sRowData*)mRows[i];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data && data->key == key )
			{
				row = i;
				break;
			}
		}
	}

	if( row == -1 )
	{
		assert(0);
		return false;
	}

	sFolderData* data = new sFolderData;
	data->mText = str;
	data->mTextColor = color;
	data->mCheck = check;
	data->mCheckImg = pImage;
	data->mExtraData = pExtraData;

	bool ret = cListCtrl::AddColumeData( row, data );

	UpdateColumnCount();
	UpdateScrollBar();
	return ret;
}

sFolderData* cListFolderBox::GetFolderData( unsigned int row, unsigned int column )
{
	sRowData* p = GetRowData( row );
	if( p )
	{
		if( column >= p->mColumes.GetSize() )
		{
			assert(0);
			return 0;
		}

		return (sFolderData*)p->mColumes[column];
	}
	return 0;
}

sFolderData* cListFolderBox::GetFolderDataByExtra( long key, void* extraData )
{
	int rowSize = mRows.GetSize();
	for( int ri = 0; ri < rowSize; ++ri )
	{
		sRowData* p = (sRowData*)mRows[ri];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data && data->key == key )
			{
				/// 동일 데이터 검색
				for( unsigned int ci = 0; ci < p->mColumes.GetSize(); ++ci )
				{
					sFolderData* data = (sFolderData*)p->mColumes[ci];
					if( ci > 0 && data && data->mExtraData == extraData )
					{
						return data;
					}
				}
				break;
			}
		}
	}
	return 0;
}

// 접혔으면 표시 안됨
void cListFolderBox::ChangeSelectColumn( unsigned int selectRow, bool sendEvent )
{
	if( selectRow == -1 )
	{
		mSelectedRow = 0;
		mSelectedRowColumn = 0;
		mSelectedColumnIdx = 0;

		mShowSelect = false;
		return;
	}

	if( selectRow >= mColumnCount )
	{
		mSelectedRow = 0;
		mSelectedRowColumn = 0;
		mSelectedColumnIdx = 0;

		mShowSelect = false;
		return;
	}

	sRowData* p = 0;
	unsigned int row = 0;
	unsigned int rowColumn = 0;
	unsigned int totalColumn = 0;
	for( unsigned int i = 0; i < GetRowCount(); ++i )
	{
		p = GetRowData( i );
		if( p )
		{			
			/// 해당 row의 컬럼카운트
			row = i;
			rowColumn = totalColumn;

			/// 트리가 접혀져있는지 검사.
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data->mCheck == true )
				totalColumn += p->mColumes.GetSize();
			else
				totalColumn += 1;

			if( totalColumn > selectRow )
				break;
		}
	}
	
	mSelectedRow = row;
	mSelectedRowColumn = selectRow - rowColumn;
	mSelectedColumnIdx = rowColumn + mSelectedRowColumn;

	if( mSelectedColumnIdx >= mMaxRowInPage )
	{
		/// 여백이 안남게끔 조절 
		if( mColumnCount - mSelectedColumnIdx >= mMaxRowInPage )
			mTopColumnIdx = mSelectedColumnIdx;
		else
			mTopColumnIdx = mColumnCount - mMaxRowInPage;

		/// 스크롤바를 갱신한다.
		UpdateScrollBar();
	}

	// 이벤트 발생
	if( sendEvent )
	{
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_CHANGED;
		mpParent->HandleEvent( event );
	}

	/// 선택부 갱신
	UpdateSelectImage();
}

/// 주의사항 : 선택된 이미지 영역 업데이트 
void cListFolderBox::UpdateSelectImage()
{
	mShowSelect = true;

	int scrollWidth = 0;
	if( mpScrollBar && mpScrollBar->IsVisible() )
		scrollWidth = mpScrollBar->GetAbsoluteRect().GetWidth();

	cUIRect imageRc;
	imageRc.mLeft   = GetAbsoluteRect().mLeft;
	imageRc.mRight  = imageRc.mLeft + GetAbsoluteRect().GetWidth() - scrollWidth;
	imageRc.mTop    = GetAbsoluteRect().mTop + ( mRowHeight * ( mSelectedColumnIdx - mTopColumnIdx) );
	imageRc.mBottom = imageRc.mTop + mRowHeight;

	/// 위치 세팅 
	mpSelectImage->SetScreenXY( (unsigned short)imageRc.mLeft, (unsigned short)imageRc.mTop );
	mpSelectImage->SetScreenWH( (unsigned short)imageRc.GetWidth(), (unsigned short)imageRc.GetHeight() );
}

///
void cListFolderBox::UpdateScrollBar()
{
	if( !mpScrollBar )
		return;

	if( mColumnCount > mMaxRowInPage )
	{
		/// 스크롤바의 바 버튼을 그린다 
		mpScrollBar->Show();

		unsigned int scrollHeight = mpScrollBar->GetAbsoluteRect().GetHeight();
		unsigned int buttonHeight = mpScrollBar->GetScrollButtonHeight() * 2;
		if( scrollHeight > buttonHeight ) 
		{
			scrollHeight -= buttonHeight;
		}

		double hsize = scrollHeight - scrollHeight * (double)mColumnCount / (double)mTotalMaxNum;

		/// 바 버튼의 최소사이즈랑 비교후 세팅 
		unsigned int minsize = mpScrollBar->GetMinimumHeight();
		if( hsize < minsize )
		{
			hsize = minsize;
		}

		/// 스크롤바 위치 계산..
		/// 한 아이템당 스크롤바가 움직이는 거리 구하기 
		double h = mpScrollBar->GetScrollMarginHeight() / (double)(mColumnCount-mMaxRowInPage);

		/// 현재 탑인덱스 * 한개영역 = 새로운 스크롤바 위치 
		double newposY = h * mTopColumnIdx;
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

void cListFolderBox::UpdateColumnCount()
{
	mColumnCount = 0;

	/// 총 컬럼수 세기
	for( unsigned int i = 0; i < mRows.GetSize(); ++i )
	{
		sRowData* p = (sRowData*)mRows[i];
		if( p )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( !data )
			{
				assert(0);
				break;
			}

			if( data->mCheck == true )
				mColumnCount += p->mColumes.GetSize();
			else
				mColumnCount += 1;
		}
	}
}

///
void* cListFolderBox::GetColumnData( unsigned int row, unsigned int column )
{
	if( mColumnCount == 0 )
		return 0;

	if( mRows.GetSize() <= row )
		return 0;

	sRowData* data = (sRowData*)mRows[row];
	if( !data )
		return 0;

	if( data->mColumes.GetSize() <= column )
		return 0;

	sFolderData* colData = (sFolderData*)data->mColumes[column];
	return ( colData ) ? colData->mExtraData : 0;
}

/// 접혀져 있는 카테고리 키 값 가져오기
void cListFolderBox::GetClosedCategory( tHashSet<unsigned int>& keySet )
{
	int rowSize = mRows.GetSize();
	for( int ri = 0; ri < rowSize; ++ri )
	{
		sRowData* p = (sRowData*)mRows[ri];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data && data->mCheck == false )
			{
				keySet.Insert( data->key );
			}
		}
	}
}

/// 접혀져 있는 카테고리 키 값 가져오기
void cListFolderBox::SetClosedCategory( long key )
{
	int rowSize = mRows.GetSize();
	for( int ri = 0; ri < rowSize; ++ri )
	{
		sRowData* p = (sRowData*)mRows[ri];
		if( p && p->mColumes.GetSize() > 0 )
		{
			sFolderData* data = (sFolderData*)p->mColumes[0];
			if( data && data->key == key )
			{
				data->mCheck = false;

				/// 컬럼 총 갯수 갱신
				UpdateColumnCount();
		
				UpdateScrollBar();
			}
		}
	}
}


//////////////////////////////////////////////////////////
cListFolderBoxSkin::cListFolderBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mSelectBarSkin(0)
, mpSelectBarTexture(0)
, mSelectBarWidth(0)
{
	mSelectBarSkin = new sSkinInfo;
	mRowHeight = FONTAGENT->GetTextHeight(cFontAgent::eFont_UI);
}

cListFolderBoxSkin::~cListFolderBoxSkin()
{
	SAFE_DELETE(mSelectBarSkin);
}

/// 주의사항 : 
bool cListFolderBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

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
