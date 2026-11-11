/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tArray를 구현
* 주의사항 : 
*===========================================================================*/

public:
	explicit tArray()
		: vector()
	{
	}

	explicit tArray( unsigned int size )
		: vector( size )
	{
	}

	tArray( unsigned int size, const T& val )
		: vector( size, val )
	{
	}

	tArray( const cSelf& other )
		: vector( other )
	{
	}

	void Clear()
	{
		clear();
	}
	void Assign( unsigned int n, const T& val )
	{
		assign( n, val );
	}

	void Assign( cIterator first, cIterator last )
	{
		assign( first, last );
	}

	void Reserve( unsigned int capacity )
	{
		reserve( capacity );
	}

	void PushBack( const T& val )
	{
		push_back( val );
	}

	bool PopBack()
	{
		if( size() == 0 )
		{
			assert( 0 && "zero size" );
			return false;
		}
		pop_back();
		return true;
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

	void Resize( unsigned int size, const T& val )
	{
		resize( size, val );
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

	T& Front()
	{
		return *begin();
	}

	const T& Front() const
	{
		return *begin();
	}

	T& Back()
	{
		return *(end() - 1);
	}

	const T& Back() const
	{
		return *(end() - 1);
	}

	bool GetAt( T* val, unsigned int i )
	{
		assert( val );

		if( i >= size() )
		{
			assert( 0 && "index out of range" );
			return false;
		}
		else
		{
			*val = *(begin() + i);
			return true;
		}
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
		assert(i < size());
		return *(begin() + i);
	}

	const T& operator [] ( unsigned int i ) const
	{
		assert(i < size());
		return *(begin() + i);
	}
