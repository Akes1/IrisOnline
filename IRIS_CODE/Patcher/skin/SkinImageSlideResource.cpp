#include "stdafx.h"
#include "SkinResource.h"

cSkinImageSlideResource::cSkinImageSlideResource()
{
}

cSkinImageSlideResource::~cSkinImageSlideResource()
{
	for(int i = 0, numImages = mBitmapArray.GetCount(); i < numImages; ++i)
	{
		mBitmapArray[i].DeleteObject();
	}
}

bool cSkinImageSlideResource::Load( cParser& parser )
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

		if( keyword == "images" )
		{
			/// 이미지 수
			int numImages = parser.ParseInt();

			/// 이미지 로딩
			mBitmapArray.SetSize( numImages );
			cString fileName;

			for( int i = 0; i < numImages; ++i)
			{
				parser.ParseString( &fileName );

				if( LoadImage( &mBitmapArray[i], &mWidth, &mHeight, fileName ) == false )
				{
					return false;
				}
			}
		}
		else if( keyword == "pos" )
		{
			/// 위치
			int x = parser.ParseInt();
			int y = parser.ParseInt();
			mRect.left = x;
			mRect.top = y;
			mRect.right = x + mWidth;
			mRect.bottom = y + mHeight;
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
