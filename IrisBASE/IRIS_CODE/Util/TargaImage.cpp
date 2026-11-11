#include "stdafx.h"
#include "TargaImage.h"
#include "FileLoader.h"
#include "FileSaver.h"
#ifdef _REGENTOOL_
	#include "../GameSrv/ServerCommon/Console.h"
#else
	#include "Console.h"
#endif


const int TGA_RGB = 2;
const int TGA_GRAYSCALE = 3;
const int TGA_RLERGB = 10;
const int TGA_RLEGRAYSCALE = 11;

#pragma pack( push, 1 )
struct sHeader
{
	/// 식별 필드의 길이
	unsigned char mIDLength;

	/// 색상 맵의 종류 ( 항상 0 )
	unsigned char mColorMapType;

	/// 종류
	unsigned char mType;

	/// 색상 맵 정보
	unsigned char mChunk[5];

	/// 좌측 하단 x좌표
	unsigned short mXOrigin;

	/// 좌측 하단 y좌표
	unsigned short mYOrigin;

	/// 너비
	unsigned short mWidth;

	/// 높이
	unsigned short mHeight;

	/// 픽셀 당 비트 수 ( 8, 24, 32 )
	unsigned char mBitsPerPixel;

	/// 더미
	unsigned char mDesc;
};
#pragma pack( pop )

cTargaImage::cTargaImage()
{
	mWidth = 0;
	mHeight = 0;
	mBytesPerPixel = 0;
	mSize = 0;
}

void cTargaImage::Clear()
{
	mWidth = 0;
	mHeight = 0;
	mBytesPerPixel = 0;
	mSize = 0;
	mBuffer.Clear();
}

bool cTargaImage::Load( const cString& pathName )
{
	cFileLoader loader;
	if( loader.Open( pathName, true ) == false )
	{
		return false;
	}

	return Load( loader );
}

bool cTargaImage::Load( cFileLoader& loader )
{
	Clear();
	loader.Seek( 0 );

	/// 헤더
	sHeader header;
	if( loader.Read( &header, sizeof( header ) ) == 0 )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to read header" );
		return false;
	}

	/// 타입
	bool compressed;

	switch( header.mType )
	{
	case TGA_RGB:
	case TGA_GRAYSCALE:
		compressed = false;
		break;

	case TGA_RLERGB:
	case TGA_RLEGRAYSCALE:
		compressed = true;
		break;

	default:
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "unknown image type" );
		return false;
	}

	/// 픽셀당 비트 수
	switch( header.mBitsPerPixel )
	{
	case 8:
	case 24:
	case 32:
		break;
	default:
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "invalid bits per pixel" );
		return false;
	}

	/// 식별 필드
	if( header.mIDLength > 0 )
	{
		char id[256];
		if( loader.Read( id, header.mIDLength ) == 0 )
		{
			//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to read identification field" );
			return false;
		}
	}

	/// 이미지
	mWidth = header.mWidth;
	mHeight = header.mHeight;
	mBytesPerPixel = header.mBitsPerPixel >> 3;
	mSize = mWidth * mHeight * mBytesPerPixel;
	if( mSize <= 0 )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "image size is 0" );
		return false;
	}
	mBuffer.Resize( mSize );
	unsigned char* buffer = &mBuffer[0];

	if( compressed )
	{
		unsigned char rle;
		unsigned char color[4];
		unsigned int bufIndex = 0;

		while( bufIndex < mSize )
		{
			if( loader.Read( &rle, 1 ) == 0 )
			{
				//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to read image" );
				return false;
			}
			if( rle & 128 )
			{
				/// run-Length packet
				rle -= 127;
				if( loader.Read( &color, mBytesPerPixel ) == 0 )
				{
					//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to read image" );
					return false;
				}
				switch( mBytesPerPixel )
				{
				case 1:
					for( unsigned int i = 0; i < rle; ++i )
					{
						*buffer = color[0];
						++buffer;
					}
					bufIndex += rle;
					break;

				case 3:
					for( unsigned int i = 0; i < rle; ++i )
					{
						*buffer = color[2];
						*( buffer+1 ) = color[1];
						*( buffer+2 ) = color[0];
						buffer += 3;
					}
					bufIndex += ( rle*3 );
					break;

				case 4:
					for( unsigned int i = 0; i < rle; ++i )
					{
						*buffer = color[2];
						*( buffer+1 ) = color[1];
						*( buffer+2 ) = color[0];
						*( buffer+3 ) = color[3];
						buffer += 4;
					}
					bufIndex += ( rle*4 );
					break;
				}
			}
			else
			{
				/// raw packet
				++rle;
				unsigned int numBytes = rle * mBytesPerPixel;
				if( loader.Read( buffer, numBytes ) == 0 )
				{
					//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to read image" );
					return false;
				}

				switch( mBytesPerPixel )
				{
				case 3:
				case 4:
					/// BGR를 RGB로 바꿈
					for( unsigned int i = 0; i < numBytes; i += mBytesPerPixel )
					{
						*( buffer+i ) ^= *( buffer+i+2 ) ^= *( buffer+i ) ^= *( buffer+i+2 );
					}
				}
				buffer += numBytes;
				bufIndex += numBytes;
			}
		}
	}
	else
	{
		/// 압축이 안된 경우 통째로 읽어 들임
		if( loader.Read( buffer, mSize ) == 0 )
		{
			//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to read image" );
			return false;
		}

		switch( mBytesPerPixel )
		{
		case 3:
		case 4:
			/// BGR를 RGB로 바꿈
			for( unsigned int i = 0; i < mSize; i += mBytesPerPixel )
			{
				*( buffer+i ) ^= *( buffer+i+2 ) ^= *( buffer+i ) ^= *( buffer+i+2 );
			}
		}
	}
	return true;
}

bool cTargaImage::Save( const cString& pathName )
{
	/// 이미지 검사
	if( mBuffer.GetSize() <= 0 )
	{
		return false;
	}

	switch( mBytesPerPixel )
	{
	case 1:
	case 3:
	case 4:
		break;
	default:
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "bit depth must be 8 or 24 or 32" );
		return false;
	}

	/// 파일 열기
	cFileSaver saver;
	if( saver.Open( pathName ) == false )
	{
		return false;
	}

	/// 헤더
	sHeader header;
	header.mIDLength = 0;
	header.mColorMapType = 0;
	header.mType = mBytesPerPixel == 1 ? TGA_GRAYSCALE : TGA_RGB;
	header.mChunk[0] = 0;
	header.mChunk[1] = 0;
	header.mChunk[2] = 0;
	header.mChunk[3] = 0;
	header.mChunk[4] = 0;
	header.mXOrigin = 0;
	header.mYOrigin = 0;
	header.mWidth = ( unsigned short )mWidth;
	header.mHeight = ( unsigned short )mHeight;
	header.mBitsPerPixel = ( unsigned char )( mBytesPerPixel << 3 );
	header.mDesc = 0;

	if( saver.Write( &header, sizeof( sHeader ) ) != sizeof( sHeader ) )
	{
		//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to write header" );
		return false;
	}

	/// 이미지
	unsigned char* buffer = &mBuffer[0];

	if( mBytesPerPixel == 1 )
	{
		if( saver.Write( buffer, mSize ) != mSize )
		{
			//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to write image" );
			return false;
		}
	}
	else
	{
		unsigned int stride = mWidth * mBytesPerPixel;
		cBuffer row( stride );
		unsigned char* prow = &row[0];

		for( unsigned int i = 0; i < mHeight; ++i )
		{
			memcpy( prow, buffer, stride );
			buffer += stride;

			/// RGB를 BGR로 바꿈
			for( unsigned int i = 0; i < stride; i += mBytesPerPixel )
			{
				*( prow+i ) ^= *( prow+i+2 ) ^= *( prow+i ) ^= *( prow+i+2 );
			}

			if( saver.Write( prow, stride ) != stride )
			{
				//cConsole::Error( ERROR_FILE, ERROR_LINE, "failed to write image" );
				return false;
			}
		}
	}
	return true;
}

void cTargaImage::Resize( unsigned int width, unsigned int height, unsigned short bytesPerPixel )
{
	mWidth = width;
	mHeight = height;
	mBytesPerPixel = bytesPerPixel;
	mSize = width * height * bytesPerPixel;
	mBuffer.Resize( mSize );
}

void cTargaImage::ChangeColorByteOrder()
{
	unsigned char* buffer = &mBuffer[0];

	switch( mBytesPerPixel )
	{
	case 3:
	case 4:
		/// BGR를 RGB로 바꿈
		for( unsigned int i = 0; i < mSize; i += mBytesPerPixel )
		{
			*( buffer+i ) ^= *( buffer+i+2 ) ^= *( buffer+i ) ^= *( buffer+i+2 );
		}
	}
}

bool cTargaImage::SetPixel( unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a )
{
	if( x >= mWidth || y >= mHeight )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	switch( mBytesPerPixel )
	{
	case 1:
		{
			unsigned int i = x + mWidth * y;
			mBuffer[i] = r;
		}
		break;
	case 3:
		{
			unsigned int i = (x + mWidth * y) * mBytesPerPixel;
			mBuffer[i + 0] = r;
			mBuffer[i + 1] = g;
			mBuffer[i + 2] = b;
		}
		break;
	case 4:
		{
			unsigned int i = (x + mWidth * y) * mBytesPerPixel;
			mBuffer[i + 0] = r;
			mBuffer[i + 1] = g;
			mBuffer[i + 2] = b;
			mBuffer[i + 3] = a;
		}
		break;
	default:
		assert( 0 && "invalid bytes per pixel" );
		return false;
	}
	return true;
}

bool cTargaImage::GetPixel( unsigned char* r, unsigned char* g, unsigned char* b, unsigned int x, unsigned int y )
{
	if( x >= mWidth || y >= mHeight )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	switch( mBytesPerPixel )
	{
	case 1:
		{
			unsigned int i = x + mWidth * y;
			if( r )
				*r = mBuffer[i];
			if( g )
				*g = mBuffer[i];
			if( b )
				*b = mBuffer[i];
		}
		break;
	case 3:
	case 4:
		{
			unsigned int i = (x + mWidth * y) * mBytesPerPixel;
			if( r )
				*r = mBuffer[i + 0];
			if( g )
				*g = mBuffer[i + 1];
			if( b )
				*b = mBuffer[i + 2];
		}
		break;
	default:
		assert( 0 && "invalid bytes per pixel" );
		return false;
	}
	return true;
}

bool cTargaImage::GetPixel( unsigned char* r, unsigned char* g, unsigned char* b, unsigned char* a, unsigned int x, unsigned int y )
{
	if( x >= mWidth || y >= mHeight )
	{
		assert( 0 && "index out of range" );
		return false;
	}

	if( mBytesPerPixel == 4)
	{
		unsigned int i = (x + mWidth * y) * mBytesPerPixel;
		if( r )
			*r = mBuffer[i + 0];
		if( g )
			*g = mBuffer[i + 1];
		if( b )
			*b = mBuffer[i + 2];
		if( a )
			*a = mBuffer[i + 3];
	}
	else
	{
		assert( 0 && "invalid bytes per pixel" );
		return false;
	}
	return true;
}

bool cTargaImage::GetPixel( unsigned char* r, unsigned char* g, unsigned char* b, float x, float y )
{
	if( x < 0.0f || x > 1.0f )
	{
		assert( 0 && "ratio out of range" );
		return false;
	}

	if( y < 0.0f || y > 1.0f )
	{
		assert( 0 && "ratio out of range" );
		return false;
	}

	unsigned int xi = (unsigned int)(x * (mWidth-1));
	unsigned int yi = (unsigned int)(y * (mHeight-1));

	switch( mBytesPerPixel )
	{
	case 1:
		{
			unsigned int i = xi + mWidth * yi;
			if( r )
				*r = mBuffer[i];
			if( g )
				*g = mBuffer[i];
			if( b )
				*b = mBuffer[i];
		}
		break;
	case 3:
	case 4:
		{
			unsigned int i = (xi + mWidth * yi) * mBytesPerPixel;
			if( r )
				*r = mBuffer[i + 0];
			if( g )
				*g = mBuffer[i + 1];
			if( b )
				*b = mBuffer[i + 2];
		}
		break;
	default:
		assert( 0 && "invalid bytes per pixel" );
		return false;
	}
	return true;
}
