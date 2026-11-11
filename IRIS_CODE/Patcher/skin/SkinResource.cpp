#include "stdafx.h"
#include "SkinResource.h"

cString cSkinComponentResource::mPath;

cSkinComponentResource::cSkinComponentResource()
: mWidth( 0 )
, mHeight( 0 )
{
	mRect.SetRectEmpty();
}

cSkinComponentResource::~cSkinComponentResource()
{
	mBitmap.DeleteObject();
}

bool cSkinComponentResource::LoadImage( CBitmap* bitmap, int* width, int* height, const cString& fileName )
{
	cString pathName = mPath;
	pathName += fileName;

	/// 이미지 로딩
	cTargaImage img;
	if( img.Load( pathName ) == false )
	{
		return false;
	}

	img.ChangeColorByteOrder();

	/// 이미지로부터 장치 독립 비트맵을 생성
	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bi.bmiHeader.biWidth = img.GetWidth();
	bi.bmiHeader.biHeight = img.GetHeight();
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = img.GetBytesPerPixel() * 8;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = (DWORD)img.GetSize();
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;
	bi.bmiColors[0].rgbBlue = 0;
	bi.bmiColors[0].rgbGreen = 0;
	bi.bmiColors[0].rgbRed = 0;
	bi.bmiColors[0].rgbReserved = 0;

	void* p = 0;
	HDC memdc = CreateCompatibleDC( 0 );
	HBITMAP hbmp = CreateDIBSection( memdc, &bi, DIB_RGB_COLORS, &p, 0, 0 );

	if( p )
	{
		unsigned int bytes = (((bi.bmiHeader.biBitCount*img.GetWidth() + 31) & (~31))/8)*img.GetHeight();
		if( bytes == img.GetSize() )
		{
			::memcpy( p, img.GetBufferPtr(), img.GetSize() );
		}
		else
		{
			char* src = (char*)img.GetBufferPtr();
			char* dest = (char*)p;

			unsigned int srcOffset = img.GetSize()/img.GetHeight();
			unsigned int destOffset = bytes/img.GetHeight();

			for( unsigned int i = 0;i<img.GetHeight();i++ )
			{
				::memcpy( dest, src, srcOffset );

				src += srcOffset;
				dest += destOffset;
			}
		}
	}
	DeleteDC( memdc );

	/// 비트맵 부착
	bitmap->DeleteObject();
	bitmap->Attach( hbmp );
	if( width )
	{
		*width = img.GetWidth();
	}
	if( height )
	{
		*height = img.GetHeight();
	}
	return true;
}

cSkinContainerResource::cSkinContainerResource()
{
}

cSkinContainerResource::~cSkinContainerResource()
{
	cComponentResourceMap::cIterator i = mComponentResourceMap.Begin();
	cComponentResourceMap::cIterator end = mComponentResourceMap.End();

	for( ; i != end; ++i )
	{
		delete (*i).mSecond;
	}
}

cSkinComponentResource* cSkinContainerResource::GetChild( const cString& name )
{
	cComponentResourceMap::cIterator i = mComponentResourceMap.Find( name );
	if( i == mComponentResourceMap.End() )
	{
		return 0;
	}
	return i->mSecond;
}

cSkinManager::cSkinManager( const cString& path )
{
	cSkinComponentResource::mPath = path;
}

cSkinManager::~cSkinManager()
{
	cContainerResourceMap::cIterator i = mContainerResourceMap.Begin();
	cContainerResourceMap::cIterator end = mContainerResourceMap.End();

	for( ; i != end; ++i )
	{
		delete i->mSecond;
	}
}

bool cSkinManager::Load( const cString& fileName )
{
	/// 파일 열기
	cFileLoader loader;
	if( loader.Open( fileName, true ) == false )
	{
		return false;
	}

	/// 렉서에 밥주고 파싱
	cToken token;
	cLexer lexer( loader.GetBufferPtr(), loader.GetSize() );
	cParser parser( &lexer, fileName );

	while( lexer.IsEnd() == false )
	{
		lexer.GetNextToken( &token );

		if( token.mType == eTOKEN_NULL )
		{
			continue;
		}
		else if( token.mType == eTOKEN_ERROR )
		{
			return false;
		}

		if( token == "dialog" )
		{
			/// 다이얼로그 속성 생성
			cSkinDialogResource* p = new cSkinDialogResource;
			cString key;

			if( parser.ParseString( &key ) == false )
			{
				delete p;
				return false;
			}

			/// 로딩
			if( p->Load( parser ) )
			{
				/// 컨테이너 맵에 추가
				mContainerResourceMap.Insert( key, (cSkinContainerResource*)p );
			}
			else
			{
				delete p;
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}


cSkinContainerResource* cSkinManager::GetContainer( const cString& name )
{
	cContainerResourceMap::cIterator i = mContainerResourceMap.Find( name );
	if( i == mContainerResourceMap.End() )
	{
		return 0;
	}
	return i->mSecond;
}
