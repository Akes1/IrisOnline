/* ==========================================================================
* 파    일 : UIRect.h
* 목    적 : 
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.30
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "UIPos.h"
#include "UISize.h"

#pragma warning( disable: 4201 )

/// UI 노드의 영역
template <class T>
class tUIRect
{
public:
	tUIRect();
	tUIRect( T left, T top, T right, T bottom );
	tUIRect( const tUIPos<T>& upperLeft, const tUIPos<T>& lowerRight );
	tUIRect( const tUIRect<T>& other );
	tUIRect( const tUIPos<T>& pos, const tUISize<T>& Size );

	/// 설정
	void Set( T left, T top, T right, T bottom );
	void SetPos( const tUIPos<T>& pos );

	/// 클리핑
	void ClipAgainst( const tUIRect<T>& other );

	/// 잘못된 부분을 수정
	void Repair();

	/// 점을 포함하는지 여부를 리턴
	bool ContainPoint( const tUIPos<T>& pos ) const;

	/// 다른 영역과 교차하는지 여부를 리턴
	bool IntersectRect( const tUIRect<T>& other ) const;

	/// 중심을 리턴
	tUIPos<T> GetCenter() const;

	/// 너비를 리턴
	T GetWidth() const;

	/// 높이를 리턴
	T GetHeight() const;

	/// 크기를 리턴
	tUISize<T> GetSize() const;

	/// 적합성 여부를 리턴
	bool IsValid() const;

	const tUIRect<T>& operator = ( const tUIRect<T>& other );
	tUIRect<T> operator + ( const tUIPos<T>& pos ) const;
	const tUIRect<T>& operator += ( const tUIPos<T>& pos );
	bool operator == ( const tUIRect<T>& other ) const;
	bool operator != ( const tUIRect<T>& other ) const;

public:
	union
	{
		struct
		{
			T mLeft;
			T mTop;
			T mRight;
			T mBottom;
		};

		struct
		{
			tUIPos<T> mUpperLeft;
			tUIPos<T> mLowerRight;
		};
	};
};

template <class T> inline
tUIRect<T>::tUIRect()
: mLeft( 0 )
, mTop( 0 )
, mRight( 0 )
, mBottom( 0 )
{
}

template <class T> inline
tUIRect<T>::tUIRect( T left, T top, T right, T bottom )
{
	mLeft = left;
	mTop = top;
	mRight = right;
	mBottom = bottom;
}

template <class T> inline
tUIRect<T>::tUIRect( const tUIPos<T>& upperLeft, const tUIPos<T>& lowerRight )
: mUpperLeft( upperLeft )
, mLowerRight( lowerRight )
{
}

template <class T> inline
tUIRect<T>::tUIRect( const tUIRect<T>& other )
: mUpperLeft( other.mUpperLeft )
, mLowerRight( other.mLowerRight )
{
}

template <class T> inline
tUIRect<T>::tUIRect( const tUIPos<T>& pos, const tUISize<T>& Size )
: mUpperLeft( pos )
, mLowerRight( pos.mX + Size.mWidth, pos.mY + Size.mHeight )
{
}

template <class T> inline
void tUIRect<T>::Set( T left, T top, T right, T bottom )
{
	mLeft = left;
	mTop = top;
	mRight = right;
	mBottom = bottom;
}

template <class T> inline
void tUIRect<T>::SetPos( const tUIPos<T>& pos )
{
	tUISize<T> Size;

	Size.mWidth = GetWidth();
	Size.mHeight = GetHeight();

	mLeft = pos.mX;
	mTop = pos.mY;
	mRight = mLeft + Size.mWidth;
	mBottom = mTop + Size.mHeight;
}

template <class T> inline
void tUIRect<T>::ClipAgainst( const tUIRect<T>& other ) 
{
	if( other.mRight < mRight )
	{
		mRight = other.mRight;
	}
	if( other.mBottom < mBottom )
	{
		mBottom = other.mBottom;
	}

	if( other.mLeft > mLeft )
	{
		mLeft = other.mLeft;
	}
	if( other.mTop > mTop )
	{
		mTop = other.mTop;
	}

	if( mTop > mBottom )
	{
		mTop = mBottom;
	}
	if( mLeft > mRight )
	{
		mLeft = mRight;
	}
}

template <class T> inline
void tUIRect<T>::Repair()
{
	if( mRight < mLeft )
	{
		T t = mRight;
		mRight = mLeft;
		mLeft = t;
	}

	if( mBottom < mTop )
	{
		T t = mBottom;
		mBottom = mTop;
		mTop = t;
	}
}

template <class T> inline
bool tUIRect<T>::ContainPoint( const tUIPos<T>& pos ) const
{
	return ( mLeft <= pos.mX &&
		mTop <= pos.mY &&
		mRight >= pos.mX &&
		mBottom >= pos.mY );
}

template <class T> inline
bool tUIRect<T>::IntersectRect( const tUIRect<T>& other ) const
{
	return ( mBottom > other.mTop &&
		mTop < other.mBottom &&
		mRight > other.mLeft &&
		mLeft < other.mRight );
}

template <class T> inline
tUIPos<T> tUIRect<T>::GetCenter() const
{
	return tUIPos<T>( ( mLeft + mRight ) / 2, ( mTop + mBottom ) / 2 );
}

template <class T> inline
T tUIRect<T>::GetWidth() const
{
	return mRight - mLeft;
}

template <class T> inline
T tUIRect<T>::GetHeight() const
{
	return mBottom - mTop;
}

template <class T> inline
tUISize<T> tUIRect<T>::GetSize() const
{
	return tUISize<T>( GetWidth(), GetHeight() );
}

template <class T> inline
bool tUIRect<T>::IsValid() const
{
	T dx = mRight - mLeft;
	T dy = mBottom - mTop;

	return !( dx < 0 || dy < 0 || ( dx == 0 && dy == 0 ) );
}

template <class T> inline
const tUIRect<T>& tUIRect<T>::operator = ( const tUIRect<T>& other )
{
	mUpperLeft = other.mUpperLeft;
	mLowerRight = other.mLowerRight;
	return *this;
}

template <class T> inline
tUIRect<T> tUIRect<T>::operator + ( const tUIPos<T>& pos ) const
{
	tUIRect<T> ret( *this );
	ret.mUpperLeft += pos;
	ret.mLowerRight += pos;
	return ret;
}

template <class T> inline
const tUIRect<T>& tUIRect<T>::operator += ( const tUIPos<T>& pos )
{
	mUpperLeft += pos;
	mLowerRight += pos;
	return *this;
}

template <class T> inline
bool tUIRect<T>::operator == ( const tUIRect<T>& other ) const
{
	return ( mUpperLeft == other.mUpperLeft && mLowerRight == other.mLowerRight );
}

template <class T> inline
bool tUIRect<T>::operator != ( const tUIRect<T>& other ) const
{
	return ( mUpperLeft != other.mUpperLeft || mLowerRight != other.mLowerRight );
}
