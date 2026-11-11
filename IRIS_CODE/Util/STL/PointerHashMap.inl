/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tPointerHashMap를 구현
* 주의사항 : 
*===========================================================================*/

public:
	tPointerHashMap()
		: mTable( 50, cHasher(), cKeyEqual(), cAllocatorType() )
	{
	}

	explicit tPointerHashMap( unsigned int hint )
		: mTable( hint, cHasher(), cKeyEqual(), cAllocatorType() )
	{
	}

	void Clear()
	{
		mTable.clear();
	}

	void Resize( unsigned int hint )
	{
		mTable.resize( hint );
	}

	cIterator Find( const KEY& key )
	{
		return mTable.find( key );
	}

	cConstIterator Find( const KEY& key) const
	{
		return mTable.find( key );
	}

	bool Insert( const KEY& key, const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return false;
		}
		return mTable.insert_unique( cPair( key, val ) ).second;
	}

	unsigned int Erase( const KEY& key )
	{
		return mTable.erase( key );
	}

	void Erase( cIterator pos )
	{
		mTable.erase( pos );
	}

	void Erase( cIterator first, cIterator last )
	{
		mTable.erase( first, last );
	}

	cIterator Begin()
	{
		return mTable.begin();
	}

	cConstIterator Begin() const
	{
		return mTable.begin();
	}

	cIterator End()
	{
		return mTable.end();
	}

	cConstIterator End() const
	{
		return mTable.end();
	}

	bool GetAt( T* val, const KEY& key ) const
	{
		assert( val );

		cConstIterator i = mTable.find( key );
		if( i == mTable.end() )
		{
			return false;
		}

		/// 널 포인터 검사
		T p = (*i).mSecond;
		if( p == 0 )
		{
			assert( p && "null pointer" );
			return false;
		}

		*val = p;
		return true;
	}

	T GetAt( const KEY& key ) const
	{
		cConstIterator i = mTable.find( key );
		if( i == mTable.end() )
		{
			return 0;
		}

		return (*i).mSecond;
	}

	unsigned int GetCount( const KEY& key ) const
	{
		return mTable.count( key );
	}

	unsigned int GetSize() const
	{
		return mTable.size();
	}

	bool IsEmpty() const
	{
		return mTable.empty();
	}

	T operator [] ( const KEY& key )
	{
		cIterator i = mTable.find( key );
		if( i == mTable.end() )
		{
			assert( 0 && "failed to find key" );
			return (T)0;
		}
		else
		{
			return (*i).mSecond;
		}
	}
