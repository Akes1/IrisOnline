/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tPointerQueue를 구현
* 주의사항 : 
*===========================================================================*/

public:
	tPointerQueue()
		: mHead( NPOS )
		, mTail( 0 )
		, mEnd( 0 )
	{
		mpBuffer = 0;
	}

	explicit tPointerQueue( unsigned int capacity )
		: mHead( NPOS )
		, mTail( 0 )
		, mEnd( capacity )
	{
		mpBuffer = ALLOC::Allocate( capacity );
		assert( mpBuffer );
	}

	~tPointerQueue()
	{
		Clear();
		ALLOC::Deallocate( mpBuffer );
	}

	bool IsEmpty() const
	{
		return mHead == NPOS;
	}

	bool IsFull() const
	{
		return mHead == mTail;
	}

	unsigned int GetSize() const
	{
		if( IsEmpty() )
		{
			return 0;
		}
		else if( mHead >= mTail )
		{
			return mEnd - mHead + mTail;
		}
		else
		{
			return mTail - mHead;
		}
	}

	unsigned int GetCapacity() const
	{
		return mEnd;
	}

	void Clear()
	{
		while( IsEmpty() == false )
		{
			PopFront();
		}
		mHead = NPOS;
		mTail = 0;
	}

	void Reserve( unsigned int capacity )
	{
		if( capacity <= GetCapacity() )
		{
			return;
		}
		else
		{
			T* newBase = ALLOC::Allocate( capacity );
			assert( newBase != 0 );

			if( IsEmpty() == false )
			{
				unsigned int j = 0;

				if( mHead >= mTail )
				{
					for( unsigned int i = mHead; i < mEnd; ++i )
					{
						newBase[j++] = mpBuffer[i];
					}

					for( unsigned int i = 0; i < mTail; ++i )
					{
						newBase[j++] = mpBuffer[i];
					}
				}
				else
				{
					for( unsigned int i = mHead; i < mTail; ++i )
					{
						newBase[j++] = mpBuffer[i];
					}
				}

				assert( j == GetSize() );
				mHead = 0;
				mTail = j;
			}

			mEnd = capacity;
			ALLOC::Deallocate( mpBuffer );
			mpBuffer = newBase;
		}
	}

	bool PushBack( const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return false;
		}
		/// 용량 검사
		if( GetCapacity() == 0 )
		{
			assert( 0 && "zero capacity" );
			return false;
		}
		if( IsEmpty() )
		{
			mHead = 0;
			tElement<T>::Construct( mpBuffer + 0, val );
			mTail = 1;
		}
		else
		{
			if( IsFull() )
			{
				return false;
			}

			tElement<T>::Construct( mpBuffer + mTail, val );
			++mTail;
		}

		if( mTail == mEnd)
		{
			mTail = 0;
		}
		return true;
	}

	bool PopFront()
	{
		if( IsEmpty() )
		{
			return false;
		}

		tElement<T>::Destroy( mpBuffer + mHead );

		if( ++mHead == mEnd )
		{
			mHead = 0;
		}
		if( mHead == mTail )
		{
			mHead = NPOS;
			mTail = 0;
		}
		return true;
	}

	T Front()
	{
		assert( IsEmpty() == false );
		return mpBuffer[mHead];
	}

	const T Front() const
	{
		assert( IsEmpty() == false );
		return mpBuffer[mHead];
	}

	T Back()
	{
		assert( IsEmpty() == false );
		if( mTail == 0 )
			return mpBuffer[mEnd-1];
		else
			return mpBuffer[mTail-1];
	}

	const T Back() const
	{
		assert( IsEmpty() == false );
		if( mTail == 0 )
			return mpBuffer[mEnd-1];
		else
			return mpBuffer[mTail-1];
	}

	T operator [] ( unsigned int i )
	{
		assert( IsEmpty() == false );
		assert( i < GetSize() );
		return mpBuffer[(mHead + i) % mEnd];
	}

	const T operator [] ( unsigned int i ) const
	{
		assert( IsEmpty() == false );
		assert( i < GetSize() );
		return mpBuffer[(mHead + i) % mEnd];
	}
