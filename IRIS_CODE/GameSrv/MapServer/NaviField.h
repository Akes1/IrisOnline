/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.03
* 내    용 : 네비필드
* 주의사항 : 
*===========================================================================*/
#pragma once

/// 네비필드 상수
const unsigned int NAVIFIELD_DEFAULT_RESOLUTION = 512;
const unsigned int NAVIFIELD_BUFF_CELL_COUNT = 128;
const unsigned int NAVIFIELD_BUFF_LINE_COUNT = NAVIFIELD_BUFF_CELL_COUNT + 1;
const unsigned int NAVIFIELD_BUFF_VERT_COUNT = NAVIFIELD_BUFF_LINE_COUNT * NAVIFIELD_BUFF_LINE_COUNT;
const unsigned int NAVIFIELD_LEAF_CELL_COUNT = 8;
const unsigned int NAVIFIELD_LEAF_LINE_COUNT = NAVIFIELD_LEAF_CELL_COUNT + 1;
const unsigned int NAVIFIELD_LEAF_LINE_COUNT_X2 = NAVIFIELD_LEAF_LINE_COUNT * 2;
const unsigned int NAVIFIELD_LEAF_VERT_COUNT = NAVIFIELD_LEAF_LINE_COUNT * NAVIFIELD_LEAF_LINE_COUNT;

/// 네비필드 파일 헤더
#pragma pack( push, 1 )
class cNaviFieldFileHeader
{
public:
	/// 식별 코드
	char mCode[14];

	/// 버전
	unsigned int mVersion;

	/// 그리드 크기 (셀 수)
	unsigned int mCellCount;

	/// 정점당 미터 수
	float mMetersPerVertex;

	/// 미터당 단위 수 ( 100 )
	unsigned int mUnitsPerMeter;
};
#pragma pack( pop )

/// 네비필드
class cNaviField
{
public:
	cNaviField();
	~cNaviField();

	/// 지움
	void Clear();

	/// 로딩
	bool Load( const cString& pathName );

	/// 필드값
	void SetValue( unsigned int xi, unsigned int yi, unsigned char value );
	unsigned char GetValue( unsigned int xi, unsigned int yi ) const;

	/// 높이값
	bool CalcHeight( float* height, float x, float y ) const;
	bool GetHeight( float* height, unsigned int xi, unsigned int yi ) const;
	bool GetHeight( float* height, const NiPoint3& pos ) const;

	/// 그리드 크기
	unsigned int GetCellCount() const;

private:
	/// 그리드 크기를 검사
	bool CheckCellCount( unsigned int cellCount );

private:
	/// 그리드 크기
	unsigned int mCellCount;
	unsigned int mLineCount;

	/// 필드값 배열
	unsigned char** mpValues;
	unsigned char* mValues;

	/// 높이값 배열
	float** mpHeights;
	float* mHeights;
};

inline
void cNaviField::SetValue( unsigned int xi, unsigned int yi, unsigned char value )
{
	assert( mValues );

	unsigned int i = yi * mLineCount + xi;
	mValues[i] = value;
}

inline
unsigned int cNaviField::GetCellCount() const
{
	return mCellCount;
}
