#include "stdafx.h"
#include "SkinResource.h"

cSkinRoundGaugeResource::cSkinRoundGaugeResource()
{
}

cSkinRoundGaugeResource::~cSkinRoundGaugeResource()
{
}

bool cSkinRoundGaugeResource::Load( cParser& parser )
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
			/// 이미지 로딩
			cString fileName;
			parser.ParseString( &fileName );

			if( LoadImage( &mBitmap, &mWidth, &mHeight, fileName ) == false )
			{
				return false;
			}

			/// 화살표 이미지 로딩
			parser.ParseString( &fileName );

			if( LoadImage( &mArrowBitmap, &mWidth, &mHeight, fileName ) == false )
			{
				return false;
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
