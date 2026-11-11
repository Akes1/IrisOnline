#include "stdafx.h"
#include "StringTable.h"

cStringTable::cStringTable()
{
	mMaxCol = 0;
}

cStringTable::~cStringTable()
{
	Clear();
}

void cStringTable::Clear()
{
	for( unsigned int r = 0, rend = mTable.GetSize(); r < rend; ++r )
	{
		cArray* parray = mTable[r];

		if( parray == 0 )
			continue;

		cArray& array = *parray;

		for( unsigned int c = 0, cend = array.GetSize(); c < cend; ++c )
		{
			cString* pstr = array[c];

			if( pstr == 0 )
				continue;

			delete pstr;
		}
		delete parray;
	}

	mTable.Clear();
}

bool cStringTable::Save( const cString& pathName )
{
	/// 파일을 열기
	cFileSaver saver;

	if( saver.Open( pathName ) == false )
	{
		return false;
	}

	for( unsigned int r = 0, rend = mTable.GetSize(); r < rend; ++r )
	{
		cArray* parray = mTable[r];
		unsigned int c = 0;

		if( parray == 0 )
		{
			saver.Write( ",\r\n", 3 );
		}
		else
		{
			cArray& array = *parray;

			for( unsigned int cend = array.GetSize(); c < cend; ++c )
			{
				cString* pstr = array[c];

				if( pstr )
					saver.Write( pstr->Cstr(), pstr->GetSize() );

				if( c < cend - 1 )
					saver.Write( ",", 1 );
			}
			saver.Write( "\r\n", 2 );
		}
	}
	return true;
}

void cStringTable::SetValue( unsigned int row, unsigned int col, const cString& value )
{
	if( row >= mTable.GetSize() )
		mTable.Resize( row + 1 );

	cArray* parray = mTable[row];

	if( parray == 0 )
		parray = mTable[row] = new cArray;

	cArray& array = *parray;

	if( col >= array.GetSize() )
	{
		array.Resize( col + 1 );

		if( col > mMaxCol )
			mMaxCol = col;
	}

	cString* pstr = array[col];

	if( pstr == 0 )
		pstr = array[col] = new cString;

	*pstr = value;
}

void cStringTable::SetValue( unsigned int row, unsigned int col, const char* value )
{
	if( row >= mTable.GetSize() )
		mTable.Resize( row + 1 );

	cArray* parray = mTable[row];

	if( parray == 0 )
		parray = mTable[row] = new cArray;

	cArray& array = *parray;

	if( col >= array.GetSize() )
	{
		array.Resize( col + 1 );

		if( col > mMaxCol )
			mMaxCol = col;
	}

	cString* pstr = array[col];

	if( pstr == 0 )
		pstr = array[col] = new cString;

	*pstr = value;
}

void cStringTable::SetValue( unsigned int row, unsigned int col, unsigned int value )
{
	char str[64];
	_itoa_s( value, str, 10 );

	SetValue( row, col, str );
}

void cStringTable::SetValue( unsigned int row, unsigned int col, float value )
{
	cString str;
	str.Format( "%.2f", value );

	SetValue( row, col, str );
}
