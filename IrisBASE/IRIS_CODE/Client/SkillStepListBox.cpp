#include "StdAfx.h"
#include "SkillStepListBox.h"
#include "SkillManager.h"
#include "SkillIcon.h"
#include "FontAgent.h"
#include "UIManager.h"
#include "InputSystem.h"
#include "TipWindow.h"
#include "ScrollBar.h"
#include "GameUIManager.h"
#include "SkillWindow.h"
#include "SkillSheetWindow.h"


// 스킬 단계별 리스트 박스 정렬 함수
bool SkillCompare( const void* arg1, const void* arg2 )
{
	sRowData* lhs = (sRowData*)arg1;
	sRowData* rhs = (sRowData*)arg2;

	sSkillInfoEx* plhsExtraData = (sSkillInfoEx*)lhs->mExtraData;
	sSkillInfoEx* prhsExtraData = (sSkillInfoEx*)rhs->mExtraData;

	if( ((sPlayerSkillBaseInfo*)(plhsExtraData->mpSkillInfo))->mSkillIdx		> 
		((sPlayerSkillBaseInfo*)(prhsExtraData->mpSkillInfo))->mSkillIdx		)
	{
		return false;
	}
	else if(	((sPlayerSkillBaseInfo*)(plhsExtraData->mpSkillInfo))->mSkillIdx	== 
		((sPlayerSkillBaseInfo*)(prhsExtraData->mpSkillInfo))->mSkillIdx	)
	{
		if( plhsExtraData->mSkillStep > prhsExtraData->mSkillStep )
		{
			return false;
		}
		else
			return true;
	}
	else
	{
		return true;
	}
}


cSkillStepListBox::cSkillStepListBox( eUINodeType type )
: cListBox( type )
, mSplitSize(200)
, mCurrentSplitIndex(0)
{
	mSplitInfo.PushBack(0);
}

cSkillStepListBox::~cSkillStepListBox()
{
	Clear();

	mSkillIconArr.Clear();
	mSplitInfo.Clear();
}

// 리스트 박스 전체 영역 삭제
void cSkillStepListBox::Clear()
{
	for( unsigned char i = 0; i < mSkillIconArr.GetSize(); i++ )
	{
		cSkillIcon* pIcon = (cSkillIcon*)mSkillIconArr[i];
		if( pIcon )
		{
			pIcon->SetUsedFlag( false );
			pIcon->ChangeImage( 0 );
			pIcon->SetAniParam( 0 );
			pIcon->StopDelay();
		}
	}

	// 각 분할 영역에 원소 제거
	for( unsigned char iSplit = 0; iSplit < mSplitInfo.GetSize(); iSplit++ )
	{
		ClearSplitRange( iSplit );
	}

	mRows.Clear();

	mCurrentSplitIndex	= 0;
	mTopRowIdx			= 0;
	mSelectedRowIdx		= 0;
	mShowImage			= true;
}

unsigned int cSkillStepListBox::GetRowCount()
{
	if( mCurrentSplitIndex >= mSplitInfo.GetSize() )
	{
		assert(0);
		return 0;
	}

	if( mSplitInfo.IsEmpty() )
		return 0;

	unsigned int i = mSplitInfo[ mCurrentSplitIndex ];
	return i;
}


// 리스트의 특정 구간만 Clear
void cSkillStepListBox::ClearSplitRange( unsigned char nSplitIndex )
{
	// 스킬 아이콘 초기화
	for( unsigned char i = 0; i < mSkillIconArr.GetSize(); i++ )
	{
		cSkillIcon* pIcon = (cSkillIcon*)mSkillIconArr[i];
		if( pIcon )
		{
			pIcon->SetUsedFlag( false );
			pIcon->ChangeImage( 0 );
			pIcon->SetAniParam( 0 );
			pIcon->StopDelay();
		}
	}

	// 인덱스가 분할 갯수보다 클때
	if( mSplitInfo.GetSize() <= nSplitIndex )
	{
		assert(0);
		return;
	}

	mSelectedRowIdx		= 0;
	mTopRowIdx			= 0; 
	mHoveredRowIdx		= -1;
	mImageHoveredIdx	= -1;
	mShowImage			= true;			

	UpdateScrollBar();

	/// 선택바
	if( mpSelectImage )
		UpdateSelectImage();		

	unsigned int nSize = mSplitInfo[ nSplitIndex ];

	// 해당 영역에 갯수가 0보가 작거나
	if( nSize == 0 )
		return;
	
	unsigned int startIndex = nSplitIndex * mSplitSize;

	sRowData* pRowData	= NULL;
	sData* pData		= NULL;
	for( unsigned int i = startIndex; i < startIndex + nSize; i++ )
	{
		pRowData = (sRowData*)mRows[ i ];

		if( pRowData != NULL )
		{						
			for( unsigned int iData = 0; iData <  pRowData->mColumes.GetSize(); iData++ )
			{
				pData = (sData*)pRowData->mColumes[ iData ];

				if( pData != NULL )
				{					
					SAFE_DELETE( pData );
				}				
			}

			SAFE_DELETE( pRowData->mExtraData );
			SAFE_DELETE( pRowData );			
		}
	}	

	mSplitInfo[ nSplitIndex ] = 0;
}

// 분할 개수, 각 분할 영역 크기
void cSkillStepListBox::CreateSplitSize( unsigned char nSplitCnt, unsigned int nSplitSize )
{
	mRows.Resize( nSplitCnt *  nSplitSize );
	mSplitInfo.Resize( nSplitCnt );

	for( int i = 0; i < nSplitCnt; i++ )
		mSplitInfo[i] = 0;

	mSplitSize = nSplitSize;		// 분할 영역 크기
}


/// 스킬 아이콘 N개 생성
void cSkillStepListBox::CreateSkillICon( unsigned char nCount )
{
	cListBoxSkin* pStepListSkin = (cListBoxSkin*)mpSkin;

	if( !pStepListSkin )
	{
		assert(0);
		return;
	}
	
	int nIconPos		= pStepListSkin->mpImgParam->mColumnX;
	int nRowHeight		= pStepListSkin->mRowHeight;

	for( unsigned char i = 0; i < nCount; ++i )
	{
		cSkillIcon *pIcon = new cSkillIcon;
		if( pIcon->CreateBySkinName( "SkillIcon", this, i ) == false )
			return;

		pIcon->SetRelativePos( cUIPos( nIconPos, ( nRowHeight ) * i + 4  ) );
		pIcon->ChangeImage( 0 );		
		pIcon->SetUsedFlag(false);

		mSkillIconArr.PushBack( pIcon );
	}
}

// 렌더 데이타( 스킬 스트링 정보 ) 설정  
void cSkillStepListBox::RenderData( cUIFontItemKeeper* pKeeper )
{
	if( mCurrentSplitIndex >= mSplitInfo.GetSize() )
	{
		assert(0);
		return;
	}

	// 현재 영역에 사이즈가 0보다 작다면!
	if( mSplitInfo[ mCurrentSplitIndex ] <= 0 ) 
		return;

	/// 스킨 받아오기 
	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	if( !pListSkin )
		return;

	sListCtrlParam* pParam = pListSkin->GetCtrlParam( 0 );
	if( !pParam ) 
		return;

	int textposX		= 0;
	int textPosY		= 0;
	unsigned int tX		= pParam->mColumnX;
	unsigned int tW		= pParam->mColumnW;
	eText_Align align	= pParam->mAlign;

	unsigned int nCurrentSplitSize	= mSplitInfo[ mCurrentSplitIndex ];
	unsigned int nStartIndex		= mCurrentSplitIndex * mSplitSize;

	for( unsigned int i = 0;i<mSkillIconArr.GetSize(); i++ )
	{
		unsigned int row = nStartIndex + mTopRowIdx + i;
		if( row >= nStartIndex+nCurrentSplitSize )
			break;

		cSkillIcon *pSkillIcon = (cSkillIcon*)mSkillIconArr[i];
		if( pSkillIcon == 0 )
		{
			assert(0);
			continue;
		}

		sRowData* rData = (sRowData*)mRows[ row ];
		if( rData == 0 )
		{
			assert(0);
			pSkillIcon->ChangeImage( 0 );
			continue;
		}

		sSkillInfoEx *pSkillInfo = (sSkillInfoEx*)rData->mExtraData;
		if( pSkillInfo == 0 )
		{
			assert(0);
			pSkillIcon->ChangeImage( 0 );
			continue;
		}

		/// 스킬 이름 및 필요 습득 레벨, 강화 단계 표시 텍스트 표시
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			sData* data = rData->mColumes[col];
			if( !data ) continue;

			const cUIRect& rect = pSkillIcon->GetAbsoluteRect();

			unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );
			textPosY = rect.mTop;
			textPosY += (FONTAGENT->GetTextHeight(cFontAgent::eFont_System) * col + 2);

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned long color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = COLOR_RED; 

			pKeeper->AddFontItem( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), textposX, textPosY, color );
		}
	}
}

// 리스트 박스 Row 변경시 스킬 아이콘 재 설정
void cSkillStepListBox::UpdateIConListInfo()
{
	if( mCurrentSplitIndex >= mSplitInfo.GetSize() )
	{
		assert(0);
		return;
	}

	cSkillWindow *pSkillWin = GAMEUI->GetSkillWindow();
	if( !pSkillWin ) return;

	cSkillSheetWindow* pSheetWin = (cSkillSheetWindow*)pSkillWin->GetCurrentSheet();
	if( !pSheetWin ) return;

	if( pSheetWin->GetJobIdx() == 0 )
		return;

	unsigned int nCurrentSplitSize	= mSplitInfo[ mCurrentSplitIndex ];
	unsigned int nStartIndex		= mCurrentSplitIndex * mSplitSize;

	for( unsigned int i = 0; i< mSkillIconArr.GetSize(); i++ )
	{
		cSkillIcon *pSkillIcon = (cSkillIcon*)mSkillIconArr[i];
		if( pSkillIcon == 0 )
		{
			assert(0);
			continue;
		}

		unsigned int row = nStartIndex + mTopRowIdx + i;
		if( row >= nStartIndex+nCurrentSplitSize )
		{
			pSkillIcon->ChangeImage( 0 );
			continue;
		}

		sRowData* rData = (sRowData*)mRows[ row ];
		if( rData == 0 )
		{
			assert(0);
			pSkillIcon->ChangeImage( 0 );
			continue;
		}

		sSkillInfoEx *pSkillInfo = (sSkillInfoEx*)rData->mExtraData;
		if( pSkillInfo == 0 )
		{
			assert(0);
			pSkillIcon->ChangeImage( 0 );
			continue;
		}

		cIconParam* pIconParam	= UIMAN->GetIconParam( pSkillInfo->mpSkillInfo->mIconIdx );		
		if( pIconParam )
		{
			pSkillIcon->SetSkillInfo( pSkillInfo->mpSkillInfo->mSkillIdx, pSkillInfo->mSkillStep, pSheetWin->GetJobIdx(), eTIP_SKILL );
			pSkillIcon->ChangeImage( pIconParam );
			pSkillIcon->SetUsedFlag( true );
			pSkillIcon->SetColor( NiColorA::WHITE );
		}

		cSkillIcon *pSkillIconAtSheetWin = pSheetWin->GetSkillIcon( pSkillInfo->mpSkillInfo->mSkillIdx );
		if( pSkillIconAtSheetWin )
		{
			pSkillIcon->ChangeGauge( pSkillIconAtSheetWin->GetGauge() );
			pSkillIcon->SetAniParam( pSkillIconAtSheetWin->GetAniParam() );
		}
	}
}

// 해상도 변경시 아이콘 위치 값 재 설정
void cSkillStepListBox::UpdateSkin()
{
	cListBox::UpdateSkin();

	cListBoxSkin* pStepListSkin = (cListBoxSkin*)mpSkin;

	if( !pStepListSkin )
	{
		assert(0);
		return;
	}

	int nIconPos	= pStepListSkin->mpImgParam->mColumnX;
	int nRowHeight	= pStepListSkin->mRowHeight;

	// 스킬 아이콘 10개 생성
	for( u_int i = 0; i < mSkillIconArr.GetSize(); ++i )
	{
		cSkillIcon* pSkillIcon = (cSkillIcon*)mSkillIconArr[i];
		if( !pSkillIcon ) 
			continue;

		pSkillIcon->SetRelativePos( cUIPos( nIconPos, ( nRowHeight ) * i + 4  ) );
	}
}


//===========================================================
// 특정 영역에 가장 마지막 위치에 원소 삽입
//
//	nSplitIndex (분할 영역 인덱스)
//===========================================================
int cSkillStepListBox::AddRowAt( unsigned char nSplitIndex, LPCTSTR str, unsigned long color, void* pExtraData )
{
	// 분할 인데스가 잘못 되었거나
	if( nSplitIndex >= mSplitInfo.GetSize() )
	{
		assert(0);
		return -1;
	}

	// 현재 영역 원소 갯수
	unsigned int nCurrentSplitCnt = mSplitInfo[nSplitIndex];

	// 분할 영역의 원소 갯수가 분할 사이즈 보다 클때
	if( nCurrentSplitCnt >= mSplitSize )	
	{
		return -1;
	}

	// Data 생성
	sData* colData = new sData;
	colData->mText = str;
	colData->mTextColor = color;

	/// 라인 정보 생성
	sRowData* rowData = new sRowData;
	rowData->mColumes.PushBack( colData );

	if( pExtraData )
	{
		rowData->mExtraData = pExtraData;
	}
	

	// 특정 위치에 삽입
	unsigned int nStartIndex = nSplitIndex * mSplitSize;
	mRows[ nStartIndex + nCurrentSplitCnt ]	= rowData;	
	mSplitInfo[ nSplitIndex ]++; 
	
	UpdateScrollBar();

	if( mpSelectImage )
		UpdateSelectImage();

	return (nStartIndex + nCurrentSplitCnt);
}

void cSkillStepListBox::Sort()
{
	if( mCurrentSplitIndex >= mSplitInfo.GetSize() )
	{
		assert(0);
		return;
	}

	//지정한 분할 영역의 원소 갯수
	if( GetRowCount() > 1)
	{
		unsigned int nStartIndex	= mCurrentSplitIndex * mSplitSize;
		unsigned int nEndIndex		= nStartIndex + mSplitInfo[ mCurrentSplitIndex ];		

		cRowDataArray::cIterator iStart	= mRows.Begin();
		cRowDataArray::cIterator iEnd	= mRows.Begin();
		
		iStart	+= nStartIndex;
		iEnd	+= nEndIndex;

		::Sort( iStart, iEnd, SkillCompare );
	}
}

void cSkillStepListBox::OnIconDragged( cUINode* caller, unsigned int id, const cUIPos& pos, bool shift )
{
	SendEvent( mpParent, eUIEVENT_ICON_DRAGGED, caller, id, shift );
}

void cSkillStepListBox::OnIconDoubleClicked( cUINode* caller, unsigned int id )
{
	SendEvent( mpParent, eUIEVENT_ICON_DOUBLECLICKED, caller, id );	
}

void cSkillStepListBox::OnIconClicked( cUINode* caller, unsigned int id)
{
	OnLButtonDown( cUIPos(MOUSE->GetMouseX(), MOUSE->GetMouseY()), false, false, false );
}

void cSkillStepListBox::OnMouseWheel( const cUIPos& pos, int wheel )
{
	cListBox::OnMouseWheel( pos, wheel );
	// 리스트 박스 Row값 변경으로 아이콘 재 설정
	UpdateIConListInfo();
}

void cSkillStepListBox::OnScrollbarMoved( const cUIPos& pos )
{
	cListBox::OnScrollbarMoved( pos );
	// 리스트 박스 Row값 변경으로 아이콘 재 설정
	UpdateIConListInfo();
}

void cSkillStepListBox::CheckButtonEvent( unsigned int id )
{
	cListBox::CheckButtonEvent( id );
	// 리스트 박스 Row값 변경으로 아이콘 재 설정
	UpdateIConListInfo();
}

void cSkillStepListBox::OnLButtonDoubleClick( const cUIPos& pos )
{
	cUINode::OnLButtonDoubleClick( pos );

	if( mCurrentSplitIndex >= mSplitInfo.GetSize() )
	{
		assert(0);
		return;
	}

	unsigned int nStartIndex	= mCurrentSplitIndex * mSplitSize;
	unsigned int row			= (pos.mY - GetAbsoluteRect().mTop) / mRowHeight + mTopRowIdx + nStartIndex;

	if( row >= GetRowCount() )
		return;

	mSelectedRowIdx = row;
	mHoveredRowIdx	= row - mTopRowIdx;

	if( mHoveredRowIdx < 0 || mHoveredRowIdx >= (int)mSkillIconArr.GetSize() )
	{
		assert(0);
		return;
	}

	cSkillIcon* pIcon = (cSkillIcon*)mSkillIconArr[mHoveredRowIdx];
	if( pIcon != NULL )
	{	
		/// 이벤트 보내기 
		cUIEvent event;
		event.mType = eUIEVENT_LISTBOX_SELECTED;
		event.mpCaller = pIcon;
		event.mID = mID;
		mpParent->HandleEvent( event );
	}	

	/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
	if( mpSelectImage )
	{
		UpdateSelectImage();
		CheckShowImage();
	}
}
