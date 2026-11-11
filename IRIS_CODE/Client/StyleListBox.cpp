#include "StdAfx.h"
#include "StyleListBox.h"
#include "FontAgent.h"
#include "UIImage.h"
#include "UIImageFont.h"

cImageListBox::cImageListBox( eUINodeType type )
: cListBox( type )
{
}

cImageListBox::~cImageListBox()
{
	Clear();
}

void cImageListBox::Clear()
{
	mSelectedRowIdx = 0;
	mHoveredRowIdx = -1;

	mImageHoveredIdx = -1;

	mTopRowIdx = 0;

	cRowDataArray::cIterator i = mRows.Begin();
	cRowDataArray::cIterator end = mRows.End();

	sRowData* p = 0;
	for( ; i != end; ++i )
	{
		p = (sRowData*)(*i);
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				sListImageData* data = (sListImageData*)(p->mColumes[b]);
				if( data )
				{
					SAFE_DELETE( data->mpImage );
				}
				SAFE_DELETE( data );
			}
			p->mColumes.Clear();

			p->mExtraData = 0;
			SAFE_DELETE(p);
		}
	}
	mRows.Clear();

	/// 선택바
	if( mpSelectImage )
		UpdateSelectImage();

	UpdateScrollBar();
}

int cImageListBox::AddRow( LPCTSTR str, unsigned long color, cUIImage* image, void* pExtraData )
{
	if( GetRowCount() >= mTotalMaxNum )
		return -1;

	sListImageData* data = new sListImageData;
	data->mText			= str;
	data->mTextColor	= color;
	data->mpImage		= image;

	unsigned int ret = AddRowData( data, pExtraData );

	/// 스크롤바 갱신
	UpdateScrollBar();

	if( mpSelectImage )
		UpdateSelectImage();
	return ret;
}

bool cImageListBox::AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color )
{
	/// 수용갯수를 넘어가면 컬럼수 검사 
	if( GetRowCount() >= mTotalMaxNum )
	{
		unsigned int count = GetColumnCount( rowIdx - 1 );
		unsigned int countNow = GetColumnCount( rowIdx );

		if( countNow > count )
			return 0;
	}

	sListImageData* data = new sListImageData;
	data->mText = str;
	data->mTextColor = color;

	return AddColumeData( rowIdx, data );
}

/// 주의사항 : 마우스가 움직일때마다 선택된것을 바꾼다
void cImageListBox::OnMouseMove( const cUIPos& pos )
{
	cUINode::OnMouseMove( pos );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;
	if( row > mMaxRowInPage - 1 )
	{
		mHoveredRowIdx = -1;
		mImageHoveredIdx = -1;

		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_IMAGE_LEFT;
		mpParent->HandleEvent( event );
		return;
	}

	/// 줄 넘어가면
	if( row >= GetRowCount() )
	{
		mHoveredRowIdx = -1;
		mImageHoveredIdx = -1;

		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_IMAGE_LEFT;
		mpParent->HandleEvent( event );
		return;
	}

	unsigned int curRow = mTopRowIdx + row;
	if( curRow >= GetRowCount() )
		curRow = GetRowCount() - 1;

	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	sListCtrlParam* pParam = pListSkin->mpImgParam;

	int x1 = pParam->mColumnX + GetAbsoluteRect().mLeft;
	int x2 = x1 + pParam->mColumnW;

	if( pos.mX > x1 && pos.mX < x2 )
	{
		if( (unsigned int)mImageHoveredIdx != row )
		{
			cUIEvent event;
			event.mpCaller = this;
			event.mID = mID;
			event.mPos = pos;
			event.mExtraData = GetExtraData( curRow );
			event.mType = eUIEVENT_LISTBOX_IMAGE_HOVERED;
			mpParent->HandleEvent( event );

			mImageHoveredIdx = row;
		}
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

void cImageListBox::OnMouseLeft( const cUIPos& pos )
{
	cUIEvent event;
	event.mpCaller = this;
	event.mID = mID;
	event.mType = eUIEVENT_LISTBOX_IMAGE_LEFT;
	mpParent->HandleEvent( event );

	mImageHoveredIdx = -1;

	cListBox::OnMouseLeft( pos );
}

void cImageListBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
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
		event.mPos = pos;
		mpParent->HandleEvent( event );

		/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
		if( mpSelectImage )
		{
			UpdateSelectImage();
			CheckShowImage();
		}
	}
}

/// 주의사항 : 리스트 아이템 그려주기
void cImageListBox::RenderData( cUIFontItemKeeper* pKeeper )
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
		sRowData* rData = (sRowData*)GetRowData( row );
		if( !rData )
			continue;

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sListImageData* data = (sListImageData*)rData->mColumes[col];

			if( data->mpImage )
			{
				cUIImage* image = data->mpImage;

				sListCtrlParam* pParam = pListSkin->mpImgParam;
				unsigned int imageX = pParam->mColumnX;
				unsigned int imageY = textPosY + (int)((mRowHeight-image->GetHeight())*0.5f);

				image->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
				image->Draw();
			}

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
			unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = mHoverTextColor;

			int addY = (int)((mRowHeight-FONTAGENT->GetTextHeight(cFontAgent::eFont_UI))*0.5f);
			pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), textposX, textPosY + addY, color, mOutLine );
		}

		textPosY += mRowHeight;
	}
}

//-------------------------------------------------------------------------------------------------------------------------------
cCheckBoxListBox::cCheckBoxListBox( eUINodeType type )
: cImageListBox( type )
{
}

cCheckBoxListBox::~cCheckBoxListBox()
{
	Clear();
}

void cCheckBoxListBox::Clear()
{
	mSelectedRowIdx = 0;
	mHoveredRowIdx = -1;

	mImageHoveredIdx = -1;

	mTopRowIdx = 0;

	UpdateScrollBar();

	/// 선택바
	if( mpSelectImage )
		UpdateSelectImage();

	cRowDataArray::cIterator i = mRows.Begin();
	cRowDataArray::cIterator end = mRows.End();

	sRowData* p = 0;
	for( ; i != end; ++i )
	{
		p = (sRowData*)(*i);
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				sListCheckData* data = (sListCheckData*)p->mColumes[b];
				if( data )
				{
					SAFE_DELETE( data->mpImage );
				}
				SAFE_DELETE( data );
			}
			p->mColumes.Clear();

			p->mExtraData = 0;
			SAFE_DELETE(p);
		}
	}
	mRows.Clear();
}

int cCheckBoxListBox::AddRow( LPCTSTR str, unsigned long color, cUIImage* image, 
							 cUINodeSkin* checkSkin, cUINodeSkin* nocheckSkin, void* pExtraData )
{
	if( GetRowCount() >= mTotalMaxNum )
		return -1;

	sListCheckData* data = new sListCheckData;
	data->mText			= str;
	data->mTextColor	= color;
	data->mpImage		= image;
	data->mpCheckSkin	= checkSkin;
	data->mpNoCheckSkin = nocheckSkin;
	data->mCheck		= false;

	unsigned int ret = AddRowData( data, pExtraData );

	/// 스크롤바 갱신
	UpdateScrollBar();

	if( mpSelectImage )
		UpdateSelectImage();
	return ret;
}

bool cCheckBoxListBox::AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color )
{
	/// 수용갯수를 넘어가면 컬럼수 검사 
	if( GetRowCount() >= mTotalMaxNum )
	{
		unsigned int count = GetColumnCount( rowIdx - 1 );
		unsigned int countNow = GetColumnCount( rowIdx );

		if( countNow > count )
			return 0;
	}

	sListCheckData* data = new sListCheckData;
	data->mText = str;
	data->mTextColor = color;

	return AddColumeData( rowIdx, data );
}


/// 주의사항 : 마우스가 움직일때마다 선택된것을 바꾼다
void cCheckBoxListBox::OnMouseMove( const cUIPos& pos )
{
	cUINode::OnMouseMove( pos );

	unsigned int row = (pos.mY - GetAbsoluteRect().mTop) / mRowHeight;
	if( row > mMaxRowInPage - 1 )
	{
		mHoveredRowIdx = -1;
		
		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_IMAGE_LEFT;
		mpParent->HandleEvent( event );
		return;
	}

	/// 줄 넘어가면
	if( row >= GetRowCount() )
	{
		mHoveredRowIdx = -1;

		cUIEvent event;
		event.mpCaller = this;
		event.mID = mID;
		event.mType = eUIEVENT_LISTBOX_IMAGE_LEFT;
		mpParent->HandleEvent( event );
		return;
	}

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

void cCheckBoxListBox::OnLButtonDown( const cUIPos& pos, bool ctrl, bool alt, bool shift )
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
		event.mPos = pos;
		mpParent->HandleEvent( event );

		/// 마우스를 눌러 선택한 부분은 다르게 칠한다 
		if( mpSelectImage )
		{
			UpdateSelectImage();
			CheckShowImage();
		}
	}

	sListCheckData* data = (sListCheckData*)GetColumeData( mSelectedRowIdx, 0 );
	if( data )
	{
		/// 스킨 받아오기 
		cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
		sListCtrlParam* pParam = pListSkin->mpImgParam;

		int x1 = pParam->mColumnX + GetAbsoluteRect().mLeft;
		int x2 = x1 + pParam->mColumnW;

		if( pos.mX > x1 && pos.mX < x2 )
		{
			/// 이미지가 클릭된다면,
			data->mCheck = !data->mCheck;

			cUIRect rc = data->mpImage->GetTextureRect();

			cUINodeSkin* skin = ( data->mCheck == true ) ? data->mpCheckSkin : data->mpNoCheckSkin;
			if( skin )
			{
				data->mpImage->SetTextureRect( skin->mSkinInfo->mTexX, skin->mSkinInfo->mTexY, 
					skin->mSkinInfo->mTexX + skin->mSkinInfo->mTexWidth, 
					skin->mSkinInfo->mTexY + skin->mSkinInfo->mTexHeight );
			}
			else
				assert(0);
		}
	}
}

void cCheckBoxListBox::Check( bool check, unsigned int row )
{
	sListCheckData* data = (sListCheckData*)GetColumeData( row, 0 );
	if( data )
	{
		data->mCheck = check;

		cUIRect rc = data->mpImage->GetTextureRect();

		cUINodeSkin* skin = ( data->mCheck == true ) ? data->mpCheckSkin : data->mpNoCheckSkin;
		if( skin )
		{
			data->mpImage->SetTextureRect( skin->mSkinInfo->mTexX, skin->mSkinInfo->mTexY, 
				skin->mSkinInfo->mTexX + skin->mSkinInfo->mTexWidth, 
				skin->mSkinInfo->mTexY + skin->mSkinInfo->mTexHeight );
		}
		else
			assert(0);
	}
}

bool cCheckBoxListBox::GetCheck( unsigned int row )
{
	sListCheckData* data = (sListCheckData*)GetColumeData( row, 0 );
	return ( data ) ? data->mCheck : false;
}

/// 주의사항 : 리스트 아이템 그려주기
void cCheckBoxListBox::RenderData( cUIFontItemKeeper* pKeeper )
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
		sRowData* rData = (sRowData*)GetRowData( row );
		if( !rData )
			continue;

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sListCheckData* data = (sListCheckData*)rData->mColumes[col];

			if( data->mpImage ) 
			{
				cUIImage* image = data->mpImage;

				sListCtrlParam* pParam = pListSkin->mpImgParam;
				unsigned int imageX = pParam->mColumnX;
				unsigned int imageY = textPosY + (int)((mRowHeight-image->GetHeight())*0.5f);

				image->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
				image->Draw();
			}

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
			unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = mHoverTextColor;

			int addY = (int)((mRowHeight-FONTAGENT->GetTextHeight(cFontAgent::eFont_UI))*0.5f);
			pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), textposX, textPosY + addY, color, mOutLine );
		}

		textPosY += mRowHeight;
	}
}


//-------------------------------------------------------------------------------------------------------------------------------
cImageAndFontDataListBox::cImageAndFontDataListBox( eUINodeType type )
: cImageListBox( type )
{
}

cImageAndFontDataListBox::~cImageAndFontDataListBox()
{
	Clear();
}

void cImageAndFontDataListBox::Clear()
{
	mSelectedRowIdx = 0;
	mHoveredRowIdx = -1;

	mImageHoveredIdx = -1;

	mTopRowIdx = 0;

	UpdateScrollBar();

	/// 선택바
	if( mpSelectImage )
		UpdateSelectImage();

	cRowDataArray::cIterator i = mRows.Begin();
	cRowDataArray::cIterator end = mRows.End();

	sRowData* p = 0;
	for( ; i != end; ++i )
	{
		p = (sRowData*)(*i);
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				sListImageAndFontData* data = (sListImageAndFontData*)p->mColumes[b];
				if( data )
				{
					SAFE_DELETE( data->mpImage );
					SAFE_DELETE( data->mpEdgeImage );
					SAFE_DELETE( data->mpSealImage );
				}
				SAFE_DELETE( data );
			}
			p->mColumes.Clear();

			p->mExtraData = 0;
			SAFE_DELETE(p);
		}
	}
	mRows.Clear();
}

int cImageAndFontDataListBox::AddRow( LPCTSTR str, unsigned long color, cUIImage* image, cUIImage* edgeImage, 
									  cUIImage* sealImage, cUIImageFont* font, bool showfont, unsigned long count, void* pExtraData )
{
	if( GetRowCount() >= mTotalMaxNum )
		return -1;

	sListImageAndFontData* data = new sListImageAndFontData;
	data->mText = str;
	data->mTextColor = color;
	data->mpImage = image;
	data->mpEdgeImage = edgeImage;
	data->mpSealImage = sealImage;
	data->mpFont = font;
	data->mShowFont = showfont;
	data->mCount = count;

	unsigned int ret = AddRowData( data, pExtraData );

	/// 스크롤바 갱신
	UpdateScrollBar();

	if( mpSelectImage )
		UpdateSelectImage();
	return ret;
}

bool cImageAndFontDataListBox::AddColume( unsigned int rowIdx, LPCTSTR str, unsigned long color )
{
	/// 수용갯수를 넘어가면 컬럼수 검사 
	if( GetRowCount() >= mTotalMaxNum )
	{
		unsigned int count = GetColumnCount( rowIdx - 1 );
		unsigned int countNow = GetColumnCount( rowIdx );

		if( countNow > count )
			return 0;
	}

	sListImageAndFontData* data = new sListImageAndFontData;
	data->mText = str;
	data->mTextColor = color;

	return AddColumeData( rowIdx, data );
}


/// 주의사항 : 리스트 아이템 그려주기
void cImageAndFontDataListBox::RenderData( cUIFontItemKeeper* pKeeper )
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
		sRowData* rData = (sRowData*)GetRowData( row );
		if( !rData )
			continue;

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sListImageAndFontData* data = (sListImageAndFontData*)rData->mColumes[col];

			if( data->mpImage )
			{
				cUIImage* image = data->mpImage;

				sListCtrlParam* pParam = pListSkin->mpImgParam;
				unsigned int imageX = pParam->mColumnX;
				unsigned int imageY = textPosY + (int)((mRowHeight-image->GetHeight())*0.5f);

				image->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
				image->Draw();

				/// 이미지폰트 찍기
				if( data->mShowFont && data->mpFont )
				{
					cString Number;
					Number.Format( "%d", data->mCount );

					cUIImageFont* font = data->mpFont;

					int right = GetAbsoluteRect().mLeft + imageX + image->GetWidth();
					int bottom = imageY + image->GetHeight();
					font->DrawText( right - 1 - font->GetTextWidth( data->mCount ), bottom -1 - font->GetTextHeight(), Number.Cstr() );
				}

				/// 테두리이미지 찍기
				cUIImage* edgeImg = data->mpEdgeImage;
				if( edgeImg )
				{
					edgeImg->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
					edgeImg->Draw();
				}

				// 봉인이미지 찍기
				cUIImage* sealImg = data->mpSealImage;
				if( sealImg )
				{
					sealImg->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
					sealImg->Draw();
				}
			}

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
			unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = mHoverTextColor;

			int addY = (int)((mRowHeight-FONTAGENT->GetTextHeight(cFontAgent::eFont_UI))*0.5f);
			pKeeper->AddFontItem( cFontAgent::eFont_UI, (LPTSTR)data->mText.Cstr(), textposX, textPosY + addY, color, mOutLine );
		}

		textPosY += mRowHeight;
	}
}

