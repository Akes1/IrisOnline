#include "stdafx.h"
#include "SkinResource.h"

cSkinDialogResource::cSkinDialogResource()
: mErrorTxt( "Error!" )
{
	/// 폰트 초기화
	mFont.lfHeight = 12;
	mFont.lfWidth = 0;
	mFont.lfEscapement = 0;
	mFont.lfOrientation = 0;
	mFont.lfWeight = FW_NORMAL;
	mFont.lfItalic = 0;
	mFont.lfUnderline = 0;
	mFont.lfStrikeOut = 0;
	mFont.lfCharSet = DEFAULT_CHARSET;
	mFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	mFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	mFont.lfQuality = DEFAULT_QUALITY;
	mFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy( mFont.lfFaceName, "굴림" );
}

cSkinDialogResource::~cSkinDialogResource()
{
}

const cString& cSkinDialogResource::GetText( unsigned int id )
{
	cStringMap::cConstIterator i = mStringMap.Find( id );

	if( i == mStringMap.End() )
	{
		assert( 0 && "failed to find ui text" );
		return mErrorTxt;
	}
	else
		return i->mSecond;
}

bool cSkinDialogResource::Load( cParser& parser )
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
		else if( keyword == "fontface" )
		{
			/// 글자체 설정
			cString fontFace;
			parser.ParseString( &fontFace );

			strcpy( mFont.lfFaceName, fontFace.Cstr() );
		}
		else if( keyword == "fontsize" )
		{
			/// 글자 크기 설정
			mFont.lfHeight = parser.ParseInt();
		}
		else if( keyword == "dlgtext" )
		{
			if( LoadText( parser) == false )
				return false;
		}
		else if( keyword == "button" )
		{
			cString name;
			if( parser.ParseString( &name ) == false)
			{
				return false;
			}

			/// 버튼 리소스 생성
			cSkinButtonResource* p = new cSkinButtonResource;

			/// 로딩
			if( p->Load( parser ) )
			{
				/// 콤포넌트 리소스 맵에 추가
				mComponentResourceMap.Insert( name, (cSkinComponentResource*)p );
			}
			else
			{
				return false;
			}
		}
		else if( keyword == "progressbar" )
		{
			cString name;
			if( parser.ParseString( &name ) == false)
			{
				return false;
			}

			/// 진행바 리소스 생성
			cSkinProgressBarResource* p = new cSkinProgressBarResource;

			/// 로딩
			if( p->Load( parser ) )
			{
				/// 콤포넌트 리소스 맵에 추가
				mComponentResourceMap.Insert( name, (cSkinComponentResource*)p );
			}
			else
			{
				delete p;
				return false;
			}
		}
		//else if( keyword == "imageslide" )
		//{
		//	cString name;
		//	if( parser.ParseString( &name ) == false)
		//	{
		//		return false;
		//	}

		//	/// 이미지 슬라이드 리소스 생성
		//	cSkinImageSlideResource* p = new cSkinImageSlideResource;

		//	/// 로딩
		//	if( p->Load( parser ) )
		//	{
		//		/// 콤포넌트 리소스 맵에 추가
		//		mComponentResourceMap.Insert( name, (cSkinComponentResource*)p );
		//	}
		//	else
		//	{
		//		return false;
		//	}
		//}
		else if( keyword == "htmlview" )
		{
			cString name;
			if( parser.ParseString( &name ) == false)
			{
				return false;
			}

			/// Html 뷰 리소스 생성
			cSkinHtmlViewResource* p = new cSkinHtmlViewResource;

			/// 로딩
			if( p->Load( parser ) )
			{
				/// 콤포넌트 리소스 맵에 추가
				mComponentResourceMap.Insert( name, (cSkinComponentResource*)p );
			}
			else
			{
				return false;
			}
		}
		else if( keyword == "combobox" )
		{
			cString name;
			if( parser.ParseString( &name ) == false)
			{
				return false;
			}

			/// 콤보박스 리소스 생성
			cSkinComboBoxResource* p = new cSkinComboBoxResource;

			/// 로딩
			if( p->Load( parser ) )
			{
				mComponentResourceMap.Insert( name, (cSkinComponentResource*)p );
			}
			else
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

bool cSkinDialogResource::LoadText( cParser& parser )
{
	if( parser.ExpectTokenString( "{" ) == false )
		return false;

	cString keyword;
	while( true )
	{
		parser.ParseString( &keyword );
		if( keyword == "txt" )
		{
			unsigned int key = parser.ParseInt();

			cString txt;
			parser.ParseString( &txt );

			if( mStringMap.Insert( key, txt.Cstr() ) == false )
				assert(0);
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
