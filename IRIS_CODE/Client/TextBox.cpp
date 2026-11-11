#include "StdAfx.h"
#include "TextBox.h"
#include "Token.h"
#include "UISkinLexer.h"
#include "Parser.h"
#include "UIContainer.h"
#include "UIImage.h"
#include "UIFuncText.h"
#include "FontAgent.h"
#include "ScrollBar.h"

cTextBox::cTextBox( eUINodeType type )
: cTextCtrl( type )
, mTextPosX(0)
, mTextPosY(0)
, mNextPosY(0)
, mTopRowIdx(0)
, mMaxRowInPage(0)
, mRowHeight(0)
, mpScrollBar(0)
, mFixScrollH(0)
, mDrawTextBottom( true )
, mScrollBottom( false )
, mIncreseTopRowIdx( true )
, mChatMode( false )
{
	mFontType = cFontAgent::eFont_Chat;
	mScrollbarPosType = eRIGHT_SCROLLBAR;
	mTextColor = mDefaultColor;
}

cTextBox::~cTextBox()
{
	/// 스크롤바 소멸( 부모단에서 해줌 )
	mpScrollBar = 0;
}

///
void cTextBox::Clear()
{
	cTextCtrl::Clear();

	/// 모든 값 초기화 
	mTopRowIdx = 0;

	mIncreseTopRowIdx = true;

	/// 스크롤바 초기화 
	UpdateScrollBarButton();

	/// 스크롤바 위치 초기화 
	UpdateScrollBarPos();
}

/// 주의사항 :
bool cTextBox::OnCreate( cUINodeProperty* property )
{
	/// 추가적인 것은 이것으로 생성
	if( cUIWindow::OnCreate( property ) == false )
		return false;

	/// 리스트 한페이지에 최대 들어갈수 있는 갯수 세팅
	mMaxRowInPage = GetAbsoluteRect().GetHeight() / mRowHeight;

	mIncreseTopRowIdx = true;

	/// 스크롤바 생성
	if( mScrollSkin )
	{
		mpScrollBar = new cScrollBar;
		if( mpScrollBar->CreateBySkinName( mScrollSkin, this, eTEXTBOX_SCROLL ) == false )
			return false;

		/// 스크롤바 길이 세팅 
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() - mFixScrollH );

		/// 바 버튼 사이즈 세팅 
		UpdateScrollBarButton();

		///
		UpdateScrollBarPos();

		/// 스크롤바의 위치옵션을 보고 위치를 설정
		if( mScrollbarPosType == eRIGHT_SCROLLBAR )
		{
			mpScrollBar->SetPosX( GetRelativeRect().mRight - GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth());
		}
		else 
		{
			mpScrollBar->SetPosX( GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth());
		}

		mpScrollBar->Hide();
	}

	/// 너비 설정 
	if( mpScrollBar )
	{
		mMaxTextWidth = GetAbsoluteRect().GetWidth() - mpScrollBar->GetAbsoluteRect().GetWidth();
	}
	else
	{
		mMaxTextWidth = GetAbsoluteRect().GetWidth();
	}

	return true;
}

/// 주의사항 : 
bool cTextBox::SetSkin( const cUINodeSkin* pskin )
{
	if( pskin->IsKindof( eUINODE_TEXTBOX ) == false )
	{
		assert( 0 && "not list skin type" );
		return false;
	}

	if( cUINode::SetSkin( pskin ) == false )
	{
		return false;
	}

	/// 스크립트 의 속성 저장
	cTextBoxSkin* p = (cTextBoxSkin*)pskin;

	cUISkin* pUISkin = UIMAN->GetSkin();
	if( !pUISkin )
	{
		assert(0);
		return false;
	}

	/// 자식 스킨 네임 저장
	mScrollSkin = p->mScrollSkin;

	/// 스킨 네임 유효성 검사
	if( mScrollSkin && pUISkin->GetNodeSkin( p->mScrollSkin ) == 0 )
	{
		assert( 0 && "error skin name" );
		return false;
	}

	/// 리스트에 최대 들어갈수 있는 갯수 세팅
	mTotalMaxNum = p->mTotalMaxNum;

	/// 아이템 높이 세팅 
	mRowHeight = p->mRowHeight;
	if( mRowHeight < 0 )
	{
		assert(0);
		return false;
	}

	/// 스크롤바의 위치 세팅 옵션
	mScrollbarPosType = ( p->mScrollBarRightPos ) ? eRIGHT_SCROLLBAR : eLEFT_SCROLLBAR;
	return true;
}

///
void cTextBox::UpdateRect()
{
	cUIWindow::UpdateRect();
	UpdateText();
}

/// 주의사항 : 해상도 변경에 따른 재조정
void cTextBox::UpdateSkin()
{
	cUISkin* pskin = UIMAN->GetSkin();
	if( pskin == 0 )
	{
		assert(0);
	}

	/// 노드 스킨 검색                               
	cUINodeSkin* pnodeSkin = pskin->GetNodeSkin( mSkinName );
	if( pnodeSkin == 0 )
	{
		assert(0);
	}

	/// 루트의 바로 자식인 윈도우들은 기존좌표를 저장한채 해상도를 변경
	if( mpParent == 0 )
	{
		assert(0);
		return;
	}

	/// 기존 해상도 저장 
	mOldScreenWidth = GetScreenWidth();
	mOldScreenHeight = GetScreenHeight();

	/// 자식 스킨 적용
	cChildList::cIterator i = mChildList.Begin();
	cChildList::cIterator end = mChildList.End();
	for( ; i != end; ++i )
	{
		cUINode* n = (cUINode*)(*i);

		if( n )
			n->UpdateSkin();
	}


	/// 리스트 한페이지에 최대 들어갈수 있는 갯수 세팅
	mMaxRowInPage = GetAbsoluteRect().GetHeight() / mRowHeight;

	/// 스크롤바가 있으면 스크롤바도 재세팅
	if( mpScrollBar )
	{
        /// 스크롤바 길이 세팅 
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() - mFixScrollH );

		/// 바 버튼 사이즈 세팅 
		UpdateScrollBarButton();

		/// 탑인덱스 조절
		if( mScrollBottom && GetRowCount() >= mMaxRowInPage )
		{
			mTopRowIdx = GetRowCount() - mMaxRowInPage;
		}
		
		/// 스크롤바 y 위치 세팅 
		UpdateScrollBarPos();

		/// 스크롤바의 위치옵션을 보고 위치를 설정
		if( mScrollbarPosType == eRIGHT_SCROLLBAR )
		{
			mpScrollBar->SetPosX( GetRelativeRect().mRight - GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth());
		}
		else 
		{
			mpScrollBar->SetPosX( GetRelativeRect().mLeft - mpScrollBar->GetAbsoluteRect().GetWidth());
		}

		UpdateText();
	}
}

/// 주의사항 : 
bool cTextBox::HandleEvent( const cUIEvent& event )
{
	if( mEnabled == false || mVisible == false )
	{
		return mpParent->HandleEvent( event );
	}

	switch( event.mType )
	{
	case eUIEVENT_COMMAND:
	case eUIEVENT_BUTTON_DOWNED:
		{
			if( event.mID == eSCROOLBAR_UP || event.mID == eSCROOLBAR_DOWN )
				CheckButtonEvent( event.mID );
			else
				mpParent->HandleEvent( event );
		}
		break;
	case eUIEVENT_SCROLLBAR_MOVE:
		OnScrollbarMoved( event.mPos );
		break;
	case eUIEVENT_MOUSE_WHEEL:
		OnMouseWheel( event.mPos, event.mWheel );
		break;
	case eUIEVENT_LBUTTON_DOWN:
	case eUIEVENT_LBUTTON_UP:
		mpParent->HandleEvent( event );
		OnLButtonDown( event.mPos, false, false, false );
		break;
	default:
		return cUINode::HandleEvent( event );
	}
	return true;
}

/// 주의사항: 텍스트 좌표 업데이트
void cTextBox::UpdateText()
{
	/// 리스트 박스의 좌표 받아오기
	cUIRect rc = GetAbsoluteRect();

	mTextPosX = rc.mLeft + 2;
	mTextPosY = rc.mTop + 1;

	/// 텍스트박스의 텍스트를 뿌릴 위치 지정 
	if( mpScrollBar && mScrollbarPosType == eLEFT_SCROLLBAR )
	{
		/// 스크롤바를 텍스트박스 기준으로 왼쪽에 두려한다면 
		/// 스크롤바 넓이만큼을 더한 위치부터 출력한다 
		unsigned int width = mpScrollBar->GetAbsoluteRect().GetWidth();
		mTextPosX = rc.mLeft + width + 2;
	}		

	/// 페이지에 들어갈수 있는 줄수를 초과하면 텍스트창 처음 위치부터 하나씩 뿌린다.
	mNextPosY = mTextPosY;

	/// 텍스트 출력 위치 설정
	if( mDrawTextBottom && GetRowCount() <= mMaxRowInPage )
	{
		mNextPosY = rc.mBottom - (GetRowCount() * mRowHeight);
	}
}

/// 주의사항 : 
void cTextBox::OnRender( cUIFontItemKeeper* pKeeper )
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

	if( GetRowCount() > 0 )
	{
		unsigned int rowCount = ( mMaxRowInPage > GetRowCount() )? GetRowCount() : mMaxRowInPage;
 		for( unsigned int row = mTopRowIdx; row < mTopRowIdx + rowCount; ++row )
		{
			sRowTextData* rowData = GetRowData( row );
			if( !rowData )
				continue;

			int addX = 0;
			unsigned int colCount = rowData->mColumes.GetSize();

			/// 같은 열, 즉 컬럼들을 옆에 붙여가면서 출력한다. 
			for( unsigned int col = 0; col < colCount; ++col )
			{
				sTextData* colData = rowData->mColumes[col];
				if( colData )
				{
					int addY = ( row - mTopRowIdx ) * mRowHeight;

					pKeeper->AddFontItem( mFontType, (LPTSTR)(colData->mText).Cstr(), mTextPosX + addX, mNextPosY + addY, (unsigned long)(colData->mColor), mOutLine );
					addX += FONTAGENT->GetTextExtent( mFontType, (LPTSTR)(colData->mText).Cstr(), colData->mText.GetLength() );
				}
			}
		}
	}

	pKeeper->DrawAll();
}

/// 테스트용 - 라인 두개에 걸친 텍스트는 못찾는다.
void cTextBox::OnLButtonDown( const cUIPos& pos, bool /*ctrl*/, bool /*alt*/, bool )
{
	/// 내용이 없으면 리턴
	if( GetRowCount() <= 0 )
		return;
	
	/// 텍스트박스안에서의 마우스 좌표 
	unsigned int mouseX = pos.mX - mTextPosX;
	unsigned int mouseY = pos.mY - mTextPosY;

	/// 라인 구하기
	int row = 0;
	unsigned int lineNum = mouseY / mRowHeight;
	if( GetRowCount() >= mMaxRowInPage )
	{
		/// 꽉차있으면
		row = mTopRowIdx + lineNum;
	}
	else
	{
		/// 빈 줄이 있으면 계산
		row = lineNum - ( mMaxRowInPage - GetRowCount() );
	}

	if( row < 0 )
		return;

	/// 해당라인에 x 위치만 비교해서 텍스트 데이터 찾기
	sRowTextData* rowData = GetRowData( row );
	if( !rowData )
		return;

	/// 같은 열, 즉 컬럼들을 검사
	unsigned int addX = 0;
	unsigned int colWidth = 0;
	unsigned int colCount = rowData->mColumes.GetSize();
	
	bool search = false;
	for( unsigned int col = 0; col < colCount; ++col )
	{
		sTextData* colData = rowData->mColumes[col];
		if( !colData )
		{
			assert(0);
			continue;
		}

		colWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_Chat, (LPTSTR)(colData->mText).Cstr(), colData->mText.GetSize() );

		/// 너비안에 포함하는 데이터 찾기
		if( mouseX >= addX && mouseX < addX + colWidth )
		{
			if( colData->mUseInven )
			{
				sInvenData* inven = (sInvenData*)colData;
				if( inven )
				{
					UIMAN->ShowChatItemTip( pos, inven->mInventory );
					search = true;
					break;
				}
				else
				{
					assert(0);
				}
			}
		}

		/// 다음 너비 구하기
		addX += colWidth;
	}

	if( search == false )
		UIMAN->HideTip();
}

///
void cTextBox::OnMouseWheel( const cUIPos&, int wheel )
{
	if( !mpScrollBar || mpScrollBar->IsVisible() == false )
		return;
	
	assert( GetRowCount() >= mMaxRowInPage);

	/// 스크롤바 위로 휠 적용 
	if( wheel > 0 )
	{
		if( mTopRowIdx > 0 )
		{
			mTopRowIdx--;
		}
	}
	/// 아래로 휠 적용 
	else if ( wheel < 0 )
	{
		/// 남는 영역이 없게끔 스크롤
		if( GetRowCount() - mTopRowIdx > mMaxRowInPage )
		{
			mTopRowIdx++;
		}		
	}
	else
	{
		assert( 0 && "error mouse wheel message" );
		return;
	}

	/// 스크롤바 업데이트 
	UpdateScrollBarPos();
}

/// 주의사항 : 스크롤바가 움직이면, 텍스트 박스 아이템 출력 순위 갱신
void cTextBox::OnScrollbarMoved( const cUIPos& )
{
	if( mpScrollBar )
	{
		assert( GetRowCount() > mMaxRowInPage );

		/// 바 버튼 위치를 얻어옴.
		unsigned int barbuttonPos = mpScrollBar->GetBarButtonPosY();
		double h = mpScrollBar->GetScrollMarginHeight() / (double)( GetRowCount() - mMaxRowInPage );

		/// 현재 바 버튼 위치에 해당하는 아이템 인덱스 구하기
		double x = ((double)barbuttonPos / h);

		mTopRowIdx = (int)x;
	}	
}

/// 스크롤바의 위치와, 높이를 조정한다
void cTextBox::SetScrollbar( unsigned int addh )
{
	/// 보정값 세팅 
	mFixScrollH = addh;

	if( mpScrollBar )
	{
		mpScrollBar->SetPosY( mFixScrollH );

		/// 스크롤바 길이 세팅 
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() - mFixScrollH );

		//// 바 버튼 사이즈 세팅 
		UpdateScrollBarButton();

		/// 스크롤 "바" y 위치 세팅 
		UpdateScrollBarPos();
	}	
}

/// 동일 라인 찾아서 텍스트 갱신
void cTextBox::UpdateRowText( int line, LPCTSTR text )
{
	if( GetRowCount() <= 0 )
		return;

	unsigned int rowCount = GetRowCount();
	for( unsigned int row = 0; row < rowCount; ++row )
	{
		sRowTextData* rowData = GetRowData( row );
		if( !rowData )
			continue;

		unsigned int colCount = rowData->mColumes.GetSize();
		for( unsigned int col = 0; col < colCount; ++col )
		{
			sTextData* colData = rowData->mColumes[col];
			if( colData && line == (int)row )
			{
				colData->mText = text;
				break;
			}
		}
	}
}


/// 주의사항 : 빈 공백 라인 추가 ( 엔터 입력 효과 )
void cTextBox::AddEmptyRow( unsigned int line )
{
	int nOldTextCount = GetRowCount();

	cTextCtrl::AddEmptyRow( line );

	if( GetRowCount() > mMaxRowInPage )
	{
		/// 스크롤바 버튼 사이즈 업데이트 
		UpdateScrollBarButton();

		/// 맨 밑을 기준으로 하고싶으면,
		if( mScrollBottom )
		{
			/// 맨마지막위치로 계산
			mTopRowIdx = ( GetRowCount() - mMaxRowInPage );

			/// 스크롤 "바" y 위치 세팅
			UpdateScrollBarPos();
		}
		else
		{
			if( mChatMode == true )
			{
				if( mIncreseTopRowIdx == true )
				{
					int nAddCnt = GetRowCount() - nOldTextCount;
					if( mTopRowIdx == 0 && nAddCnt >= 2)
					{
						mTopRowIdx += 1;
					}
					else
					{
						mTopRowIdx += nAddCnt;
					}	

					UpdateScrollBarPos();

					if( GetRowCount() >= (mTotalMaxNum - 1)  )
						mIncreseTopRowIdx = false;
				}			
			}
			else
			{
				UpdateScrollBarPos();
			}
		}		
	}

	UpdateText();
}

/// 주의사항 : 새로운 열에 텍스트 입력
int cTextBox::AddNewRow( LPCTSTR text, unsigned long textcolor, sInventory* inventory )
{
	int nOldTextCount = GetRowCount();
	/// 삽입 
 	cTextCtrl::AddNewRow( text, textcolor, inventory );
	///
	if( GetRowCount() > mMaxRowInPage )
	{
		/// 스크롤바 버튼 사이즈 업데이트 
		UpdateScrollBarButton();

		/// 맨 밑을 기준으로 하고싶으면,
		if( mScrollBottom )
		{
			/// 맨마지막위치로 계산
			mTopRowIdx = GetRowCount() - mMaxRowInPage;

			UpdateScrollBarPos();
		}
		else
		{
			if( mChatMode == true )
			{
				if( mIncreseTopRowIdx == true )
				{
					int nAddCnt = GetRowCount() - nOldTextCount;
					if( mTopRowIdx == 0 && nAddCnt >= 2)
					{
						mTopRowIdx += 1;
					}
					else
					{
						mTopRowIdx += nAddCnt;
					}					

					UpdateScrollBarPos();

					if( GetRowCount() >= (mTotalMaxNum - 1)  )
						mIncreseTopRowIdx = false;
				}			
			}
			else
			{
				UpdateScrollBarPos();
			}
		}		
	}

	UpdateText();

	return ( GetRowCount() > 1 ) ? GetRowCount() - 1 : 0 ;
}

void cTextBox::AddPasteRow( LPCTSTR text, unsigned long textcolor, sInventory* inventory )
{
	int nOldTextCount = GetRowCount();

	cTextCtrl::AddPasteRow( text, textcolor, inventory );

	if( GetRowCount() > mMaxRowInPage )
	{
		/// 스크롤바 버튼 사이즈 업데이트 
		UpdateScrollBarButton();

		/// 맨 밑을 기준으로 하고싶으면,
		if( mScrollBottom )
		{
			/// 맨마지막위치로 계산
			mTopRowIdx = ( GetRowCount() - mMaxRowInPage );

			/// 스크롤 "바" y 위치 세팅
			UpdateScrollBarPos();
		}
		else
		{
			if( mChatMode == true )
			{
				if( mIncreseTopRowIdx == true )
				{
					int nAddCnt = GetRowCount() - nOldTextCount;
					if( mTopRowIdx == 0 && nAddCnt >= 2)
					{
						mTopRowIdx += 1;
					}
					else
					{
						mTopRowIdx += nAddCnt;
					}	

					UpdateScrollBarPos();

					if( GetRowCount() >= (mTotalMaxNum - 1)  )
						mIncreseTopRowIdx = false;
				}			
			}
			else
			{
				UpdateScrollBarPos();
			}
		}		
	}

	UpdateText();
}

///
bool cTextBox::AddFucnText( cUIFuncTextNode* node, cUIFuncTextParam* param )
{
	if( !node )
	{
		assert(0);
		return false;
	}

	bool newLine = true;
	mTextColor = mDefaultColor;

	for( ; node; node = node->GetNext() )
	{
		switch( node->mType )
		{
		case eTEXTNODE_STRING:
			{
				if( newLine == true )
					AddNewRow( node->mpString, mTextColor );
				else
					AddPasteRow( node->mpString, mTextColor );

				newLine = false;
			}
			break;
		case eTEXTNODE_COLOR:
			{
				mTextColor = node->mColor;
			}
			break;
		case eTEXTNODE_VARNAME: 
			{
				if( param )
				{
					if( newLine == true )
						AddNewRow( param->mName.Cstr(), mTextColor );
					else
						AddPasteRow( param->mName.Cstr(), mTextColor );
				}
				else
					assert(0);

				newLine = false;
			}
			break;
		case eTEXTNODE_NEWLINE:
			{
				AddEmptyRow( 1 );
			}
			break;
		default:
			{
				assert( 0 && "invalid npc text node type" );
			}
			return false;
		}
	}
	return true;
}

/// 주의사항 : 텍스트박스의 사이즈 변화에 따른 모든 정보 갱신 
void cTextBox::UpdateTextBox( bool add )
{
	/// 리스트 한페이지에 최대 들어갈수 있는 갯수 세팅
	mMaxRowInPage = GetAbsoluteRect().GetHeight() / mRowHeight;

	/// 텍스트박스 확장
	if( add == true )
	{
		/// 출력 갯수 구하기 
		if( mMaxRowInPage <= GetRowCount() )
		{
			/// tapIdx 가 한계범위를 넘어가지 않도록 한다 
			if( mTopRowIdx > 0 )
			{
				mTopRowIdx--;
			}
		}
	}
	/// 텍스트박스 축소
	else 
	{
		/// 출력 갯수 구하기 
		if( mMaxRowInPage < GetRowCount() )
		{
			/// tapIdx 가 한계범위를 넘어가지 않도록 한다 
			if( GetRowCount() - mTopRowIdx > mMaxRowInPage )
			{
				mTopRowIdx++;
			}	
		}
	}

	if( mpScrollBar )
	{
		/// 스크롤바 길이 세팅 
		mpScrollBar->SetHeight( GetAbsoluteRect().GetHeight() - mFixScrollH );

		/// 바 버튼 사이즈 세팅 
		UpdateScrollBarButton();

		/// 스크롤 "바" y 위치 세팅
		UpdateScrollBarPos();
	}

	UpdateText();
}

/// 주의사항 : 버튼의 상태 체크 
void cTextBox::CheckButtonEvent( unsigned int id )
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
	
	assert( GetRowCount() >= mMaxRowInPage );

	switch( id )
	{
	case eSCROOLBAR_UP:
		if( mTopRowIdx > 0 )
		{
			mTopRowIdx--;
		}
		break;

	case eSCROOLBAR_DOWN:
		/// 남는 GetRowCount() 없게끔 스크롤
		if( GetRowCount() - mTopRowIdx > mMaxRowInPage )
		{
			mTopRowIdx++;
		}		
		break;
	}

	/// 스크롤 "바" y 위치 세팅
	UpdateScrollBarPos();
}

/// 스크롤바 위치 업데이트 
void cTextBox::UpdateScrollBarPos()
{
	if( !mpScrollBar )
		return;

	/// 스크롤바가 그려지지 않는 상태면 리턴
	if( mpScrollBar->IsVisible() == false )
		return;

	/// 한 아이템당 스크롤바가 움직이는 거리 구하기 
	double h = mpScrollBar->GetScrollMarginHeight() / (double)( GetRowCount() - mMaxRowInPage );

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

/// 주의사항 : 스크롤바의 사이즈 계산
void cTextBox::UpdateScrollBarButton()
{
	if( !mpScrollBar )
		return;

	/// 텍스트박스가 너무작으면 스크롤바의 버튼을 그리지 않는다.
	unsigned int scrollHeight = mpScrollBar->GetAbsoluteRect().GetHeight();
	unsigned int buttonHeight = mpScrollBar->GetScrollButtonHeight() * 2;
	bool		 isHideBarButton = false;

	if( scrollHeight > buttonHeight ) 
	{
		scrollHeight -= buttonHeight;
	}
	else 
		isHideBarButton = true;

	/// 한영역에 들어갈 수 있는 갯수보다 많은 아이템수가 있으면,
	if( GetRowCount() > mMaxRowInPage )
	{
		/// 스크롤바 전체를 그린다
		mpScrollBar->Show();
		
		/// 스크롤바의 크기를 비교하여 바 버튼을 그리지 않는다.
		if( scrollHeight < mpScrollBar->GetMinimumHeight() || isHideBarButton )
			mpScrollBar->SetVisibleBarButton( false );
		else
			mpScrollBar->SetVisibleBarButton( true );
	}
	else
	{
		/// 스크롤바의 바 버튼을 그리지 않는다.
		mpScrollBar->Hide();
	}
}


//////////////////////////////////////////////////////////////////////////////
cTextBoxSkin::cTextBoxSkin( eUINodeType type )
: cUINodeSkin( type )
, mRowHeight( 0 )
, mTotalMaxNum( 0 )
, mScrollBarRightPos(true)
{
}

cTextBoxSkin::~cTextBoxSkin()
{

}

/// 주의사항 : 스크립트 파일을 읽어들여서 값을 세팅한다
bool cTextBoxSkin::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cToken token;
	cLexer* lexer = parser.GetLexer();

	/// 그다음토큰을 계속 검사
	while( lexer->GetNextToken( &token ) )
	{
		if( token == "}" )
		{
			break;
		}

		switch( token.mType )
		{
		case eTOKEN_ROWHEIGHT:
			{
				mRowHeight = parser.ParseInt();
			}
			break;
		case eTOKEN_SCROLLBAR_RIGHTPOS:
			{
				lexer->GetNextToken( &token );

				if( token.mType == eTOKEN_TRUE )
				{
					mScrollBarRightPos = true;
				}
				else if( token.mType == eTOKEN_FALSE )
				{
					mScrollBarRightPos = false;
				}
				else
				{
					assert( 0 && "invalid token" );
					return false;
				}
			}
			break;
		case eTOKEN_SCROLLSKIN:
			{
				mScrollSkin = parser.ParseString();
			}
			break;
		case eTOKEN_MAXITEMNUM:
			{
				mTotalMaxNum = parser.ParseInt();
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
