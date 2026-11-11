#include "stdafx.h"
#include "NaviField.h"



/// 정점당 미터
static const unsigned int METERS_PER_VERTEX = 1;

/// 미터당 단위
static const unsigned int UNITS_PER_METER = 100;

/// 정점당 단위
static const float UNITS_PER_VERTEX = 100;

///
static const char* gpNaviFieldFileCode = "IrisNaviField";

cNaviField::cNaviField()
: mCellCount( 0 )
, mLineCount( 0 )
, mpValues( 0 )
, mValues( 0 )
, mpHeights( 0 )
, mHeights( 0 )
{
}

cNaviField::~cNaviField()
{
	Clear();
}

void cNaviField::Clear()
{
	delete [] mpValues;
	mpValues = 0;
	delete [] mValues;
	mValues = 0;
	delete [] mpHeights;
	mpHeights = 0;
	delete [] mHeights;
	mHeights = 0;
}

bool cNaviField::CheckCellCount( unsigned int cellCount )
{
	if( cellCount % 512 == 0 )
	{
		return true;
	}
	return false;
}

bool cNaviField::Load( const cString& pathName )
{
	Clear();

	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to open file to load navifield" );
		return false;
	}

	/// 파일 헤더를 로딩
	cNaviFieldFileHeader header;

	if( loader.Read( &header, sizeof(cNaviFieldFileHeader) ) != sizeof(cNaviFieldFileHeader) )
	{
		assert( 0 && "failed to read navi file header" );
		return false;
	}

	if( ::memcmp( header.mCode, gpNaviFieldFileCode, 14 ) != 0 )
	{
		assert( 0 && "invalid file type" );
		return false;
	}

	switch( header.mVersion )
	{
	case 2:
	case 3:
		--header.mCellCount;
		break;
	case 4:
		break;
	default:
		assert( 0 && "invalid file version" );
		return false;
	}

	/// 크기 및 단위 설정
	if( CheckCellCount( header.mCellCount ) == false )
	{
		assert( 0 && "invalid grid size" );
		return false;
	}

	/// 단위 설정
	mCellCount = header.mCellCount;
	mLineCount = mCellCount + 1;


	/// 필드값 배열을 생성 및 로딩
	unsigned int numVerts = mLineCount * mLineCount;
	mValues = new unsigned char[numVerts];
	mpValues = new unsigned char*[mLineCount];

	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mpValues[i] = &mValues[i * mLineCount];
	}

	if( loader.Read( mValues, sizeof(unsigned char) * numVerts ) != sizeof(unsigned char) * numVerts )
	{
		assert( 0 && "failed to read flag array" );
		return false;
	}

	/// 높이값 배열을 로딩
	mHeights = new float[numVerts];
	mpHeights = new float*[mLineCount];

	for( unsigned int i = 0; i < mLineCount; ++i )
	{
		mpHeights[i] = &mHeights[i * mLineCount];
	}

	if( loader.Read( mHeights, sizeof(float) * numVerts ) != sizeof(float) * numVerts )
	{
		assert( 0 && "failed to read height array" );
		return false;
	}
	return true;
}

unsigned char cNaviField::GetValue( unsigned int xi, unsigned int yi ) const
{
	assert( mpValues );

	if( xi < mLineCount && yi < mLineCount )
	{
		return mpValues[yi][xi];
	}
	else
	{
		assert( 0 && "index out of range" );
		return 0;
	}
}

bool cNaviField::CalcHeight( float* height, float x, float y ) const
{
	assert( height );

	/// 범위를 벗어나면 바로 리턴
	int xi = (int)(x / UNITS_PER_VERTEX);
	int yi = (int)(y / UNITS_PER_VERTEX);

	if( xi < 0 )
		return false;
	if( yi < 0 )
		return false;
	if( xi >= (int)mLineCount )
		return false;
	if(	yi >= (int)mLineCount )
		return false;

	/// 기울기를 검사
	float x0 = xi * UNITS_PER_VERTEX;
	float y0 = yi * UNITS_PER_VERTEX;
	float d = 1.0f;

	if( x != x0 )
	{
		d = (y - y0) / (x - x0);
		assert( d >= 0.0f );
	}

	if( d >= 1.0f )
	{
		/// 왼쪽 위 삼각형
		NiPoint3 p0( x0, y0 + 100.0f, mHeights[(yi+1) * mLineCount + xi] );
		NiPoint3 p1( x0 + 100.0f, y0 + 100.0f, mHeights[(yi+1) * mLineCount + xi+1] );
		NiPoint3 p2( x0, y0, mHeights[yi * mLineCount + xi] );

		*height = p0.z + ((p1.z - p0.z) / (p1.x - p0.x)) * (x - p0.x) + ((p2.z - p0.z) / (p0.y - p2.y)) * (p0.y - y);
	}
	else
	{
		/// 오른쪽 아래 삼각형
		NiPoint3 p0( x0 + 100.0f, y0, mHeights[yi * mLineCount + xi+1] );
		NiPoint3 p1( x0, y0, mHeights[yi * mLineCount + xi] );
		NiPoint3 p2( x0 + 100.0f, y0 + 100.0f, mHeights[mLineCount * (yi+1) + xi+1] );

		*height = p0.z + ((p1.z - p0.z) / (p0.x - p1.x)) * (p0.x - x) + ((p2.z - p0.z) / (p2.y - p0.y)) * (y - p0.y);
	}
	return true;
}

bool cNaviField::GetHeight( float* height, unsigned int xi, unsigned int yi ) const
{
	assert( height );
	assert( mpHeights );

	if( xi < mLineCount && yi < mLineCount )
	{
		*height = mpHeights[yi][xi];
		return true;
	}
	else
	{
		assert( 0 && "index out of range" );
		return false;
	}
}

bool cNaviField::GetHeight( float* height, const NiPoint3& pos ) const
{
	assert( height );
	assert( mpHeights );

	/// 범위를 벗어나면 바로 리턴
	int xi = (int)(pos.x / UNITS_PER_VERTEX);
	int yi = (int)(pos.y / UNITS_PER_VERTEX);

	if( xi < 0 )
		return false;
	if( yi < 0 )
		return false;
	if( xi >= (int)mLineCount )
		return false;
	if(	yi >= (int)mLineCount )
		return false;

	*height = mpHeights[yi][xi];
	return true;
}
