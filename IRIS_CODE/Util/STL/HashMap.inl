/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.11
* 내    용 : tHashMap를 구현
* 주의사항 : 
*===========================================================================*/

public:
	tHashMap()
		: mTable( 50, cHasher(), cKeyEqual(), cAllocatorType() )
	{
	}

	explicit tHashMap( unsigned int hint )
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

	bool Insert( const KEY& key, const T& val )
	{
		return mTable.insert_unique( cPair( key, val ) ).second;
	}

	void Insert( const cPair* first, const cPair* last )
	{
		mTable.insert_unique( first, last );
	}

	void Insert( cConstIterator first, cConstIterator last )
	{
		mTable.insert_unique( first, last );
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
		else
		{
			*val = (*i).mSecond;
			return true;
		}
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

	T& operator [] ( const KEY& key )
	{
		cIterator i = mTable.find( key );
		if( i == mTable.end() )
		{
			assert( 0 && "failed to find key" );
			return mTable._M_insert( cPair(key, T()) ).mSecond;
		}
		else
		{
			return (*i).mSecond;
		}
	}
