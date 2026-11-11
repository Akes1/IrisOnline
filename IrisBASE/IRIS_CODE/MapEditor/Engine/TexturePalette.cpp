#include "StdAfx.h"
#include "TexturePalette.h"

#include "RenderSystem.h"
#include "ResourceManager.h"
#include "Stream.h"
#include "EngineFile.h"
#include "ImageReader.h"

NiFastCriticalSection mCriticalSection;

cTexturePalette::cTexturePalette( unsigned int /*sizeHint*/ )
: NiDefaultTexturePalette( false )
{
}

cTexturePalette::~cTexturePalette()
{
}

void cTexturePalette::RemoveAllTextures()
{
	mCriticalSection.Lock();

	//#ifdef _DEBUG
	//	//NiTMapIterator mapiter = m_kHash.GetFirstPos();
	//	//while (mapiter)
	//	//{
	//	//	const char* name;
	//	//	NiTexturePtr texture;
	//	//	m_kHash.GetNext(mapiter, name, texture);
	//
	//	//	unsigned int count = texture->GetRefCount();
	//	//	texture = 0;
	//	//	assert(count == 2);
	//	//}
	//#endif

	m_kHash.RemoveAll();

	mCriticalSection.Unlock();
}

void cTexturePalette::RemoveTexture( char* texName )
{
	assert( ::strlen(texName) );

	NiFilename filename(texName);

	mCriticalSection.Lock();
	m_kHash.RemoveAt( filename.GetFilename() );
	mCriticalSection.Unlock();
}


void cTexturePalette::SetTexture( const char* pathName, NiTexture* tex )
{
	assert( pathName );
	assert( tex );

	NiFilename filename(pathName);

	mCriticalSection.Lock();
	m_kHash.SetAt( filename.GetFilename(), tex );
	mCriticalSection.Unlock();
}


NiTexture* cTexturePalette::GetTexture( const char* pathName, NiTexture* /*pkSelf = NULL*/ ) const
{
	assert( pathName );

	/// 이미 존재하는지 검사
	NiFilename filename(pathName);

	if( _strcmpi(filename.GetFilename(), "tx_font_number") == 0 )
	{
		int a = 0;
		a = 1;
	}

	mCriticalSection.Lock();
	NiTexture* nitex = NiDefaultTexturePalette::GetTexture( filename.GetFilename() );
	if( nitex )
	{
		mCriticalSection.Unlock();
		return nitex;
	}
	mCriticalSection.Unlock();

	return 0;
}

NiTexture* cTexturePalette::LoadTexture( const char* pathName )
{
	NiTexture* nitex = 0;

	/// 텍스처 파일을 로딩
	NiPixelDataPtr pixelData = 0;
	NiFilename filename(pathName);
	if( _strcmpi( filename.GetExt(), ".nif") == 0 )
	{
		cFileLoader loader;
		if( loader.Open( pathName, true ) == false )
			return 0;

		cStream stream( pathName, 0 );
		if( stream.Load( (char*)loader.GetBufferPtr(), loader.GetSize() ) == false )
			return 0;

		NiObject* obj = stream.GetObjectAt(0);
		if( NiIsKindOf( NiPixelData, obj) )
			pixelData = (NiPixelData*)obj;
		else
			return 0;
	}
	else if( _strcmpi( filename.GetExt(), ".tga") == 0 )
	{
		cEngineFile file( pathName );
		if( file == false )
			return 0;

		cTGAReader tgaReader;
		pixelData = tgaReader.ReadFile( file, 0 );
	}
	else if( _strcmpi( filename.GetExt(), ".bmp" ) == 0 )
	{
		return 0;
	}
	else if( _strcmpi( filename.GetExt(), ".dds") == 0 )
	{
		//*
		cFileLoader loader;
		if( loader.Open( pathName, true ) == false )
			return 0;

		LPDIRECT3DTEXTURE9 dxTex = 0;
		cRenderer* renderer = RENDERSYS->GetRenderer();
		HRESULT ret = ::D3DXCreateTextureFromFileInMemoryEx( renderer->GetD3DDevice(), loader.GetBufferPtr(), loader.GetSize(),
			D3DX_DEFAULT_NONPOW2 , D3DX_DEFAULT_NONPOW2 , 0, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 0, 0, 0,
			&dxTex );

		if( FAILED(ret) || dxTex == 0 )
		{
			assert( 0 );
			return 0;
		}

		NiTexturePtr ddsTex = RENDERSYS->GetRenderer()->CreateNiTextureFromD3DTexture( (LPDIRECT3DBASETEXTURE9)dxTex );
		if( ddsTex == 0 )
		{
			assert( 0 );
			return 0;
		}

		cString name;
		::GetFileName( &name, pathName );

		NiSourceTexture* tex = NiDynamicCast( NiSourceTexture, ddsTex );
		tex->SetFilename( name.Cstr() );

		SetTexture( filename.GetFilename(), ddsTex );
		return ddsTex;
	}
	else
	{
		assert( 0 );
		return 0;
	}

	if( pixelData == 0 )
	{
		assert( 0 );
		return 0;
	}

	/// 픽셀 데이타로부터 엔진 텍스처를 생성
	NiTexture::FormatPrefs prefs;
	prefs.m_eMipMapped = pixelData->GetNumMipmapLevels() <= 1 ? NiTexture::FormatPrefs::NO : NiTexture::FormatPrefs::YES;

	nitex = NiSourceTexture::Create( pixelData, prefs );
	if( nitex == 0 )
	{
		assert( 0 );
		return 0;
	}

	cString name;
	::GetFileName( &name, pathName );
	((NiSourceTexture*)nitex)->SetFilename( name.Cstr() );

	SetTexture( filename.GetFilename(), nitex );
	return nitex;
}
