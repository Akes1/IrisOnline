/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.10.08
* 내    용 : 우선순위 큐
* 주의사항 : 
*===========================================================================*/
#pragma once

#include "Functor.h"

/// 우선순위 큐
/// 이진 트리를 이용하여 제공된 정렬 기준에 따라서 원소를 자동으로 정렬한다.
/// 한번 늘어난 용량은 줄어들지 않는다.
template <class T, class COMPARE = tBinaryLess<T> >
class tPriorityQueue
{
	/*
	explicit tPriorityQueue( unsigned int capacity );
	~tPriorityQueue();

	/// 모든 원소를 제거
	void Clear();

	/// 용량을 설정
	void Reserve( unsigned int capacity );

	/// 원소를 추가
	/// 용량을 넘어서면 false를 리턴한다.
	bool Push( T val );

	/// 첫번째 원소를 제거
	/// 원소가 하나도 없는 경우 false를 리턴한다.
	bool PopFront();

	/// 첫번째 원소를 리턴
	T Front();
	T Front() const;

	/// 원소 수를 리턴
	unsigned int GetCount() const;

	/// 용량을 리턴
	unsigned int GetCapacity() const;
	*/

protected:
	typedef tPriorityQueue<T, COMPARE> cSelf;

protected:
	/// 비교 함수자
	COMPARE mCompare;

	/// 배열
	T* mHead;

	/// 용량
	unsigned int mCapacity;

	/// 원소 수
	unsigned int mCount;

protected:
	void Reserve( unsigned int capacity )
	{
		if( capacity <= mCapacity )
			return;

		/// 배열을 생성
		T* newHead = new T[capacity];
		assert( newHead );

		/// 이전 배열을 복사
		for( unsigned int i = 0; i < mCount; ++i )
			newHead[i] = mHead[i];

		delete[] mHead;
		mHead = newHead;
		mCapacity = capacity;
	}

	void WalkUp( unsigned int index )
	{
		unsigned int parent = index / 2;
		unsigned int child = index;

		T temp = mHead[child];

		while( parent > 0 )
		{
			if( mCompare( temp, mHead[parent] ) )
			{
				mHead[child] = mHead[parent];
				child = parent;
				parent /= 2;
			}
			else
				break;
		}

		mHead[child] = temp;
	}

	void WalkDown( unsigned int index )
	{
		unsigned int parent = index;
		unsigned int child = index * 2;

		T temp = mHead[parent];

		while( child < mCount )
		{
			if( child < mCount - 1 )
			{
				if( mCompare( mHead[child], mHead[child + 1] ) == false )
				{
					child++;
				}
			}
			if( mCompare( temp, mHead[child] ) == false )
			{
				mHead[parent] = mHead[child];
				parent = child;
				child *= 2;
			}
			else
				break;
		}

		mHead[parent] = temp;
	}

public:
	explicit tPriorityQueue( unsigned int capacity )
		: mHead( 0 )
		, mCapacity( 0 )
		, mCount( 0 )
	{
		assert( capacity );

		Reserve( capacity + 1 );
	}

	~tPriorityQueue()
	{
		delete [] mHead;
	}

	void Clear()
	{
		mCount = 0;
	}

	bool Push( T value )
	{
		if( mCount + 1 >= mCapacity )
		{
			return false;
		}

		++mCount;
		mHead[mCount] = value;
		WalkUp( mCount );
		return true;
	}

	void PopFront()
	{
		if( mCount )
		{
			mHead[1] = mHead[mCount];
			WalkDown( 1 );
			--mCount;
		}
	}

	T Front()
	{
		return mHead[1];
	}

	T Front() const
	{
		return mHead[1];
	}

	unsigned int GetCount() const
	{
		return mCount;
	}

	unsigned int GetCapacity() const
	{
		return mCapacity;
	}
};
