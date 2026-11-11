/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.06.21
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 토크나이저
/// 렉서와 달리 단순히 구분 문자로 토큰을 분리해준다.
template<class T>
class tTokenizer
{
protected:
	/// 파일명
	const T* mFileName;

	/// 버퍼 포인터
	const T* mBuffer;

	/// 버퍼 길이
	unsigned int mBufferLen;

	/// 구분 문자열 (구분 문자들의 집합)
	const T* mDelimiters;

	/// 문자를 읽을 위치
	unsigned int mPos;
public:
	/// 줄 번호
	int mLine;

public:
	tTokenizer();
	tTokenizer( const T* buffer, unsigned int size, const T* delimiters, const T* fileName = 0 );
	~tTokenizer();

	/// 소스를 설정
	void SetSource( const T* buffer, unsigned int size, const T* delimiters, const T* fileName = 0 );

	/// 구분 문자들을 설정
	void SetDelimiters( const T* delimiters );

	/// 다음 토큰 문자열을 리턴
	/// 맨 끝에 도달하면 false를 리턴
	bool GetNext( tBasicString<T, tCharTraits<T> >* str );

	/// 토큰화시키지 않은 나머지 문자열을 리턴
	const T* GetTail() const;

	/// 종료 여부를 리턴
	bool IsEnd();
};

template<>
class tTokenizer<char>
{
protected:
	/// 파일명
	const char* mFileName;

	/// 버퍼 포인터
	const char* mBuffer;

	/// 버퍼 길이
	unsigned int mBufferLen;

	/// 구분 문자열
	bool mDelimiters[256];

	/// 문자를 읽을 위치
	unsigned int mPos;
public:
	/// 줄 번호
	int mLine;

protected:
	bool IsComment( char c )
	{
		char c1 = *( mBuffer + mPos + 1 );
		return (c == '/' && c1 == '/') || (c == '-' && c1 == '-');
	}

	bool IsDelimiter( char c )
	{
		return mDelimiters[c + 128];
	}

public:
	tTokenizer()
		: mFileName( 0 )
		, mBuffer( 0 )
		, mBufferLen( 0 )
		, mPos( 0 )
		, mLine( 1 )
	{
		memset( mDelimiters, false, 256 * sizeof(bool) );
	}

	tTokenizer( const char* buffer, unsigned int size, const char* delimiters, const char* fileName = 0 )
		: mFileName( fileName )
		, mBuffer( buffer )
		, mBufferLen( size )
		, mPos( 0 )
		, mLine( 1 )
	{
		assert( buffer );
		assert( size );

		SetDelimiters( delimiters );
	}

	~tTokenizer()
	{
	}

	void SetSource( const char* buffer, unsigned int size, const char* delimiters, const char* fileName = 0 )
	{
		assert( buffer );
		assert( size );

		mFileName = fileName;
		mBuffer = buffer;
		mBufferLen = size;
		SetDelimiters( delimiters );
	}

	void SetDelimiters( const char* delimiters )
	{
		assert( delimiters );

		memset( mDelimiters, false, 256 * sizeof(bool) );

		for( char c = *delimiters; c; c = *(++delimiters) )
		{
			mDelimiters[c + 128] = true;
		}
	}

	bool GetNext( cString* str )
	{
		assert( str );

		if( mPos >= mBufferLen )
			return false;

		char lexeme[256] = { 0 };
		int i = 0;
		char c = 0;

		for( ; mPos < mBufferLen; ++mPos )
		{
			c = *( mBuffer + mPos );

			if( c == '\n' )
			{
				++mLine;
			}
			else if( IsComment( c ) )
			{
				for( mPos += 2; mPos < mBufferLen; ++mPos )
				{
					c = *( mBuffer + mPos );

					if( c == '\n' )
					{
						++mLine;
						break;
					}
				}
				if( i == 0 )
				{
					continue;
				}
				else
				{
					++mPos;
					break;
				}
			}
			if( IsDelimiter( c ) )
			{
				if( i == 0 )
				{
					continue;
				}
				else
				{
					++mPos;
					break;
				}
			}
			if( i < 255 )
			{
				lexeme[i] = c;
				++i;
			}
		}

		lexeme[i] = 0;
		*str = lexeme;
		return i != 0;
	}

	void GotoNextLine()
	{
		for( ; mPos < mBufferLen; ++mPos )
		{
			if( *( mBuffer + mPos ) == '\n' )
			{
				++mLine;
				++mPos;
				break;
			}
		}
	}

	const char* GetTail() const
	{
		if( mPos >= mBufferLen )
			return 0;
		else
			return mBuffer + mPos;
	}

	bool IsEnd()
	{
		unsigned int pos = mPos;
		char c = 0;

		for( ; mPos < mBufferLen; ++mPos )
		{
			c = *( mBuffer + mPos );

			if( c == '\n' || c == ' ' )
			{
				continue;
			}
			if( IsComment( c ) )
			{
				for( mPos += 2; mPos < mBufferLen; ++mPos )
				{
					c = *( mBuffer + mPos );

					if( c == '\n' )
						break;
				}
				continue;
			}
			if( IsDelimiter( c ) )
			{
				continue;
			}
			mPos = pos;
			return false;
		}
		mPos = pos;
		return true;
	}
};

template<>
class tTokenizer<wchar_t>
{
protected:
	/// 파일명
	const wchar_t* mFileName;

	/// 버퍼 포인터
	const wchar_t* mBuffer;

	/// 버퍼 길이
	unsigned int mBufferLen;

	/// 구분 문자열
	const wchar_t* mDelimiters;

	/// 문자를 읽을 위치
	unsigned int mPos;

	/// 줄 번호
	int mLine;

protected:
	bool IsComment( wchar_t c )
	{
		wchar_t c1 = *( mBuffer + mPos + 1 );
		return (c == L'/' && c1 == L'/') || (c == L'-' && c1 == L'-');
	}

	bool IsDelimiter( wchar_t c )
	{
		for( const wchar_t* p = mDelimiters; *p; ++p )
		{
			if( c == *p )
			{
				return true;
			}
		}
		return false;
	}

public:
	tTokenizer()
		: mFileName( 0 )
		, mBuffer( 0 )
		, mBufferLen( 0 )
		, mDelimiters( 0 )
		, mPos( 0 )
		, mLine( 1 )
	{
	}

	tTokenizer( const wchar_t* buffer, unsigned int size, const wchar_t* delimiters, const wchar_t* fileName = 0 )
		: mFileName( fileName )
		, mBuffer( buffer )
		, mBufferLen( size )
		, mDelimiters( delimiters )
		, mPos( 0 )
		, mLine( 1 )
	{
		assert( buffer );
		assert( size );
		assert( delimiters );
	}

	~tTokenizer()
	{
	}

	void SetSource( const wchar_t* buffer, unsigned int size, const wchar_t* delimiters, const wchar_t* fileName = 0 )
	{
		assert( buffer );
		assert( size );
		assert( delimiters );

		mFileName = fileName;
		mBuffer = buffer;
		mBufferLen = size;
		mDelimiters = delimiters;
	}

	bool GetNext( cStringW* str )
	{
		assert( str );

		if( mPos >= mBufferLen )
			return false;

		wchar_t lexeme[256] = { 0 };
		int i = 0;
		wchar_t c = 0;

		for( ; mPos < mBufferLen; ++mPos )
		{
			c = *( mBuffer + mPos );

			if( c == L'\n' )
			{
				++mLine;
			}
			else if( IsComment( c ) )
			{
				for( mPos += 2; mPos < mBufferLen; ++mPos )
				{
					c = *( mBuffer + mPos );

					if( c == L'\n' )
					{
						++mLine;
						break;
					}
				}
				if( i == 0 )
				{
					continue;
				}
				else
				{
					++mPos;
					break;
				}
			}
			if( IsDelimiter( c ) )
			{
				if( i == 0 )
				{
					continue;
				}
				else
				{
					++mPos;
					break;
				}
			}
			if( i < 255 )
			{
				lexeme[i] = c;
				++i;
			}
		}

		lexeme[i] = 0;
		*str = lexeme;
		return i != 0;
	}

	void GotoNextLine()
	{
		for( ; mPos < mBufferLen; ++mPos )
		{
			if( *( mBuffer + mPos ) == L'\n' )
			{
				++mLine;
				++mPos;
				break;
			}
		}
	}

	const wchar_t* GetTail() const
	{
		if( mPos >= mBufferLen )
			return 0;
		else
			return mBuffer + mPos;
	}

	bool IsEnd()
	{
		unsigned int pos = mPos;
		wchar_t c = 0;

		for( ; mPos < mBufferLen; ++mPos )
		{
			c = *( mBuffer + mPos );

			if( c == L'\n' || c == L' ' )
			{
				continue;
			}
			if( IsComment( c ) )
			{
				for( mPos += 2; mPos < mBufferLen; ++mPos )
				{
					c = *( mBuffer + mPos );

					if( c == L'\n' )
						break;
				}
				continue;
			}
			if( IsDelimiter( c ) )
			{
				continue;
			}
			mPos = pos;
			return false;
		}
		mPos = pos;
		return true;
	}
};

typedef tTokenizer<char> cTokenizer;
typedef tTokenizer<wchar_t> cTokenizerW;

#ifdef _UNICODE
;	typedef	cTokenizerW cTokenizerT;
#else
;	typedef	cTokenizer cTokenizerT;	
#endif //_UNICODE
