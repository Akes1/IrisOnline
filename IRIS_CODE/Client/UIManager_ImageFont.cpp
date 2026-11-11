#include "stdafx.h"
#include "UIManager.h"
#include "Tokenizer.h"
#include "UIImageFont.h"

bool cUIManager::LoadImageFonts( const cString& pathName )
{
	/// 파일 열기
	cFileLoader loader;

	if( loader.Open( pathName, true ) == false )
	{
		assert( 0 && "failed to load image fonts" );
		return false;
	}

	cTokenizer tokenizer( loader.GetBufferPtr(), loader.GetSize(), " \t\r\n", pathName.Cstr() );
	cString str;

	while( tokenizer.IsEnd() == false )
	{
		/// 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int index = str.ToInt();

		/// 이미지 인덱스
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int imageIndex = str.ToInt();

		/// 텍스처 시작 위치
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int texX = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int texY = str.ToInt();

		/// 텍스처 폰트 문자 크기
		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int charWidth = str.ToInt();

		if( tokenizer.GetNext( &str ) == false )
			return false;

		unsigned int charHeight = str.ToInt();

		/// 이미지 폰트를 생성
		NiTexture* tex = UIMAN->GetSkin()->GetTexture( imageIndex );
		if( tex == 0 )
			continue;

		cUIImageFont* imageFont = new cUIImageFont( tex, texX, texY, charWidth, charHeight );

		if( mImageFontMap.Insert( index, imageFont ) == false )
		{
			assert( 0 && "failed to insert image font, maybe already exist" );
			return false;
		}
	}
	return true;
}

cUIImageFont* cUIManager::GetImageFont( unsigned int fontIndex )
{
	cImageFontMap::cIterator i = mImageFontMap.Find( fontIndex );

	if( i == mImageFontMap.End() )
		return 0;
	else
		return (cUIImageFont*)(i->mSecond);
}
