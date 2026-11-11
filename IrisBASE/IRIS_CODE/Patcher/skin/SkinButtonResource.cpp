#include "stdafx.h"
#include "SkinResource.h"

cSkinButtonResource::cSkinButtonResource()
: mStyle( 0 )
{
}

cSkinButtonResource::~cSkinButtonResource()
{
}

bool cSkinButtonResource::Load( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
	{
		return false;
	}

	cString keyword;

	while( true )
	{
		if( parser.ParseString( &keyword ) == false )
		{
			return false;
		}

		if( keyword == "image" )
		{
			/// 이미지 로딩
			cString fileName;
			parser.ParseString( &fileName );

			if( LoadImage( &mBitmap, &mWidth, &mHeight, fileName ) == false )
			{
				return false;
			}
		}
		else if( keyword == "pos" )
		{
			/// 위치 파싱
			int x = parser.ParseInt();
			int y = parser.ParseInt();
			mRect.left = x;
			mRect.top = y;
			mRect.right = x + mWidth;
			mRect.bottom = y + mHeight;
		}
		else if( keyword == "size" )
		{
			/// 크기 파싱
			mWidth = parser.ParseInt();
			mHeight = parser.ParseInt();
			mRect.right = mRect.left + mWidth;
			mRect.bottom = mRect.top + mHeight;
		}
		else if( keyword == "style" )
		{
			/// 스타일 파싱
			if( parser.ParseInt( &mStyle ) == false)
			{
				return false;
			}
		}
		else if( keyword == "}" )
		{
			/// 블록 탈출 ^^
			break;
		}
		else
		{
			return false;
		}
	}
	return true;
}