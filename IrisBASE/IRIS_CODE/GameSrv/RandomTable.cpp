// Include
#include "gamesrv.h"
#include "RandomTable.h"
#include "logprotocol.h"

#include <stdlib.h>
#include <time.h>

// Local definitions
#pragma warning( disable: 4244 )
#pragma warning( disable: 4127 )

// Global data
cRandomTable* cRandomTable::mpRandomTable = NULL;

// cRandomTable Constructor
cRandomTable::cRandomTable(void)
{
	mpRandomTable = this;

	m_million = NULL;
	m_mOffset = 0;
	m_mLength = 0;

	m_hundred = NULL;
	m_hOffset = 0;
	m_hLength = 0;
}

// ~cRandomTable Destructor
cRandomTable::~cRandomTable(void)
{
	Shutdown( );

	mpRandomTable = NULL;
}

// Initialize Method - 테이블 헤더와 최대크기 검사 필요함.
bool cRandomTable::Initialize( )
{
	FILE*  stream;
	size_t numread;

	bool   retMillion = false;
	bool   retHundred = false;

	// rand function 시드값 초기화.
	srand( (unsigned)time( NULL ) );

	stream = fopen( "./Script/Resource/rand_million.", "rb" );
	numread  = 0;
	if ( stream != NULL )
	{
		WORD type;

		// 헤더정보 확인
		fread( &type, sizeof(type), 1, stream );

		if ( type == ((WORD) ('T' << 8) | 'R') )
		{
			// 테이블이 NULL 이 아닐결우 해제 및 초기화.
			if ( m_million != NULL )
			{
				GlobalFree( m_million );
				m_million = NULL;
			}
			// 테이블 오프셋 초기화.
			m_mOffset = 0;
			// 테이블 최대 크기 가져오기.
			fread( &m_mLength, sizeof(m_mLength), 1, stream );

			// 테이블 메모리 생성.
			m_million = (double*)GlobalAlloc( GPTR, sizeof(double)*m_mLength );

			if ( m_million != NULL )
			{
				// 테이블 가져오기 & 최대 크기와 비교.
				numread = fread( m_million, sizeof(double), m_mLength, stream );
				if ( numread != m_mLength )
				{
					GlobalFree( m_million );
					m_million = NULL;
				}
				else
				{
					// 테이블 오프셋 램덤설정.
					m_mOffset  = rand( ) % m_mLength;
					retMillion = true;
				}
			}
		}

		fclose( stream );
	}

	stream = fopen( "./Script/Resource/rand_hundred.", "rb" );
	numread  = 0;
	if ( stream != NULL )
	{
		WORD type;

		// 헤더정보 확인
		fread( &type, sizeof(type), 1, stream );

		if ( type == ((WORD) ('T' << 8) | 'R') )
		{
			// 테이블이 NULL 이 아닐결우 해제 및 초기화.
			if ( m_hundred != NULL )
			{
				GlobalFree( m_hundred );
				m_hundred = NULL;
			}
			// 테이블 오프셋 초기화.
			m_hOffset = 0;
			// 테이블 최대 크기 가져오기.
			fread( &m_hLength, sizeof(m_hLength), 1, stream );

			// 테이블 메모리 생성.
			m_hundred = (double*)GlobalAlloc( GPTR, sizeof(double)*m_hLength );

			if ( m_hundred != NULL )
			{
				// 테이블 가져오기 & 최대 크기와 비교.
				numread = fread( m_hundred, sizeof(double), m_hLength, stream );
				if ( numread != m_hLength )
				{
					GlobalFree( m_hundred );
					m_hundred = NULL;
				}
				else
				{
					// 테이블 오프셋 램덤설정.
					m_hOffset  = rand( ) % m_hLength;
					retHundred = true;
				}
			}
		}

		fclose( stream );
	}

	return (retMillion && retHundred);
}

// Shutdown Method
void cRandomTable::Shutdown( )
{
	if ( m_million != NULL )
	{
		GlobalFree( m_million );
		m_million = NULL;
	}
	m_mOffset = 0;
	m_mLength = 0;

	if ( m_hundred != NULL )
	{
		GlobalFree( m_hundred );
		m_hundred = NULL;
	}
	m_hOffset = 0;
	m_hLength = 0;
}

// GetSeed Method
DWORD cRandomTable::GetSeed( )
{
	return rand( ) % m_mLength;
}

// Get Method
double cRandomTable::Get( )
{
	double value = 0;
	if ( m_million != NULL && m_mOffset < m_mLength )
	{
		value = *(m_million + m_mOffset);

		if ( ++m_mOffset >= m_mLength )
			m_mOffset = 0;
	}
	return value;
}

// Get Method
double cRandomTable::Get(DWORD& seed)
{
	double value = 0;
	if ( m_million != NULL )
	{
		// 최대 값보다 크거나 같을경우.
		if ( seed >= m_mLength )
			seed %= m_mLength;

		value = *(m_million + seed);

		// 최대 값보다 커진경우..
		if ( ++seed >= m_mLength )
			seed = 0;
	}
	return value;
}

// GetSeedx100 Method
DWORD cRandomTable::GetSeedx100( )
{
	return rand( ) % m_hLength;
}

// Getx100 Method
double cRandomTable::Getx100( )
{
	double value = 0;
	if ( m_hundred != NULL && m_hOffset < m_hLength )
	{
		value = *(m_hundred + m_hOffset);

		if ( ++m_hOffset >= m_hLength )
			m_hOffset = 0;
	}
	return value;
}

// Getx100 Method
double cRandomTable::Getx100(DWORD& seed)
{
	double value = 0;
	if ( m_hundred != NULL )
	{
		// 최대 값보다 크거나 같을경우.
		if ( seed >= m_hLength )
			seed %= m_hLength;

		value = *(m_hundred + seed);

		// 최대 값보다 커진경우..
		if ( ++seed >= m_hLength )
			seed = 0;
	}
	return value;
}
