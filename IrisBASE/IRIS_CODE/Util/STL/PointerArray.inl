/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tPointerArray를 구현
* 주의사항 : 
*===========================================================================*/

public:
	explicit tPointerArray()
		: vector()
	{
	}

	explicit tPointerArray( unsigned int size )
		: vector( size )
	{
	}

	tPointerArray( const cSelf& other )
		: vector( other )
	{
	}

	void Clear()
	{
		clear();
	}

	void Reserve( unsigned int capacity )
	{
		reserve( capacity );
	}

	void PushBack( const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
			assert( 0 && "null pointer" );
		else
			push_back( val );
	}

	void PopBack()
	{
		if( size() == 0 )
			assert( 0 && "zero size" );
		else
			pop_back();
	}

	bool PopAt( unsigned int i )
	{
		unsigned int s = size();

		if( i >= s )
		{
			assert( 0 && "index out of range" );
			return false;
		}
		if( s == 0 )
		{
			assert( 0 && "zero size" );
			return false;
		}
		if( s > 1 && i != s - 1 )
		{
			*(begin() + i) = *(end() - 1);
		}
		pop_back();
		return true;
	}

	bool PopAt( bool* orderChanged, unsigned int i )
	{
		*orderChanged = false;
		unsigned int s = size();

		if( i >= s )
		{
			assert( 0 && "index out of range" );
			return false;
		}
		if( s == 0 )
		{
			assert( 0 && "zero size" );
			return false;
		}
		if( s > 1 && i != s - 1 )
		{
			*(begin() + i) = *(end() - 1);
			*orderChanged = true;
		}
		pop_back();
		return true;
	}

	void Resize( unsigned int size )
	{
		resize( size );
	}

	T* Begin()
	{
		return begin();
	}

	const T* Begin() const
	{
		return begin();
	}

	T* End()
	{
		return end();
	}

	const T* End() const
	{
		return end();
	}

	T Front()
	{
		T p = *begin();
		assert( p && "null pointer" );
		return p;
	}

	const T Front() const
	{
		T p = *begin();
		assert( p && "null pointer" );
		return p;
	}

	T Back()
	{
		T p = *(end() - 1);
		assert( p && "null pointer" );
		return p;
	}

	const T Back() const
	{
		T p = *(end() - 1);
		assert( p && "null pointer" );
		return p;
	}

	bool GetAt( T* val, unsigned int i )
	{
		assert( val );

		if( i >= size() )
		{
			assert( 0 && "index out of range" );
			return false;
		}

		/// 널 포인터 검사
		T p = *(begin() + i);
		if( p == 0 )
		{
			assert( p && "null pointer" );
			return false;
		}

		*val = p;
		return true;
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

	T& operator [] ( unsigned int i )
	{
		return *(begin() + i);
	}

	const T& operator [] ( unsigned int i ) const
	{
		return *(begin() + i);
	}
