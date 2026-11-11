/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.09.13
* 내    용 : tSet을 구현
* 주의사항 : 
*===========================================================================*/

public:
	tSet()
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

	cIterator Find( const T& val )
	{
		return mTree.find( val );
	}

	cConstIterator Find( const T& val) const
	{
		return mTree.find( val );
	}

	bool Insert( const T& val )
	{
		return mTree.insert_unique( val ).second;
	}

	void Insert( const T* first, const T* last )
	{
		mTree.insert_unique( first, last );
	}

	void Insert( cConstIterator first, cConstIterator last )
	{
		mTree.insert_unique( first, last );
	}

	unsigned int Erase( const T& val )
	{
		return mTree.erase( val );
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

	unsigned int GetCount( const T& val ) const
	{
		return mTree.count( val );
	}

	unsigned int GetSize() const
	{
		return mTree.size();
	}

	bool IsEmpty() const
	{
		return mTree.empty();
	}
