#include "stdafx.h"
#include "SkinResource.h"

cSkinHtmlViewResource::cSkinHtmlViewResource()
{
}

cSkinHtmlViewResource::~cSkinHtmlViewResource()
{
}

bool cSkinHtmlViewResource::Load( cParser& parser )
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

		if( keyword == "url" )
		{
			/// url
			parser.ParseString( &mUrl );
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
		else if( keyword == "size" )
		{
			/// 크기
			mWidth = parser.ParseInt();
			mHeight = parser.ParseInt();
			mRect.right = mRect.left + mWidth;
			mRect.bottom = mRect.top + mHeight;
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
