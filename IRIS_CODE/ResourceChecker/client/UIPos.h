/* ==========================================================================
* 파    일 : UIPos.h
* 목    적 :
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.30
* 주의사항 : 
*===========================================================================*/
#pragma once

/// UI 노드의 위치
template <class T>
class tUIPos
{
public:
	tUIPos( T x, T y );
	tUIPos();
	tUIPos( const tUIPos<T>& other );

	tUIPos<T> operator + ( const tUIPos<T>& other ) const;
	tUIPos<T> operator - ( const tUIPos<T>& other ) const;
	const tUIPos<T>& operator = ( const tUIPos<T>& other );
	const tUIPos<T>& operator += ( const tUIPos<T>& other );
	const tUIPos<T>& operator -= ( const tUIPos<T>& other );
	bool operator == ( const tUIPos<T>& other ) const;
	bool operator != ( const tUIPos<T>& other ) const;

public:
	T mX;
	T mY;
};

template <class T> inline
tUIPos<T>::tUIPos( T x, T y )
: mX( x )
, mY( y )
{
}

template <class T> inline
tUIPos<T>::tUIPos()
: mX( 0 )
, mY( 0 )
{
}

template <class T> inline
tUIPos<T>::tUIPos( const tUIPos<T>& other )
: mX( other.mX )
, mY( other.mY )
{
}

template <class T> inline
tUIPos<T> tUIPos<T>::operator + ( const tUIPos<T>& other ) const
{
	return tUIPos<T>( mX+other.mX, mY+other.mY );
}

template <class T> inline
tUIPos<T> tUIPos<T>::operator - ( const tUIPos<T>& other ) const
{
	return tUIPos<T>( mX-other.mX, mY-other.mY );
}

template <class T> inline
const tUIPos<T>& tUIPos<T>::operator = ( const tUIPos<T>& other ) 
{
	mX = other.mX;
	mY = other.mY;
	return *this;
}

template <class T> inline
const tUIPos<T>& tUIPos<T>::operator += ( const tUIPos<T>& other )
{
	mX += other.mX;
	mY += other.mY;
	return *this;
}

template <class T> inline
const tUIPos<T>& tUIPos<T>::operator -= ( const tUIPos<T>& other )
{
	mX -= other.mX;
	mY -= other.mY;
	return *this;
}

template <class T> inline
bool tUIPos<T>::operator == ( const tUIPos<T>& other ) const
{
	return mX == other.mX && mY == other.mY;
}

template <class T> inline
bool tUIPos<T>::operator != ( const tUIPos<T>& other ) const
{
	return mX != other.mX || mY != other.mY;
}
