/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.15
* 내    용 : tStack를 구현
* 주의사항 : 
*===========================================================================*/

public:
	tStack()
	{
	}

	explicit tStack( unsigned int capacity )
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

	bool PushBack( const T& val )
	{
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
