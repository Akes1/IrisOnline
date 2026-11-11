/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.11
* 내    용 : tHashSet을 구현
* 주의사항 : 
*===========================================================================*/

public:
	tHashSet()
		: mTable( 50, cHasher(), cKeyEqual(), cAllocatorType() )
	{
	}

	tHashSet( unsigned int hint )
		: mTable( hint, cHasher(), cKeyEqual(), cAllocatorType() )
	{
	}

	tHashSet( const cSelf& other )
		: mTable( other.mTable )
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

	cIterator Find( const T& val )
	{
		return mTable.find( val );
	}

	cConstIterator Find( const T& val) const
	{
		return mTable.find( val );
	}

	void NoneUniqueInsert( const T& val )
	{
		mTable.insert_equal( val );
	}

	bool Insert( const T& val )
	{
		return mTable.insert_unique( val ).second;
	}

	void Insert( const T* first, const T* last )
	{
		mTable.insert_unique( first, last );
	}

	void Insert( cConstIterator first, cConstIterator last )
	{
		mTable.insert_unique( first, last );
	}

	unsigned int Erase( const T& val )
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

	unsigned int GetCount( const T& val ) const
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
