#include "stdafx.h"
#include "HeightMap.h"

#include "FileLoader.h"
#include "FileSaver.h"

cHeightMap::cHeightMap()
{
	mWidth = 0;
	mHeight = 0;
}

cHeightMap::cHeightMap( unsigned int width, unsigned int height )
: mWidth( width )
, mHeight( height )
{
	unsigned int size = mWidth * mHeight;

	if( size <= 0 )
	{
		assert( 0 );
	}
	else
	{
		mBuffer.Resize( size );
	}
}

void cHeightMap::Clear()
{
	mWidth = 0;
	mHeight = 0;
	mBuffer.Clear();
}

bool cHeightMap::Load( const cString& pathName )
{
	Clear();

	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
		return false;

	cTokenizer tok( loader.GetBufferPtr(), loader.GetSize(), " ,\t\r\n", pathName.Cstr() );
	cString str;

	/// 크기
	tok.GetNext( &str );
	mWidth = str.ToInt();

	tok.GetNext( &str );
	mHeight = str.ToInt();

	unsigned int size = mWidth * mHeight;

	if( size <= 0 )
		return false;

	/// 높이값 배열
	mBuffer.Resize( size );

	for( unsigned int i = 0; i < size; ++i )
	{
		if( tok.IsEnd() )
			break;

		tok.GetNext( &str );
        mBuffer[i] = str.ToFloat();
	}
	return true;
}

bool cHeightMap::Save( const cString& pathName )
{
	cFileSaver saver;
	if( saver.Open( pathName ) == false )
		return false;

	unsigned int size = mWidth * mHeight;

	if( size <= 0 )
		return false;

	/// 크기
	cString str;
	str.Format( "%d,%d\n", mWidth, mHeight );
	saver.Write( str.Cstr(), str.GetSize() );

	/// 높이값 배열
	for( unsigned int i = 0, y = 0; y < mHeight; ++y )
	{
		for( unsigned int x = 0; x < mWidth; ++x, ++i )
		{
			str.Format( "%f,", mBuffer[i] );
			saver.Write( str.Cstr(), str.GetSize() );
		}

		saver.Write( "\n", 1 );
	}
	return true;
}

bool cHeightMap::SetValue( unsigned int x, unsigned int y, float height )
{
	if( x >= mWidth || y >= mHeight )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	unsigned int i = x + mWidth * y;
	mBuffer[i] = height;
	return true;
}

bool cHeightMap::GetValue( float* height, unsigned int x, unsigned int y )
{
	assert( height );

	if( x >= mWidth || y >= mHeight )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	unsigned int i = x + mWidth * y;
	*height = mBuffer[i];
	return true;
}

bool cHeightMap::GetValue( float* height, float x, float y )
{
	assert( height );

	if( x < 0.0f || x > 1.0f )
	{
		assert( 0 && "ratio out of range" );
		return false;
	}

	if( y < 0.0f || y > 1.0f )
	{
		assert( 0 && "ratio out of range" );
		return false;
	}

	unsigned int xi = (unsigned int)(x * (mWidth-1));
	unsigned int yi = (unsigned int)(y * (mHeight-1));

	unsigned int i = xi + mWidth * yi;
	*height = mBuffer[i];
	return true;
}
