/* ==========================================================================
* 작 성 자 : 이순규
* 작 성 일 : 2007.01.11
* 내    용 : 타가 비트맵 이미지
* 주의사항 : 
*===========================================================================*/
#pragma once

class cFileLoader;

/// 타가 비트맵 이미지
class cTargaImage
{
public:
	cTargaImage();

	/// 지움
	void Clear();

	/// 로딩
	bool Load( const cString& pathName );
	bool Load( cFileLoader& loader );

	/// 저장
	bool Save( const cString& pathName );

	/// 크기 설정
	void Resize( unsigned int width, unsigned int height, unsigned short bytesPerPixel );

	/// 색상 바이트 순서를 바꿈 ( RGB <-> BGR )
	void ChangeColorByteOrder();

	/// 픽셀값을 설정
	bool SetPixel( unsigned int x, unsigned int y, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 1.0f );

	/// 너비( 픽셀 수 ) 리턴
	unsigned int GetWidth() const;

	/// 높이( 픽셀 수 ) 리턴
	unsigned int GetHeight() const;

	/// 픽셀당 바이트 수 리턴
	unsigned short GetBytesPerPixel() const;

	/// 크기( 바이트 수 ) 리턴
	unsigned int GetSize() const;

	/// 픽셀값을 리턴
	bool GetPixel( unsigned char* red, unsigned char* green, unsigned char* blue, unsigned int x, unsigned int y );
	bool GetPixel( unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* alpha, unsigned int x, unsigned int y );

	/// 주의: x와 y는 [0, 1]사이의 값을 가져야한다.
	bool GetPixel( unsigned char* red, unsigned char* green, unsigned char* blue, float x, float y );

	/// 버퍼 리턴
	const tArray<unsigned char>& GetBuffer() const;

	/// 버퍼 포인터 리턴
	const unsigned char* GetBufferPtr() const;

protected:
	/// 너비
	unsigned int mWidth;

	/// 높이
	unsigned int mHeight;

	/// 픽셀 당 바이트 수
	unsigned short mBytesPerPixel;

	/// 이미지 크기 ( mWidth * mHeight * mBytesPerPixel )
	unsigned int mSize;

	/// 버퍼
	typedef tArray<unsigned char> cBuffer;
	cBuffer mBuffer;
};

inline
unsigned int cTargaImage::GetWidth() const
{
	return mWidth;
}

inline
unsigned int cTargaImage::GetHeight() const
{
	return mHeight;
}

inline
unsigned short cTargaImage::GetBytesPerPixel() const
{
	return mBytesPerPixel;
}

inline
unsigned int cTargaImage::GetSize() const
{
	return mSize;
}

inline
const tArray<unsigned char>& cTargaImage::GetBuffer() const
{
	return mBuffer;
}

inline
const unsigned char* cTargaImage::GetBufferPtr() const
{
	return &mBuffer[0];
}
