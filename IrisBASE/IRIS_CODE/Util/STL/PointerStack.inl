/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tPointerStack를 구현
* 주의사항 : 
*===========================================================================*/

public:
	tPointerStack()
	{
	}

	explicit tPointerStack( unsigned int capacity )
	{
		assert( capacity > 0 );
		reserve( capacity );
	}

	void Clear()
	{
		clear();
	}

	void Reserve( unsigned int capacity )
	{
		reserve( capacity );
	}

	bool PushBack( const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return false;
		}

		if( IsFull() )
		{
			assert( 0 && "stack overflow" );
			return false;
		}
		else
		{
			push_back( val );
			return true;
		}
	}

	bool PopBack( )
	{
		if( IsEmpty() )
		{
			return false;
		}
		else
		{
			pop_back();
			return true;
		}
	}

	T& Back()
	{
		assert( empty() == false );
		return back();
	}

	const T& Back() const
	{
		assert( empty() == false );
		return back();
	}

	unsigned int GetSize() const
	{
		return size();
	}

	unsigned int GetCapacity() const
	{
		return capacity();
	}

	bool IsEmpty() const
	{
		return empty();
	}

	bool IsFull() const
	{
		return size() == capacity();
	}
