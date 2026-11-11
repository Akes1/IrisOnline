/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.02.06
* 내    용 : tList를 구현
* 주의사항 : 
*===========================================================================*/

public:
	explicit tList()
		: list()
	{
	}

	explicit tList( unsigned int size )
		: list( size )
	{
	}

	tList( unsigned int size, const T& val )
		: list( size, val )
	{
	}

	tList( cIterator first, cIterator last )
		: list( first, last )
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

	cIterator Insert( cIterator pos, const T& val )
	{
		return insert( pos, val );
	}

	void Insert( cIterator pos, cIterator first, cIterator last )
	{
		insert( pos, first, last );
	}

	void PushFront( const T& val )
	{
		push_front( val );
	}

	void PushBack( const T& val )
	{
		push_back( val );
	}

	cIterator Erase( cIterator pos )
	{
		if( pos == end() )
		{
			return pos;
		}
		else
		{
			return erase( pos );
		}
	}

	cIterator Erase( cIterator first, cIterator last )
	{
		return erase( first, last );
	}

	void PopFront()
	{
		pop_front();
	}

	void PopBack()
	{
		pop_back();
	}

	void Splice( cIterator pos, cSelf& other )
	{
		splice( pos, other );
	}

	void Splice( cIterator pos, cSelf& other, cIterator first, cIterator last )
	{
		splice( pos, other, first, last );
	}


	void Splice( cIterator pos, cSelf& other, cIterator i )
	{
		splice( pos, other, i );
	}

	void Remove( const T& val )
	{
		remove( val );
	}

	template<class Predicate>
		void RemoveIf( Predicate op )
	{
		remove_if( op );
	}

	void Sort()
	{
		sort();
	}

	template<class StrictWeakOrdering>
		void Sort( StrictWeakOrdering op )
	{
		sort( op );
	}

	cIterator Begin()
	{
		return begin();
	}

	cConstIterator Begin() const
	{
		return begin();
	}

	cIterator End()
	{
		return end();
	}

	cConstIterator End() const
	{
		return end();
	}

	T& Front()
	{
		return front();
	}

	const T& Front() const
	{
		return front();
	}

	T& Back()
	{
		return back();
	}

	const T& Back() const
	{
		return back();
	}

	unsigned int GetSize() const
	{
		return size();
	}

	bool IsEmpty() const
	{
		return empty();
	}
