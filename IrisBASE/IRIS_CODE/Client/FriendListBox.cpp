#include "StdAfx.h"
#include "FriendListbox.h"
#include "ListBox.h"
#include "UIImage.h"
#include "FontAgent.h"

cFriendListbox::cFriendListbox( eUINodeType type )
: cListBox( type )
{

}

cFriendListbox::~cFriendListbox()
{

}

/// 주의사항 : 리스트 아이템 그려주기
void cFriendListbox::RenderData( cUIFontItemKeeper* pKeeper )
{
	/// 전체 리스트를 얻어옴 
	if( GetRowCount() <= 0 )
		return;

	/// 스킨 받아오기 
	cListBoxSkin* pListSkin = (cListBoxSkin*)mpSkin;
	if( !pListSkin )
		return;

	/// 리스트 박스의 좌표 받아오기
	int textposX = 0;
	int textPosY = GetAbsoluteRect().mTop;

	unsigned int rowCount = ( mMaxRowInPage > GetRowCount() )? GetRowCount() : mMaxRowInPage;
	for( unsigned int row = mTopRowIdx; row < mTopRowIdx + rowCount; ++row )
	{
		sRowData* rData = GetRowData( row );
		if( !rData )
			continue;

		/// image 출력 
		if( rData->mImage )
		{
			cUIImage* image = rData->mImage;

			sListCtrlParam* pParam = pListSkin->mpImgParam;
			unsigned int imageX = pParam->mColumnX;
			unsigned int imageY = textPosY + (int)((mRowHeight-image->GetHeight())*0.5f);

			image->SetPos( GetAbsoluteRect().mLeft + imageX, imageY );
			image->Draw();
		}

		/// 리스트 데이터 출력
		unsigned int colCount = rData->mColumes.GetSize();
		unsigned int fixposY = textPosY;
		for( unsigned int col = 0; col< colCount; ++col )
		{
			/// 각 컬럼 데이터 가져오기
			sData* data = rData->mColumes[col];
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
			unsigned int strWidth = FONTAGENT->GetTextExtent( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), data->mText.GetLength() );
			textposX = UpdateTextPos( align, strWidth, tX, tW );

			/// Hover 된 아이템은 다른 색으로 표시
			unsigned color = data->mTextColor;
			if( mHoveredRowIdx != -1 && row - mTopRowIdx == (unsigned int)mHoveredRowIdx )
				color = eCOLOR_RED;

			pKeeper->AddFontItem( cFontAgent::eFont_System, (LPTSTR)data->mText.Cstr(), textposX, fixposY + 5, color );

			/// 두번째 컬럼부터는 바로 한줄 밑에 찍는다
			fixposY += FONTAGENT->GetTextHeight(cFontAgent::eFont_System) + 2;
		}

		textPosY += mRowHeight;
	}
}