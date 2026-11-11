/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2006.09.12
* 내    용 : tBasicString을 구현
* 주의사항 : 
*===========================================================================*/

public:
	explicit tBasicString()
		: basic_string()
	{
	}

	tBasicString( const cSelf& s )
		: basic_string( s )
	{
	}

	tBasicString( const cSelf& s, unsigned int pos, unsigned int n = NPOS )
		: basic_string( s, pos, n )
	{
	}

	tBasicString( const T* s )
		: basic_string( s )
	{
	}

	tBasicString( const T* s, unsigned int n )
		: basic_string( s, n )
	{
	}

	tBasicString( const T* first, const T* last )
		: basic_string( first, last )
	{
	}

	tBasicString( unsigned int n, T c )
		: basic_string( n, c )
	{
	}

	explicit tBasicString( unsigned int n )
		: basic_string( _Reserve_t(), n )
	{
	}

	void Clear()
	{
		clear();
	}

	void Reserve( unsigned int n )
	{
		reserve( n );
	}

	void PushBack( T c )
	{
		push_back( c );
	}

	void PopBack()
	{
		if( empty() == false )
		{
			erase( _M_finish, _M_finish );
		}
	}

	void Resize( unsigned int n )
	{
		resize( n );
	}

	void Resize( unsigned int n, T c )
	{
		resize( n, c );
	}

	void Append( cIterator first, cIterator last )
	{
		append( first, last );
	}

	void Append( const cSelf& s )
	{
		append( s );
	}

	bool Append( const cSelf& s, unsigned int pos, unsigned int n )
	{
		if( pos > s.size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			return append( s._M_Start() + pos, s._M_Start() + pos + (std::min)(n, s.size() - pos) );
		}
	}

	void Append( const T* s )
	{
		append( s );
	}

	void Append( const T* s, unsigned int n )
	{
		append( s, n );
	}

	void Append( T c )
	{
		push_back( c );
	}

	bool Insert( unsigned int pos, const cSelf& s )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			insert( begin() + pos, s._M_Start(), s._M_finish );
			return true;
		}
	}

	bool Insert( unsigned int pos, const cSelf& s, unsigned int beg, unsigned int n )
	{
		if( pos > size() || beg > s.size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			insert( begin() + pos, s._M_Start() + beg, s._M_Start() + beg + (std::min)(n, s.size() - beg) );
			return true;
		}
	}

	bool Insert( unsigned int pos, const T* s, unsigned int n )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			insert( begin() + pos, s, s + n );
			return true;
		}
	}

	bool Insert( unsigned int pos, const T* s )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			insert( this->_M_Start() + pos, s, s + TRAITS::length(s) );
			return false;
		}
	}

	void Erase( cIterator first, cIterator last )
	{
		erase( first, last );
	}

	bool Erase( unsigned int pos = 0, unsigned int n = NPOS )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			erase( begin() + pos, begin() + pos + (std::min)(n, size() - pos) );
			return true;
		}
	}

	void TrimLeft()
	{
		cIterator i = _M_Start();

		for(  ; i != _M_finish; ++i )
		{
			if( TRAITS::IsNotSpace(*i) )
			{
				break;
			}
		}
		erase( _M_Start(), i );
	}

	void TrimLeft( const T* s )
	{
		assert( s );
		erase( 0, find_first_not_of( s ) );
	}

	void TrimLeft( T c )
	{
		erase( 0, find_first_not_of(c) );
	}

	void TrimRight()
	{
		cIterator i = _M_finish;

		for( ; i >= _M_Start(); --i )
		{
			if( TRAITS::IsNotSpace(*i) )
			{
				break;
			}
		}
		erase( i + 1, _M_finish );
	}

	void TrimRight( const T* s )
	{
		assert( s );
		unsigned int pos = find_last_not_of( s );
		erase( pos != NPOS ? ++pos : 0 );
	}

	void TrimRight( T c )
	{
		unsigned int pos = find_last_not_of( c );
		erase( pos != NPOS ? ++pos: 0 );
	}

	void Replace( cIterator first0, cIterator last0, cIterator first1, cIterator last1 )
	{
		replace( first0, last0, first1, last1 );
	}

	bool Replace( unsigned int pos, unsigned int n, const cSelf& s )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			replace( begin() + pos, begin() + pos + (std::min)(n, size() - pos), 
				s._M_Start(), s._M_finish );
			return true;
		}
	}

	bool Replace( unsigned int pos0, unsigned int n0, const cSelf& s, unsigned int pos1, unsigned int n1 )
	{
		if( pos0 > size() || pos1 > s.size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			replace( begin() + pos0, begin() + pos0 + (std::min)(n0, size() - pos0),
				s._M_Start() + pos1, s._M_Start() + pos1 + (std::min)(n1, s.size() - pos1) );
			return true;
		}
	}

	bool Replace( unsigned int pos, unsigned int n0, const T* s, unsigned int n1 )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			replace( begin() + pos, begin() + pos + (std::min)(n0, size() - pos),
				s, s + n1 );
			return true;
		}
	}

	bool Replace( unsigned int pos, unsigned int n, const T* s )
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return false;
		}
		else
		{
			replace(begin() + pos, begin() + pos + (std::min)(n, size() - pos),
				s, s + _Traits::length(s));
			return true;
		}
	}

	void Replace( cIterator first, cIterator last, const cSelf& s )
	{
		replace( first, last, s );
	}

	void Replace( cIterator first, cIterator last, const T* s, unsigned int n )
	{
		replace( first, last, s, n );
	}

	void Replace( cIterator first, cIterator last, const T* s )
	{
		replace( first, last, s );
	}

	void Replace( const cSelf& oldStr, const cSelf& newStr )
	{
		unsigned int pos = 0;
		unsigned int oldLen = oldStr.length();
		unsigned int newLen = newStr.length();

		while( (pos = Find(oldStr, pos)) != NPOS )
		{
			replace( _M_Start() + pos, _M_Start() + pos + oldLen, newStr );
			pos += newLen;
		}
	}

	void Replace( const T* oldStr, const T* newStr )
	{
		assert(oldStr && newStr);

		if( oldStr && newStr )
		{
			unsigned int pos = 0;
			unsigned int oldLen = TRAITS::length( oldStr );
			unsigned int newLen = TRAITS::length( newStr );

			while( (pos = find(oldStr, pos)) != NPOS )
			{
				replace( _M_Start() + pos, _M_Start() + pos + oldLen, newStr );
				pos += newLen;
			}
		}
	}

	void Replace( T oldChar, T newChar )
	{
		for( cIterator i = _M_Start(); i != _M_finish; ++i )
		{
			if( *i == oldChar )
			{
				*i = newChar;
			}
		}
	}

	unsigned int Find( const cSelf& s, unsigned int pos = 0 ) const
	{
		return find( s, pos );
	}

	unsigned int Find( const T* s, unsigned int pos = 0 ) const
	{
		return find( s, pos );
	}

	unsigned int Find( T c, unsigned int pos = 0 ) const
	{
		return find( c, pos );
	}

	unsigned int FindReverse( const cSelf& s, unsigned int pos = NPOS ) const
	{
		return rfind( s, pos );
	}

	unsigned int FindReverse( const T* s, unsigned int pos = NPOS ) const
	{
		return rfind( s, pos );
	}

	unsigned int FindReverse( T c, unsigned int pos = NPOS ) const
	{
		return rfind( c, pos );
	}

	unsigned int FindFirstof( const cSelf& s, unsigned int pos = 0 ) const
	{
		return find_first_of( s, pos );
	}

	unsigned int FindFirstof( const T* s, unsigned int pos = 0 ) const
	{
		return find_first_of( s, pos );
	}

	unsigned int FindLastof( const cSelf& s, unsigned int pos = NPOS ) const
	{
		return find_first_of( s, pos );
	}

	unsigned int FindLastof( const T* s, unsigned int pos = NPOS ) const
	{
		return find_first_of( s, pos );
	}

	unsigned int FindFirstNotof( const cSelf& s, unsigned int pos = 0 ) const
	{
		return find_first_not_of( s, pos );
	}

	unsigned int FindFirstNotof( const T* s, unsigned int pos = 0 ) const
	{
		return find_first_not_of( s, pos );
	}

	unsigned int FindFirstNotof( T c, unsigned int pos = 0 ) const
	{
		return find_first_not_of( c, pos );
	}

	unsigned int FindLastNotof( const cSelf& s, unsigned int pos = NPOS ) const
	{
		return find_last_not_of( s, pos );
	}

	unsigned int FindLastNotof( const T* s, unsigned int pos = NPOS ) const
	{
		return find_last_not_of( s, pos );
	}

	unsigned int FindLastNotof( T c, unsigned int pos = NPOS ) const
	{
		return find_last_not_of( c, pos );
	}

	cSelf SubStr( unsigned int pos = 0, unsigned int n = NPOS ) const
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return *this;
		}
		return tBasicString( _M_Start() + pos, _M_Start() + pos + (std::min)(n, size() - pos) );
	}

	cSelf Mid( unsigned int pos ) const
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return *this;
		}
		return tBasicString( *this, pos );
	}

	cSelf Mid( unsigned int pos, unsigned int count ) const
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return *this;
		}
		return tBasicString( *this, pos, count );
	}

	cSelf Left( unsigned int count ) const
	{
		return tBasicString( *this, 0, count );
	}

	cSelf Right( unsigned int count ) const
	{
		unsigned int len = size();
		return tBasicString( *this, count < len ? len - count : 0, count );
	}

	int Compare( const cSelf& s ) const
	{
		return compare( s );
	}

	int Compare( const cSelf& s, unsigned int pos, unsigned int len ) const
	{
		const T* p0 = _M_Start() + pos;
		const T* p1 = s._M_Start() + pos;

		while( *p0 && *p0 == *p1 && --len > 0 )
		{
			++p0;
			++p1;
		}
		return *p0 - *p1;
	}

	int Compare( unsigned int pos, unsigned int n, const cSelf& s ) const
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return -1;
		}
		return _M_compare( _M_Start() + pos, _M_Start() + pos + (std::min)(n, size() - pos),
			s._M_Start(), s._M_finish );
	}

	int Compare( unsigned int pos0, unsigned int n0, const cSelf& s, unsigned int pos1, unsigned int n1 ) const
	{
		if( pos0 > size() || pos1 > s.size() )
		{
			assert( 0 && "index out of range!" );
			return -1;
		}
		return _M_compare( _M_Start() + pos0, _M_Start() + pos0 + (std::min)(n0, size() - pos0),
			s._M_Start() + pos1, s._M_Start() + pos1 + (std::min)(n1, s.size() - pos1) );
	}

	int Compare( const T* s ) const
	{
		return _M_compare(_M_Start(), _M_finish, s, s + TRAITS::length(s));
	}

	int Compare( const T* s, unsigned int pos, unsigned int len ) const
	{
		const T* p0 = _M_Start() + pos;
		s += pos;

		while( *p0 && *p0 == *s && --len > 0 )
		{
			++p0;
			++s;
		}
		return *p0 - *s;
	}

	int Compare( unsigned int pos, unsigned int n, const T* s ) const
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return -1;
		}
		return _M_compare(_M_Start() + pos, _M_Start() + pos + (std::min)(n, size() - pos), s, s + TRAITS::length(s));
	}

	int Compare( unsigned int pos, unsigned int n, const T* s, unsigned int n1 ) const
	{
		if( pos > size() )
		{
			assert( 0 && "index out of range!" );
			return -1;
		}
		return _M_compare( _M_Start() + pos, _M_Start() + pos + (std::min)(n, size() - pos), s, s + n1 );
	}

	int CompareNoCase( const cSelf& s ) const
	{
		const T* p0 = _M_Start();
		const T* p1 = s._M_Start();
		T c0, c1;

		do
		{
			c0 = TRAITS::ToUpper( *p0 );
			c1 = TRAITS::ToUpper( *p1 );
			++p0;
			++p1;
		}
		while( c0 && c0 == c1 );
		return c0 - c1;
	}

	int CompareNoCase( const cSelf& str, unsigned int pos, unsigned int len ) const
	{
		const T* p0 = _M_Start() + pos;
		const T* p1 = str._M_Start() + pos;
		T c0, c1;

		do
		{
			c0 = TRAITS::ToUpper( *p0 );
			c1 = TRAITS::ToUpper( *p1 );
			++p0;
			++p1;
		}
		while( c0 && c0 == c1 && --len > 0 );
		return c0 - c1;
	}

	int CompareNoCase( const T* str ) const
	{
		const T* p0 = _M_Start();
		T c0, c1;

		do
		{
			c0 = TRAITS::ToUpper( *p0 );
			c1 = TRAITS::ToUpper( *str );
			++p0;
			++str;
		}
		while( c0 && c0 == c1 );
		return c0 - c1;
	}

	int CompareNoCase( const T* str, unsigned int pos, unsigned int len ) const
	{
		const T* p0 = _M_Start() + pos;
		str += pos;
		T c0, c1;

		do
		{
			c0 = TRAITS::ToUpper( *p0 );
			c1 = TRAITS::ToUpper( *str );
			++p0;
			++str;
		}
		while( c0 && c0 == c1 && --len > 0 );
		return c0 - c1;
	}

	void Format( const T* fmt, ... )
	{
		assert( fmt );

		T buffer[BUFFER_SIZE];
		va_list vl;
		va_start( vl, fmt );
		TRAITS::_vsnprintf( buffer, BUFFER_SIZE - 1, fmt, vl );
		va_end( vl );
		buffer[BUFFER_SIZE - 1] = 0;
		*this = buffer;
	}

	void FormatV( const T* fmt, va_list vl )
	{
		assert( fmt );

		T buffer[BUFFER_SIZE];
		TRAITS::_vsnprintf( buffer, BUFFER_SIZE - 1, fmt, vl );
		buffer[BUFFER_SIZE - 1] = 0;
		*this = buffer;
	}

	void ToUpper()
	{
		TRAITS::ToUpper( _M_Start() );
	}

	void ToLower()
	{
		TRAITS::ToLower( _M_Start() );
	}

	int ToInt() const
	{
		return TRAITS::ToInt( _M_Start() );
	}

	int ToFixInt() const
	{
		return TRAITS::ToFixInt( _M_Start() );
	}

	float ToFloat() const
	{
		return TRAITS::ToFloat( _M_Start() );
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

	bool IsEmpty() const
	{
		return empty();
	}

	operator unsigned int() const
	{
		return TRAITS::GetHashCode( _M_Start() );
	}

	unsigned int GetHashCode() const
	{
		return TRAITS::GetHashCode( _M_Start() );
	}

	unsigned int GetSize() const
	{
		return size();
	}

	unsigned int GetLength() const
	{
		return length();
	}

	unsigned int GetCapacity() const
	{
		return capacity();
	}

	const T* Cstr() const
	{
		return c_str();
	}

	cSelf& operator = ( const cSelf& s )
	{
		if( &s != this )
		{
			assign( s._M_Start(), s._M_finish );
		}
		return *this;
	}

	cSelf& operator = ( const T* s )
	{
		assert( s );
		assign( s );
		return *this;
	}

	cSelf& operator = ( T c )
	{
		assign( 1, c );
		return *this;
	}

	cSelf& operator += ( const cSelf& s )
	{
		append( s );
		return *this;
	}

	cSelf& operator += ( const T* s )
	{
		append( s );
		return *this;
	}

	cSelf& operator += ( T c )
	{
		push_back( c );
		return *this;
	}

	T& operator [] ( unsigned int i )
	{
		return *(_M_Start() + i);
	}

	const T& operator [] ( unsigned int i ) const
	{
		return *(_M_Start() + i);
	}
