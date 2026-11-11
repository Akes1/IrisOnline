#include "stdafx.h"
#include "BitArray.h"

cBitArray::cBitArray( unsigned int size )
: mArray( 0 )
, mSize( 0 )
, mWidth( 0 )
, mHeight( 0 )
{
	assert( size > 0 );

	Resize( size );
}

cBitArray::~cBitArray()
{
	Clear();
}

void cBitArray::Clear()
{
	if( mArray )
	{
		delete [] mArray;
		mArray = 0;
	}
	mSize = 0;
}

void cBitArray::Resize( unsigned int size )
{
	Clear();

	if( size )
	{
		// convert the bit-size into an integer-size
		if( size % BITSIZE == 0 )
			mSize = size / BITSIZE;
		else
			mSize = size / BITSIZE + 1;

		mArray = new unsigned int[mSize];
		assert( mArray );
	}
	else
	{
		mSize = 0;
	}
}

void cBitArray::Set( unsigned int index, bool value )
{
	unsigned int cell = index / BITSIZE;
	unsigned int bit = index % BITSIZE;
	assert( cell < mSize );

	if( value )
		mArray[cell] = (mArray[cell] | (1 << bit));
	else
		mArray[cell] = (mArray[cell] & (~(1 << bit)));
}

void cBitArray::SetAll( bool value )
{
	if( value )
	{
		for( unsigned int i = 0; i < mSize; i++ )
			mArray[i] = 0xFFFFFFFF;
	}
	else
	{
		for( unsigned int i = 0; i < mSize; i++ )
			mArray[i] = 0;
	}
}
