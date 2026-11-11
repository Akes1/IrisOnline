/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2008.05.15
* 내    용 : 
* 주의사항 : 
*===========================================================================*/
#pragma once

#pragma warning( push )
#pragma warning( disable: 4800 )

/// 비트 배열
class cBitArray
{
	static const unsigned int BITSIZE = sizeof(unsigned int) * 8;

public:
	unsigned int mWidth;
	unsigned int mHeight;

protected:
	unsigned int* mArray;
	unsigned int mSize;

public:
	cBitArray( unsigned int size );
	~cBitArray();

	void Clear();

	void Resize( unsigned int size );

	void Set( unsigned int index, bool value );
	void SetAll( bool value );

	unsigned int GetSize() const;

	bool operator[] ( unsigned int index );
};

inline
unsigned int cBitArray::GetSize() const
{
	return mSize * BITSIZE;
}

inline
bool cBitArray::operator[] ( unsigned int index )
{
	unsigned int cell = index / BITSIZE;
	unsigned int bit = index % BITSIZE;
	assert( cell < mSize );

	return (bool)((mArray[cell] & (1 << bit)) >> bit);
}

#pragma warning( pop )
