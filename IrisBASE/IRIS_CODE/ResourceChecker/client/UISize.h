/* ==========================================================================
* 파    일 : UISize.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.30
* 주의사항 : 
*===========================================================================*/
#pragma once

/// UI 노드의 크기
template <class T>
class tUISize
{
public:
	tUISize();
	tUISize( T width, T height );
	tUISize( const tUISize<T>& other );

	bool operator == ( const tUISize<T>& other ) const;
	bool operator != ( const tUISize<T>& other ) const;
	const tUISize<T>& operator = ( const tUISize<T>& other );

public:
	T mWidth;
	T mHeight;
};

template <class T> inline
tUISize<T>::tUISize()
: mWidth( 0 )
, mHeight( 0 )
{
}

template <class T> inline
tUISize<T>::tUISize( T width, T height )
: mWidth( width )
, mHeight( height )
{
}

template <class T> inline
tUISize<T>::tUISize( const tUISize<T>& other )
: mWidth( other.mWidth )
, mHeight( other.mHeight )
{
}

template <class T> inline
bool tUISize<T>::operator == ( const tUISize<T>& other ) const
{
	return mWidth == other.mWidth && mHeight == other.mHeight;
}

template <class T> inline
bool tUISize<T>::operator != ( const tUISize<T>& other ) const
{
	return mWidth != other.mWidth || mHeight != other.mHeight;
}

template <class T> inline
const tUISize<T>& tUISize<T>::operator = ( const tUISize<T>& other ) 
{
	mWidth = other.mWidth;
	mHeight = other.mHeight;
	return *this;
}
