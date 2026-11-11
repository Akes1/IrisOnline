/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.11
* 내    용 : 높이맵
* 주의사항 : 
*===========================================================================*/
#pragma once

class cFileLoader;

/// 높이맵
class cHeightMap
{
public:
	cHeightMap();
	cHeightMap( unsigned int width, unsigned int height );

	/// 지움
	void Clear();

	/// 로딩
	bool Load( const cString& pathName );

	/// 저장
	bool Save( const cString& pathName );

	/// 높이값
	bool SetValue( unsigned int x, unsigned int y, float height );
	bool GetValue( float* height, unsigned int x, unsigned int y );

	/// 주의: x와 y는 [0, 1]사이의 값을 가져야한다.
	bool GetValue( float* height , float x, float y );

	/// 너비
	unsigned int GetWidth() const;

	/// 높이
	unsigned int GetHeight() const;

protected:
	/// 너비
	unsigned int mWidth;

	/// 높이
	unsigned int mHeight;

	/// 버퍼
	typedef tArray<float> cBuffer;
	cBuffer mBuffer;
};

inline
unsigned int cHeightMap::GetWidth() const
{
	return mWidth;
}

inline
unsigned int cHeightMap::GetHeight() const
{
	return mHeight;
}
