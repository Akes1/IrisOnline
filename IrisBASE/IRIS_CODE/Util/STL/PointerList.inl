/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.12.06
* 내    용 : tPointerList를 구현
* 주의사항 : 
*===========================================================================*/

public:
	explicit tPointerList()
		: list()
	{
	}

	void Clear()
	{
		clear();
	}

	cIterator Insert( cIterator pos, const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return pos;
		}
		else
		{
			return insert( pos, val );
		}
	}

	bool PushFront( const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return false;
		}
		else
		{
			push_front( val );
			return true;
		}
	}

	bool PushBack( const T val )
	{
		/// 널 포인터 검사
		if( val == 0 )
		{
			assert( 0 && "null pointer" );
			return false;
		}
		else
		{
			push_back( val );
			return true;
		}
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

	void Remove( const T val )
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
