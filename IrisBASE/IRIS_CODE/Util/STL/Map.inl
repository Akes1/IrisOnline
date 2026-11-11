/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.13
* 내    용 : tMap를 구현
* 주의사항 : 
*===========================================================================*/

public:
	tMap()
	{
	}

	void Clear()
	{
		mTree.clear();
	}

	void Resize( unsigned int hint )
	{
		mTree.resize( hint );
	}

	cIterator Find( const KEY& key )
	{
		return mTree.find( key );
	}

	cConstIterator Find( const KEY& key) const
	{
		return mTree.find( key );
	}

	bool Insert( const KEY& key, const T& val )
	{
		return mTree.insert_unique( cPair( key, val ) ).second;
	}

	void Insert( const cPair* first, const cPair* last )
	{
		mTree.insert_unique( first, last );
	}

	void Insert( cConstIterator first, cConstIterator last )
	{
		mTree.insert_unique( first, last );
	}

	unsigned int Erase( const KEY& key )
	{
		return mTree.erase( key );
	}

	void Erase( cIterator pos )
	{
		mTree.erase( pos );
	}

	void Erase( cIterator first, cIterator last )
	{
		mTree.erase( first, last );
	}

	cIterator Begin()
	{
		return mTree.begin();
	}

	cConstIterator Begin() const
	{
		return mTree.begin();
	}

	cIterator End()
	{
		return mTree.end();
	}

	cConstIterator End() const
	{
		return mTree.end();
	}

	bool GetAt( T* val, const KEY& key ) const
	{
		assert( val );

		cConstIterator i = mTree.find( key );
		if( i == mTree.end() )
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
		return mTree.count( key );
	}

	unsigned int GetSize() const
	{
		return mTree.size();
	}

	bool IsEmpty() const
	{
		return mTree.empty();
	}

	T& operator [] ( const KEY& key )
	{
		cIterator i = mTree.find( key );
		if( i == mTree.end() )
		{
			assert( 0 && "failed to find key" );
			return mTree.insert_unique( cPair(key, T()) ).mSecond;
		}
		else
		{
			return (*i).mSecond;
		}
	}
