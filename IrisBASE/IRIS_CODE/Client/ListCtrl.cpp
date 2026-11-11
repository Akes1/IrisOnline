#include "Stdafx.h"
#include "ListCtrl.h"
#include "UIImage.h"

cListCtrl::cListCtrl( eUINodeType type )
: cUIWindow( type )
{
}

cListCtrl::~cListCtrl()
{
	Clear();
}

/// 주의사항 : 전체를 지운다
void cListCtrl::Clear()
{
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
				sData* data = (sData*)p->mColumes[b];
				SAFE_DELETE( data );
			}
			p->mColumes.Clear();

			p->mExtraData = 0;
			SAFE_DELETE(p);
		}
	}
	mRows.Clear();
}

/// 새로운 행을 생성하고 하나의 열을 추가한다.
unsigned int cListCtrl::AddRowData( sData* colData, void* pExtraData )
{
	/// 라인 정보 생성
	sRowData* data = new sRowData;

	/// 정보 갱신
	data->mColumes.PushBack( colData );
	data->mExtraData = pExtraData;

	/// 라인 정보 등록
	mRows.PushBack( data );

	/// 등록 인덱스 리턴..
	return mRows.GetSize() - 1;
}

/// 특정 라인에 열을 추가한다.
bool cListCtrl::AddColumeData( unsigned int rowIdx, sData* colData )
{
	if( rowIdx > mRows.GetSize()-1 )
		return false;

	sRowData* pRow = (sRowData*)mRows[rowIdx];
	pRow->mColumes.PushBack( colData );
	return true;
}

///
void* cListCtrl::GetExtraData( unsigned int rowIdx )
{
	if( mRows.GetSize() <= 0 )
		return 0;

	if( rowIdx > mRows.GetSize()-1 )
	{
		assert(0);
		return 0;
	}

	sRowData* pRow = (sRowData*)mRows[rowIdx];
	return ( pRow != 0 ) ? pRow->mExtraData : 0;
}

sRowData* cListCtrl::GetRowData( unsigned int rowIdx ) 
{
	if( mRows.GetSize() <= 0 )
		return 0;

	if( rowIdx > mRows.GetSize()-1 )
		return 0;

	return (sRowData*)mRows[rowIdx]; 
}

unsigned int cListCtrl::GetColumnCount( unsigned int rowIdx )
{
	if( mRows.GetSize() <= 0 )
		return 0;

	if( rowIdx > mRows.GetSize()-1 )
	{
		assert(0);
		return 0;
	}

	sRowData* pRow = (sRowData*)mRows[rowIdx];
	return pRow->mColumes.GetSize();
}

bool Compare( const void* arg1, const void* arg2 )
{
	sRowData* rowData1 = (sRowData*)arg1;
	sRowData* rowData2 = (sRowData*)arg2;

	sData* data1 = rowData1->mColumes[0];
	sData* data2 = rowData2->mColumes[0];
	if( ::_tcscmp( (LPCTSTR)data1->mText.Cstr(), (LPCTSTR)data2->mText.Cstr() ) >= 0 )
		return false;
	return true;
}

void cListCtrl::Sort()
{
	if( mRows.GetSize() > 1 )
		::Sort( mRows.Begin(), mRows.End(), Compare );
}

