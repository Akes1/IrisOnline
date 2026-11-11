#include "Stdafx.h"
#include "TextCtrl.h"

#include "FontAgent.h"
#include "UIImage.h"

cTextCtrl::cTextCtrl( eUINodeType type )
: cUIWindow( type )
, mMaxTextWidth(0)
, mTotalMaxNum(0)
{
	mTextQueue.Reserve( 60 );
	mFontType = cFontAgent::eFont_System;
}

cTextCtrl::~cTextCtrl()
{
	Clear();
}

/// 주의사항 : 전체를 지운다
void cTextCtrl::Clear()
{
	sRowTextData* p = 0;
	for( unsigned int i = 0; i < mTextQueue.GetSize(); ++i )
	{
		p = (sRowTextData*)mTextQueue[i];
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				SAFE_DELETE( p->mColumes[b] );
			}
			p->mColumes.Clear();
			SAFE_DELETE(p);
		}
	}
	mTextQueue.Clear();
}

/// 공백라인 추가
void cTextCtrl::AddEmptyRow( unsigned int line )
{
	for( unsigned int i = 0; i < line; ++i )
	{
		// 라인 정보
		sRowTextData* rowData = new sRowTextData;

		// 데이터 채우기
		sTextData* data = new sTextData;
		data->mColor = mDefaultColor;
		data->mText = _T("");
		data->mEnter = true;

		/// 라인정보를 삽입
		rowData->mColumes.PushBack( data );

		/// 실제 열에 삽입
		mTextQueue.PushBack( rowData );

		/// 최대 갯수보다 크면
		if( mTextQueue.GetSize() >= mTotalMaxNum )
		{
			sRowTextData* p = (sRowTextData*)mTextQueue.Front();
			if( p )
			{
				for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
				{
					SAFE_DELETE( p->mColumes[b] );
				}
				p->mColumes.Clear();

				SAFE_DELETE(p);
			}		

			mTextQueue.PopFront();
		}
	}
}

/// 새로운 라인 추가
void cTextCtrl::AddNewRow( LPCTSTR str, unsigned long color, sInventory* inventory )
{
	typedef tArray<cStringT> cStringArr;
	cStringArr stringArr;

	unsigned int len = ::_tcslen( str );
	if( len == 0 )
		return;

	/// \\n 부호로 문자열 자르기
	LPCTSTR s = str;
	LPCTSTR p = str;
	TCHAR temp[256] = {0,};
	for( unsigned int i = 0; i < len - 1; ++i, ++p )
	{
		if( ::_tcsncmp( p, _T("\\n"), 2 ) == 0 )
		{
			::ZeroMemory( temp, 256 );

			Sstrncpy( temp, 256, s, p - s );
			stringArr.PushBack( temp );
			
			s = p + 2;
		}
	}

	unsigned int checkLen = p - s;
	if( checkLen < len )
	{
		::ZeroMemory( temp, 256 );

		Sstrncpy( temp, 256, s, len );
		stringArr.PushBack( temp );
	}

	/// 문자열 삽입
	TCHAR tempStr[256] = {0,};
	for( unsigned int i = 0; i < stringArr.GetSize(); ++i )
	{
		cStringT data = stringArr[i];

		unsigned int width = FONTAGENT->GetTextExtent( mFontType, data.Cstr(), data.GetLength() );
		unsigned int count = width / mMaxTextWidth;

		/// 한줄이면
		if( count <= 0 )
		{
			::ZeroMemory( tempStr, 256 );

			Sstrncpy( tempStr, 256, data.Cstr(), data.GetLength() );
			InsertRow( tempStr, color, inventory );
		}
		/// 여러줄 이면
		else
		{
			/// 너무 길면 잘라내기.
			unsigned int cutLen = 0;
			unsigned int cutWidth = 0;

			LPCTSTR orginStr = data.Cstr();
			LPCTSTR checkStr = data.Cstr();

			bool flag = true;
			while( flag )
			{
				int length = checkStr - orginStr;
				if( cutLen + length >= data.GetLength() )
					break;

				cutLen++;
				cutWidth = FONTAGENT->GetTextExtent( mFontType, checkStr, cutLen );

				if( cutWidth > mMaxTextWidth )
				{
					/// 넘어간 글자 하나 땡기기
					if( cutLen > 1)
						cutLen--;

					::ZeroMemory( tempStr, 256 );

					Sstrncpy( tempStr, 256, checkStr, cutLen );
					InsertRow( tempStr, color, inventory );		

					checkStr += cutLen;
					cutLen = 0;
				}
			}

			if( cutLen != 0 )
			{
				::ZeroMemory( tempStr, 256 );

				Sstrncpy( tempStr, 256, checkStr, cutLen );
				InsertRow( tempStr, color, inventory );
			}
		}
	}
}


/// 기존라인에 연결하여 추가
void cTextCtrl::AddPasteRow( LPCTSTR str, unsigned long color, sInventory* inventory )
{
	if( !str )
		return;

	TCHAR tempStr[256] = {0,};
	unsigned int len = ::_tcslen( str );
	if( len > 256 )
	{
		assert(0);
		return;
	}

	if( len == 0 )
		return;

	if( mTextQueue.GetSize() == 0 )
	{
		assert(0);
		return;
	}

	int line = mTextQueue.GetSize();

	sRowTextData* pRow = (sRowTextData*)mTextQueue[line-1];
	if( pRow )
	{
		sTextData* pColumn = 0;
		unsigned int beforeWidth = 0;

		int size = pRow->mColumes.GetSize();
		if( size > 0 )
		{
			for( int i = 0; i < size; ++i )
			{
				pColumn = pRow->mColumes[i];

				/// 엔터가 있으면,
				if( pColumn->mEnter == false )
					beforeWidth += FONTAGENT->GetTextExtent( mFontType, (LPCTSTR)pColumn->mText.Cstr(), pColumn->mText.GetLength() );
				else
					beforeWidth += 1;
			}
		}

		unsigned int width = FONTAGENT->GetTextExtent( mFontType, str, len );

		/// 한열에 다 들어가면,
		if( beforeWidth + width <= mMaxTextWidth )
		{
			InsertColumn( str, color, inventory );
		}
		else
		{
			LPCTSTR ptokenStr = str;

			/// 너무 길면 잘라내기.
			unsigned int cutLen = 0;
			unsigned int cutWidth = 0; 

			bool flag = true;
			while( flag )
			{
				int length = ptokenStr - str;
				if( cutLen + length >= len )
					break;

				cutLen++;
				cutWidth = FONTAGENT->GetTextExtent( mFontType, ptokenStr, cutLen );

				if( beforeWidth + cutWidth > mMaxTextWidth )
				{
					/// 넘어간 글자 하나 땡기기
					if( cutLen > 1)
						cutLen--;

					::ZeroMemory( tempStr, 256 );

					Sstrncpy( tempStr, 256, ptokenStr, cutLen );

					if( beforeWidth > 0 )
						InsertColumn( tempStr, color, inventory );
					else
						InsertRow( tempStr, color, inventory );
					
					ptokenStr += cutLen;

					beforeWidth = 0;
					cutLen = 0;
				}
			}

			if( cutLen != 0 )
			{
				::ZeroMemory( tempStr, 256 );
				Sstrncpy( tempStr, 256, ptokenStr, cutLen );

				if( beforeWidth > 0 )
					InsertColumn( tempStr, color, inventory );
				else
					InsertRow(  tempStr, color, inventory );
			}
		}
	}
	else
	{
		assert(0);
	}
}

///
void cTextCtrl::InsertRow( LPCTSTR str, unsigned long color, sInventory* inventory )
{
	// 라인 정보
	sRowTextData* rowData = new sRowTextData;

	// 데이터 채우기
	if( inventory == NULL )
	{
		sTextData* data = new sTextData;
		data->mColor = color;
		data->mText = str;

		/// 라인정보를 삽입
		rowData->mColumes.PushBack( data );
	}
	else
	{
		sInvenData* data = new sInvenData;
		data->mColor = color;
		data->mText = str;
		data->mUseInven = true;
		data->mInventory = *inventory;

		/// 라인정보를 삽입
		rowData->mColumes.PushBack( data );
	}

	/// 실제 열에 삽입
	mTextQueue.PushBack( rowData );

	/// 최대 갯수보다 크면
	if( mTextQueue.GetSize() >= mTotalMaxNum )
	{
		sRowTextData* p = (sRowTextData*)mTextQueue.Front();
		if( p )
		{
			for( unsigned int b = 0; b < p->mColumes.GetSize(); ++b )
			{
				SAFE_DELETE( p->mColumes[b] );
			}
			p->mColumes.Clear();
	
			SAFE_DELETE(p);
		}		

		mTextQueue.PopFront();
	}
}

void cTextCtrl::InsertColumn( LPCTSTR str, unsigned long color, sInventory* inventory )
{
	if( mTextQueue.GetSize() == 0 )
	{
		assert(0);
		return;
	}

	int line = mTextQueue.GetSize();
	sRowTextData* pRow = (sRowTextData*)mTextQueue[line-1];

	if( inventory == NULL )
	{
		sTextData* data = new sTextData;
		data->mColor = color;
		data->mText = str;

		pRow->mColumes.PushBack( data );
	}
	else
	{
		sInvenData* data = new sInvenData;
		data->mColor = color;
		data->mText = str;
		data->mUseInven = true;
		data->mInventory = *inventory;

		pRow->mColumes.PushBack( data );
	}
}

sRowTextData* cTextCtrl::GetRowData( unsigned int rowIdx )
{
	if( rowIdx >= mTextQueue.GetSize() )
		return 0;

	return (sRowTextData*)mTextQueue[rowIdx];
}
