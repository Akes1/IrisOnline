/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tPointerHashSet을 구현
* 주의사항 : 
*===========================================================================*/

public:
	tPointerHashSet()
		: mTable( 50, cHasher(), cKeyEqual(), cAllocatorType() )
	{
	}

	explicit tPointerHashSet( unsigned int hint )
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

	cIterator Find( const T val )
	{
		return mTable.find( val );
	}

	cConstIterator Find( const T val) const
	{
		return mTable.find( val );
	}

	bool Insert( const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return false;
		}
		return mTable.insert_unique( val ).second;
	}

	unsigned int Erase( const T val )
	{
		return mTable.erase( val );
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

	unsigned int GetCount( const T val ) const
	{
		return mTable.count( val );
	}

	unsigned int GetSize() const
	{
		return mTable.size();
	}

	bool IsEmpty() const
	{
		return mTable.empty();
	}
